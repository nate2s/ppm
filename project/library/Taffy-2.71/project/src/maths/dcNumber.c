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

#include "dcDecNumber.h"
#include "dcNumber.h"
#include "dcError.h"
#include "dcHash.h"
#include "dcInt32.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcPair.h"
#include "dcString.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dcNumberClass.h"
#include "dcPairClass.h"

#ifndef TAFFY_WINDOWS
    #include <inttypes.h>
#endif

#define CAST_INTEGER(_node)    _node->types.integer
#define CAST_DEC_NUMBER(_node) _node->types.decNumber
#define IS_INTEGER(_node)      (_node->type == NUMBER_INTEGER_TYPE)
#define IS_DEC_NUMBER(_node)   (_node->type == NUMBER_DEC_NUMBER_TYPE)

// ORLY?
#define MAX_INT_32 ((1 << 16) - 1)
#define MIN_INT_32 -MAX_INT_32
#define OUT_OF_BOUNDS(_value) (_value > MAX_INT_32       \
                               || _value < MIN_INT_32)

#define CONSTANTS_SIZE 21

static dcNumber *sConstants[CONSTANTS_SIZE];
static dcNumber *sOneHalf = NULL;
static dcNumber *sNegativeOneHalf = NULL;
static dcNumber *sNegativeOne = NULL;
static dcNumber *sMax64BitNumber = NULL;
static dcNumber *sZero = NULL;
static dcNumber *sMaxInt32 = NULL;
static dcNumber *sMinInt32 = NULL;

void dcNumber_initialize(void)
{
    uint8_t a;

    for (a = 0; a < dcTaffy_countOf(sConstants); a++)
    {
        sConstants[a] = dcNumber_createFromInt32u(a);
    }

    sOneHalf = dcNumber_createFromDouble(0.5);
    sNegativeOneHalf = dcNumber_createFromDouble(-0.5);
    sMax64BitNumber = dcNumber_createFromString("18446744073709556935");
    sZero = dcNumber_createFromInt32u(0);
    sNegativeOne = dcNumber_createFromInt32u(-1);
    sMaxInt32 = dcNumber_createFromInt32s(MAX_INT_32);
    sMinInt32 = dcNumber_createFromInt32s(MIN_INT_32);
}

void dcNumber_deinitialize(void)
{
    uint8_t a;

    for (a = 0; a < dcTaffy_countOf(sConstants); a++)
    {
        dcNumber_free(&sConstants[a], DC_DEEP);
    }

    dcNumber_free(&sOneHalf, DC_DEEP);
    dcNumber_free(&sNegativeOneHalf, DC_DEEP);
    dcNumber_free(&sNegativeOne, DC_DEEP);
    dcNumber_free(&sMax64BitNumber, DC_DEEP);
    dcNumber_free(&sZero, DC_DEEP);
    dcNumber_free(&sMaxInt32, DC_DEEP);
    dcNumber_free(&sMinInt32, DC_DEEP);
}

const dcNumber *dcNumber_getConstant(uint8_t _value)
{
    dcError_assert(_value < dcTaffy_countOf(sConstants));
    return sConstants[_value];
}

const dcNumber *dcNumber_getOneHalf(void)
{
    return sOneHalf;
}

const dcNumber *dcNumber_getNegativeOneHalf(void)
{
    return sNegativeOneHalf;
}

dcNumber *dcNumber_getNegativeOne(void)
{
    dcError_assert(sNegativeOne != NULL);
    return sNegativeOne;
}

static dcNumber *create(dcNumberType _type)
{
    dcNumber *number =
        (dcNumber *)dcMemory_allocateAndInitialize(sizeof(dcNumber));
    number->type = _type;
    return number;
}

dcNumber *dcNumber_createFromInt32u(uint32_t _value)
{
    dcNumber *result = create(NUMBER_INTEGER_TYPE);
    result->types.integer = (int32_t)_value;
    return result;
}

dcNumber *dcNumber_createFromInt32uWithLsuSize(uint32_t _value, uint32_t _size)
{
    dcNumber *result = create(NUMBER_DEC_NUMBER_TYPE);
    result->types.decNumber = dcDecNumber_createWithLsuSize(_size);
    result->types.decNumber =
        decNumberFromUInt32(result->types.decNumber, _value);
    return result;
}

dcNode *dcNumber_createNodeFromInt32u(uint32_t _value)
{
    return dcNode_createWithGuts(NODE_NUMBER,
                                 dcNumber_createFromInt32u(_value));
}

dcNumber *dcNumber_createFromInt32s(int32_t _value)
{
    dcNumber *result = create(NUMBER_INTEGER_TYPE);
    result->types.integer = _value;
    return result;
}

dcNumber *dcNumber_createFromDouble(double _value)
{
    dcNumber *result = create(NUMBER_DEC_NUMBER_TYPE);
    result->types.decNumber = dcDecNumber_createFromDouble(_value);
    return result;
}

dcNumber *dcNumber_createWithLsuSize(uint32_t _size)
{
    dcNumber *result = create(NUMBER_DEC_NUMBER_TYPE);
    result->types.decNumber = dcDecNumber_createWithLsuSize(_size);
    return result;
}

dcNode *dcNumber_createNodeFromInt64u(uint64_t _value)
{
    return dcNode_createWithGuts(NODE_NUMBER,
                                 dcNumber_createFromInt64u(_value));
}

dcNumber *dcNumber_createFromInt64u(uint64_t _value)
{
    dcNumber *result = create(NUMBER_DEC_NUMBER_TYPE);
    result->types.decNumber = dcDecNumber_createFromDouble((double)_value);
    return result;
}

dcNode *dcNumber_createNodeFromSizet(size_t _value)
{
    return dcNode_createWithGuts(NODE_NUMBER,
                                 dcNumber_createFromInt64u(_value));
}

dcNumber *dcNumber_createFromSizet(size_t _value)
{
    dcNumber *result = create(NUMBER_DEC_NUMBER_TYPE);
    result->types.decNumber = dcDecNumber_createFromSizet(_value);
    return result;
}

dcNumber *dcNumber_createFromDecNumber(decNumber *_number)
{
    dcNumber *result = create(NUMBER_DEC_NUMBER_TYPE);
    result->types.decNumber = _number;
    return result;
}

dcNumber *dcNumber_createFromString(const char *_string)
{
    bool hasDot = false;
    dcNumber *result = NULL;
    size_t length = strlen(_string);
    ssize_t i;
    int64_t value = 0;
    int64_t power = 1;
    bool over = false;

    for (i = length - 1; i >= 0; i--)
    {
        if (_string[i] == '.')
        {
            hasDot = true;
            break;
        }

        value += 9 * power;
        power *= 10;

        if (OUT_OF_BOUNDS(value))
        {
            over = true;
        }
    }

    // less than 7 digits, slightly arbitrary
    if (! hasDot && ! over)
    {
        result = dcNumber_createFromInt32s(atoi(_string));
    }
    else
    {
        result = dcNumber_createFromDecNumber
            (dcDecNumber_createFromString(_string));
    }

    return result;
}

dcNode *dcNumber_createShell(dcNumber *_number)
{
    return dcNode_createWithGuts(NODE_NUMBER, _number);
}

void dcNumber_free(dcNumber **_number, dcDepth _depth)
{
    if (_number != NULL && *_number != NULL)
    {
        dcNumber *number = *_number;

        if (number->type == NUMBER_DEC_NUMBER_TYPE)
        {
            dcDecNumber_free(&number->types.decNumber);
        }

        dcMemory_free(*_number);
    }
}

void dcNumber_freeNode(dcNode *_node, dcDepth _depth)
{
    dcNumber_free(&(CAST_NUMBER(_node)), _depth);
}

dcNumber *dcNumber_copy(const dcNumber *_from, dcDepth _depth)
{
    dcNumber *to = (dcNumber *)dcMemory_allocate(sizeof(dcNumber));
    to->type = _from->type;

    if (_from->type == NUMBER_INTEGER_TYPE)
    {
        to->types.integer = _from->types.integer;
    }
    else if (_from->type == NUMBER_DEC_NUMBER_TYPE)
    {
        to->types.decNumber = dcDecNumber_copy(_from->types.decNumber);
    }
    else
    {
        dcError_internal("dcNumber_copyNode::unknown Number type %d",
                         _from->type);
    }

    return to;
}

