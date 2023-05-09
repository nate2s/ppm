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
#include "dcList.h"
#include "dcNode.h"
#include "dcTestUtilities.h"
#include "dcUnsignedInt32.h"

int main(int _argc, char **_argv)
{
    dcGarbageCollector_create();
    dcTestUtilities_start("Array Test", _argc, _argv);

    // test dcArray_createWithObjects()
    {
        dcArray *array = dcArray_createWithObjects(gTestNodes[1],
                                                   gTestNodes[2],
                                                   NULL);
        dcTestUtilities_checkIntEqual(array->size, 2);
        dcTestUtilities_checkEqual(array->objects[0], gTestNodes[1]);
        dcTestUtilities_checkEqual(array->objects[1], gTestNodes[2]);
        dcArray_free(&array, DC_SHALLOW);
    }

    // test dcArray_createNodeWithObjects()
    {
        dcNode *array = dcArray_createNodeWithObjects(gTestNodes[1],
                                                      gTestNodes[2],
                                                      NULL);
        dcTestUtilities_checkIntEqual(CAST_ARRAY(array)->size, 2);
        dcTestUtilities_checkEqual(CAST_ARRAY(array)->objects[0],
                                   gTestNodes[1]);
        dcTestUtilities_checkEqual(CAST_ARRAY(array)->objects[1],
                                   gTestNodes[2]);
        dcNode_free(&array, DC_SHALLOW);
    }

    // test dcArray_createNodeFromList()
    {
        dcList *list = dcList_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                NULL);
        dcNode *array = dcArray_createNodeFromList(list, DC_SHALLOW);
        dcTestUtilities_checkIntEqual(dcArray_getSize(array), 2);
        dcNode **objects = dcArray_getObjects(array);
        dcTestUtilities_checkEqual(objects[0], gTestNodes[1]);
        dcTestUtilities_checkEqual(objects[1], gTestNodes[2]);
        dcNode_free(&array, DC_SHALLOW);

        dcNode *deep = dcArray_createNodeFromList(list, DC_DEEP);
        dcTestUtilities_checkIntEqual(dcArray_getSize(deep), 2);
        objects = dcArray_getObjects(deep);

        dcTaffyOperator compareResult;
        dcTestUtilities_assert(dcNode_compare(objects[0],
                                              gTestNodes[1],
                                              &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_EQUALS);
        dcTestUtilities_assert(dcNode_compare(objects[1],
                                              gTestNodes[2],
                                              &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_EQUALS);
        dcNode_free(&deep, DC_DEEP);
        dcList_free(&list, DC_SHALLOW);
    }

    // test dcArray_set()
    {
        dcArray *array = dcArray_createWithSize(3);
        dcArray_set(array, gTestNodes[1], 1);
        dcArray_set(array, gTestNodes[2], 2);
        dcNode **objects = array->objects;
        //dcTestUtilities_checkEqual(objects[0], nil);
        dcTestUtilities_checkEqual(objects[1], gTestNodes[1]);
        dcTestUtilities_checkEqual(objects[2], gTestNodes[2]);
        dcTestUtilities_checkIntEqual(array->size, 2);
        dcArray_free(&array, DC_SHALLOW);
    }

    // test dcArray_converge()
    {
        // converge at index 0
        dcArray *to = dcArray_createWithObjects(gTestNodes[1],
                                                gTestNodes[2],
                                                NULL);
        dcArray *from = dcArray_createWithObjects(gTestNodes[3],
                                                  gTestNodes[4],
                                                  NULL);
        dcArray_converge(to, from, 0);
        // check contents here
        dcArray_free(&to, DC_SHALLOW);
        dcArray_free(&from, DC_SHALLOW);

        // converge at index 1
        to = dcArray_createWithObjects(gTestNodes[1], NULL);
        from = dcArray_createWithObjects(gTestNodes[2],
                                         gTestNodes[3],
                                         gTestNodes[4],
                                         NULL);
        dcArray_converge(to, from, 1);
        // check contents here
        dcArray_free(&to, DC_SHALLOW);
        dcArray_free(&from, DC_SHALLOW);
    }

    // test dcArray_unshift()
    {
        dcArray *array = dcArray_createWithObjects(gTestNodes[1], NULL);
        dcArray_unshift(array, gTestNodes[3]);
        dcArray_unshift(array, gTestNodes[2]);
        dcArray_unshift(array, gTestNodes[1]);
        dcTestUtilities_checkIntEqual(array->size, 4);
        dcTestUtilities_checkEqual(array->objects[0], gTestNodes[1]);
        dcTestUtilities_checkEqual(array->objects[1], gTestNodes[2]);
        dcTestUtilities_checkEqual(array->objects[2], gTestNodes[3]);
        dcTestUtilities_checkEqual(array->objects[3], gTestNodes[1]);
        dcArray_free(&array, DC_SHALLOW);
    }

    // test dcArray_shift()
    {
        dcArray *array = dcArray_createWithObjects(gTestNodes[1],
                                                   gTestNodes[2],
                                                   gTestNodes[3],
                                                   NULL);
        dcNode *shiftResult = dcArray_shift(array, 1, DC_SHALLOW);
        dcTestUtilities_checkEqual(shiftResult, gTestNodes[2]);
        dcTestUtilities_checkIntEqual(array->size, 2);
        shiftResult = dcArray_shift(array, 0, DC_SHALLOW);
        dcTestUtilities_checkEqual(shiftResult, gTestNodes[1]);
        dcTestUtilities_checkIntEqual(array->size, 1);
        shiftResult = dcArray_shift(array, 0, DC_SHALLOW);
        dcTestUtilities_checkEqual(shiftResult, gTestNodes[3]);
        dcTestUtilities_checkIntEqual(array->size, 0);
        shiftResult = dcArray_shift(array, 1, DC_SHALLOW);
        dcTestUtilities_checkEqual(shiftResult, NULL);
        dcTestUtilities_checkIntEqual(array->size, 0);
        dcArray_free(&array, DC_DEEP);
    }

    // test dcArray_add()
    {
        dcArray *array = dcArray_createWithObjects(gTestNodes[1], NULL);
        dcArray_add(array, gTestNodes[1]);
        dcArray_add(array, gTestNodes[2]);
        dcArray_add(array, gTestNodes[3]);
        dcTestUtilities_checkIntEqual(array->size, 4);
        dcTestUtilities_checkEqual(array->objects[0], gTestNodes[1]);
        dcTestUtilities_checkEqual(array->objects[1], gTestNodes[1]);
        dcTestUtilities_checkEqual(array->objects[2], gTestNodes[2]);
        dcTestUtilities_checkEqual(array->objects[3], gTestNodes[3]);
        dcArray_free(&array, DC_SHALLOW);
    }

    // test dcArray_addObjects()
    {
        dcArray *array = dcArray_createWithObjects(gTestNodes[1], NULL);
        dcArray_addObjects(array, gTestNodes[1], gTestNodes[2], NULL);
        dcTestUtilities_checkIntEqual(array->size, 3);
        dcTestUtilities_checkEqual(array->objects[0], gTestNodes[1]);
        dcTestUtilities_checkEqual(array->objects[1], gTestNodes[1]);
        dcTestUtilities_checkEqual(array->objects[2], gTestNodes[2]);
        dcArray_free(&array, DC_SHALLOW);
    }

    // test dcArray_copy()
    {
        dcArray *from = dcArray_createWithObjects(gTestNodes[1],
                                                  gTestNodes[2],
                                                  NULL);
        dcArray *shallowCopy = dcArray_copy(from, DC_SHALLOW);
        dcTestUtilities_checkIntEqual(shallowCopy->size, 2);
        dcTestUtilities_checkEqual(shallowCopy->objects[0], gTestNodes[1]);
        dcTestUtilities_checkEqual(shallowCopy->objects[1], gTestNodes[2]);

        dcArray *deepCopy = dcArray_copy(from, DC_DEEP);
        dcTestUtilities_checkIntEqual(deepCopy->size, 2);

        dcTaffyOperator compareResult;
        dcTestUtilities_assert(dcNode_compare(deepCopy->objects[0],
                                              gTestNodes[1],
                                              &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_EQUALS);
        dcTestUtilities_assert(dcNode_compare(shallowCopy->objects[1],
                                              gTestNodes[2],
                                              &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_EQUALS);
        dcArray_free(&from, DC_SHALLOW);
        dcArray_free(&shallowCopy, DC_SHALLOW);
        dcArray_free(&deepCopy, DC_DEEP);
    }

    // test dcArray_mark()
    {
        dcArray *array = dcArray_createWithObjects(gTestNodes[1],
                                                   gTestNodes[2],
                                                   gTestNodes[3],
                                                   NULL);
        dcTestUtilities_doMark(false);
        dcArray_mark(array);
        dcTestUtilities_assert(dcNode_isMarked(gTestNodes[1]));
        dcTestUtilities_assert(dcNode_isMarked(gTestNodes[2]));
        dcTestUtilities_assert(dcNode_isMarked(gTestNodes[3]));
        dcArray_free(&array, DC_SHALLOW);
    }

    // test dcArray_resize()
    {
        dcArray *array = dcArray_createWithObjects(gTestNodes[1],
                                                   gTestNodes[2],
                                                   NULL);
        dcArray_resize(array, 100);
        dcTestUtilities_checkIntEqual(array->capacity, 100);
        dcTestUtilities_checkIntEqual(array->size, 2);
        dcTestUtilities_checkEqual(array->objects[0], gTestNodes[1]);
        dcTestUtilities_checkEqual(array->objects[1], gTestNodes[2]);
        dcArray_free(&array, DC_SHALLOW);
    }

    // test array->size
    {
        dcArray *array = dcArray_createWithObjects(gTestNodes[1],
                                                   gTestNodes[2],
                                                   NULL);
        dcTestUtilities_checkIntEqual(array->size, 2);
        dcArray_free(&array, DC_SHALLOW);
    }

    // test dcArray_sanctifyIndex()
    {
        dcArray *array = dcArray_createWithObjects(gTestNodes[1],
                                                   gTestNodes[2],
                                                   gTestNodes[3],
                                                   NULL);
        dcTestUtilities_checkIntEqual(dcArray_sanctifyIndex(array, 0), 0);
        dcTestUtilities_checkIntEqual(dcArray_sanctifyIndex(array, 1), 1);
        dcTestUtilities_checkIntEqual(dcArray_sanctifyIndex(array, 2), 2);
        dcTestUtilities_checkIntEqual(dcArray_sanctifyIndex(array, 3), 0);
        dcTestUtilities_checkIntEqual(dcArray_sanctifyIndex(array, 4), 1);
        dcTestUtilities_checkIntEqual(dcArray_sanctifyIndex(array, -1), 2);
        dcTestUtilities_checkIntEqual(dcArray_sanctifyIndex(array, -2), 1);
        dcTestUtilities_checkIntEqual(dcArray_sanctifyIndex(array, -3), 0);
        dcTestUtilities_checkIntEqual(dcArray_sanctifyIndex(array, -4), 2);
        dcTestUtilities_checkIntEqual(dcArray_sanctifyIndex(array, -5), 1);
        dcArray_free(&array, DC_SHALLOW);
    }

    // lots of nodes
    {
        uint32_t size = 50000;
        dcArray *array = dcArray_createWithSize(size);
        uint32_t i;

        for (i = 0; i < size; i++)
        {
            dcArray_set(array, dcUnsignedInt32_createNode(i), i);
        }

        for (i = 0; i < size; i++)
        {
            dcError_assert(dcUnsignedInt32_getInt(array->objects[i]) == i);
        }

        dcTestUtilities_assert(array->size == size);
        dcArray_free(&array, DC_DEEP);
    }

    dcTestUtilities_end();
    dcGarbageCollector_free();

    return 0;
}
