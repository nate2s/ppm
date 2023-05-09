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

#ifndef __DC_IF_H__
#define __DC_IF_H__

#include "dcDefines.h"

/**
 * Structure used to store information for a Taffy if-else-block
 */
struct dcIf_t
{
    /**
     * The condition of the if \n
     * \n
     * Example: \n
     * \n
     * if (a == 1) {}
     * \n
     * (a == 1) is the condition
     */
    struct dcNode_t *condition;

    /**
     * The statement of the if \n
     * \n
     * Example: \n
     * \n
     * if (a == 1)
     * \n
     * { \n
     * \<statement\> \n
     * } \n
     */
    struct dcNode_t *statement;

    /**
     * The next dcIf block, in dcIf-node form \n
     * This variable can hold data for either an if-else block, or an else block
     */
    struct dcNode_t *next;
};

typedef struct dcIf_t dcIf;

//////////////
// creating //
//////////////

/**
 * Creates a dcIf stuffed with the given arguments
 * \param _condition The condition field
 * \param _statement The statement field
 * \param _next The next field
 * \return A dcIf object stuffed with the given arguments
 */
dcIf *dcIf_create(struct dcNode_t *_condition,
                  struct dcNode_t *_statement,
                  struct dcNode_t *_next);

/**
 * Creates a dcIf-node stuffed with the given arguments
 * \param _condition The condition field
 * \param _statement The statement field
 * \param _next The next field
 * \return A dcIf-node stuffed with the given arguments
 */
struct dcNode_t *dcIf_createNode(struct dcNode_t *_condition,
                                 struct dcNode_t *_statement,
                                 struct dcNode_t *_next);

/////////////
// freeing //
/////////////

void dcIf_free(dcIf **_if, dcDepth _depth);

/////////////
// getting //
/////////////

/**
 * Gets the condition field from a dcIf
 * \param _if The dcIf to query
 * \return The condition field of _if
 */
struct dcNode_t *dcIf_getCondition(const struct dcNode_t *_if);

/**
 * Gets the statement field from a dcIf
 * \param _if The dcIf to query
 * \return The statement field of _if
 */
struct dcNode_t *dcIf_getStatement(const struct dcNode_t *_if);

/**
 * Gets the next field from a dcIf
 * \param _if The dcIf to query
 * \return The next field of _if
 */
struct dcNode_t *dcIf_getNext(const struct dcNode_t *_if);

/////////////////
// marshalling //
/////////////////

#define IF_HAS_NEXT 27
#define IF_HAS_NO_NEXT 28

// standard functions //
COPY_FUNCTION(dcIf_copyNode);
FREE_FUNCTION(dcIf_freeNode);
MARK_FUNCTION(dcIf_markNode);
MARSHALL_FUNCTION(dcIf_marshallNode);
PRINT_FUNCTION(dcIf_printNode);
UNMARSHALL_FUNCTION(dcIf_unmarshallNode);

#endif