void dcNumber_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_NUMBER(_to) = dcNumber_copy(CAST_NUMBER(_from), _depth);
}

char *dcNumber_display(const dcNumber *_number)
{
    char *result = NULL;

    if (_number->type == NUMBER_INTEGER_TYPE)
    {
        result = dcLexer_sprintf("%d", _number->types.integer);
    }
    else if (_number->type == NUMBER_DEC_NUMBER_TYPE)
    {
        result = dcDecNumber_display(_number->types.decNumber);
    }
    else
    {
        assert(false);
    }

    return result;
}

char *dcNumber_displayNode(const dcNode *_node)
{
    return dcNumber_display(CAST_NUMBER(_node));
}

dcResult dcNumber_hash(const dcNumber *_number, dcHashType *_hashResult)
{
    dcResult result = TAFFY_SUCCESS;
    char *tempString = dcNumber_display(_number);
    result = dcString_hashBytes((uint8_t*)tempString,
                                strlen(tempString),
                                _hashResult);
    dcMemory_free(tempString);
    return result;
}

bool dcNumber_isExact(const dcNumber *_number)
{
    bool result = false;

    if (_number->type == NUMBER_INTEGER_TYPE)
    {
        result = true;
    }
    else if (_number->type == NUMBER_DEC_NUMBER_TYPE)
    {
        result = dcDecNumber_isExact(CAST_DEC_NUMBER(_number));
    }
    else
    {
        dcError_assert(false);
    }

    return result;
}

bool dcNumber_isWhole(const dcNumber *_number)
{
    bool result = false;

    if (_number->type == NUMBER_INTEGER_TYPE)
    {
        result = true;
    }
    else if (_number->type == NUMBER_DEC_NUMBER_TYPE)
    {
        result = dcDecNumber_isWhole(_number->types.decNumber);
    }
    else
    {
        dcError_assert(false);
    }

    return result;
}

bool dcNumber_extractUInt8(const dcNumber *_template, uint8_t *_value)
{
    uint32_t value;
    bool result = false;

    if (dcNumber_extractUInt32(_template, &value)
        && value <= 0xFF)
    {
        result = true;
        *_value = (uint8_t)value;
    }

    return result;
}

bool dcNumber_extractUInt16(const dcNumber *_template, uint16_t *_value)
{
    uint32_t value;
    bool result = false;

    if (dcNumber_extractUInt32(_template, &value)
        && value <= 0xFFFF)
    {
        result = true;
        *_value = (uint16_t)value;
    }

    return result;
}

bool dcNumber_extractUInt32(const dcNumber *_template, uint32_t *_value)
{
    bool result = false;

    if (_template->type == NUMBER_INTEGER_TYPE)
    {
        result = true;
        *_value = CAST_INTEGER(_template);
    }
    else if (_template->type == NUMBER_DEC_NUMBER_TYPE
             && dcDecNumber_convertToUInt32(_template->types.decNumber, _value))
    {
        result = true;
    }

    return result;
}

bool dcNumber_extractInt32(const dcNumber *_template, int32_t *_value)
{
    uint32_t value = 0;
    bool result = false;

    if (dcNumber_extractUInt32(_template, &value))
    {
        result = true;
        *_value = value;
    }

    return result;
}

bool dcNumber_extractUInt64(const dcNumber *_template, uint64_t *_value)
{
    bool result = false;

    if (_template->type == NUMBER_INTEGER_TYPE)
    {
        result = true;
        *_value = CAST_INTEGER(_template);
    }
    else if (_template->type == NUMBER_DEC_NUMBER_TYPE
             && dcNumber_isWhole(_template)
             && dcNumber_lessThanOrEqual(_template, sMax64BitNumber)
             && dcNumber_greaterThanOrEqual(_template, sZero))
    {
        // TODO: this is way too slow, create a method in decNumber to convert
        // to 64-bit value instead
        char *display = dcDecNumber_display(CAST_DEC_NUMBER(_template));
#ifdef TAFFY_WINDOWS
        result = (sscanf(display, "%llu", _value) == 1);
#else
        result = (sscanf(display, "%" SCNu64 "", _value) == 1);
#endif
        dcMemory_free(display)
    }

    return result;
}

bool dcNumber_extractInt64(const dcNumber *_template, int64_t *_value)
{
    uint64_t value = 0;
    bool result = false;

    if (dcNumber_extractUInt64(_template, &value))
    {
        result = true;
        *_value = value;
    }

    return result;
}

void dcDecNumber_printLsu(const decNumber *_number)
{
    int32_t i;

    for (i = 0; i < _number->digits; i++)
    {
        fprintf(stderr, "%u ", _number->lsu[i]);
    }

    fprintf(stderr, "\n");
}

// test function
void dcNumber_printLsu(const dcNumber *_value)
{
    if (! IS_DEC_NUMBER(_value))
    {
        fprintf(stderr, "???");
    }
    else
    {
        dcDecNumber_printLsu(CAST_DEC_NUMBER(_value));
    }
}

bool dcNumber_extractDouble(const dcNumber *_template, double *_value)
{
    bool result = false;

    if (_template->type == NUMBER_INTEGER_TYPE)
    {
        result = true;
        *_value = CAST_INTEGER(_template);
    }
    else if (_template->type == NUMBER_DEC_NUMBER_TYPE)
    {
#ifndef TAFFY_WINDOWS
        result = (dcDecNumber_convertToDouble(_template->types.decNumber,
                                              _value)
                  && *_value != NAN
                  && *_value != INFINITY);
#else
        // this makes me sad
        result = dcDecNumber_convertToDouble(_template->types.decNumber, _value);
#endif
    }

    return result;
}

////////////////
// comparison //
////////////////

static dcTaffyOperator compareIntegerToInteger(const dcNumber *_left,
                                               const dcNumber *_right)
{
    return (CAST_INTEGER(_left) < CAST_INTEGER(_right)
            ? TAFFY_LESS_THAN
            : (CAST_INTEGER(_left) > CAST_INTEGER(_right)
               ? TAFFY_GREATER_THAN
               : TAFFY_EQUALS));
}

static dcTaffyOperator compareIntegerToDecNumber(const dcNumber *_left,
                                                 const dcNumber *_right)
{
    dcTaffyOperator result = dcDecNumber_easyCompareToInt32s
        (CAST_DEC_NUMBER(_right), CAST_INTEGER(_left));

    return (result == TAFFY_LESS_THAN
            ? TAFFY_GREATER_THAN
            : (result == TAFFY_GREATER_THAN
               ? TAFFY_LESS_THAN
               : TAFFY_EQUALS));
}

static dcTaffyOperator compareDecNumberToInteger(const dcNumber *_left,
                                                 const dcNumber *_right)
{
    return dcDecNumber_easyCompareToInt32s(CAST_DEC_NUMBER(_left),
                                           CAST_INTEGER(_right));
}

static dcTaffyOperator compareDecNumberToDecNumber(const dcNumber *_left,
                                                   const dcNumber *_right)
{
    return dcDecNumber_easyCompare(CAST_DEC_NUMBER(_left),
                                   CAST_DEC_NUMBER(_right));
}

typedef dcTaffyOperator (*internalComparisonFunction)(const dcNumber *_left,
                                                      const dcNumber *_right);

static internalComparisonFunction sIntegerComparisonFunctions[2] =
{
    &compareIntegerToInteger,
    &compareIntegerToDecNumber
};

static internalComparisonFunction sDecNumberComparisonFunctions[2] =
{
    &compareDecNumberToInteger,
    &compareDecNumberToDecNumber
};

static const internalComparisonFunction *sComparisonFunctions[2] =
{
    sIntegerComparisonFunctions,
    sDecNumberComparisonFunctions
};

bool dcNumber_greaterThan(const dcNumber *_left, const dcNumber *_right)
{
    return ((*sComparisonFunctions[_left->type][_right->type])(_left, _right)
            == TAFFY_GREATER_THAN);
}

bool dcNumber_greaterThanOrEqual(const dcNumber *_left,
                                 const dcNumber *_right)
{
    dcTaffyOperator result =
        (*sComparisonFunctions[_left->type][_right->type])(_left, _right);

    return (result == TAFFY_GREATER_THAN || result == TAFFY_EQUALS);
}

bool dcNumber_lessThan(const dcNumber *_left, const dcNumber *_right)
{
    return ((*sComparisonFunctions[_left->type][_right->type])(_left, _right)
            == TAFFY_LESS_THAN);
}

