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

#include "CompiledHash.h"
#include "dcCFunctionArgument.h"
#include "dcClassTemplate.h"
#include "dcContainerClass.h"
#include "dcHashClass.h"
#include "dcArray.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHash.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcPair.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

#include "dcClass.h"
#include "dcArrayClass.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNumberClass.h"
#include "dcPairClass.h"
#include "dcProcedureClass.h"
#include "dcStringClass.h"
#include "dcYesClass.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcHashClass_asString,
        gCFunctionArgument_none
    },
    {
        "each:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_CONST),
        &dcHashClass_each,
        gCFunctionArgument_block
    },
    {
        "eachKey:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_CONST),
        &dcHashClass_eachKey,
        gCFunctionArgument_block
    },
    {
        "eachValue:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_CONST),
        &dcHashClass_eachValue,
        gCFunctionArgument_block
    },
    {
        "#operator([]):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcHashClass_object,
        gCFunctionArgument_wild
    },
    {
        "objectForKey:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcHashClass_object,
        gCFunctionArgument_wild
    },
    {
        "remove:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcHashClass_remove,
        gCFunctionArgument_wild
    },
    {
        "#operator([]=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcHashClass_setKeyForObject,
        gCFunctionArgument_array
    },
    {
        "setValue:forKey:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcHashClass_setValueForKey,
        gCFunctionArgument_wildWild
    },
    {
        "size",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcHashClass_size,
        gCFunctionArgument_none
    },
    {
        0
    }
};

#define HASH_CLASS_MARSHALL_LENGTH 2
#define CAST_HASH_AUX(_node_) ((dcHashClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcHashClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (CONTAINER_PACKAGE_NAME,             // package name
          HASH_CLASS_NAME,                    // class name
          CONTAINER_CLASS_NAME,               // super type
          CLASS_HAS_READ_WRITE_LOCK,          // class flags
          NO_FLAGS,                           // scope data flags
          NULL,                               // meta methods
          sMethodWrappers,                    // methods
          &dcHashClass_initialize,            // initialization function
          NULL,                               // deinitialization function
          &dcHashClass_allocateNode,          // allocate
          &dcHashClass_deallocateNode,        // deallocate
          NULL,                               // meta mark
          &dcHashClass_markNode,              // mark
          &dcHashClass_copyNode,              // copy
          &dcHashClass_freeNode,              // free
          &dcHashClass_registerNode,          // register
          &dcHashClass_marshallNode,          // marshall
          &dcHashClass_unmarshallNode,        // unmarshall
          NULL));                             // set template
};

static dcHashClassAux *createAux(bool _initialized)
{
    dcHashClassAux *aux = (dcHashClassAux *)(dcMemory_allocate
                                             (sizeof(dcHashClassAux)));
    aux->initialized = _initialized;
    aux->types.hash = NULL;

    if (! _initialized)
    {
        aux->types.uninitializedData =
            (dcHashClassUninitializedData *)
            dcMemory_allocateAndInitialize
            (sizeof(dcHashClassUninitializedData));
    }

    return aux;
}

static void setHash(dcHashClassAux *_aux, dcHash *_hash)
{
    _aux->types.hash = _hash;
}

static void setTempKeys(dcHashClassAux *_aux, dcList *_keys)
{
    _aux->types.uninitializedData->keys = _keys;
}

static void setTempValues(dcHashClassAux *_aux, dcList *_values)
{
    _aux->types.uninitializedData->values = _values;
}

void dcHashClass_allocateNode(dcNode *_node)
{
    dcHashClassAux *aux = createAux(true);
    setHash(aux, dcHash_create());
    CAST_CLASS_AUX(_node) = aux;
}

void dcHashClass_deallocateNode(dcNode *_node)
{
    if (dcHashClass_isInitialized(_node))
    {
        dcHash_clear(dcHashClass_getHash(_node), DC_SHALLOW);
    }
}

