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

#include "dcClass.h"
#include "dcList.h"
#include "dcMainClass.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcObjectStack.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcSystem.h"

dcObjectStack *dcObjectStack_create(void)
{
    dcObjectStack *stack =
        (dcObjectStack *)dcMemory_allocateAndInitialize(sizeof(dcObjectStack));
    stack->scopes = dcList_create();
    return stack;
}

dcNode *dcObjectStack_createNode(void)
{
    return dcObjectStack_createShell(dcObjectStack_create());
}

dcNode *dcObjectStack_createShell(dcObjectStack *_stack)
{
    return dcNode_createWithGuts(NODE_OBJECT_STACK, _stack);
}

void dcObjectStack_free(dcObjectStack **_stack, dcDepth _depth)
{
    dcObjectStack *stack = *_stack;
    dcList_free(&stack->scopes, _depth);
    dcMemory_free(*_stack);
}

dcNode *dcObjectStack_setSelf(dcObjectStack *_stack,
                              dcNode *_self,
                              bool _selfIsConst)
{
    _stack->self = _self;
    _stack->selfIsConst = _selfIsConst;
    return _self;
}

dcNode *dcObjectStack_getSelf(dcObjectStack *_stack)
{
    return _stack->self;
}

dcScope *dcObjectStack_getHeadScope(dcObjectStack *_stack)
{
    assert(_stack->scopes->size > 0);
    return (CAST_SCOPE(dcList_getHead(_stack->scopes)));
}

void dcObjectStack_pushScope(dcObjectStack *_stack, dcNode *_node)
{
    assert(_node->type == NODE_SCOPE);
    dcList_push(_stack->scopes, _node);
}

void dcObjectStack_popScope(dcObjectStack *_stack, dcDepth _depth)
{
    assert(_stack->scopes->size > 0);
    dcList_pop(_stack->scopes, _depth);
}

dcResult dcObjectStack_print(const dcObjectStack *_stack, dcString *_stream)
{
    dcString_appendString(_stream, "ObjectStack:\nSelf:");
    dcResult result = dcNode_print(_stack->self, _stream);
    dcString_appendString(_stream, "\nStack: ");
    dcList_print(_stack->scopes, _stream);
    return result;
}

dcNode *dcObjectStack_getScopeDataForObject(dcObjectStack *_stack,
                                            const char *_name,
                                            dcScope **_foundScope)
{
    dcListElement *that;
    dcNode *result = NULL;

    for (that = _stack->scopes->tail; that != NULL; that = that->previous)
    {
        result = dcScope_getScopeDataForObject(CAST_SCOPE(that->object), _name);

        if (result != NULL)
        {
            if (_foundScope != NULL)
            {
                *_foundScope = CAST_SCOPE(that->object);
            }

            break;
        }
    }

    if (result == NULL)
    {
        result = dcClass_getScopeDataForObject(_stack->self, // receiver
                                               _name,        // name
                                               true,         // search UP
                                               NULL,         // requestor
                                               NULL,         // foundTemplate
                                               _foundScope);
    }

    return result;
}

dcNode *dcObjectStack_getObject(dcObjectStack *_stack, const char *_name)
{
    dcNode *result = dcObjectStack_getScopeDataForObject(_stack, _name, NULL);
    return (result == NULL
            ? NULL
            : dcScopeData_getObject(result));
}

//
// Climb into the sky, never wonder why, tail scope!
// You're a Tail Scope!
// You're a Tail Scope!
//
dcScope *dcObjectStack_getTailScope(dcObjectStack *_stack)
{
    assert(_stack->scopes->size > 0);
    return (CAST_SCOPE(dcList_getTail(_stack->scopes)));
}

void dcObjectStack_mark(dcObjectStack *_stack)
{
    dcNode_mark(_stack->self);
    dcList_mark(_stack->scopes);
}

dcTaffy_createDisplayFunctionMacro(dcObjectStack);
dcTaffy_createFreeNodeFunctionMacro(dcObjectStack, CAST_OBJECT_STACK);
dcTaffy_createMarkNodeFunctionMacro(dcObjectStack, CAST_OBJECT_STACK);
dcTaffy_createPrintNodeFunctionMacro(dcObjectStack, CAST_OBJECT_STACK);
