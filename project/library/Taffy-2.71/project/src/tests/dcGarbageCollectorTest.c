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

#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcGarbageCollector.h"
#include "dcHash.h"
#include "dcHeap.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcPair.h"
#include "dcParser.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcThreadClass.h"
#include "dcTestUtilities.h"
#include "dcUnsignedInt32.h"

#define TEST_NODE_COUNT 10
#define GARBAGE_COLLECTOR_TEST_FILE_NAME "src/tests/dcGarbageCollectorTest.c"

static dcGarbageCollectorDebugState sWantedState[TEST_NODE_COUNT] =
    {TAFFY_GARBAGE_COLLECTOR_NO_TEST_STATE};
static dcNode *sTestNodes[TEST_NODE_COUNT] = {0};
static dcList *sMarkers = NULL;
static int sWatchCount = 0;
static bool sMarkContainer = false;

static void watcher(dcNode *_node, dcGarbageCollectorDebugState _state)
{
    int index = 0;

    for (index = 0; index < TEST_NODE_COUNT; index++)
    {
        if (sTestNodes[index] == _node)
        {
            break;
        }
    }

    dcTestUtilities_assert(index < TEST_NODE_COUNT);
    dcTestUtilities_assert(sWantedState[index] == _state);

    if (sWantedState[index] == TAFFY_GARBAGE_COLLECTOR_MARK_TO_NO)
    {
        sWantedState[index] = TAFFY_GARBAGE_COLLECTOR_CONDEMN;
    }
    else if (sWantedState[index] == TAFFY_GARBAGE_COLLECTOR_CONDEMN)
    {
        sWantedState[index] = TAFFY_GARBAGE_COLLECTOR_FREE;
    }

    sWatchCount++;
}

static void marker(void)
{
    if (sMarkContainer)
    {
        dcList_mark(sMarkers);
    }
    else
    {
        int i;

        for (i = 0; i < TEST_NODE_COUNT; i++)
        {
            dcNode_mark(sTestNodes[i]);
        }
    }
}

static void pushNodeToMark(dcNode *_node)
{
    dcList_push(sMarkers, _node);
}

static void popNodeToMark(void)
{
    dcNode *node = dcList_pop(sMarkers, DC_SHALLOW);
    dcNode_free(&node, DC_SHALLOW);
}

static void reset(void)
{
    int i;

    for (i = 0; i < TEST_NODE_COUNT; i++)
    {
        sTestNodes[i] = dcUnsignedInt32_createNode(i);
        dcNode_register(sTestNodes[i]);
    }
}

static void setWantedState(dcGarbageCollectorDebugState _state)
{
    int a;

    for (a = 0; a < TEST_NODE_COUNT; a++)
    {
        sWantedState[a] = _state;
    }
}

static void checkMarkToFalseWithCount(int _count)
{
    sWatchCount = 0;
    setWantedState(TAFFY_GARBAGE_COLLECTOR_MARK_TO_NO);
    dcGarbageCollector_go();
    dcTestUtilities_assert(sWatchCount == _count);
}

static void checkFreeWithCount(int _count)
{
    sWatchCount = 0;
    setWantedState(TAFFY_GARBAGE_COLLECTOR_CONDEMN);
    dcGarbageCollector_go();
    dcTestUtilities_assert(sWatchCount == _count * 2);
}

static void checkMarkToFalse(void)
{
    checkMarkToFalseWithCount(TEST_NODE_COUNT);
}

static void checkFree(void)
{
    checkFreeWithCount(TEST_NODE_COUNT);
}

static void removeRoot(void)
{
    dcGarbageCollector_removeRoot(&marker);
}

static void addRoot(void)
{
    dcGarbageCollector_addRoot(&marker);
}

static void testNodesMark(void)
{
    checkMarkToFalse();
    removeRoot();
    checkFree();
    addRoot();
}

static void testBasicMarking(void)
{
    int i;

    for (i = 0; i < TEST_NODE_COUNT; i++)
    {
        dcNode_mark(sTestNodes[i]);
    }

    testNodesMark();
}

static void testContainerWithCount(int _count)
{
    sMarkContainer = true;
    checkMarkToFalseWithCount(_count);
    popNodeToMark();
    checkFreeWithCount(_count);
    sMarkContainer = false;
}

static void testContainer(void)
{
    testContainerWithCount(TEST_NODE_COUNT);
}

