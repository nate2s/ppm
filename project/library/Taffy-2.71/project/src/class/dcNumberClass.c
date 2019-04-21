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

#include "CompiledNumber.h"

#include "dcNumberClass.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcComplexNumberClass.h"
#include "dcContainers.h"
#include "dcDecNumber.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcExceptions.h"
#include "dcFunctionClass.h"
#include "dcGraphData.h"
#include "dcMatrixClass.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcMutex.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcObjectClass.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcThread.h"
#include "dcYesClass.h"

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "digitLimit",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcNumberMetaClass_digitLimit,
        gCFunctionArgument_none
    },
    {
        "pushFractionalOutputLength:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcNumberMetaClass_pushFractionalOutputLength,
        gCFunctionArgument_number
    },
    {
        "popFractionalOutputLength",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcNumberMetaClass_popFractionalOutputLength,
        gCFunctionArgument_none
    },
    {
        "setDefaultDigitLimit:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcNumberMetaClass_setDefaultDigitLimit,
        gCFunctionArgument_number
    },
    {
        "pushDigitLimit:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcNumberMetaClass_pushDigitLimit,
        gCFunctionArgument_number
    },
    {
        "popDigitLimit",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcNumberMetaClass_popDigitLimit,
        gCFunctionArgument_none
    },
    {
        "digitLimit",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcNumberMetaClass_digitLimit,
        gCFunctionArgument_none
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "absoluteValue",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_absoluteValue,
        gCFunctionArgument_none
    },
    {
        "#operator(+):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_add,
        gCFunctionArgument_wild
    },
    {
        "#operator(+=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ),
        &dcNumberClass_addEquals,
        gCFunctionArgument_number
    },
    {
        "#prefixOperator(+)",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_antiNegate,
        gCFunctionArgument_none
    },
    {
        "realBitAnd:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_bitAnd,
        gCFunctionArgument_wild
    },
    {
        "#operator(&=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_bitAndEquals,
        gCFunctionArgument_number
    },
    {
        "#prefixOperator(~)",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_bitNot,
        gCFunctionArgument_none
    },
    {
        "realBitOr:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_bitOr,
        gCFunctionArgument_wild
    },
    {
        "#operator(|=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_bitOrEquals,
        gCFunctionArgument_number
    },
    {
        "realBitXOr:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_bitXOr,
        gCFunctionArgument_wild
    },
    {
        "#operator(^^=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_bitXOrEquals,
        gCFunctionArgument_number
    },
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_asString,
        gCFunctionArgument_none
    },
    {
        "ceiling",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_ceiling,
        gCFunctionArgument_none
    },
    {
        "chomp",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_chomp,
        gCFunctionArgument_none
    },
    {
        "#operator(/):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_divide,
        gCFunctionArgument_wild
    },
    {
        "#operator(/=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_divideEquals,
        gCFunctionArgument_number
    },
    {
        "digitLimit",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST
         | SCOPE_DATA_SYNCHRONIZED_READ),
        &dcNumberClass_digitLimit,
        gCFunctionArgument_none
    },
    {
        "downTo:do:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_downTo,
        gCFunctionArgument_numberBlock
    },
    {
        "trulyEquals:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_trulyEquals,
        gCFunctionArgument_wild
    },
    {
        "deltaEquals:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_deltaEquals,
        gCFunctionArgument_array
    },
    {
        "!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_factorial,
        gCFunctionArgument_none
    },
    {
        "#operator(!)",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_factorial,
        gCFunctionArgument_none
    },
    {
        "factorPairs",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_factorPairs,
        gCFunctionArgument_none
    },
    {
        "factors",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_factors,
        gCFunctionArgument_none
    },
    {
        "factorial",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_factorial,
        gCFunctionArgument_none
    },
    {
        "floor",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_floor,
        gCFunctionArgument_none
    },
    {
        "getFractionalSize",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_getFractionalSize,
        gCFunctionArgument_none
    },
    {
        "#operator(>):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_greaterThan,
        gCFunctionArgument_number
    },
    {
        "#operator(>=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ),
        &dcNumberClass_greaterThanOrEqual,
        gCFunctionArgument_number
    },
    {
        "hash",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_hash,
        gCFunctionArgument_none
    },
    {
        "isWhole",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_isWhole,
        gCFunctionArgument_none
    },
    {
        "#operator(<):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_lessThan,
        gCFunctionArgument_number
    },
    {
        "#operator(<=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ),
        &dcNumberClass_lessThanOrEqual,
        gCFunctionArgument_number
    },
    {
        "realLeftShift:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_leftShift,
        gCFunctionArgument_wild
    },
    {
        "#operator(<<=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_leftShiftEquals,
        gCFunctionArgument_number
    },
    {
        "#operator(--)",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_minusMinus,
        gCFunctionArgument_none
    },
    {
        "realModulus:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_modulus,
        gCFunctionArgument_wild
    },
    {
        "#operator(%=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_modulusEquals,
        gCFunctionArgument_number
    },
    {
        "#operator(*):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_multiply,
        gCFunctionArgument_wild
    },
    {
        "#operator(*=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_multiplyEquals,
        gCFunctionArgument_number
    },
    {
        "#prefixOperator(-)",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_negate,
        gCFunctionArgument_none
    },
    {
        "negate",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_negate,
        gCFunctionArgument_none
    },
    {
        "#operator(++)",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_plusPlus,
        gCFunctionArgument_none
    },
    {
        "raise:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_raise,
        gCFunctionArgument_wild
    },
    {
        "#operator(^=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_raiseEquals,
        gCFunctionArgument_number
    },
    {
        "realRightShift:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_rightShift,
        gCFunctionArgument_wild
    },
    {
        "#operator(>>=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_rightShiftEquals,
        gCFunctionArgument_number
    },
    {
        "#operator(-):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_subtract,
        gCFunctionArgument_wild
    },
    {
        "#operator(-=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcNumberClass_subtractEquals,
        gCFunctionArgument_number
    },
    {
        "upTo:do:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_upTo,
        gCFunctionArgument_numberBlock
    },
    {
        "squareRoot",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcNumberClass_squareRoot,
        gCFunctionArgument_none
    },
    {
        0
    }
};

#define CAST_NUMBER_AUX(node) ((dcNumberClassAux*)(CAST_CLASS_AUX(node)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcNumberClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (NUMBER_PACKAGE_NAME,                      // package name
          NUMBER_CLASS_NAME,                        // class name
          MAKE_FULLY_QUALIFIED(OBJECT),             // super name
          (CLASS_ATOMIC
           | CLASS_HAS_READ_WRITE_LOCK),            // class flags
          NO_FLAGS,                                 // scope data flags
          sMetaMethodWrappers,                      // meta methods
          sMethodWrappers,                          // methods
          &dcNumberClass_initialize,                // initialization function
          &dcNumberClass_deinitialize,              // deinitialization function
          &dcNumberClass_allocateNode,              // allocate
          NULL,                                     // deallocate
          &dcNumberMetaClass_markNode,              // meta mark
          NULL,                                     // mark
          &dcNumberClass_copyNode,                  // copy
          &dcNumberClass_freeNode,                  // free
          NULL,                                     // register
          &dcNumberClass_marshallNode,              // marshall
          &dcNumberClass_unmarshallNode,            // unmarshall
          NULL));                                   // set template
};

static dcNumberMetaClassAux *sMetaAux = NULL;

