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

#ifndef __DC_SYMBOL_H__
#define __DC_SYMBOL_H__

#include "dcDefines.h"

/**
 * Structure used to store information of a symbol
 */
struct dcSymbol_t
{
    /**
     * The name of the symbol
     */
    char *symbol;
};

typedef struct dcSymbol_t dcSymbol;

//////////////
// creating //
//////////////

/**
 * Creates a dcSymbol with given name
 * \param _name The name
 * \return A newly allocated dcSymbol with given name
 */
dcSymbol *dcSymbol_create(const char *_name);

/**
 * Creates a dcSymbol-node with given name
 * \param _name The name
 * \return A newly allocated dcSymbol-node with given name
 */
struct dcNode_t *dcSymbol_createNode(const char *_name);

/////////////
// freeing //
/////////////

/**
 * Frees a dcSymbol and potentially its contents
 * \param _symbol The dcSymbol to free
 * \param _depth The depth of the free
 */
void dcSymbol_free(dcSymbol **_symbol, dcDepth _depth);

/////////////
// copying //
/////////////

/**
 * Creates a copy of a dcSymbol
 * \param _from The dcSymbol to copy
 * \param _depth The depth of the copy
 * \return A copy of _from
 */
dcSymbol *dcSymbol_copy(const dcSymbol *_from, dcDepth _depth);

/////////////
// getting //
/////////////

/**
 * Gets the name of a dcSymbol
 * \param _symbol The dcSymbol to query
 * \return The name of _symbol
 */
const char *dcSymbol_getSymbol(const struct dcNode_t *_symbol);

bool dcSymbol_equals(const dcSymbol *_left, const dcSymbol *_right);

// standard functions //
FREE_FUNCTION(dcSymbol_freeNode);
COPY_FUNCTION(dcSymbol_copyNode);
PRINT_FUNCTION(dcSymbol_printNode);

#endif
