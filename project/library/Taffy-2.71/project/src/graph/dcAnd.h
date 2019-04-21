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

#ifndef __DC_AND_H__
#define __DC_AND_H__

#include "dcDefines.h"

//////////////
// creating //
//////////////

/**
 * Creates a dcAnd-node object for anding \n
 * example: if (objectA and objectB) ....
 * \param _left The left-hand-side of the and
 * \param _right The right-hand-side of the and
 * \return A newly allocated dcAnd-node object
 */
struct dcNode_t *dcAnd_createNode(struct dcNode_t *_left,
                                  struct dcNode_t *_right);

//////////
// gets //
//////////

/**
 * Gets the left-hand-side of a dcAnd-node object
 * \param _andNode The dcAnd-node object to query
 * \return The left-hand-side of _andNode
 */
struct dcNode_t *dcAnd_getLeft(const struct dcNode_t *_andNode);

/**
 * Gets the right-hand-side of a dcAnd-node object
 * \param _andNode The dcAnd-node object to query
 * \return The right-hand-side of _andNode
 */
struct dcNode_t *dcAnd_getRight(const struct dcNode_t *_andNode);

#endif