dcNode *dcHashClass_createInitializedNode(dcHash *_hash, bool _object)
{
    dcHashClassAux *aux = createAux(true);
    setHash(aux,
            (_hash != NULL
             ? _hash
             : dcHash_create()));
    return (dcClass_createNode
            (sTemplate,
             dcContainerClass_createNode(_object),
             NULL, // scope
             _object,
             aux));
}

dcNode *dcHashClass_createUninitializedNode(dcList *_keys,
                                            dcList *_values,
                                            bool _object)
{
    dcHashClassAux *aux = createAux(false);
    setTempKeys(aux, _keys);
    setTempValues(aux, _values);
    return (dcClass_createNode
            (sTemplate,
             dcContainerClass_createNode(_object),
             NULL, // scope
             _object,
             aux));
}

dcNode *dcHashClass_createUninitializedObject(dcList *_keys,
                                              dcList *_values)
{
    return dcHashClass_createUninitializedNode(_keys, _values, true);
}

dcNode *dcHashClass_createInitializedObject(dcHash *_hash)
{
    return dcHashClass_createInitializedNode(_hash, true);
}

dcNode *dcHashClass_createObjectFromInitializedLists
    (dcList *_keys,
     dcList *_values,
     dcNode *_exception,
     dcNodeEvaluator *_evaluator)
{
    dcListElement *keyElement = NULL;
    dcListElement *valueElement = NULL;
    dcNode *result = dcNode_register
        (dcHashClass_createInitializedObject(dcHash_create()));
    uint32_t markCount = dcNodeEvaluator_pushMark(_evaluator, result);

    for (keyElement = _keys->head, valueElement = _values->head;
         keyElement != NULL;
         keyElement = keyElement->next, valueElement = valueElement->next)
    {
        if (dcHashClass_setHashValue(result,
                                     keyElement->object,
                                     valueElement->object)
            == NULL)
        {
            // exception occurred
            dcNode_register(result);
            result = NULL;
            break;
        }
    }

    dcNodeEvaluator_popMarks(_evaluator, markCount);
    return result;
}

#define HASH_TAFFY_FILE_NAME "src/class/Hash.ty"

void dcHashClass_initialize(void)
{
    dcError_assert(dcStringEvaluator_evalString(__compiledHash,
                                                HASH_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

void dcHashClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcHashClassAux *aux = CAST_HASH_AUX(_node);

    if (aux != NULL)
    {
        if (aux->initialized)
        {
            dcHash_free(&aux->types.hash, DC_DEEP);
        }
        else
        {
            dcList_free(&(aux->types.uninitializedData->keys), DC_DEEP);
            dcList_free(&(aux->types.uninitializedData->values), DC_DEEP);
            dcMemory_free(aux->types.uninitializedData);
        }

        dcMemory_free(aux);
    }
}

bool dcHashClass_isInitialized(const dcNode *_hashNode)
{
    return CAST_HASH_AUX(_hashNode)->initialized;
}

void dcHashClass_markNode(dcNode *_node)
{
    if (CAST_HASH_AUX(_node)->initialized)
    {
        dcHash_mark(dcHashClass_getHash(_node));
    }
}

void dcHashClass_registerNode(dcNode *_node)
{
    dcHash_register(dcHashClass_getHash(_node));
}

void dcHashClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcHashClassAux *fromAux = CAST_HASH_AUX(_from);
    dcHashClassAux *toAux = createAux(fromAux->initialized);

    if (fromAux->initialized)
    {
        toAux->types.hash = dcHash_copy(fromAux->types.hash, DC_SHALLOW);
    }
    else
    {
        if (fromAux->types.uninitializedData->keys
            && fromAux->types.uninitializedData->values)
        {
            toAux->types.uninitializedData->keys =
                dcList_copy(fromAux->types.uninitializedData->keys,
                            _depth);
            toAux->types.uninitializedData->values =
                dcList_copy(fromAux->types.uninitializedData->values,
                            _depth);
        }
        else
        {
            toAux->types.uninitializedData->keys =
                dcList_create();
            toAux->types.uninitializedData->values =
                dcList_create();
        }
    }

    CAST_CLASS_AUX(_to) = toAux;
}

void dcHashClass_initializeObject(dcNode *_hash, dcList *_keys, dcList *_values)
{
    dcHashClassAux *hashAux = CAST_HASH_AUX(_hash);
    dcListElement *keyElement = _keys->head;
    dcListElement *valueElement = _values->head;
    dcHash *hash = dcHash_create();

    while (keyElement != NULL)
    {
        if (dcHashClass_setHashValue(_hash,
                                     keyElement->object,
                                     valueElement->object)
            == NULL)
        {
            break;
        }

        keyElement = keyElement->next;
        valueElement = valueElement->next;
    }

    CAST_HASH_AUX(_hash)->initialized = true;
    hashAux->types.hash = hash;
}

dcList *dcHashClass_getTempKeys(const dcNode *_node)
{
    return CAST_HASH_AUX(_node)->types.uninitializedData->keys;
}

dcList *dcHashClass_getTempValues(const dcNode *_node)
{
    return CAST_HASH_AUX(_node)->types.uninitializedData->values;
}

dcHash *dcHashClass_getHash(const dcNode *_node)
{
    return CAST_HASH_AUX(_node)->types.hash;
}

dcContainerSizeType dcHashClass_getSize(const dcNode *_node)
{
    return CAST_HASH_AUX(_node)->types.hash->size;
}

dcResult dcHashClass_hashifyNode(dcNode *_node, dcHashType *_hashValue)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *hashNode = dcNodeEvaluator_callMethod(evaluator, _node, "hash");
    dcResult result = TAFFY_EXCEPTION;
    uint32_t markCount = dcNodeEvaluator_pushMark(evaluator, hashNode);

    if (hashNode != NULL)
    {
        if (dcNumberClass_isMe(hashNode)
            && (dcNumber_hash(dcNumberClass_getNumber(hashNode), _hashValue)
                == TAFFY_SUCCESS))
        {
            result = TAFFY_SUCCESS;
        }
        else
        {
            dcInvalidHashValueExceptionClass_throwObject(hashNode);
        }
    }

    dcNodeEvaluator_popMarks(evaluator, markCount);
    return result;
}

