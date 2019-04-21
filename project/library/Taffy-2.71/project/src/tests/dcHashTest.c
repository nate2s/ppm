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
#include <stdio.h>
#include <stdlib.h>

#include "dcContainers.h"
#include "dcError.h"
#include "dcGarbageCollector.h"
#include "dcHash.h"
#include "dcUnsignedInt32.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcScope.h"
#include "dcSystem.h"
#include "dcString.h"
#include "dcTestUtilities.h"
#include "dcThread.h"
#include "dcVoid.h"

#define testNode(x) nodes[x]

static void setValueWithStringKey(dcHash *_hash,
                                  const char *_key,
                                  dcNode *_value)
{
    dcTestUtilities_assert(dcHash_setValueWithStringKey(_hash, _key, _value)
                           == TAFFY_SUCCESS);
}

static void testLotsOfNodes(uint32_t _startingSize)
{
    uint32_t size = 50000;
    uint32_t i;
    dcHash *hash = dcHash_createWithCapacity(_startingSize);

    for (i = 0; i < size; i++)
    {
        dcHash_setValue(hash,
                        dcUnsignedInt32_createNode(i),
                        dcUnsignedInt32_createNode(i + 1));
    }

    for (i = 0; i < size; i++)
    {
        dcNode *value;
        dcNode *key = dcUnsignedInt32_createNode(i);
        dcError_assert((dcHash_getValue(hash, key, &value)
                        == TAFFY_SUCCESS)
                       && dcUnsignedInt32_getInt(value) == i + 1);
        dcNode_free(&key, DC_DEEP);
    }

    dcHash_free(&hash, DC_DEEP);
}

static void testLotsOfNodesStartingSmall(void)
{
    testLotsOfNodes(20);
}

static void testLotsOfNodesStartingBig(void)
{
    testLotsOfNodes(50000);
}

