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

#ifndef __DC_MATH_CLASS_H__
#define __DC_MATH_CLASS_H__

#include "dcDefines.h"

// creating //
struct dcNode_t *dcMathClass_createNode(bool _object);
struct dcNode_t *dcMathClass_createObject(void);

// standard functions //
GET_TEMPLATE_FUNCTION(dcMathClass_getTemplate);
DEINITIALIZE_FUNCTION(dcMathClass_deinitialize);
INITIALIZE_FUNCTION(dcMathClass_initialize);

//
// Taffy-linked C functions
//
TAFFY_C_METHOD(dcMathClass_atan);
TAFFY_C_METHOD(dcMathClass_chooseAnd);
TAFFY_C_METHOD(dcMathClass_ln);
TAFFY_C_METHOD(dcMathClass_log);
TAFFY_C_METHOD(dcMathClass_lg);
TAFFY_C_METHOD(dcMathClass_log10);
TAFFY_C_METHOD(dcMathClass_logBase);
TAFFY_C_METHOD(dcMathClass_nAn);
TAFFY_C_METHOD(dcMathClass_random);
TAFFY_C_METHOD(dcMathClass_sin);

struct dcNode_t *dcMathClass_getSinObject(void);
struct dcNode_t *dcMathClass_getCosObject(void);

#define MATH_PACKAGE_NAME MATHS_PACKAGE_NAME
#define MATH_CLASS_NAME "Math"

#endif
