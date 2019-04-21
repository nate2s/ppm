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

#ifndef __DC_COMPLEX_NUMBER_H__
#define __DC_COMPLEX_NUMBER_H__

#include "dcDefines.h"

struct dcComplexNumber_t
{
    struct dcNumber_t *real;
    struct dcNumber_t *imaginary;
};

typedef struct dcComplexNumber_t dcComplexNumber;

// creating //
dcComplexNumber *dcComplexNumber_create(struct dcNumber_t *_real,
                                        struct dcNumber_t *_imaginary);
dcComplexNumber *dcComplexNumber_createFromDoubles(double _real,
                                                   double _imaginary);
dcComplexNumber *dcComplexNumber_createFromInt32s(int32_t _real,
                                                  int32_t _imaginary);
dcComplexNumber *dcComplexNumber_createFromString(const char *_string);

struct dcNode_t *dcComplexNumber_createNode(struct dcNumber_t *_real,
                                            struct dcNumber_t *_imaginary);

// freeing //
void dcComplexNumber_free(dcComplexNumber **_number, dcDepth _depth);

// copying //
dcComplexNumber *dcComplexNumber_copy(const dcComplexNumber *_from,
                                      dcDepth _depth);

void dcComplexNumber_inlineCopy(dcComplexNumber *_to,
                                const dcComplexNumber *_from);

// displaying //
char *dcComplexNumber_display(const dcComplexNumber *_number);

// querying //
bool dcComplexNumber_equals(const dcComplexNumber *_left,
                            const dcComplexNumber *_right);

// marshalling //
struct dcString_t *dcComplexNumber_marshall(const dcComplexNumber *_number,
                                            struct dcString_t *_stream);
dcComplexNumber *dcComplexNumber_unmarshall(struct dcString_t *_stream);

// arithmetic //

#define COMPLEX_ARITHMETIC_FUNCTION(__f__)                      \
    dcNumberResult __f__(dcComplexNumber *_result,              \
                         const dcComplexNumber *_left,          \
                         const dcComplexNumber *_right)

COMPLEX_ARITHMETIC_FUNCTION(dcComplexNumber_add);
COMPLEX_ARITHMETIC_FUNCTION(dcComplexNumber_divide);
COMPLEX_ARITHMETIC_FUNCTION(dcComplexNumber_multiply);
COMPLEX_ARITHMETIC_FUNCTION(dcComplexNumber_raise);
COMPLEX_ARITHMETIC_FUNCTION(dcComplexNumber_subtract);

dcNumberResult dcComplexNumber_multiplyReal(dcComplexNumber *_result,
                                            const dcComplexNumber *_left,
                                            const struct dcNumber_t *_right);

dcNumberResult dcComplexNumber_modulus(struct dcNumber_t *_result,
                                       const dcComplexNumber *_value);

dcNumberResult dcComplexNumber_conjugate(dcComplexNumber *_result,
                                         const dcComplexNumber *_value);

dcNumberResult dcComplexNumber_absoluteValue(dcComplexNumber *_result,
                                             const dcComplexNumber *_number);

dcResult dcComplexNumber_hash(const dcComplexNumber *_number,
                              dcHashType *_hashResult);

// random //
dcNumberResult dcComplexNumber_random(dcComplexNumber *_result);

typedef dcNumberResult (*dcComplexNumber_arithmeticFunction)
    (dcComplexNumber *_result,
     const dcComplexNumber *_left,
     const dcComplexNumber *_right);

typedef dcNumberResult (*dcComplexNumber_singleOperandFunction)
    (dcComplexNumber *_result,
     const dcComplexNumber *_number);

typedef bool (*dcComplexNumber_deltaEqualFunction)
    (const dcComplexNumber *_left,
     const dcComplexNumber *_right,
     uint32_t _delta);

dcComplexNumber_arithmeticFunction dcComplexNumber_getArithmeticOperation
    (dcTaffyOperator _operation);

// standard functions //
COPY_FUNCTION(dcComplexNumber_copyNode);
FREE_FUNCTION(dcComplexNumber_freeNode);

bool dcComplexNumber_isNaN(const struct dcComplexNumber_t *_number);

struct dcNumber_t *dcComplexNumber_getLength(const dcComplexNumber *_number);

dcNumberResult dcComplexNumber_addReal(dcComplexNumber *_result,
                                       const dcComplexNumber *_left,
                                       const struct dcNumber_t *_right);
dcNumberResult dcComplexNumber_subtractReal(dcComplexNumber *_result,
                                            const dcComplexNumber *_left,
                                            const struct dcNumber_t *_right);
dcNumberResult dcComplexNumber_multiplyReal(dcComplexNumber *_result,
                                            const dcComplexNumber *_left,
                                            const struct dcNumber_t *_right);
dcNumberResult dcComplexNumber_divideReal(dcComplexNumber *_result,
                                          const dcComplexNumber *_left,
                                          const struct dcNumber_t *_right);

#endif
