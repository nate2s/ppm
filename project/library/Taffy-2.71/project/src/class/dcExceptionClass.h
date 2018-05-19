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

#ifndef __DC_EXCEPTION_CLASS_H__
#define __DC_EXCEPTION_CLASS_H__

#include "dcDefines.h"

/////////////////////////
// dcExceptionClassAux //
/////////////////////////

struct dcExceptionClassAux_t
{
    struct dcNode_t *data;
};

typedef struct dcExceptionClassAux_t dcExceptionClassAux;

//////////////////////
// dcExceptionClass //
//////////////////////

// creating //
struct dcNode_t *dcExceptionClass_createNode(struct dcNode_t *_data,
                                             bool _object);

#define dcExceptionClass_createObject(data)     \
    dcExceptionClass_createNode(data, true)

struct dcNode_t *dcExceptionClass_createObjectFromString(const char *_data);

// getting //
struct dcNode_t *dcExceptionClass_getData(const struct dcNode_t *_exception);

// standard functions //
ALLOCATE_FUNCTION(dcExceptionClass_allocateNode);
COPY_FUNCTION(dcExceptionClass_copyNode);
CREATE_INSTANCE_FUNCTION(dcExceptionClass_createInstance);
FREE_FUNCTION(dcExceptionClass_freeNode);
GET_TEMPLATE_FUNCTION(dcExceptionClass_getTemplate);
INITIALIZE_FUNCTION(dcExceptionClass_initialize);
MARK_FUNCTION(dcExceptionClass_markNode);
MARSHALL_FUNCTION(dcExceptionClass_marshallNode);
REGISTER_FUNCTION(dcExceptionClass_registerNode);
UNMARSHALL_FUNCTION(dcExceptionClass_unmarshallNode);

// taffy c methods //
TAFFY_C_METHOD(dcExceptionClass_asString);
TAFFY_C_METHOD(dcExceptionClass_data);
TAFFY_C_METHOD(dcExceptionClass_init);
TAFFY_C_METHOD(dcExceptionClass_setData);
TAFFY_C_METHOD(dcExceptionClass_setDataObject);

// helpers //
char *dcExceptionClass_asString_helper(struct dcNode_t *_node,
                                       const char *_exceptionString);

char *dcExceptionClass_asString_helperWithClassname
    (struct dcNode_t *_node,
     const char *_className,
     const char *_exceptionString);

#define EXCEPTION_CLASS_NAME "Exception"

#endif
