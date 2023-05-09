//
// This file is part of Taffy, a mathematical programming language.
// Copyright (C) 2016-2017 Arithmagic, LLC
//
// Taffy is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Taffy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <assert.h>
#include <string.h>

#include "dcHash.h"
#include "dcArray.h"
#include "dcError.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"
#include "dcUnsignedInt32.h"

///////////////////
// dcHashElement //
///////////////////

static dcHashElement *createHashElement(dcHashElementKey *_key, dcNode *_value)
{
    dcHashElement *element = (dcHashElement *)(dcMemory_allocate
                                               (sizeof(dcHashElement)));
    memcpy(&element->key, _key, sizeof(dcHashElementKey));
    element->value = _value;
    return element;
}

static dcNode *createHashElementNode(dcHashElementKey *_key, dcNode *_value)
{
    return dcNode_createWithGuts(NODE_HASH_ELEMENT,
                                 createHashElement(_key, _value));
}

void dcHashElement_free(dcHashElement **_element, dcDepth _depth)
{
    dcHashElement *element = *_element;
    dcNode_tryFree(&element->value, _depth);

    if (element->key.isNodeKey)
    {
        dcNode_tryFree(&element->key.keyUnion.nodeKey, _depth);
    }
    else
    {
        dcMemory_free(element->key.keyUnion.stringKey);
    }

    dcMemory_free(*_element);
}

dcHashElement *dcHashElement_copy(const dcHashElement *_from, dcDepth _depth)
{
    dcHashElementKey key;
    memcpy(&key, &_from->key, sizeof(dcHashElementKey));

    if (_from->key.isNodeKey)
    {
        key.keyUnion.nodeKey =
            dcNode_tryCopy(_from->key.keyUnion.nodeKey, _depth);
    }
    else
    {
        key.keyUnion.stringKey = dcMemory_strdup(_from->key.keyUnion.stringKey);
    }

    return createHashElement(&key, dcNode_tryCopy(_from->value, _depth));
}

dcResult dcHashElement_printNode(const dcNode *_node, dcString *_stream)
{
    dcHashElement *element = CAST_HASH_ELEMENT(_node);
    dcResult result = TAFFY_SUCCESS;
    char *valueDisplay = dcNode_display(element->value);

    if (valueDisplay != NULL)
    {
        char *keyDisplay = (element->key.isNodeKey
                            ? dcNode_display(element->key.keyUnion.nodeKey)
                            : element->key.keyUnion.stringKey);

        if (keyDisplay != NULL)
        {
            dcString_append(_stream,
                            "(%u) %s%s => %s",
                            element->key.hash,
                            (element->key.isNodeKey
                             ? "N-"
                             : "S-"),
                            keyDisplay,
                            valueDisplay);
        }
        else
        {
            // stringKey should never be NULL
            dcError_assert(element->key.isNodeKey);
            result = TAFFY_EXCEPTION;
        }
    }
    else
    {
        result = TAFFY_EXCEPTION;
    }

    return result;
}

void dcHashElement_mark(dcHashElement *_hashElement)
{
    dcNode_mark(_hashElement->value);

    // don't double-mark
    if (_hashElement->key.isNodeKey
        && _hashElement->key.keyUnion.nodeKey != _hashElement->value)
    {
        dcNode_mark(_hashElement->key.keyUnion.nodeKey);
    }
}

void dcHashElement_markNode(dcNode *_hashElementNode)
{
    dcHashElement_mark(CAST_HASH_ELEMENT(_hashElementNode));
}

void dcHashElement_registerNode(dcNode *_hashElementNode)
{
    dcHashElement *hashElement = CAST_HASH_ELEMENT(_hashElementNode);

    if (hashElement->key.isNodeKey)
    {
        dcNode_register(hashElement->key.keyUnion.nodeKey);
    }

    dcNode_register(hashElement->value);
}

////////////
// dcHash //
////////////

dcHash *dcHash_create(void)
{
    return dcHash_createWithCapacity(DEFAULT_HASH_CAPACITY);
}

dcNode *dcHash_createNode(void)
{
    return (dcNode_createWithGuts
            (NODE_HASH,
             dcHash_createWithCapacity(DEFAULT_HASH_CAPACITY)));
}

dcHash *dcHash_createWithCapacity(uint16_t _capacity)
{
    dcHash *hash = (dcHash *)dcMemory_allocate(sizeof(dcHash));
    hash->buckets = (dcList **)(dcMemory_allocateAndInitialize
                                (sizeof(dcList *) * _capacity));
    hash->capacity = _capacity;
    hash->size = 0;
    return hash;
}

