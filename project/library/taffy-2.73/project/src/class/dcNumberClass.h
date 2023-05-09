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

#ifndef __DC_NUMBER_CLASS_H__
#define __DC_NUMBER_CLASS_H__

#include <stdio.h>

#include "dcDefines.h"

//////////////////////////
// dcNumberMetaClassAux //
//////////////////////////

struct dcNumberMetaClassAux_t
{
    uint32_t defaultDelta;
    struct dcNode_t *defaultDeltaNode;
    uint32_t defaultDigitLimit;

    // this points to the top-most storage value, a helper/speedup variable
    uint32_t digitLimit;

    struct dcList_t *digitLimitStack;
    struct dcList_t *fractionalOutputLengthStack;

    struct dcNode_t *oneNumberObject;
    struct dcNode_t *negativeOneNumberObject;
    struct dcNode_t *zeroNumberObject;

    struct dcMutex_t *digitLimitMutex;
};

typedef struct dcNumberMetaClassAux_t dcNumberMetaClassAux;

//////////////////////
// dcNumberClassAux //
//////////////////////

struct dcNumberClassAux_t
{
    struct dcNumber_t *number;
};

typedef struct dcNumberClassAux_t dcNumberClassAux;

// misc funcs //
bool dcNumberClass_verifyType(struct dcNode_t *_preNumber, uint16_t _type);

// creating //
struct dcNode_t *dcNumberClass_createNode(dcNumberClassAux *_aux,
                                          bool _object);

struct dcNode_t *dcNumberClass_createObject(struct dcNumber_t *_number);
struct dcNode_t *dcNumberClass_createObjectFromInt32s(int32_t _value);
struct dcNode_t *dcNumberClass_createObjectFromInt32u(uint32_t _value);
struct dcNode_t *dcNumberClass_createObjectFromInt64u(uint64_t _value);
struct dcNode_t *dcNumberClass_createObjectFromSizet(size_t _value);
struct dcNode_t *dcNumberClass_createObjectFromDouble(double _value);
struct dcNode_t *dcNumberClass_createObjectFromString(const char *_string);

struct dcNode_t *dcNumberClass_negateHelper(struct dcNode_t *_number);

// getting //
uint32_t dcNumberMetaClass_getDigitLimit(void);
struct dcNumber_t *dcNumberClass_getNumber(const struct dcNode_t *_node);
bool dcNumberClass_equalsInt32u(const struct dcNode_t *_object,
                                uint32_t _number);
bool dcNumberClass_equalsInt32s(const struct dcNode_t *_object,
                                int32_t _number);
bool dcNumberClass_isNegative(const struct dcNode_t *_number);
bool dcNumberClass_isPositive(const struct dcNode_t *_number);

struct dcNode_t *dcNumberClass_getZeroNumberObject(void);
struct dcNode_t *dcNumberClass_getOneNumberObject(void);
struct dcNode_t *dcNumberClass_getNegativeOneNumberObject(void);

bool dcNumberClass_isZero(struct dcNode_t *_value);
bool dcNumberClass_isOne(struct dcNode_t *_value);
bool dcNumberClass_isNegativeOne(struct dcNode_t *_value);
bool dcNumberClass_isEven(struct dcNode_t *_value);
bool dcNumberClass_isWholeHelper(struct dcNode_t *_value);

struct dcNode_t *dcNumberClass_convertToInteger(struct dcNode_t *_value);

// converting between types //
bool dcNumberClass_extractDouble(const struct dcNode_t *_preNumber,
                                 double *_convertedValue);

bool dcNumberClass_extractDouble_withException
    (struct dcNode_t *_preNumber, double *_convertedValue);

bool dcNumberClass_extractInt32s(const struct dcNode_t *_preNumber,
                                 int32_t *_convertedValue);

bool dcNumberClass_extractInt32u(const struct dcNode_t *_preNumber,
                                 uint32_t *_convertedValue);

bool dcNumberClass_extractInt16u(const struct dcNode_t *_preNumber,
                                 uint16_t *_convertedValue);

bool dcNumberClass_extractInt16s(const struct dcNode_t *_preNumber,
                                 int16_t *_convertedValue);

bool dcNumberClass_extractInt32u_withException(struct dcNode_t *_preNumber,
                                               uint32_t *_convertedValue);
