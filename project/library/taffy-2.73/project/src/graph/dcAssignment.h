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

#ifndef __DC_ASSIGNMENT_H__
#define __DC_ASSIGNMENT_H__

#include "dcDefines.h"

/**
 * A structure containing data useful for performing
 * an assigment in Taffy code
 */
struct dcAssignment_t
{
    dcScopeDataFlags flags;

    /**
     * The identifier of the assignment
     * example: \n
     * a = 1 \n
     * 'a' is the identifier
     */
    struct dcNode_t *identifier;

    /**
     * The value of the assignment
     * example: \n
     * a = 1 \n
     * '1' is the value
     */
    struct dcNode_t *value;
};

typedef struct dcAssignment_t dcAssignment;

//////////////
// creating //
//////////////

/**
 * Creates a dcAssignment object.
 * For each parameter below, see struct dcAssingment for placement
 * \param _identifier A dcIdentifier-node for the identifier field
 * \param _value Used to set the value field
 * \param _global Used to set the global field
 * \param _constant Used to set the constant field
 * \return A newly allocated dcAssignment object, stuffed with the given values
 */
dcAssignment *dcAssignment_create(struct dcNode_t *_identifier,
                                  struct dcNode_t *_value,
                                  dcScopeDataFlags _flags);

/**
 * Creates a dcAssignment-node object.
 * For each parameter below, see struct dcAssingment for placement
 * \param _identifier A dcNode(dcGraphData(dcIdentifier))
 * for the identifier field
 * \param _value Used to set the value field
 * \param _global Used to set the global field
 * \param _constant Used to set the constant field
 * \return A newly allocated dcAssignment-node object,
 * stuffed with the given values
 */
struct dcNode_t *dcAssignment_createNode(struct dcNode_t *_identifier,
                                         struct dcNode_t *_value,
                                         dcScopeDataFlags _flags);

//////////////////////
// getting/querying //
//////////////////////

/**
 * Gets the identifier field from a dcAssignment object
 * \param _assignment The dcAssignment object to query
 * \return The identifier field of type dcNode(dcGraphData(dcIdentifier))
 */
struct dcNode_t *dcAssignment_getIdentifier(const struct dcNode_t *_assignment);

/**
 * Gets the value field from a dcAssignment object
 * \param _assignment The dcAssignment object to query
 * \return The value field
 */
struct dcNode_t *dcAssignment_getValue(const struct dcNode_t *_assignment);

dcScopeDataFlags dcAssignment_getFlags(const struct dcNode_t *_assigment);

/**
 * Gets the global field from a dcAssignment object
 * \param _assignment The dcAssignment object to query
 * \return The global field
 */
bool dcAssignment_isGlobal(const dcAssignment *_assignment);

/**
 * Gets the constant field from a dcAssignment object
 * \param _assignment The dcAssignment object to query
 * \return The constant field
 */
bool dcAssignment_isConstant(const dcAssignment *_assignment);

/////////////////
// marshalling //
/////////////////

#define ASSIGNMENT_MARSHALL_SIZE 5

// standard functions //
COPY_FUNCTION(dcAssignment_copyNode);
FREE_FUNCTION(dcAssignment_freeNode);
MARK_FUNCTION(dcAssignment_markNode);
MARSHALL_FUNCTION(dcAssignment_marshallNode);
PRINT_FUNCTION(dcAssignment_printNode);
UNMARSHALL_FUNCTION(dcAssignment_unmarshallNode);

#endif