static dcNumberMetaClassAux *createMetaAux(void)
{
    dcNumberMetaClassAux *aux =
        (dcNumberMetaClassAux *)(dcMemory_allocateAndInitialize
                                 (sizeof(dcNumberMetaClassAux)));

    aux->defaultDigitLimit = 16;
    // don't initialize this yet since Number isn't initialized yet
    aux->defaultDeltaNode = NULL;
    aux->defaultDelta = 6;
    aux->digitLimit = aux->defaultDigitLimit;
    aux->digitLimitStack = dcList_create();

    return aux;
}

static dcNumberClassAux *createAux(dcNumber *_number)
{
    dcError_assert(_number != NULL);
    dcNumberClassAux *aux =
        (dcNumberClassAux *)dcMemory_allocate(sizeof(dcNumberClassAux));
    aux->number = _number;
    return aux;
}

static void createNumberObject(dcNode **_nodeLoc, int32_t _value)
{
    *_nodeLoc = dcNumberClass_createObjectFromInt32s(_value);
    dcNode_register(*_nodeLoc);
}

void dcNumberClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux(dcNumber_createFromInt32u(0));
}

#define NUMBER_TAFFY_FILE_NAME "src/class/Number.ty"

void dcNumberClass_initialize(void)
{
    dcError_assert(sMetaAux == NULL);
    sMetaAux = createMetaAux();
    sMetaAux->defaultDeltaNode =
        dcNumberClass_createObjectFromInt32s(sMetaAux->defaultDelta);

    // create the constant number objects
    createNumberObject(&sMetaAux->zeroNumberObject, 0);
    createNumberObject(&sMetaAux->oneNumberObject, 1);
    createNumberObject(&sMetaAux->negativeOneNumberObject, -1);
    sMetaAux->digitLimitMutex = dcMutex_create(false);

    dcError_assert(dcStringEvaluator_evalString(__compiledNumber,
                                                NUMBER_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

void dcNumberClass_deinitialize(void)
{
    dcError_assert(sMetaAux != NULL);

    dcMutex_lock(sMetaAux->digitLimitMutex);
    {
        dcList_free(&sMetaAux->digitLimitStack, DC_SHALLOW);
        sMetaAux->digitLimitStack = NULL;
    }
    dcMutex_unlock(sMetaAux->digitLimitMutex);

    dcNode_free(&sMetaAux->defaultDeltaNode, DC_DEEP);
    dcMutex_free(&sMetaAux->digitLimitMutex);
    dcMemory_free(sMetaAux);
    sMetaAux = NULL;
}

void dcNumberMetaClass_markNode(dcNode *_node)
{
    if (sMetaAux != NULL)
    {
        dcMutex_lock(sMetaAux->digitLimitMutex);
        dcList_mark(sMetaAux->digitLimitStack);
        dcMutex_unlock(sMetaAux->digitLimitMutex);

        dcNode_mark(sMetaAux->defaultDeltaNode);
        dcNode_mark(sMetaAux->zeroNumberObject);
        dcNode_mark(sMetaAux->oneNumberObject);
        dcNode_mark(sMetaAux->negativeOneNumberObject);
    }
}

void dcNumberClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_CLASS_AUX(_to) =
        createAux(dcNumber_copy(CAST_NUMBER_AUX(_from)->number, DC_DEEP));
}

void dcNumberClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcNumberClassAux *aux = CAST_NUMBER_AUX(_node);
    dcNumber_free(&aux->number, DC_DEEP);
    dcMemory_free(aux);
}

dcNode *dcNumberClass_getZeroNumberObject(void)
{
    dcError_assert(sMetaAux->zeroNumberObject != NULL);
    return sMetaAux->zeroNumberObject;
}

dcNode *dcNumberClass_getOneNumberObject(void)
{
    dcError_assert(sMetaAux->oneNumberObject != NULL);
    return sMetaAux->oneNumberObject;
}

bool dcNumberClass_isZero(dcNode *_value)
{
    return (dcNumberClass_isMe(_value)
            && dcNumberClass_equalsNumber(_value,
                                          dcNumberClass_getZeroNumberObject()));
}

bool dcNumberClass_isOne(dcNode *_value)
{
    return (dcNumberClass_isMe(_value)
            && dcNumberClass_equalsNumber(_value,
                                          dcNumberClass_getOneNumberObject()));
}

bool dcNumberClass_isEven(dcNode *_value)
{
    const dcNumber *two = dcNumber_getConstant(2);
    dcNumber *modulusResult = dcNumber_createFromInt32u(1);
    dcNumber_modulus(modulusResult, dcNumberClass_getNumber(_value), two);
    bool result = dcNumber_equalsInt32u(modulusResult, 0);
    dcNumber_free(&modulusResult, DC_DEEP);
    return result;
}

bool dcNumberClass_isWholeHelper(dcNode *_value)
{
    return dcNumber_isWhole(dcNumberClass_getNumber(_value));
}

bool dcNumberClass_isNegativeOne(dcNode *_value)
{
    return (dcNumberClass_isMe(_value)
            && (dcNumberClass_equalsNumber
                (_value, dcNumberClass_getNegativeOneNumberObject())));
}

dcNode *dcNumberClass_getNegativeOneNumberObject(void)
{
    dcError_assert(sMetaAux->negativeOneNumberObject != NULL);
    return sMetaAux->negativeOneNumberObject;
}

//
// Create and initialize a Number class
//
dcNode *dcNumberClass_createNode(dcNumberClassAux *_aux, bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL, // super
                              NULL, // scope
                              _object,
                              _aux);
}

dcNode *dcNumberClass_createObject(dcNumber *_number)
{
    dcNumberClassAux *aux = createAux(_number);
    return dcNumberClass_createNode(aux, true);
}

////////////////////////////////////////////////////
//
// Creates a Number class object that stores an
// int as its internal number
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_createObjectFromInt32s(int32_t _value)
{
    return dcNumberClass_createNode
        (createAux(dcNumber_createFromInt32s(_value)), true);
}

////////////////////////////////////////////////////
//
// Creates a Number class object that stores an
// int as its internal number
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_createObjectFromInt32u(uint32_t _value)
{
    return dcNumberClass_createNode
        (createAux(dcNumber_createFromInt32u(_value)), true);
}

dcNode *dcNumberClass_createObjectFromInt64u(uint64_t _value)
{
    return dcNumberClass_createNode
        (createAux(dcNumber_createFromInt64u(_value)), true);
}

dcNode *dcNumberClass_createObjectFromSizet(size_t _value)
{
    return dcNumberClass_createNode
        (createAux(dcNumber_createFromSizet(_value)), true);
}

////////////////////////////////////////////////////
//
// Creates a Number class object that stores an
// decNumber as its internal number
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_createObjectFromString(const char *_value)
{
    return dcNumberClass_createNode
        (createAux(dcNumber_createFromString(_value)), true);
}

////////////////////////////////////////////////////
//
// Creates a Number class object that stores a
// double as its internal number
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_createObjectFromDouble(double _value)
{
    return dcNumberClass_createNode
        (createAux(dcNumber_createFromDouble(_value)), true);
}

bool dcNumberClass_extractInt16u(const dcNode *_preNumber,
                                 uint16_t *_convertedValue)
{
    return (dcClass_hasTemplate(_preNumber,
                                sTemplate,
                                true)
            && dcNumber_extractUInt16(CAST_NUMBER_AUX(_preNumber)->number,
                                      _convertedValue));
}

bool dcNumberClass_extractInt32u(const dcNode *_preNumber,
                                 uint32_t *_convertedValue)
{
    return (dcClass_hasTemplate(_preNumber,
                                sTemplate,
                                true)
            && dcNumber_extractUInt32(CAST_NUMBER_AUX(_preNumber)->number,
                                      _convertedValue));
}

