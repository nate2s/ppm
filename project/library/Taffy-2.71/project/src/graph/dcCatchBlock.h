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

#ifndef __DC_CATCHBLOCK_H__
#define __DC_CATCHBLOCK_H__

#include "dcDefines.h"

/**
 * Structure holding data useful for Taffy catch blocks
 */
struct dcCatchBlock_t
{
    /**
     * The identifier, or variable name, of the catch variable \n
     * \n
     * Example: \n
     * \n
     * catch (UnidentifiedMethodException e) \n
     * \n
     * 'e' is the identifier
     */
    struct dcNode_t *identifier;

    /**
     * The type of the catch variable \n
     * \n
     * Example: \n
     * \n
     * catch (UnidentifiedMethodException e) \n
     * \n
     * 'UnidentfifiedMethodException' is the type
     */
    struct dcNode_t *type;

    /**
     * The statement of the catch block \n
     * \n
     * Example: \n
     * \n
     * catch (UnidentifiedMethodException e) \n
     * { \n
     * \<statement\> \n
     * } \n
     */
    struct dcNode_t *statement;
};

typedef struct dcCatchBlock_t dcCatchBlock;

//////////////
// creating //
//////////////

/**
 * Creates a dcCatchBlock-node stuffed with the given arguments
 * \param _identifier The value for the identifier field
 * \param _type The value for the type field
 * \param _statement The value for the statement field
 */
struct dcNode_t *dcCatchBlock_createNode(struct dcNode_t *_identifier,
                                         struct dcNode_t *_type,
                                         struct dcNode_t *_statement);

////////////////////////
// standard functions //
////////////////////////

FREE_FUNCTION(dcCatchBlock_freeNode);
COPY_FUNCTION(dcCatchBlock_copyNode);
MARSHALL_FUNCTION(dcCatchBlock_marshallNode);
UNMARSHALL_FUNCTION(dcCatchBlock_unmarshallNode);

#endif