void dcHash_free(dcHash **_hash, dcDepth _depth)
{
    if (*_hash != NULL)
    {
        dcHash *hash = *_hash;

        if (hash->buckets != NULL)
        {
            uint16_t i;

            for (i = 0; i < hash->capacity; i++)
            {
                dcList_free(&hash->buckets[i], _depth);
            }

            dcMemory_free(hash->buckets);
        }
    }

    dcMemory_free(*_hash);
}

dcHashIterator *dcHash_createIterator(const dcHash *_hash)
{
    return dcHashIterator_create(_hash);
}

void dcHash_markNode(dcNode *_hash)
{
    dcHash *hash = CAST_HASH(_hash);
    uint16_t i;

    for (i = 0; i < hash->capacity; i++)
    {
        dcList_mark(hash->buckets[i]);
    }
}

void dcHash_register(dcHash *_hash)
{
    if (_hash != NULL)
    {
        uint16_t i;

        for (i = 0; i < _hash->capacity; i++)
        {
            dcList_register(_hash->buckets[i]);
        }
    }
}

static void set(dcHash *_hash, dcNode *_hashElement)
{
    dcHashElement *element = CAST_HASH_ELEMENT(_hashElement);
    dcContainerSizeType k = element->key.hash % _hash->capacity;
    dcList *list = _hash->buckets[k];

    if (list == NULL)
    {
        list = dcList_create();
        _hash->buckets[k] = list;
    }

    dcList_push(list, _hashElement);
    _hash->size++;
}

static float sLoadFactor = 0.75;

static dcResult getHashElementWithKeys(const dcHash *_hash,
                                       dcHashElementKey *_key,
                                       dcNode **_getResult,
                                       dcList **_listResult,
                                       dcListElement **_listElementResult)
{
    dcResult result = TAFFY_FAILURE;
    dcList *bucket = _hash->buckets[_key->hash % _hash->capacity];

    if (bucket != NULL)
    {
        FOR_EACH_IN_LIST(bucket, that)
        {
            dcHashElement *element = CAST_HASH_ELEMENT(that->object);

            if (element->key.hash == _key->hash)
            {
                // we have preliminary success...
                dcResult myResult = TAFFY_SUCCESS;
                dcTaffyOperator compareResult = TAFFY_EQUALS;

                if (_key != NULL)
                {
                    // both the key and keyHash were given, so do a comparison
                    if (element->key.isNodeKey == _key->isNodeKey)
                    {
                        if (element->key.isNodeKey)
                        {
                            myResult = dcNode_compareEqual
                                (element->key.keyUnion.nodeKey,
                                 _key->keyUnion.nodeKey,
                                 &compareResult);
                        }
                        else
                        {
                            myResult = TAFFY_SUCCESS;
                            compareResult = dcMemory_taffyStringCompare
                                (element->key.keyUnion.stringKey,
                                 _key->keyUnion.stringKey);
                        }
                    }
                    else
                    {
                        compareResult = TAFFY_LESS_THAN;
                    }
                }

                if (myResult == TAFFY_SUCCESS
                    && compareResult == TAFFY_EQUALS)
                {
                    result = TAFFY_SUCCESS;

                    if (_getResult != NULL)
                    {
                        *_getResult = that->object;
                    }

                    if (_listElementResult != NULL)
                    {
                        *_listElementResult = that;
                    }

                    if (_listResult != NULL)
                    {
                        *_listResult = bucket;
                    }

                    break;
                }
                else if (myResult == TAFFY_EXCEPTION)
                {
                    result = TAFFY_EXCEPTION;
                    break;
                }
            }
        }
    }

    return result;
}