bool dcNumberClass_extractInt32s(const dcNode *_preNumber,
                                 int32_t *_convertedValue)
{
    return (dcClass_hasTemplate(_preNumber,
                                sTemplate,
                                true)
            && dcNumber_extractInt32(dcNumberClass_getNumber(_preNumber),
                                      _convertedValue));
}

bool dcNumberClass_extractInt32s_withException(dcNode *_preNumber,
                                               int32_t *_convertedValue)
{
    bool success = dcNumber_extractInt32(CAST_NUMBER_AUX(_preNumber)->number,
                                         _convertedValue);
    if (! success)
    {
        dcNeedIntegerExceptionClass_throwObject(_preNumber);
    }

    return success;
}

bool dcNumberClass_extractInt64s_withException(dcNode *_preNumber,
                                               int64_t *_convertedValue)
{
    bool success = dcNumber_extractInt64(CAST_NUMBER_AUX(_preNumber)->number,
                                         _convertedValue);
    if (! success)
    {
        dcNeedIntegerExceptionClass_throwObject(_preNumber);
    }

    return success;
}

bool dcNumberClass_extract64BitIndex(struct dcNode_t *_preNumber,
                                     uint64_t *_extractedValue,
                                     uint64_t _limit)
{
    bool result = false;
    dcNumber *preNumber = dcNumberClass_getNumber(_preNumber);

    if (! dcNumber_isWhole(preNumber))
    {
        dcNeedIntegerExceptionClass_throwObject(_preNumber);
    }
    else if (dcNumber_isNonNegative(preNumber))
    {
        result = (dcNumberClass_extractInt64u_withException(_preNumber,
                                                            _extractedValue)
                  && ! (dcIndexOutOfBoundsExceptionClass_checkThrow
                        (*_extractedValue,
                         _limit)));
    }
    else
    {
        int64_t value;

        if (dcNumberClass_extractInt64s_withException(_preNumber, &value))
        {
            while (value < 0)
            {
                value += _limit;
            }

            *_extractedValue = value;
            result = true;
        }
        else
        {
            result = false;
        }
    }

    return result;
}

bool dcNumberClass_extractIndex(dcNode *_preNumber,
                                uint32_t *_extractedValue,
                                uint32_t _limit)
{
    bool result = false;
    dcNumber *preNumber = dcNumberClass_getNumber(_preNumber);

    if (! dcNumber_isWhole(preNumber))
    {
        dcNeedIntegerExceptionClass_throwObject(_preNumber);
    }
    else if (dcNumber_isNonNegative(preNumber))
    {
        result = (dcNumberClass_extractInt32u_withException(_preNumber,
                                                            _extractedValue)
                  && ! (dcIndexOutOfBoundsExceptionClass_checkThrow
                        (*_extractedValue,
                         _limit)));
    }
    else
    {
        int32_t value;

        if (dcNumberClass_extractInt32s_withException(_preNumber, &value))
        {
            while (value < 0)
            {
                value += _limit;
            }

            *_extractedValue = value;
            result = true;
        }
        else
        {
            result = false;
        }
    }

    return result;
}

bool dcNumberClass_extractInt16u_withException(dcNode *_preNumber,
                                               uint16_t *_convertedValue)
{
    bool success = dcNumber_extractUInt16(CAST_NUMBER_AUX(_preNumber)->number,
                                          _convertedValue);
    if (! success)
    {
        dcNeedIntegerExceptionClass_throwObject(_preNumber);
    }

    return success;
}

bool dcNumberClass_extractInt32u_withException(dcNode *_preNumber,
                                               uint32_t *_convertedValue)
{
    bool success = dcNumber_extractUInt32(CAST_NUMBER_AUX(_preNumber)->number,
                                          _convertedValue);
    if (! success)
    {
        dcNeedIntegerExceptionClass_throwObject(_preNumber);
    }

    return success;
}

bool dcNumberClass_extractInt64u_withException(dcNode *_preNumber,
                                               uint64_t *_convertedValue)
{
    bool success = dcNumber_extractUInt64(CAST_NUMBER_AUX(_preNumber)->number,
                                          _convertedValue);
    if (! success)
    {
        dcNeedIntegerExceptionClass_throwObject(_preNumber);
    }

    return success;
}

bool dcNumberClass_extractInt8u_withException(dcNode *_preNumber,
                                              uint8_t *_convertedValue)
{
    uint8_t value = 0;
    bool success = false;

    if (! dcNumber_extractUInt8(dcNumberClass_getNumber(_preNumber), &value))
    {
        dcNeedByteExceptionClass_throwObject(_preNumber);
    }
    else
    {
        success = true;
        *_convertedValue = value;
    }

    return success;
}

bool dcNumberClass_extractDouble_withException(dcNode *_preNumber,
                                               double *_convertedValue)
{
    double value = 0;
    bool success = false;

    if (! dcNumber_extractDouble(dcNumberClass_getNumber(_preNumber), &value))
    {
        dcNeedDoubleExceptionClass_throwObject(_preNumber);
    }
    else
    {
        success = true;
        *_convertedValue = value;
    }

    return success;
}

bool dcNumberClass_equalsInt32u(const dcNode *_object, uint32_t _number)
{
    return (dcClass_hasTemplate(_object, sTemplate, true)
            && dcNumber_equalsInt32u(dcNumberClass_getNumber(_object),
                                     _number));
}

bool dcNumberClass_equalsInt32s(const dcNode *_object, int32_t _number)
{
    return (dcClass_hasTemplate(_object, sTemplate, true)
            && dcNumber_equalsInt32s(dcNumberClass_getNumber(_object),
                                     _number));
}

bool dcNumberClass_extractDouble(const dcNode *_preNumber,
                                 double *_convertedValue)
{
    return dcNumber_extractDouble(CAST_NUMBER_AUX(_preNumber)->number,
                                  _convertedValue);
}

// verifies the type of _preNumber //
bool dcNumberClass_verifyType(dcNode *_preNumber, uint16_t _type)
{
    bool result = false;

    if (IS_CLASS(_preNumber)
        && dcClass_hasTemplate(_preNumber, sTemplate, true))
    {
        if (dcNumberClass_getNumber(_preNumber)->type == _type)
        {
            result = true;
        }
    }

    return result;
}

dcNumber *dcNumberClass_getNumber(const dcNode *_numberObject)
{
    dcNumberClassAux *aux = CAST_NUMBER_AUX(_numberObject);
    return aux->number;
}

void dcNumberClass_increment(dcNode *_node)
{
    dcNumber_increment(dcNumberClass_getNumber(_node));
}

void dcNumberClass_decrement(dcNode *_node)
{
    dcNumber_decrement(dcNumberClass_getNumber(_node));
}

static dcNode *inlineOperation(dcNode *_left,
                               dcNode *_right,
                               dcNumber_arithmeticFunction _operation)
{
    dcNumber *left = dcNumberClass_getNumber(_left);
    _operation(left, left, dcNumberClass_getNumber(_right));
    return _left;
}

dcNode *dcNumberClass_inlineAdd(dcNode *_left, dcNode *_right)
{
    return inlineOperation(_left, _right, &dcNumber_add);
}

dcNode *dcNumberClass_inlineSubtract(dcNode *_left, dcNode *_right)
{
    return inlineOperation(_left, _right, &dcNumber_subtract);
}

dcNode *dcNumberClass_inlineMultiply(dcNode *_left, dcNode *_right)
{
    return inlineOperation(_left, _right, &dcNumber_multiply);
}

dcNode *dcNumberClass_inlineDivide(dcNode *_left, dcNode *_right)
{
    return inlineOperation(_left, _right, &dcNumber_divide);
}

