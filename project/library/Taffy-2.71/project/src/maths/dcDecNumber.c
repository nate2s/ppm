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
#include <math.h>
#include <string.h>

#include "dcDecNumber.h"
#include "dcDefines.h"
#include "dcError.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNumber.h"
#include "dcString.h"
#include "dcUnsignedInt32.h"

#ifndef TAFFY_WINDOWS
    #include <inttypes.h>
#endif

static uint32_t sLsuSize = 32;
static decNumber *sZero = NULL;
static decNumber *sOne = NULL;
static decNumber *sTwo = NULL;
static decNumber *sNegativeOne = NULL;
static uint8_t sDeltaCount = 0;
static decNumber **sDeltas = NULL;
static decNumber *sPi = NULL;
static decNumber *sTwoPi = NULL;
static decNumber *sHalfPi = NULL;
static decNumber *sNegativeHalfPi = NULL;

decNumber *dcDecNumber_createWithLsuSize(uint32_t _lsuSize)
{
    decNumber *result = (decNumber *)dcMemory_allocate(sizeof(decNumber));
    result->lsu = (uint8_t *)(dcMemory_allocateAndInitialize
                              (sizeof(decNumberUnit) * _lsuSize));
    result->digits = 0;
    result->lsuSize = _lsuSize;
    result->bits = 0;
    result->exponent = 0;
    return result;
}

decNumber *dcDecNumber_create(void)
{
    return dcDecNumber_createWithLsuSize(sLsuSize);
}

decNumber *dcDecNumber_createFromString(const char *_string)
{
    size_t length = strlen(_string);

    // TODO: account for overflow
    if (length > (uint32_t)-1)
    {
        return NULL;
    }

    // is there a dot in it?
    uint32_t dot = 0;
    uint32_t i;

    for (i = 0; i < (uint32_t)length; i++)
    {
        if (_string[i] == '.')
        {
            dot = 1;
            break;
        }
    }

    decContext set;
    decContextDefault(&set, DEC_INIT_BASE);
    uint32_t lsuSize = (dot
                        ? (uint32_t)length - 1
                        : (uint32_t)length);
    set.digits = (lsuSize > sLsuSize
                  ? lsuSize
                  : sLsuSize);
    decNumber *result = dcDecNumber_createWithLsuSize(set.digits);
    decNumberFromString(result, _string, &set);
    return result;
}

void dcDecNumber_free(decNumber **_number)
{
    if ((*_number)->lsu != NULL)
    {
        dcMemory_free((*_number)->lsu);
    }

    dcMemory_free(*_number);
}

#ifndef TAFFY_WINDOWS
    #define EASY_NUMBER_CREATOR(_name, _value)      \
        decNumber _name;                            \
        memset(&_name, 0, sizeof(decNumber));       \
        decNumberUnit _name##Lsu[sLsuSize];         \
        _name.lsu = _name##Lsu;                     \
        _name.lsuSize = sLsuSize;                   \
        memset(_name.lsu, 0, sLsuSize);             \
        _name.lsu[0] = _value;                      \
        _name.digits = 1;
#else
    // windows uses c++
#include <vector>

    #define EASY_NUMBER_CREATOR(_name, _value)      \
        decNumber _name;                            \
        memset(&_name, 0, sizeof(decNumber));       \
        std::vector<decNumberUnit> _name##Lsu(sLsuSize); \
        _name.lsu = &_name##Lsu[0];                  \
        _name.lsuSize = sLsuSize;                   \
        memset(_name.lsu, 0, sLsuSize);             \
        _name.lsu[0] = _value;                      \
        _name.digits = 1;
#endif

#define EASY_CONTEXT_CREATOR(_name, _result)               \
    decContext _name;                                      \
    decContextDefault(&_name, DEC_INIT_BASE);              \
    _name.traps = 0;                                       \
    _name.digits = (_result)->lsuSize;

dcResult dcDecNumber_compare(const decNumber *_left,
                             const decNumber *_right,
                             dcTaffyOperator *_compareResult)
{
    if (decNumberIsNaN(_left) && decNumberIsNaN(_right))
    {
        *_compareResult = TAFFY_EQUALS;
    }
    else if (decNumberIsNaN(_left))
    {
        *_compareResult = TAFFY_LESS_THAN;
    }
    else if (decNumberIsNaN(_right))
    {
        *_compareResult = TAFFY_GREATER_THAN;
    }
    else
    {
        EASY_NUMBER_CREATOR(comparison, 0);
        EASY_CONTEXT_CREATOR(set, &comparison);
        decNumberCompare(&comparison, _left, _right, &set);
        dcError_assert(decNumberIsFinite(&comparison));
        *_compareResult = (comparison.lsu[0] == 0
                           ? TAFFY_EQUALS
                           : ((comparison.lsu[0] == 1
                               && (comparison.bits & DECNEG) != 0)
                              ? TAFFY_LESS_THAN
                              : TAFFY_GREATER_THAN));
    }

    return TAFFY_SUCCESS;
}

dcTaffyOperator dcDecNumber_easyCompare(const decNumber *_left,
                                        const decNumber *_right)
{
    dcTaffyOperator comparisonResult;
    dcError_assert(dcDecNumber_compare(_left,
                                       _right,
                                       &comparisonResult)
                   == TAFFY_SUCCESS);
    return comparisonResult;
}

dcTaffyOperator dcDecNumber_easyCompareToInt32u(const decNumber *_left,
                                                uint32_t _right)
{
    decNumber *right = dcDecNumber_createFromInt32u(_right);
    dcTaffyOperator result = dcDecNumber_easyCompare(_left, right);
    dcDecNumber_free(&right);
    return result;
}

dcTaffyOperator dcDecNumber_easyCompareToInt32s(const decNumber *_left,
                                                int32_t _right)
{
    decNumber *right = dcDecNumber_createFromInt32s(_right);
    dcTaffyOperator result = dcDecNumber_easyCompare(_left, right);
    dcDecNumber_free(&right);
    return result;
}

