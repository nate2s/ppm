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

#ifndef __DC_SUPER_H__
#define __DC_SUPER_H__

#include "dcDefines.h"

//////////////
// creating //
//////////////

/**
 * Creates a dcSuper-node
 * \return A newly allocated dcSuper-node
 */
struct dcNode_t *dcSuper_createNode(void);

// standard functions //
PRINT_FUNCTION(dcSuper_printNode);

#endif
