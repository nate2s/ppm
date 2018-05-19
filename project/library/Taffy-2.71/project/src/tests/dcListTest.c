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

#include "dcArray.h"
#include "dcError.h"
#include "dcGarbageCollector.h"
#include "dcUnsignedInt32.h"
#include "dcList.h"
#include "dcNode.h"
#include "tests/dcTestUtilities.h"
#include "dcFlatArithmetic.h"
#include "dcGraphData.h"

static dcNode **sEachNodes = NULL;
static size_t sEachNodesSize = 0;
static size_t sEachNodesIndex = 0;
static dcNode *sEachToken = NULL;
static uint32_t sCombinationIterations = 0;

static void combinationFunction(dcList *_left, dcList *_right, uint32_t _token)
{
    //printf("left:  [%s]\n"
    //       "right: [%s]\n\n",
    //       dcList_display(_left),
    //       dcList_display(_right));
    sCombinationIterations++;
}

static dcResult eachFunction(dcNode *_node, dcNode *_token)
{
    dcTestUtilities_assert(_token == sEachToken
                           && sEachNodesIndex < sEachNodesSize
                           && sEachNodes[sEachNodesIndex] == _node);
    sEachNodesIndex++;
    return TAFFY_SUCCESS;
}

static dcNode *createInt(uint32_t _value)
{
    return dcNode_setTemplate(dcUnsignedInt32_createNode(_value), true);
}