decNumber *dcDecNumber_copy(const decNumber *_number)
{
    decNumber *result =
        (decNumber *)dcMemory_allocateAndInitialize(sizeof(decNumber));
    dcNumber_inlineCopyDecNumberToDecNumber(result, _number);
    return result;
}

decNumber *dcDecNumber_negate(decNumber *_number)
{
    _number->bits ^= DECNEG; // invert the sign
    return _number;
}

void dcDecNumber_setLsuSize(uint32_t _lsuSize)
{
    sLsuSize = _lsuSize;
}

uint32_t dcDecNumber_getLsuSize(void)
{
    return sLsuSize;
}

#define HUNDREDS_DIGIT(value) (value / 10)
#define TENS_DIGIT(value) (value - ((value / 10) * 10))

bool dcDecNumber_isExact(const decNumber *_value)
{
    return ((_value->bits & DEC_Inexact) == 0);
}

bool dcDecNumber_isWhole(const decNumber *_value)
{
    bool isWhole = true;

    if (_value->exponent < 0)
    {
        uint32_t zeroBucket = (-_value->exponent) / DECDPUN;
        uint32_t i;

        for (i = 0; i < zeroBucket; i++)
        {
            if (_value->lsu[i] != 0)
            {
                isWhole = false;
                break;
            }
        }

        // is there one more digit left over?
        if (isWhole && i * DECDPUN < (uint32_t)(-_value->exponent))
        {
            TAFFY_DEBUG(dcError_assert(i * DECDPUN
                                       == (uint32_t)(-_value->exponent) - 1););

            if (TENS_DIGIT(_value->lsu[i]) != 0)
            {
                isWhole = false;
            }
        }
    }

    return isWhole;
}

