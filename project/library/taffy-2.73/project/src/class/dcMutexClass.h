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

#ifndef __DC_MUTEX_CLASS_H__
#define __DC_MUTEX_CLASS_H__

#include "dcDefines.h"

/////////////////////
// dcMutexClassAux //
/////////////////////

struct dcMutexClassAux_t
{
    struct dcMutex_t *mutex;
};

typedef struct dcMutexClassAux_t dcMutexClassAux;

//////////////////
// dcMutexClass //
//////////////////

// creating //
struct dcNode_t *dcMutexClass_createNode(bool _object);
struct dcNode_t *dcMutexClass_createObject(void);

struct dcMutex_t *dcMutexClass_getMutex(struct dcNode_t *_node);

// standard functions //
ALLOCATE_FUNCTION(dcMutexClass_allocateNode);
COPY_FUNCTION(dcMutexClass_copyNode);
FREE_FUNCTION(dcMutexClass_freeNode);
GET_TEMPLATE_FUNCTION(dcMutexClass_getTemplate);

// taffy c methods //
TAFFY_C_METHOD(dcMutexClass_asString);
TAFFY_C_METHOD(dcMutexClass_init);
TAFFY_C_METHOD(dcMutexClass_initWithReentrant);
TAFFY_C_METHOD(dcMutexClass_lock);
TAFFY_C_METHOD(dcMutexClass_unlock);

#define MUTEX_PACKAGE_NAME THREADING_PACKAGE_NAME
#define MUTEX_CLASS_NAME "Mutex"

#endif
