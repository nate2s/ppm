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

#include "dcContainers.h"
#include "dcError.h"
#include "dcGraphData.h"
#include "dcUnsignedInt32.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcMethodCall.h"
#include "dcNode.h"
#include "dcString.h"

///////////////////
// dcListElement //
///////////////////

#ifdef LIST_DEBUG
static void assertSanity(dcList *_list)
{
    if (_list->size == 1)
    {
        dcError_assert(_list->head == _list->tail);
    }

    dcListElement *that;

    for (that = _list->head; that != NULL; that = that->next)
    {
        dcError_assert(that->object != NULL);

        if (that->next == NULL)
        {
            dcError_assert(that == _list->tail);
        }
    }
}
#endif

dcListElement *dcListElement_create(dcNode *_what)
{
    dcListElement *element = (dcListElement *)(dcMemory_allocate
                                               (sizeof(dcListElement)));
    element->object = _what;
    element->next = NULL;
    element->previous = NULL;
    return element;
}

dcNode *dcListElement_free(dcListElement **_element,
                           dcList *_list,
                           dcDepth _depth)
{
    // result will point to (*_element)->object if it's not freed //
    dcNode *result = NULL;

    if (*_element != NULL)
    {
        if (*_element == _list->head)
        {
            _list->head = _list->head->next;
        }

        if (*_element == _list->tail)
        {
            _list->tail = _list->tail->previous;
        }

        dcListElement *next = (*_element)->next;

        if (_depth == DC_FLOATING)
        {
            result = (*_element)->object;
        }
        else
        {
            result = dcNode_tryFree(&((*_element)->object), _depth);
        }

        // take the element out //
        if ((*_element)->next != NULL)
        {
            (*_element)->next->previous = (*_element)->previous;
        }

        if ((*_element)->previous != NULL)
        {
            (*_element)->previous->next = (*_element)->next;
        }

        if (_list->size > 0)
        {
            _list->size--;
        }
        else
        {
            // the list is empty, decimate the head and tail //
            _list->head = _list->tail = NULL;
        }

        TAFFY_DEBUG(if (_list->size == 0)
                    {
                        dcError_assert(_list->head == NULL
                                       && _list->tail == NULL);
                    });

        if (_depth != DC_SPACE)
        {
            dcMemory_free(*_element);
        }

        *_element = next;
    }

#ifdef LIST_DEBUG
    assertSanity(_list);
#endif

    return result;
}

dcListElement *dcListElement_copy(const dcListElement *_from, dcDepth _depth)
{
    dcNode *object = dcNode_tryCopy(_from->object, _depth);
    dcListElement *to = dcListElement_create(object);
    to->next = to->previous = NULL;
    return to;
}

void dcListElement_setObject(dcListElement *_element, dcNode *_object)
{
    _element->object = _object;
}

void dcListElement_replaceObject(dcListElement *_element,
                                 dcNode *_object,
                                 dcDepth _depth)
{
    dcNode_tryFree(&_element->object, _depth);
    _element->object = _object;
}

dcListElement *dcListElement_getNext(const dcListElement *_element)
{
    return _element->next;
}

////////////////////
// dcListIterator //
////////////////////

dcListIterator *dcListIterator_create(dcListElement *_element)
{
    dcListIterator *iterator =
        (dcListIterator *)dcMemory_allocate(sizeof(dcListIterator));
    iterator->element = _element;
    return iterator;
}

void dcListIterator_free(dcListIterator **_iterator)
{
    dcMemory_free((*_iterator));
}

dcNode *dcListIterator_getNext(dcListIterator *_iterator)
{
    dcNode *result = NULL;

    // check if we have good data to work with //
    if (_iterator != NULL && _iterator->element != NULL)
    {
        // set the result to the current element //
        result = _iterator->element->object;

        // increment the iterator //
        _iterator->element = _iterator->element->next;
    }

    return result;
}