bool dcNumber_lessThanOrEqual(const dcNumber *_left, const dcNumber *_right)
{
    dcTaffyOperator result =
        (*sComparisonFunctions[_left->type][_right->type])(_left, _right);
    return (result == TAFFY_LESS_THAN || result == TAFFY_EQUALS);
}

bool dcNumber_equals(const dcNumber *_left, const dcNumber *_right)
{
    return ((*sComparisonFunctions[_left->type][_right->type])(_left, _right)
            == TAFFY_EQUALS);
}

bool dcNumber_equalsInt32s(const dcNumber *_template, int32_t _number)
{
    return ((_number >= 0 && _number < CONSTANTS_SIZE)
            ? dcNumber_equals(_template, sConstants[_number])
            : (_template->type == NUMBER_INTEGER_TYPE
               ? CAST_INTEGER(_template) == _number
               : (dcDecNumber_easyCompareToInt32s(CAST_DEC_NUMBER(_template),
                                                  _number)
                  == TAFFY_EQUALS)));
}

bool dcNumber_equalsInt32u(const dcNumber *_template, uint32_t _number)
{
    return (_number < CONSTANTS_SIZE
            ? dcNumber_equals(_template, sConstants[_number])
            : (_template->type == NUMBER_INTEGER_TYPE
               ? (uint32_t)CAST_INTEGER(_template) == _number
               : (dcDecNumber_easyCompareToInt32u(CAST_DEC_NUMBER(_template),
                                                  _number)
                  == TAFFY_EQUALS)));
}

static void convertIntegerToDecNumber(dcNumber *_number)
{
    if (_number->type == NUMBER_INTEGER_TYPE)
    {
        _number->type = NUMBER_DEC_NUMBER_TYPE;
        _number->types.decNumber =
            dcDecNumber_createFromInt32u(_number->types.integer);
    }
}

static void convertToDecNumber(dcNumber *_number)
{
    if (_number->type == NUMBER_INTEGER_TYPE)
    {
        _number->types.decNumber =
            dcDecNumber_createFromInt32s(_number->types.integer);
        _number->type = NUMBER_DEC_NUMBER_TYPE;
    }
}

static bool convertToInteger(dcNumber *_number)
{
    int32_t converted = 0;

    if (_number->type == NUMBER_DEC_NUMBER_TYPE
        && dcDecNumber_convertToInt32(_number->types.decNumber, &converted))
    {
        dcDecNumber_free(&_number->types.decNumber);
        _number->types.integer = converted;
        _number->type = NUMBER_INTEGER_TYPE;
        return true;
    }

    return false;
}

void dcNumber_setInt32sValue(dcNumber *_number, int32_t _value)
{
    if (_number->type == NUMBER_DEC_NUMBER_TYPE)
    {
        dcDecNumber_free(&_number->types.decNumber);
    }

    _number->type = NUMBER_INTEGER_TYPE;
    CAST_INTEGER(_number) = _value;
}

void dcNumber_setDoubleValue(dcNumber *_number, double _value)
{
    if (_number->type == NUMBER_DEC_NUMBER_TYPE)
    {
        dcDecNumber_free(&_number->types.decNumber);
    }

    CAST_DEC_NUMBER(_number) = dcDecNumber_createFromDouble(_value);
    _number->type = NUMBER_DEC_NUMBER_TYPE;
}

static dcNumber *copyAndConvertToDecNumber(const dcNumber *_number)
{
    dcNumber *result = dcNumber_copy(_number, DC_DEEP);
    convertToDecNumber(result);
    return result;
}

typedef bool (*deltaEqualComparisonFunction)(const dcNumber *_left,
                                             const dcNumber *_right,
                                             uint32_t _delta);

bool deltaEqualIntegerToInteger(const dcNumber *_left,
                                const dcNumber *_right,
                                uint32_t _delta)
{
    int32_t left = CAST_INTEGER(_left);
    int32_t right = CAST_INTEGER(_right);

    return (left == right
            ? true
            : (left == right - 1
               && _delta == 0
               ? true
               : (left == right + 1
                  && _delta == 0
                  ? true
                  : false)));
}

bool deltaEqualIntegerToDecNumber(const dcNumber *_left,
                                  const dcNumber *_right,
                                  uint32_t _delta)
{
    dcNumber *leftDecNumber = copyAndConvertToDecNumber(_left);
    bool result = dcDecNumber_deltaEqual(CAST_DEC_NUMBER(leftDecNumber),
                                         CAST_DEC_NUMBER(_right),
                                         _delta);
    dcNumber_free(&leftDecNumber, DC_DEEP);
    return result;
}

bool deltaEqualDecNumberToInteger(const dcNumber *_left,
                                  const dcNumber *_right,
                                  uint32_t _delta)
{
    dcNumber *rightDecNumber = copyAndConvertToDecNumber(_right);
    bool result = dcDecNumber_deltaEqual(CAST_DEC_NUMBER(_left),
                                         CAST_DEC_NUMBER(rightDecNumber),
                                         _delta);
    dcNumber_free(&rightDecNumber, DC_DEEP);
    return result;
}

bool deltaEqualDecNumberToDecNumber(const dcNumber *_left,
                                    const dcNumber *_right,
                                    uint32_t _delta)
{
    return dcDecNumber_deltaEqual(CAST_DEC_NUMBER(_left),
                                  CAST_DEC_NUMBER(_right),
                                  _delta);
}

static deltaEqualComparisonFunction sIntegerDeltaEqualFunctions[2] =
{
    &deltaEqualIntegerToInteger,
    &deltaEqualIntegerToDecNumber
};

static deltaEqualComparisonFunction sDecNumberDeltaEqualFunctions[2] =
{
    &deltaEqualDecNumberToInteger,
    &deltaEqualDecNumberToDecNumber
};

static const deltaEqualComparisonFunction *sDeltaEqualFunctions[2] =
{
    sIntegerDeltaEqualFunctions,
    sDecNumberDeltaEqualFunctions
};

bool dcNumber_deltaEqual(const dcNumber *_left,
                         const dcNumber *_right,
                         uint32_t _delta)
{
    return (*sDeltaEqualFunctions[_left->type][_right->type])
        (_left, _right, _delta);
}

dcNumberResult dcNumber_absoluteValue(dcNumber *_result,
                                      const dcNumber *_number)
{
    dcNumberResult result = TAFFY_NUMBER_SUCCESS;

    if (IS_DEC_NUMBER(_number))
    {
        convertIntegerToDecNumber(_result);
        result = dcDecNumber_absoluteValue(CAST_DEC_NUMBER(_result),
                                           CAST_DEC_NUMBER(_number));
    }
    else
    {
        convertToInteger(_result);
        int32_t value = CAST_INTEGER(_number);
        CAST_INTEGER(_result) = (value < 0
                                 ? -value
                                 : value);
    }

    return result;
}

dcNumberResult dcNumber_ceiling(dcNumber *_result, const dcNumber *_number)
{
    dcNumberResult result = TAFFY_NUMBER_SUCCESS;

    if (IS_DEC_NUMBER(_number))
    {
        convertIntegerToDecNumber(_result);
        result = dcDecNumber_ceiling(CAST_DEC_NUMBER(_result),
                                     CAST_DEC_NUMBER(_number));
    }
    else
    {
        convertToInteger(_result);
        CAST_INTEGER(_result) = CAST_INTEGER(_number);
    }

    return result;
}

dcNumberResult dcNumber_chomp(dcNumber *_result, const dcNumber *_number)
{
    dcNumberResult result = TAFFY_NUMBER_SUCCESS;

    if (IS_DEC_NUMBER(_number))
    {
        convertIntegerToDecNumber(_result);
        result = dcDecNumber_chomp(CAST_DEC_NUMBER(_result),
                                   CAST_DEC_NUMBER(_number));
    }
    else
    {
        // we're done!
        convertToInteger(_result);
        CAST_INTEGER(_result) = CAST_INTEGER(_number);
    }

    return result;
}

dcNumber *dcNumber_snip(dcNumber *_number)
{
    if (IS_DEC_NUMBER(_number))
    {
        decNumber *number = CAST_DEC_NUMBER(_number);

        if (number->exponent + number->digits
            <= -((int)dcNumberMetaClass_getDigitLimit() - 1))
        {
            //printf("number->exponent: %d, precision: %zu\n",
            //       number->exponent,
            //       dcNumberMetaClass_getPrecision());

            number->exponent = 0;
            number->digits = 1;
            memset(number->lsu, 0, number->lsuSize);
        }
    }

    return _number;
}

