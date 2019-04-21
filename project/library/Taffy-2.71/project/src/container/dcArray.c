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
#include <stdarg.h>
#include <string.h>

#include "dcArray.h"
#include "dcGraphData.h"
#include "dcUnsignedInt32.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

// create dcArray_printNode
dcTaffy_createPrintNodeFunctionMacro(dcArray, CAST_ARRAY);

// create dcArray_castMe
dcTaffy_createCastFunctionMacro(dcArray, CAST_ARRAY);

// create dcArray_display
dcTaffy_createDisplayFunctionMacro(dcArray);

// create dcArray_freeNode
dcTaffy_createFreeNodeFunctionMacro(dcArray, CAST_ARRAY);

// create dcArray_copyNode
dcTaffy_createCopyNodeFunctionMacro(dcArray, CAST_ARRAY);

// create dcArray_markNode
dcTaffy_createMarkNodeFunctionMacro(dcArray, CAST_ARRAY);

// create dcArray_registerNode
dcTaffy_createRegisterNodeFunctionMacro(dcArray, CAST_ARRAY);

// create dcArray_unmarshallNode
dcTaffy_createUnmarshallNodeFunctionMacro(dcArray, CAST_ARRAY);

// create dcArray_marshallNode
dcTaffy_createMarshallNodeFunctionMacro(dcArray, CAST_ARRAY);

dcArray *dcArray_createWithSize(dcContainerSizeType _capacity)
{
    dcArray *array = (dcArray *)dcMemory_allocate(sizeof(dcArray));

    if (_capacity > 0)
    {
        // allocate the objects //
        array->objects = (dcNode **)(dcMemory_allocateAndInitialize
                                     (sizeof(dcNode *) * _capacity));
    }
    else
    {
        // capacity is zero, so don't waste space on objects //
        array->objects = NULL;
    }

    array->capacity = _capacity;
    array->size = 0;
    return array;
}

dcNode *dcArray_createNodeWithCapacity(dcContainerSizeType _capacity)
{
    return dcNode_createWithGuts(NODE_ARRAY,
                                 dcArray_createWithSize(_capacity));
}

dcNode *dcArray_createNodeWithObjects(dcNode *_first, ...)
{
    va_list vl;
    va_start(vl, _first);

    dcNode *node = dcNode_create(NODE_ARRAY);

    // isn't 5 an awesome number? //
    dcArray *array = dcArray_createWithSize(5);
    CAST_ARRAY(node) = array;
    dcArray_add(array, _first);

    dcNode *iteratorNode = va_arg(vl, dcNode*);

    while (iteratorNode != NULL)
    {
        dcArray_add(array, iteratorNode);
        iteratorNode = va_arg(vl, dcNode*);
    }

    va_end(vl);
    return node;
}

dcArray *dcArray_createWithObjects(dcNode *_first, ...)
{
    va_list vl;
    va_start(vl, _first);

    // guess at the length //
    dcArray *array = dcArray_createWithSize(5);
    dcNode *iterator = va_arg(vl, dcNode*);

    if (_first)
    {
        dcArray_add(array, _first);

        while (iterator != NULL)
        {
            dcArray_add(array, iterator);
            iterator = va_arg(vl, dcNode*);
        }
    }

    va_end(vl);
    return array;
}

dcArray *dcArray_createFromList(const dcList *_list, dcDepth _depth)
{
    dcArray *result = dcArray_createWithSize(_list->size);
    dcListElement *listIterator = _list->head;
    dcContainerSizeType arrayIterator = 0;

    while (listIterator != NULL)
    {
        result->size++;

        if (_depth == DC_SHALLOW)
        {
            result->objects[arrayIterator] = listIterator->object;
        }
        else
        {
            result->objects[arrayIterator] =
                dcNode_copy(listIterator->object, _depth);
        }

        arrayIterator++;
        listIterator = listIterator->next;
    }

    return result;
}

dcNode *dcArray_createNodeFromList(const dcList *_list, dcDepth _depth)
{
    dcNode *result = dcNode_create(NODE_ARRAY);
    CAST_ARRAY(result) = dcArray_createFromList(_list, _depth);
    return result;
}

dcNode *dcArray_createShell(dcArray *_array)
{
    dcNode *result = dcNode_create(NODE_ARRAY);
    CAST_ARRAY(result) = _array;
    return result;
}

void dcArray_free(dcArray **_array, dcDepth _depth)
{
    dcArray *array = *_array;

    if (array != NULL)
    {
        dcArray_clear(array, _depth);
        dcMemory_free(array->objects);
    }

    dcMemory_free(*_array);
}

