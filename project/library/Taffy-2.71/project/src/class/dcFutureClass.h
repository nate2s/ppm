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

#ifndef __DC_FUTURE_CLASS_H__
#define __DC_FUTURE_CLASS_H__

#include "dcDefines.h"

  //////////////////////
 // dcFutureClassAux //
//////////////////////

struct dcFutureClassAux_t
{
    struct dcNode_t *value;
    bool valueSet;
    struct dcCondition_t *valueCondition;
    struct dcMutex_t *valueMutex;
};

typedef struct dcFutureClassAux_t dcFutureClassAux;

///////////////////
// dcFutureClass //
///////////////////

// creating //
struct dcNode_t *dcFutureClass_createNode(bool _object);
struct dcNode_t *dcFutureClass_createObject(void);

struct dcNode_t *dcFutureClass_waitForValue(struct dcNode_t *_future);
void dcFutureClass_setValue(struct dcNode_t *_future, struct dcNode_t *_value);

bool dcFutureClass_isMe(const struct dcNode_t *_node);

// standard functions //
ALLOCATE_FUNCTION(dcFutureClass_allocateNode);
COPY_FUNCTION(dcFutureClass_copyNode);
DEALLOCATE_FUNCTION(dcFutureClass_deallocateNode);
FREE_FUNCTION(dcFutureClass_freeNode);
GET_TEMPLATE_FUNCTION(dcFutureClass_getTemplate);
INITIALIZE_FUNCTION(dcFutureClass_initialize);
REGISTER_FUNCTION(dcFutureClass_registerNode);
MARK_FUNCTION(dcFutureClass_markNode);
MARSHALL_FUNCTION(dcFutureClass_marshallNode);
UNMARSHALL_FUNCTION(dcFutureClass_unmarshallNode);

// taffy c methods
TAFFY_C_METHOD(dcFutureMetaClass_createWithBlock);
TAFFY_C_METHOD(dcFutureMetaClass_createWithBlockWithArguments);

TAFFY_C_METHOD(dcFutureClass_asString);
TAFFY_C_METHOD(dcFutureClass_name);

#define FUTURE_PACKAGE_NAME CORE_PACKAGE_NAME
#define FUTURE_CLASS_NAME "Future"

#endif
