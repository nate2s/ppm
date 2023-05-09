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

#ifndef __DC_IN_H__
#define __DC_IN_H__

#include "dcDefines.h"

struct dcIn_t
{
    struct dcNode_t *left;
    struct dcArray_t *array;
};

typedef struct dcIn_t dcIn;

//////////////
// creating //
//////////////

dcIn *dcIn_create(struct dcNode_t *_left,
                  struct dcArray_t *_array);

struct dcNode_t *dcIn_createNode(struct dcNode_t *_left,
                                 struct dcArray_t *_array);

/////////////
// freeing //
/////////////

void dcIn_free(dcIn **_if, dcDepth _depth);

/////////////////
// marshalling //
/////////////////

// standard functions //
COPY_FUNCTION(dcIn_copyNode);
FREE_FUNCTION(dcIn_freeNode);
MARK_FUNCTION(dcIn_markNode);
MARSHALL_FUNCTION(dcIn_marshallNode);
PRINT_FUNCTION(dcIn_printNode);
UNMARSHALL_FUNCTION(dcIn_unmarshallNode);

#endif
