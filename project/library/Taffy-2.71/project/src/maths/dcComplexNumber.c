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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcComplexNumber.h"
#include "dcDefines.h"
#include "dcError.h"
#include "dcLexer.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNumber.h"
#include "dcString.h"

// creating //
dcComplexNumber *dcComplexNumber_create(dcNumber *_real, dcNumber *_imaginary)
{
    dcComplexNumber *result =
        (dcComplexNumber *)dcMemory_allocate(sizeof(dcComplexNumber));
    result->real = (_real == NULL
                    ? dcNumber_createFromInt32u(0)
                    : _real);
    result->imaginary = (_imaginary == NULL
                         ? dcNumber_createFromInt32u(0)
                         : _imaginary);
    return result;
}

dcComplexNumber *dcComplexNumber_createFromDoubles(double _real,
                                                   double _imaginary)
{
    return dcComplexNumber_create(dcNumber_createFromDouble(_real),
                                  dcNumber_createFromDouble(_imaginary));
}

dcComplexNumber *dcComplexNumber_createFromInt32s(int32_t _real,
                                                  int32_t _imaginary)
{
    return dcComplexNumber_create(dcNumber_createFromInt32s(_real),
                                  dcNumber_createFromInt32s(_imaginary));
}

dcComplexNumber *dcComplexNumber_createFromString(const char *_string)
{
    return dcComplexNumber_create(NULL, dcNumber_createFromString(_string));
}

struct dcNode_t *dcComplexNumber_createNode(dcNumber *_real,
                                            dcNumber *_imaginary)
{
    return dcNode_createWithGuts(NODE_COMPLEX_NUMBER,
                                 dcComplexNumber_create(_real, _imaginary));
}

// freeing //
void dcComplexNumber_free(dcComplexNumber **_number, dcDepth _depth)
{
    dcNumber_free(&(*_number)->real, _depth);
    dcNumber_free(&(*_number)->imaginary, _depth);
    dcMemory_free(*_number);
}

// copying //
dcComplexNumber *dcComplexNumber_copy(const dcComplexNumber *_from,
                                      dcDepth _depth)
{
    return dcComplexNumber_create(dcNumber_copy(_from->real, _depth),
                                  dcNumber_copy(_from->imaginary, _depth));
}

void dcComplexNumber_inlineCopy(dcComplexNumber *_to,
                                const dcComplexNumber *_from)
{
    dcNumber_inlineCopy(_to->real, _from->real);
    dcNumber_inlineCopy(_to->imaginary, _from->imaginary);
}

static void inlineCopy(dcComplexNumber *_to, const dcComplexNumber *_from)
{
    if (_to != _from)
    {
        dcNumber_inlineCopy(_to->real, _from->real);
        dcNumber_inlineCopy(_to->imaginary, _from->imaginary);
    }
}

// displaying //
char *dcComplexNumber_display(const dcComplexNumber *_number)
{
    char *result = NULL;
    char *real = dcNumber_display(_number->real);
    char *imaginary = dcNumber_display(_number->imaginary);

    if (dcNumber_isNaN(_number->real)
        || dcNumber_isNaN(_number->imaginary))
    {
        result = dcMemory_strdup("NaN");
    }
    else if (dcNumber_equalsInt32u(_number->real, 0))
    {
        // 0 + yi

        if (dcNumber_equalsInt32u(_number->imaginary, 0))
        {
            result = dcMemory_strdup("0");
        }
        else if (dcNumber_equalsInt32u(_number->imaginary, 1))
        {
            result = dcMemory_strdup("i");
        }
        else if (dcNumber_equalsInt32s(_number->imaginary, -1))
        {
            result = dcMemory_strdup("-i");
        }
        else
        {
            result = dcLexer_sprintf("%si", imaginary);
        }
    }
    else
    {
        // x + yi, where x != 0

        if (dcNumber_equalsInt32u(_number->imaginary, 0))
        {
            result = dcNumber_display(_number->real);
        }
        else
        {
            if (dcNumber_equalsInt32u(_number->imaginary, 1))
            {
                result = dcLexer_sprintf("(%s + i)", real, imaginary);
            }
            else if (dcNumber_isPositive(_number->imaginary))
            {
                result = dcLexer_sprintf("(%s + %si)", real, imaginary);
            }
            else
            {
                if (dcNumber_equalsInt32s(_number->imaginary, -1))
                {
                    result = dcLexer_sprintf("(%s - i)", real);
                }
                else
                {
                    TAFFY_DEBUG(dcError_assert(strlen(imaginary) > 0));
                    result = dcLexer_sprintf("(%s - %si)", real, imaginary + 1);
                }
            }
        }
    }

    dcMemory_free(real);
    dcMemory_free(imaginary);
    return result;
}

