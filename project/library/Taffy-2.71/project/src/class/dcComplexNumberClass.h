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

#ifndef __DC_COMPLEX_NUMBER_CLASS_H__
#define __DC_COMPLEX_NUMBER_CLASS_H__

#include "dcDefines.h"

/////////////////////////////
// dcComplexNumberClassAux //
/////////////////////////////

struct dcComplexNumberClassAux_t
{
    struct dcComplexNumber_t *number;
};

typedef struct dcComplexNumberClassAux_t dcComplexNumberClassAux;

// creating //
struct dcNode_t *dcComplexNumberClass_createNode(dcComplexNumberClassAux *_aux,
                                                 bool _object);

struct dcNode_t *dcComplexNumberClass_createObject
    (struct dcComplexNumber_t *_number);

struct dcNode_t *dcComplexNumberClass_createObjectFromNumbers
    (struct dcNumber_t *_real,
     struct dcNumber_t *_imaginary);

struct dcNode_t *dcComplexNumberClass_numberOperation
    (struct dcNode_t *_left,
     struct dcNode_t *_right,
     dcTaffyOperator _operation,
     bool _complexOnLeft);

// getting //
size_t dcComplexNumberMetaClass_getPrecision(void);
struct dcComplexNumber_t *dcComplexNumberClass_getNumber
    (const struct dcNode_t *_node);
bool dcComplexNumberClass_equalsInt32u(const struct dcNode_t *_object,
                                       uint32_t _number);
bool dcComplexNumberClass_isNegative(const struct dcNode_t *_number);
bool dcComplexNumberClass_isMe(const struct dcNode_t *_node);
bool dcComplexNumberClass_isWhole(const struct dcNode_t *_node);
void dcComplexNumberClass_chomp(const struct dcNode_t *_node);

struct dcNode_t *dcComplexNumberClass_inlineAdd(struct dcNode_t *_left,
                                                struct dcNode_t *_right);
struct dcNode_t *dcComplexNumberClass_inlineSubtract(struct dcNode_t *_left,
                                                     struct dcNode_t *_right);
struct dcNode_t *dcComplexNumberClass_inlineMultiply(struct dcNode_t *_left,
                                                     struct dcNode_t *_right);
struct dcNode_t *dcComplexNumberClass_inlineDivide(struct dcNode_t *_left,
                                                   struct dcNode_t *_right);
struct dcNode_t *dcComplexNumberClass_inlineRaise(struct dcNode_t *_left,
                                                  struct dcNode_t *_right);

struct dcNode_t *dcComplexNumberClass_inlineAddReal
    (struct dcNode_t *_left, struct dcNode_t *_right, bool _complexOnLeft);
struct dcNode_t *dcComplexNumberClass_inlineSubtractReal
    (struct dcNode_t *_left, struct dcNode_t *_right, bool _complexOnLeft);
struct dcNode_t *dcComplexNumberClass_inlineMultiplyReal
    (struct dcNode_t *_left, struct dcNode_t *_right, bool _complexOnLeft);
struct dcNode_t *dcComplexNumberClass_inlineDivideReal
    (struct dcNode_t *_left, struct dcNode_t *_right, bool _complexOnLeft);

struct dcNode_t *dcComplexNumberClass_maybeConvertToReal
    (struct dcNode_t *_object);

// standard functions //
ALLOCATE_FUNCTION(dcComplexNumberClass_allocateNode);
COPY_FUNCTION(dcComplexNumberClass_copyNode);
FREE_FUNCTION(dcComplexNumberClass_freeNode);
GET_TEMPLATE_FUNCTION(dcComplexNumberClass_getTemplate);
INITIALIZE_FUNCTION(dcComplexNumberClass_initialize);
MARK_FUNCTION(dcComplexNumberMetaClass_markNode);
MARSHALL_FUNCTION(dcComplexNumberClass_marshallNode);
UNMARSHALL_FUNCTION(dcComplexNumberClass_unmarshallNode);

TAFFY_C_METHOD(dcComplexNumberClass_asString);

#define COMPLEX_NUMBER_PACKAGE_NAME MATHS_PACKAGE_NAME
#define COMPLEX_NUMBER_CLASS_NAME "ComplexNumber"

#endif
