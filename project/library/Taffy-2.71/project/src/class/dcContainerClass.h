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

#ifndef __DC_CONTAINER_CLASS_H__
#define __DC_CONTAINER_CLASS_H__

#include "dcDefines.h"

struct dcContainerClassAux_t
{
    bool modified;

    // the stack will overflow before this variable hits 0xFFFF
    uint16_t loopCount;
};

typedef struct dcContainerClassAux_t dcContainerClassAux;

struct dcNode_t *dcContainerClass_createNode(bool _object);

#define dcContainerClass_createObject()         \
    dcContainerClass_createNode(true)

bool dcContainerClass_isModified(struct dcNode_t *_node);
bool dcContainerClass_checkModified(struct dcNode_t *_node);
struct dcNode_t *dcContainerClass_setModified(struct dcNode_t *_object,
                                              bool _modified);
void dcContainerClass_startLoop(struct dcNode_t *_node);
void dcContainerClass_stopLoop(struct dcNode_t *_node);

struct dcNode_t *dcContainerClass_objectAtIndexesArrayHelper
    (struct dcNode_t *_receiver,
     struct dcArray_t *_arguments,
     bool _set,
     struct dcArray_t *_receiverArray);

// standard functions //
ALLOCATE_FUNCTION(dcContainerClass_allocateNode);
COPY_FUNCTION(dcContainerClass_copyNode);
FREE_FUNCTION(dcContainerClass_freeNode);
GET_TEMPLATE_FUNCTION(dcContainerClass_getTemplate);
INITIALIZE_FUNCTION(dcContainerClass_initialize);

//
// Taffy C Methods
//
TAFFY_C_METHOD(dcContainerClass_asString);

#define CONTAINER_CLASS_NAME "Container"

#endif