// methods //
////////////////////////////////////////////////////////////////////////////
//
// Hash#"[]"
//
// Gets an object from a key
//
// h1 = ()
// h1[1] = "one"
// h1[1]
// ==> "one"
//
////////////////////////////////////////////////////////////////////////////
dcNode *dcHashClass_object(dcNode *_receiver, dcArray *_arguments)
{
    // if an exception occurred, gotten == NULL
    dcNode *gotten;
    return ((dcHash_getValue(dcHashClass_getHash(_receiver),
                             dcArray_get(_arguments, 0),
                             &gotten)
             == TAFFY_FAILURE)
            ? dcNilClass_getInstance()
            : gotten);
}

////////////////////////////////////////////////////////////////////////////
//
// Hash#"[]="
//
// Sets an object for a key
//
// h1 = ()
// h1[1] = "one"
// ==> [<1,"one">]
//
////////////////////////////////////////////////////////////////////////////
dcNode *dcHashClass_setKeyForObject(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *realArguments =
        dcArrayClass_getObjects(dcArray_get(_arguments, 0));
    dcNode *result = dcHashClass_setHashValue(_receiver,
                                              dcArray_get(realArguments, 0),
                                              dcArray_get(realArguments, 1));
    dcContainerClass_setModified(_receiver, true);
    return result;
}

//
// The link between Hash Class land and Hash land
//
dcNode *dcHashClass_setHashValue(dcNode *_receiver,
                                 dcNode *_key,
                                 dcNode *_value)
{
    dcNode *key = dcClass_copyIfTemplateOrAtomic(_key);
    dcNode *value = dcClass_copyIfTemplateOrAtomic(_value);
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNodeEvaluator_pushMark(evaluator, key);
    dcNodeEvaluator_pushMark(evaluator, value);
    dcResult result = dcHash_setValue(dcHashClass_getHash(_receiver),
                                      key,
                                      value);
    dcNodeEvaluator_popMarks(evaluator, 2);
    dcError_assert(result != TAFFY_FAILURE);
    return (result == TAFFY_SUCCESS
            ? _receiver
            : NULL);
}