void dcArray_clear(dcArray *_array, dcDepth _depth)
{
    dcContainerSizeType i;
    dcContainerSizeType freed;

    for (i = 0, freed = 0;
         i < _array->capacity && freed < _array->size;
         i++)
    {
        if (_depth != DC_FLOATING)
        {
            if (_array->objects[i] != NULL)
            {
                dcNode_tryFree(&(_array->objects[i]), _depth);
                freed++;
            }
        }

        _array->objects[i] = NULL;
    }

    _array->size = 0;
}

bool dcArray_removeObject(dcArray *_array, dcNode *_object, dcDepth _depth)
{
    bool foundObject = false;
    dcContainerSizeType i = 0;

    for (i = 0; i < _array->size; i++)
    {
        if (_array->objects[i] == _object)
        {
            dcNode_tryFree(&(_array->objects[i]), _depth);
            foundObject = true;
            _array->objects[i] = NULL;
        }
    }

    _array->size--;
    return foundObject;
}

void dcArray_autoResize(dcArray *_array)
{
    while (_array->size >= _array->capacity)
    {
        dcArray_resize(_array,
                       (_array->capacity > 0
                        ? _array->capacity * 2
                        : 2));
    }
}

void dcArray_resize(dcArray *_array, dcContainerSizeType _newCapacity)
{
    if (_newCapacity > _array->capacity)
    {
        _array->objects =
            (dcNode **)(dcMemory_realloc
                        (_array->objects, sizeof(dcNode *) * _newCapacity));
        memset(&(_array->objects[_array->capacity]),
               0,
               (_newCapacity - _array->capacity) * sizeof(dcNode *));
        _array->capacity = _newCapacity;
    }
}

void dcArray_converge(dcArray *_to,
                      const dcArray *_from,
                      dcContainerSizeType _startingIndex)
{
    dcContainerSizeType toIndex = 0;
    dcContainerSizeType fromIndex = 0;

    if (_startingIndex + _from->size >= _to->size)
    {
        dcArray_resize(_to, (_startingIndex + _from->size) * 2);
    }

    for (fromIndex = 0, toIndex = _startingIndex;
         fromIndex < _from->size;
         toIndex++, fromIndex++)
    {
        dcArray_set(_to, _from->objects[fromIndex], toIndex);
    }

    _to->size = toIndex;
}

void dcArray_add(dcArray *_array, dcNode *_node)
{
    assert(_node != NULL);
    dcArray_autoResize(_array);
    dcArray_set(_array, _node, _array->size);
}

void dcArray_set(dcArray *_array, dcNode *_node, dcContainerSizeType _index)
{
    assert(_node != NULL);
    dcArray_autoResize(_array);
    assert(_index < _array->capacity);

    if (_array->objects[_index] == NULL)
    {
        _array->size++;
    }

    _array->objects[_index] = _node;
}

void dcArray_addObjects(dcArray *_array, dcNode *_first, ...)
{
    va_list vl;
    va_start(vl, _first);
    dcNode *iterator = _first;

    while (iterator != NULL)
    {
        dcArray_add(_array, iterator);
        iterator = va_arg(vl, dcNode*);
    }

    va_end(vl);
}

void dcArray_unshift(dcArray *_array, dcNode *_node)
{
    assert(_node != NULL);
    dcArray_autoResize(_array);
    dcContainerSizeType i;

    for (i = _array->size; i >= 1; i--)
    {
        _array->objects[i] = _array->objects[i - 1];
    }

    _array->objects[0] = NULL;
    dcArray_set(_array, _node, 0);
}

void dcArray_unshiftAtIndex(dcArray *_array,
                            dcNode *_node,
                            dcContainerSizeType _index)
{
    dcContainerSizeType i = 0;

    assert(_index < _array->size);
    dcContainerSizeType start = _array->size;
    _array->size++;
    dcArray_autoResize(_array);

    for (i = start; i > _index; i--)
    {
        _array->objects[i] = _array->objects[i - 1];
    }

    dcArray_set(_array, _node, _index);
}

dcNode *dcArray_shift(dcArray *_array,
                      dcContainerSizeType _index,
                      dcDepth _depth)
{
    dcNode *result = NULL;

    if (_array->size > 0)
    {
        dcContainerSizeType i = 0;
        result = _array->objects[_index];

        for (i = _index; i < _array->size - 1; i++)
        {
            _array->objects[i] = _array->objects[i + 1];
        }

        dcNode_tryFree(&result, _depth);
        _array->size--;
    }

    return result;
}

dcNode *dcArray_get(const dcArray *_array, dcContainerSizeType _index)
{
    return _array->objects[_index];
}

dcContainerSizeType dcArray_getSize(const dcNode *_array)
{
    return CAST_ARRAY(_array)->size;
}

dcContainerSizeType dcArray_getCapacity(const dcNode *_array)
{
    return CAST_ARRAY(_array)->capacity;
}

dcNode **dcArray_getObjects(const dcNode *_array)
{
    return CAST_ARRAY(_array)->objects;
}