dcNode *dcNumberClass_inlineRaise(dcNode *_left, dcNode *_right)
{
    return inlineOperation(_left, _right, &dcNumber_raise);
}

TAFFY_HIDDEN dcNode *performTwoOperandsOperation
    (dcNode *_left,
     dcNode *_candidate,
     dcTaffyOperator _operation,
     dcNumber_arithmeticFunction _numberFunction,
     bool _needInteger)
{
    dcNode *right = NULL;
    dcNode *result = NULL;

    if (_needInteger
        && ! dcNumber_isWhole(dcNumberClass_getNumber(_left)))
    {
        dcNeedIntegerExceptionClass_throwObject(_left);
    }
    else if ((right = dcClass_castNode(_candidate, sTemplate, false))
             != NULL)
    {
        if (_needInteger && ! dcNumber_isWhole(dcNumberClass_getNumber(right)))
        {
            dcNeedIntegerExceptionClass_throwObject(right);
        }
        else
        {
            dcNumber *numberResult = dcNumber_createFromInt32u(0);
            _numberFunction(numberResult,
                            CAST_NUMBER_AUX(_left)->number,
                            CAST_NUMBER_AUX(right)->number);
            result = dcNumberClass_createObject(numberResult);
        }
    }
    else if ((right = dcClass_castNode(_candidate,
                                       dcMatrixClass_getTemplate(),
                                       false))
             != NULL)
    {
        result = dcMatrixClass_matrixAndNumberOperation
            (right, _left, _operation, false);
    }
    else if ((right = dcClass_castNode(_candidate,
                                       dcFunctionClass_getTemplate(),
                                       false))
             != NULL)
    {
        result = dcFunctionClass_numberOperation
            (right, _left, _operation, false);
    }
    else if ((right = dcClass_castNode(_candidate,
                                       dcComplexNumberClass_getTemplate(),
                                       false))
             != NULL)
    {
        result = dcComplexNumberClass_numberOperation
            (_left, right, _operation, false);
    }
    else
    {
        dcInvalidCastExceptionClass_throwObject
            (dcClass_getName(_candidate), "(Number, Matrix or Function)");
    }

    dcNode_register(result);
    return result;
}

dcNode *dcNumberClass_addHelper(dcNode *_left, dcNode *_right)
{
    return performTwoOperandsOperation(_left,
                                       _right,
                                       TAFFY_ADD,
                                       &dcNumber_add,
                                       false);
}

dcNode *dcNumberClass_subtractHelper(dcNode *_left, dcNode *_right)
{
    return performTwoOperandsOperation(_left,
                                       _right,
                                       TAFFY_SUBTRACT,
                                       &dcNumber_subtract,
                                       false);
}

dcNode *dcNumberClass_multiplyHelper(dcNode *_left, dcNode *_right)
{
    return performTwoOperandsOperation(_left,
                                       _right,
                                       TAFFY_MULTIPLY,
                                       &dcNumber_multiply,
                                       false);
}

dcNode *dcNumberClass_divideHelper(dcNode *_left, dcNode *_right)
{
    return performTwoOperandsOperation(_left,
                                       _right,
                                       TAFFY_DIVIDE,
                                       &dcNumber_divide,
                                       false);
}

dcNode *dcNumberClass_modulusHelper(dcNode *_left, dcNode *_right)
{
    dcNode *result = NULL;

    if (dcNumberClass_isMe(_right)
        && dcNumber_equals(dcNumberClass_getNumber(_right),
                           dcNumber_getConstant(0)))
    {
        dcDivideByZeroExceptionClass_throwObject();
    }
    else
    {
        result = performTwoOperandsOperation
            (_left, _right, TAFFY_MODULUS, &dcNumber_modulus, true);
    }

    return result;
}

uint32_t dcNumberMetaClass_getDigitLimit(void)
{
    return sMetaAux->digitLimit;
}

dcNode *dcNumberMetaClass_setDefaultDigitLimit(dcNode *_receiver,
                                            dcArray *_arguments)
{
    dcNode *digitLimitNode = dcArray_get(_arguments, 0);
    dcNode *result = digitLimitNode;
    int32_t defaultDigitLimit = 0;

    if (!(dcNumberClass_extractInt32s_withException(digitLimitNode,
                                                    &defaultDigitLimit)))
    {
        // do nothing, exception is already set //
    }
    else
    {
        sMetaAux->defaultDigitLimit = defaultDigitLimit;
    }

    return result;
}

dcNode *dcNumberMetaClass_pushFractionalOutputLength(dcNode *_receiver,
                                                     dcArray *_arguments)
{
    return NULL;
}

dcNode *dcNumberMetaClass_popFractionalOutputLength(dcNode *_receiver,
                                                    dcArray *_arguments)
{
    return NULL;
}

void dcNumberMetaClass_pushDigitLimitHelper(dcNode *_digitLimitNode,
                                            uint32_t _digitLimit)
{
    dcMutex_lock(sMetaAux->digitLimitMutex);

    if (_digitLimitNode == NULL)
    {
        _digitLimitNode =
            dcNode_register(dcNumberClass_createObjectFromInt32u(_digitLimit));
    }

    _digitLimitNode = dcNode_copyIfTemplate(_digitLimitNode);
    dcList_push(sMetaAux->digitLimitStack, _digitLimitNode);
    sMetaAux->digitLimit = _digitLimit;
    dcDecNumber_setLsuSize(_digitLimit);
    dcMutex_unlock(sMetaAux->digitLimitMutex);
}

dcNode *dcNumberMetaClass_pushDigitLimit(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *digitLimitNode = dcArray_get(_arguments, 0);
    dcNode *result = NULL;
    int32_t digitLimit = 0;

    if (dcNumberClass_extractInt32s_withException(digitLimitNode, &digitLimit))
    {
        if (digitLimit < 0)
        {
            dcNeedPositiveIntegerExceptionClass_throwObject(digitLimitNode);
            result = NULL;
        }
        else if (digitLimit < 5)
        {
            result = dcNoClass_getInstance();
        }
        else
        {
            // we're good to go
            dcNumberMetaClass_pushDigitLimitHelper(digitLimitNode, digitLimit);
            result = digitLimitNode;
        }
    }
    // else do nothing, exception is already set

    return result;
}

void dcNumberMetaClass_popDigitLimitHelper(void)
{
    dcMutex_lock(sMetaAux->digitLimitMutex);
    dcList_pop(sMetaAux->digitLimitStack, DC_SHALLOW);

    dcNode *tail = dcList_getTail(sMetaAux->digitLimitStack);
    uint32_t digitLimitToSet = 0;

    if (tail != NULL)
    {
        dcNumber *number = dcNumberClass_getNumber(tail);
        dcNumber_extractUInt32(number, &digitLimitToSet);
    }
    else
    {
        digitLimitToSet = sMetaAux->defaultDigitLimit;
    }

    sMetaAux->digitLimit = digitLimitToSet;
    dcDecNumber_setLsuSize(digitLimitToSet);
    dcMutex_unlock(sMetaAux->digitLimitMutex);
}

dcNode *dcNumberMetaClass_popDigitLimit(dcNode *_receiver, dcArray *_arguments)
{
    dcNumberMetaClass_popDigitLimitHelper();
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32u
             (sMetaAux->digitLimit)));
}

dcNode *dcNumberMetaClass_digitLimit(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32u
             (sMetaAux->digitLimit)));
}

