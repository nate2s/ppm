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

#ifndef __DC_NEW_H__
#define __DC_NEW_H__

#include "dcDefines.h"

//////////////
// creating //
//////////////

/**
 * Constructs a dcNew-node, used for the 'new' operation
 * \param _className The name of the class for the 'new' operation
 * \return A dcNew-node with the given class name
 */
struct dcNode_t *dcNew_createNode(struct dcNode_t *_node);

char *dcNew_display(const struct dcNode_t *_node);

// standard functions //
PRINT_FUNCTION(dcNew_printNode);

#endif
