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

#ifndef __DC_HASH_H__
#define __DC_HASH_H__

#include "dcDefines.h"

#define DEFAULT_HASH_CAPACITY 19

struct dcHashIterator_t;

///////////////////
// dcHashElement //
///////////////////

struct dcHashElementKey_t
{
    union dcHashElementKeyType_e
    {
        struct dcNode_t *nodeKey;
        char *stringKey;
    } keyUnion;

    // TODO: change to hasNodeKey
    bool isNodeKey;
    dcHashType hash;
};

typedef struct dcHashElementKey_t dcHashElementKey;

/**
 * dcHash contain these
 */
struct dcHashElement_t
{
    dcHashElementKey key;
    struct dcNode_t *value;
};

typedef struct dcHashElement_t dcHashElement;

/**
 * Standard functions
 */
FREE_FUNCTION(dcHashElement_freeNode);
MARK_FUNCTION(dcHashElement_markNode);
PRINT_FUNCTION(dcHashElement_printNode);
REGISTER_FUNCTION(dcHashElement_registerNode);

////////////
// dcHash //
////////////

typedef uint16_t dcHashCapacityType;

/**
 * A hash container
 */
struct dcHash_t
{
    /**
     * The buckets. An dynamic array of Lists
     */
    struct dcList_t **buckets;

    /**
     * The capacity, or bucket count
     */
    dcHashCapacityType capacity;

    /**
     * The number of elements in the hash
     */
    dcContainerSizeType size;
};

typedef struct dcHash_t dcHash;

//////////////
// creating //
//////////////

/**
 * Allocates a dcHash with default capacity
 * \return A newly allocated dcHash
 */
dcHash *dcHash_create(void);

/**
 * Allocates a dcHash node with default capacity
 * \return A newly allocated dcHash
 */
struct dcNode_t *dcHash_createNode(void);

/**
 * Allocates a dcHash with given bucket
 * \param _bucketSize The bucket size
 * \return A newly allocated dcHash
 */
dcHash *dcHash_createWithCapacity(dcHashCapacityType _capacity);

/////////////
// freeing //
/////////////

/**
 * Frees a dcHash, and potentially the objects it contains
 * \param _hash The dcHash to free
 * \param _depth The depth of the free
 */
void dcHash_free(dcHash **_hash, dcDepth _depth);

/////////////
// setting //
/////////////

/**
 * Sets a value in a hash. Computes a hash value for _key and calls
 * dcHash_setValueWithHashValue().
 */
dcResult dcHash_setValue(dcHash *_hash,
                         struct dcNode_t *_key,
                         struct dcNode_t *_value);

/**
 * Sets a value in a hash.
 */
dcResult dcHash_setValueWithHashValue(dcHash *_hash,
                                      struct dcNode_t *_key,
                                      dcHashType _hashValue,
                                      struct dcNode_t *_value);

dcResult dcHash_setValueWithStringKey(dcHash *_hash,
                                      const char *_key,
                                      struct dcNode_t *_value);

dcResult dcHash_setValueWithStringKeys(dcHash *_hash,
                                       const char *_key,
                                       dcHashType _hashValue,
                                       struct dcNode_t *_value);

/////////////
// getting //
/////////////

/**
 * Gets a Node(HashElement) from a Hash
 * \param _hash The Hash
 * \param _key The _key of the Node to find
 * \param _keyHash The hash of _key
 * \return A Node(HashElement) found using _key, may be NULL
 */
dcResult dcHash_getHashElement(const dcHash *_hash,
                               struct dcNode_t *_key,
                               struct dcNode_t **_result);

dcResult dcHash_getHashElementWithKeyHashResult(const dcHash *_hash,
                                                struct dcNode_t *_key,
                                                struct dcNode_t **_getResult,
                                                dcHashType *_keyHashResult);

dcResult dcHash_getValueWithKeys(const dcHash *_hash,
                                 struct dcNode_t *_key,
                                 dcHashType _keyHash,
                                 struct dcNode_t **_result);

dcResult dcHash_getValue(const dcHash *_hash,
                         struct dcNode_t *_key,
                         struct dcNode_t **_result);

dcResult dcHash_getValueWithStringKey(const dcHash *_hash,
                                      const char *_key,
                                      struct dcNode_t **_value);

dcResult dcHash_getValueWithStringKeys(const dcHash *_hash,
                                       const char *_key,
                                       dcHashType _keyHash,
                                       struct dcNode_t **_value);

dcContainerSizeType dcHash_getSize(const struct dcNode_t *_hash);

/**
 * Gets all values from a dcHash
 * \param _hash The dcHash
 * \return A dcArray containing the values of _hash
 */
struct dcArray_t *dcHash_getValues(const dcHash *_hash);

/////////////
// removes //
/////////////

dcResult dcHash_removeValue(dcHash *_hash,
                            struct dcNode_t *_key,
                            struct dcNode_t **_removed,
                            dcDepth _depth);

