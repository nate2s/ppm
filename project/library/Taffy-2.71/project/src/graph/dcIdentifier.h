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

#ifndef __DC_IDENTIFIER_H__
#define __DC_IDENTIFIER_H__

#include "dcDefines.h"

/**
 * Structure used to store information of a Taffy identifier
 */
struct dcIdentifier_t
{
    dcScopeDataFlags scopeDataFlags;

    /**
     * The name of the identifier
     */
    char *name;
};

typedef struct dcIdentifier_t dcIdentifier;

//////////////
// creating //
//////////////

/**
 * @brief Creates a dcIdentifier with given name
 * @param _name The name
 * @return A newly allocated dcIdentifier with given name
 */
dcIdentifier *dcIdentifier_create(const char *_name,
                                  dcScopeDataFlags _flags);

/**
 * @brief Creates a dcIdentifier-node with given name
 * @param _name The name
 * @return A newly allocated dcIdentifier-node with given name
 */
struct dcNode_t *dcIdentifier_createNode(const char *_name,
                                         dcScopeDataFlags _flags);

/////////////
// freeing //
/////////////

/**
 * @brief Frees a dcIdentifier and potentially its contents
 * @param _identifier The dcIdentifier to free
 * @param _depth The depth of the free
 */
void dcIdentifier_free(dcIdentifier **_identifier, dcDepth _depth);

/////////////
// copying //
/////////////

/**
 * @brief Creates a copy of a dcIdentifier
 * @param _from The dcIdentifier to copy
 * @param _depth The depth of the copy
 * @return A copy of _from
 */
dcIdentifier *dcIdentifier_copy(const dcIdentifier *_from, dcDepth _depth);

/////////////
// getting //
/////////////

/**
 * Gets the name of a dcIdentifier
 * @param _identifier The dcIdentifier node to query
 * @return The name of _identifier
 */
const char *dcIdentifier_getName(const struct dcNode_t *_identifier);

/**
 * @brief Gets the flags of a dcIdentifier
 * @param _identifier The dcIdentifier to query
 * @return The flags of _identifier
 */
dcScopeDataFlags dcIdentifier_getScopeDataFlags
    (const struct dcNode_t *_identifier);

bool dcIdentifier_equals(const dcIdentifier *_left,
                         const dcIdentifier *_right);

/**
 * @brief Tests whether the given identifier equals a char * string
 * @return true if the identifier's name equals the given string
 */
bool dcIdentifier_equalsString(const struct dcNode_t *_left,
                               const char *_name);

/**
 * @brief Returns true if _node is an identifier
 */
bool dcIdentifier_isMe(const struct dcNode_t *_node);

// standard functions //
COPY_FUNCTION(dcIdentifier_copyNode);
FREE_FUNCTION(dcIdentifier_freeNode);
HASH_FUNCTION(dcIdentifier_hashNode);
PRINT_FUNCTION(dcIdentifier_printNode);
COMPARE_FUNCTION(dcIdentifier_compareNode);
MARSHALL_FUNCTION(dcIdentifier_marshallNode);
UNMARSHALL_FUNCTION(dcIdentifier_unmarshallNode);

/////////////////
// marshalling //
/////////////////

#define IDENTIFIER_SCOPE_INDEX 0
#define IDENTIFIER_LENGTH_INDEX 1
#define IDENTIFIER_NAME_INDEX 2

dcIdentifier *dcIdentifier_unmarshall(struct dcString_t *_stream);
struct dcString_t *dcIdentifier_marshall(dcIdentifier *_identifer,
                                         struct dcString_t *_stream);

#endif