dcNode *dcListIterator_getPrevious(dcListIterator *_iterator)
{
    dcNode *result = NULL;

    // check if we have good data to work with //
    if (_iterator->element != NULL)
    {
        // set the result to the current element //
        result = _iterator->element->object;

        // decrement the iterator //
        _iterator->element = _iterator->element->previous;
    }

    return result;
}

bool dcListIterator_hasNext(dcListIterator *_iterator)
{
    return (_iterator->element != NULL
            ? true
            : false);
}

bool dcListIterator_hasPrevious(dcListIterator *_iterator)
{
    // really?
    return (_iterator->element != NULL
            ? true
            : false);
}

////////////
// dcList //
////////////

dcList *dcList_create(void)
{
    return (dcList *)dcMemory_allocateAndInitialize(sizeof(dcList));
}

dcNode *dcList_createNode(void)
{
    return dcNode_createWithGuts(NODE_LIST, dcList_create());
}

dcNode *dcList_createGraphDataNode(void)
{
    return dcGraphData_createNodeWithGuts(NODE_GRAPH_DATA_LIST,
                                          dcList_create());
}

void dcList_markGraphDataNode(dcNode *_node)
{
    dcList_mark(CAST_GRAPH_DATA_LIST(_node));
}

dcList *dcList_createWithObjects(dcNode *_first, ...)
{
    va_list vaList;
    va_start(vaList, _first);

    dcList *list = dcList_create();
    dcNode *iterator = va_arg(vaList, dcNode*);

    if (_first != NULL)
    {
        dcList_push(list, _first);

        while (iterator != NULL)
        {
            dcList_push(list, iterator);
            iterator = va_arg(vaList, dcNode*);
        }
    }

    va_end(vaList);
    return list;
}

dcNode *dcList_createNodeWithObjects(dcNode *_first, ...)
{
    va_list vl;
    va_start(vl, _first);

    dcNode *node = dcList_createNode();
    dcList *list = CAST_LIST(node);
    dcNode *iterator = va_arg(vl, dcNode*);

    if (_first != NULL)
    {
        dcList_push(list, _first);

        while (iterator != NULL)
        {
            dcList_push(list, iterator);
            iterator = va_arg(vl, dcNode*);
        }
    }

    va_end(vl);
    return node;
}

dcNode *dcList_createGraphDataNodeWithObjects(dcNode *_first, ...)
{
    va_list vl;
    va_start(vl, _first);

    dcNode *node = dcList_createGraphDataNode();
    dcList *list = CAST_GRAPH_DATA_LIST(node);
    dcNode *iterator = va_arg(vl, dcNode*);

    if (_first != NULL)
    {
        dcList_push(list, _first);

        while (iterator != NULL)
        {
            dcList_push(list, iterator);
            iterator = va_arg(vl, dcNode*);
        }
    }

    va_end(vl);
    return node;
}

dcNode *dcList_createShell(dcList *_list)
{
    dcNode *listShell = dcNode_create(NODE_LIST);
    CAST_LIST(listShell) = _list;
    return listShell;
}

void dcList_free(dcList **_list, dcDepth _depth)
{
    if (_list != NULL && *_list != NULL)
    {
        dcList_clear(*_list, _depth);
        dcMemory_free(*_list);
    }
}

void dcList_memoryRegionsFree(dcList *_list)
{
    _list->head = NULL;
    _list->tail = NULL;
    _list->size = 0;
}

void dcList_freeGraphDataNode(dcNode *_node, dcDepth _depth)
{
    dcList_free(&(CAST_GRAPH_DATA_LIST(_node)), _depth);
}

void dcList_clear(dcList *_list, dcDepth _depth)
{
    if (_list != NULL)
    {
        dcListElement *iterator = _list->head;
        dcListElement *next = NULL;

        while (iterator != NULL)
        {
            next = iterator->next;
            dcListElement_free(&iterator, _list, _depth);
            iterator = next;
        }

        _list->head = _list->tail = NULL;
        _list->size = 0;
    }
}

void dcList_setHead(dcList *_list, dcNode *_node)
{
    if (_list->head != NULL)
    {
        dcListElement_setObject(_list->head, _node);
    }
    else
    {
        dcList_unshift(_list, _node);
    }
}

