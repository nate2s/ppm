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

#ifndef __DC_EXIT_H__
#define __DC_EXIT_H__

#include "dcDefines.h"

//////////////
// creating //
//////////////

/**
 * Creates a dcExit-node (dcNode(dcGraphData(dcNode))), used for the 'exit'
 * operation
 * \param _exitValue The exit value
 * \return A newly allocated dcExit-node, stuffed with _exitValue
 */
struct dcNode_t *dcExit_createNode(struct dcNode_t *_exitValue);

/////////////
// getting //
/////////////

/**
 * Gets the exit value from a dcExit-node
 * \param _exitNode The dcExit-node to query
 * \return The exit value of _exitNode
 */
struct dcNode_t *dcExit_getValueFromNode(const struct dcNode_t *_exitNode);

// standard functions //
PRINT_FUNCTION(dcExit_printNode);
FREE_FUNCTION(dcExit_freeNode);
COPY_FUNCTION(dcExit_copyNode);

#endif
