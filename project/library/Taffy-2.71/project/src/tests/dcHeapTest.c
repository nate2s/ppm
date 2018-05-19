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

#include "dcArray.h"
#include "dcError.h"
#include "dcGarbageCollector.h"
#include "dcHeap.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"
#include "dcUnsignedInt32.h"

#include <stdio.h>

#define HEAP_TEST_FILE_NAME "src/tests/dcHeapTest.c"

static dcHeap *createHeap(dcHeapType _type, uint32_t _size)
{
    dcHeap *heap = dcHeap_create(_type);
    uint32_t i;

    for (i = 0; i < _size; i++)
    {
        dcHeap_insert(heap, dcUnsignedInt32_createNode(rand() % 1000));
    }

    return heap;
}

static void testHeap(dcHeapType _type)
{
    const uint32_t size = 50000;
    dcHeap *heap = createHeap(_type, size);
    uint32_t previous = 0;
    uint32_t i;

    for (i = 0; i < size; i++)
    {
        dcNode *node;
        dcTestUtilities_assert(dcHeap_pop(heap, &node) == TAFFY_SUCCESS
                               && node != NULL);
        uint32_t that = dcUnsignedInt32_getInt(node);

        if (i > 0)
        {
            if (_type == HEAP_MIN)
            {
                dcTestUtilities_assert(previous <= that);
            }
            else if (_type == HEAP_MAX)
            {
                dcTestUtilities_assert(previous >= that);
            }
            else
            {
                dcTestUtilities_assert(false);
            }
        }

        previous = that;
        dcNode_free(&node, DC_DEEP);
    }

    dcHeap_free(&heap, DC_DEEP);
}

static void testMinHeap(void)
{
    testHeap(HEAP_MIN);
}

static void testMaxHeap(void)
{
    testHeap(HEAP_MAX);
}

static void testCopy(void)
{
    dcHeap *heap = createHeap(HEAP_MIN, 1000);
    dcHeap *copy = dcHeap_copy(heap, DC_DEEP);

    dcTaffyOperator operatorResult;
    dcTestUtilities_assert((dcHeap_compare(heap, copy, &operatorResult)
                            == TAFFY_SUCCESS)
                           && operatorResult == TAFFY_EQUALS);

    dcHeap_free(&heap, DC_DEEP);
    dcHeap_free(&copy, DC_DEEP);
}

static void testClear(void)
{
    dcHeap *heap = createHeap(HEAP_MAX, 1000);
    dcTestUtilities_assert(heap->objects->size == 1000);
    dcHeap_clear(heap, DC_DEEP);
    dcTestUtilities_assert(heap->objects->size == 0);
    dcHeap_free(&heap, DC_DEEP);
}

static void testRegister(void)
{
    dcHeap *heap = createHeap(HEAP_MAX, 1000);
    dcNode *heapNode = dcHeap_createShell(heap);
    uint32_t i;

    for (i = 0; i < heap->objects->size; i++)
    {
        dcTestUtilities_assert
            (! dcNode_isRegistered(heap->objects->objects[i]));
    }

    dcNode_register(heapNode);

    for (i = 0; i < heap->objects->size; i++)
    {
        dcTestUtilities_assert(dcNode_isRegistered(heap->objects->objects[i]));
    }

    // the garbage collector will free the heap contents
    dcNode_free(&heapNode, DC_SHALLOW);
}

static dcNode *sHeapNode = NULL;

static void markHeap(void)
{
    dcNode_mark(sHeapNode);
}

static void testRegisterWhileInserting(void)
{
    dcHeap *heap = createHeap(HEAP_MAX, 20);
    sHeapNode = dcHeap_createShell(heap);
    uint32_t i;
    uint32_t count = 150;

    dcGarbageCollector_addRoot(&markHeap);
    dcNode_register(sHeapNode);

    for (i = 0; i < count; i++)
    {
        dcNode *integer = dcNode_register(dcUnsignedInt32_createNode(i));
        dcError_assert(dcHeap_insert(heap, integer) == TAFFY_SUCCESS);
        dcGarbageCollector_go();
    }

    for (i = 0; i < count; i++)
    {
        dcTestUtilities_assert(dcNode_isRegistered(heap->objects->objects[i]));
    }

    // the garbage collector will free the heap contents
    dcNode_free(&sHeapNode, DC_SHALLOW);
    dcGarbageCollector_removeRoot(&markHeap);
}

static const dcTestFunctionMap sTests[] =
{
    {"Min Heap",                 &testMinHeap},
    {"Max Heap",                 &testMaxHeap},
    {"Copy",                     &testCopy},
    {"Clear",                    &testClear},
    {"Register",                 &testRegister},
    {"Register While Inserting", &testRegisterWhileInserting},
    {NULL}
};

static void testNumberUpToWithHeapInsert(void)
{
    dcTestUtilities_assert(dcStringEvaluator_evalString
                           ("h = [org.taffy.core.container.Heap newMin];"
                            "-100 upTo: 100 do: ^{ <val>"
                            "h insert: val}",
                            HEAP_TEST_FILE_NAME,
                            NO_STRING_EVALUATOR_FLAGS)
                           != NULL);
}

static const dcTestFunctionMap sRuntimeTests[] =
{
    {"Number#upTo: with Heap#insert", &testNumberUpToWithHeapInsert},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcTestUtilities_go("Heap Test", _argc, _argv, NULL, sTests, false);

    if (dcTestUtilities_runSystem())
    {
        dcSystem_createWithArguments
            (dcTaffyCommandLineArguments_parseAndCreateWithFailure
             (_argc,
              _argv,
              false));
        dcTestUtilities_go("Heap Runtime Test",
                           _argc,
                           _argv,
                           NULL,
                           sRuntimeTests,
                           true);
        dcSystem_free();
    }

    dcGarbageCollector_free();
    dcMemory_deinitialize();
    return 0;
}