dcResult dcList_printGraphDataNode(const dcNode *_listNode, dcString *_string)
{
    dcList *list = CAST_GRAPH_DATA_LIST(_listNode);
    dcListIterator *it = dcList_createHeadIterator(list);
    dcNode *node = dcList_getHead(list);
    dcResult result = TAFFY_SUCCESS;

    if (dcGraphData_isType(dcList_getHead(list), NODE_METHOD_CALL))
    {
        // it's a method call list!
        uint32_t i;

        for (i = 1; i < list->size; i++)
        {
            dcString_appendCharacter(_string, '[');
        }

        while ((node = dcListIterator_getNext(it))
               != NULL)
        {
            dcMethodCall_printNode(node, _string);
        }
    }
    else
    {
        while ((node = dcListIterator_getNext(it))
               != NULL
               && result == TAFFY_SUCCESS)
        {
            result = dcNode_print(node, _string);

            if (dcListIterator_hasNext(it))
            {
                dcString_appendString(_string, ", ");
            }
        }
    }

    dcListIterator_free(&it);
    return result;
}

// O(n) getter //
dcNode *dcList_get(const dcList *_list, dcContainerSizeType _index)
{
    size_t i = 0;
    dcListElement *elementIterator = _list->head;

    for (i = 0; i < _index && i < _list->size; i++)
    {
        elementIterator = elementIterator->next;
    }

    return (elementIterator != NULL
            ? elementIterator->object
            : NULL);
}

dcContainerSizeType dcList_getSize(const dcNode *_list)
{
    return (_list != NULL
            ? CAST_LIST(_list)->size
            : 0);
}

dcNode *dcList_getHead(const dcList *_list)
{
    return (_list->head != NULL
            ? _list->head->object
            : NULL);
}

dcNode *dcList_getNeck(const dcList *_list)
{
    return ((_list->head != NULL
             && _list->head->next != NULL)
            ? _list->head->next->object
            : NULL);
}

dcNode *dcList_getTail(const dcList *_list)
{
    return (_list->tail != NULL
            ? _list->tail->object
            : NULL);
}

dcListElement *dcList_getHeadElement(const dcList *_list)
{
    return (_list != NULL
            ? _list->head
            : NULL);
}

dcListElement *dcList_getNeckElement(const dcList *_list)
{
    if (_list != NULL && _list->head != NULL)
    {
        return _list->head->next;
    }

    return NULL;
}

dcListElement *dcList_insertBeforeListElement(dcList *_list,
                                              dcListElement *_element,
                                              dcNode *_node)
{
    dcListElement *toInsert = dcListElement_create(_node);

    if (_element == NULL)
    {
        if (_list->head == NULL)
        {
            _list->head = toInsert;
            _list->tail = toInsert;
        }
        else
        {
            _list->tail->next = toInsert;
            toInsert->previous = _list->tail;
            _list->tail = toInsert;
        }
    }
    else
    {
        assert(_list->head != NULL);

        dcListElement *previousSave = _element->previous;
        _element->previous = toInsert;
        toInsert->previous = previousSave;

        if (previousSave != NULL)
        {
            previousSave->next = toInsert;
        }

        toInsert->next = _element;

        if (_list->head == _element)
        {
            _list->head = toInsert;
        }
    }

    _list->size++;
    return toInsert;
}

void dcList_push(dcList *_list, dcNode *_node)
{
    if (_node != NULL)
    {
        dcListElement *element = dcListElement_create(_node);
        dcList_pushElement(_list, element);
    }
}

void dcList_pushElement(dcList *_list, dcListElement *_element)
{
    if (_list->head == NULL)
    {
        _list->head = _element;
        _list->tail = _element;
    }
    else
    {
        _list->tail->next = _element;
        _element->previous = _list->tail;
        _element->next = NULL;
        _list->tail = _element;
    }

    _list->size++;
}