int main(int _argc, char **_argv)
{
    dcGarbageCollector_create();
    dcTestUtilities_start("Hash Test", _argc, _argv);

    // verify that a new hash has size 0
    {
        dcHash *hash = dcHash_create();
        dcTestUtilities_checkHashSize(hash, 0);
        dcHash_free(&hash, DC_DEEP);
    }

    // copy a blank hash
    {
        dcHash *hash = dcHash_create();
        dcHash *copy = dcHash_copy(hash, DC_DEEP);
        dcTestUtilities_checkHashSize(copy, 0);
        dcHash_free(&hash, DC_DEEP);
        dcHash_free(&copy , DC_DEEP);
    }

    // copy a hash with one element, node key
    {
        dcHash *hash = dcHash_create();
        dcHash_setValue(hash, gTestNodes[1], gTestNodes[2]);
        dcHash *copy = dcHash_copy(hash, DC_SHALLOW);
        dcTestUtilities_checkHashSize(copy, 1);
        dcHash_free(&hash, DC_DEEP);
        dcHash_free(&copy, DC_DEEP);
    }

    // copy a hash with one element, string key
    {
        dcHash *hash = dcHash_create();
        dcHash_setValueWithStringKey(hash, "maKey", gTestNodes[2]);
        dcHash *copy = dcHash_copy(hash, DC_SHALLOW);
        dcTestUtilities_checkHashSize(copy, 1);
        dcHash_free(&hash, DC_DEEP);
        dcHash_free(&copy, DC_DEEP);
    }


    // copy a hash with two elements, string and node key
    {
        dcHash *hash = dcHash_create();
        dcHash_setValue(hash, gTestNodes[1], gTestNodes[2]);
        dcHash_setValueWithStringKey(hash, "maKey", gTestNodes[2]);
        dcHash *copy = dcHash_copy(hash, DC_SHALLOW);
        dcTestUtilities_checkHashSize(copy, 2);
        dcHash_free(&hash, DC_DEEP);
        dcHash_free(&copy, DC_DEEP);
    }

    // insert an element with node key, check size
    {
        dcHash *hash = dcHash_create();
        dcHash_setValue(hash, gTestNodes[1], gTestNodes[2]);
        dcTestUtilities_checkHashSize(hash, 1);
        dcHash_free(&hash, DC_SHALLOW);
    }

    // insert an element with string key, check size
    {
        // a new dcNode(dcString) will be created for the string, so
        // DEEP free it after
        dcHash *hash = dcHash_create();
        setValueWithStringKey(hash,
                              "nodes(1)",
                              dcNode_copy(gTestNodes[2], DC_DEEP));
        dcTestUtilities_checkHashSize(hash, 1);
        dcHash_free(&hash, DC_DEEP);
    }

    // insert an element, get it!
    {
        dcHash *hash = dcHash_create();
        dcHash_setValue(hash, gTestNodes[1], gTestNodes[2]);
        dcTestUtilities_checkHashSize(hash, 1);

        dcNode *gotten = NULL;
        dcTestUtilities_assert(dcHash_getValue(hash, gTestNodes[1], &gotten)
                               == TAFFY_SUCCESS);
        dcTestUtilities_checkEqual(gotten, gTestNodes[2]);

        dcHash_free(&hash, DC_SHALLOW);
    }

    // overwrite an element with string key, check size
    {
        // a new dcNode(dcString) will be created for the string, so
        // DEEP free it after
        dcHash *hash = dcHash_create();
        setValueWithStringKey(hash, "gTestNodes[1]", gTestNodes[2]);
        setValueWithStringKey(hash,
                              "gTestNodes[1]",
                              dcNode_copy(gTestNodes[2], DC_DEEP));
        dcTestUtilities_checkHashSize(hash, 1);
        dcHash_free(&hash, DC_DEEP);
    }

    // get a value with a string key
    {
        dcHash *hash = dcHash_create();
        setValueWithStringKey(hash, "gTestNodes[1]", gTestNodes[2]);
        dcNode *gotten = NULL;
        dcTestUtilities_assert(dcHash_getValueWithStringKey
                               (hash,
                                "gTestNodes[1]",
                                &gotten)
                               == TAFFY_SUCCESS);
        dcTestUtilities_checkEqual(gotten, gTestNodes[2]);
        dcHash_free(&hash, DC_SHALLOW);
    }

    // get a value with a string key + hash value
    {
        dcHash *hash = dcHash_create();
        const char *key = "gTestNodes[1]";
        dcHashType hashValue;
        assert(dcString_hashCharArray(key, &hashValue) == TAFFY_SUCCESS);
        assert(dcHash_setValueWithStringKeys(hash,
                                             key,
                                             hashValue,
                                             gTestNodes[1])
               == TAFFY_SUCCESS);
        dcNode *gotten = NULL;
        assert(dcHash_getValueWithStringKey
               (hash,
                "gTestNodes[1]",
                &gotten)
               == TAFFY_SUCCESS);
        dcTestUtilities_checkEqual(gotten, gTestNodes[1]);
        dcHash_free(&hash, DC_SHALLOW);
    }

    // get a value with a string key + hash
    {
        dcHash *hash = dcHash_create();
        const char *key = "gTestNodes[1]";
        setValueWithStringKey(hash, key, gTestNodes[2]);
        dcHashType hashValue = 0;
        assert(dcString_hashCharArray(key, &hashValue) == TAFFY_SUCCESS);
        dcNode *gotten = NULL;
        dcTestUtilities_assert(dcHash_getValueWithStringKeys
                               (hash,
                                key,
                                hashValue,
                                &gotten)
                               == TAFFY_SUCCESS);
        dcTestUtilities_checkEqual(gotten, gTestNodes[2]);
        dcHash_free(&hash, DC_SHALLOW);
    }

    // get a value with an string-operator key
    {
        dcHash *hash = dcHash_create();
        const char *name = "operator(==):";

        setValueWithStringKey(hash, name, gTestNodes[2]);
        dcNode *gotten = NULL;
        dcTestUtilities_assert(dcHash_getValueWithStringKey
                               (hash,
                                name,
                                &gotten)
                               == TAFFY_SUCCESS);
        dcTestUtilities_checkEqual(gotten, gTestNodes[2]);
        dcHash_free(&hash, DC_SHALLOW);
    }

    // overwrite an element with node key a few times, check size
    {
        // a new dcNode(dcString) will be created for the string, so
        // DEEP free it after
        dcHash *hash = dcHash_create();

        // overwrite 1 => X pairs three times
        dcHash_setValue(hash, gTestNodes[1], gTestNodes[2]);
        dcHash_setValue(hash, gTestNodes[1], gTestNodes[3]);
        dcHash_setValue(hash, gTestNodes[1], gTestNodes[4]);
        dcTestUtilities_checkHashSize(hash, 1);

        // overwrite 2 => X pairs two times
        dcHash_setValue(hash, gTestNodes[2], gTestNodes[4]);
        dcTestUtilities_checkHashSize(hash, 2);
        dcHash_setValue(hash, gTestNodes[2], gTestNodes[4]);
        dcTestUtilities_checkHashSize(hash, 2);

        dcHash_free(&hash, DC_SHALLOW);
    }

    // overwrite an element with node key a few times and get
    {
        // a new dcNode(dcString) will be created for the string, so
        // DEEP free it after
        dcHash *hash = dcHash_create();
        dcHash_setValue(hash, gTestNodes[1], gTestNodes[2]);
        dcHash_setValue(hash, gTestNodes[1], gTestNodes[3]);
        dcHash_setValue(hash, gTestNodes[1], gTestNodes[4]);
        dcTestUtilities_checkHashSize(hash, 1);

        dcNode *gotten = NULL;
        dcTestUtilities_assert(dcHash_getValue(hash, gTestNodes[1], &gotten)
                               == TAFFY_SUCCESS);
        dcTestUtilities_checkEqual(gotten, gTestNodes[4]);
        dcHash_free(&hash, DC_SHALLOW);
    }

    // remove an element
    {
        dcHash *hash = dcHash_create();
        dcNode *value = dcNode_copy(gTestNodes[2], DC_DEEP);
        dcNode_register(value);
        dcHash_setValue(hash, gTestNodes[1], value);
        dcNode *removed = NULL;
        dcTestUtilities_assert(dcHash_removeValue(hash,
                                                  gTestNodes[1],
                                                  &removed,
                                                  DC_SHALLOW)
                               == TAFFY_SUCCESS);
        dcTestUtilities_checkEqual(removed, value);
        dcHash_free(&hash, DC_SHALLOW);
    }

    // remove an element with string key
    {
        dcHash *hash = dcHash_create();
        dcNode *value = dcNode_copy(gTestNodes[2], DC_DEEP);
        dcNode_register(value);
        setValueWithStringKey(hash, "gTestNodes[2]", value);
        dcNode *removed = NULL;
        dcTestUtilities_assert(dcHash_removeValueWithStringKey(hash,
                                                               "gTestNodes[2]",
                                                               &removed,
                                                               DC_SHALLOW)
                               == TAFFY_SUCCESS);
        dcTestUtilities_checkEqual(removed, value);
        dcHash_free(&hash, DC_SHALLOW);
    }

    // remove an element with string key + hash value
    {
        dcHash *hash = dcHash_create();
        dcNode *value = dcNode_copy(gTestNodes[2], DC_DEEP);
        dcNode_register(value);
        const char *key = "gTestNodes[2]";
        setValueWithStringKey(hash, key, value);
        dcNode *removed = NULL;
        dcHashType hashValue = 0;
        assert(dcString_hashCharArray(key, &hashValue) == TAFFY_SUCCESS);
        assert(dcHash_removeValueWithStringKeys(hash,
                                                key,
                                                hashValue,
                                                &removed,
                                                DC_SHALLOW)
               == TAFFY_SUCCESS);
        dcTestUtilities_checkEqual(removed, value);
        dcHash_free(&hash, DC_SHALLOW);
    }

    // test rehash
    {
        dcHash *hash = dcHash_createWithCapacity(4);
        dcContainerSizeType i;

        for (i = 0; i < 30; i++)
        {
            dcHash_setValue(hash, gTestNodes[i], gTestNodes[i + 1]);
            dcTestUtilities_checkHashSize(hash, i + 1);
        }

        dcHash_free(&hash, DC_SHALLOW);
    }

    // test void
    {
        size_t i;

        for (i = 0; i < 100; i++)
        {
            dcHash *hash = dcHash_createWithCapacity(4);
            uint8_t *data = (uint8_t *)dcMemory_allocate(3);
            dcHash_setValue(hash,
                            dcThread_getSelf(),
                            dcVoid_createNode(data));

            dcNode *theNode;
            dcNode *me = dcThread_getSelf();
            dcResult hashResult = dcHash_getValue(hash, me, &theNode);
            assert(hashResult == TAFFY_SUCCESS
                   && theNode != NULL);

            dcHash_removeValue(hash, me, NULL, DC_DEEP);
            dcNode_free(&me, DC_DEEP);

            me = dcThread_getSelf();
            hashResult = dcHash_getValue(hash, me, &theNode);
            assert(hashResult == TAFFY_FAILURE
                   && theNode == NULL);

            dcNode_free(&me, DC_DEEP);
            dcHash_free(&hash, DC_DEEP);
        }
    }

    // test empty iterator
    {
        dcHash *hash = dcHash_createWithCapacity(2);
        dcHashIterator *it = dcHash_createIterator(hash);
        assert(dcHashIterator_getNext(it) == NULL);

        // try again
        assert(dcHashIterator_getNext(it) == NULL);

        dcHashIterator_free(&it);
        dcHash_free(&hash, DC_DEEP);
    }

    // test small iterator
    {
        size_t i;
        dcHash *hash = dcHash_createWithCapacity(2);

        for (i = 0; i < 2; i++)
        {
            dcHash_setValue(hash,
                            dcUnsignedInt32_createNode(i),
                            dcUnsignedInt32_createNode(i + 1));
        }

        dcHash *copy = dcHash_copy(hash, DC_DEEP);
        dcHashIterator *it = dcHash_createIterator(hash);
        dcNode *value;

        while ((value = dcHashIterator_getNext(it))
               != NULL)
        {
            dcHashElement *element = CAST_HASH_ELEMENT(value);
            assert(element->key.isNodeKey);
            assert(dcHash_removeValue(copy,
                                      element->key.keyUnion.nodeKey,
                                      NULL,
                                      DC_DEEP)
                   == TAFFY_SUCCESS);
        }

        // try again
        assert(dcHashIterator_getNext(it) == NULL);

        assert(copy->size == 0);
        dcHashIterator_free(&it);
        dcHash_free(&copy, DC_DEEP);
        dcHash_free(&hash, DC_DEEP);
    }

    // test medium iterator
    {
        size_t i;
        dcHash *hash = dcHash_createWithCapacity(15);

        for (i = 0; i < 20; i++)
        {
            dcHash_setValue(hash,
                            dcUnsignedInt32_createNode(i),
                            dcUnsignedInt32_createNode(i + 1));
        }

        dcHash *copy = dcHash_copy(hash, DC_DEEP);
        dcHashIterator *it = dcHash_createIterator(hash);
        dcNode *value;

        while ((value = dcHashIterator_getNext(it))
               != NULL)
        {
            dcHashElement *element = CAST_HASH_ELEMENT(value);
            assert(element->key.isNodeKey);
            assert(dcHash_removeValue(copy,
                                      element->key.keyUnion.nodeKey,
                                      NULL,
                                      DC_DEEP)
                   == TAFFY_SUCCESS);
        }

        // try again
        assert(dcHashIterator_getNext(it) == NULL);

        assert(copy->size == 0);
        dcHashIterator_free(&it);
        dcHash_free(&copy, DC_DEEP);
        dcHash_free(&hash, DC_DEEP);
    }

    // test large iterator
    {
        size_t i;
        dcHash *hash = dcHash_createWithCapacity(15);

        for (i = 0; i < 200; i++)
        {
            dcHash_setValue(hash,
                            dcUnsignedInt32_createNode(i),
                            dcUnsignedInt32_createNode(i + 1));
        }

        dcHash *copy = dcHash_copy(hash, DC_DEEP);
        dcHashIterator *it = dcHash_createIterator(hash);
        dcNode *value;

        while ((value = dcHashIterator_getNext(it))
               != NULL)
        {
            dcHashElement *element = CAST_HASH_ELEMENT(value);
            assert(element->key.isNodeKey);
            assert(dcHash_removeValue(copy,
                                      element->key.keyUnion.nodeKey,
                                      NULL,
                                      DC_DEEP)
                   == TAFFY_SUCCESS);
        }

        // try again
        assert(dcHashIterator_getNext(it) == NULL);

        assert(copy->size == 0);
        dcHashIterator_free(&it);
        dcHash_free(&copy, DC_DEEP);
        dcHash_free(&hash, DC_DEEP);
    }

    // test merge with no overlap
    {
        size_t i;
        dcHash *from = dcHash_createWithCapacity(15);
        size_t size = 5;

        for (i = 0; i < size; i++)
        {
            char key[20];
            snprintf(key, sizeof(key), "%zu", i);
            dcHash_setValueWithStringKey(from,
                                         key,
                                         dcUnsignedInt32_createNode(i));
        }

        dcHash *to = dcHash_create();
        dcHash_merge(to, from);
        assert(to->size == size);
        assert(from->size == size);

        dcHash_free(&from, DC_SHALLOW);
        dcHash_free(&to, DC_DEEP);
    }

    // test merge with some overlap
    {
        size_t i;
        dcHash *from = dcHash_createWithCapacity(15);
        size_t size = 5;
        dcHash *to = dcHash_create();

        for (i = 0; i < size; i++)
        {
            char key[20];
            snprintf(key, sizeof(key), "%zu", i);
            dcHash_setValueWithStringKey(from,
                                         key,
                                         dcUnsignedInt32_createNode(i));
        }

        for (i = 0; i < 2; i++)
        {
            char key[20];
            snprintf(key, sizeof(key), "%zu", i);
            dcHash_setValueWithStringKey(to,
                                         key,
                                         dcUnsignedInt32_createNode(i));
        }

        dcHash_merge(to, from);
        assert(to->size == size);
        assert(from->size == size - 2);

        dcHash_free(&from, DC_SHALLOW);
        dcHash_free(&to, DC_DEEP);
    }

    // test merge with all overlap
    {
        size_t i;
        dcHash *from = dcHash_createWithCapacity(15);
        size_t size = 5;
        dcHash *to = dcHash_create();

        for (i = 0; i < size; i++)
        {
            char key[20];
            snprintf(key, sizeof(key), "%zu", i);
            dcHash_setValueWithStringKey(from,
                                         key,
                                         dcUnsignedInt32_createNode(i));
        }

        for (i = 0; i < size; i++)
        {
            char key[20];
            snprintf(key, sizeof(key), "%zu", i);
            dcHash_setValueWithStringKey(to,
                                         key,
                                         dcUnsignedInt32_createNode(i));
        }

        dcHash_merge(to, from);
        assert(to->size == size);
        assert(from->size == 0);

        dcHash_free(&from, DC_SHALLOW);
        dcHash_free(&to, DC_DEEP);
    }

    testLotsOfNodesStartingSmall();
    testLotsOfNodesStartingBig();

    dcTestUtilities_end();
    dcGarbageCollector_free();

    return 0;
}
