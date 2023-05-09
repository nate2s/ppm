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

#include <math.h>

#include "dcArray.h"
#include "dcError.h"
#include "dcHeap.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

// for runtime code
#include "dcNodeEvaluator.h"
#include "dcSystem.h"

dcNode *dcHeap_createNode(dcHeapType _type)
{
    return dcNode_createWithGuts(NODE_HEAP, dcHeap_create(_type));
}

dcNode *dcHeap_createShell(dcHeap *_heap)
{
    return dcNode_createWithGuts(NODE_HEAP, _heap);
}

dcHeap *dcHeap_create(dcHeapType _type)
{
    return dcHeap_createWithCapacity(_type, DEFAULT_HEAP_SIZE);
}

dcHeap *dcHeap_createWithCapacity(dcHeapType _type,
                                  dcContainerSizeType _capacity)
{
    dcHeap *heap = (dcHeap *)dcMemory_allocate(sizeof(dcHeap));
    heap->objects = dcArray_createWithSize(_capacity);
    heap->type = _type;
    return heap;
}

dcHeap *dcHeap_createFromArray(dcHeapType _type, dcArray *_objects)
{
    dcHeap *heap = (dcHeap *)dcMemory_allocate(sizeof(dcHeap));
    heap->objects = _objects;
    heap->type = _type;
    return heap;
}

void dcHeap_free(dcHeap **_heap, dcDepth _depth)
{
    dcArray_free(&(*_heap)->objects, _depth);
    dcMemory_free(*_heap);
}

void dcHeap_clear(dcHeap *_heap, dcDepth _depth)
{
    dcArray_clear(_heap->objects, _depth);
}

dcHeap *dcHeap_copy(const dcHeap *_from, dcDepth _depth)
{
    return dcHeap_createFromArray(_from->type,
                                  dcArray_copy(_from->objects, _depth));
}

dcResult dcHeap_compare(const dcHeap *_left,
                        const dcHeap *_right,
                        dcTaffyOperator *_operatorResult)
{
    dcResult result = TAFFY_SUCCESS;

    if (_left->objects->size == _right->objects->size)
    {
        uint32_t i;
        result = TAFFY_SUCCESS;

        for (i = 0; i < _left->objects->size; i++)
        {
            result = dcNode_compare(_left->objects->objects[i],
                                    _right->objects->objects[i],
                                    _operatorResult);

            if (result != TAFFY_SUCCESS
                || *_operatorResult != TAFFY_EQUALS)
            {
                break;
            }
        }
    }
    else if (_left->objects->size < _right->objects->size)
    {
        *_operatorResult = TAFFY_LESS_THAN;
    }
    else
    {
        *_operatorResult = TAFFY_GREATER_THAN;
    }

    return result;
}

#define PARENT_OF(index) (int)floor((double)(int)((index - 1) / 2))

dcResult dcHeap_insert(dcHeap *_heap, dcNode *_object)
{
    if (_heap->objects->size + 1 >= _heap->objects->capacity)
    {
        dcArray_autoResize(_heap->objects);
    }

    dcResult result = TAFFY_SUCCESS;
    ssize_t position = _heap->objects->size;
    ssize_t parent = PARENT_OF(position);
    dcTaffyOperator comparison = (_heap->type == HEAP_MIN
                                  ? TAFFY_GREATER_THAN
                                  : TAFFY_LESS_THAN);

    _heap->objects->size++;
    _heap->objects->objects[position] = _object;

    while (position > 0)
    {
        dcTaffyOperator compareResult;
        dcResult myResult = dcNode_compare(_heap->objects->objects[parent],
                                           _heap->objects->objects[position],
                                           &compareResult);

        if (myResult == TAFFY_SUCCESS && comparison == compareResult)
        {
            dcError_assert(parent >= 0
                           && position >= 0
                           && parent < (ssize_t)_heap->objects->capacity
                           && (position
                               < (ssize_t)_heap->objects->capacity));

            // we can keep going, flip the child and the parent
            dcNode *temp = _heap->objects->objects[parent];
            _heap->objects->objects[parent] = _heap->objects->objects[position];
            _heap->objects->objects[position] = temp;
            position = PARENT_OF(position);
            parent = PARENT_OF(parent);
        }
        else
        {
            // myResult == TAFFY_EXCEPTION or comparison != compareResult
            // either way, we're done
            result = myResult;
            break;
        }
    }

    TAFFY_DEBUG(dcError_assert(parent < (ssize_t)_heap->objects->capacity
                               && (position
                                   < (ssize_t)_heap->objects->capacity)));

    if (result != TAFFY_SUCCESS)
    {
        _heap->objects->size--;
    }

    return result;
}