void dcList_unshift(dcList *_list, dcNode *_node)
{
    dcListElement *element = dcListElement_create(_node);

    if (_list->head == NULL)
    {
        _list->head = element;
        _list->tail = element;
    }
    else
    {
        element->next = _list->head;
        _list->head->previous = element;
        _list->head = element;

        if (_list->head == _list->head->next)
        {
            dcError_internal("dcList_unshift::list loop detected");
        }
    }

    _list->size++;
}

dcNode *dcList_pop(dcList *_list, dcDepth _depth)
{
    dcListElement *toFree = _list->tail;
    dcNode *result = dcListElement_free(&toFree, _list, _depth);
    return result;
}

void dcList_popNTimes(dcList *_list, dcContainerSizeType _n, dcDepth _depth)
{
    size_t i = 0;
    size_t maximum = dcTaffy_min(_n, _list->size);

    for (i = 0; i < maximum; i++)
    {
        dcListElement *toFree = _list->tail;
        dcListElement_free(&toFree, _list, _depth);
    }
}

dcNode *dcList_shift(dcList *_list, dcDepth _depth)
{
    dcListElement *toFree = _list->head;

    return (toFree != NULL
            ? dcListElement_free(&toFree, _list, _depth)
            : NULL);
}

dcResult dcList_print(const dcList *_list, dcString *_stream)
{
    return dcList_printWithDelimiter(_list, _stream, ", ");
}

dcResult dcList_printWithDelimiter(const dcList *_list,
                                   dcString *_stream,
                                   const char *_delimiter)
{
    dcListIterator *it = dcList_createHeadIterator(_list);
    dcNode *node = NULL;
    dcString_appendString(_stream, "#SimpleList(");
    dcResult result = TAFFY_SUCCESS;

    while ((node = dcListIterator_getNext(it))
           != NULL
           && result == TAFFY_SUCCESS)
    {
        result = dcNode_print(node, _stream);

        if (dcListIterator_hasNext(it))
        {
            dcString_appendString(_stream, _delimiter);
        }
    }

    dcString_appendCharacter(_stream, ')');
    dcListIterator_free(&it);
    return result;
}

// create dcList_compareNode
dcTaffy_createCompareFunctionMacro(dcList, CAST_LIST);

// create dcList_printNode
dcTaffy_createPrintNodeFunctionMacro(dcList, CAST_LIST);

// create dcList_display
dcTaffy_createDisplayFunctionMacro(dcList);

// create dcList_copyNode
dcTaffy_createCopyNodeFunctionMacro(dcList, CAST_LIST);

// create dcList_freeNode
dcTaffy_createFreeNodeFunctionMacro(dcList, CAST_LIST);

// create dcList_markNode
dcTaffy_createMarkNodeFunctionMacro(dcList, CAST_LIST);

// create dcList_registerNode
dcTaffy_createRegisterNodeFunctionMacro(dcList, CAST_LIST);

// create dcList_marshallNode
dcTaffy_createMarshallNodeFunctionMacro(dcList, CAST_LIST);

// create dcList_unmarshallNode
dcTaffy_createUnmarshallNodeFunctionMacro(dcList, CAST_LIST);

dcList *dcList_copy(const dcList *_from, dcDepth _depth)
{
    dcList *to = NULL;

    if (_from != NULL)
    {
        to = dcList_create();
        dcListElement *toIterator = NULL;

        if (_from->head != NULL)
        {
            to->head = dcListElement_copy(_from->head, _depth);
            toIterator = to->head;
            dcListElement *fromIterator = _from->head->next;

            while (fromIterator != NULL)
            {
                toIterator->next = dcListElement_copy(fromIterator, _depth);
                toIterator->next->previous = toIterator;
                toIterator = toIterator->next;
                fromIterator = fromIterator->next;
            }
        }

        to->tail = toIterator;
        to->size = _from->size;
    }

    return to;
}

void dcList_copyGraphDataNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_GRAPH_DATA_LIST(_to) =
        dcList_copy(CAST_GRAPH_DATA_LIST(_from), _depth);
}

