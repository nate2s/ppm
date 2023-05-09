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

#ifndef __DC_NUMBER_H__
#define __DC_NUMBER_H__

#include "dcDefines.h"

typedef enum
{
    NUMBER_INTEGER_TYPE    = 0,
    NUMBER_DEC_NUMBER_TYPE = 1
} dcNumberType_e;

typedef uint8_t dcNumberType;

struct dcNumber_t
{
    union dcNumberTypes_t
    {
        struct decNumber_t *decNumber;
        int32_t integer;
    } types;

    dcNumberType type;
};

typedef struct dcNumber_t dcNumber;

void dcNumber_initialize(void);
void dcNumber_deinitialize(void);

// misc funcs //
bool dcNumber_verifyType(struct dcNode_t *_preNumber, dcNumberType _type);

// creating //
dcNumber *dcNumber_createFromInt32u(uint32_t _value);
dcNumber *dcNumber_createFromInt32s(int32_t _value);
dcNumber *dcNumber_createFromInt64u(uint64_t _value);
dcNumber *dcNumber_createFromSizet(size_t _value);
dcNumber *dcNumber_createFromDouble(double _value);
dcNumber *dcNumber_createFromDecNumber(struct decNumber_t *_value);
dcNumber *dcNumber_createFromString(const char *_string);
dcNumber *dcNumber_createWithLsuSize(uint32_t _size);
dcNumber *dcNumber_createFromInt32uWithLsuSize(uint32_t _value, uint32_t _size);

struct dcNode_t *dcNumber_createNode(dcNumberType _type);
struct dcNode_t *dcNumber_createNodeFromInt32u(uint32_t _value);
struct dcNode_t *dcNumber_createNodeFromInt32s(int32_t _value);
struct dcNode_t *dcNumber_createNodeFromInt64u(uint64_t _value);
struct dcNode_t *dcNumber_createNodeFromSizet(size_t _value);
struct dcNode_t *dcNumber_createNodeFromDouble(int32_t _value);
struct dcNode_t *dcNumber_createNodeFromDecNumber(struct decNumber_t *_value);

struct dcNode_t *dcNumber_createShell(dcNumber *_number);

// freeing //
void dcNumber_free(dcNumber **_number, dcDepth _depth);

// copying //
dcNumber *dcNumber_copy(const dcNumber *_from, dcDepth _depth);

// displaying //
char *dcNumber_display(const dcNumber *_number);

// querying //
bool dcNumber_equals(const dcNumber *_left, const dcNumber *_right);
bool dcNumber_equalsInt32s(const dcNumber *_preNumber, int32_t _number);
bool dcNumber_equalsInt32u(const dcNumber *_preNumber, uint32_t _number);

bool dcNumber_deltaEqual(const dcNumber *_left,
                         const dcNumber *_right,
                         uint32_t _delta);

// converting between types //
bool dcNumber_extractDouble(const dcNumber *_template, double *_convertedValue);
bool dcNumber_extractUInt8(const dcNumber *_template, uint8_t *_value);
bool dcNumber_extractUInt16(const dcNumber *_template, uint16_t *_value);
bool dcNumber_extractUInt32(const dcNumber *_template,
                            uint32_t *_convertedValue);
bool dcNumber_extractUInt64(const dcNumber *_template,
                            uint64_t *_convertedValue);
bool dcNumber_extractInt32(const dcNumber *_template, int32_t *_convertedValue);
bool dcNumber_extractInt64(const dcNumber *_template, int64_t *_value);
bool dcNumber_tryToConvertToInteger(dcNumber *_result);

bool dcNumber_isExact(const dcNumber *_number);
bool dcNumber_isWhole(const dcNumber *_number);

// marshalling //
struct dcString_t *dcNumber_marshall(const dcNumber *_number,
                                     struct dcString_t *_stream);
dcNumber *dcNumber_unmarshall(struct dcString_t *_stream);

// arithmetic //

#define ARITHMETIC_FUNCTION(__f__)                                      \
    dcNumberResult __f__(dcNumber *_result,                             \
                         const dcNumber *_left,                         \
                         const dcNumber *_right)

ARITHMETIC_FUNCTION(dcNumber_add);
ARITHMETIC_FUNCTION(dcNumber_bitAnd);
ARITHMETIC_FUNCTION(dcNumber_bitOr);
ARITHMETIC_FUNCTION(dcNumber_bitXOr);
ARITHMETIC_FUNCTION(dcNumber_divide);
ARITHMETIC_FUNCTION(dcNumber_leftShift);
ARITHMETIC_FUNCTION(dcNumber_modulus);
ARITHMETIC_FUNCTION(dcNumber_multiply);
ARITHMETIC_FUNCTION(dcNumber_raise);
ARITHMETIC_FUNCTION(dcNumber_rightShift);
ARITHMETIC_FUNCTION(dcNumber_subtract);
ARITHMETIC_FUNCTION(dcNumber_choose);
ARITHMETIC_FUNCTION(dcNumber_gcd);