/////////////////////////////////////////////////
//
// Number#-
//
// Returns the negative of a Number object
//
// Example
//
//   a = 1
//   -a
//   ==> -1
//
//   b = -1
//   -b
//   ==> 1
//
//////////////////////////////////////////////////
dcNode *dcNumberClass_negate(dcNode *_receiver, dcArray *_arguments)
{
    dcNumber *number = dcNumber_createFromInt32u(0);
    dcNumber_multiply(number,
                      CAST_NUMBER_AUX(_receiver)->number,
                      dcNumber_getNegativeOne());
    return dcNode_register(dcNumberClass_createObject(number));
}

/////////////////////////////////////////////////
//
// Number#isWhole
//
// Returns if the number has no fractional component
//
// Example
//
//   a = 1.1
//   a isWhole
//   ==> false
//
//   b = 1
//   b isWhole
//   ==> true
//
//////////////////////////////////////////////////
dcNode *dcNumberClass_isWhole(dcNode *_receiver, dcArray *_arguments)
{
    return dcSystem_convertBoolToNode(dcNumberClass_isWholeHelper(_receiver));
}

///////////////////////////////////////////////////////
//
// Number#+(prefix)
//
// Returns self
//
// Example
//
//   a = 1
//   +a
//   ==> 1
//
//   b = -1
//   +b
//   ==> -1
//
///////////////////////////////////////////////////////
dcNode *dcNumberClass_antiNegate(dcNode *_receiver, dcArray *_arguments)
{
    return _receiver;
}

//////////////////////////////////////////////////
//
// Number#absoluteValue
//
// Returns the absolute value of a Number object
//
// Example
//
//   a = 1
//   a absoluteValue
//   ==> 1
//
//   b = -1.1
//   b absoluteValue
//   ==> 1.1
//
//////////////////////////////////////////////////
dcNode *dcNumberClass_absoluteValue(dcNode *_receiver, dcArray *_arguments)
{
    dcNumber *number = dcNumber_createFromInt32u(0);
    dcNumber_absoluteValue(number, CAST_NUMBER_AUX(_receiver)->number);
    return dcNode_register(dcNumberClass_createObject(number));
}

bool dcNumberClass_equalsNumber(dcNode *_left, dcNode *_right)
{
    return (dcNumberClass_isMe(_left)
            && dcNumberClass_isMe(_right)
            && dcNumber_equals(dcNumberClass_getNumber(_left),
                               dcNumberClass_getNumber(_right)));
}

//////////////////////////////////////////
//
// Number#equals
//
// Returns the equality of two Number objects
//
// Example
//
//   a = 1
//   b = 2
//   a == b
//   ==> no
//
//   a == 1
//   ==> yes
//
//////////////////////////////////////////
dcNode *dcNumberClass_trulyEquals(dcNode *_receiver, dcArray *_arguments)
{
    bool equals = false;
    dcNode *right = dcClass_castNode(dcArray_get(_arguments, 0),
                                     sTemplate,
                                     false);
    if (right != NULL)
    {
        equals = dcNumberClass_equalsNumber(_receiver, right);
    }

    return dcSystem_convertBoolToNode(equals);
}

//////////////////////////////////////////
//
// Number#+
//
// Returns the sum of two Number objects
//
// Example
//
//   a = 1
//   b = 2
//   a + b
//   ==> 3
//
//////////////////////////////////////////
dcNode *dcNumberClass_add(dcNode *_receiver, dcArray *_arguments)
{
    return dcNumberClass_addHelper(_receiver, dcArray_get(_arguments, 0));
}

///////////////////////////////////////////////////
//
// Number#&
//
// Returns the bit-wise "and" of two Number objects
//
// Example
//
//   a = 0b1001
//   b = 0b1011
//   a & b
//   ==> 0b1001
//
///////////////////////////////////////////////////
dcNode *dcNumberClass_bitAnd(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_BIT_AND,
                                       &dcNumber_bitAnd,
                                       true);
}

///////////////////////////////////////////////////
//
// Number#|
//
// Returns the bit-wise "or" of two Number objects
//
// Example
//
//   a = 0b1001
//   b = 0b0011
//   a | b
//   ==> 0b1011
//
///////////////////////////////////////////////////
dcNode *dcNumberClass_bitOr(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_BIT_OR,
                                       &dcNumber_bitOr,
                                       true);
}

dcNode *dcNumberClass_bitXOr(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_BIT_XOR,
                                       &dcNumber_bitXOr,
                                       true);
}

///////////////////////////////////////////////////
//
// Number#prefixOperator(~)
//
// Returns the bit-wise not of a number object
//
// Example
//
//   a = 0b0001
//   b = 0b0011
//   a & b
//   ==> 0b0001
//
///////////////////////////////////////////////////
dcNode *dcNumberClass_bitNot(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = _receiver;

    if (! dcNumber_isWhole(dcNumberClass_getNumber(result)))
    {
        dcNeedIntegerExceptionClass_throwObject(_receiver);
        result = NULL;
    }
    else
    {
        result = dcNode_register(dcNode_copy(_receiver, DC_DEEP));
        dcNumber *number = dcNumberClass_getNumber(result);
        dcNumber_bitNot(number, number);
    }

    return result;
}

///////////////////////////////////////////////////
//
// Number#<<
//
// Left shifts receiver
//
// Example
//
//   a = 1
//   a << 1
//   ==> 2
//
///////////////////////////////////////////////////
dcNode *dcNumberClass_leftShift(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_LEFT_SHIFT,
                                       &dcNumber_leftShift,
                                       true);
}

///////////////////////////////////////////////////
//
// Number#>>
//
// Right shifts receiver
//
// Example
//
//   a = 2
//   a >> 1
//   ==> 1
//
///////////////////////////////////////////////////
dcNode *dcNumberClass_rightShift(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_RIGHT_SHIFT,
                                       &dcNumber_rightShift,
                                       true);
}

dcNode *dcNumberClass_addEquals(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *right = dcArray_get(_arguments, 0);
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_receiver);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(right);
    dcNumber *result = dcNumber_createFromInt32u(0);
    dcNumber_add(result, leftAux->number, rightAux->number);
    dcNumber_free(&leftAux->number, DC_DEEP);
    leftAux->number = result;
    return _receiver;
}

//////////////////////////////////////////
//
// Number#/
//
// Returns the quotient of two numbers
//
// Example
//
//   a = 1
//   b = 0.5
//   a / b
//   ==> 2
//
//////////////////////////////////////////
dcNode *dcNumberClass_divide(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *right = dcArray_get(_arguments, 0);
    dcNode *result = NULL;

    if (dcNumberClass_isMe(right)
        && dcNumber_equals(dcNumberClass_getNumber(right),
                           dcNumber_getConstant(0)))
    {
        dcDivideByZeroExceptionClass_throwObject();
    }
    else
    {
        result = (dcNode_register
                  (dcNumberClass_divideHelper
                   (_receiver, dcArray_get(_arguments, 0))));
    }

    return result;
}

dcNode *dcNumberClass_digitLimit(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32u
             (dcNumber_getDigitLimit
              (dcNumberClass_getNumber(_receiver)))));
}

dcNode *dcNumberClass_divideEquals(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *right = dcArray_get(_arguments, 0);
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_receiver);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(right);
    dcNumber *result = dcNumber_createFromInt32u(0);
    dcNumber_divide(result, leftAux->number, rightAux->number);
    dcNumber_free(&leftAux->number, DC_DEEP);
    leftAux->number = result;
    return _receiver;
}

///////////////////////////////////////
//
// Number#*
//
// Returns the product of two Numbers
//
// Example
//
//   a = 100000
//   b = 200008
//   a * b
//   ==> 20000800000
//
///////////////////////////////////////
dcNode *dcNumberClass_multiply(dcNode *_receiver, dcArray *_arguments)
{
    return dcNumberClass_multiplyHelper(_receiver, dcArray_get(_arguments, 0));
}

