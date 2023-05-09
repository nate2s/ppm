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

#ifndef __DC_UNSIGNED_INT_64_H__
#define __DC_UNSIGNED_INT_64_H__

#include "dcDefines.h"

//////////////
// creating //
//////////////

/**
 * Creates a dcInt-node
 * \param _value The value
 * \return A newly created dcInt-node with value _value
 */
struct dcNode_t *dcUnsignedInt64_createNode(uint64_t _value);

/////////////
// getting //
/////////////

/**
 * Gets the int value of a dcInt-node
 * \param _intNode The dcInt-node
 * \return The int value of _intNode
 */
uint64_t dcUnsignedInt64_getInt(const struct dcNode_t *_intNode);

////////////////////////
// standard functions //
////////////////////////

COPY_FUNCTION(dcUnsignedInt64_copyNode);
COMPARE_FUNCTION(dcUnsignedInt64_compareNode);
HASH_FUNCTION(dcUnsignedInt64_hashNode);
MARSHALL_FUNCTION(dcUnsignedInt64_marshallNode);
UNMARSHALL_FUNCTION(dcUnsignedInt64_unmarshallNode);
PRINT_FUNCTION(dcUnsignedInt64_printNode);

/////////////////
// marshalling //
/////////////////

struct dcString_t *dcUnsignedInt64_marshall(uint64_t _number,
                                            struct dcString_t *_stream);
bool dcUnsignedInt64_unmarshall(struct dcString_t *_stream,
                                uint64_t *_number);

#endif
