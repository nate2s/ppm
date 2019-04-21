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

#ifndef __DC_WHILE_H__
#define __DC_WHILE_H__

#include "dcDefines.h"

// creating //
struct dcNode_t *dcWhile_createNode(struct dcNode_t *_condition,
                                    struct dcNode_t *_statement);

// gets //
struct dcNode_t *dcWhile_getCondition(const struct dcNode_t *_whileNode);
struct dcNode_t *dcWhile_getStatement(const struct dcNode_t *_whileNode);

PRINT_FUNCTION(dcWhile_printNode);

#endif