///////////////////////////////////////
//
// Number#%
//
// Returns the modulus of two Numbers
//
// Example
//
//   a = 10
//   b = 7
//   a % b
//   ==> 3
//
///////////////////////////////////////
dcNode *dcNumberClass_modulus(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *right = dcArray_get(_arguments, 0);
    dcNode *casted = dcClass_castNode(right, sTemplate, false);

    if (casted != NULL)
    {
        right = casted;

        if (dcNumber_equals(dcNumberClass_getNumber(casted),
                            dcNumber_getConstant(0)))
        {
            dcDivideByZeroExceptionClass_throwObject();
            return NULL;
        }
    }

    return performTwoOperandsOperation(_receiver,
                                       right,
                                       TAFFY_MODULUS,
                                       &dcNumber_modulus,
                                       false);
}

dcNode *dcNumberClass_multiplyEquals(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *right = dcArray_get(_arguments, 0);
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_receiver);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(right);
    dcNumber *result = dcNumber_createFromInt32u(0);
    dcNumber_multiply(result, leftAux->number, rightAux->number);
    dcNumber_free(&leftAux->number, DC_DEEP);
    leftAux->number = result;
    return _receiver;
}

//////////////////////////////////////////////////
//
// Number#-
//
// Returns the difference of two Number objects
//
// Example
//
//   a = 1
//   b = 2
//   a - b
//   ==> -1
//
//////////////////////////////////////////////////
dcNode *dcNumberClass_subtract(dcNode *_receiver, dcArray *_arguments)
{
    return dcNumberClass_subtractHelper(_receiver, dcArray_get(_arguments, 0));
}

dcNode *dcNumberClass_subtractEquals(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *right = dcArray_get(_arguments, 0);
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_receiver);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(right);
    dcNumber *result = dcNumber_createFromInt32u(0);
    dcNumber_subtract(result, leftAux->number, rightAux->number);
    dcNumber_free(&leftAux->number, DC_DEEP);
    leftAux->number = result;
    return _receiver;
}

//////////////////////////////////////////////////////////////////////////
//
// Number#ceiling
//
// Returns the ceiling of the calling number. This effectively "casts"
// any rational/irrational to an integer
//
// Example
//
//   a = 2.333
//   a ceiling
//   ==> 3
//
//////////////////////////////////////////////////////////////////////////
dcNode *dcNumberClass_ceiling(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNode_register(dcNumberClass_createObjectFromInt32u(0));
    dcNumber_ceiling(dcNumberClass_getNumber(result),
                     dcNumberClass_getNumber(_receiver));
    return result;
}

dcNode *dcNumberClass_chompHelper(dcNode *_receiver)
{
    dcNumber_chomp(dcNumberClass_getNumber(_receiver),
                   dcNumberClass_getNumber(_receiver));
    return _receiver;
}

dcNode *dcNumberClass_chomp(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *chomped = dcNode_register(dcNode_copy(_receiver, DC_DEEP));
    dcNumberClass_chompHelper(chomped);
    return chomped;
}

dcNode *dcNumberMetaClass_getDefaultDelta(void)
{
    return sMetaAux->defaultDeltaNode;
}

dcNode *dcNumberClass_deltaEquals(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();
    dcArray *arguments = dcArrayClass_getObjects(dcArray_get(_arguments, 0));
    int32_t digitLimit = 0;
    dcNumber *left = dcNumberClass_getNumber(_receiver);
    dcNumber *right = NULL;
    dcNode *otherCandidate = dcClass_castNode(dcArray_get(arguments, 0),
                                              sTemplate,
                                              false);

    if (otherCandidate != NULL)
    {
        right = dcNumberClass_getNumber(otherCandidate);
        dcNode *digitLimitNode = dcClass_castNode(dcArray_get(arguments, 1),
                                                  sTemplate,
                                                  false);

        if (digitLimitNode != NULL)
        {
            if (dcNumberClass_extractInt32s(digitLimitNode, &digitLimit))
            {
                if (dcNumber_deltaEqual(left, right, digitLimit))
                {
                    result = dcYesClass_getInstance();
                }
            }
            else
            {
                result = NULL;
                dcNeedIntegerExceptionClass_throwObject(digitLimitNode);
            }
        }
        // else result is no
    }
    // else result is no

    return result;
}

//////////////////////////////////////////////////////////////////////////
//
// Number#floor
//
// Returns the floor of the calling number. This effectively casts any
// rational/irrational to an integer
//
// Example
//
//   a = 2.333
//   a floor
//   ==> 2
//
//////////////////////////////////////////////////////////////////////////
dcNode *dcNumberClass_floor(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNode_register(dcNumberClass_createObjectFromInt32u(0));
    dcNumber_floor(dcNumberClass_getNumber(result),
                   dcNumberClass_getNumber(_receiver));
    return result;
}

////////////////////////////////////////////////////
//
// Number#^
//
// Returns the exponentiation of two Number objects
//
// Example
//
//   a = 2
//   b = 10
//   a ^ b
//   ==> 1024
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_raise(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_RAISE,
                                       &dcNumber_raise,
                                       false);
}

////////////////////////////////////////////////////
//
// Number#^=
//
// Raises left by right
//
// Example
//
//   a = 2
//   b = 10
//   a ^= b
//   ==> 1024
//   a == 1024
//   ==> yes
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_raiseEquals(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *right = dcArray_get(_arguments, 0);
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_receiver);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(right);
    dcNumber *result = NULL;

    if (dcNumber_equals(rightAux->number, dcNumber_getConstant(0)))
    {
        result = dcNumber_createFromInt32u(1);
    }
    else
    {
        result = dcNumber_createFromInt32u(0);
        dcNumber_raise(result,
                       leftAux->number,
                       rightAux->number);
    }

    dcNumber_free(&leftAux->number, DC_DEEP);
    leftAux->number = result;
    return _receiver;
}

////////////////////////////////////////////////////
//
// Number#%=
//
// Sets left to be left modulo right
//
// Example
//
//   a = 101
//   b = 100
//   a %= b
//   ==> 1
//   a == 1
//   ==> true
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_modulusEquals(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *right = dcArray_get(_arguments, 0);
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_receiver);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(right);
    dcNode *result = _receiver;

    if (dcNumber_equals(rightAux->number, dcNumber_getConstant(0)))
    {
        dcDivideByZeroExceptionClass_throwObject();
        result = NULL;
    }
    else
    {
        dcNumber_modulus(leftAux->number,
                         leftAux->number,
                         rightAux->number);
    }

    return result;
}

static dcNode *operationAndSet(dcNode *_receiver,
                               dcArray *_arguments,
                               dcNumber_arithmeticFunction _function)
{
    dcNode *right = dcArray_get(_arguments, 0);
    dcNode *result = _receiver;

    if (dcNumberClass_isWholeHelper(right))
    {
        dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_receiver);
        _function(leftAux->number,
                  leftAux->number,
                  CAST_NUMBER_AUX(right)->number);
    }
    else
    {
        dcNeedIntegerExceptionClass_throwObject(right);
        result = NULL;
    }

    return result;
}

////////////////////////////////////////////////////
//
// Number#<<=
//
// Left shifts and sets
//
// Example
//
//   a = 1
//   a <<= 2
//   ==> 4
//   a == 4
//   ==> true
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_leftShiftEquals(dcNode *_receiver, dcArray *_arguments)
{
    return operationAndSet(_receiver, _arguments, dcNumber_leftShift);
}

