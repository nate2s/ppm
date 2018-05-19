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

#ifndef __DC_MATRIX_CLASS_H__
#define __DC_MATRIX_CLASS_H__

#include "dcDefines.h"

struct dcMatrix_t;

//////////////////////
// dcMatrixClassAux //
//////////////////////

struct dcMatrixClassAux_t
{
    struct dcMatrix_t *matrix;
    bool initialized;
};

typedef struct dcMatrixClassAux_t dcMatrixClassAux;

///////////////////
// dcMatrixClass //
///////////////////

void dcMatrixMetaClass_init(void);
void dcMatrixMetaClass_dealloc(void);
void dcMatrixMetaClass_cleanup(void);

struct dcNode_t *dcMatrixClass_createNode(struct dcMatrix_t *_matrix,
                                          bool _initialized,
                                          bool _object);

struct dcNode_t *dcMatrixClass_createObject(struct dcMatrix_t *_matrix,
                                            bool _initialized);

bool dcMatrixClass_isInitialized(const struct dcNode_t *_node);

struct dcMatrix_t *dcMatrixClass_getMatrix(const struct dcNode_t *_node);

// standard functions //
ALLOCATE_FUNCTION(dcMatrixClass_allocateNode);
COPY_FUNCTION(dcMatrixClass_copyNode);
DEALLOCATE_FUNCTION(dcMatrixClass_deallocateNode);
DEINITIALIZE_FUNCTION(dcMatrixClass_deinitialize);
FREE_FUNCTION(dcMatrixClass_freeNode);
GET_TEMPLATE_FUNCTION(dcMatrixClass_getTemplate);
INITIALIZE_FUNCTION(dcMatrixClass_initialize);
MARK_FUNCTION(dcMatrixClass_markNode);
MARK_FUNCTION(dcMatrixClass_markMetaNode);
MARSHALL_FUNCTION(dcMatrixClass_marshallNode);
PRINT_FUNCTION(dcMatrixClass_printNode);
REGISTER_FUNCTION(dcMatrixClass_registerNode);
SET_TEMPLATE_FUNCTION(dcMatrixClass_setTemplate);
UNMARSHALL_FUNCTION(dcMatrixClass_unmarshallNode);

#define CAST_MATRIX_AUX(_node_) ((dcMatrixClassAux*)CAST_CLASS_AUX(_node_))

TAFFY_C_METHOD(dcMatrixMetaClass_createWithRowsColumns);
TAFFY_C_METHOD(dcMatrixMetaClass_eye);

TAFFY_C_METHOD(dcMatrixClass_add);
TAFFY_C_METHOD(dcMatrixClass_asString);
TAFFY_C_METHOD(dcMatrixClass_columnCount);
TAFFY_C_METHOD(dcMatrixClass_equals);
TAFFY_C_METHOD(dcMatrixClass_deltaEqual);
TAFFY_C_METHOD(dcMatrixClass_mathOperationObject);
TAFFY_C_METHOD(dcMatrixClass_multiply);
TAFFY_C_METHOD(dcMatrixClass_objectAtIndex);
TAFFY_C_METHOD(dcMatrixClass_objectAtIndexes);
TAFFY_C_METHOD(dcMatrixClass_rowCount);
TAFFY_C_METHOD(dcMatrixClass_setObjectAtIndexes);
TAFFY_C_METHOD(dcMatrixClass_subMatrixFromRowColumnToRowColumn);
TAFFY_C_METHOD(dcMatrixClass_transpose);

// helpers //
struct dcNode_t *dcMatrixClass_matrixAndNumberOperation
    (struct dcNode_t *_leftMatrixNode,
     struct dcNode_t *_rightNumber,
     dcTaffyOperator _operation,
     bool _left);

bool dcMatrixClass_verifyMatrixDimensions_withException
    (struct dcNode_t *_matrix, uint32_t rowCount, uint32_t columnCount);

bool dcMatrixClass_extractDoubles_withException(struct dcNode_t *_matrix,
                                                  double *_output,
                                                  uint32_t _rows,
                                                  uint32_t _columns);

#define MATRIX_PACKAGE_NAME MATHS_PACKAGE_NAME
#define MATRIX_CLASS_NAME "Matrix"

#endif
