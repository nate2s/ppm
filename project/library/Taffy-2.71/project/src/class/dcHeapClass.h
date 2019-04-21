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

#ifndef __DC_HEAP_CLASS_H__
#define __DC_HEAP_CLASS_H__

#include "dcDefines.h"

////////////////////
// dcHeapClassAux //
////////////////////

struct dcHeapClassAux_t
{
    struct dcHeap_t *objects;
    bool initialized;
};

typedef struct dcHeapClassAux_t dcHeapClassAux;

/////////////////
// dcHeapClass //
/////////////////

// creating //
struct dcNode_t *dcHeapClass_createNode(struct dcHeap_t *_objects,
                                        bool _initialized,
                                        bool _object);

struct dcNode_t *dcHeapClass_createObject(struct dcHeap_t *_objects,
                                          bool _initialized);

struct dcNode_t *dcHeapClass_createEmptyObject(void);

// querying/getting //
bool dcHeapClass_isInitialized(const struct dcNode_t *_heapNode);
void dcHeapClass_setInitialized(struct dcNode_t *_heapNode);
struct dcHeap_t *dcHeapClass_getObjects(const struct dcNode_t *_node);

// setting //
void dcHeapClass_setObjects(struct dcNode_t *_heapNode,
                             struct dcHeap_t *_objects,
                             dcDepth _depth);

// standard functions //
ALLOCATE_FUNCTION(dcHeapClass_allocateNode);
DEALLOCATE_FUNCTION(dcHeapClass_deallocateNode);
COPY_FUNCTION(dcHeapClass_copyNode);
FREE_FUNCTION(dcHeapClass_freeNode);
GET_TEMPLATE_FUNCTION(dcHeapClass_getTemplate);
INITIALIZE_FUNCTION(dcHeapClass_initialize);
MARK_FUNCTION(dcHeapClass_markNode);
MARSHALL_FUNCTION(dcHeapClass_marshallNode);
REGISTER_FUNCTION(dcHeapClass_registerNode);
UNMARSHALL_FUNCTION(dcHeapClass_unmarshallNode);

// taffy c methods //
TAFFY_C_METHOD(dcHeapClass_asString);
TAFFY_C_METHOD(dcHeapClass_insert);
TAFFY_C_METHOD(dcHeapClass_pop);
TAFFY_C_METHOD(dcHeapClass_size);
TAFFY_C_METHOD(dcHeapClass_verify);

TAFFY_C_METHOD(dcHeapMetaClass_newMin);
TAFFY_C_METHOD(dcHeapMetaClass_newMax);

#define HEAP_PACKAGE_NAME CONTAINER_PACKAGE_NAME
#define HEAP_CLASS_NAME "Heap"

#endif
