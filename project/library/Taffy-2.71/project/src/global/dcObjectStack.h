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

#ifndef __DC_OBJECT_STACK_H__
#define __DC_OBJECT_STACK_H__

#include "dcDefines.h"

struct dcObjectStack_t
{
    struct dcList_t *scopes;
    struct dcNode_t *self;
    bool selfIsConst;
    bool breakthrough;
    uint16_t loopCount;
};

typedef struct dcObjectStack_t dcObjectStack;

// creating //
dcObjectStack *dcObjectStack_create(void);
struct dcNode_t *dcObjectStack_createNode(void);
struct dcNode_t *dcObjectStack_createShell(dcObjectStack *_stack);
void dcObjectStack_free(dcObjectStack **_stack, dcDepth _depth);

// printing
const char *dcObjectStack_display(const dcObjectStack *_stack);
dcResult dcObjectStack_print(const dcObjectStack *_stack,
                             struct dcString_t *_stream);

// self
struct dcNode_t *dcObjectStack_getSelf(dcObjectStack *_stack);
struct dcNode_t *dcObjectStack_setSelf(dcObjectStack *_stack,
                                       struct dcNode_t *_self,
                                       bool _selfIsConst);

// getting //
struct dcScope_t *dcObjectStack_getHeadScope(dcObjectStack *_stack);
struct dcScope_t *dcObjectStack_getTailScope(dcObjectStack *_stack);

// push a node onto the stack
void dcObjectStack_pushScope(dcObjectStack *_stack, struct dcNode_t *_node);

// pop a node from the stack
void dcObjectStack_popScope(dcObjectStack *_stack, dcDepth _depth);

// for LHS and RHS
struct dcNode_t *dcObjectStack_getScopeDataForObject
    (dcObjectStack *_stack,
     const char *_name,
     struct dcScope_t **_foundScope);
// for RHS only
struct dcNode_t *dcObjectStack_getObject(dcObjectStack *_stack,
                                         const char *_name);

// marking //
void dcObjectStack_mark(dcObjectStack *_stack);

// linkages
FREE_FUNCTION(dcObjectStack_freeNode);
PRINT_FUNCTION(dcObjectStack_printNode);
MARK_FUNCTION(dcObjectStack_markNode);

#endif