dcResult dcHash_removeValueWithHashValue(dcHash *_hash,
                                         struct dcNode_t *_key,
                                         dcHashType _hashValue,
                                         struct dcNode_t **_removed,
                                         dcDepth _depth);

dcResult dcHash_removeValueWithStringKey(dcHash *_hash,
                                         const char *_key,
                                         struct dcNode_t **_removed,
                                         dcDepth _depth);

dcResult dcHash_removeValueWithStringKeys(dcHash *_hash,
                                          const char *_key,
                                          dcHashType _hashValue,
                                          struct dcNode_t **_removed,
                                          dcDepth _depth);

/**
 * Clears all objects (dcNodeS) out of a dcHash
 * \param _hash The hash to clear
 * \param _depth The depth of the clear
 */
void dcHash_clear(dcHash *_hash, dcDepth _depth);

/////////////
// marking //
/////////////

/**
 * Marks a dcHash, and potentially its objects, for garbage collection
 * \param _hash The dcHash to mark
 */
void dcHash_mark(dcHash *_hash);

/////////////
// copying //
/////////////

/**
 * Copies a dcHash
 * \param _from The dcHash to copy
 * \param _depth The depth of the copy
 * \return The copied dcHash
 */
dcHash *dcHash_copy(const dcHash *_from, dcDepth _depth);

//////////////
// printing //
//////////////

/**
 * Prints the contents of _hash to _stream
 * \param _hash The dcHash to print
 * \param _stream The stream to print to
 */
dcResult dcHash_print(const dcHash *_hash, struct dcString_t *_stream);

///////////////
// iterating //
///////////////

/**
 * Creates an iterator of type dcHashIterator for a dcHash
 * \param _hash The hash
 * \return A dcHashIterator iterator for _hash
 */
struct dcHashIterator_t *dcHash_createIterator(const dcHash *_hash);

/////////////////
// registering //
/////////////////

/**
 * Registers a dcHash, and potentially its objects, for garbage collection
 * \param _hash The dcHash to register
 */
void dcHash_register(dcHash *_hash);

/////////////
// merging //
/////////////

void dcHash_merge(dcHash *_to, dcHash *_from);

////////////////////
// dcHashIterator //
////////////////////

/**
 * An iterator for a dcHash
 */
struct dcHashIterator_t
{
    /**
     * The hash to iterator over
     */
    const dcHash *hash;

    /**
     * The current bucket
     */
    dcHashCapacityType bucketIt;

    /**
     * A list iterator for the current bucket
     */
    struct dcListElement_t *element;
};

typedef struct dcHashIterator_t dcHashIterator;

//////////////
// creating //
//////////////

/**
 * Creates a dcHashIterator
 * \param _hash The dcHash to iterator over
 * \return A newly allocated dcHashIterator
 */
dcHashIterator *dcHashIterator_create(const dcHash *_hash);

///////////////
// iterating //
///////////////

/**
 * Gets the next dcNode(dcHashElement) in a dcHash
 * \param _iterator The iterator
 * \return The next dcNode(dcHashElement) in the dcHash via _iterator
 */
struct dcNode_t *dcHashIterator_getNext(dcHashIterator *_iterator);

/**
 * Gets the next value in a dcHash
 * \param _iterator The iterator
 * \return The next value in the dcHash iterated by _iterator
 */
struct dcNode_t *dcHashIterator_getNextValue(dcHashIterator *_iterator);

/**
 * Gets the next key in a dcHash
 * \param _iterator The iterator
 * \return The next key in the dcHash iterated by _iterator
 */
struct dcNode_t *dcHashIterator_getNextValue(dcHashIterator *_iterator);

void dcHash_eachValue(dcHash *_hash,
                      dcContainerEachFunction _function,
                      void *_token);

///////////////
// resetting //
///////////////

void dcHashIterator_reset(dcHashIterator *_iterator);

/////////////
// freeing //
/////////////

/**
 * Frees a dcHashIterator
 * \param _iterator The dcHashIterator to free
 */
void dcHashIterator_free(dcHashIterator **_iterator);

////////////////
// displaying //
///////////////

const char *dcHash_display(const dcHash *_hash);

////////////////////////
// standard functions //
////////////////////////

COPY_FUNCTION(dcHashElement_copyNode);
COPY_FUNCTION(dcHash_copyNode);
FREE_FUNCTION(dcHash_freeNode);
MARK_FUNCTION(dcHash_markNode);
REGISTER_FUNCTION(dcHash_registerNode);
MARSHALL_FUNCTION(dcHash_marshallNode);
UNMARSHALL_FUNCTION(dcHash_unmarshallNode);
PRINT_FUNCTION(dcHash_printNode);

dcHash *dcHash_unmarshall(struct dcString_t *_stream);
struct dcString_t *dcHash_marshall(const dcHash *_hash,
                                   struct dcString_t *_stream);

#define HASH_ELEMENT_IS_NODE_KEY   0x10
#define HASH_ELEMENT_IS_STRING_KEY 0x20

#endif