dcArray *dcList_asArray(const dcList *_list)
{
    dcArray *array = dcArray_createWithSize(_list->size);
    dcListElement *that = NULL;

    for (that = _list->head; that != NULL; that = that->next)
    {
        dcArray_add(array, that->object);
    }

    return array;
}

dcList *dcList_createFromArray(const dcArray *_array)
{
    dcList *list = dcList_create();

    if (_array != NULL)
    {
        const size_t arraySize = _array->size;
        uint32_t i = 0;

        for (i = 0; i < arraySize; i++)
        {
            dcList_push(list, dcArray_get(_array, i));
        }
    }

    return list;
}

dcListIterator *dcList_createHeadIterator(const dcList *_list)
{
    return (_list == NULL
            ? NULL
            : dcListIterator_create(_list->head));
}

dcListIterator *dcList_createTailIterator(const dcList *_list)
{
    return (_list == NULL
            ? NULL
            : dcListIterator_create(_list->tail));
}

dcResult dcList_containsEqual(const dcList *_list, dcNode *_node)
{
    dcListElement *that = NULL;
    dcResult result = TAFFY_FAILURE;

    for (that = _list->head; that != NULL; that = that->next)
    {
        dcTaffyOperator compareResult;
        dcResult myResult =
            dcNode_compareEqual(that->object, _node, &compareResult);

        if ((myResult == TAFFY_SUCCESS
             && compareResult == TAFFY_EQUALS) // success
            || myResult == TAFFY_EXCEPTION) // doh
        {
            result = myResult;
            break;
        }
    }

    return result;
}

bool dcList_contains(const dcList *_list, const dcNode *_node)
{
    dcListElement *that = NULL;
    bool result = false;

    for (that = _list->head; that != NULL; that = that->next)
    {
        if (that->object == _node)
        {
            result = true;
            break;
        }
    }

    return result;
}

dcResult dcList_remove(dcList *_list, dcNode *_node, dcDepth _depth)
{
    return dcList_removeWithComparisonFunction(_list,
                                               _node,
                                               _depth,
                                               &dcNode_compare);
}

dcResult dcList_removeWithComparisonFunction
    (dcList *_list,
     dcNode *_node,
     dcDepth _depth,
     dcNode_comparePointer _comparisonFunction)
{
    dcListElement *that = _list->tail;
    dcListElement *saveElement = NULL;
    dcResult result = TAFFY_FAILURE;

    while (that != NULL)
    {
        dcTaffyOperator compareResult;
        dcResult myResult = _comparisonFunction(that->object,
                                                _node,
                                                &compareResult);
        if (myResult == TAFFY_EXCEPTION)
        {
            result = TAFFY_EXCEPTION;
            break;
        }
        else if (myResult == TAFFY_SUCCESS
                 && compareResult == TAFFY_EQUALS)
        {
            result = TAFFY_SUCCESS;
            saveElement = that;

            if (that->previous != NULL)
            {
                that = that->previous;
            }

            dcListElement_free(&saveElement, _list, _depth);
            break;
        }
        else
        {
            that = that->previous;
        }
    }

    return result;
}

dcNode *dcList_removeElement(dcList *_list,
                             dcListElement **_element,
                             dcDepth _depth)
{
    return dcListElement_free(_element, _list, _depth);
}

void dcList_append(dcList *_list, dcList **_other)
{
    if ((*_other)->size > 0)
    {
        if (_list->tail != NULL)
        {
            _list->tail->next = (*_other)->head;

            if (_list->tail->next != NULL)
            {
                _list->tail->next->previous = _list->tail;
            }
        }
        else
        {
            // sanity
            dcError_assert(_list->head == NULL);
            _list->head = (*_other)->head;
        }

        _list->size += (*_other)->size;
        _list->tail = (*_other)->tail;
    }

    // free _other
    (*_other)->head = NULL;
    (*_other)->tail = NULL;
    (*_other)->size = 0;
    dcList_free(_other, DC_SHALLOW);
}

