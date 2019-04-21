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

#ifndef __DC_FUNCTION_UPDATE_H__
#define __DC_FUNCTION_UPDATE_H__

#include "dcDefines.h"

struct dcFunctionUpdate_t
{
    struct dcNode_t *identifier;
    struct dcList_t *arguments;
    struct dcNode_t *arithmetic;
};

typedef struct dcFunctionUpdate_t dcFunctionUpdate;

//////////////
// creating //
//////////////

struct dcNode_t *dcFunctionUpdate_createNode(struct dcNode_t *_identifier,
                                             struct dcList_t *_arguments,
                                             struct dcNode_t *_arithmetic);

/////////////
// getting //
/////////////

struct dcNode_t *dcFunctionUpdate_getIdentifier
    (const struct dcNode_t *_functionUpdate);
struct dcList_t *dcFunctionUpdate_getArguments
    (const struct dcNode_t *_functionUpdate);
struct dcNode_t *dcFunctionUpdate_getArithmetic
    (const struct dcNode_t *_functionUpdate);

////////////////////////
// standard functions //
////////////////////////

COPY_FUNCTION(dcFunctionUpdate_copyNode);
FREE_FUNCTION(dcFunctionUpdate_freeNode);
MARSHALL_FUNCTION(dcFunctionUpdate_marshallNode);
UNMARSHALL_FUNCTION(dcFunctionUpdate_unmarshallNode);

#endif
