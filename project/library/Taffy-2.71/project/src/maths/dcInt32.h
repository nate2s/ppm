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

#ifndef __DC_INT_32_H__
#define __DC_INT_32_H__

#include "dcDefines.h"

//////////////
// creating //
//////////////

/**
 * Creates a dcInt-node
 * \param _value The value
 * \return A newly created dcInt-node with value _value
 */
struct dcNode_t *dcInt32_createNode(int32_t _value);

/////////////
// getting //
/////////////

/**
 * Gets the int value of a dcInt-node
 * \param _intNode The dcInt-node
 * \return The int value of _intNode
 */
int32_t dcInt32_getInt(const struct dcNode_t *_intNode);

////////////////////////
// standard functions //
////////////////////////

COPY_FUNCTION(dcInt32_copyNode);
COMPARE_FUNCTION(dcInt32_compareNode);
HASH_FUNCTION(dcInt32_hashNode);
MARSHALL_FUNCTION(dcInt32_marshallNode);
UNMARSHALL_FUNCTION(dcInt32_unmarshallNode);
PRINT_FUNCTION(dcInt32_printNode);

/////////////////
// marshalling //
/////////////////

bool dcInt32_unmarshallInt16u(struct dcString_t *_stream,
                              uint16_t *_number);
bool dcInt32_unmarshallInt32u(struct dcString_t *_stream,
                              uint32_t *_number);
bool dcInt32_unmarshall(struct dcString_t *_stream,
                        int32_t *_number);
struct dcString_t *dcInt32_marshall(int32_t _number,
                                    struct dcString_t *_stream);
struct dcString_t *dcInt32_marshallInt16u(uint16_t _number,
                                          struct dcString_t *_stream);
struct dcString_t *dcInt32_marshallInt32u(uint32_t _number,
                                          struct dcString_t *_stream);

#endif
