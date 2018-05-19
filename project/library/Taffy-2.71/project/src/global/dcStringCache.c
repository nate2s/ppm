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

#include "dcHash.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcPair.h"
#include "dcReadWriteLock.h"
#include "dcString.h"
#include "dcStringCache.h"
#include "dcUnsignedInt64.h"
#include "dcVoid.h"

void dcStringCacheElement_free(dcStringCacheElement *_element)
{
    dcString_free(&_element->keyString, DC_DEEP);
}

dcStringCache *dcStringCache_create(bool _ownsValues)
{
    return dcStringCache_createWithLimit(DEFAULT_STRING_CACHE_LIMIT,
                                         _ownsValues);
}

dcStringCache *dcStringCache_createWithLimit(uint32_t _limit,
                                             bool _ownsValues)
{
    dcStringCache *cache =
        (dcStringCache *)dcMemory_allocate(sizeof(dcStringCache));
    cache->cache = dcHash_create();
    cache->cacheList = (_limit == STRING_CACHE_NO_LIMIT
                        ? NULL
                        : dcList_create());
    cache->lock = dcReadWriteLock_create();
    cache->limit = _limit;
    cache->ownsValues = _ownsValues;
    return cache;
}

void dcStringCache_free(dcStringCache **_cache)
{
    dcStringCache *cache = *_cache;
    dcHash_free(&cache->cache,
                (cache->ownsValues
                 ? DC_DEEP
                 : DC_SHALLOW));
    dcList_free(&cache->cacheList, DC_DEEP);
    dcReadWriteLock_free(&cache->lock);
    dcMemory_free(cache);
}

dcResult dcStringCache_get(dcStringCache *_cache,
                           dcNode *_key,                   // in
                           dcStringCacheElement *_element) // out
{
    return dcStringCache_getWithSymbol(_cache,
                                       _key,
                                       NULL,
                                       _element);
}

dcResult dcStringCache_getVoidOrNot(dcStringCache *_cache, // in
                                    dcNode *_key,          // in ;)
                                    dcStringCacheElement *_element, // out
                                    bool *_modified)       // in
{
    dcResult result = dcStringCache_get(_cache, _key, _element);

    if (result == TAFFY_SUCCESS)
    {
        // if it's void then it wasn't modified
        if (_element->value->type == NODE_VOID)
        {
            _element->value = _key;
        }
        else
        {
            if (_modified != NULL)
            {
                *_modified = true;
            }

            _element->value = dcNode_copy(_element->value, DC_DEEP);

            if (_cache->ownsValues)
            {
                dcNode_free(&_key, DC_DEEP);
            }
        }
    }

    return result;
}

dcResult dcStringCache_getWithSymbol(dcStringCache *_cache, // in
                                     dcNode *_key,          // in
                                     const char *_symbol,   // in
                                     dcStringCacheElement *_element) // out
{
    dcResult result = TAFFY_FAILURE;

    if (_cache->lock != NULL)
    {
        dcReadWriteLock_lockForRead(_cache->lock);
    }

    if (_element->keyHashValue == 0)
    {
        if (_element->keyString == NULL)
        {
            dcString *display = dcString_create();
            result = dcNode_print(_key, display);

            if (_symbol != NULL)
            {
                dcString_append(display, "!!%s", _symbol);
            }

            if (result == TAFFY_SUCCESS)
            {
                _element->keyString = display;
            }
            else
            {
                dcString_free(&display, DC_DEEP);
            }
        }
        else
        {
            result = TAFFY_SUCCESS;
        }

        if (result == TAFFY_SUCCESS)
        {
            if (_element->keyHashValue == 0)
            {
                dcString_hashCharArray(_element->keyString->string,
                                       &_element->keyHashValue);
            }

            result = dcHash_getValueWithStringKeys(_cache->cache,
                                                   _element->keyString->string,
                                                   _element->keyHashValue,
                                                   &_element->value);
        }
    }
    else
    {
        result = dcHash_getValueWithStringKeys(_cache->cache,
                                               _element->keyString->string,
                                               _element->keyHashValue,
                                               &_element->value);
    }

    if (_cache->lock != NULL)
    {
        dcReadWriteLock_unlock(_cache->lock);
    }

    return result;
}

static void removeHashValue(dcStringCache *_cache, dcNode *_node)
{
    dcPair *pair = CAST_PAIR(_node);
    assert(dcHash_removeValueWithStringKeys
           (_cache->cache,
            CAST_STRING(pair->right)->string,
            CAST_INT_64(pair->left),
            NULL,
            (_cache->ownsValues
             ? DC_DEEP
             : DC_SHALLOW))
           != TAFFY_EXCEPTION);
}

void dcStringCache_addHelper(dcStringCache *_cache,
                             dcStringCacheElement *_element,
                             dcNode *_value)
{
    if (_value == NULL)
    {
        dcStringCache_add(_cache, _element, dcVoid_createNode(NULL));
    }
    else
    {
        dcStringCache_add(_cache,
                          _element,
                          (_cache->ownsValues
                           ? dcNode_copy(_value, DC_DEEP)
                           : _value));
    }
}

dcNode *dcStringCache_add(dcStringCache *_cache,
                          dcStringCacheElement *_element, // in
                          dcNode *_value)
{
    if (_cache->lock != NULL)
    {
        dcReadWriteLock_lockForWrite(_cache->lock);
    }

    _element->value = _value;

    if (_value == NULL)
    {
        _value = dcVoid_createNode(NULL);
    }
    else if (_cache->ownsValues)
    {
        _value = dcNode_copy(_value, DC_DEEP);
    }

    dcHash_removeValueWithStringKeys(_cache->cache,
                                     _element->keyString->string,
                                     _element->keyHashValue,
                                     NULL,
                                     (_cache->ownsValues
                                      ? DC_DEEP
                                      : DC_SHALLOW));
    assert(dcHash_setValueWithStringKeys(_cache->cache,
                                         _element->keyString->string,
                                         _element->keyHashValue,
                                         _value)
           == TAFFY_SUCCESS);

    if (_cache->limit != 0)
    {
        dcNode *element =
            dcPair_createNode
            (dcUnsignedInt64_createNode(_element->keyHashValue),
             dcString_createNodeWithString(_element->keyString->string, true));

        dcList_push(_cache->cacheList, element);

        if (_cache->cache->size > _cache->limit)
        {
            dcNode *first = dcList_shift(_cache->cacheList, DC_FLOATING);
            removeHashValue(_cache, first);
            dcNode_free(&first, DC_DEEP);
        }
    }

    if (_cache->lock != NULL)
    {
        dcReadWriteLock_unlock(_cache->lock);
    }

    return _value;
}

bool dcStringCache_pop(dcStringCache *_cache)
{
    bool result = false;

    if (_cache->cacheList->size > 0)
    {
        result = true;
        dcNode *tail = dcList_pop(_cache->cacheList, DC_FLOATING);
        removeHashValue(_cache, tail);
        dcNode_free(&tail, DC_DEEP);
    }

    return result;
}