// querying //
bool dcComplexNumber_equals(const dcComplexNumber *_left,
                            const dcComplexNumber *_right)
{
    return (dcNumber_equals(_left->real, _right->real)
            && dcNumber_equals(_left->imaginary, _right->imaginary));
}

dcString *dcComplexNumber_marshall(const dcComplexNumber *_number,
                                   dcString *_stream)
{
    return dcMarshaller_marshall(_stream,
                                 "NN",
                                 _number->real,
                                 _number->imaginary);
}

dcComplexNumber *dcComplexNumber_unmarshall(struct dcString_t *_stream)
{
    dcNumber *real;
    dcNumber *imaginary;
    dcComplexNumber *result = NULL;

    if (dcMarshaller_unmarshallNoNull(_stream, "NN", &real, &imaginary))
    {
        result = dcComplexNumber_create(real, imaginary);
    }

    return result;
}

dcNumberResult dcComplexNumber_add(dcComplexNumber *_result,
                                   const dcComplexNumber *_left,
                                   const dcComplexNumber *_right)
{
    dcNumber_add(_result->real, _left->real, _right->real);
    dcNumber_add(_result->imaginary, _left->imaginary, _right->imaginary);
    return TAFFY_NUMBER_SUCCESS;
}

//
// divide two complex numbers: a / b
// multiply a and b by b's conjugate, then divide!
//
dcNumberResult dcComplexNumber_divide(dcComplexNumber *_result,
                                      const dcComplexNumber *_left,
                                      const dcComplexNumber *_right)
{
    dcComplexNumber *conjugate = dcComplexNumber_copy(_right, DC_DEEP);
    dcComplexNumber_conjugate(conjugate, conjugate);

    dcComplexNumber *top = dcComplexNumber_copy(_left, DC_DEEP);
    dcComplexNumber_multiply(top, _left, conjugate);

    dcComplexNumber *bottom = dcComplexNumber_copy(_right, DC_DEEP);

    dcComplexNumber_multiply(bottom, _right, conjugate);
    TAFFY_DEBUG(dcError_assert(dcNumber_equalsInt32u(bottom->imaginary, 0)));

    dcNumber_divide(top->real, top->real, bottom->real);
    dcNumber_divide(top->imaginary, top->imaginary, bottom->real);

    inlineCopy(_result, top);

    //printf("%s / %s => %s\n",
    //       dcComplexNumber_display(_left),
    //       dcComplexNumber_display(_right),
    //       dcComplexNumber_display(_result));

    dcComplexNumber_free(&conjugate, DC_DEEP);
    dcComplexNumber_free(&top, DC_DEEP);
    dcComplexNumber_free(&bottom, DC_DEEP);

    return TAFFY_NUMBER_SUCCESS;
}

