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

#ifndef __DC_EQUATION_CLASS_H__
#define __DC_EQUATION_CLASS_H__

#include "dcDefines.h"

////////////////////////
// dcEquationClassAux //
////////////////////////

struct dcEquationClassAux_t
{
    struct dcNode_t *left;
    struct dcNode_t *right;
};

typedef struct dcEquationClassAux_t dcEquationClassAux;

/////////////////////
// dcEquationClass //
/////////////////////

// creating //
struct dcNode_t *dcEquationClass_createObject(struct dcNode_t *_left,
                                              struct dcNode_t *_right);

GET_TEMPLATE_FUNCTION(dcEquationClass_getTemplate);

// external standard equations //
IS_ME_FUNCTION(dcEquationClass_isMe);

#define EQUATION_PACKAGE_NAME MATHS_PACKAGE_NAME
#define EQUATION_CLASS_NAME "Equation"

#endif
