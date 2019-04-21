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

#include <stdlib.h>

#include "dcError.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcObjectStack.h"
#include "dcObjectStackList.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcSystem.h" // derr

dcObjectStackList *dcObjectStackList_create(void)
{
    dcObjectStackList *list =
        (dcObjectStackList *)dcMemory_allocate(sizeof(dcObjectStackList));
    list->objectStacks = dcList_create();
    return list;
}

void dcObjectStackList_free(dcObjectStackList **_list)
{
    // don't touch global scope
    dcObjectStackList *list = *_list;
    dcList_free(&list->objectStacks, DC_DEEP);
    dcMemory_free(*_list);
}

void dcObjectStackList_pushObjectStack(dcObjectStackList *_list,
                                       dcObjectStack *_stack)
{
    dcList_push(_list->objectStacks, dcObjectStack_createShell(_stack));
}

dcObjectStack *dcObjectStackList_popObjectStack(dcObjectStackList *_list,
                                                dcDepth _depth)
{
    dcNode *last = dcList_pop(_list->objectStacks, _depth);
    dcObjectStack *result = NULL;

    if (last != NULL)
    {
        result = CAST_OBJECT_STACK(last);
        dcNode_freeShell(&last);
    }

    return result;
}

void dcObjectStackList_pushScope(dcObjectStackList *_list, dcNode *_scope)
{
    dcError_assert(_list->objectStacks->size > 0);
    dcObjectStack_pushScope
        (CAST_OBJECT_STACK(dcList_getTail(_list->objectStacks)), _scope);
}

void dcObjectStackList_popScope(dcObjectStackList *_list, dcDepth _depth)
{
    dcError_assert(_list->objectStacks->size > 0);
    dcObjectStack_popScope
        (CAST_OBJECT_STACK(dcList_getTail(_list->objectStacks)), _depth);
}

dcNode *dcObjectStackList_getObject(dcObjectStackList *_list, const char *_name)
{
    dcNode *result =
        dcObjectStackList_getScopeDataForObject(_list, _name, NULL);
    return (result == NULL
            ? NULL
            : dcScopeData_getObject(result));
}

dcNode *dcObjectStackList_getScopeDataForObject(dcObjectStackList *_list,
                                                const char *_name,
                                                dcScope **_foundScope)
{
    dcListElement *that = _list->objectStacks->tail;
    dcNode *result = NULL;

    while (that != NULL && result == NULL)
    {
        dcObjectStack *stack = CAST_OBJECT_STACK(that->object);
        result = dcObjectStack_getScopeDataForObject(stack, _name, _foundScope);
        that = (result == NULL && stack->breakthrough
                ? that->previous
                : NULL);
    }

    if (result == NULL)
    {
        dcNode *globalScope = dcSystem_getGlobalScope();
        result = dcScope_getScopeDataForObject(CAST_SCOPE(globalScope), _name);

        if (result != NULL && _foundScope != NULL)
        {
            *_foundScope = CAST_SCOPE(globalScope);
        }
    }

    return result;
}

dcObjectStack *dcObjectStackList_getTailObjectStack(dcObjectStackList *_list)
{
    return (_list->objectStacks->tail == NULL
            ? NULL
            : CAST_OBJECT_STACK(dcList_getTail(_list->objectStacks)));
}

void dcObjectStackList_print(const dcObjectStackList *_list, dcString *_stream)
{
    dcString_appendString(_stream, "ObjectStackList\n");

    dcContainerSizeType i;
    dcListElement *that;

    for (i = 0, that = _list->objectStacks->head;
         that != NULL;
         i++, that = that->next)
    {
        dcString_append(_stream, "---------------\nLevel %u: \n", i);
        dcNode_print(that->object, _stream);
    }
}

dcTaffy_createDisplayFunctionMacro(dcObjectStackList);

dcScope *dcObjectStackList_getTailScope(dcObjectStackList *_list)
{
    dcError_assert(_list->objectStacks->size > 0);
    return (dcObjectStack_getTailScope
            (CAST_OBJECT_STACK(dcList_getTail(_list->objectStacks))));
}

dcNode *dcObjectStackList_getTailSelf(dcObjectStackList *_list)
{
    dcError_assert(_list->objectStacks->size > 0);
    return (CAST_OBJECT_STACK(dcList_getTail(_list->objectStacks))->self);
}

dcNode *dcObjectStackList_getUpSelf(dcObjectStackList *_list)
{
    dcListElement *that = NULL;

    for (that = _list->objectStacks->tail;
         that != NULL && CAST_OBJECT_STACK(that->object)->breakthrough;
         that = that->previous)
    {
    }

    dcError_assert(that != NULL);
    return CAST_OBJECT_STACK(that->object)->self;
}

void dcObjectStackList_mark(dcObjectStackList *_list)
{
    if (_list != NULL)
    {
        dcList_mark(_list->objectStacks);
    }
}

bool dcObjectStackList_isObjectConst(dcObjectStackList *_list,
                                     const dcNode *_node)
{
    dcListElement *that;
    bool result = false;

    for (that = _list->objectStacks->tail; that != NULL; that = that->previous)
    {
        dcObjectStack *stack = CAST_OBJECT_STACK(that->object);

        if (stack->self == _node
            && stack->selfIsConst)
        {
            result = true;
            break;
        }
    }

    return result;
}