static dcResult setValue(dcHash *_hash,
                         dcHashElementKey *_key,
                         dcNode *_value)
{
    dcNode *element = NULL;
    dcResult result = TAFFY_FAILURE;
    dcResult getResult = getHashElementWithKeys(_hash,
                                                _key,
                                                &element,
                                                NULL,
                                                NULL);
    if (getResult == TAFFY_EXCEPTION)
    {
        result = TAFFY_EXCEPTION;

        if (! _key->isNodeKey)
        {
            dcMemory_free(_key->keyUnion.stringKey);
        }
    }
    else
    {
        result = TAFFY_SUCCESS;

        if (element != NULL)
        {
            CAST_HASH_ELEMENT(element)->value = _value;

            if (! _key->isNodeKey)
            {
                dcMemory_free(_key->keyUnion.stringKey);
            }
        }
        else
        {
            element = createHashElementNode(_key, _value);
            set(_hash, element);
        }

        //
        // rehash if needed!
        // overflow possibility
        //
        if ((float)_hash->size * sLoadFactor >= _hash->capacity)
        {
            dcHash *tempHash =
                dcHash_createWithCapacity(_hash->capacity * 2);
            dcHashIterator *i = dcHash_createIterator(_hash);
            dcNode *that = NULL;

            while ((that = dcHashIterator_getNext(i))
                   != NULL)
            {
                set(tempHash, that);
            }

            // perform a bucket switcheroo!
            dcContainerSizeType j;

            for (j = 0; j < _hash->capacity; j++)
            {
                dcList_free(&_hash->buckets[j], DC_FLOATING);
            }

            dcMemory_free(_hash->buckets);

            _hash->buckets = tempHash->buckets;
            _hash->capacity = tempHash->capacity;
            tempHash->buckets = NULL;
            dcHash_free(&tempHash, DC_DEEP);
            dcHashIterator_free(&i);
        }
    }

    return result;
}

dcResult dcHash_setValueWithHashValue(dcHash *_hash,
                                      dcNode *_key,
                                      dcHashType _hashValue,
                                      dcNode *_value)
{
    dcHashElementKey key;
    key.hash = _hashValue;
    key.keyUnion.nodeKey = _key;
    key.isNodeKey = true;
    return setValue(_hash, &key, _value);
}

dcResult dcHash_setValue(dcHash *_hash, dcNode *_key, dcNode *_value)
{
    dcHashType hashValue = 0;
    dcResult result = dcNode_hash(_key, &hashValue);

    return (result == TAFFY_SUCCESS
            ? dcHash_setValueWithHashValue(_hash, _key, hashValue, _value)
            : result);
}

dcResult dcHash_setValueWithStringKey(dcHash *_hash,
                                      const char *_key,
                                      dcNode *_value)
{
    dcHashType hashValue;
    assert(dcString_hashCharArray(_key, &hashValue) == TAFFY_SUCCESS);
    return dcHash_setValueWithStringKeys(_hash, _key, hashValue, _value);
}

dcResult dcHash_setValueWithStringKeys(dcHash *_hash,
                                       const char *_key,
                                       dcHashType _hashValue,
                                       dcNode *_value)
{
    dcHashElementKey key;
    key.keyUnion.stringKey = dcMemory_strdup(_key);
    key.isNodeKey = false;
    key.hash = _hashValue;
    return setValue(_hash, &key, _value);
}

dcResult dcHash_getHashElement(const dcHash *_hash,
                               dcNode *_key,
                               dcNode **_result)
{
    dcHashElementKey key;
    key.isNodeKey = true;
    key.keyUnion.nodeKey = _key;
    dcResult hashResult = dcNode_hash(_key, &key.hash);

    return (hashResult == TAFFY_SUCCESS
            ? getHashElementWithKeys(_hash,
                                     &key,
                                     _result,
                                     NULL,
                                     NULL)
            : hashResult);
}

static dcResult extractValue(dcResult _givenResult, dcNode **_hashElement)
{
    if (_hashElement != NULL)
    {
        *_hashElement = (_givenResult == TAFFY_SUCCESS
                         ? CAST_HASH_ELEMENT(*_hashElement)->value
                         : NULL);
    }

    return _givenResult;
}

dcResult dcHash_getValueWithKeys(const dcHash *_hash,
                                 dcNode *_key,
                                 dcHashType _keyHash,
                                 dcNode **_value)
{
    dcHashElementKey key;
    key.isNodeKey = true;
    key.hash = _keyHash;
    key.keyUnion.nodeKey = _key;

    return (extractValue(getHashElementWithKeys(_hash,
                                                &key,
                                                _value,
                                                NULL,
                                                NULL),
                         _value));
}

dcResult dcHash_getValue(const dcHash *_hash, dcNode *_key, dcNode **_value)
{
    return extractValue(dcHash_getHashElement(_hash, _key, _value), _value);
}

dcResult dcHash_getValueWithStringKey(const dcHash *_hash,
                                      const char *_key,
                                      dcNode **_value)
{
    dcHashType hashValue = 0;
    assert(dcString_hashCharArray(_key, &hashValue) == TAFFY_SUCCESS);
    return dcHash_getValueWithStringKeys(_hash, _key, hashValue, _value);
}