//
// multiplication roole:
// (x + yi) * (u + vi) = (xu - yv)    + (xv + yu)i
//                        |     |         |    |
//                      first1 first2 second1 second2
//                      +-----------+ +---------------+
//                            |                |
//                          first            second
//
dcNumberResult dcComplexNumber_multiply(dcComplexNumber *_result,
                                        const dcComplexNumber *_left,
                                        const dcComplexNumber *_right)
{
    // first
    dcNumber *first1 = dcNumber_createFromInt32u(0);
    dcNumber_multiply(first1, _left->real, _right->real);
    dcNumber *first2 = dcNumber_createFromInt32u(0);
    dcNumber_multiply(first2, _left->imaginary, _right->imaginary);

    // second
    dcNumber *second1 = dcNumber_createFromInt32u(0);
    dcNumber_multiply(second1, _left->real, _right->imaginary);
    dcNumber *second2 = dcNumber_createFromInt32u(0);
    dcNumber_multiply(second2, _left->imaginary, _right->real);

    dcNumber_subtract(_result->real, first1, first2);
    dcNumber_add(_result->imaginary, second1, second2);

    //printf("%s * %s => %s\n",
    //       dcComplexNumber_display(_left),
    //       dcComplexNumber_display(_right),
    //       dcComplexNumber_display(_result));

    dcNumber_free(&first1, DC_DEEP);
    dcNumber_free(&first2, DC_DEEP);
    dcNumber_free(&second1, DC_DEEP);
    dcNumber_free(&second2, DC_DEEP);

    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcComplexNumber_raise(dcComplexNumber *_result,
                                     const dcComplexNumber *_left,
                                     const dcComplexNumber *_right)
{
    // implement?
    assert(false);
    return TAFFY_NUMBER_NOT_IMPLEMENTED;
}

dcNumberResult dcComplexNumber_subtract(dcComplexNumber *_result,
                                        const dcComplexNumber *_left,
                                        const dcComplexNumber *_right)
{
    dcNumber_subtract(_result->real, _left->real, _right->real);
    dcNumber_subtract(_result->imaginary, _left->imaginary, _right->imaginary);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcComplexNumber_absoluteValue(dcComplexNumber *_result,
                                             const dcComplexNumber *_number)
{
    return TAFFY_NUMBER_SUCCESS;
}

dcResult dcComplexNumber_hash(const dcComplexNumber *_number,
                              dcHashType *_hashResult);

// random //
dcNumberResult dcComplexNumber_random(dcComplexNumber *_result);

void dcComplexNumber_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_COMPLEX_NUMBER(_to) =
        dcComplexNumber_copy(CAST_COMPLEX_NUMBER(_from), _depth);
}

void dcComplexNumber_freeNode(dcNode *_node, dcDepth _depth)
{
    dcComplexNumber_free(&CAST_COMPLEX_NUMBER(_node), _depth);
}

static const dcComplexNumber_arithmeticFunction sArithmeticOperations[] =
    {
        &dcComplexNumber_add,
        &dcComplexNumber_subtract,
        &dcComplexNumber_multiply,
        &dcComplexNumber_divide,
        &dcComplexNumber_raise
    };

dcComplexNumber_arithmeticFunction dcComplexNumber_getArithmeticOperation
    (dcTaffyOperator _operation)
{
    dcError_assert(_operation < (sizeof(sArithmeticOperations)
                                 / sizeof(dcComplexNumber_arithmeticFunction)));
    return sArithmeticOperations[_operation];
}

double dcComplexNumber_getAngle(const dcComplexNumber *_number)
{
    //return atan(_number->
    return 0;
}

dcNumberResult dcComplexNumber_conjugate(dcComplexNumber *_result,
                                         const dcComplexNumber *_value)
{
    dcNumber_inlineCopy(_result->real, _value->real);
    dcNumber_multiply(_result->imaginary,
                      _value->imaginary,
                      dcNumber_getNegativeOne());
    return TAFFY_NUMBER_SUCCESS;
}

//
// modulus(x + yi) = sqrt(x^2 + y^2)
//                         ^     ^
//                         |     |
//                      first   second
//
dcNumberResult dcComplexNumber_modulus(dcNumber *_result,
                                       const dcComplexNumber *_value)
{
    dcNumber *first = dcNumber_createFromInt32u(0);
    dcNumber *second = dcNumber_createFromInt32u(0);
    dcNumber_raise(first, _value->real, dcNumber_getConstant(2));
    dcNumber_raise(second, _value->imaginary, dcNumber_getConstant(2));
    dcNumber_add(first, first, second);
    dcNumber_squareRoot(first, first);
    dcNumber_inlineCopy(_result, first);
    dcNumber_free(&first, DC_DEEP);
    dcNumber_free(&second, DC_DEEP);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcComplexNumber_addReal(dcComplexNumber *_result,
                                       const dcComplexNumber *_left,
                                       const dcNumber *_right)
{
    inlineCopy(_result, _left);
    dcNumber_add(_result->real, _result->real, _right);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcComplexNumber_subtractReal(dcComplexNumber *_result,
                                            const dcComplexNumber *_left,
                                            const dcNumber *_right)
{
    inlineCopy(_result, _left);
    dcNumber_subtract(_result->real, _result->real, _right);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcComplexNumber_multiplyReal(dcComplexNumber *_result,
                                            const dcComplexNumber *_left,
                                            const dcNumber *_right)
{
    inlineCopy(_result, _left);
    dcNumber_multiply(_result->real, _result->real, _right);
    dcNumber_multiply(_result->imaginary, _result->imaginary, _right);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcComplexNumber_divideReal(dcComplexNumber *_result,
                                          const dcComplexNumber *_left,
                                          const dcNumber *_right)
{
    inlineCopy(_result, _left);
    dcNumber_divide(_result->real, _result->real, _right);
    dcNumber_divide(_result->imaginary, _result->imaginary, _right);
    return TAFFY_NUMBER_SUCCESS;
}

bool dcComplexNumber_isNaN(const dcComplexNumber *_number)
{
    return (dcNumber_isNaN(_number->real)
            || dcNumber_isNaN(_number->imaginary));
}
