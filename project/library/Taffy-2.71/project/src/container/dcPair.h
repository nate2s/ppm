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

#ifndef __DC_PAIR_H__
#define __DC_PAIR_H__

#include "dcDefines.h"

/**
 * A container for two dcNodeS
 */
struct dcPair_t
{
    /**
     * The left dcNode
     */
    struct dcNode_t *left;

    /**
     * The right dcNode
     */
    struct dcNode_t *right;
};

typedef struct dcPair_t dcPair;

//////////////
// creating //
//////////////

/**
 * Creates a dcPair-node
 * \param _left The left dcNode
 * \param _right The right dcNode
 * \return A newly allocated dcPair-node that contains _left and _right
 */
struct dcNode_t *dcPair_createNode(struct dcNode_t *_left,
                                   struct dcNode_t *_right);

struct dcNode_t *dcPair_createGraphDataNode(struct dcNode_t *_left,
                                            struct dcNode_t *_right);

/**
 * Creates a dcPair
 * \param _left The left dcNode
 * \param _right The right dcNode
 * \return A newly allocated dcPair that contains _left and _right
 */
dcPair *dcPair_create(struct dcNode_t *_left, struct dcNode_t *_right);

/////////////
// copying //
/////////////

/**
 * Copies a dcPair
 * \param _from The dcPair to copy from
 * \param _depth The depth at which to perform the copy
 * \return A newly allocated dcPair, which is a copy of _from
 */
dcPair *dcPair_copy(const dcPair *_from, dcDepth _depth);

/**
 * Copies a dcNode(dcGraphData(dcPair))
 * \param _to The dcNode(dcGraphData(cPair)) to copy to
 * \param _from The dcNode(dcGraphData(dcPair)) to copy from
 * \param _depth The depth at which to perform the copy
 */
void dcPair_copyGraphDataNode(struct dcNode_t *_to,
                              const struct dcNode_t *_from,
                              dcDepth _depth);

void dcPair_copyGraphDataTree(struct dcNode_t *_to,
                              const struct dcNode_t *_from,
                              dcDepth _depth);

/////////////
// getting //
/////////////

/**
 * Gets the left dcNode from a dcPair
 * \param _pair The dcPair
 * \return The left dcNode in _pair
 */
struct dcNode_t *dcPair_getLeft(const struct dcNode_t *_pair);

/**
 * Gets the right dcNode from a dcPair
 * \param _pair The dcPair
 * \return The right dcNode in _pair
 */
struct dcNode_t *dcPair_getRight(const struct dcNode_t *_pair);

/////////////
// setting //
/////////////

/**
 * Sets the left dcNode in a dcPair
 * \param _pair The dcPair
 * \param _left The dcNode to set as _pair's left
 */
void dcPair_setLeft(dcPair *_pair, struct dcNode_t *_left);

/**
 * Sets the right dcNode in a dcPair
 * \param _pair The dcPair
 * \param _right The dcNode to set as _pair's right
 */
void dcPair_setRight(dcPair *_pair, struct dcNode_t *_right);

/**
 * Sets the left dcNode in a dcPair
 * \param _pair The dcPair
 * \param _left The dcNode to set as _pair's left
 * \param _right The dcNode to set as _pair's right
 */
void dcPair_set(dcPair *_pair,
                struct dcNode_t *_left,
                struct dcNode_t *_right);

/////////////
// freeing //
/////////////

/**
 * Frees a dcPair, and potentially its contents
 * \param _pair The dcPair to free
 * \param _depth The depth of the free
 */
void dcPair_free(dcPair **_pair, dcDepth _depth);

//////////////
// clearing //
//////////////

/**
 * Clears a dcPair, and potentially frees its contents
 * \param _pair The dcPair to clear
 * \param _depth The depth at which to perform the clear
 */
void dcPair_clear(dcPair *_pair, dcDepth _depth);

/**
 * Clears a dcPair's left, and potentially frees it
 * \param _pair The dcPair to clear
 * \param _depth The depth at which to perform the clear
 */
void dcPair_clearLeft(dcPair *_pair, dcDepth _depth);

/**
 * Clears a dcPair's right, and potentially frees it
 * \param _pair The dcPair to clear
 * \param _depth The depth at which to perform the clear
 */
void dcPair_clearRight(dcPair *_pair, dcDepth _depth);

/////////////
// marking //
/////////////

/**
 * Marks a dcPair's contents for garbage collection marking,
 * if _yesno == true
 * \param _pair The dcPair whose contents will be marked
 */
void dcPair_mark(dcPair *_pair);

////////////////
// displaying //
////////////////

dcResult dcPair_print(const dcPair *_pair, struct dcString_t *_stream);
const char *dcPair_display(const dcPair *_list);

////////////////////////
// standard functions //
////////////////////////

FREE_FUNCTION(dcPair_freeGraphDataNode);
COMPARE_FUNCTION(dcPair_compareNode);
COPY_FUNCTION(dcPair_copyNode);
FREE_FUNCTION(dcPair_freeNode);
REGISTER_FUNCTION(dcPair_registerNode);
MARK_FUNCTION(dcPair_markNode);
MARK_FUNCTION(dcPair_markGraphDataNode);
MARSHALL_FUNCTION(dcPair_marshallNode);
MARSHALL_FUNCTION(dcPair_marshallGraphDataNode);
UNMARSHALL_FUNCTION(dcPair_unmarshallNode);
UNMARSHALL_FUNCTION(dcPair_unmarshallGraphDataNode);
PRINT_FUNCTION(dcPair_printNode);

/////////////////
// marshalling //
/////////////////

struct dcString_t *dcPair_marshall(const dcPair *_pair,
                                   struct dcString_t *_stream);
dcPair *dcPair_unmarshall(struct dcString_t *_stream);
bool dcPair_unmarshallGraphDataTree(struct dcNode_t *_node,
                                    struct dcString_t *_stream);
struct dcString_t *dcPair_marshallGraphDataTree(const struct dcNode_t *_node,
                                                struct dcString_t *_stream);

#endif