int main(int _argc, char **_argv)
{
    dcGarbageCollector_create();
    dcTestUtilities_start("List Test", _argc, _argv);

    // verify that a new list has size 0
    {
        dcList *list = dcList_create();
        dcTestUtilities_checkListSize(list, 0);
        dcList_free(&list, DC_DEEP);
    }

    // test dcList_push()
    //      dcList_clear()
    // verify that a call to dcList_push() makes the size 1
    {
        dcList *list = dcList_create();
        dcList_push(list, gTestNodes[1]);
        dcTestUtilities_checkListSize(list, 1);

        // verify that a call to dcList_push() makes the size 2
        dcList_push(list, gTestNodes[2]);
        dcTestUtilities_checkListSize(list, 2);

        // verify dcList_clear() makes the size 0
        dcList_clear(list, DC_SHALLOW);
        dcTestUtilities_checkListSize(list, 0);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_unshift()
    {
        dcList *list = dcList_create();
        // verify that a call to dcList_unshift() makes the size 1
        dcList_unshift(list, gTestNodes[1]);
        dcTestUtilities_checkListSize(list, 1);

        // verify that a call to dcList_unshift() makes the size 2
        dcList_unshift(list, gTestNodes[2]);
        dcTestUtilities_checkListSize(list, 2);

        // clear the list and verify all sizes & reference counts are correct
        dcList_clear(list, DC_SHALLOW);
        dcTestUtilities_checkListSize(list, 0);
        dcList_free(&list, DC_DEEP);
    }

    // test dcList_createWithObjects()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                NULL);
        dcTestUtilities_checkListSize(list, 2);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_createNodeWithObjects
    // and test dcList_get()
    {
        dcNode *list = dcList_createNodeWithObjects(gTestNodes[1],
                                                    gTestNodes[2],
                                                    NULL);
        dcTestUtilities_checkListSize(CAST_LIST(list), 2);
        dcTestUtilities_checkEqual(dcList_get(CAST_LIST(list), 0),
                                   gTestNodes[1]);
        dcTestUtilities_checkEqual(dcList_get(CAST_LIST(list), 1),
                                   gTestNodes[2]);
        dcNode_free(&list, DC_SHALLOW);
    }

    // test dcList_setHead(),
    //      dcList_getHead(),
    //      dcList_getTail()
    {
        dcList *list = dcList_create();
        dcList_setHead(list, gTestNodes[1]);
        dcTestUtilities_checkListSize(list, 1);
        dcTestUtilities_checkEqual(dcList_getHead(list), gTestNodes[1]);
        dcList_setHead(list, gTestNodes[1]);
        dcTestUtilities_checkListSize(list, 1);
        dcTestUtilities_checkEqual(dcList_getTail(list), gTestNodes[1]);
        dcTestUtilities_checkEqual(dcList_getHead(list), gTestNodes[1]);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_insertBeforeListElement()
    //      dcList_getNeckElement()
    {
        dcList *list = dcList_create();
        dcList_push(list, gTestNodes[1]);
        dcListElement *headElement = list->head;
        dcList_insertBeforeListElement(list, headElement, gTestNodes[2]);
        dcList_insertBeforeListElement(list, NULL, gTestNodes[3]);
        dcTestUtilities_checkListSize(list, 3);
        dcTestUtilities_checkEqual(dcList_getHead(list), gTestNodes[2]);
        dcTestUtilities_checkEqual
            (dcList_getNeckElement(list)->object, gTestNodes[1]);
        dcTestUtilities_checkEqual(dcList_getTail(list), gTestNodes[3]);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_pop()
    {
        dcList *list = dcList_create();
        dcList_push(list, gTestNodes[1]);
        dcList_push(list, gTestNodes[2]);
        dcList_push(list, gTestNodes[3]);
        dcTestUtilities_checkListSize(list, 3);
        dcList_pop(list, DC_SHALLOW);
        dcTestUtilities_checkListSize(list, 2);
        dcTestUtilities_checkEqual(dcList_getTail(list), gTestNodes[2]);
        dcList_pop(list, DC_SHALLOW);
        dcTestUtilities_checkListSize(list, 1);
        dcTestUtilities_checkEqual(dcList_getTail(list), gTestNodes[1]);
        dcList_pop(list, DC_SHALLOW);
        dcTestUtilities_checkListSize(list, 0);
        dcList_pop(list, DC_SHALLOW);
        dcTestUtilities_checkListSize(list, 0);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_popNTimes()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                gTestNodes[3],
                                                NULL);
        dcList_popNTimes(list, 2, DC_SHALLOW);
        dcTestUtilities_checkListSize(list, 1);
        dcTestUtilities_checkEqual(dcList_getTail(list), gTestNodes[1]);
        // go over the number of objects on purpose
        dcList_popNTimes(list, 2, DC_SHALLOW);
        dcTestUtilities_checkListSize(list, 0);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_shift()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                gTestNodes[3],
                                                NULL);
        assert(dcList_shift(list, DC_SHALLOW) == gTestNodes[1]);
        dcTestUtilities_checkListSize(list, 2);
        assert(dcList_shift(list, DC_SHALLOW) == gTestNodes[2]);
        dcTestUtilities_checkListSize(list, 1);
        assert(dcList_shift(list, DC_SHALLOW) == gTestNodes[3]);
        dcTestUtilities_checkListSize(list, 0);
        assert(dcList_shift(list, DC_SHALLOW) == NULL);
        dcTestUtilities_checkListSize(list, 0);
        dcList_free(&list, DC_DEEP);
    }

    // test dcList_copy()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                gTestNodes[3],
                                                NULL);
        dcList *deepCopy = dcList_copy(list, DC_DEEP);
        dcTestUtilities_checkListSize(deepCopy, 3);
        dcList_free(&deepCopy, DC_DEEP);

        dcList *shallowCopy = dcList_copy(list, DC_SHALLOW);
        dcTestUtilities_checkListSize(shallowCopy, 3);
        dcList_free(&shallowCopy, DC_SHALLOW);
        dcList_free(&list, DC_SHALLOW);
    }

    // test pop copy
    {
        dcList *preList = dcList_createWithObjects(gTestNodes[1],
                                                   gTestNodes[2],
                                                   gTestNodes[3],
                                                   NULL);
        dcList *list = dcList_create();

        while (preList->size > 0)
        {
            dcList_push(list, dcList_getTail(preList));
            dcList_pop(preList, DC_SHALLOW);
        }

        dcTestUtilities_assert(preList->size == 0
                               && list->size == 3);
        dcList_free(&list, DC_SHALLOW);
        dcList_free(&preList, DC_SHALLOW);
    }

    // test dcList_asArray()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                gTestNodes[3],
                                                NULL);
        dcArray *array = dcList_asArray(list);
        dcTestUtilities_checkArraySize(array, 3);
        dcArray_free(&array, DC_SHALLOW);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_createFromArray()
    {
        dcArray *array = dcArray_createWithObjects(gTestNodes[1],
                                                   gTestNodes[2],
                                                   gTestNodes[3],
                                                   NULL);
        dcList *list = dcList_createFromArray(array);
        dcTestUtilities_checkListSize(list, 3);
        dcArray_free(&array, DC_SHALLOW);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_remove()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                gTestNodes[3],
                                                NULL);
        dcTestUtilities_assert(dcList_remove(list,
                                             gTestNodes[1],
                                             DC_SHALLOW)
                               == TAFFY_SUCCESS);
        dcTestUtilities_checkListSize(list, 2);
        dcTestUtilities_assert(dcList_remove(list,
                                             gTestNodes[1],
                                             DC_SHALLOW)
                               == TAFFY_FAILURE);
        dcTestUtilities_checkListSize(list, 2);

        dcTestUtilities_assert(dcList_remove(list,
                                             gTestNodes[2],
                                             DC_SHALLOW)
                               == TAFFY_SUCCESS);
        dcTestUtilities_checkListSize(list, 1);

        dcTestUtilities_assert(dcList_remove(list,
                                             gTestNodes[3],
                                             DC_SHALLOW)
                               == TAFFY_SUCCESS);
        dcTestUtilities_checkListSize(list, 0);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_contains()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                NULL);
        dcTestUtilities_assert(dcList_contains(list, gTestNodes[1]));
        dcTestUtilities_assert(dcList_contains(list, gTestNodes[2]));
        dcTestUtilities_assert(! dcList_contains(list, gTestNodes[3]));
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_removeElement()
    //      dcList_getNeckElement()
    //      dcList_getHeadElement()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                gTestNodes[3],
                                                NULL);
        dcListElement *neck = dcList_getNeckElement(list);
        dcList_removeElement(list, &neck, DC_SHALLOW);
        dcTestUtilities_checkListSize(list, 2);
        dcListElement *head = dcList_getHeadElement(list);
        dcList_removeElement(list, &head, DC_SHALLOW);
        dcTestUtilities_checkListSize(list, 1);
        dcList_free(&list, DC_SHALLOW);
    }

    // test removeElement on head
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                gTestNodes[3],
                                                NULL);
        uint32_t i;
        uint32_t size = 3;

        for (i = 0; i < size; i++)
        {
            dcListElement *head = list->head;
            dcList_removeElement(list, &head, DC_SHALLOW);
            dcTestUtilities_assert(list->size == (size - i - 1));
        }

        dcList_free(&list, DC_DEEP);
    }

    // test removeElement on tail
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                gTestNodes[3],
                                                NULL);
        uint32_t i;
        uint32_t size = 3;

        for (i = 0; i < size; i++)
        {
            dcListElement *tail = list->tail;
            dcList_removeElement(list, &tail, DC_SHALLOW);
            dcTestUtilities_assert(list->size == (size - i - 1));
        }

        dcList_free(&list, DC_DEEP);
    }

    // test removeElement on head and tail
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                NULL);
        dcListElement *tail = list->tail;
        dcList_removeElement(list, &tail, DC_SHALLOW);
        dcListElement *head = list->head;
        dcList_removeElement(list, &head, DC_SHALLOW);
        dcTestUtilities_assert(list->size == 0);
        dcList_free(&list, DC_DEEP);
    }

    // test removeElement tail update
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                NULL);
        dcListElement *head = list->head;
        dcList_removeElement(list, &head, DC_SHALLOW);
        dcTestUtilities_assert(list->head == list->tail
                               && list->tail->object != NULL);
        dcList_free(&list, DC_DEEP);
    }

    // test removeElement head update
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                NULL);
        dcListElement *tail = list->tail;
        dcList_removeElement(list, &tail, DC_SHALLOW);
        dcTestUtilities_assert(list->head == list->tail
                               && list->head->object != NULL);
        dcList_free(&list, DC_DEEP);
    }

    // test dcList_concat() -- shallow
    {
        dcList *first = dcList_createWithObjects(gTestNodes[1], NULL);
        dcList *second = dcList_createWithObjects(gTestNodes[2],
                                                  gTestNodes[3],
                                                  NULL);
        dcList_concat(first, second, DC_SHALLOW);
        dcTestUtilities_checkListSize(first, 3);
        dcTestUtilities_checkListSize(second, 2);
        dcList *all = dcList_createWithObjects(gTestNodes[1],
                                               gTestNodes[2],
                                               gTestNodes[3],
                                               NULL);
        dcTaffyOperator compareResult;

        dcTestUtilities_assert(dcList_compare(first, all, &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_EQUALS);
        dcList_free(&first, DC_SHALLOW);
        dcList_free(&second, DC_SHALLOW);
        dcList_free(&all, DC_SHALLOW);
    }

    // test dcList_compare()
    {
        dcList *first = dcList_createWithObjects(gTestNodes[1], NULL);
        dcList *second = dcList_createWithObjects(gTestNodes[2],
                                                  gTestNodes[3],
                                                  NULL);
        dcList *third = dcList_createWithObjects(gTestNodes[3], NULL);
        dcList *fourth = dcList_createWithObjects(gTestNodes[3], NULL);
        dcList *fifth = dcList_createWithObjects(gTestNodes[2], NULL);
        dcTaffyOperator compareResult;

        dcTestUtilities_assert(dcList_compare(first, second, &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_LESS_THAN);
        dcTestUtilities_assert(dcList_compare(third, fourth, &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_EQUALS);
        dcTestUtilities_assert(dcList_compare(fourth, fifth, &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_GREATER_THAN);
        dcTestUtilities_assert(dcList_compare(second, fifth, &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_GREATER_THAN);
        dcTestUtilities_assert(dcList_compare(first, fifth, &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_LESS_THAN);
        dcList_free(&first, DC_SHALLOW);
        dcList_free(&second, DC_SHALLOW);
        dcList_free(&third, DC_SHALLOW);
        dcList_free(&fourth, DC_SHALLOW);
        dcList_free(&fifth, DC_SHALLOW);
    }

    // test dcList_mark()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                NULL);
        dcTestUtilities_doMark(false);
        dcList_mark(list);
        dcTestUtilities_assert(dcNode_isMarked(gTestNodes[1]));
        dcTestUtilities_assert(dcNode_isMarked(gTestNodes[2]));
        dcList_free(&list, DC_SHALLOW);
    }

    // check that a blank list doesn't crash
    {
        dcList *list = dcList_createWithObjects(NULL);
        dcList_mark(list);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcList_each()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                gTestNodes[3],
                                                NULL);
        dcNode *nodes[] = {gTestNodes[1], gTestNodes[2], gTestNodes[3]};
        sEachNodes = nodes;
        sEachNodesSize = 3;
        sEachNodesIndex = 0;
        sEachToken = gTestNodes[4];

        dcList_each(list, &eachFunction, sEachToken);
        dcTestUtilities_assert(sEachNodesIndex == 3);
        dcList_free(&list, DC_SHALLOW);
    }

    // lots of nodes
    {
        uint32_t size = 50000;
        dcList *list = dcList_create();
        uint32_t i;

        for (i = 0; i < size; i++)
        {
            dcList_push(list, dcUnsignedInt32_createNode(i));
        }

        dcListElement *that;

        for (i = 0, that = list->head; i < size; i++, that = that->next)
        {
            dcError_assert(that != NULL
                           && (dcUnsignedInt32_getInt(that->object)
                               == i));
        }

        dcTestUtilities_assert(list->size == size);
        dcList_free(&list, DC_DEEP);
    }

    // replace range, all
    {
        uint32_t i;
        uint32_t size = 10;
        dcList *list = dcList_create();

        for (i = 0; i < size; i++)
        {
            dcList_push(list, dcUnsignedInt32_createNode(i));
        }

        dcList_replaceRange(list,
                            list->head,
                            list->tail,
                            dcUnsignedInt32_createNode(100),
                            DC_DEEP);

        dcError_assert(list->size == 1
                       && CAST_INT(list->head->object) == 100);

        dcList_free(&list, DC_DEEP);
    }

    // replace range, some
    {
        uint32_t i;
        uint32_t size = 10;
        dcList *list = dcList_create();

        for (i = 0; i < size; i++)
        {
            dcList_push(list, dcUnsignedInt32_createNode(i));
        }

        dcList_replaceRange(list,
                            list->head->next,
                            list->tail,
                            dcUnsignedInt32_createNode(100),
                            DC_DEEP);

        dcError_assert(list->size == 2
                       && CAST_INT(list->head->object) == 0
                       && CAST_INT(list->tail->object) == 100);

        dcList_free(&list, DC_DEEP);
    }

    // replace range, some, shallow
    {
        uint32_t i;
        uint32_t size = 3;
        dcList *list = dcList_create();
        dcList *toFree = dcList_create();

        for (i = 0; i < size; i++)
        {
            dcNode *inty = dcUnsignedInt32_createNode(i);
            dcList_push(list,
                        (i == 0
                         ? dcNode_copy(inty, DC_DEEP)
                         : inty));
            dcList_push(toFree, inty);
        }

        dcList_replaceRange(list,
                            list->head->next,
                            list->tail,
                            dcUnsignedInt32_createNode(100),
                            DC_SHALLOW);

        dcError_assert(list->size == 2
                       && CAST_INT(list->head->object) == 0
                       && CAST_INT(list->tail->object) == 100);

        dcList_free(&list, DC_DEEP);
        dcList_free(&toFree, DC_DEEP);
    }

    // flat arithmetic -- insert before tail, remove tail
    {
        dcNode *flats = (dcFlatArithmetic_createNodeWithValues
                         (TAFFY_ADD,
                          createInt(100),
                          createInt(101),
                          NULL));
        dcNode *parentNode = (dcFlatArithmetic_createNodeWithValues
                              (TAFFY_ADD,
                               createInt(0),
                               createInt(1),
                               NULL));
        dcFlatArithmetic *parent = CAST_FLAT_ARITHMETIC(parentNode);
        dcList_unshift(parent->values, flats);
        dcFlatArithmetic *other =
            CAST_FLAT_ARITHMETIC(parent->values->head->object);
        dcListElement *that = parent->values->head;

        while (other->values->size > 0)
        {
            dcList_insertBeforeListElement
                (parent->values,
                 that,
                 dcList_shift(other->values, DC_SHALLOW));
        }

        //printf("before remove: %s\n", dcNode_display(parentNode));
        dcList_removeElement(parent->values, &that, DC_DEEP);
        //printf("after remove: %s\n", dcNode_display(parentNode));
        dcNode_free(&parentNode, DC_DEEP);
    }

    // find other lists
    {
        dcNode *listA = dcList_createNodeWithObjects(createInt(1),
                                                     createInt(1),
                                                     createInt(0),
                                                     NULL);
        dcList *list = dcList_createWithObjects(listA, NULL);
        dcNode *listB = dcList_createNodeWithObjects(createInt(1),
                                                     createInt(1),
                                                     createInt(0),
                                                     NULL);
        dcError_assert(dcList_find(list, listB, NULL));

        dcList_free(&list, DC_DEEP);
        dcNode_free(&listB, DC_DEEP);
    }

    // iterate combinations
    {
        struct
        {
            uint32_t size;
            uint32_t expectedIterations;
        } combinations[] =
        {
            {0, 0},
            {1, 1},
            {2, 2},
            {3, 6},
            {4, 12},
            {5, 20}
        };

        size_t i;

        for (i = 0; i < dcTaffy_countOf(combinations); i++)
        {
            sCombinationIterations = 0;

            dcList *list = dcList_create();
            uint32_t j;

            for (j = 0; j < combinations[i].size; j++)
            {
                dcList_push(list, dcUnsignedInt32_createNode(j));
            }

            //printf("iterating combinations on: [%s]\n", dcList_display(list));
            dcList_iterateCombinations(list, &combinationFunction, 0);
            dcList_free(&list, DC_DEEP);

            if (sCombinationIterations != combinations[i].expectedIterations)
            {
                fprintf(stderr,
                        "[%zu] iterations: %u != expected: %u\n",
                        i,
                        sCombinationIterations,
                        combinations[i].expectedIterations);
                dcError_assert(false);
            }
        }
    }

    // insert before list element in blank list
    {
        dcList *list = dcList_create();
        dcNode *one = createInt(1);
        dcList_insertBeforeListElement(list, NULL, one);
        dcError_assert(list->size == 1
                       && list->head->object == one
                       && list->tail->object == one
                       && list->head->next == NULL
                       && list->head->previous == NULL
                       && list->tail->next == NULL
                       && list->tail->previous == NULL);
        dcList_free(&list, DC_DEEP);
    }

    // append empty list
    {
        dcNode *one = createInt(1);
        dcNode *two = createInt(2);

        dcList *appended = dcList_createWithObjects(NULL);
        dcList *list = dcList_createWithObjects(one, two, NULL);

        dcList_append(list, &appended);

        dcError_assert(list->size == 2);
        dcError_assert(list->head->object == one);
        dcError_assert(list->head->next->object == two);
        dcList_verify(list);
        dcList_free(&list, DC_DEEP);
    }

    // append onto empty list another list with one element
    {
        dcNode *one = createInt(1);

        dcList *appended = dcList_createWithObjects(one, NULL);
        dcList *list = dcList_createWithObjects(NULL);

        dcList_append(list, &appended);

        dcError_assert(list->size == 1);
        dcError_assert(list->head->object == one);
        dcList_verify(list);
        dcList_free(&list, DC_DEEP);
    }

    // append onto empty list another list with two elements
    {
        dcNode *one = createInt(1);
        dcNode *two = createInt(2);

        dcList *appended = dcList_createWithObjects(one, two, NULL);
        dcList *list = dcList_createWithObjects(NULL);

        dcList_append(list, &appended);

        dcError_assert(list->size == 2);
        dcError_assert(list->head->object == one);
        dcError_assert(list->tail->object == two);
        dcList_verify(list);
        dcList_free(&list, DC_DEEP);
    }

    // append onto empty list another list with three elements
    {
        dcNode *one = createInt(1);
        dcNode *two = createInt(2);
        dcNode *three = createInt(2);

        dcList *appended = dcList_createWithObjects(one, two, three, NULL);
        dcList *list = dcList_createWithObjects(NULL);

        dcList_append(list, &appended);

        dcError_assert(list->size == 3);
        dcError_assert(list->head->object == one);
        dcError_assert(list->head->next->object == two);
        dcError_assert(list->tail->object == three);
        dcList_verify(list);
        dcList_free(&list, DC_DEEP);
    }

    // append list with one element
    {
        dcNode *one = createInt(1);
        dcNode *two = createInt(2);
        dcNode *three = createInt(3);

        dcList *appended = dcList_createWithObjects(three, NULL);
        dcList *list = dcList_createWithObjects(one, two, NULL);

        dcList_append(list, &appended);

        dcError_assert(list->size == 3);
        dcError_assert(list->head->object == one);
        dcError_assert(list->head->next->object == two);
        dcError_assert(list->head->next->next->object == three);
        dcList_verify(list);
        dcList_free(&list, DC_DEEP);
    }

    // append neither empty
    {
        dcNode *one = createInt(1);
        dcNode *two = createInt(2);
        dcNode *three = createInt(3);
        dcNode *four = createInt(4);

        dcList *appended = dcList_createWithObjects(three, four, NULL);
        dcList *list = dcList_createWithObjects(one, two, NULL);

        dcList_append(list, &appended);

        dcError_assert(list->size == 4);
        dcError_assert(list->head->object == one);
        dcError_assert(list->head->next->object == two);
        dcError_assert(list->head->next->next->object == three);
        dcError_assert(list->tail->object == four);
        dcList_verify(list);
        dcList_free(&list, DC_DEEP);
    }

    dcTestUtilities_end();
    dcGarbageCollector_free();

    return 0;
}
