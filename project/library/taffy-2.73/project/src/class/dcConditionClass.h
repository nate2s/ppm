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

#ifndef __DC_CONDITION_CLASS_H__
#define __DC_CONDITION_CLASS_H__

#include "dcDefines.h"

/////////////////////////
// dcConditionClassAux //
/////////////////////////

struct dcConditionClassAux_t
{
    struct dcCondition_t *condition;
};

typedef struct dcConditionClassAux_t dcConditionClassAux;

//////////////////////
// dcConditionClass //
//////////////////////

// creating //
struct dcNode_t *dcConditionClass_createNode(bool _object);
struct dcNode_t *dcConditionClass_createObject(void);

struct dcCondition_t *dcConditionClass_getCondition(struct dcNode_t *_node);

// standard functions //
ALLOCATE_FUNCTION(dcConditionClass_allocateNode);
COPY_FUNCTION(dcConditionClass_copyNode);
FREE_FUNCTION(dcConditionClass_freeNode);
GET_TEMPLATE_FUNCTION(dcConditionClass_getTemplate);

// taffy c methods //
TAFFY_C_METHOD(dcConditionClass_asString);
TAFFY_C_METHOD(dcConditionClass_signal);
TAFFY_C_METHOD(dcConditionClass_broadcast);
TAFFY_C_METHOD(dcConditionClass_wait);

#define CONDITION_PACKAGE_NAME THREADING_PACKAGE_NAME
#define CONDITION_CLASS_NAME "Condition"

#endif