////////////////////////////////////////////////////
//
// Number#>>=
//
// Right shifts and sets
//
// Example
//
//   a = 4
//   a <<= 2
//   ==> 1
//   a == 1
//   ==> true
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_rightShiftEquals(dcNode *_receiver, dcArray *_arguments)
{
    return operationAndSet(_receiver, _arguments, dcNumber_rightShift);
}

////////////////////////////////////////////////////
//
// Number#&=
//
// Bit-ands and sets
//
// Example
//
//   a = 0b1001
//   a &= 0b1011
//   ==> 0b1001
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_bitAndEquals(dcNode *_receiver, dcArray *_arguments)
{
    return operationAndSet(_receiver, _arguments, dcNumber_bitAnd);
}

////////////////////////////////////////////////////
//
// Number#|=
//
// Bit-ors and sets
//
// Example
//
//   a = 0b1001
//   a |= 0b1011
//   ==> 0b1011
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_bitOrEquals(dcNode *_receiver, dcArray *_arguments)
{
    return operationAndSet(_receiver, _arguments, dcNumber_bitOr);
}

////////////////////////////////////////////////////
//
// Number#^^=
//
// Bit-xors and sets
//
// Example
//
//   a = 0b1001
//   a ^^= 0b1011
//   ==> 0b0010
//
////////////////////////////////////////////////////
dcNode *dcNumberClass_bitXOrEquals(dcNode *_receiver, dcArray *_arguments)
{
    return operationAndSet(_receiver, _arguments, dcNumber_bitXOr);
}

bool dcNumberClass_lessThanHelper(dcNode *_left, dcNode *_right)
{
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_left);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(_right);
    return dcNumber_lessThan(leftAux->number, rightAux->number);
}

bool dcNumberClass_greaterThanHelper(dcNode *_left, dcNode *_right)
{
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_left);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(_right);
    return dcNumber_greaterThan(leftAux->number, rightAux->number);
}

//////////////////////////////////////////
//
// Number#<
//
// Checks if the receiver is less than
// the argument
//
// Example
//
//   a = 100000000000000000000000000
//   b = 10000000000000000000000000.1
//   a < b
//   ==> yes
//
//////////////////////////////////////////
dcNode *dcNumberClass_lessThan(dcNode *_receiver, dcArray *_arguments)
{
    return (dcSystem_convertBoolToNode
            (dcNumberClass_lessThanHelper(_receiver,
                                          dcArray_get(_arguments, 0))));
}

//////////////////////////////////////////////
// Number#<=
//
// Checks if the receiver is less or equal to
// the argument
//
// Example
//
//   a = 1.1111111131313131313131312
//   b = 1.1111111131313131313131313
//   a <= b
//   ==> yes
//
//////////////////////////////////////////////
dcNode *dcNumberClass_lessThanOrEqual(dcNode *_receiver, dcArray *_arguments)
{
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_receiver);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(dcArray_get(_arguments, 0));
    return dcSystem_convertBoolToNode
        (dcNumber_lessThanOrEqual(leftAux->number, rightAux->number));
}

//////////////////////////////////////////////
// Number#>
//
// Checks if the receiver is greater than
// the argument
//
// Example
//
//   a = 2.000000000000000000000001
//   b = 2
//   a > b
//   ==> yes
//
//////////////////////////////////////////////
dcNode *dcNumberClass_greaterThan(dcNode *_receiver, dcArray *_arguments)
{
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_receiver);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(dcArray_get(_arguments, 0));
    return dcSystem_convertBoolToNode
        (dcNumber_greaterThan(leftAux->number, rightAux->number));
}

//////////////////////////////////////////////////////
//
// Number#>=
//
// Checks if the receiver is greater than or equal to
// the argument
//
// Example
//
//   a = 3
//   b = 2
//   a >= b
//
//   ==> yes
//
//////////////////////////////////////////////////////
dcNode *dcNumberClass_greaterThanOrEqual(dcNode *_receiver, dcArray *_arguments)
{
    dcNumberClassAux *leftAux = CAST_NUMBER_AUX(_receiver);
    dcNumberClassAux *rightAux = CAST_NUMBER_AUX(dcArray_get(_arguments, 0));
    return dcSystem_convertBoolToNode
        (dcNumber_greaterThanOrEqual(leftAux->number, rightAux->number));
}

dcNode *dcNumberClass_factorial(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNode_register(dcNode_copy(_receiver, DC_DEEP));
    dcNumber *number = dcNumberClass_getNumber(result);

    if (dcNumber_factorial(number, number) == TAFFY_NUMBER_NEED_INTEGER)
    {
        dcNeedIntegerExceptionClass_throwObject(result);
        result = NULL;
    }

    return result;
}

//////////////////////////////////////////////
//
// Number#++
//
// Returns the caller plus 1
//
// Example
//
//   a = 1
//   a++
//
//   ==> 2
//
//////////////////////////////////////////////
dcNode *dcNumberClass_plusPlus(dcNode *_receiver, dcArray *_arguments)
{
    dcNumber_increment(CAST_NUMBER_AUX(_receiver)->number);
    return _receiver;
}

//////////////////////////////////////////////
//
// Number#--
//
// Returns the number minus 1
//
// Example
//
//   a = 1
//   a--
//
//   ==> 0
//
//////////////////////////////////////////////
dcNode *dcNumberClass_minusMinus(dcNode *_receiver, dcArray *_arguments)
{
    dcNumber_decrement(CAST_NUMBER_AUX(_receiver)->number);
    return _receiver;
}

dcHashType dcNumberClass_hashHelper(dcNode *_receiver)
{
    dcNumber *number = CAST_NUMBER_AUX(_receiver)->number;
    dcHashType hashValue;
    dcError_assert(dcNumber_hash(number, &hashValue) == TAFFY_SUCCESS);
    return hashValue;
}

//////////////////////////////////////////////
//
// Number#hash
//
// Computes the hash of the caller
//
// If number = x, then hash = x * 2 + 1
//
// Example
//
//   a = 2
//   a hash
//   ==> 5
//
//////////////////////////////////////////////
dcNode *dcNumberClass_hash(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObjectFromInt64u
             (dcNumberClass_hashHelper(_receiver))));
}

///////////////////////////////
//
// Number#asString
//
// Returns self as a String
//
///////////////////////////////
dcNode *dcNumberClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    return dcNode_register
        (dcStringClass_createObject
         (dcNumber_display(CAST_NUMBER_AUX(_receiver)->number), false));
}

