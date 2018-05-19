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

#include "dcGarbageCollector.h"
#include "dcHash.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"
#include "dcStringCache.h"
#include "dcTestUtilities.h"

static void testGetAndAnddNoOwn(void)
{
    dcStringCache *cache = dcStringCache_create(false);
    dcStringCacheElement element = {0};

    assert(dcStringCache_get(cache,
                             gTestNodes[0],
                             &element)
           == TAFFY_FAILURE);

    assert(element.keyString != NULL);
    assert(element.value == NULL);
    dcStringCache_add(cache, &element, gTestNodes[0]);

    uint8_t i;

    // try a few times, for fun
    for (i = 0; i < 10; i++)
    {
        assert(dcStringCache_get(cache,
                                 gTestNodes[0],
                                 &element)
               == TAFFY_SUCCESS);
        assert(element.value == gTestNodes[0]);
    }

    dcStringCacheElement_free(&element);
    dcStringCache_free(&cache);
}

static void testGetAndOwn(void)
{
    dcStringCache *cache = dcStringCache_create(true);
    dcStringCacheElement element = {0};

    assert(dcStringCache_get(cache,
                             gTestNodes[0],
                             &element)
           == TAFFY_FAILURE);

    assert(element.keyString != NULL);
    assert(element.value == NULL);
    dcStringCache_add(cache, &element, gTestNodes[0]);
    uint8_t i;

    // try a few times, for fun
    for (i = 0; i < 10; i++)
    {
        assert(dcStringCache_get(cache,
                                 gTestNodes[0],
                                 &element)
               == TAFFY_SUCCESS);
        assert(dcNode_easyCompare(element.value, gTestNodes[0]));
    }

    dcStringCacheElement_free(&element);
    dcStringCache_free(&cache);
}

static void assertHas(dcStringCache *_cache, dcNode *_value, bool _yesno)
{
    dcString *display = dcString_create();
    assert(dcNode_print(_value, display) == TAFFY_SUCCESS);
    assert(dcHash_getValueWithStringKey(_cache->cache,
                                        display->string,
                                        NULL)
           == (_yesno
               ? TAFFY_SUCCESS
               : TAFFY_FAILURE));
    dcString_free(&display, DC_DEEP);
}

static void testPastLimit(bool _ownsValues)
{
    dcStringCache *cache = dcStringCache_createWithLimit(3, _ownsValues);

    uint8_t i;

    for (i = 0; i < 5; i++)
    {
        dcStringCacheElement element = {0};
        assert(dcStringCache_get(cache,
                                 gTestNodes[i],
                                 &element)
               == TAFFY_FAILURE);

        assert(element.keyString != NULL);
        assert(element.value == NULL);

        dcStringCache_add(cache, &element, gTestNodes[i]);

        assert(dcStringCache_get(cache,
                                 gTestNodes[i],
                                 &element)
               == TAFFY_SUCCESS);

        if (_ownsValues)
        {
            assert(dcNode_easyCompare(element.value, gTestNodes[i]));
        }
        else
        {
            assert(element.value == gTestNodes[i]);
        }

        dcStringCacheElement_free(&element);
    }

    assert(cache->cache->size == 3);
    assertHas(cache, gTestNodes[0], false);
    assertHas(cache, gTestNodes[1], false);
    assertHas(cache, gTestNodes[2], true);
    assertHas(cache, gTestNodes[3], true);
    assertHas(cache, gTestNodes[4], true);
    dcStringCache_free(&cache);
}

static void testPastLimitNoOwn(void)
{
    testPastLimit(false);
}

static void testPastLimitOwn(void)
{
    testPastLimit(true);
}

static void testGetVoidOrNot(void)
{
    dcStringCache *cache = dcStringCache_create(false);
    dcStringCacheElement element = {0};

    assert(dcStringCache_get(cache,
                             gTestNodes[0],
                             &element)
           == TAFFY_FAILURE);

    // we didn't modify it
    dcNode *voidy = dcStringCache_add(cache, &element, NULL);
    assert(voidy->type == NODE_VOID);

    bool modified = false;
    assert(dcStringCache_getVoidOrNot(cache,
                                      gTestNodes[0],
                                      &element,
                                      &modified));
    assert(! modified);
    assert(element.value == gTestNodes[0]);

    // we modified it!
    dcStringCache_add(cache, &element, gTestNodes[1]);

    // reset modified
    modified = false;
    assert(dcStringCache_getVoidOrNot(cache,
                                      gTestNodes[0],
                                      &element,
                                      &modified));
    assert(modified);
    dcTestUtilities_expectEqual(&element.value, &gTestNodes[1]);

    dcNode_free(&element.value, DC_DEEP);
    dcNode_free(&voidy, DC_DEEP);
    dcStringCacheElement_free(&element);
    dcStringCache_free(&cache);
}

static const dcTestFunctionMap map[] =
{
    {"Get and Add No Own", &testGetAndAnddNoOwn},
    {"Get and Add Own",    &testGetAndOwn},
    {"Past Limit No Own",  &testPastLimitNoOwn},
    {"Past Limit Own",     &testPastLimitOwn},
    {"Get Void or Not",    &testGetVoidOrNot},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcTestUtilities_go("String Cache Test", _argc, _argv, NULL, map, false);
    dcGarbageCollector_free();
    dcMemory_deinitialize();
    return 0;
}