void dcNumber_round(dcNumber *_number);

void dcNumber_setInt32sValue(dcNumber *_number, int32_t _value);
void dcNumber_setDoubleValue(dcNumber *_number, double _value);

dcNumberResult dcNumber_absoluteValue(dcNumber *_result,
                                      const dcNumber *_number);
dcNumberResult dcNumber_ceiling(dcNumber *_result, const dcNumber *_number);
dcNumberResult dcNumber_chomp(dcNumber *_result, const dcNumber *_number);
dcNumberResult dcNumber_floor(dcNumber *_result, const dcNumber *_number);
bool dcNumber_lessThan(const dcNumber *_left, const dcNumber *_right);
bool dcNumber_greaterThan(const dcNumber *_left, const dcNumber *_right);
bool dcNumber_greaterThanOrEqual(const dcNumber *_left,
                                 const dcNumber *_right);
bool dcNumber_lessThanOrEqual(const dcNumber *_left, const dcNumber *_right);
bool dcNumber_isPositive(const dcNumber *_number);
bool dcNumber_isNonNegative(const dcNumber *_number);

dcResult dcNumber_hash(const dcNumber *_number, dcHashType *_hashResult);

dcNumberResult dcNumber_increment(dcNumber *_number);
dcNumberResult dcNumber_decrement(dcNumber *_number);

#define SINGLE_OPERAND_OPERATION(__f__)             \
    dcNumberResult __f__(dcNumber *_result, const dcNumber *_op)

SINGLE_OPERAND_OPERATION(dcNumber_bitNot);
SINGLE_OPERAND_OPERATION(dcNumber_squareRoot);
SINGLE_OPERAND_OPERATION(dcNumber_ln);
SINGLE_OPERAND_OPERATION(dcNumber_lg);
SINGLE_OPERAND_OPERATION(dcNumber_log10);
SINGLE_OPERAND_OPERATION(dcNumber_factorial);

// random //
dcNumberResult dcNumber_random(dcNumber *_result);

typedef bool (*dcNumber_comparisonFunction)(const dcNumber *_left,
                                            const dcNumber *_right);

typedef dcNumberResult (*dcNumber_arithmeticFunction)(dcNumber *_result,
                                                      const dcNumber *_left,
                                                      const dcNumber *_right);

typedef dcNumberResult (*dcNumber_singleOperandFunction)
    (dcNumber *_result,
     const dcNumber *_number);

typedef bool (*dcNumber_deltaEqualFunction)(const dcNumber *_left,
                                            const dcNumber *_right,
                                            uint32_t _delta);

void dcNumber_inlineCopy(dcNumber *_to, const dcNumber *_from);
void dcNumber_inlineCopyDecNumberToDecNumber(struct decNumber_t *_to,
                                             const struct decNumber_t *_from);
bool dcNumber_isNaN(const dcNumber *_number);

dcNumber *dcNumber_getNaN(void);
dcNumber *dcNumber_getNegativeOne(void);

typedef dcNumberResult (*dcNumber_operation)
    (dcNumber *_result, const dcNumber *_left, const dcNumber *_right);

dcNumber_operation dcNumber_getOperation(dcTaffyOperator _operator);

const dcNumber *dcNumber_getConstant(uint8_t _constant);
const dcNumber *dcNumber_getOneHalf(void);
const dcNumber *dcNumber_getNegativeOneHalf(void);

uint32_t dcNumber_getDigitLimit(const dcNumber *_number);

int32_t dcNumber_getExponent(const dcNumber *_number);

dcNumber *dcNumber_castMe(struct dcNode_t *_node);
dcNumber *dcNumber_snip(dcNumber *_result);

struct dcList_t *dcNumber_getFactorPairs(const dcNumber *_number,
                                         bool _wantObjects);

struct dcList_t *dcNumber_getFactorPairsWithLimit(const dcNumber *_number,
                                                  uint32_t _maxIterations,
                                                  bool _wantObjects);

struct dcList_t *dcNumber_getFactors(const dcNumber *_number,
                                     bool _wantObjects);

struct dcList_t *dcNumber_getFactorsWithLimit(const dcNumber *_number,
                                              uint32_t _maxIterations,
                                              bool _wantObjects);

// standard functions //
COPY_FUNCTION(dcNumber_copyNode);
FREE_FUNCTION(dcNumber_freeNode);

#define DEFAULT_FACTOR_LIMIT 1000

#endif