bool dcNumber_tryToConvertToInteger(dcNumber *_result)
{
    bool result = false;

    if (IS_INTEGER(_result))
    {
        result = true;
    }
    else
    {
        // be extra careful. we might not need these extra checks,
        // since converToInteger() does some checks in decNumber
        if (dcNumber_isWhole(_result)
            && dcNumber_lessThanOrEqual(_result, sMaxInt32)
            && dcNumber_greaterThanOrEqual(_result, sMinInt32))
        {
            dcNumber_floor(_result, _result);
            assert(convertToInteger(_result));
        }
    }

    return result;
}

dcNumberResult dcNumber_floor(dcNumber *_result, const dcNumber *_number)
{
    dcNumberResult result = TAFFY_NUMBER_SUCCESS;

    if (IS_DEC_NUMBER(_number))
    {
        convertIntegerToDecNumber(_result);
        result = dcDecNumber_floor(CAST_DEC_NUMBER(_result),
                                   CAST_DEC_NUMBER(_number));
    }
    else
    {
        // it's already floored, silly
        convertToInteger(_result);
        CAST_INTEGER(_result) = CAST_INTEGER(_number);
    }

    return result;
}

static dcNumberResult decNumberAndIntegerOperation
    (dcNumber *_result,
     const dcNumber *_left,
     const dcNumber *_right,
     dcDecNumber_arithmeticFunction _function,
     bool _needIntegers)
{
    dcNumberResult result = TAFFY_NUMBER_SUCCESS;

    if (_needIntegers
        && (! dcNumber_isWhole(_left)
            || ! dcNumber_isWhole(_right)))
    {
        result = TAFFY_NUMBER_NEED_INTEGER;
    }
    else
    {
        convertToDecNumber(_result);
        dcNumber *right = copyAndConvertToDecNumber(_right);
        _function(CAST_DEC_NUMBER(_result),
                  CAST_DEC_NUMBER(_left),
                  CAST_DEC_NUMBER(right));
        dcNumber_free(&right, DC_DEEP);
    }

    return result;
}

static dcNumberResult integerAndDecNumberOperation
    (dcNumber *_result,
     const dcNumber *_left,
     const dcNumber *_right,
     dcDecNumber_arithmeticFunction _function,
     bool _needIntegers)
{
    dcNumberResult result = TAFFY_NUMBER_SUCCESS;

    if (_needIntegers
        && (! dcNumber_isWhole(_left)
            || ! dcNumber_isWhole(_right)))
    {
        result = TAFFY_NUMBER_NEED_INTEGER;
    }
    else
    {
        convertToDecNumber(_result);
        dcNumber *left = copyAndConvertToDecNumber(_left);
        _function(CAST_DEC_NUMBER(_result),
                  CAST_DEC_NUMBER(left),
                  CAST_DEC_NUMBER(_right));
        dcNumber_free(&left, DC_DEEP);
    }

    return result;
}

static dcNumber_operation sOperations[] =
{
    &dcNumber_add,
    &dcNumber_subtract,
    &dcNumber_multiply,
    &dcNumber_divide
};

dcNumber_operation dcNumber_getOperation(dcTaffyOperator _operator)
{
    return ((size_t)_operator < dcTaffy_countOf(sOperations)
            ? sOperations[(int)_operator]
            : NULL);
}

static dcNumberResult outOfBoundsConvert(dcNumber *_result,
                                         const dcNumber *_left,
                                         const dcNumber *_right,
                                         dcNumber_arithmeticFunction _function)
{
    // expensive
    dcNumber *left = copyAndConvertToDecNumber(_left);
    dcNumber *right = copyAndConvertToDecNumber(_right);
    _function(_result, left, right);
    dcNumber_free(&left, DC_DEEP);
    dcNumber_free(&right, DC_DEEP);
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult addDecNumberToDecNumber(dcNumber *_result,
                                              const dcNumber *_left,
                                              const dcNumber *_right);

static dcNumberResult addIntegerToInteger(dcNumber *_result,
                                          const dcNumber *_left,
                                          const dcNumber *_right)
{
    int64_t result = CAST_INTEGER(_left) + CAST_INTEGER(_right);
    dcNumberResult numberResult = TAFFY_NUMBER_SUCCESS;

    if (OUT_OF_BOUNDS(result))
    {
        numberResult = outOfBoundsConvert(_result,
                                          _left,
                                          _right,
                                          &addDecNumberToDecNumber);
    }
    else
    {
        dcNumber_setInt32sValue(_result, (int32_t)result);
    }

    return numberResult;
}

static dcNumberResult addIntegerToDecNumber(dcNumber *_result,
                                            const dcNumber *_left,
                                            const dcNumber *_right)
{
    return integerAndDecNumberOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_add,
                                        false);
}

static dcNumber_arithmeticFunction sIntegerAddFunctions[] =
{
    &addIntegerToInteger,
    &addIntegerToDecNumber
};

static dcNumberResult addDecNumberToInteger(dcNumber *_result,
                                            const dcNumber *_left,
                                            const dcNumber *_right)
{
    return addIntegerToDecNumber(_result, _right, _left);
}