dcResult dcHash_getValueWithStringKeys(const dcHash *_hash,
                                       const char *_key,
                                       dcHashType _hashValue,
                                       dcNode **_value)
{
    dcHashElementKey key;
    key.isNodeKey = false;
    key.keyUnion.stringKey = (char*)_key; // will not be modified
    key.hash = _hashValue;
    return (extractValue(getHashElementWithKeys(_hash,
                                                &key,
                                                _value,
                                                NULL,
                                                NULL),
                         _value));
}

dcHash *dcHash_copy(const dcHash *_from, dcDepth _depth)
{
    dcHash *result = dcHash_createWithCapacity(_from->capacity);
    uint16_t i;

    for (i = 0; i < _from->capacity; i++)
    {
        result->buckets[i] = dcList_copy(_from->buckets[i], _depth);
    }

    result->size = _from->size;
    return result;
}

void dcHash_clear(dcHash *_hash, dcDepth _depth)
{
    if (_hash != NULL)
    {
        uint16_t i = 0;

        for (i = 0; i < _hash->capacity; i++)
        {
            dcList_clear(_hash->buckets[i], _depth);
        }
    }
}

void dcHash_mark(dcHash *_hash)
{
    if (_hash != NULL)
    {
        uint16_t i;

        for (i = 0; i < _hash->capacity; i++)
        {
            dcList_mark(_hash->buckets[i]);
        }
    }
}

dcArray *dcHash_getValues(const dcHash *_hash)
{
    dcArray *values = dcArray_createWithSize(_hash->size);
    size_t i = 0;

    for (i = 0; i < _hash->capacity; i++)
    {
        if (_hash->buckets[i] != NULL)
        {
            FOR_EACH_IN_LIST(_hash->buckets[i], that)
            {
                dcArray_add(values, CAST_HASH_ELEMENT(that->object)->value);
            }
        }
    }

    return values;
}

dcResult dcHash_print(const dcHash *_hash, dcString *_stream)
{
    dcHashIterator *i = dcHash_createIterator(_hash);
    dcNode *node = NULL;
    dcResult result = TAFFY_SUCCESS;
    bool first = true;

    while ((node = dcHashIterator_getNext(i))
           != NULL
           && result == TAFFY_SUCCESS)
    {
        if (! first)
        {
            dcString_appendString(_stream, ", ");
        }

        first = false;
        result = dcNode_print(node, _stream);
    }

    dcHashIterator_free(&i);
    return result;
}

static dcResult dcHash_remove(dcHash *_hash,
                              dcHashElementKey *_key,
                              dcNode **_removed,
                              dcDepth _depth)
{
    dcNode *hashElementNode = NULL;
    dcListElement *listElement = NULL;
    dcList *list = NULL;

    dcResult result = getHashElementWithKeys(_hash,
                                             _key,
                                             &hashElementNode,
                                             &list,
                                             &listElement);

    if (result == TAFFY_SUCCESS)
    {
        if (_removed != NULL && _depth != DC_DEEP)
        {
            *_removed = CAST_HASH_ELEMENT(hashElementNode)->value;
        }

        dcList_removeElement(list, &listElement, _depth);
        dcError_assert(_hash->size > 0);
        _hash->size--;
    }

    return result;
}

dcResult dcHash_removeValue(dcHash *_hash,
                            dcNode *_key,
                            dcNode **_removed,
                            dcDepth _depth)
{
    dcHashType hashValue = 0;
    dcResult result = dcNode_hash(_key, &hashValue);
    return (result == TAFFY_SUCCESS
            ? dcHash_removeValueWithHashValue(_hash,
                                              _key,
                                              hashValue,
                                              _removed,
                                              _depth)
            : result);
}

dcResult dcHash_removeValueWithHashValue(dcHash *_hash,
                                         dcNode *_key,
                                         dcHashType _hashValue,
                                         dcNode **_removed,
                                         dcDepth _depth)
{
    dcHashElementKey key;
    key.keyUnion.nodeKey = _key;
    key.isNodeKey = true;
    key.hash = _hashValue;
    return dcHash_remove(_hash, &key, _removed, _depth);
}

dcResult dcHash_removeValueWithStringKey(dcHash *_hash,
                                         const char *_key,
                                         struct dcNode_t **_removed,
                                         dcDepth _depth)
{
    dcHashType hashValue;
    assert(dcString_hashCharArray(_key, &hashValue) == TAFFY_SUCCESS);
    return dcHash_removeValueWithStringKeys(_hash,
                                            _key,
                                            hashValue,
                                            _removed,
                                            _depth);
}

