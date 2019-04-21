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

#ifndef __DC_FOR_H__
#define __DC_FOR_H__

#include "dcDefines.h"

struct dcFor_t
{
    // a list of expressions
    struct dcNode_t *initial;

    // an expression
    struct dcNode_t *condition;

    // a list of expressions
    struct dcNode_t *increment;

    // a statement
    struct dcNode_t *statement;
};

typedef struct dcFor_t dcFor;

dcFor *dcFor_create(struct dcNode_t *_initial,
                    struct dcNode_t *_condition,
                    struct dcNode_t *_increment,
                    struct dcNode_t *_statement);

// creating //
struct dcNode_t *dcFor_createNode(struct dcNode_t *_initial,
                                  struct dcNode_t *_condition,
                                  struct dcNode_t *_inrement,
                                  struct dcNode_t *_statement);

// gets //
struct dcNode_t *dcFor_getInitial(const struct dcNode_t *_forNode);
struct dcNode_t *dcFor_getCondition(const struct dcNode_t *_forNode);
struct dcNode_t *dcFor_getIncrement(const struct dcNode_t *_forNode);
struct dcNode_t *dcFor_getStatement(const struct dcNode_t *_forNode);

// standard functions //
COPY_FUNCTION(dcFor_copyNode);
FREE_FUNCTION(dcFor_freeNode);
MARK_FUNCTION(dcFor_markNode);
MARSHALL_FUNCTION(dcFor_marshallNode);
UNMARSHALL_FUNCTION(dcFor_unmarshallNode);
PRINT_FUNCTION(dcFor_printNode);

#endif
