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

#ifndef __DC_MATRIX_H__
#define __DC_MATRIX_H__

#include "dcDefines.h"

/**
 * A matrix container
 */
struct dcMatrix_t
{
    struct dcArray_t *objects;
    uint32_t rowCount;
    uint32_t columnCount;
};

typedef struct dcMatrix_t dcMatrix;

//////////////
// creating //
//////////////

dcMatrix *dcMatrix_create(struct dcArray_t *_objects,
                          uint32_t _rowCount,
                          uint32_t _columnCount);

dcMatrix *dcMatrix_createBlank(uint32_t _rowCount, uint32_t _columnCount);
dcMatrix *dcMatrix_createFromLists(struct dcList_t *_objects);
dcMatrix *dcMatrix_createEye(uint32_t _rowCount,
                             uint32_t _columnCount);

/////////////
// freeing //
/////////////

/**
 * Frees a dcMatrix, and potentially all its contents
 * \param _matrix The matrix to free
 * \param _depth The depth of the free
 */
void dcMatrix_free(dcMatrix **_matrix, dcDepth _depth);

void dcMatrix_clear(dcMatrix *_matrix, dcDepth _depth);

/////////////
// copying //
/////////////

/**
 * Creates a new dcMatrix whose elements are copies of a dcMatrix
 * \param _from The matrix to copy from
 * \param _depth The depth of the copy
 * \return The newly allocated copy of _from
 */
dcMatrix *dcMatrix_copy(const dcMatrix *_from, dcDepth _depth);

// getting //
uint32_t dcMatrix_getSize(const dcMatrix *_matrix);

struct dcNode_t *dcMatrix_get(const dcMatrix *_matrix,
                              uint32_t _row,
                              uint32_t _column);

void dcMatrix_set(const dcMatrix *_matrix,
                  struct dcNode_t *_node,
                  uint32_t _row,
                  uint32_t _column);

// setting //
void dcMatrix_setTemplate(dcMatrix *_matrix, bool _yesNo);

dcMatrix *dcMatrix_transpose(dcMatrix *_matrix);

/////////////
// marking //
/////////////

void dcMatrix_mark(dcMatrix *_matrix);

/////////////////
// marshalling //
/////////////////

dcMatrix *dcMatrix_unmarshall(struct dcString_t *_stream);
struct dcString_t *dcMatrix_marshall(const dcMatrix *_matrix,
                                     struct dcString_t *_stream);

////////////////////////
// standard functions //
////////////////////////

COPY_FUNCTION(dcMatrix_copyNode);
FREE_FUNCTION(dcMatrix_freeNode);
MARSHALL_FUNCTION(dcMatrix_marshallNode);
UNMARSHALL_FUNCTION(dcMatrix_unmarshallNode);

/////////////
// testing //
/////////////

void dcMatrix_assertIsTemplate(dcMatrix *_matrix);

#endif
