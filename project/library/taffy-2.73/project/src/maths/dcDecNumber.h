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

#ifndef __DC_DEC_NUMBER_H__
#define __DC_DEC_NUMBER_H__

#include "dcDefines.h"
#include "decNumber.h"

void dcDecNumber_initialize(void);
void dcDecNumber_deinitialize(void);

/**
 * @brief Create with default digit limit.
 */
decNumber *dcDecNumber_create(void);

decNumber *dcDecNumber_createWithLsuSize(uint32_t _lsuSize);
decNumber *dcDecNumber_createFromInt32u(uint32_t _value);
decNumber *dcDecNumber_createFromInt32s(int32_t _value);
decNumber *dcDecNumber_createFromInt64u(uint64_t _value);
decNumber *dcDecNumber_createFromSizet(size_t _value);
decNumber *dcDecNumber_createFromDouble(double _value);
decNumber *dcDecNumber_createFromString(const char *_string);
void dcDecNumber_free(decNumber **_number);
decNumber *dcDecNumber_copy(const decNumber *_number);
bool dcDecNumber_resize(decNumber *_number,
                        uint32_t _needBytes,
                        uint32_t *_status);
void dcDecNumber_safeResize(decNumber *_number, size_t _needBytes);
char *dcDecNumber_display(const decNumber *_number);
char *dcDecNumber_displayWithGarbageCollection(const decNumber *_number);
char *dcDecNumber_displayBytes(const decNumber *_number);
dcResult dcDecNumber_compare(const decNumber *_left,
                             const decNumber *_right,
                             dcTaffyOperator *_compareResult);
dcTaffyOperator dcDecNumber_easyCompare(const decNumber *_left,
                                        const decNumber *_right);
dcTaffyOperator dcDecNumber_easyCompareToInt32u(const decNumber *_left,
                                                uint32_t _right);
dcTaffyOperator dcDecNumber_easyCompareToInt32s(const decNumber *_left,
                                                int32_t _right);

typedef dcNumberResult (*dcDecNumber_arithmeticFunction)
    (decNumber *_result,
     const decNumber *_left,
     const decNumber *_right);

#define DEC_NUMBER_ARITHMETIC_FUNCTION(_name)       \
    dcNumberResult _name(decNumber *_result,        \
                         const decNumber *_left,    \
                         const decNumber *_right)

DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_add);
DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_subtract);
DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_multiply);
DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_divide);
DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_raise);
DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_and);
DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_or);
DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_xor);
DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_leftShift);
DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_rightShift);
DEC_NUMBER_ARITHMETIC_FUNCTION(dcDecNumber_choose);

decNumber *dcDecNumber_round(decNumber *_number);

bool dcDecNumber_isWhole(const decNumber *_number);

decNumber *dcDecNumber_negate(decNumber *_number);

typedef dcNumberResult (*dcDecNumber_singleOperandArithmeticFunction)
    (decNumber *_result, const decNumber *_value);

dcNumberResult dcDecNumber_modulus(decNumber *_result,
                                   const decNumber *_left,
                                   const decNumber *_right);

dcNumberResult dcDecNumber_absoluteValue(decNumber *_result,
                                         const decNumber *_value);

#define SINGLE_OPERAND_FUNCTION(_function)                              \
    dcNumberResult _function(decNumber *_result, const decNumber *_value);

SINGLE_OPERAND_FUNCTION(dcDecNumber_bitNot);
SINGLE_OPERAND_FUNCTION(dcDecNumber_chomp);
SINGLE_OPERAND_FUNCTION(dcDecNumber_ln);
SINGLE_OPERAND_FUNCTION(dcDecNumber_log10);
SINGLE_OPERAND_FUNCTION(dcDecNumber_lg);
SINGLE_OPERAND_FUNCTION(dcDecNumber_e);
SINGLE_OPERAND_FUNCTION(dcDecNumber_pi);
SINGLE_OPERAND_FUNCTION(dcDecNumber_floor);
SINGLE_OPERAND_FUNCTION(dcDecNumber_ceiling);
SINGLE_OPERAND_FUNCTION(dcDecNumber_factorial);
SINGLE_OPERAND_FUNCTION(dcDecNumber_squareRoot);

dcNumberResult dcDecNumber_increment(decNumber *_value);
dcNumberResult dcDecNumber_decrement(decNumber *_value);

bool dcDecNumber_convertToUInt32(const decNumber *_value, uint32_t *_integer);
bool dcDecNumber_convertToInt32(const decNumber *_value, int32_t *_integer);
bool dcDecNumber_convertToDouble(const decNumber *_value, double *_output);
bool dcDecNumber_convertToBinary(const decNumber *_value,
                                 uint8_t **_result,
                                 uint32_t *_resultLength);
void dcDecNumber_convertToDecimal(const uint8_t *_binary,
                                  uint32_t _binaryLength,
                                  decNumber *_result);

uint32_t dcDecNumber_getLsuSize(void);
void dcDecNumber_setLsuSize(uint32_t _precision);

bool dcDecNumber_deltaEqual(const decNumber *_left,
                              const decNumber *_right,
                              uint32_t _delta);

bool dcDecNumber_isPositive(const decNumber *_value);
bool dcDecNumber_isExact(const decNumber *_value);

#define DEC_NUMBER_CONSTANT_COUNT 2000

#endif
