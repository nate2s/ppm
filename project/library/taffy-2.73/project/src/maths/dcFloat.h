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

#ifndef __DC_FLOAT_H__
#define __DC_FLOAT_H__

#include "dcDefines.h"

//////////////
// creating //
//////////////

/**
 * Creates a dcInt-node
 * \param _value The value
 * \return A newly created dcInt-node with value _value
 */
struct dcNode_t *dcFloat_createNode(float _value);

/////////////
// getting //
/////////////

/**
 * Gets the value
 * \param _intNode The dcFloat-node
 * \return The float value of _floatNode
 */
float dcFloat_getFloat(const struct dcNode_t *_floatNode);

////////////////////////
// standard functions //
////////////////////////

COPY_FUNCTION(dcFloat_copyNode);
COMPARE_FUNCTION(dcFloat_compareNode);
HASH_FUNCTION(dcFloat_hashNode);
MARSHALL_FUNCTION(dcFloat_marshallNode);
UNMARSHALL_FUNCTION(dcFloat_unmarshallNode);
PRINT_FUNCTION(dcFloat_printNode);

/////////////////
// marshalling //
/////////////////

bool dcFloat_unmarshall(struct dcString_t *_stream, float *_number);
struct dcString_t *dcFloat_marshall(float _number, struct dcString_t *_stream);

#endif