dcResult dcHeap_pop(dcHeap *_heap, dcNode **_popResult)
{
    dcTaffyOperator comparison = (_heap->type == HEAP_MIN
                                  ? TAFFY_LESS_THAN
                                  : TAFFY_GREATER_THAN);
    *_popResult = NULL;
    dcResult result = TAFFY_FAILURE;
    dcNodeEvaluator *evaluator = (dcSystem_isLive()
                                  ? dcSystem_getCurrentNodeEvaluator()
                                  : NULL);
    uint32_t marks = 0;

    if (_heap->objects->size > 0)
    {
        result = TAFFY_SUCCESS;
        size_t swapIndex = 0;
        *_popResult = _heap->objects->objects[swapIndex];

        if (evaluator != NULL)
        {
            marks += dcNodeEvaluator_pushMark(evaluator, *_popResult);
        }

        if (_heap->objects->size > 1)
        {
            dcNode *swap = _heap->objects->objects[_heap->objects->size - 1];
            size_t childIndex = 1;

            _heap->objects->objects[swapIndex] = swap;

            while (childIndex < _heap->objects->size)
            {
                size_t childIndexToUse = 0;
                bool rightCompare = false;
                bool leftCompare = false;
                dcTaffyOperator compareResult;

                // try the left //
                dcResult myResult = dcNode_compare
                    (_heap->objects->objects[childIndex],
                     _heap->objects->objects[swapIndex],
                     &compareResult);

                if (myResult == TAFFY_EXCEPTION)
                {
                    result = myResult;
                    break;
                }
                else if (myResult == TAFFY_SUCCESS
                         && compareResult == comparison)
                {
                    leftCompare = true;
                    childIndexToUse = childIndex;
                }

                // try the right, if we can //
                if (childIndex + 1 < _heap->objects->size)
                {
                    myResult = dcNode_compare
                        (_heap->objects->objects[childIndex + 1],
                         _heap->objects->objects[swapIndex],
                         &compareResult);

                    if (myResult == TAFFY_EXCEPTION)
                    {
                        result = myResult;
                        break;
                    }
                    else if (myResult == TAFFY_SUCCESS
                             && compareResult == comparison)
                    {
                        rightCompare = true;
                    }
                }

                if (!leftCompare && rightCompare)
                {
                    // go right
                    childIndexToUse = childIndex + 1;
                }
                else if (leftCompare && rightCompare)
                {
                    dcTaffyOperator thisComparison =
                        (comparison == TAFFY_GREATER_THAN
                         ? TAFFY_LESS_THAN
                         : TAFFY_GREATER_THAN);

                    myResult = dcNode_compare
                        (_heap->objects->objects[childIndex],
                         _heap->objects->objects[childIndex + 1],
                         &compareResult);

                    if (myResult == TAFFY_EXCEPTION)
                    {
                        result = myResult;
                        break;
                    }
                    else if (myResult == TAFFY_SUCCESS
                             && compareResult == thisComparison)
                    {
                        // go right
                        childIndexToUse = childIndex + 1;
                    }
                }

                // if we need to swap again //
                if (childIndexToUse > 0)
                {
                    childIndex = childIndexToUse;
                    dcNode *temp = _heap->objects->objects[childIndex];
                    _heap->objects->objects[childIndex] =
                        _heap->objects->objects[swapIndex];
                    _heap->objects->objects[swapIndex] = temp;
                    swapIndex = childIndex;
                    childIndex = childIndex * 2 + 1;
                }
                else
                {
                    break;
                }
            }
        }

        _heap->objects->size--;
    }

    if (evaluator != NULL)
    {
        dcNodeEvaluator_popMarks(evaluator, marks);
    }

    return result;
}

dcNode *dcHeap_get(const dcHeap *_heap, dcContainerSizeType _index)
{
    dcError_assert(_index < _heap->objects->size);
    return _heap->objects->objects[_index];
}

// for debugging
dcResult dcHeap_verify(const dcHeap *_heap)
{
    dcResult result = TAFFY_SUCCESS;
    const dcArray *objects = _heap->objects;
    uint32_t i = 0;
    uint32_t size = objects->size;

    for (i = 0; i < size; i++)
    {
        dcNode *node = dcArray_get(objects, i);
        uint32_t childIndexes[2] = {i * 2 + 1, i * 2 + 2};
        uint32_t j;

        for (j = 0; j < dcTaffy_countOf(childIndexes); j++)
        {
            uint32_t childIndex = childIndexes[j];

            if (childIndex < size)
            {
                dcTaffyOperator compareResult;
                // compare with a child
                dcResult myResult = dcNode_compare
                    (node, dcArray_get(objects, childIndex), &compareResult);

                if (myResult == TAFFY_EXCEPTION
                    || ! ((compareResult == TAFFY_GREATER_THAN
                           && _heap->type == HEAP_MAX)
                          || (compareResult == TAFFY_LESS_THAN
                              && _heap->type == HEAP_MIN)))
                {
                    result = myResult;
                    break;
                }
            }
        }

        if (result != TAFFY_SUCCESS)
        {
            break;
        }
    }

    return result;
}

// create dcHeap_freeNode
dcTaffy_createFreeNodeFunctionMacro(dcHeap, CAST_HEAP);

// create dcHeap_markNode
dcTaffy_createMarkNodeFunctionMacro(dcHeap, CAST_HEAP);

// create dcHeap_registerNode
dcTaffy_createRegisterNodeFunctionMacro(dcHeap, CAST_HEAP);

void dcHeap_mark(dcHeap *_heap)
{
    dcArray_mark(_heap->objects);
}

void dcHeap_register(dcHeap *_heap)
{
    dcArray_register(_heap->objects);
}
