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

#ifndef __DC_LIST_CLASS_H__
#define __DC_LIST_CLASS_H__

#include "dcDefines.h"

////////////////////
// dcListClassAux //
////////////////////

struct dcListClassAux_t
{
    struct dcList_t *objects;
    bool initialized;
};

typedef struct dcListClassAux_t dcListClassAux;

/////////////////
// dcListClass //
/////////////////

// creating //
struct dcNode_t *dcListClass_createNode(struct dcList_t *_objects,
                                        bool _initialized,
                                        bool _object);

struct dcNode_t *dcListClass_createObject(struct dcList_t *_objects,
                                          bool _initialized);

struct dcNode_t *dcListClass_createEmptyObject(void);

// querying/getting //
bool dcListClass_isInitialized(const struct dcNode_t *_listNode);
void dcListClass_setInitialized(struct dcNode_t *_listNode);
struct dcList_t *dcListClass_getObjects(const struct dcNode_t *_node);
dcContainerSizeType dcListClass_getSize(const struct dcNode_t *_node);

// setting //
void dcListClass_setObjects(struct dcNode_t *_listNode,
                            struct dcList_t *_objects,
                            dcDepth _depth);

// standard functions //
ALLOCATE_FUNCTION(dcListClass_allocateNode);
COPY_FUNCTION(dcListClass_copyNode);
DEALLOCATE_FUNCTION(dcListClass_deallocateNode);
GET_TEMPLATE_FUNCTION(dcListClass_getTemplate);
INITIALIZE_FUNCTION(dcListClass_initialize);
FREE_FUNCTION(dcListClass_freeNode);
MARK_FUNCTION(dcListClass_markNode);
MARSHALL_FUNCTION(dcListClass_marshallNode);
REGISTER_FUNCTION(dcListClass_registerNode);
UNMARSHALL_FUNCTION(dcListClass_unmarshallNode);

// taffy c methods //
TAFFY_C_METHOD(dcListMetaClass_fromArray);

TAFFY_C_METHOD(dcListClass_collect);
TAFFY_C_METHOD(dcListClass_collectBang);
TAFFY_C_METHOD(dcListClass_concat);
TAFFY_C_METHOD(dcListClass_each);
TAFFY_C_METHOD(dcListClass_equals);
TAFFY_C_METHOD(dcListClass_find);
TAFFY_C_METHOD(dcListClass_head);
TAFFY_C_METHOD(dcListClass_objectAtIndex);
TAFFY_C_METHOD(dcListClass_hash);
TAFFY_C_METHOD(dcListClass_pop);
TAFFY_C_METHOD(dcListClass_push);
TAFFY_C_METHOD(dcListClass_reject);
TAFFY_C_METHOD(dcListClass_rejectBang);
TAFFY_C_METHOD(dcListClass_removeAll);
TAFFY_C_METHOD(dcListClass_reverse);
TAFFY_C_METHOD(dcListClass_reverseBang);
TAFFY_C_METHOD(dcListClass_select);
TAFFY_C_METHOD(dcListClass_selectBang);
TAFFY_C_METHOD(dcListClass_shift);
TAFFY_C_METHOD(dcListClass_size);
TAFFY_C_METHOD(dcListClass_tail);
TAFFY_C_METHOD(dcListClass_unshift);

// helpers //
struct dcString_t *dcListClass_printHelper(struct dcList_t *_list,
                                           const char *_separator);

#define LIST_PACKAGE_NAME CONTAINER_PACKAGE_NAME
#define LIST_CLASS_NAME "List"

#endif
