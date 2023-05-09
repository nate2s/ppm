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
#include "dcUnsignedInt32.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMatrix.h"
#include "dcMemory.h"
#include "dcNil.h"
#include "dcNode.h"
#include "dcNumberClass.h"
#include "dcString.h"
#include "dcSystem.h"

dcMatrix *dcMatrix_create(dcArray *_objects,
                          uint32_t _rowCount,
                          uint32_t _columnCount)
{
    dcMatrix *matrix = (dcMatrix *)dcMemory_allocate(sizeof(dcMatrix));
    matrix->objects = _objects;
    matrix->rowCount = _rowCount;
    matrix->columnCount = _columnCount;
    return matrix;
}

dcMatrix *dcMatrix_createBlank(uint32_t _rowCount, uint32_t _columnCount)
{
    return dcMatrix_create(dcArray_createWithSize(_rowCount * _columnCount),
                           _rowCount,
                           _columnCount);
}

dcMatrix *dcMatrix_createFromLists(dcList *_objects)
{
    //
    // <verify> that the rows are formed correctly
    //
    dcListElement *that;
    uint32_t columnCount = 0;

    for (that = _objects->head; that != NULL; that = that->next)
    {
        if (columnCount == 0)
        {
            columnCount = dcList_getSize(that->object);
        }
        else if (columnCount != dcList_getSize(that->object))
        {
            return NULL;
        }
    }
    //
    // </verify>
    //

    dcArray *objects =
        dcArray_createWithSize(_objects->size * columnCount);
    dcListIterator *rowIt = dcList_createHeadIterator(_objects);
    dcNode *listNode = NULL;

    while ((listNode = dcListIterator_getNext(rowIt))
           != NULL)
    {
        dcListIterator *columnIt =
            dcList_createHeadIterator(CAST_LIST(listNode));
        dcNode *node = NULL;
        uint32_t i = 0;

        while ((node = dcListIterator_getNext(columnIt))
               != NULL)
        {
            dcArray_add(objects, node);
            i++;
        }

        for (; i < columnCount; i++)
        {
            dcArray_add(objects, dcNil_createNode());
        }

        dcListIterator_free(&columnIt);
    }

    dcListIterator_free(&rowIt);
    return dcMatrix_create(objects, _objects->size, columnCount);
}

dcMatrix *dcMatrix_createEye(uint32_t _rowCount, uint32_t _columnCount)
{
    dcMatrix *matrix = dcMatrix_createBlank(_rowCount, _columnCount);
    uint32_t rowIt = 0;
    uint32_t columnIt = 0;

    for (rowIt = 0; rowIt < _rowCount; rowIt++)
    {
        for (columnIt = 0; columnIt < _columnCount; columnIt++)
        {
            if (rowIt == columnIt)
            {
                dcMatrix_set(matrix,
                             dcNumberClass_getOneNumberObject(),
                             rowIt,
                             columnIt);
            }
            else
            {
                dcMatrix_set(matrix,
                             dcNumberClass_getZeroNumberObject(),
                             rowIt,
                             columnIt);
            }
        }
    }

    return matrix;
}

dcMatrix *dcMatrix_copy(const dcMatrix *_from, dcDepth _depth)
{
    return dcMatrix_create(dcArray_copy(_from->objects, _depth),
                           _from->rowCount,
                           _from->columnCount);
}

void dcMatrix_free(dcMatrix **_matrix, dcDepth _depth)
{
    if (_matrix != NULL)
    {
        dcMatrix *matrix = *_matrix;
        dcArray_free(&matrix->objects, _depth);
        dcMemory_free(*_matrix);
    }
}

void dcMatrix_freeNode(dcNode *_node, dcDepth _depth)
{
    dcMatrix_free(&CAST_MATRIX(_node), _depth);
}

void dcMatrix_clear(dcMatrix *_matrix, dcDepth _depth)
{
    dcArray_clear(_matrix->objects, _depth);
}

