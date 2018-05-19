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

#ifndef __DC_TRYBLOCK_H__
#define __DC_TRYBLOCK_H__

#include "dcDefines.h"

struct dcTryBlock_t
{
    struct dcNode_t *statement;
    struct dcList_t *catches;
};

typedef struct dcTryBlock_t dcTryBlock;

// creating //
dcTryBlock *dcTryBlock_create(struct dcNode_t *_statement,
                              struct dcList_t *_catches);

struct dcNode_t *dcTryBlock_createNode(struct dcNode_t *_statement,
                                       struct dcList_t *_catches);

// getting //
struct dcNode_t *dcTryBlock_getStatement(const struct dcNode_t *_tryBlock);
struct dcList_t *dcTryBlock_getCatches(const struct dcNode_t *_tryBlock);

// standard functions //
COPY_FUNCTION(dcTryBlock_copyNode);
FREE_FUNCTION(dcTryBlock_freeNode);
MARSHALL_FUNCTION(dcTryBlock_marshallNode);
UNMARSHALL_FUNCTION(dcTryBlock_unmarshallNode);

#endif