static dcNumberResult addDecNumberToDecNumber(dcNumber *_result,
                                              const dcNumber *_left,
                                              const dcNumber *_right)
{
    convertToDecNumber(_result);
    dcDecNumber_add(CAST_DEC_NUMBER(_result),
                    CAST_DEC_NUMBER(_left),
                    CAST_DEC_NUMBER(_right));
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumber_arithmeticFunction sDecNumberAddFunctions[] =
{
    &addDecNumberToInteger,
    &addDecNumberToDecNumber
};

static const dcNumber_arithmeticFunction *sAddFunctions[] =
{
    sIntegerAddFunctions,
    sDecNumberAddFunctions
};

dcNumberResult dcNumber_add(dcNumber *_result,
                            const dcNumber *_left,
                            const dcNumber *_right)
{
    return (*sAddFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

static dcNumberResult subtractDecNumberAndInteger(dcNumber *_result,
                                                  const dcNumber *_left,
                                                  const dcNumber *_right)
{
    return decNumberAndIntegerOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_subtract,
                                        false);
}

static dcNumberResult subtractDecNumberAndDecNumber(dcNumber *_result,
                                                    const dcNumber *_left,
                                                    const dcNumber *_right)
{
    convertToDecNumber(_result);
    dcDecNumber_subtract(CAST_DEC_NUMBER(_result),
                         CAST_DEC_NUMBER(_left),
                         CAST_DEC_NUMBER(_right));
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumber_arithmeticFunction sDecNumberSubtractFunctions[] =
{
    &subtractDecNumberAndInteger,
    &subtractDecNumberAndDecNumber
};

static dcNumberResult subtractIntegerAndInteger(dcNumber *_result,
                                                const dcNumber *_left,
                                                const dcNumber *_right)
{
    int64_t result = CAST_INTEGER(_left) - CAST_INTEGER(_right);

    if (OUT_OF_BOUNDS(result))
    {
        outOfBoundsConvert(_result,
                           _left,
                           _right,
                           &subtractDecNumberAndDecNumber);
    }
    else
    {
        dcNumber_setInt32sValue(_result, (int32_t)result);
    }

    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult subtractIntegerAndDecNumber(dcNumber *_result,
                                                  const dcNumber *_left,
                                                  const dcNumber *_right)
{
    return integerAndDecNumberOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_subtract,
                                        false);
}

static dcNumber_arithmeticFunction sIntegerSubtractFunctions[] =
{
    &subtractIntegerAndInteger,
    &subtractIntegerAndDecNumber
};

static const dcNumber_arithmeticFunction *sSubtractFunctions[] =
{
    sIntegerSubtractFunctions,
    sDecNumberSubtractFunctions
};

dcNumberResult dcNumber_subtract(dcNumber *_result,
                                 const dcNumber *_left,
                                 const dcNumber *_right)
{
    return (*sSubtractFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

static dcNumberResult multiplyDecNumberWithInteger(dcNumber *_result,
                                                   const dcNumber *_left,
                                                   const dcNumber *_right)
{
    return decNumberAndIntegerOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_multiply,
                                        false);
}

static dcNumberResult multiplyDecNumberWithDecNumber(dcNumber *_result,
                                                     const dcNumber *_left,
                                                     const dcNumber *_right)
{
    convertToDecNumber(_result);
    dcDecNumber_multiply(CAST_DEC_NUMBER(_result),
                         CAST_DEC_NUMBER(_left),
                         CAST_DEC_NUMBER(_right));
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumber_arithmeticFunction sDecNumberMultiplyFunctions[] =
{
    &multiplyDecNumberWithInteger,
    &multiplyDecNumberWithDecNumber
};

static dcNumberResult multiplyIntegerWithInteger(dcNumber *_result,
                                                 const dcNumber *_left,
                                                 const dcNumber *_right)
{
    int64_t result = CAST_INTEGER(_left) * CAST_INTEGER(_right);

    if (OUT_OF_BOUNDS(result))
    {
        outOfBoundsConvert(_result,
                           _left,
                           _right,
                           &multiplyDecNumberWithDecNumber);
    }
    else
    {
        dcNumber_setInt32sValue(_result, (int32_t)result);
    }

    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult multiplyIntegerWithDecNumber(dcNumber *_result,
                                                   const dcNumber *_left,
                                                   const dcNumber *_right)
{
    return multiplyDecNumberWithInteger(_result, _right, _left);
}

static dcNumber_arithmeticFunction sIntegerMultiplyFunctions[] =
{
    &multiplyIntegerWithInteger,
    &multiplyIntegerWithDecNumber
};

static const dcNumber_arithmeticFunction *sMultiplyFunctions[] =
{
    sIntegerMultiplyFunctions,
    sDecNumberMultiplyFunctions
};

dcNumberResult dcNumber_multiply(dcNumber *_result,
                                 const dcNumber *_left,
                                 const dcNumber *_right)
{
    return (*sMultiplyFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

static bool tryToFloor(dcNumber *_number)
{
    if (dcNumber_isWhole(_number))
    {
        dcNumber_floor(_number, _number);
        return true;
    }

    return false;
}

static dcNumberResult divideDecNumberByInteger(dcNumber *_result,
                                               const dcNumber *_left,
                                               const dcNumber *_right)
{
    dcNumberResult result = decNumberAndIntegerOperation(_result,
                                                         _left,
                                                         _right,
                                                         &dcDecNumber_divide,
                                                         false);
    tryToFloor(_result);
    return result;
}

static dcNumberResult divideDecNumberByDecNumber(dcNumber *_result,
                                                 const dcNumber *_left,
                                                 const dcNumber *_right)
{
    convertToDecNumber(_result);
    dcDecNumber_divide(CAST_DEC_NUMBER(_result),
                       CAST_DEC_NUMBER(_left),
                       CAST_DEC_NUMBER(_right));
    tryToFloor(_result);
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumber_arithmeticFunction sDecNumberDivideFunctions[] =
{
    &divideDecNumberByInteger,
    &divideDecNumberByDecNumber
};

static dcNumberResult divideIntegerByInteger(dcNumber *_result,
                                             const dcNumber *_left,
                                             const dcNumber *_right)
{
    dcNumberResult result = outOfBoundsConvert(_result,
                                               _left,
                                               _right,
                                               &divideDecNumberByDecNumber);
    tryToFloor(_result);
    return result;
}

static dcNumberResult divideIntegerByDecNumber(dcNumber *_result,
                                               const dcNumber *_left,
                                               const dcNumber *_right)
{
    dcNumberResult result = integerAndDecNumberOperation(_result,
                                                         _left,
                                                         _right,
                                                         &dcDecNumber_divide,
                                                         false);
    tryToFloor(_result);
    return result;
}

static dcNumber_arithmeticFunction sIntegerDivideFunctions[] =
{
    &divideIntegerByInteger,
    &divideIntegerByDecNumber
};

static const dcNumber_arithmeticFunction *sDivideFunctions[] =
{
    sIntegerDivideFunctions,
    sDecNumberDivideFunctions
};

dcNumberResult dcNumber_divide(dcNumber *_result,
                               const dcNumber *_left,
                               const dcNumber *_right)
{
    return (*sDivideFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

static dcNumberResult raiseDecNumberToInteger(dcNumber *_result,
                                              const dcNumber *_left,
                                              const dcNumber *_right)
{
    return decNumberAndIntegerOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_raise,
                                        false);
}

static dcNumberResult raiseDecNumberToDecNumber(dcNumber *_result,
                                                const dcNumber *_left,
                                                const dcNumber *_right)
{
    convertToDecNumber(_result);
    dcDecNumber_raise(CAST_DEC_NUMBER(_result),
                      CAST_DEC_NUMBER(_left),
                      CAST_DEC_NUMBER(_right));
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumber_arithmeticFunction sDecNumberRaiseFunctions[] =
{
    &raiseDecNumberToInteger,
    &raiseDecNumberToDecNumber
};

static dcNumberResult raiseIntegerToInteger(dcNumber *_result,
                                            const dcNumber *_left,
                                            const dcNumber *_right)
{
    return outOfBoundsConvert(_result,
                              _left,
                              _right,
                              &raiseDecNumberToDecNumber);
}

static dcNumberResult raiseIntegerToDecNumber(dcNumber *_result,
                                              const dcNumber *_left,
                                              const dcNumber *_right)
{
    return integerAndDecNumberOperation
        (_result, _left, _right, &dcDecNumber_raise, false);
}

static dcNumber_arithmeticFunction sIntegerRaiseFunctions[] =
{
    &raiseIntegerToInteger,
    &raiseIntegerToDecNumber
};

static const dcNumber_arithmeticFunction *sRaiseFunctions[] =
{
    sIntegerRaiseFunctions,
    sDecNumberRaiseFunctions
};

dcNumberResult dcNumber_raise(dcNumber *_result,
                              const dcNumber *_left,
                              const dcNumber *_right)
{
    return (*sRaiseFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

static dcNumberResult bitAndDecNumberAndInteger(dcNumber *_result,
                                                const dcNumber *_left,
                                                const dcNumber *_right)
{
    return decNumberAndIntegerOperation(_result,
                                        _left,
                                        _right,
                                        dcDecNumber_and,
                                        true);
}

static dcNumberResult bitAndDecNumberAndDecNumber(dcNumber *_result,
                                                  const dcNumber *_left,
                                                  const dcNumber *_right)
{
    convertToDecNumber(_result);
    return dcDecNumber_and(CAST_DEC_NUMBER(_result),
                           CAST_DEC_NUMBER(_left),
                           CAST_DEC_NUMBER(_right));
}

static dcNumber_arithmeticFunction sDecNumberBitAndFunctions[] =
{
    &bitAndDecNumberAndInteger,
    &bitAndDecNumberAndDecNumber
};

static dcNumberResult bitAndIntegerAndInteger(dcNumber *_result,
                                              const dcNumber *_left,
                                              const dcNumber *_right)
{
    convertToInteger(_result);
    CAST_INTEGER(_result) = CAST_INTEGER(_left) & CAST_INTEGER(_right);
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult bitAndIntegerAndDecNumber(dcNumber *_result,
                                                const dcNumber *_left,
                                                const dcNumber *_right)
{
    return integerAndDecNumberOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_and,
                                        true);
}

static dcNumber_arithmeticFunction sIntegerBitAndFunctions[] =
{
    &bitAndIntegerAndInteger,
    &bitAndIntegerAndDecNumber
};

static const dcNumber_arithmeticFunction *sBitAndFunctions[] =
{
    sIntegerBitAndFunctions,
    sDecNumberBitAndFunctions
};

dcNumberResult dcNumber_bitAnd(dcNumber *_result,
                               const dcNumber *_left,
                               const dcNumber *_right)
{
    return (*sBitAndFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

static dcNumberResult bitOrDecNumberAndInteger(dcNumber *_result,
                                               const dcNumber *_left,
                                               const dcNumber *_right)
{
    return decNumberAndIntegerOperation(_result,
                                        _left,
                                        _right,
                                        dcDecNumber_or,
                                        true);
}

static dcNumberResult bitOrDecNumberAndDecNumber(dcNumber *_result,
                                                 const dcNumber *_left,
                                                 const dcNumber *_right)
{
    convertToDecNumber(_result);
    return dcDecNumber_or(CAST_DEC_NUMBER(_result),
                          CAST_DEC_NUMBER(_left),
                          CAST_DEC_NUMBER(_right));
}

static dcNumber_arithmeticFunction sDecNumberBitOrFunctions[] =
{
    &bitOrDecNumberAndInteger,
    &bitOrDecNumberAndDecNumber
};

static dcNumberResult bitOrIntegerAndInteger(dcNumber *_result,
                                             const dcNumber *_left,
                                             const dcNumber *_right)
{
    convertToInteger(_result);
    CAST_INTEGER(_result) = CAST_INTEGER(_left) | CAST_INTEGER(_right);
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult bitOrIntegerAndDecNumber(dcNumber *_result,
                                               const dcNumber *_left,
                                               const dcNumber *_right)
{
    return integerAndDecNumberOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_or,
                                        true);
}

static dcNumber_arithmeticFunction sIntegerBitOrFunctions[] =
{
    &bitOrIntegerAndInteger,
    &bitOrIntegerAndDecNumber
};

static const dcNumber_arithmeticFunction *sBitOrFunctions[] =
{
    sIntegerBitOrFunctions,
    sDecNumberBitOrFunctions
};

dcNumberResult dcNumber_bitOr(dcNumber *_result,
                              const dcNumber *_left,
                              const dcNumber *_right)
{
    return (*sBitOrFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

static dcNumberResult bitXOrDecNumberAndInteger(dcNumber *_result,
                                                const dcNumber *_left,
                                                const dcNumber *_right)
{
    return decNumberAndIntegerOperation(_result,
                                        _left,
                                        _right,
                                        dcDecNumber_xor,
                                        true);
}

static dcNumberResult bitXOrDecNumberAndDecNumber(dcNumber *_result,
                                                  const dcNumber *_left,
                                                  const dcNumber *_right)
{
    convertToDecNumber(_result);
    return dcDecNumber_xor(CAST_DEC_NUMBER(_result),
                           CAST_DEC_NUMBER(_left),
                           CAST_DEC_NUMBER(_right));
}

static dcNumber_arithmeticFunction sDecNumberBitXOrFunctions[] =
{
    &bitXOrDecNumberAndInteger,
    &bitXOrDecNumberAndDecNumber
};

static dcNumberResult bitXOrIntegerAndInteger(dcNumber *_result,
                                              const dcNumber *_left,
                                              const dcNumber *_right)
{
    convertToInteger(_result);
    CAST_INTEGER(_result) = CAST_INTEGER(_left) ^ CAST_INTEGER(_right);
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult bitXOrIntegerAndDecNumber(dcNumber *_result,
                                                const dcNumber *_left,
                                                const dcNumber *_right)
{
    return integerAndDecNumberOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_xor,
                                        true);
}

static dcNumber_arithmeticFunction sIntegerBitXOrFunctions[] =
{
    &bitXOrIntegerAndInteger,
    &bitXOrIntegerAndDecNumber
};

static const dcNumber_arithmeticFunction *sBitXOrFunctions[] =
{
    sIntegerBitXOrFunctions,
    sDecNumberBitXOrFunctions
};

dcNumberResult dcNumber_bitXOr(dcNumber *_result,
                               const dcNumber *_left,
                               const dcNumber *_right)
{
    return (*sBitXOrFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

static dcNumberResult leftShiftDecNumberAndDecNumber(dcNumber *_result,
                                                     const dcNumber *_left,
                                                     const dcNumber *_right);

static dcNumberResult leftShiftIntegerAndInteger(dcNumber *_result,
                                                 const dcNumber *_left,
                                                 const dcNumber *_right)
{
    return outOfBoundsConvert(_result,
                              _left,
                              _right,
                              &leftShiftDecNumberAndDecNumber);
}

static dcNumberResult leftShiftIntegerAndDecNumber(dcNumber *_result,
                                                   const dcNumber *_left,
                                                   const dcNumber *_right)
{
    return integerAndDecNumberOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_leftShift,
                                        true);
}

static dcNumberResult leftShiftDecNumberAndInteger(dcNumber *_result,
                                                   const dcNumber *_left,
                                                   const dcNumber *_right)
{
    convertToDecNumber(_result);
    return decNumberAndIntegerOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_leftShift,
                                        true);
}

static dcNumberResult leftShiftDecNumberAndDecNumber(dcNumber *_result,
                                                     const dcNumber *_left,
                                                     const dcNumber *_right)
{
    convertToDecNumber(_result);
    return dcDecNumber_leftShift(CAST_DEC_NUMBER(_result),
                                 CAST_DEC_NUMBER(_left),
                                 CAST_DEC_NUMBER(_right));
}

static dcNumber_arithmeticFunction sIntegerLeftShiftFunctions[] =
{
    &leftShiftIntegerAndInteger,
    &leftShiftIntegerAndDecNumber
};

static dcNumber_arithmeticFunction sDecNumberLeftShiftFunctions[] =
{
    &leftShiftDecNumberAndInteger,
    &leftShiftDecNumberAndDecNumber
};

static const dcNumber_arithmeticFunction *sLeftShiftFunctions[] =
{
    sIntegerLeftShiftFunctions,
    sDecNumberLeftShiftFunctions
};

dcNumberResult dcNumber_leftShift(dcNumber *_result,
                                  const dcNumber *_left,
                                  const dcNumber *_right)
{
    return (*sLeftShiftFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

static dcNumberResult rightShiftDecNumberAndDecNumber(dcNumber *_result,
                                                      const dcNumber *_left,
                                                      const dcNumber *_right);

static dcNumberResult rightShiftIntegerAndInteger(dcNumber *_result,
                                                  const dcNumber *_left,
                                                  const dcNumber *_right)
{
    return outOfBoundsConvert(_result,
                              _left,
                              _right,
                              &rightShiftDecNumberAndDecNumber);
}

static dcNumberResult rightShiftIntegerAndDecNumber(dcNumber *_result,
                                                    const dcNumber *_left,
                                                    const dcNumber *_right)
{
    return integerAndDecNumberOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_rightShift,
                                        true);
}

static dcNumberResult rightShiftDecNumberAndInteger(dcNumber *_result,
                                                    const dcNumber *_left,
                                                    const dcNumber *_right)
{
    convertToDecNumber(_result);
    return decNumberAndIntegerOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_rightShift,
                                        true);
}

static dcNumberResult rightShiftDecNumberAndDecNumber(dcNumber *_result,
                                                      const dcNumber *_left,
                                                      const dcNumber *_right)
{
    convertToDecNumber(_result);
    return dcDecNumber_rightShift(CAST_DEC_NUMBER(_result),
                                  CAST_DEC_NUMBER(_left),
                                  CAST_DEC_NUMBER(_right));
}

static dcNumber_arithmeticFunction sIntegerRightShiftFunctions[] =
{
    &rightShiftIntegerAndInteger,
    &rightShiftIntegerAndDecNumber
};

static dcNumber_arithmeticFunction sDecNumberRightShiftFunctions[] =
{
    &rightShiftDecNumberAndInteger,
    &rightShiftDecNumberAndDecNumber
};

static const dcNumber_arithmeticFunction *sRightShiftFunctions[] =
{
    sIntegerRightShiftFunctions,
    sDecNumberRightShiftFunctions
};

dcNumberResult dcNumber_rightShift(dcNumber *_result,
                                   const dcNumber *_left,
                                   const dcNumber *_right)
{
    return (*sRightShiftFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

dcNumberResult dcNumber_squareRoot(dcNumber *_result, const dcNumber *_number)
{
    convertIntegerToDecNumber(_result);

    if (IS_INTEGER(_number))
    {
        dcNumber *number = copyAndConvertToDecNumber(_number);
        dcDecNumber_squareRoot(CAST_DEC_NUMBER(_result),
                               CAST_DEC_NUMBER(number));
        dcNumber_free(&number, DC_DEEP);
    }
    else if (IS_DEC_NUMBER(_number))
    {
        dcDecNumber_squareRoot(CAST_DEC_NUMBER(_result),
                               CAST_DEC_NUMBER(_number));
    }

    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult modulusIntegerAndInteger(dcNumber *_result,
                                               const dcNumber *_left,
                                               const dcNumber *_right)
{
    convertToInteger(_result);
    CAST_INTEGER(_result) = CAST_INTEGER(_left) % CAST_INTEGER(_right);
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult modulusIntegerAndDecNumber(dcNumber *_result,
                                                 const dcNumber *_left,
                                                 const dcNumber *_right)
{
    return integerAndDecNumberOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_modulus,
                                        false);
}

static dcNumberResult modulusDecNumberAndInteger(dcNumber *_result,
                                                 const dcNumber *_left,
                                                 const dcNumber *_right)
{
    return decNumberAndIntegerOperation(_result,
                                        _left,
                                        _right,
                                        &dcDecNumber_modulus,
                                        false);
}

static dcNumberResult modulusDecNumberAndDecNumber(dcNumber *_result,
                                                   const dcNumber *_left,
                                                   const dcNumber *_right)
{
    convertToDecNumber(_result);
    return dcDecNumber_modulus(CAST_DEC_NUMBER(_result),
                               CAST_DEC_NUMBER(_left),
                               CAST_DEC_NUMBER(_right));
}

static dcNumber_arithmeticFunction sDecNumberModulusFunctions[] =
{
    &modulusDecNumberAndInteger,
    &modulusDecNumberAndDecNumber
};

static dcNumber_arithmeticFunction sIntegerModulusFunctions[] =
{
    &modulusIntegerAndInteger,
    &modulusIntegerAndDecNumber
};

static const dcNumber_arithmeticFunction *sModulusFunctions[] =
{
    sIntegerModulusFunctions,
    sDecNumberModulusFunctions
};

dcNumberResult dcNumber_modulus(dcNumber *_result,
                                const dcNumber *_left,
                                const dcNumber *_right)
{
    return (*sModulusFunctions[_left->type][_right->type])
        (_result, _left, _right);
}

static dcNumberResult decNumberOperation
    (dcNumber *_result,
     const dcNumber *_value,
     dcDecNumber_singleOperandArithmeticFunction _function)
{
    convertToDecNumber(_result);
    dcNumberResult realResult = TAFFY_NUMBER_SUCCESS;

    if (_value->type == NUMBER_INTEGER_TYPE)
    {
        dcNumber *valueCopy = copyAndConvertToDecNumber(_value);
        realResult = _function(CAST_DEC_NUMBER(_result),
                               CAST_DEC_NUMBER(valueCopy));
        dcNumber_free(&valueCopy, DC_DEEP);
    }
    else
    {
        realResult = _function(CAST_DEC_NUMBER(_result),
                               CAST_DEC_NUMBER(_value));
    }

    return realResult;
}

dcNumberResult dcNumber_lg(dcNumber *_result, const dcNumber *_value)
{
    return decNumberOperation(_result, _value, &dcDecNumber_lg);
}

dcNumberResult dcNumber_ln(dcNumber *_result, const dcNumber *_value)
{
    return decNumberOperation(_result, _value, &dcDecNumber_ln);
}

dcNumberResult dcNumber_log10(dcNumber *_result, const dcNumber *_value)
{
    return decNumberOperation(_result, _value, &dcDecNumber_log10);
}

dcNumberResult dcNumber_increment(dcNumber *_number)
{
    if (_number->type == NUMBER_INTEGER_TYPE)
    {
        if (CAST_INTEGER(_number) > 0
            && (((uint32_t)CAST_INTEGER(_number)) + 1) > MAX_INT_32)
        {
            convertIntegerToDecNumber(_number);
        }
        else
        {
            CAST_INTEGER(_number)++;
        }
    }

    if (_number->type == NUMBER_DEC_NUMBER_TYPE)
    {
        dcDecNumber_increment(CAST_DEC_NUMBER(_number));
    }

    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcNumber_decrement(dcNumber *_number)
{
    if (_number->type == NUMBER_INTEGER_TYPE)
    {
        if (_number->types.integer < 0
            && _number->types.integer - 1 > 0)
        {
            // rollover
            convertIntegerToDecNumber(_number);
        }
        else
        {
            CAST_INTEGER(_number)--;
        }
    }

    if (_number->type == NUMBER_DEC_NUMBER_TYPE)
    {
        dcDecNumber_decrement(CAST_DEC_NUMBER(_number));
    }

    return TAFFY_NUMBER_SUCCESS;
}

dcString *dcNumber_marshall(const dcNumber *_number, dcString *_stream)
{
    char *display = dcNumber_display(_number);
    dcString *result = dcMarshaller_marshall(_stream, "s", display);
    dcMemory_free(display);
    return result;
}

dcNumber *dcNumber_unmarshall(dcString *_stream)
{
    char *display;
    dcNumber *result = NULL;

    if (dcMarshaller_unmarshallNoNull(_stream, "s", &display))
    {
        bool success = true;
        size_t i;
        size_t length = strlen(display);
        bool oneE = false;
        bool hasDot = false;

        // verify it's a valid number
        for (i = 0; i < length; i++)
        {
            if (display[i] < 0
                || (! (display[i] >= '0'
                       && display[i] <= '9')
                    && display[i] != '.'
                    && display[i] != '-'
                    && display[i] != 'E')
                || (display[i] == '-'
                    && i > 0)
                || (display[i] == 'E'
                    && oneE)
                || (display[i] == '.'
                    && hasDot))
            {
                success = false;
                break;
            }

            if (display[i] == 'E')
            {
                oneE = true;
            }
            else if (display[i] == '.')
            {
                hasDot = true;
            }
        }

        if (success
            && (! hasDot
                || length >= 3))
        {
            result = dcNumber_createFromString(display);
        }

        dcMemory_free(display);
    }

    return result;
}

dcNumberResult dcNumber_random(dcNumber *_result)
{
    convertToInteger(_result);
    CAST_INTEGER(_result) = rand();
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcNumber_factorial(dcNumber *_result, const dcNumber *_value)
{
    return decNumberOperation(_result, _value, &dcDecNumber_factorial);
}

dcNumberResult dcNumber_bitNot(dcNumber *_result, const dcNumber *_value)
{
    dcNumber_inlineCopy(_result, _value);
    convertToDecNumber(_result);
    dcDecNumber_bitNot(CAST_DEC_NUMBER(_result), CAST_DEC_NUMBER(_value));
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult chooseDecNumberAndInteger(dcNumber *_result,
                                                const dcNumber *_top,
                                                const dcNumber *_bottom)
{
    return decNumberAndIntegerOperation(_result,
                                        _top,
                                        _bottom,
                                        &dcDecNumber_choose,
                                        true);
}

static dcNumberResult chooseDecNumberAndDecNumber(dcNumber *_result,
                                                  const dcNumber *_top,
                                                  const dcNumber *_bottom)
{
    convertToDecNumber(_result);
    dcDecNumber_choose(CAST_DEC_NUMBER(_result),
                       CAST_DEC_NUMBER(_top),
                       CAST_DEC_NUMBER(_bottom));
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult chooseIntegerAndInteger(dcNumber *_result,
                                              const dcNumber *_top,
                                              const dcNumber *_bottom)
{
    return outOfBoundsConvert(_result,
                              _top,
                              _bottom,
                              &chooseDecNumberAndDecNumber);
}

static dcNumberResult chooseIntegerAndDecNumber(dcNumber *_result,
                                                const dcNumber *_top,
                                                const dcNumber *_bottom)
{
    return integerAndDecNumberOperation(_result,
                                        _top,
                                        _bottom,
                                        &dcDecNumber_choose,
                                        false);
}

static dcNumber_arithmeticFunction sDecNumberChooseFunctions[] =
{
    &chooseDecNumberAndInteger,
    &chooseDecNumberAndDecNumber
};

static dcNumber_arithmeticFunction sIntegerChooseFunctions[] =
{
    &chooseIntegerAndInteger,
    &chooseIntegerAndDecNumber
};

static const dcNumber_arithmeticFunction *sChooseFunctions[] =
{
    sIntegerChooseFunctions,
    sDecNumberChooseFunctions
};

dcNumberResult dcNumber_choose(dcNumber *_result,
                               const dcNumber *_top,
                               const dcNumber *_bottom)
{
    return (*sChooseFunctions[_top->type][_bottom->type])
        (_result, _top, _bottom);
}

// euclid's algorithm
// could be sped up, yep yep
dcNumberResult dcNumber_gcd(dcNumber *_result,
                            const dcNumber *_left,
                            const dcNumber *_right)
{
    if (dcNumber_equals(_right, sConstants[0]))
    {
        dcNumber_inlineCopy(_result, _left);
        dcNumber_absoluteValue(_result, _result);
        tryToFloor(_result);
    }
    else if (dcNumber_equals(_left, sConstants[0]))
    {
        dcNumber_inlineCopy(_result, _right);
        dcNumber_absoluteValue(_result, _result);
        tryToFloor(_result);
    }
    else
    {
        dcNumber *modded = dcNumber_copy(_left, DC_DEEP);
        dcNumber_modulus(modded, modded, _right);
        dcNumber_gcd(_result, _right, modded);
        dcNumber_free(&modded, DC_DEEP);
    }

    return TAFFY_NUMBER_SUCCESS;
}

bool dcNumber_isNonNegative(const dcNumber *_number)
{
    return dcNumber_greaterThanOrEqual(_number, sConstants[0]);
}

bool dcNumber_isPositive(const dcNumber *_number)
{
    return dcNumber_greaterThan(_number, sConstants[0]);
}

void dcNumber_inlineCopyDecNumberToDecNumber(decNumber *_to,
                                             const decNumber *_from)
{
    _to->digits = _from->digits;
    _to->exponent = _from->exponent;
    _to->bits = _from->bits;
    _to->lsuSize = _from->lsuSize;

    if (_to->lsu != NULL)
    {
        dcMemory_free(_to->lsu);
    }

    _to->lsu = (uint8_t *)dcMemory_duplicate(_from->lsu, _from->lsuSize);
}

void dcNumber_inlineCopy(dcNumber *_to, const dcNumber *_from)
{
    if (_to != _from)
    {
        if (_from->type == NUMBER_INTEGER_TYPE)
        {
            if (_to->type == NUMBER_DEC_NUMBER_TYPE)
            {
                dcDecNumber_free(&_to->types.decNumber);
            }

            _to->types.integer = _from->types.integer;
        }
        else if (_from->type == NUMBER_DEC_NUMBER_TYPE)
        {
            if (_to->type == NUMBER_DEC_NUMBER_TYPE)
            {
                dcNumber_inlineCopyDecNumberToDecNumber(CAST_DEC_NUMBER(_to),
                                                        CAST_DEC_NUMBER(_from));
            }
            else
            {
                CAST_DEC_NUMBER(_to) = dcDecNumber_copy(CAST_DEC_NUMBER(_from));
            }
        }

        _to->type = _from->type;
    }
}

void dcNumber_round(dcNumber *_number)
{
    if (IS_DEC_NUMBER(_number))
    {
        CAST_DEC_NUMBER(_number) = dcDecNumber_round(CAST_DEC_NUMBER(_number));
    }
}

dcNumber *dcNumber_getNaN(void)
{
    dcNumber *result = create(NUMBER_DEC_NUMBER_TYPE);
    CAST_DEC_NUMBER(result) = dcDecNumber_createFromDouble(0);
    CAST_DEC_NUMBER(result)->bits = DECNAN;
    return result;
}

bool dcNumber_isNaN(const dcNumber *_number)
{
    return (IS_DEC_NUMBER(_number)
            && (CAST_DEC_NUMBER(_number)->bits & DECNAN) != 0);
}

dcNumber *dcNumber_castMe(struct dcNode_t *_node)
{
    return CAST_NUMBER(_node);
}

uint32_t dcNumber_getDigitLimit(const dcNumber *_number)
{
    return (IS_INTEGER(_number)
            ? 0
            : CAST_DEC_NUMBER(_number)->lsuSize);
}

int32_t dcNumber_getExponent(const dcNumber *_number)
{
    return (IS_INTEGER(_number)
            ? 0
            : CAST_DEC_NUMBER(_number)->exponent);
}

static void checkFactorPairMatch(int32_t _value,
                                 int32_t _i,
                                 dcList *_list,
                                 bool _isNegative,
                                 bool _wantObjects)
{
    if (_value % _i == 0)
    {
        int32_t divided = _value / _i;

        if (_wantObjects)
        {
            dcList_push(_list,
                        dcPairClass_createObject
                        (dcNumberClass_createObjectFromInt32s(_i),
                         dcNumberClass_createObjectFromInt32s(divided),
                         true));
        }
        else
        {
            dcList_push(_list,
                        dcPair_createNode(dcInt32_createNode(_i),
                                          dcInt32_createNode(divided)));
        }

        if (_isNegative && abs(divided) != abs(_i))
        {
            if (_wantObjects)
            {
                dcList_push(_list,
                            dcPairClass_createObject
                            (dcNumberClass_createObjectFromInt32s(-_i),
                             dcNumberClass_createObjectFromInt32s(-divided),
                             true));
            }
            else
            {
                dcList_push(_list,
                            dcPair_createNode(dcInt32_createNode(-_i),
                                              dcInt32_createNode(-divided)));
            }
        }
    }
}

dcList *dcNumber_getFactorPairs(const dcNumber *_number,
                                bool _wantObjects)
{
    return dcNumber_getFactorPairsWithLimit(_number,
                                            DEFAULT_FACTOR_LIMIT,
                                            _wantObjects);
}

dcList *dcNumber_getFactorPairsWithLimit(const dcNumber *_number,
                                         uint32_t _maxIterations,
                                         bool _wantObjects)
{
    dcList *result = NULL;

    if (IS_INTEGER(_number)
        && CAST_INTEGER(_number) != 0)
    {
        int32_t value = CAST_INTEGER(_number);
        int32_t squareRoot = 0;
        result = dcList_create();

        if (value > 0)
        {
            squareRoot = (int32_t)sqrt((double)value) + 1;
        }
        else
        {
            squareRoot = (int32_t)sqrt((double)-value) + 1;
        }

        if (value > 0)
        {
            uint32_t i;

            for (i = 1; i < (uint32_t)squareRoot && i < _maxIterations; i++)
            {
                checkFactorPairMatch(value, i, result, false, _wantObjects);
            }
        }
        else
        {
            int32_t i;
            uint32_t count;

            for (i = -1, count = 0;
                 i > -squareRoot && count < _maxIterations;
                 i--, count++)
            {
                if (value % i == 0)
                {
                    checkFactorPairMatch(value, i, result, true, _wantObjects);
                }
            }
        }
    }

    return result;
}

static void checkFactorMatch(int32_t _value,
                             int32_t _i,
                             dcList *_list,
                             bool _isNegative,
                             bool _wantObjects)
{
    if (_value % _i == 0)
    {
        int32_t divided = _value / _i;

        if (_wantObjects)
        {
            dcList_push(_list, dcNumberClass_createObjectFromInt32s(_i));
        }
        else
        {
            dcList_push(_list, dcInt32_createNode(_i));
        }

        if (_isNegative && abs(divided) != abs(_i))
        {
            if (_wantObjects)
            {
                dcList_push(_list, dcNumberClass_createObjectFromInt32s(-_i));
            }
            else
            {
                dcList_push(_list, dcInt32_createNode(-_i));
            }
        }
    }
}

dcList *dcNumber_getFactors(const dcNumber *_number,
                            bool _wantObjects)
{
    return dcNumber_getFactorsWithLimit(_number,
                                        DEFAULT_FACTOR_LIMIT,
                                        _wantObjects);
}

dcList *dcNumber_getFactorsWithLimit(const dcNumber *_number,
                                     uint32_t _maxIterations,
                                     bool _wantObjects)
{
    dcList *result = NULL;

    if (IS_INTEGER(_number)
        && CAST_INTEGER(_number) != 0)
    {
        int32_t value = CAST_INTEGER(_number);
        int32_t half = value / 2 + 1;
        result = dcList_create();

        if (value > 0)
        {
            int32_t i;

            for (i = 1; i < half && i < (int32_t)_maxIterations; i++)
            {
                checkFactorMatch(value, i, result, false, _wantObjects);
                checkFactorMatch(-value, -i, result, true, _wantObjects);
            }
        }
        else
        {
            int32_t i;
            uint32_t count;

            for (i = -1, count = 0;
                 i > half && count < _maxIterations;
                 i--, count++)
            {
                if (value % i == 0)
                {
                    checkFactorMatch(value, i, result, true, _wantObjects);
                }
            }

            for (i = 1;
                 ((uint32_t)i < (uint32_t)half
                  && (uint32_t)count < _maxIterations);
                 i++, count++)
            {
                checkFactorMatch(-value, i, result, false, _wantObjects);
            }

            if (value == -1)
            {
                checkFactorMatch(-value, 1, result, false, _wantObjects);
            }
        }

        checkFactorMatch(value, value, result, false, _wantObjects);
        checkFactorMatch(-value, -value, result, true, _wantObjects);
    }

    return result;
}