void dcMatrix_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_MATRIX(_to) = dcMatrix_copy(CAST_MATRIX(_from), _depth);
}

uint32_t dcMatrix_getSize(const dcMatrix *_matrix)
{
    return _matrix->objects->size;
}

static int getIndex(const dcMatrix *_matrix, int _row, int _column)
{
    return ((_row * _matrix->columnCount) + _column);
}

dcNode *dcMatrix_get(const dcMatrix *_matrix, uint32_t _row, uint32_t _column)
{
    uint32_t arrayIndex = getIndex(_matrix, _row, _column);
    return (arrayIndex < _matrix->objects->size
            ? dcArray_get(_matrix->objects, arrayIndex)
            : NULL);
}

void dcMatrix_set(const dcMatrix *_matrix,
                  dcNode *_node,
                  uint32_t _row,
                  uint32_t _column)
{
    dcArray_set(_matrix->objects,
                _node,
                getIndex(_matrix, _row, _column));
}

// create dcMatrix_unmarshallNode //
dcTaffy_createUnmarshallNodeFunctionMacro(dcMatrix, CAST_MATRIX);

// create dcMatrix_marshallNode //
dcTaffy_createMarshallNodeFunctionMacro(dcMatrix, CAST_MATRIX);

dcMatrix *dcMatrix_unmarshall(dcString *_stream)
{
    dcMatrix *result = NULL;
    uint32_t rowCount = 0;
    uint32_t columnCount = 0;
    uint32_t type;
    dcArray *array = NULL;

    if (dcMarshaller_unmarshallNoNull(_stream, "c", &type)
        && type == NODE_MATRIX
        && dcMarshaller_unmarshallNoNull(_stream, "iia",
                                         &rowCount,
                                         &columnCount,
                                         &array))
    {
        result = dcMatrix_create(array, rowCount, columnCount);
    }

    return result;
}

dcString *dcMatrix_marshall(const dcMatrix *_matrix, dcString *_stream)
{
    return dcMarshaller_marshall(_stream,
                                 "ciia",
                                 NODE_MATRIX,
                                 _matrix->rowCount,
                                 _matrix->columnCount,
                                 _matrix->objects);
}

void dcMatrix_mark(dcMatrix *_matrix)
{
    dcArray_mark(_matrix->objects);
}

void dcMatrix_assertIsTemplate(dcMatrix *_matrix)
{
    uint32_t i;

    for (i = 0; i < _matrix->objects->size; i++)
    {
        dcError_assert(dcNode_isTemplate(_matrix->objects->objects[i]));
    }
}

void dcMatrix_setTemplate(dcMatrix *_matrix, bool _yesNo)
{
    dcArray_setTemplate(_matrix->objects, _yesNo);
}

dcMatrix *dcMatrix_transpose(dcMatrix *_matrix)
{
    // if rowCount == 1 || columnCount == 1 then we just need to swap them later
    if (_matrix->rowCount != 1
        && _matrix->columnCount != 1)
    {
        //
        // TODO: speed this up
        //
        uint32_t end = _matrix->rowCount * _matrix->columnCount;
        dcArray *newArray = dcArray_createWithSize(end);
        newArray->size = newArray->capacity;

        // iterator for the column
        uint32_t c = 0;

        // storage location for newArray
        uint32_t j = 0;

        for (c = 0; c < _matrix->columnCount; c++)
        {
            // plucker from _matrix->objects
            uint32_t i = 0;

            for (i = c; i < end; i += _matrix->columnCount, j++)
            {
                newArray->objects[j] = _matrix->objects->objects[i];
            }
        }

        dcArray_free(&_matrix->objects, DC_SHALLOW);
        _matrix->objects = newArray;
    }

    // swap rows and columns
    uint32_t temp = _matrix->rowCount;
    _matrix->rowCount = _matrix->columnCount;
    _matrix->columnCount = temp;

    return _matrix;
}
