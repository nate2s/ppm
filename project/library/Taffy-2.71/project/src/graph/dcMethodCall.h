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

#ifndef __DC_METHOD_CALL_H__
#define __DC_METHOD_CALL_H__

#include "dcDefines.h"

/**
 * A structure that holds data for a Taffy method call
 */
struct dcMethodCall_t
{
    /**
     * The receiver
     *
     * Example:
     *
     * [dog sit]
     * dog is the receiver
     */
    struct dcNode_t *receiver;

    /**
     * The method name
     */
    char *methodName;

    /**
     * The arguments, each is of type dcNode(dcGraphData(*))
     */
    struct dcList_t *arguments;
};

typedef struct dcMethodCall_t dcMethodCall;

//////////////
// creating //
//////////////

/**
 * Creates a dcMethodCall
 * @param _receiver The value for the receiver field
 * @param _methodName The value for the methodName field
 * @param _arguments The value for the arguments field
 * @return A newly allocated dcMethodCall, stuffed with given arguments
 */
dcMethodCall *dcMethodCall_create(struct dcNode_t *_receiver,
                                  const char *_methodName,
                                  struct dcList_t *_arguments);

/**
 * Creates a dcMethodCall-node
 * @param _receiver The value for the receiver field
 * @param _methodName The value for the methodName field
 * @param _arguments The value for the arguments field
 * @return A newly allocated dcMethodCall-node,
 *         stuffed with given parameters
 */
struct dcNode_t *dcMethodCall_createNode(struct dcNode_t *_receiver,
                                         const char *_methodName,
                                         struct dcList_t *_arguments);

/**
 * Creates a dcMethodCall-node
 * @param _receiver The value for the receiver field
 * @param _methodName The value for the methodName field
 * @param _argument A value for the arguments field
 * @return A newly allocated dcMethodCall-node,
 *         stuffed with given parameters
 */
struct dcNode_t *dcMethodCall_createNodeWithArgument
    (struct dcNode_t *_receiver,
     const char *_methodName,
     struct dcNode_t *_argument);

/**
 * Creates a dcMethodCall-node
 * @param _receiver The value for the receiver field
 * @param _methodName The value for the methodName field
 * @return A newly allocated dcMethodCall-node,
 *         with no arguments, stuffed with given arguments
 */
struct dcNode_t *dcMethodCall_createNodeWithNoArguments
    (struct dcNode_t *_receiver,
     const char *_methodName);

struct dcNode_t *dcMethodCall_createShell(dcMethodCall *_call);

/////////////
// copying //
/////////////

dcMethodCall *dcMethodCall_copy(const dcMethodCall *_from, dcDepth _depth);

/////////////
// getting //
/////////////

/**
 * Gets the methodName field from a dcMethodCall
 * @param _methodCall The dcMethodCall to query
 * @return The methodName field of _methodCall
 */
const char *dcMethodCall_getMethodName(const struct dcNode_t *_methodCall);

/**
 * Gets the receiver field from a dcMethodCall
 * @param _methodCall The dcMethodCall to query
 * @return The receiver field of _methodCall
 */
struct dcNode_t *dcMethodCall_getReceiver(const struct dcNode_t *_methodCall);

/**
 * Gets the arguments field from a dcMethodCall
 * @param _methodCall The dcMethodCall to query
 * @return The arguments field of _methodCall
 */
struct dcList_t *dcMethodCall_getArguments(const struct dcNode_t *_methodCall);

/////////////
// setting //
/////////////

void dcMethodCall_setMethodName
    (dcMethodCall *_methodCall, const char *_newName);

// standard functions //
COMPARE_FUNCTION(dcMethodCall_compareNode);
COPY_FUNCTION(dcMethodCall_copyNode);
FREE_FUNCTION(dcMethodCall_freeNode);
HASH_FUNCTION(dcMethodCall_hashNode);
MARK_FUNCTION(dcMethodCall_markNode);
MARSHALL_FUNCTION(dcMethodCall_marshallNode);
PRETTY_PRINT_FUNCTION(dcMethodCall_prettyPrintNode);
PRINT_FUNCTION(dcMethodCall_printNode);
UNMARSHALL_FUNCTION(dcMethodCall_unmarshallNode);

#endif