dcArray *dcArray_copy(const dcArray *_from, dcDepth _depth)
{
    dcArray *to = dcArray_createWithSize(_from->capacity);
    dcContainerSizeType i = 0;

    for (i = 0; i < _from->size; i++)
    {
        dcArray_set(to, dcNode_tryCopy(_from->objects[i], _depth), i);
    }

    return to;
}

dcContainerSizeType dcArray_sanctifyIndex(const dcArray *_array, int _index)
{
    int result = _index;

    if ((_array->size == 0 || _array->size == 1) && _index < 0)
    {
        result = 0;
    }
    else if (_index != 0 && _array->size > 0)
    {
        if (_index < 0)
        {
            if (_index < (signed int)_array->size * -1)
            {
                result += (_array->size
                           * (abs(_index) / (signed int)_array->size));
            }

            result = _array->size + result;
        }
        else
        {
            result = _index % _array->size;
        }
    }

    return (dcContainerSizeType)result;
}

dcNode *dcArray_pop(dcArray *_array, dcDepth _depth)
{
    dcNode *result = NULL;

    if (_array->size > 0)
    {
        #ifdef DC_DEBUG
        {
            if (!_array->objects[_array->size - 1])
            {
                dcError_internal("dcArray_pop::NULL");
            }
        }
        #endif

        dcContainerSizeType popPosition = _array->size - 1;

        if (_array->objects[popPosition] != NULL)
        {
            result = dcNode_tryFree(&_array->objects[popPosition], _depth);
            _array->objects[_array->size - 1] = NULL;
            (_array->size)--;
        }
    }

    return result;
}

dcResult dcArray_print(const dcArray *_array, dcString *_string)
{
    dcContainerSizeType i = 0;
    dcResult result = TAFFY_SUCCESS;

    for (i = 0; i < _array->size && result == TAFFY_SUCCESS; i++)
    {
        result = dcNode_print(_array->objects[i], _string);

        if (i < _array->size - 1)
        {
            dcString_appendCharacter(_string, ',');
        }
    }

    return result;
}

void dcArray_mark(dcArray *_array)
{
    dcContainerSizeType i = 0;

    for (i = 0; i < _array->size; i++)
    {
        dcNode_mark(_array->objects[i]);
    }
}

void dcArray_register(dcArray *_array)
{
    dcContainerSizeType i = 0;

    for (i = 0; i < _array->size; i++)
    {
        dcNode_register(_array->objects[i]);
    }
}

dcArray *dcArray_unmarshall(dcString *_stream)
{
    dcArray *result = NULL;
    uint32_t arraySize;
    int type;

    if (dcString_getLengthLeft(_stream) > 1
        && dcString_peek(_stream) != 0xFF
        && dcMarshaller_unmarshall(_stream, "ci", &type, &arraySize)
        // do a basic size check
        && dcString_hasLengthLeft(_stream, ((uint64_t)arraySize
                                            * MIN_MARSHALL_SIZE))
        && type == NODE_ARRAY)
    {
        uint32_t i = 0;
        result = dcArray_createWithSize(arraySize);

        for (i = 0; i < arraySize; i++)
        {
            dcNode *node = dcNode_unmarshall(_stream);

            if (node != NULL)
            {
                dcArray_add(result, node);
            }
            else
            {
                dcArray_free(&result, DC_DEEP);
                result = NULL;
                break;
            }
        }
    }

    return result;
}

dcString *dcArray_marshall(const dcArray *_array, dcString *_stream)
{
    dcString *result = NULL;

    if (_array == NULL)
    {
        result = dcMarshaller_marshall(_stream, "N");
    }
    else
    {
        result = dcMarshaller_marshall(_stream,
                                       "ci",
                                       NODE_ARRAY,
                                       _array->size);
        uint32_t i = 0;

        for (i = 0; i < _array->size; i++)
        {
            dcNode_marshall(dcArray_get(_array, i), result);
        }
    }

    return result;
}

void dcArray_setTemplate(dcArray *_array, bool _yesNo)
{
    uint32_t i;

    for (i = 0; i < _array->size; i++)
    {
        dcNode_setTemplate(_array->objects[i], _yesNo);
    }
}

dcResult dcArray_compare(dcArray *_left,
                         dcArray *_right,
                         dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_SUCCESS;

    if (_left->size == _right->size)
    {
        uint32_t i;

        for (i = 0; i < _left->size; i++)
        {
            dcResult myResult = dcNode_compare
                (_left->objects[i], _right->objects[i], _compareResult);

            if (myResult != TAFFY_SUCCESS
                || *_compareResult != TAFFY_EQUALS)
            {
                result = myResult;
                break;
            }
        }
    }
    else
    {
        *_compareResult = (_left->size < _right->size
                           ? TAFFY_LESS_THAN
                           : TAFFY_GREATER_THAN);
    }

    return result;
}