static void testArrayMarking(void)
{
    int i;
    dcNode *array = dcArray_createNodeWithCapacity(TEST_NODE_COUNT);
    pushNodeToMark(array);

    for (i = 0; i < TEST_NODE_COUNT; i++)
    {
        dcArray_set(CAST_ARRAY(array), sTestNodes[i], i);
    }

    testContainer();
}

static void testHashMarking(void)
{
    int i;
    dcNode *hash = dcHash_createNode();
    pushNodeToMark(hash);

    for (i = 0; i < TEST_NODE_COUNT; i++)
    {
        dcHash_setValue(CAST_HASH(hash),
                        sTestNodes[i],
                        sTestNodes[i]);
    }

    testContainer();
}

static void testListMarking(void)
{
    int i;
    dcNode *list = dcList_createNode();
    pushNodeToMark(list);

    for (i = 0; i < TEST_NODE_COUNT; i++)
    {
        dcList_push(CAST_LIST(list), sTestNodes[i]);
    }

    testContainer();
}

static void testPairMarking(void)
{
    // clear the auto-added nodes, der
    testNodesMark();

    sTestNodes[1] = dcNode_register(dcUnsignedInt32_createNode(1));
    sTestNodes[2] = dcNode_register(dcUnsignedInt32_createNode(2));

    dcNode *pair = dcPair_createNode(sTestNodes[1], sTestNodes[2]);
    pushNodeToMark(pair);

    testContainerWithCount(2);
}

static void testHeapMarking(void)
{
    dcNode *heap = dcHeap_createNode(HEAP_MIN);
    uint32_t i;

    pushNodeToMark(heap);

    for (i = 0; i < TEST_NODE_COUNT; i++)
    {
        dcHeap_insert(CAST_HEAP(heap), sTestNodes[i]);
    }

    testContainer();
}

static const dcTestFunctionMap basicMap[] =
{
    {"Basic Marking", &testBasicMarking},
    {"Array Marking", &testArrayMarking},
    {"Hash Marking",  &testHashMarking},
    {"List Marking",  &testListMarking},
    {"Pair Marking",  &testPairMarking},
    {"Heap Marking",  &testHeapMarking},
    {NULL}
};

static void testArrayObject(void)
{
    dcArray *objects = dcArray_createWithSize(500);
    size_t i;

    for (i = 0; i < 500; i++)
    {
        dcNode *object = dcNumberClass_createObjectFromInt32s(i);
        dcArray_set(objects, object, i);
    }

    dcNode *array = dcArrayClass_createObject(objects, true);
    dcNode_register(array);

    dcGarbageCollector_go();
    dcGarbageCollector_go();
}

static void testProcedureObject(void)
{
    dcNode *procedure = dcProcedureClass_createObject
        (dcUnsignedInt32_createNode(1),
         dcMethodHeader_create("hiThere", NULL));
    dcNode_register(procedure);
    dcGarbageCollector_go();
}

static void testObjectWithComposition(void)
{
    const char *program =
        "class Test"
        "{"
        "    @value, @rw"
        "}\n"
        "array = [1, 2, 3, 4, 6, 7, 8, 9, 10]\n"
        "test = new Test\n"
        "test setValue: array\n";
    dcStringEvaluator_evalString(program,
                                 GARBAGE_COLLECTOR_TEST_FILE_NAME,
                                 NO_STRING_EVALUATOR_FLAGS);
}

static const dcTestFunctionMap runtimeMap[] =
{
    {"Array Object",                      &testArrayObject},
    {"Procedure Object",                  &testProcedureObject},
    {"Object with Composition",           &testObjectWithComposition},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    sMarkers = dcList_create();
    dcGarbageCollector_addRoot(&marker);
    dcGarbageCollector_setWatcher(&watcher);
    dcTestUtilities_startAndCreateNodes("GarbageCollectorTest",
                                        _argc,
                                        _argv,
                                        false);

    // test basic tests
    dcTestUtilities_runTests(&reset, basicMap, false);
    dcGarbageCollector_removeRoot(&marker);
    dcGarbageCollector_clearWatcher();

    dcSystem_create();

    // test object tests
    dcTestUtilities_runTests(NULL, runtimeMap, false);

    // we're done
    dcTestUtilities_end();
    dcSystem_free();
    dcList_free(&sMarkers, DC_DEEP);
    dcMemory_deinitialize();
    return 0;
}