dcResult dcHash_removeValueWithStringKeys(dcHash *_hash,
                                          const char *_key,
                                          dcHashType hashValue,
                                          struct dcNode_t **_removed,
                                          dcDepth _depth)
{
    dcHashElementKey key;
    key.keyUnion.stringKey = (char*)_key; // will not be modified
    key.isNodeKey = false;
    key.hash = hashValue;
    return dcHash_remove(_hash, &key, _removed, _depth);
}

// create dcHash_display
dcTaffy_createDisplayFunctionMacro(dcHash);

// create dcHash_printNode
dcTaffy_createPrintNodeFunctionMacro(dcHash, CAST_HASH);

// create dcHash_copyNode
dcTaffy_createCopyNodeFunctionMacro(dcHash, CAST_HASH);

// create dcHash_freeNode
dcTaffy_createFreeNodeFunctionMacro(dcHash, CAST_HASH);

// create dcHash_registerNode
dcTaffy_createRegisterNodeFunctionMacro(dcHash, CAST_HASH);

// create dcHashElement_copyNode
dcTaffy_createCopyNodeFunctionMacro(dcHashElement, CAST_HASH_ELEMENT);

// create dcHashElement_freeNode
dcTaffy_createFreeNodeFunctionMacro(dcHashElement, CAST_HASH_ELEMENT);

// create dcHash_unmarshallNode
dcTaffy_createUnmarshallNodeFunctionMacro(dcHash, CAST_HASH);

// create dcHash_marshallNode
dcTaffy_createMarshallNodeFunctionMacro(dcHash, CAST_HASH);

////////////////////
// dcHashIterator //
////////////////////

void dcHashIterator_reset(dcHashIterator *_iterator)
{
    if (_iterator->hash != NULL)
    {
        _iterator->bucketIt = 0;
        _iterator->element = NULL;

        while (_iterator->bucketIt < _iterator->hash->capacity
               && _iterator->hash->buckets[_iterator->bucketIt] == NULL)
        {
            _iterator->bucketIt++;
        }

        if (_iterator->bucketIt < _iterator->hash->capacity)
        {
            _iterator->element =
                _iterator->hash->buckets[_iterator->bucketIt]->head;
        }
    }
}

dcHashIterator *dcHashIterator_create(const dcHash *_hash)
{
    dcHashIterator *iterator = (dcHashIterator *)(dcMemory_allocate
                                                  (sizeof(dcHashIterator)));
    iterator->hash = _hash;
    iterator->element = NULL;
    dcHashIterator_reset(iterator);
    return iterator;
}

void dcHashIterator_free(dcHashIterator **_iterator)
{
    dcMemory_free(*_iterator);
}

dcNode *dcHashIterator_getNext(dcHashIterator *_iterator)
{
    dcNode *result = NULL;

    if (_iterator->hash != NULL)
    {
        // try to iterate along the list first //
        if (_iterator->element != NULL)
        {
            result = _iterator->element->object;
            _iterator->element = _iterator->element->next;
        }
        else
        {
            // fail. find a non-null bucket //
            _iterator->bucketIt++;

            while (_iterator->bucketIt < _iterator->hash->capacity
                   && (_iterator->hash->buckets[_iterator->bucketIt] == NULL
                       || (_iterator->hash->buckets[_iterator->bucketIt]->size
                           == 0)))
            {
                _iterator->bucketIt++;
            }

            // did we really find a woot bucket? //
            if (_iterator->bucketIt < _iterator->hash->capacity)
            {
                // grab the list //
                _iterator->element =
                    _iterator->hash->buckets[_iterator->bucketIt]->head;

                if (_iterator->element != NULL)
                {
                    result = _iterator->element->object;
                    _iterator->element = _iterator->element->next;
                }
            }
            // else fail and do nothing, result is already NULL
        }
        // else success
    }

    return result;
}

dcNode *dcHashIterator_getNextValue(dcHashIterator *_iterator)
{
    dcNode *hashElement = dcHashIterator_getNext(_iterator);
    return (hashElement != NULL
            ? CAST_HASH_ELEMENT(hashElement)->value
            : NULL);
}

