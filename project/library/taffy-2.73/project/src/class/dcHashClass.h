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

#ifndef __DC_HASH_CLASS_H__
#define __DC_HASH_CLASS_H__

#include "dcDefines.h"

////////////////////
// dcHashClassAux //
////////////////////

struct dcHashClassUninitializedData_t
{
    struct dcList_t *keys;
    struct dcList_t *values;
};

typedef struct dcHashClassUninitializedData_t dcHashClassUninitializedData;

struct dcHashClassAux_t
{
    union types_t
    {
        struct dcHash_t *hash;
        dcHashClassUninitializedData *uninitializedData;
    } types;

    bool initialized;
};

typedef struct dcHashClassAux_t dcHashClassAux;

/////////////////
// dcHashClass //
/////////////////

// creating //
struct dcNode_t *dcHashClass_createInitializedNode(struct dcHash_t *_hash,
                                                   bool _object);

struct dcNode_t *dcHashClass_createUninitializedNode(struct dcList_t *_keys,
                                                     struct dcList_t *_vals,
                                                     bool _object);

struct dcNode_t *dcHashClass_createUninitializedObject(struct dcList_t *_keys,
                                                       struct dcList_t *_vals);

struct dcNode_t *dcHashClass_createInitializedObject(struct dcHash_t *_hash);

struct dcNode_t * dcHashClass_createObjectFromInitializedLists
    (struct dcList_t *_keys,
     struct dcList_t *_values,
     struct dcNode_t *_exception,
     struct dcNodeEvaluator_t *_evaluator);

// initializating //
void dcHashClass_initializeObject(struct dcNode_t *_hash,
                                  struct dcList_t *_keys,
                                  struct dcList_t *_values);

// standard functions //
ALLOCATE_FUNCTION(dcHashClass_allocateNode);
COPY_FUNCTION(dcHashClass_copyNode);
DEALLOCATE_FUNCTION(dcHashClass_deallocateNode);
DEINITIALIZE_FUNCTION(dcHashClass_deinitialize);
FREE_FUNCTION(dcHashClass_freeNode);
GET_TEMPLATE_FUNCTION(dcHashClass_getTemplate);
INITIALIZE_FUNCTION(dcHashClass_initialize);
MARK_FUNCTION(dcHashClass_markNode);
MARSHALL_FUNCTION(dcHashClass_marshallNode);
PRINT_FUNCTION(dcHashClass_printNode);
REGISTER_FUNCTION(dcHashClass_registerNode);
UNMARSHALL_FUNCTION(dcHashClass_unmarshallNode);

// querying/getting //
bool dcHashClass_isInitialized(const struct dcNode_t *_node);
struct dcHash_t *dcHashClass_getHash(const struct dcNode_t *_node);
struct dcList_t *dcHashClass_getTempKeys(const struct dcNode_t *_node);
struct dcList_t *dcHashClass_getTempValues(const struct dcNode_t *_nod);
dcContainerSizeType dcHashClass_getSize(const struct dcNode_t *_node);

// misc //
dcResult dcHashClass_hashifyNode(struct dcNode_t *_node,
                                 dcHashType *_hashResult);

// taffy c methods //
TAFFY_C_METHOD(dcHashClass_asString);
TAFFY_C_METHOD(dcHashClass_each);
TAFFY_C_METHOD(dcHashClass_eachKey);
TAFFY_C_METHOD(dcHashClass_eachValue);
TAFFY_C_METHOD(dcHashClass_object);
TAFFY_C_METHOD(dcHashClass_remove);
TAFFY_C_METHOD(dcHashClass_setKeyForObject);
TAFFY_C_METHOD(dcHashClass_setValueForKey);
TAFFY_C_METHOD(dcHashClass_size);

/**
 * @brief The link between Hash Class land and Hash land
 */
struct dcNode_t *dcHashClass_setHashValue(struct dcNode_t *_hashObject,
                                          struct dcNode_t *_key,
                                          struct dcNode_t *_value);

#define HASH_PACKAGE_NAME CONTAINER_CLASS_NAME
#define HASH_CLASS_NAME "Hash"

#endif
