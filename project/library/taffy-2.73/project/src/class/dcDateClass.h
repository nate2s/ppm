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

#ifndef __DC_DATE_CLASS_H__
#define __DC_DATE_CLASS_H__

#include <time.h>

#include "dcDefines.h"

////////////////////
// dcDateClassAux //
////////////////////

struct dcDateClassAux_t
{
    time_t time;
};

typedef struct dcDateClassAux_t dcDateClassAux;

/////////////////
// dcDateClass //
/////////////////

// creating //
struct dcNode_t *dcDateClass_createNode(bool _object);
struct dcNode_t *dcDateClass_createObject(void);

// standard functions //
ALLOCATE_FUNCTION(dcDateClass_allocateNode);
COPY_FUNCTION(dcDateClass_copyNode);
FREE_FUNCTION(dcDateClass_freeNode);
GET_TEMPLATE_FUNCTION(dcDateClass_getTemplate);

// taffy methods //
TAFFY_C_METHOD(dcDateClass_day);
TAFFY_C_METHOD(dcDateClass_equalEqual);
TAFFY_C_METHOD(dcDateClass_fullDay);
TAFFY_C_METHOD(dcDateClass_fullMonth);
TAFFY_C_METHOD(dcDateClass_greaterThan);
TAFFY_C_METHOD(dcDateClass_greaterThanOrEqual);
TAFFY_C_METHOD(dcDateClass_hour);
TAFFY_C_METHOD(dcDateClass_init);
TAFFY_C_METHOD(dcDateClass_lessThan);
TAFFY_C_METHOD(dcDateClass_lessThanOrEqual);
TAFFY_C_METHOD(dcDateClass_minute);
TAFFY_C_METHOD(dcDateClass_month);
TAFFY_C_METHOD(dcDateClass_year);
TAFFY_C_METHOD(dcDateClass_second);

#endif
