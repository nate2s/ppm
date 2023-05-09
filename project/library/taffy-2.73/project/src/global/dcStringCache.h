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

#ifndef __DC_STRING_CACHE_H__
#define __DC_STRING_CACHE_H__

#include "dcDefines.h"

struct dcStringCacheElement_t
{
    struct dcString_t *keyString;
    dcHashType keyHashValue;
    struct dcNode_t *value;
};

typedef struct dcStringCacheElement_t dcStringCacheElement;

void dcStringCacheElement_free(dcStringCacheElement *_element);

struct dcStringCache_t
{
    struct dcHash_t *cache;

    // last-to-first ordering list
    struct dcList_t *cacheList;

    // for locking success
    struct dcReadWriteLock_t *lock;

    // how many values can this cache contain?
    // 0 means infinite
    uint32_t limit;

    // are the values mine?
    bool ownsValues;
};

typedef struct dcStringCache_t dcStringCache;

#define DEFAULT_STRING_CACHE_LIMIT 50
#define STRING_CACHE_NO_LIMIT 0

dcStringCache *dcStringCache_create(bool _ownsValues);
dcStringCache *dcStringCache_createWithLimit(uint32_t _limit, bool _ownsValues);

void dcStringCache_free(dcStringCache **_cache);

// This comes first
dcResult dcStringCache_get(dcStringCache *_cache,
                           struct dcNode_t *_key,           // in
                           dcStringCacheElement *_element); // out

// This comes first
dcResult dcStringCache_getWithSymbol(dcStringCache *_cache,
                                     struct dcNode_t *_key,           // in
                                     const char *_symbol,             // in
                                     dcStringCacheElement *_element); // out

// This comes first
dcResult dcStringCache_getVoidOrNot(dcStringCache *_cache, // in
                                    struct dcNode_t *_key,          // in ;)
                                    dcStringCacheElement *_element, // out
                                    bool *_modified);      // in

// This comes last
dcNode *dcStringCache_add(dcStringCache *_cache,
                          dcStringCacheElement *_element, // in
                          struct dcNode_t *_value);       // in

void dcStringCache_addHelper(dcStringCache *_cache,
                             dcStringCacheElement *_element,
                             struct dcNode_t *_value);

// optional
bool dcStringCache_pop(dcStringCache *_cache);

#endif