static dcNode *march(dcNode *_receiver,
                     dcArray *_arguments,
                     dcNumber_arithmeticFunction _marcher,
                     dcNumber_comparisonFunction _comparator)
{
    dcNumber *fromNumberIt =
        dcNumber_copy(dcNumberClass_getNumber(_receiver), DC_DEEP);
    dcNumber *upToNumber = dcNumberClass_getNumber(dcArray_get(_arguments, 0));
    dcNode *result = NULL;
    dcList *arguments = dcList_create();
    dcNode *procedure = dcClass_castNodeWithAssert(dcArray_get(_arguments, 1),
                                               dcProcedureClass_getTemplate(),
                                               false,
                                               true);
    dcList *procedureArguments =
        dcMethodHeader_getArguments
        (dcProcedureClass_getMethodHeader(procedure));
    uint32_t givenNumberArguments = (procedureArguments == NULL
                                     ? 0
                                     : procedureArguments->size);
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    const dcNumber *one = dcNumber_getConstant(1);

    if (givenNumberArguments == 1)
    {
        dcNodeEvaluator_startLoop(evaluator);

        while (((*_comparator)(fromNumberIt, upToNumber)
                || dcNumber_equals(fromNumberIt, upToNumber))
               && dcNodeEvaluator_canContinueEvaluating(evaluator))
        {
            dcNode *number =
                dcNode_register
                (dcNumberClass_createObject(dcNumber_copy(fromNumberIt,
                                                          DC_DEEP)));
            dcList_unshift(arguments, number);

            if (arguments->size > 1)
            {
                dcList_pop(arguments, DC_SHALLOW);
            }

            dcNodeEvaluator_pushMark(evaluator, number);

            result = dcNodeEvaluator_evaluateProcedure(evaluator,
                                                       _receiver,
                                                       procedure,
                                                       (SCOPE_DATA_BREAKTHROUGH
                                                        | SCOPE_DATA_CONST),
                                                       arguments);

            if (result == NULL
                || dcNodeEvaluator_isBreaking(evaluator))
            {
                dcNodeEvaluator_popMark(evaluator);
                dcNodeEvaluator_setBreaking(evaluator, false);
                break;
            }

            _marcher(fromNumberIt, fromNumberIt, one);
            dcNodeEvaluator_popMark(evaluator);
        }

        dcNodeEvaluator_stopLoop(evaluator);
    }
    else
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject
            (1, givenNumberArguments);
    }

    dcNumber_free(&fromNumberIt, DC_DEEP);
    dcList_free(&arguments, DC_SHALLOW);
    return result;
}

/////////////////////////////////////////////////
//
// Number#upTo:do:
//
// Evaluates a block over a given set of numbers
//
// Example
//
//  1 upTo: 2 do: { <val>
//    io put: "val is: #[val]"
//  }
//
//  ==> val is: 1
//      val is: 2
//
/////////////////////////////////////////////////
dcNode *dcNumberClass_upTo(dcNode *_receiver, dcArray *_arguments)
{
    return march(_receiver,
                 _arguments,
                 &dcNumber_add,
                 &dcNumber_lessThan);
}

/////////////////////////////////////////////////
//
// Number#squareRoot
//
// Computes the square root of a Number
//
/////////////////////////////////////////////////
dcNode *dcNumberClass_squareRoot(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNode_register(dcNode_copy(_receiver, DC_DEEP));
    dcNumber *number = dcNumberClass_getNumber(result);
    dcNumber_squareRoot(number, number);

    if (dcNumber_isWhole(number))
    {
        dcNumber_snip(number);
    }

    return result;
}

/////////////////////////////////////////////////
//
// Number#downTo:do:
//
// Evaluates a block over a given set of numbers
//
// Example
//
//   2 downTo: 1 do: { <val>
//     io put: "val is: #[val]"
//   }
//
//   ==> val is: 2
//       val is: 1
//
/////////////////////////////////////////////////
dcNode *dcNumberClass_downTo(dcNode *_receiver, dcArray *_arguments)
{
    return march(_receiver,
                 _arguments,
                 &dcNumber_subtract,
                 &dcNumber_greaterThan);
}

//////////////////////////////////////////////
// Number#getFractionalSize
//
// Gets the number of fractional digits of a number
//
// Example
//
//   a = 2.01
//   a getFractionalSize
//   ==> 2
//
//////////////////////////////////////////////
dcNode *dcNumberClass_getFractionalSize(dcNode *_receiver, dcArray *_arguments)
{
    dcNumber *number = dcNumberClass_getNumber(_receiver);
    uint32_t result = 0;

    if (number->type == NUMBER_DEC_NUMBER_TYPE)
    {
        result = abs(number->types.decNumber->exponent);
    }

    return dcNode_register(dcNumberClass_createObjectFromInt32u(result));
}

dcString *dcNumberClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcNumberClassAux *aux = CAST_NUMBER_AUX(_node);
    return dcNumber_marshall(aux->number, _stream);
}

bool dcNumberClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcNumber *number = dcNumber_unmarshall(_stream);

    if (number != NULL)
    {
        result = true;
        CAST_CLASS_AUX(_node) = createAux(number);
    }

    return result;
}

//
// dcNumberClass_isNegative
//
// Returns true if the given Number object is negative
// Returns false otherwise
//
bool dcNumberClass_isNegative(const dcNode *_number)
{
    dcNumber *number = dcNumberClass_getNumber(_number);
    return (! dcNumber_isNaN(number)
            && dcNumber_lessThan(number, dcNumber_getConstant(0)));
}

bool dcNumberClass_isPositive(const struct dcNode_t *_number)
{
    dcNumber *number = dcNumberClass_getNumber(_number);
    return (! dcNumber_isNaN(number)
            && dcNumber_isPositive(number));
}

bool dcNumberClass_divides(dcNode *_left, dcNode *_right)
{
    dcNumber *left = dcNumberClass_getNumber(_left);
    dcNumber *right = dcNumberClass_getNumber(_right);

    if (! dcNumber_isWhole(left)
        || ! dcNumber_isWhole(right))
    {
        return false;
    }

    dcNumber *result = dcNumber_createFromInt32u(0);

    dcNumber_divide(result, right, left);

    bool realResult = dcNumber_isWhole(result);
    dcNumber_free(&result, DC_DEEP);
    return realResult;
}

dcTaffy_createIsMeFunctionMacro(dcNumberClass);

dcNode *dcNumberClass_negateHelper(dcNode *_number)
{
    dcNumberClass_inlineMultiply(_number,
                                 dcNumberClass_getNegativeOneNumberObject());
    return _number;
}

dcNode *dcNumberClass_factorPairs(dcNode *_node, dcArray *_arguments)
{
    dcList *factorPairs = NULL;
    dcNode *result = NULL;

    if (! dcNumberClass_isWholeHelper(_node))
    {
        dcNeedIntegerExceptionClass_throwObject(_node);
        return NULL;
    }

    factorPairs = dcNumber_getFactorPairs(dcNumberClass_getNumber(_node), true);

    if (factorPairs == NULL)
    {
        dcNeedIntegerExceptionClass_throwObject(_node);
        return NULL;
    }

    result = (dcNode_register
              (dcArrayClass_createObject
               (dcArray_createFromList(factorPairs, DC_SHALLOW),
                true)));
    dcList_free(&factorPairs, DC_SHALLOW);
    return result;
}

dcNode *dcNumberClass_factors(dcNode *_node, dcArray *_arguments)
{
    dcList *factors = NULL;
    dcNode *result = NULL;

    if (! dcNumberClass_isWholeHelper(_node))
    {
        dcNeedIntegerExceptionClass_throwObject(_node);
        return NULL;
    }

    factors = dcNumber_getFactors(dcNumberClass_getNumber(_node), true);

    if (factors == NULL)
    {
        dcNeedIntegerExceptionClass_throwObject(_node);
        return NULL;
    }

    result = (dcNode_register
              (dcArrayClass_createObject
               (dcArray_createFromList(factors, DC_SHALLOW),
                true)));
    dcList_free(&factors, DC_SHALLOW);
    return result;
}

bool dcNumberClass_verifyInteger(dcNode *_node)
{
    if (! dcNumberClass_isWholeHelper(_node))
    {
        dcNeedIntegerExceptionClass_throwObject(_node);
        return false;
    }

    return true;
}

dcNode *dcNumberClass_convertToInteger(dcNode *_value)
{
    dcNumber_tryToConvertToInteger(dcNumberClass_getNumber(_value));
    return _value;
}

bool dcNumberClass_isOneFractional(dcNode *_value)
{
    return ((dcNumberClass_greaterThanHelper
             (_value,
              dcNumberClass_getNegativeOneNumberObject()))
            && (dcNumberClass_lessThanHelper
                (_value,
                 dcNumberClass_getOneNumberObject())));
}