void dcList_concat(dcList *_list,
                   const dcList *_other,
                   dcDepth _depth)
{
    FOR_EACH_IN_LIST(_other, that)
    {
        if (_depth == DC_DEEP)
        {
            dcList_push(_list, dcNode_copy(that->object, DC_DEEP));
        }
        else
        {
            dcList_push(_list, that->object);
        }
    }
}

void dcList_mark(dcList *_list)
{
    if (_list != NULL)
    {
        TAFFY_DEBUG(size_t markCount = 0;);
        dcListElement *element = _list->head;

        while (element != NULL)
        {
            dcNode_mark(element->object);
            element = element->next;
            TAFFY_DEBUG(dcError_assert(markCount < _list->size);
                        markCount++;);
        }
    }
}

void dcList_register(dcList *_list)
{
    if (_list != NULL)
    {
        dcListElement *that = NULL;

        for (that = _list->head; that != NULL; that = that->next)
        {
            dcNode_register(that->object);
        }
    }
}

dcList *dcList_unmarshall(dcString *_stream)
{
    dcList *result = NULL;
    uint8_t type;
    uint32_t listSize;

    if (dcMarshaller_unmarshallNoNull(_stream,
                                      "uw",
                                      &type,
                                      &listSize)
        && type == NODE_LIST
        // do a basic size check
        && dcString_hasLengthLeft(_stream, ((uint64_t)listSize
                                            * MIN_MARSHALL_SIZE)))
    {
        uint32_t listIt = 0;
        result = dcList_create();

        for (listIt = 0; listIt < listSize; listIt++)
        {
            dcNode *node = dcNode_unmarshall(_stream);

            if (node != NULL)
            {
                dcList_push(result, node);
            }
            else
            {
                // FAILURE //
                dcList_free(&result, DC_DEEP);
                result = NULL;
                break;
            }
        }
    } // else FAILURE //

    return result;
}

bool dcList_unmarshallGraphDataNode(dcNode *_node, dcString *_stream)
{
    CAST_GRAPH_DATA_LIST(_node) = dcList_unmarshall(_stream);
    return (CAST_GRAPH_DATA_LIST(_node) != NULL);
}

dcString *dcList_marshall(const dcList *_list, dcString *_stream)
{
    dcString *result = dcMarshaller_marshall(_stream,
                                             "uw",
                                             NODE_LIST,
                                             _list->size);
    dcListElement *that = NULL;

    for (that = _list->head; that != NULL; that = that->next)
    {
        dcNode_marshall(that->object, result);
    }

    return result;
}

dcString *dcList_marshallGraphDataNode(const dcNode *_node, dcString *_stream)
{
    return dcList_marshall(CAST_GRAPH_DATA_LIST(_node), _stream);
}