dcNode *dcHashClass_setValueForKey(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcHashClass_setHashValue(_receiver,
                                              dcArray_get(_arguments, 1),
                                              dcArray_get(_arguments, 0));
    dcContainerClass_setModified(_receiver, true);
    return result;
}

////////////////////////////////////////////////////////////////////////////
//
// Hash#"remove:"
//
// Removes an object from self using the provided key
//
// h1 = (1 => "one", 2 => "two")
// h1 remove: 1
// ==> (2 => "two")
//
////////////////////////////////////////////////////////////////////////////
dcNode *dcHashClass_remove(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcResult removeResult = dcHash_removeValue(dcHashClass_getHash(_receiver),
                                               dcArray_get(_arguments, 0),
                                               NULL,
                                               DC_SHALLOW);
    if (removeResult == TAFFY_SUCCESS)
    {
        result = dcYesClass_getInstance();
        dcContainerClass_setModified(_receiver, true);
    }
    else if (removeResult == TAFFY_FAILURE)
    {
        result = dcNoClass_getInstance();
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////
//
// Hash#"count"
//
// Returns the count of elements in the hash
//
// h = (1=>"one", 2=>"two")
// h count
// ==> 2
//
////////////////////////////////////////////////////////////////////////////
dcNode *dcHashClass_size(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32u
             (dcHashClass_getHash(_receiver)->size)));
}

enum EachKind_e
{
    EACH_KEY = 1,
    EACH_VALUE,
    EACH_PAIR
};

typedef enum EachKind_e EachKind;

static dcNode *eachHelper(dcNode *_receiver,
                          dcArray *_arguments,
                          EachKind _eachKind)
{
    // for exceptions //
    bool exception = false;

    // extract the block, procedure, and methodHeader out of _arguments[0] //
    dcNode *blockNode = dcArray_get(_arguments, 0);
    dcNode *procedureNode =
        dcClass_castNodeWithAssert(blockNode,
                                   dcProcedureClass_getTemplate(),
                                   false,
                                   true);
    dcNode *result = NULL;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    size_t argumentsSize = dcMethodHeader_getArgumentsSize
        (dcProcedureClass_getMethodHeader(procedureNode));

    if (argumentsSize == 1)
    {
        dcHashIterator *hashIt =
            dcHash_createIterator(dcHashClass_getHash(_receiver));
        dcNode *that;
        // used for evaluating the procedure //
        dcList *arguments = dcList_create();
        int marks = 0;

        dcContainerClass_startLoop(_receiver);
        dcNodeEvaluator_startLoop(evaluator);

        while ((that = dcHashIterator_getNext(hashIt))
               != NULL)
        {
            if (dcContainerClass_checkModified(_receiver))
            {
                exception = true;
                break;
            }

            dcHashElement *element = CAST_HASH_ELEMENT(that);
            dcList_clear(arguments, DC_SHALLOW);
            dcError_assert(element->key.isNodeKey);

            if (_eachKind == EACH_KEY)
            {
                dcList_unshift(arguments, element->key.keyUnion.nodeKey);
            }
            else if (_eachKind == EACH_VALUE)
            {
                dcList_unshift(arguments, element->value);
            }
            else if (_eachKind == EACH_PAIR)
            {
                dcNode *pair = dcNode_register(dcPairClass_createObject
                                               (element->key.keyUnion.nodeKey,
                                                element->value,
                                                true));
                marks += dcNodeEvaluator_pushMark(evaluator, pair);
                dcList_unshift(arguments, pair);
            }

            // evaluate the procedure //
            if (dcNodeEvaluator_evaluateProcedure(evaluator,
                                                  _receiver,
                                                  procedureNode,
                                                  (SCOPE_DATA_BREAKTHROUGH
                                                   | SCOPE_DATA_CONST),
                                                  arguments)
                == NULL)
            {
                exception = true;
                break;
            }
            else if (!dcNodeEvaluator_canContinueEvaluating(evaluator))
            {
                break;
            }
        }

        dcNodeEvaluator_popMarks(evaluator, marks);
        dcNodeEvaluator_stopLoop(evaluator);
        dcContainerClass_stopLoop(_receiver);
        dcHashIterator_free(&hashIt);

        // free up the iterator and arguments array //
        dcList_free(&arguments, DC_SHALLOW);
    }
    else
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject(1, argumentsSize);
    }

    if (!exception)
    {
        result = dcNilClass_getInstance();
    }

    return result;
}

