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

#ifndef __DC_HEAP_H__
#define __DC_HEAP_H__

#include "dcDefines.h"

#define DEFAULT_HEAP_SIZE 20

enum dcHeapType_e
{
    NO_HEAP_TYPE,
    HEAP_MIN,
    HEAP_MAX
};

typedef uint8_t dcHeapType;

struct dcHeap_t
{
    dcHeapType type;
    struct dcArray_t *objects;
};

typedef struct dcHeap_t dcHeap;

struct dcNode_t *dcHeap_createNode(dcHeapType _type);
struct dcNode_t *dcHeap_createShell(dcHeap *_heap);

dcHeap *dcHeap_create(dcHeapType _type);
dcHeap *dcHeap_createFromArray(dcHeapType _type, struct dcArray_t *_objects);
dcHeap *dcHeap_createWithCapacity(dcHeapType _type,
                                  dcContainerSizeType _capacity);
void dcHeap_free(dcHeap **_heap, dcDepth _depth);
void dcHeap_clear(dcHeap *_heap, dcDepth _depth);
dcHeap *dcHeap_copy(const dcHeap *_from, dcDepth _depth);

// getters //
struct dcNode_t *dcHeap_get(const dcHeap *_heap, dcContainerSizeType _index);

// modifers //
dcResult dcHeap_insert(dcHeap *_heap, struct dcNode_t *_object);
dcResult dcHeap_pop(dcHeap *_heap, struct dcNode_t **_popResult);

// comparing //
dcResult dcHeap_compare(const dcHeap *_left,
                        const dcHeap *_right,
                        dcTaffyOperator *_operatorResult);

// registering //
void dcHeap_register(dcHeap *_heap);

// marking //
void dcHeap_mark(dcHeap *_heap);

// standard functions //
FREE_FUNCTION(dcHeap_freeNode);
MARK_FUNCTION(dcHeap_markNode);
REGISTER_FUNCTION(dcHeap_registerNode);

// for debugging
dcResult dcHeap_verify(const dcHeap *_heap);

#endif