bool dcNumberClass_extractInt32s_withException(struct dcNode_t *_preNumber,
                                               int32_t *_convertedValue);

bool dcNumberClass_extractInt64u_withException(struct dcNode_t *_preNumber,
                                               uint64_t *_convertedValue);
bool dcNumberClass_extractInt64s_withException(struct dcNode_t *_preNumber,
                                               int64_t *_convertedValue);

bool dcNumberClass_extractInt8u_withException(struct dcNode_t *_preNumber,
                                              uint8_t *_convertedValue);

bool dcNumberClass_extractInt16s_withException(struct dcNode_t *_preNumber,
                                               int16_t *_convertedValue);

bool dcNumberClass_extractInt16u_withException(struct dcNode_t *_preNumber,
                                               uint16_t *_convertedValue);

bool dcNumberClass_extractIndex(struct dcNode_t *_preNumber,
                                uint32_t *_extractedValue,
                                uint32_t _limit);

bool dcNumberClass_extract64BitIndex(struct dcNode_t *_preNumber,
                                     uint64_t *_extractedValue,
                                     uint64_t _limit);

// helpers //
bool dcNumberClass_isMe(const struct dcNode_t *_node);
void dcNumberClass_increment(struct dcNode_t *_node);
void dcNumberClass_decrement(struct dcNode_t *_node);

struct dcNode_t *dcNumberClass_addHelper(struct dcNode_t *_left,
                                         struct dcNode_t *_right);

struct dcNode_t *dcNumberClass_inlineAdd(struct dcNode_t *_left,
                                         struct dcNode_t *_right);

struct dcNode_t *dcNumberClass_inlineSubtract(struct dcNode_t *_left,
                                              struct dcNode_t *_right);

struct dcNode_t *dcNumberClass_inlineMultiply(struct dcNode_t *_left,
                                              struct dcNode_t *_right);

struct dcNode_t *dcNumberClass_inlineDivide(struct dcNode_t *_left,
                                            struct dcNode_t *_right);

struct dcNode_t *dcNumberClass_inlineRaise(struct dcNode_t *_left,
                                           struct dcNode_t *_right);

struct dcNode_t *dcNumberClass_divideHelper(struct dcNode_t *_left,
                                            struct dcNode_t *_right);

struct dcNode_t *dcNumberClass_multiplyHelper(struct dcNode_t *_left,
                                              struct dcNode_t *_right);

struct dcNode_t *dcNumberClass_subtractHelper(struct dcNode_t *_left,
                                              struct dcNode_t *_right);

struct dcNode_t *dcNumberMetaClass_getDefaultDelta(void);

bool dcNumberClass_equalsNumber(struct dcNode_t *_left,
                                struct dcNode_t *_right);

bool dcNumberClass_divides(struct dcNode_t *_left,
                           struct dcNode_t *_right);

bool dcNumberClass_lessThanHelper(struct dcNode_t *_left,
                                  struct dcNode_t *_right);

bool dcNumberClass_greaterThanHelper(struct dcNode_t *_left,
                                     struct dcNode_t *_right);

void dcNumberMetaClass_pushDigitLimitHelper(struct dcNode_t *_digitLimitNode,
                                            uint32_t _digitLimit);
void dcNumberMetaClass_popDigitLimitHelper(void);

dcHashType dcNumberClass_hashHelper(struct dcNode_t *_receiver);

bool dcNumberClass_verifyInteger(struct dcNode_t *_node);

struct dcNode_t *dcNumberClass_chompHelper(struct dcNode_t *_receiver);

bool dcNumberClass_isOneFractional(struct dcNode_t *_value);

// standard functions //
ALLOCATE_FUNCTION(dcNumberClass_allocateNode);
COPY_FUNCTION(dcNumberClass_copyNode);
DEINITIALIZE_FUNCTION(dcNumberClass_deinitialize);
FREE_FUNCTION(dcNumberClass_freeNode);
GET_TEMPLATE_FUNCTION(dcNumberClass_getTemplate);
INITIALIZE_FUNCTION(dcNumberClass_initialize);
MARK_FUNCTION(dcNumberMetaClass_markNode);
MARSHALL_FUNCTION(dcNumberClass_marshallNode);
UNMARSHALL_FUNCTION(dcNumberClass_unmarshallNode);