dcHash *dcHash_unmarshall(dcString *_stream)
{
    dcHash *result = NULL;
    uint32_t hashSize;
    uint8_t type;

    if (dcMarshaller_unmarshallNoNull(_stream, "uw", &type, &hashSize)
        // do a basic size check
        && dcString_hasLengthLeft(_stream, ((uint64_t)hashSize
                                            * MIN_MARSHALL_SIZE))
        && type == NODE_HASH)
    {
        uint32_t i = 0;
        result = dcHash_create();
        bool success = true;

        for (i = 0; i < hashSize; i++)
        {
            uint8_t elementType;

            if (dcMarshaller_unmarshallNoNull(_stream, "u", &elementType))
            {
                dcNode *key;
                dcNode *value;
                char *stringKey;

                if (elementType == HASH_ELEMENT_IS_NODE_KEY
                    && dcMarshaller_unmarshallNoNull(_stream,
                                                     "nn",
                                                     &key,
                                                     &value))
                {
                    if (dcHash_setValue(result, key, value)
                        == TAFFY_EXCEPTION)
                    {
                        dcNode_free(&key, DC_DEEP);
                        dcNode_free(&value, DC_DEEP);
                        success = false;
                        break;
                    }
                }
                else if (elementType == HASH_ELEMENT_IS_STRING_KEY
                         && dcMarshaller_unmarshallNoNull(_stream,
                                                          "sn",
                                                          &stringKey,
                                                          &value))
                {
                    if (dcHash_setValueWithStringKey(result,
                                                     stringKey,
                                                     value)
                        != TAFFY_SUCCESS)
                    {
                        success = false;
                        dcNode_free(&value, DC_DEEP);
                    }

                    dcMemory_free(stringKey);

                    if (! success)
                    {
                        break;
                    }
                }
                else
                {
                    success = false;
                    break;
                }
            }
        }

        if (! success)
        {
            dcHash_free(&result, DC_DEEP);
            result = NULL;
        }
    } // else FAILURE //

    return result;
}

dcString *dcHash_marshall(const dcHash *_hash, dcString *_stream)
{
    dcString *result = dcMarshaller_marshall
        (_stream, "uw", NODE_HASH, _hash->size);
    dcHashIterator *iterator = dcHashIterator_create(_hash);
    dcNode *that = NULL;

    while ((that = dcHashIterator_getNext(iterator))
           != NULL)
    {
        // add the key hash!
        dcHashElement *element = CAST_HASH_ELEMENT(that);

        if (element->key.isNodeKey)
        {
            dcMarshaller_marshall(result,
                                  "unn",
                                  HASH_ELEMENT_IS_NODE_KEY,
                                  element->key.keyUnion.nodeKey,
                                  element->value);
        }
        else
        {
            dcMarshaller_marshall(result,
                                  "usn",
                                  HASH_ELEMENT_IS_STRING_KEY,
                                  element->key.keyUnion.stringKey,
                                  element->value);
        }
    }

    dcHashIterator_free(&iterator);
    return result;
}

void dcHash_eachValue(dcHash *_hash,
                      dcContainerEachFunction _function,
                      void *_token)
{
    dcHashIterator *iterator = dcHashIterator_create(_hash);
    dcNode *that;

    while ((that = dcHashIterator_getNextValue(iterator))
           != NULL)
    {
        _function(that, (dcNode *)_token);
    }

    dcHashIterator_free(&iterator);
}

dcContainerSizeType dcHash_getSize(const struct dcNode_t *_hash)
{
    return CAST_HASH(_hash)->size;
}

void dcHash_merge(dcHash *_to, dcHash *_from)
{
    dcHashIterator *it = dcHash_createIterator(_from);
    dcNode *element;
    dcList *toRemove = dcList_create();

    while ((element = dcHashIterator_getNext(it))
           != NULL)
    {
        dcHashElement *hashElement = CAST_HASH_ELEMENT(element);

        // only merge if it doesn't already exist in _to
        if (dcHash_getValueWithStringKey(_to,
                                         hashElement->key.keyUnion.stringKey,
                                         NULL)
            == TAFFY_FAILURE)
        {
            dcHash_setValueWithStringKey(_to,
                                         hashElement->key.keyUnion.stringKey,
                                         hashElement->value);
        }
        else
        {
            dcList_push(toRemove,
                        dcString_createNodeWithString
                        (hashElement->key.keyUnion.stringKey, true));
        }
    }

    FOR_EACH_IN_LIST(toRemove, that)
    {
        dcHash_removeValueWithStringKey(_from,
                                        CAST_STRING(that->object)->string,
                                        NULL,
                                        DC_DEEP);
    }

    dcList_free(&toRemove, DC_DEEP);
    dcHashIterator_free(&it);
}