dcResult dcList_compare(dcList *_left,
                        dcList *_right,
                        dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_SUCCESS;
    *_compareResult = TAFFY_EQUALS;

    if (_left->size == _right->size)
    {
        dcListElement *leftThat;
        dcListElement *rightThat;

        for (leftThat = _left->head, rightThat = _right->head;
             leftThat != NULL && rightThat != NULL;
             leftThat = leftThat->next, rightThat = rightThat->next)
        {
            dcResult myResult = dcNode_compare
                (leftThat->object, rightThat->object, _compareResult);

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

void dcList_each(const dcList *_list,
                 dcContainerEachFunction _function,
                 void *_token)
{
    const dcListElement *that = NULL;

    for (that = _list->head; that != NULL; that = that->next)
    {
        if (_function(that->object, (dcNode *)_token) != TAFFY_SUCCESS)
        {
            break;
        }
    }
}

void dcList_verifyTemplate(const dcList *_list)
{
    const dcListElement *that = NULL;

    for (that = _list->head; that != NULL; that = that->next)
    {
        dcError_assert(dcNode_isTemplate(that->object));
    }
}

void dcList_verify(const dcList *_list)
{
    const dcListElement *that = NULL;
    const dcListElement *previous = NULL;

    for (that = _list->head;
         that != NULL;
         previous = that, that = that->next)
    {
        dcError_assert(that->previous == previous);
    }

    dcError_assert(previous == _list->tail);
    previous = NULL;

    for (that = _list->tail;
         that != NULL;
         previous = that, that = that->previous)
    {
        dcError_assert(that->next == previous);
    }
}

// selection sort
void dcList_doSelectionSort(dcList *_list)
{
    dcListElement *min = NULL;
    dcListElement *i = NULL;

    for (i = _list->head; i->next != NULL; i = i->next)
    {
        min = i;
        dcListElement *j = NULL;

        for (j = i->next; j != NULL; j = j->next)
        {
            dcTaffyOperator compareResult;

            if ((dcNode_compare(j->object,
                               min->object,
                               &compareResult)
                 == TAFFY_SUCCESS)
                && compareResult == TAFFY_LESS_THAN)
            {
                min = j;
            }
        }

        if (min != i)
        {
            dcNode *temp = i->object;
            i->object = min->object;
            min->object = temp;
        }
    }
}

dcResult dcList_find(dcList *_list, dcNode *_node, dcListElement **_found)
{
    return dcList_findWithComparisonFunction(_list,
                                             _node,
                                             &dcNode_compare,
                                             _found);
}

dcResult dcList_findWithComparisonFunction(dcList *_list,
                                           dcNode *_node,
                                           ListFindFunction *_findFunction,
                                           dcListElement **_found)
{
    dcListElement *that;
    dcResult result = TAFFY_FAILURE;

    for (that = _list->head; that != NULL; that = that->next)
    {
        dcTaffyOperator operatorResult;
        dcResult compareResult = _findFunction(that->object,
                                               _node,
                                               &operatorResult);

        if (compareResult == TAFFY_SUCCESS
            && operatorResult == TAFFY_EQUALS)
        {
            result = TAFFY_SUCCESS;

            if (_found != NULL)
            {
                *_found = that;
            }

            break;
        }
        else if (compareResult == TAFFY_EXCEPTION)
        {
            result = TAFFY_EXCEPTION;
            break;
        }
    }

    return result;
}

dcListElement *dcList_replaceRange(dcList *_list,
                                   dcListElement *_start,
                                   dcListElement *_end,
                                   dcNode *_replacement,
                                   dcDepth _depth)
{
    dcListElement *inserted =
        dcList_insertBeforeListElement(_list, _start, _replacement);
    dcListElement *that;
    bool endWasReached = false;

    for (that = _start; ! endWasReached; )
    {
        endWasReached = (that == _end);
        dcListElement *next = that->next;
        dcList_removeElement(_list, &that, _depth);
        that = next;
    }

    return inserted;
}

void dcList_iterateCombinations(dcList *_list,
                                CombinationFunction _function,
                                uint32_t _token)
{
    if (_list->size == 0)
    {
        return;
    }

    if (_list->size == 1)
    {
        dcList *right = dcList_create();
        _function(_list, right, _token);
        dcList_free(&right, DC_DEEP);
        return;
    }

    dcListElement *leftStart = NULL;

    for (leftStart = _list->head;
         leftStart != _list->tail;
         leftStart = leftStart->next)
    {
        dcListElement *leftEnd;

        for (leftEnd = leftStart->next;
             leftEnd != NULL;
             leftEnd = leftEnd->next)
        {
            dcListElement *rightThat;

            dcList *left = dcList_create();
            dcList *right = dcList_create();

            dcListElement *leftThat;

            for (leftThat = leftStart;
                 leftThat != leftEnd;
                 leftThat = leftThat->next)
            {
                dcList_push(left, leftThat->object);
            }

            for (rightThat = leftEnd;
                 rightThat != NULL;
                 rightThat = rightThat->next)
            {
                dcList_push(right, rightThat->object);
            }

            for (rightThat = _list->head;
                 rightThat != leftStart;
                 rightThat = rightThat->next)
            {
                dcList_push(right, rightThat->object);
            }

            _function(left, right, _token);
            _function(right, left, _token);

            dcList_free(&left, DC_SHALLOW);
            dcList_free(&right, DC_SHALLOW);
        }
    }
}