TAFFY_C_METHOD(dcNumberMetaClass_digitLimit);
TAFFY_C_METHOD(dcNumberMetaClass_popFractionalOutputLength);
TAFFY_C_METHOD(dcNumberMetaClass_popDigitLimit);
TAFFY_C_METHOD(dcNumberMetaClass_pushFractionalOutputLength);
TAFFY_C_METHOD(dcNumberMetaClass_pushDigitLimit);
TAFFY_C_METHOD(dcNumberMetaClass_setDefaultDigitLimit);
TAFFY_C_METHOD(dcNumberMetaClass_digitLimit);

TAFFY_C_METHOD(dcNumberClass_absoluteValue);
TAFFY_C_METHOD(dcNumberClass_add);
TAFFY_C_METHOD(dcNumberClass_addEquals);
TAFFY_C_METHOD(dcNumberClass_antiNegate);
TAFFY_C_METHOD(dcNumberClass_asString);
TAFFY_C_METHOD(dcNumberClass_bitAnd);
TAFFY_C_METHOD(dcNumberClass_bitAndEquals);
TAFFY_C_METHOD(dcNumberClass_bitNot);
TAFFY_C_METHOD(dcNumberClass_bitOr);
TAFFY_C_METHOD(dcNumberClass_bitOrEquals);
TAFFY_C_METHOD(dcNumberClass_bitXOr);
TAFFY_C_METHOD(dcNumberClass_bitXOrEquals);
TAFFY_C_METHOD(dcNumberClass_ceiling);
TAFFY_C_METHOD(dcNumberClass_chomp);
TAFFY_C_METHOD(dcNumberClass_deltaEquals);
TAFFY_C_METHOD(dcNumberClass_digitLimit);
TAFFY_C_METHOD(dcNumberClass_divide);
TAFFY_C_METHOD(dcNumberClass_divideEquals);
TAFFY_C_METHOD(dcNumberClass_downTo);
TAFFY_C_METHOD(dcNumberClass_factorial);
TAFFY_C_METHOD(dcNumberClass_factorPairs);
TAFFY_C_METHOD(dcNumberClass_factors);
TAFFY_C_METHOD(dcNumberClass_floor);
TAFFY_C_METHOD(dcNumberClass_getFractionalSize);
TAFFY_C_METHOD(dcNumberClass_greaterThan);
TAFFY_C_METHOD(dcNumberClass_greaterThanOrEqual);
TAFFY_C_METHOD(dcNumberClass_hash);
TAFFY_C_METHOD(dcNumberClass_isWhole);
TAFFY_C_METHOD(dcNumberClass_leftShift);
TAFFY_C_METHOD(dcNumberClass_leftShiftEquals);
TAFFY_C_METHOD(dcNumberClass_lessThan);
TAFFY_C_METHOD(dcNumberClass_lessThanOrEqual);
TAFFY_C_METHOD(dcNumberClass_minusMinus);
TAFFY_C_METHOD(dcNumberClass_modulus);
TAFFY_C_METHOD(dcNumberClass_modulusEquals);
TAFFY_C_METHOD(dcNumberClass_multiply);
TAFFY_C_METHOD(dcNumberClass_multiplyEquals);
TAFFY_C_METHOD(dcNumberClass_negate);
TAFFY_C_METHOD(dcNumberClass_plusPlus);
TAFFY_C_METHOD(dcNumberClass_raise);
TAFFY_C_METHOD(dcNumberClass_raiseEquals);
TAFFY_C_METHOD(dcNumberClass_rightShift);
TAFFY_C_METHOD(dcNumberClass_rightShiftEquals);
TAFFY_C_METHOD(dcNumberClass_subtract);
TAFFY_C_METHOD(dcNumberClass_subtractEquals);
TAFFY_C_METHOD(dcNumberClass_squareRoot);
TAFFY_C_METHOD(dcNumberClass_trulyEquals);
TAFFY_C_METHOD(dcNumberClass_upTo);

#define NUMBER_PACKAGE_NAME MATHS_PACKAGE_NAME
#define NUMBER_CLASS_NAME "Number"

#endif