dcNumberResult dcDecNumber_add(decNumber *_result,
                               const decNumber *_left,
                               const decNumber *_right)
{
    EASY_CONTEXT_CREATOR(set, _result);
    decNumberAdd(_result, _left, _right, &set);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_subtract(decNumber *_result,
                                    const decNumber *_left,
                                    const decNumber *_right)
{
    EASY_CONTEXT_CREATOR(set, _result);
    decNumberSubtract(_result, _left, _right, &set);
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult multiply(decNumber *_result,
                               const decNumber *_left,
                               const decNumber *_right,
                               decContext *_context)
{
    decNumberMultiply(_result, _left, _right, _context);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_multiply(decNumber *_result,
                                    const decNumber *_left,
                                    const decNumber *_right)
{
    EASY_CONTEXT_CREATOR(set, _result);
    return multiply(_result, _left, _right, &set);
}

dcNumberResult dcDecNumber_divide(decNumber *_result,
                                  const decNumber *_left,
                                  const decNumber *_right)
{
    EASY_CONTEXT_CREATOR(set, _result);
    decNumberDivide(_result, _left, _right, &set);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_raise(decNumber *_result,
                                 const decNumber *_left,
                                 const decNumber *_right)
{
    EASY_CONTEXT_CREATOR(set, _result);
    decNumberPower(_result, _left, _right, &set);
    return TAFFY_NUMBER_SUCCESS;
}

typedef dcNumberResult (*dcDecNumber_arithmeticAndContextFunction)
    (decNumber *_result,
     const decNumber *_left,
     const decNumber *_right,
     decContext *_context);


static void moveOver(decNumber *_to, decNumber *_from)
{
    if (_to != _from)
    {
        dcMemory_free(_to->lsu);
        memcpy(_to, _from, sizeof(decNumber));
    }

    _from->lsu = NULL;
    dcDecNumber_free(&_from);
}

dcNumberResult dcDecNumber_choose(decNumber *_result,
                                  const decNumber *_top,
                                  const decNumber *_bottom)
{
    EASY_CONTEXT_CREATOR(set, _result);
    decNumber *k = dcDecNumber_createWithLsuSize(_bottom->lsuSize);
    decNumberFromUInt32(k, 0);
    decNumber *result = dcDecNumber_createFromInt32u(1);
    EASY_NUMBER_CREATOR(one, 1);

    while (dcDecNumber_easyCompare(k, _bottom) == TAFFY_LESS_THAN)
    {
        // C[k + 1] = (C[k] * (_top - k)) / (k + 1)
        decNumber *topMinusK = dcDecNumber_copy(_top);
        dcDecNumber_subtract(topMinusK, topMinusK, k);
        dcDecNumber_multiply(result, result, topMinusK);

        decNumber *kPlusOne = dcDecNumber_copy(k);
        dcDecNumber_add(kPlusOne, kPlusOne, &one);
        dcDecNumber_divide(result, result, kPlusOne);

        dcDecNumber_free(&topMinusK);
        dcDecNumber_free(&kPlusOne);

        dcDecNumber_add(k, k, &one);
    }

    dcDecNumber_free(&k);
    moveOver(_result, result);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_log10(decNumber *_result, const decNumber *_value)
{
    EASY_CONTEXT_CREATOR(set, _result);
    decNumberLog10(_result, _value, &set);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_ln(decNumber *_result, const decNumber *_value)
{
    EASY_CONTEXT_CREATOR(set, _result);
    decNumberLn(_result, _value, &set);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_modulus(decNumber *_result,
                                   const decNumber *_left,
                                   const decNumber *_right)
{
    EASY_CONTEXT_CREATOR(set, _result);
    decNumberRemainder(_result, _left, _right, &set);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_and(decNumber *_result,
                               const decNumber *_left,
                               const decNumber *_right)
{
    uint8_t *left;
    uint32_t leftLength;
    uint8_t *right;
    uint32_t rightLength;

    if (! dcDecNumber_convertToBinary(_left, &left, &leftLength))
    {
        return TAFFY_NUMBER_NEED_INTEGER;
    }

    if (! dcDecNumber_convertToBinary(_right, &right, &rightLength))
    {
        dcMemory_free(left);
        return TAFFY_NUMBER_NEED_INTEGER;
    }

    if (leftLength < rightLength)
    {
        rightLength = leftLength;
    }

    if (rightLength < leftLength)
    {
        leftLength = rightLength;
    }

    uint32_t i;
    uint32_t j;

    for (i = 0, j = 0; i < leftLength && j < rightLength; i++, j++)
    {
        left[i] &= right[j];
    }

    // initialize to 0
    decNumberFromUInt32(_result, 0);

    dcDecNumber_convertToDecimal(left, leftLength, _result);

    dcMemory_free(left);
    dcMemory_free(right);
    return TAFFY_NUMBER_SUCCESS;
}

static dcNumberResult orOrXOrOperation(decNumber *_result,
                                       const decNumber *_left,
                                       const decNumber *_right,
                                       bool _or)
{
    uint8_t *left;
    uint32_t leftLength;
    uint8_t *right;
    uint32_t rightLength;

    if (! dcDecNumber_convertToBinary(_left, &left, &leftLength))
    {
        return TAFFY_NUMBER_NEED_INTEGER;
    }

    if (! dcDecNumber_convertToBinary(_right, &right, &rightLength))
    {
        dcMemory_free(left);
        return TAFFY_NUMBER_NEED_INTEGER;
    }

    uint32_t i;
    uint32_t j;

    if (rightLength > leftLength)
    {
        // have the biggest on the left
        uint8_t *temp = left;
        left = right;
        right = temp;
        uint32_t tempLength = leftLength;
        leftLength = rightLength;
        rightLength = tempLength;
    }

    for (i = 0, j = 0; i < leftLength && j < rightLength; i++, j++)
    {
        if (_or)
        {
            left[i] |= right[j];
        }
        else
        {
            left[i] ^= right[j];
        }
    }

    // initialize to 0
    decNumberFromUInt32(_result, 0);

    dcDecNumber_convertToDecimal(left, leftLength, _result);

    dcMemory_free(left);
    dcMemory_free(right);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_or(decNumber *_result,
                              const decNumber *_left,
                              const decNumber *_right)
{
    return orOrXOrOperation(_result, _left, _right, true);
}

dcNumberResult dcDecNumber_xor(decNumber *_result,
                               const decNumber *_left,
                               const decNumber *_right)
{
    return orOrXOrOperation(_result, _left, _right, false);
}

static void copyMerge(decNumber *_to, const decNumber *_from)
{
    // this might change the size of _to
    if (_to != _from)
    {
        dcMemory_free(_to->lsu);
        memcpy(_to, _from, sizeof(decNumber));
        _to->lsu = (uint8_t *)dcMemory_duplicate(_from->lsu, _from->lsuSize);
    }
}

decNumber *dcDecNumber_createFromInt32u(uint32_t _value)
{
    decNumber *result = dcDecNumber_create();
    decNumberFromUInt32(result, _value);
    return result;
}

decNumber *dcDecNumber_createFromInt32s(int32_t _value)
{
    decNumber *result = dcDecNumber_create();
    decNumberFromInt32(result, _value);
    return result;
}

decNumber *dcDecNumber_createFromDouble(double _value)
{
    // slow and silly, but easy
    char string[100] = {0};
    assert((size_t)sprintf(string, "%f", _value)
           < sizeof(string));
    return dcDecNumber_createFromString(string);
}

decNumber *dcDecNumber_createFromInt64u(uint64_t _value)
{
    // slow and silly, but easy
    char string[100] = {0};
#ifdef TAFFY_WINDOWS
    assert((size_t)sprintf(string, "%llu", _value)
           < sizeof(string));
#else
    assert((size_t)sprintf(string, "%" PRIu64, _value)
           < sizeof(string));
#endif
    return dcDecNumber_createFromString(string);
}

decNumber *dcDecNumber_createFromSizet(size_t _value)
{
    // slow and silly, but easy
    char string[100] = {0};
    assert((size_t)sprintf(string, "%zu", _value)
           < sizeof(string));
    return dcDecNumber_createFromString(string);
}

dcNumberResult dcDecNumber_bitNot(decNumber *_result,
                                  const decNumber *_value)
{
    uint8_t *bits = NULL;
    uint32_t bitsLength = 0;

    if (! dcDecNumber_convertToBinary(_value, &bits, &bitsLength))
    {
        return TAFFY_NUMBER_NEED_INTEGER;
    }

    uint32_t i;
    assert(bits != NULL);

    for (i = 0; i < bitsLength; i++)
    {
        bits[i] = (bits[i] == 1 ? 0 : 1);
    }

    // initialize to 0
    decNumberFromUInt32(_result, 0);
    dcDecNumber_convertToDecimal(bits, bitsLength, _result);

    dcMemory_free(bits);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_factorial(decNumber *_result,
                                     const decNumber *_value)
{
    dcNumberResult result = TAFFY_NUMBER_SUCCESS;

    if (dcDecNumber_isWhole(_value))
    {
        decNumber *factorialResult = NULL;
        EASY_NUMBER_CREATOR(one, 1);
        factorialResult = dcDecNumber_createFromInt32u(1);
        EASY_CONTEXT_CREATOR(context, factorialResult);
        decNumber *iterator = dcDecNumber_copy(_value);

        while (! decNumberIsZero(iterator))
        {
            decNumberMultiply(factorialResult,
                              factorialResult,
                              iterator,
                              &context);
            decNumberSubtract(iterator, iterator, &one, &context);
        }

        dcDecNumber_free(&iterator);
        moveOver(_result, factorialResult);
    }
    else
    {
        result = TAFFY_NUMBER_NEED_INTEGER;
    }

    return result;
}

dcNumberResult dcDecNumber_chomp(decNumber *_result,
                                 const decNumber *_value)
{
    return (decNumberIsNegative(_value)
            ? dcDecNumber_ceiling(_result, _value)
            : dcDecNumber_floor(_result, _value));
}

dcNumberResult dcDecNumber_floor(decNumber *_result,
                                 const decNumber *_value)
{
    EASY_CONTEXT_CREATOR(set, _result);
    copyMerge(_result, _value);

    if (! dcDecNumber_isWhole(_result))
    {
        set.round = DEC_ROUND_FLOOR;
        decNumberToIntegralExact(_result, _result, &set);
        //decNumberNormalize(_result, _result, &set);
    }

    decNumberQuantize(_result, _result, sZero, &set);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_ceiling(decNumber *_result,
                                   const decNumber *_value)
{
    EASY_CONTEXT_CREATOR(set, _result);
    copyMerge(_result, _value);

    if (! dcDecNumber_isWhole(_result))
    {
        set.round = DEC_ROUND_CEILING;
        decNumberToIntegralExact(_result, _result, &set);
    }

    decNumberQuantize(_result, _result, sZero, &set);
    return TAFFY_NUMBER_SUCCESS;
}

bool dcDecNumber_isPositive(const decNumber *_value)
{
    EASY_NUMBER_CREATOR(zero, 0);
    dcTaffyOperator comparison;

    return ((dcDecNumber_compare(_value, &zero, &comparison)
             == TAFFY_SUCCESS)
            && comparison == TAFFY_GREATER_THAN);
}

dcNumberResult dcDecNumber_leftShift(decNumber *_result,
                                     const decNumber *_left,
                                     const decNumber *_right)
{
    dcNumberResult result = TAFFY_NUMBER_SUCCESS;

    copyMerge(_result, _left);

    if (dcDecNumber_isWhole(_right))
    {
        EASY_NUMBER_CREATOR(zero, 0);
        dcTaffyOperator taffyOperator;
        dcResult compareResult =
            dcDecNumber_compare(_right, &zero, &taffyOperator);

        if (compareResult == TAFFY_SUCCESS)
        {
            if (taffyOperator == TAFFY_GREATER_THAN)
            {
                uint32_t status;

                if (! dcDecNumber_resize(_result, sLsuSize, &status))
                {
                    result = TAFFY_NUMBER_OUT_OF_MEMORY;
                    goto kickout;
                }

                decNumber *shiftAmount = dcDecNumber_createFromInt32u(0);
                EASY_NUMBER_CREATOR(two, 2);
                dcDecNumber_raise(shiftAmount, &two, _right);
                EASY_CONTEXT_CREATOR(resultSet, _result);
                decNumberMultiply
                    (_result, _left, shiftAmount, &resultSet);
                dcDecNumber_free(&shiftAmount);
            }
            else if (taffyOperator == TAFFY_LESS_THAN)
            {
                decNumber *right = dcDecNumber_copy(_right);
                right = dcDecNumber_negate(right);
                dcDecNumber_rightShift(_result, _left, right);
                dcDecNumber_free(&right);
            }
        }
    }
    else
    {
        result = TAFFY_NUMBER_NEED_INTEGER;
    }

kickout:
    return result;
}

dcNumberResult dcDecNumber_squareRoot(decNumber *_result,
                                      const decNumber *_number)
{
    EASY_CONTEXT_CREATOR(context, _result);
    decNumberSquareRoot(_result, _number, &context);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_rightShift(decNumber *_result,
                                      const decNumber *_left,
                                      const decNumber *_right)
{
    dcNumberResult result = TAFFY_NUMBER_SUCCESS;
    copyMerge(_result, _left);

    if (! dcDecNumber_isWhole(_left)
        || ! dcDecNumber_isWhole(_right))
    {
        result = TAFFY_NUMBER_NEED_INTEGER;
    }
    else
    {
        EASY_NUMBER_CREATOR(zero, 0);
        dcTaffyOperator taffyOperator;
        dcResult compareResult =
            dcDecNumber_compare(_right, &zero, &taffyOperator);

        if (compareResult == TAFFY_SUCCESS)
        {
            if (taffyOperator == TAFFY_GREATER_THAN)
            {
                decNumber *shiftAmount = dcDecNumber_copy(_right);
                EASY_NUMBER_CREATOR(one, 1);
                EASY_NUMBER_CREATOR(two, 2);
                dcDecNumber_raise(shiftAmount, &two, shiftAmount);
                dcDecNumber_divide(_result, _result, shiftAmount);
                dcDecNumber_floor(_result, _result);
                dcDecNumber_free(&shiftAmount);
            }
            else if (taffyOperator == TAFFY_LESS_THAN)
            {
                decNumber *right = dcDecNumber_copy(_right);
                right = dcDecNumber_negate(right);
                dcDecNumber_leftShift(_result, _left, right);
                dcDecNumber_free(&right);
            }
        }
    }

    return result;
}

char *dcDecNumber_display(const decNumber *_number)
{
    // be safe
    char *result = (char *)dcMemory_allocate((_number->lsuSize * 2) + 1);
    memset(result, 0, (_number->lsuSize * 2) + 1);
    return decNumberToString(_number, result);
}

char *dcDecNumber_displayWithGarbageCollection(const decNumber *_number)
{
    char *result = dcDecNumber_display(_number);
    dcNode_register(dcString_createNodeWithString(result, false));
    return result;
}

char *dcDecNumber_displayBytes(const decNumber *_number)
{
    dcString *result = dcString_create();
    dcString_append(result,
                    "[Count: %ld | exponent: %ld | bytes: ",
                    _number->digits,
                    _number->exponent);

    ssize_t i;

    for (i = 0; i < ceil((float)_number->digits / (float)DECDPUN); i++)
    {
        dcString_append(result, "%u ", _number->lsu[i]);
    }

    return dcString_freeAndReturn(&result);
}

static const uint32_t sFactorialDepth = 100;
static decNumber **sFactorials = NULL;

static void factorialize(decNumber **_number, uint32_t _value)
{
    *_number = dcDecNumber_createFromInt32u(_value);
    dcDecNumber_factorial(*_number, *_number);
}

static decNumber *getDelta(uint32_t _delta, bool *_freeIt)
{
    decNumber *result = NULL;

    if (_delta < sDeltaCount && sDeltas[_delta] != NULL)
    {
        result = sDeltas[_delta];

        if (_freeIt != NULL)
        {
            *_freeIt = false;
        }
    }
    else
    {
        result = dcDecNumber_createFromInt32u(1);
        decNumber *power = dcDecNumber_createFromInt32u(10);
        decNumber *delta = dcDecNumber_createFromInt32u(_delta);
        dcDecNumber_raise(power, power, delta);
        dcDecNumber_divide(result, result, power);
        dcDecNumber_free(&power);
        dcDecNumber_free(&delta);

        if (_freeIt != NULL)
        {
            *_freeIt = true;
        }
    }

    return result;
}

static const char *sBernoulliNumberStrings[] =
{
    "1/6",   // B(2)
    "-1/30", // B(4)
    "1/42",  // B(6)
    "-1/30", // etc...
    "5/66",
    "-691/2730",
    "7/6",
    "-3617/510",
    "43867/798",
    "-174611/330",
    "854513/138",
    "-236364091/2730",
    "8553103/6",
    "-23749461029/870",
    "8615841276005/14322",
    "-7709321041217/510",
    "2577687858367/6",
    "-26315271553053477373/1919190",
    "2929993913841559/6",
    "-261082718496449122051/13530",
    "1520097643918070802691/1806",
    "-27833269579301024235023/690",
    "596451111593912163277961/282",
    "-5609403368997817686249127547/46410",
    "495057205241079648212477525/66",
    "-801165718135489957347924991853/1590",
    "29149963634884862421418123812691/798",
    "-2479392929313226753685415739663229/870",
    "84483613348880041862046775994036021/354",
    "-1215233140483755572040304994079820246041491/56786730",
    "12300585434086858541953039857403386151/6",
    "-106783830147866529886385444979142647942017/510",
    "1472600022126335654051619428551932342241899101/64722",
    "-78773130858718728141909149208474606244347001/30",
    "1505381347333367003803076567377857208511438160235/4686",
    "-5827954961669944110438277244641067365282488301844260429/140100870",
    "34152417289221168014330073731472635186688307783087/6",
    "-24655088825935372707687196040585199904365267828865801/30",
    "414846365575400828295179035549542073492199375372400483487/3318",
    "-4603784299479457646935574969019046849794257872751288919656867/230010",
    "1677014149185145836823154509786269900207736027570253414881613/498",
    "-2024576195935290360231131160111731009989917391198090877281083932477/3404310",
    "660714619417678653573847847426261496277830686653388931761996983/6",
    "-1311426488674017507995511424019311843345750275572028644296919890574047/61410",
    "1179057279021082799884123351249215083775254949669647116231545215727922535/272118",
    "-1295585948207537527989427828538576749659341483719435143023316326829946247/1410",
    "1220813806579744469607301679413201203958508415202696621436215105284649447/6",
    "-211600449597266513097597728109824233673043954389060234150638733420050668349987259/4501770",
    "67908260672905495624051117546403605607342195728504487509073961249992947058239/6",
    "-94598037819122125295227433069493721872702841533066936133385696204311395415197247711/33330",
    "3204019410860907078243020782116241775491817197152717450679002501086861530836678158791/4326",
    "-319533631363830011287103352796174274671189606078272738327103470162849568365549721224053/1590",
    "36373903172617414408151820151593427169231298640581690038930816378281879873386202346572901/642",
    "-3469342247847828789552088659323852541399766785760491146870005891371501266319724897592306597338057/209191710",
    "7645992940484742892248134246724347500528752413412307906683593870759797606269585779977930217515/1518",
    "-2650879602155099713352597214685162014443151499192509896451788427680966756514875515366781203552600109/1671270",
    "21737832319369163333310761086652991475721156679090831360806110114933605484234593650904188618562649/42",
    "-309553916571842976912513458033841416869004128064329844245504045721008957524571968271388199595754752259/1770",
    "366963119969713111534947151585585006684606361080699204301059440676414485045806461889371776354517095799/6",
    "-51507486535079109061843996857849983274095170353262675213092869167199297474922985358811329367077682677803282070131/2328255930",
    "49633666079262581912532637475990757438722790311060139770309311793150683214100431329033113678098037968564431/6",
    "-95876775334247128750774903107542444620578830013297336819553512729358593354435944413631943610268472689094609001/30",
    "5556330281949274850616324408918951380525567307126747246796782304333594286400508981287241419934529638692081513802696639/4357878",
    "-267754707742548082886954405585282394779291459592551740629978686063357792734863530145362663093519862048495908453718017/510",
    "1928215175136130915645299522271596435307611010164728458783733020528548622403504078595174411693893882739334735142562418015/8646",
    "-410951945846993378209020486523571938123258077870477502433469747962650070754704863812646392801863686694106805747335370312946831/4206930",
    "264590171870717725633635737248879015151254525593168688411918554840667765591690540727987316391252434348664694639349484190167/6",
    "-84290226343367405131287578060366193649336612397547435767189206912230442242628212786558235455817749737691517685781164837036649737/4110",
    "2694866548990880936043851683724113040849078494664282483862150893060478501559546243423633375693325757795709438325907154973590288136429/274386",
    "-3289490986435898803930699548851884006880537476931130981307467085162504802973618096693859598125274741604181467826651144393874696601946049/679470",
    "14731853280888589565870080442453214239804217023990642676194878997407546061581643106569966189211748270209483494554402556608073385149191/6",
    "-3050244698373607565035155836901726357405007104256566761884191852434851033744761276392695669329626855965183503295793517411526056244431024612640493/2381714790",
    "4120570026280114871526113315907864026165545608808541153973817680034790262683524284855810008621905238290240143481403022987037271683989824863/6",
    "-1691737145614018979865561095112166189607682852147301400816480675916957871178648433284821493606361235973346584667336181793937950344828557898347149/4470",
    "463365579389162741443284425811806264982233725425295799852299807325379315501572305760030594769688296308375193913787703707693010224101613904227979066275/2162622",
    "-3737018141155108502105892888491282165837489531488932951768507127182409731328472084456653639812530140212355374618917309552824925858430886313795805601/30",
    "10259718682038021051027794238379184461025738652460569233992776489750881337506863808448685054322627708245455888249006715516690124228801409697850408284121/138",
    "-81718086083262628510756459753673452313595710396116467582152090596092548699138346942995509488284650803976836337164670494733866559829768848363506624334818961419869/1794590070",
    "171672676901153210072183083506103395137513922274029564150500135265308148197358551999205867870374013289728260984269623579880772408522396975250682773558018919/6",
    "-4240860794203310376065563492361156949989398087086373214710625778458441940477839981850928830420029285687066701804645453159767402961229305942765784122421197736180867/230010",
    "1584451495144416428390934243279426140836596476080786316960222380784239380974799880364363647978168634590418215854419793716549388865905348534375629928732008786233507729/130074",
    "-20538064609143216265571979586692646837805331023148645068133372383930344948316600591203926388540940814833173322793804325084945094828524860626092013547281335356200073083/2490",
    "5734032969370860921631095311392645731505222358555208498573088911303001784652122964703205752709194193095246308611264121678834250704468082648313788124754168671815815821441/1002",
    "-13844828515176396081238346585063517228531109156984345249260453934317772754836791258987516540324983611569758649525983347408589045734176589270143058509026392246407576578281097477/3404310",
    "195334207626637530414976779238462234481410337350988427215139995707346979124686918267688171536352650572535330369818176979951931477427594872783018749894699157917782460035894085/66",
    "-11443702211333328447187179942991846613008046506032421731755258148665287832264931024781365962633301701773088470841621804328201008020129996955549467573217659587609679405537739509973/5190",
    "4166161554662042831884959593250717297395614318182561412048180684077407803317591270831194619293832107482426945655143357909807251852859279483176373435697607639883085093246499347128331/2478",
    "-1369347910486705707645621362512824332220360774476594348356938715366608044588614657557436131706543948464159947970464346070253278291989696390096800799614617317655510118710460076077638883999/1043970",
    "1124251816617941290026484851206299982774720467712867275292043701618829826708395745459654170718363182143418314514085426692857018428614935412736063946853033094328968069656979232446257101741/1074",
    "-6173136454016248924640522272263470960199559328290655337530202055853397791747341312347030141906500993752700612233695954532816018207721731818225290076670213481102834647254685911917265818955932383093313/7225713885390",
    "4277269279349192541137304400628629348327468135828402291661683018622451659989595510712915810436238721139546963558655260384328988773219688091443529626531335687951612545946030357929306651006711/6",
    "-857321333523056180131194437347933216431403305730705359015465649285681432317514010686029079324479659634642384809061711319481020030715989009140595170556956196762318625529645723516532076273012244047/1410",
    "22258646098436968050639602221816385181596567918515338169946670500599612225742487595012775838387331550474751212260636163500086787417640903770807353228157478339547041472679880890292167353534100797481/42",
    "-14158277750623758793309386870401397333112823632717478051426522029712001260747920789473711562165031101665618225654329210473605281619696918061316240634857984019071572591940586875558943580878119388321001/30",
    "5411555842544259796131885546196787277987837486638756184149141588783989774511509608733429067517383750706299486822702171672522203106730993581242777825864203487238429479957280273093904025319950569633979493395/12606",
    "-346465752997582699690191405750952366871923192340955593486485715370392154894102000406980162521728492501917598012711402163530166516991115122131398542029056286959857727373568402417020319761912636411646719477318166587/868841610",
    "2269186825161532962833665086968359967389321429297588337232986752409765414223476696863199759981611817660735753831323900456495253961837175924312108872915089534970310604331636484174526399721365966337809334021247/6",
    "-62753135110461193672553106699893713603153054153311895305590639107017824640241378480484625554578576142115835788960865534532214560982925549798683762705231316611716668749347221458005671217067357943416524984438771831113/171390",
    "88527914861348004968400581010530565220544526400339548429439843908721196349579494069282285662653465989920237253162555666526385826449862863083834096823053048072002986184254693991336699593468906111158296442729034119206322233/244713882",
    "-498384049428333414764928632140399662108495887457206674968055822617263669621523687568865802302210999132601412697613279391058654527145340515840099290478026350382802884371712359337984274122861159800280019110197888555893671151/1366530", // 200
};

static decNumber **sBernoulliNumbers = NULL;
static uint16_t sBernoulliNumberCount = 0;

static void createBernoulliNumbers(void)
{
    uint32_t lsuSizeSave = sLsuSize;
    sLsuSize = 250;
    sBernoulliNumberCount = dcTaffy_countOf(sBernoulliNumberStrings);
    sBernoulliNumbers =
        (decNumber **)(dcMemory_allocate
                       (sizeof(decNumber *) * sBernoulliNumberCount));
    uint16_t i;

    for (i = 0; i < sBernoulliNumberCount; i++)
    {
        dcList *numbers = dcLexer_splitString(sBernoulliNumberStrings[i], '/');
        dcError_assert(numbers->size == 2);
        decNumber *left =
            dcDecNumber_createFromString
            (dcString_getString(dcList_getHead(numbers)));
        decNumber *right =
            dcDecNumber_createFromString
            (dcString_getString(dcList_getTail(numbers)));

        sBernoulliNumbers[i] = dcDecNumber_create();
        dcDecNumber_divide(sBernoulliNumbers[i], left, right);
        dcList_free(&numbers, DC_DEEP);
        dcDecNumber_free(&left);
        dcDecNumber_free(&right);
    }

    sLsuSize = lsuSizeSave;
}

static void freeBernoulliNumbers(void)
{
    uint16_t i;

    for (i = 0; i < dcTaffy_countOf(sBernoulliNumberStrings); i++)
    {
        dcDecNumber_free(&sBernoulliNumbers[i]);
    }

    dcMemory_free(sBernoulliNumbers);
}

static bool initialized = false;

void dcDecNumber_initialize(void)
{
    if (! initialized)
    {
        initialized = true;
        uint32_t i;
        sFactorials = (decNumber **)(dcMemory_allocate
                                     (sizeof(decNumber *) * sFactorialDepth));
        uint32_t lsuSizeSave = sLsuSize;

        // overkill, but big enough, we can probably shrink
        sLsuSize = 512;

        for (i = 0; i < sFactorialDepth; i++)
        {
            factorialize(&sFactorials[i], i);
        }

        sLsuSize = lsuSizeSave;
        createBernoulliNumbers();
        sZero = dcDecNumber_createFromInt32u(0);
        sOne = dcDecNumber_createFromInt32u(1);
        sTwo = dcDecNumber_createFromInt32u(2);
        sNegativeOne = dcDecNumber_createFromInt32s(-1);
        uint32_t deltaCount = 20;
        sDeltas = (decNumber **)dcMemory_allocate(sizeof(decNumber *)
                                                  * deltaCount);

        sPi = dcDecNumber_createFromInt32u(0);
        dcDecNumber_pi(sPi, sOne);

        sTwoPi = dcDecNumber_createFromInt32u(0);
        dcDecNumber_pi(sTwoPi, sOne);
        dcDecNumber_multiply(sTwoPi, sTwoPi, sTwo);

        sHalfPi = dcDecNumber_createFromInt32u(0);
        dcDecNumber_pi(sHalfPi, sOne);
        decNumber *oneHalf = dcDecNumber_createFromString("0.5");
        dcDecNumber_multiply(sHalfPi, sHalfPi, oneHalf);
        dcDecNumber_free(&oneHalf);

        sNegativeHalfPi = dcDecNumber_createFromInt32u(0);
        dcDecNumber_pi(sNegativeHalfPi, sOne);
        decNumber *negativeOneHalf = dcDecNumber_createFromString("-0.5");
        dcDecNumber_multiply(sNegativeHalfPi, sNegativeHalfPi, negativeOneHalf);
        dcDecNumber_free(&negativeOneHalf);

        for (i = 0; i < deltaCount; i++)
        {
            sDeltas[i] = getDelta(i, NULL);
        }

        sDeltaCount = deltaCount;
    }
}

void dcDecNumber_deinitialize(void)
{
    if (initialized)
    {
        uint32_t i;

        for (i = 0; i < sFactorialDepth; i++)
        {
            dcDecNumber_free(&sFactorials[i]);
        }

        dcMemory_free(sFactorials);

        for (i = 0; i < sDeltaCount; i++)
        {
            dcDecNumber_free(&sDeltas[i]);
        }

        dcMemory_free(sDeltas);
        dcDecNumber_free(&sZero);
        dcDecNumber_free(&sOne);
        dcDecNumber_free(&sTwo);
        dcDecNumber_free(&sNegativeOne);
        dcDecNumber_free(&sPi);
        dcDecNumber_free(&sTwoPi);
        dcDecNumber_free(&sHalfPi);
        dcDecNumber_free(&sNegativeHalfPi);
        freeBernoulliNumbers();
    }
}

bool dcDecNumber_deltaEqual(const decNumber *_left,
                              const decNumber *_right,
                              uint32_t _delta)
{
    bool freeIt = false;
    decNumber *delta = getDelta(_delta, &freeIt);
    decNumber *top = dcDecNumber_copy(_right);
    decNumber *bottom = dcDecNumber_copy(_right);
    dcDecNumber_add(top, top, delta);
    dcDecNumber_subtract(bottom, bottom, delta);
    dcTaffyOperator topCompare = dcDecNumber_easyCompare(_left, top);
    dcTaffyOperator bottomCompare = dcDecNumber_easyCompare(_left, bottom);

    bool result = ((topCompare == TAFFY_EQUALS
                    || topCompare == TAFFY_LESS_THAN)
                   && (bottomCompare == TAFFY_EQUALS
                       || bottomCompare == TAFFY_GREATER_THAN));

    if (freeIt)
    {
        dcDecNumber_free(&delta);
    }

    dcDecNumber_free(&top);
    dcDecNumber_free(&bottom);

    return result;
}

dcNumberResult dcDecNumber_absoluteValue(decNumber *_result,
                                         const decNumber *_value)
{
    copyMerge(_result, _value);
    _result->bits &= ~DECNEG;
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_lg(decNumber *_result, const decNumber *_value)
{
    return TAFFY_NUMBER_NOT_IMPLEMENTED;
}

//
// Chudnovsky formula!
//
// 1/PI =
//
//    termA
// (12 / 640320^(3/2)) * [SUM(k = 0, inf) = (6k)! * (13591409 + 545140134k)
//                                          -------------------------------
//                                          (3k)! * (k!)^3 * (-640320)^(3k)]
//
// This is a complicated expression, so we use Taffy to create a graph tree
// for it
//
dcNumberResult dcDecNumber_pi(decNumber *_result,
                              const decNumber *_value)
{
    // do this for now...
    decNumber *result =
        dcDecNumber_createFromString("3.14159265358979323846264338327");
    moveOver(_result, result);
    return TAFFY_NUMBER_SUCCESS;
}

bool dcDecNumber_convertToDouble(const decNumber *_value, double *_output)
{
    int32_t i = 0;
    double tensValue = 0.1;
    int32_t exponentRemaining = -_value->exponent;
    int32_t zeroBucket = (-_value->exponent) / DECDPUN;
    *_output = 0;
    int32_t exponent = _value->exponent;

    if (exponent > 20)
    {
        exponent = 20;
    }
    else if (exponent < -20)
    {
        exponent = -20;
    }

    if (exponent < 0)
    {
        for (i = (-exponent - 1) / DECDPUN; i >= 0; i--)
        {
            dcError_assert(exponentRemaining >= 0);

            if (exponentRemaining % 2 != 0)
            {
                *_output += TENS_DIGIT(_value->lsu[i]) * tensValue;
                exponentRemaining -= 1;
                tensValue *= 0.1;
            }
            else
            {
                *_output += HUNDREDS_DIGIT(_value->lsu[i]) * tensValue;
                *_output += (TENS_DIGIT(_value->lsu[i])
                             * tensValue
                             * 0.1);
                exponentRemaining -= 2;
                tensValue *= 0.01;
            }
        }
    }

    tensValue = 1;
    bool start = true;

    for (i = zeroBucket; i < _value->digits; i++)
    {
        if (start
            && exponent < 0
            && (-exponent) % 2 != 0)
        {
            start = false;
            *_output += HUNDREDS_DIGIT(_value->lsu[i]) * tensValue;
            tensValue *= 10;
        }
        else
        {
            *_output += TENS_DIGIT(_value->lsu[i]) * tensValue;
            tensValue *= 10;

            if (i < _value->digits - 1)
            {
                *_output += HUNDREDS_DIGIT(_value->lsu[i]) * tensValue;
                tensValue *= 10;
            }
        }
    }

    if ((_value->bits & DECNEG) != 0)
    {
        *_output *= -1;
    }

    return true;
}

bool dcDecNumber_convertToUInt32(const decNumber *_value, uint32_t *_integer)
{
    EASY_CONTEXT_CREATOR(context, _value);
    *_integer = decNumberToUInt32(_value, &context);
    return (context.status == 0);
}

bool dcDecNumber_convertToInt32(const decNumber *_value, int32_t *_integer)
{
    EASY_CONTEXT_CREATOR(context, _value);
    *_integer = decNumberToInt32(_value, &context);
    return (context.status == 0);
}

void dcDecNumber_convertToDecimal(const uint8_t *_binary,
                                  uint32_t _binaryLength,
                                  decNumber *_result)
{
    uint32_t i;
    decNumber *power = dcDecNumber_createFromInt32u(1);

    for (i = 0; i < _binaryLength; i++)
    {
        if (_binary[i] == 1)
        {
            dcDecNumber_add(_result, _result, power);
        }

        dcDecNumber_multiply(power, power, sTwo);
    }

    dcDecNumber_free(&power);
}

bool dcDecNumber_convertToBinary(const decNumber *_value,
                                 uint8_t **_result,
                                 uint32_t *_resultLength)
{
    if (! dcDecNumber_isWhole(_value))
    {
        return false;
    }

    bool result = true;

    if (decNumberIsZero(_value))
    {
        *_resultLength = 1;
        *_result = (uint8_t *)dcMemory_allocateAndInitialize(1);
    }
    else if (! decNumberIsNegative(_value))
    {
        dcString *string = dcString_create();
        uint32_t i;
        decNumber *copy = dcDecNumber_copy(_value);

        for (i = 0; ! decNumberIsZero(copy); i++)
        {
            dcDecNumber_divide(copy, copy, sTwo);

            if (dcDecNumber_isWhole(copy))
            {
                dcString_appendCharacter(string, 0);
            }
            else
            {
                dcString_appendCharacter(string, 1);
                dcDecNumber_floor(copy, copy);
            }
        }

        if (string->index > UINT32_MAX)
        {
            result = false;
        }
        else
        {
            *_result = (uint8_t *)string->string;
            *_resultLength = (uint32_t)string->index;
        }

        dcString_free(&string, DC_SHALLOW);
        dcDecNumber_free(&copy);
    }

    return result;
}

dcNumberResult dcDecNumber_increment(decNumber *_value)
{
    dcDecNumber_add(_value, _value, sOne);
    return TAFFY_NUMBER_SUCCESS;
}

dcNumberResult dcDecNumber_decrement(decNumber *_value)
{
    dcDecNumber_subtract(_value, _value, sOne);
    return TAFFY_NUMBER_SUCCESS;
}

bool dcDecNumber_resize(decNumber *_number,
                        uint32_t _needBytes,
                        uint32_t *_status)
{
    bool result = true;

    if (_needBytes > (size_t)_number->lsuSize)
    {
        _number->lsu = (uint8_t *)dcMemory_realloc(_number->lsu, _needBytes);

        if (_number->lsu == NULL)
        {
            result = false;
            *_status |= DEC_Insufficient_storage;
        }
        else
        {
            // memset the rest to 0
            memset(_number->lsu + _number->digits,
                   0,
                   _needBytes - _number->digits);
            //_number->digits = _needBytes;
            _number->lsuSize = _needBytes;
        }
    }

    return result;
}

decNumber *dcDecNumber_round(decNumber *_number)
{
    EASY_CONTEXT_CREATOR(set, _number);

    if (_number->exponent < 0
        && _number->digits >= DECDPUN
        && _number->exponent <= -(_number->digits - DECDPUN) * DECDPUN)
    {
        // it's zero
        dcNumber_inlineCopyDecNumberToDecNumber(_number, sZero);
        return _number;
    }

    _number = decNumberReduce(_number, _number, &set);

    if (_number->exponent < -DECDPUN)
    {
        dcNumber *right = (dcNumber_createFromDouble
                           (_number->exponent + DECDPUN));
        dcNumber_floor(right, right);
        _number = decNumberRescale(_number,
                                   _number,
                                   right->types.decNumber,
                                   &set);
        dcNumber_free(&right, DC_DEEP);
    }

    if (dcDecNumber_isWhole(_number))
    {
        dcDecNumber_chomp(_number, _number);
    }

    return _number;
}
