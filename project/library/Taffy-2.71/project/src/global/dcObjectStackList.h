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

#ifndef __DC_OBJECT_STACK_LIST_H__
#define __DC_OBJECT_STACK_LIST_H__

#include "dcDefines.h"

// Object Stack List, a container for selves and scopes
//
// Example:
// Let theList be a node evaluator's ObjectStackList. Then:
//
// test = new Test
// [test sooSar] // <-- a new object stack is pushed onto theList
//
// class Test
// {
//     (@) sooSar
//     {
//         // let theStack be theList's tail of objectStacks (see below)
//         // theStack's self is now test
//
//         if (booBar)
//         {
//             // a new scope is pushed onto theStack
//         }
//     }
// }
//

struct dcObjectStackList_t
{
    // the object stacks
    struct dcList_t *objectStacks;
};

typedef struct dcObjectStackList_t dcObjectStackList;

// creating //
dcObjectStackList *dcObjectStackList_create(void);

// freeing //
void dcObjectStackList_free(dcObjectStackList **_list);

// modifying //
void dcObjectStackList_pushObjectStack(dcObjectStackList *_list,
                                       struct dcObjectStack_t *_stack);
struct dcObjectStack_t *dcObjectStackList_popObjectStack
    (dcObjectStackList *_list,
     dcDepth _depth);

// modification helpers //
void dcObjectStackList_pushScope(dcObjectStackList *_list,
                                 struct dcNode_t *_scope);
void dcObjectStackList_popScope(dcObjectStackList *_list, dcDepth _depth);

// getting //
struct dcNode_t *dcObjectStackList_getObject(dcObjectStackList *_list,
                                             const char *_name);
struct dcNode_t *dcObjectStackList_getScopeDataForObject
    (dcObjectStackList *_list,
     const char *_name,
     struct dcScope_t **_foundScope);
struct dcNode_t *dcObjectStackList_getTailSelf(dcObjectStackList *_list);
struct dcNode_t *dcObjectStackList_getUpSelf(dcObjectStackList *_list);

// getting scopes //
struct dcScope_t *dcObjectStackList_getTailScope(dcObjectStackList *_list);
struct dcObjectStack_t *dcObjectStackList_getTailObjectStack
    (dcObjectStackList *_list);

// printing //
const char *dcObjectStackList_display(const dcObjectStackList *_list);

// marking //
void dcObjectStackList_mark(dcObjectStackList *_list);

// querying //
bool dcObjectStackList_isObjectConst(dcObjectStackList *_list,
                                     const struct dcNode_t *_node);

#endif