dcNode *dcHashClass_eachKey(dcNode *_receiver, dcArray *_arguments)
{
    return eachHelper(_receiver, _arguments, EACH_KEY);
}

dcNode *dcHashClass_eachValue(dcNode *_receiver, dcArray *_arguments)
{
    return eachHelper(_receiver, _arguments, EACH_VALUE);
}

dcNode *dcHashClass_each(dcNode *_receiver, dcArray *_arguments)
{
    return eachHelper(_receiver, _arguments, EACH_PAIR);
}

dcNode *dcHashClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    if (dcHashClass_isInitialized(_receiver))
    {
        // if we're initialized, printing is easier in Taffy
        // asStringInitialized is located in Hash.ty
        return dcNodeEvaluator_callMethod(dcSystem_getCurrentNodeEvaluator(),
                                          _receiver,
                                          "asStringInitialized");
    }

    dcListElement *key;
    dcListElement *value;
    dcHashClassAux *aux = CAST_HASH_AUX(_receiver);
    dcString *resultString = dcString_create();
    dcNode *result = NULL;
    bool exception = false;

    assert(aux->types.uninitializedData->keys->size
           == aux->types.uninitializedData->values->size);

    dcString_appendString(resultString, "#Hash(");

    for (key = aux->types.uninitializedData->keys->head,
             value = aux->types.uninitializedData->values->head;
         key != NULL;
         key = key->next, value = value->next)
    {
        if (dcNode_print(key->object, resultString) == TAFFY_EXCEPTION)
        {
            exception = true;
            break;
        }

        dcString_appendString(resultString, "=>");

        if (dcNode_print(value->object, resultString) == TAFFY_EXCEPTION)
        {
            exception = true;
            break;
        }

        if (key->next != NULL)
        {
            dcString_appendString(resultString, ", ");
        }
    }

    dcString_appendString(resultString, ")");

    if (exception)
    {
        dcString_free(&resultString, DC_DEEP);
    }
    else
    {
        result = (dcNode_register
                  (dcStringClass_createObject(resultString->string, false)));
        dcString_free(&resultString, DC_SHALLOW);
    }

    return result;
}

bool dcHashClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;

    if (dcString_getLengthLeft(_stream) >= HASH_CLASS_MARSHALL_LENGTH)
    {
        bool isInitialized = (dcString_getCharacter(_stream) == 1);

        if (isInitialized)
        {
            dcHash *objects = dcHash_unmarshall(_stream);

            if (objects != NULL)
            {
                dcHashClassAux *aux = createAux(true);
                setHash(aux, objects);
                CAST_CLASS_AUX(_node) = aux;
                result = true;
            }
            // else FAILURE
        }
        else
        {
            dcList *keys = NULL;
            dcList *values = NULL;

            if (dcMarshaller_unmarshallNoNull(_stream, "ll", &keys, &values))
            {
                dcHashClassAux *aux = createAux(false);
                setTempKeys(aux, keys);
                setTempValues(aux, values);
                CAST_CLASS_AUX(_node) = aux;
                result = true;
            }
        }
    }

    return result;
}

dcString *dcHashClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    bool isInitialized = dcHashClass_isInitialized(_node);
    dcString_appendCharacter(_stream, isInitialized);

    if (isInitialized)
    {
        dcHash_marshall(dcHashClass_getHash(_node), _stream);
    }
    else
    {
        dcMarshaller_marshall(_stream,
                              "ll",
                              dcHashClass_getTempKeys(_node),
                              dcHashClass_getTempValues(_node));
    }

    return _stream;
}
