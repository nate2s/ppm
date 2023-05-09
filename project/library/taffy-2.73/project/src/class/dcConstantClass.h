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

#ifndef __DC_CONSTANT_CLASS_H__
#define __DC_CONSTANT_CLASS_H__

#include "dcDefines.h"

TAFFY_C_METHOD(dcConstantClass_add);
TAFFY_C_METHOD(dcConstantClass_divide);
TAFFY_C_METHOD(dcConstantClass_modulus);
TAFFY_C_METHOD(dcConstantClass_multiply);
TAFFY_C_METHOD(dcConstantClass_raise);
TAFFY_C_METHOD(dcConstantClass_subtract);

GET_TEMPLATE_FUNCTION(dcConstantClass_getTemplate);
DO_GRAPH_OPERATION_FUNCTION(dcConstantClass_doGraphOperation);

#define CONSTANT_PACKAGE_NAME MATHS_PACKAGE_NAME
#define CONSTANT_CLASS_NAME "Constant"

#endif
