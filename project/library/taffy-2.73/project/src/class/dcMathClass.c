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

#include "CompiledMath.h"
#include "CompiledMathConstants.h"

#include "dcMathClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcComplexNumber.h"
#include "dcComplexNumberClass.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcGraphData.h"
#include "dcKernelClass.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcNumberClass.h"
#include "dcObjectClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "atan:",
        (SCOPE_DATA_PUBLIC | SCOPE_DATA_CONST),
        &dcMathClass_atan,
        gCFunctionArgument_number
    },
    {
        "ln:",
        (SCOPE_DATA_PUBLIC | SCOPE_DATA_CONST),
        &dcMathClass_ln,
        gCFunctionArgument_number
    },
    {
        "lg:",
        (SCOPE_DATA_PUBLIC | SCOPE_DATA_CONST),
        &dcMathClass_lg,
        gCFunctionArgument_number
    },
    {
        "log10:",
        (SCOPE_DATA_PUBLIC | SCOPE_DATA_CONST),
        &dcMathClass_log10,
        gCFunctionArgument_number
    },
    {
        "nAn",
        (SCOPE_DATA_PUBLIC | SCOPE_DATA_CONST),
        &dcMathClass_nAn,
        gCFunctionArgument_none
    },
    {
        "random",
        (SCOPE_DATA_PUBLIC | SCOPE_DATA_CONST),
        &dcMathClass_random,
        gCFunctionArgument_none
    },
    {
        "sin:",
        (SCOPE_DATA_PUBLIC | SCOPE_DATA_CONST),
        &dcMathClass_sin,
        gCFunctionArgument_number
    },
    {
        0
    }
};

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcMathClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (MATH_PACKAGE_NAME,                  // package name
          MATH_CLASS_NAME,                    // class name
          MAKE_FULLY_QUALIFIED(OBJECT),       // super name
          CLASS_SINGLETON,                    // class flags
          NO_FLAGS,                           // scope data flags
          NULL,                               // meta methods
          sMethodWrappers,                    // methods
          &dcMathClass_initialize,            // initialization function
          &dcMathClass_deinitialize,          // deinitialization function
          NULL,                               // allocate
          NULL,                               // deallocate
          NULL,                               // meta mark
          NULL,                               // mark
          NULL,                               // copy
          NULL,                               // free
          NULL,                               // register
          NULL,                               // marshall
          NULL,                               // unmarshall
          NULL));                             // set template
};

// maintained by dcMathClass.c
static uint32_t sCoefficientsSize = 0;
static dcNumber **sSinCoefficients = NULL;
static dcNumber **sSinExponents = NULL;
static dcComplexNumber **sSinComplexCoefficients = NULL;
static dcComplexNumber **sSinComplexExponents = NULL;
static bool sFastSin = false;
static dcNumber *sTwoPi = NULL;
static dcNode *sTheMath = NULL;

dcNode *dcMathClass_createNode(bool _object)
{
    return dcClass_createBasicNode(sTemplate, _object);
}

dcNode *dcMathClass_createObject(void)
{
    return dcMathClass_createNode(true);
}

#define MATH_CONSTANTS_TAFFY_FILE_NAME "src/class/MathConstants.ty"
#define MATH_CONSTANTS2_TAFFY_FILE_NAME "src/class/MathConstants2.ty"
#define MATH_CLASS_TAFFY_FILE_NAME "src/class/Math.ty"
#define MATH_CLASS_FILE_NAME "src/class/dcMathClass.c"

void dcMathClass_initialize(void)
{
    // evaluate the rest of Math class from Math.ty //
    dcError_assert(dcStringEvaluator_evalString(__compiledMath,
                                                MATH_CLASS_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);

    // evaluate the constants from MathConstants.ty //
    dcError_assert(dcStringEvaluator_evalString(__compiledMathConstants,
                                                MATH_CONSTANTS_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);

    sTheMath = dcNodeEvaluator_findObject(dcSystem_getCurrentNodeEvaluator(),
                                          "math",
                                          true);
    assert(sTheMath != NULL);

    dcNode *mathObject = dcStringEvaluator_evalString
        ("math initialize", MATH_CLASS_FILE_NAME, NO_STRING_EVALUATOR_FLAGS);
    dcError_assert(mathObject != NULL);
    dcArray *templateCoefficients = dcArrayClass_getObjects
        (dcClass_getObject(mathObject, "@sinCoefficients"));
    dcArray *templateExponents = dcArrayClass_getObjects
        (dcClass_getObject(mathObject, "@sinExponents"));

    dcNode *pi = dcStringEvaluator_evalString
        ("PI", MATH_CLASS_FILE_NAME, STRING_EVALUATOR_ASSERT_NO_EXCEPTION);

    sTwoPi = dcNumber_createFromInt32u(0);

    dcNumber *two = dcNumber_createFromInt32u(2);
    dcNumber_multiply(sTwoPi, dcNumberClass_getNumber(pi), two);
    dcNumber_free(&two, DC_DEEP);

    sCoefficientsSize = templateCoefficients->size;
    sSinCoefficients = (dcNumber **)(dcMemory_allocate
                                     (sizeof(dcNumber *)
                                      * templateCoefficients->size));
    sSinExponents = (dcNumber **)(dcMemory_allocate
                                  (sizeof(dcNumber *)
                                   * templateCoefficients->size));
    sSinComplexCoefficients =
        (dcComplexNumber **)(dcMemory_allocate
                             (sizeof(dcComplexNumber *)
                              * templateCoefficients->size));
    sSinComplexExponents =
        (dcComplexNumber **)dcMemory_allocate(sizeof(dcComplexNumber *)
                                              * templateCoefficients->size);

    uint32_t a;

    for (a = 0; a < sCoefficientsSize; a++)
    {
        sSinCoefficients[a] = (dcNumber_copy
                               (dcNumberClass_getNumber
                                (templateCoefficients->objects[a]),
                                DC_DEEP));
        sSinExponents[a] = (dcNumber_copy
                            (dcNumberClass_getNumber
                             (templateExponents->objects[a]),
                             DC_DEEP));
        sSinComplexCoefficients[a] = (dcComplexNumber_create
                                      (dcNumber_copy
                                       (dcNumberClass_getNumber
                                        (templateCoefficients->objects[a]),
                                        DC_DEEP),
                                       dcNumber_createFromInt32u(0)));
        sSinComplexExponents[a] = (dcComplexNumber_create
                                   (dcNumber_copy
                                    (dcNumberClass_getNumber
                                     (templateExponents->objects[a]),
                                     DC_DEEP),
                                    dcNumber_createFromInt32u(0)));
    }

    dcClassManager_registerSingleton(sTheMath, "math");
}

void dcMathClass_deinitialize(void)
{
    uint32_t a;

    for (a = 0; a < sCoefficientsSize; a++)
    {
        dcNumber_free(&sSinCoefficients[a], DC_DEEP);
        dcNumber_free(&sSinExponents[a], DC_DEEP);
        dcComplexNumber_free(&sSinComplexCoefficients[a], DC_DEEP);
        dcComplexNumber_free(&sSinComplexExponents[a], DC_DEEP);
    }

    dcNumber_free(&sTwoPi, DC_DEEP);
    dcMemory_free(sSinCoefficients);
    dcMemory_free(sSinExponents);
    dcMemory_free(sSinComplexCoefficients);
    dcMemory_free(sSinComplexExponents);
}

static dcNode *realSingleOperandOperationHelper
    (dcNumber_singleOperandFunction _operation,
     dcNode *_object)
{
    dcNumber *number = dcNumberClass_getNumber(_object);
    dcNumber *numberResult =
        dcNumber_createFromInt32uWithLsuSize
        (0,
         dcNumberMetaClass_getDigitLimit());

    _operation(numberResult, number);

    return dcNode_register(dcNumberClass_createObject(numberResult));
}

static dcNode *singleOperandOperationHelper
    (dcNumber_singleOperandFunction _operation,
     dcArray *_arguments)
{
    dcNode *numberNode = dcArray_get(_arguments, 0);
    return realSingleOperandOperationHelper(_operation, numberNode);
}

// taffy methods //
dcNode *dcMathClass_ln(dcNode *_receiver, dcArray *_arguments)
{
    return singleOperandOperationHelper(&dcNumber_ln, _arguments);
}

dcNode *dcMathClass_lg(dcNode *_receiver, dcArray *_arguments)
{
    return singleOperandOperationHelper(&dcNumber_lg, _arguments);
}

dcNode *dcMathClass_log10(dcNode *_receiver, dcArray *_arguments)
{
    return singleOperandOperationHelper(&dcNumber_log10, _arguments);
}

dcNode *dcMathClass_random(dcNode *_receiver, dcArray *_arguments)
{
    dcNumber *result = dcNumber_createFromInt32u(0);
    dcNumber_random(result);
    return dcNode_register(dcNumberClass_createObject(result));
}

dcNode *dcMathClass_nAn(dcNode *_receiver, dcArray *_arguments)
{
    // force decNumber
    return dcNode_register(dcNumberClass_createObject(dcNumber_getNaN()));
}

dcNode *dcMathClass_sin(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;

    if (sFastSin)
    {
        dcNumber *number = dcNumberClass_getNumber(dcArray_get(_arguments, 0));

        if (number->type == NUMBER_INTEGER_TYPE)
        {
            result = dcNode_register
                (dcNumberClass_createObjectFromDouble
                 (sin((double)number->types.integer)));
        }
        else
        {
            dcNumber *modded = dcNumber_createFromDouble(0);
            dcNumber_modulus(modded, number, sTwoPi);

            double doubleValue;
            dcError_assert(dcNumber_extractDouble(modded, &doubleValue));
            result = dcNode_register
                (dcNumberClass_createObjectFromDouble(sin(doubleValue)));
            dcNumber_free(&modded, DC_DEEP);
        }
    }
    else
    {
        dcNumber *numberResult = dcNumber_createFromInt32u(0);
        const dcNumber *rightNumber =
            dcNumberClass_getNumber(dcArray_get(_arguments, 0));
        uint32_t a;

        for (a = 0; a < sCoefficientsSize; a++)
        {
            dcNumber *tempResult = dcNumber_copy(rightNumber, DC_DEEP);
            dcNumber_raise(tempResult, tempResult, sSinExponents[a]);
            dcNumber_multiply(tempResult, sSinCoefficients[a], tempResult);
            dcNumber_add(numberResult, numberResult, tempResult);
            dcNumber_free(&tempResult, DC_DEEP);
        }

        dcNumber_round(numberResult);
        result = (dcNode_register
                  (dcNumberClass_createObject(dcNumber_snip(numberResult))));
    }

    return (result);
}

//
// Inverse Tangent algorithm, from Wolfram Mathworld
//
// a0 = (1 + x^2)^(-0.5)
// b0 = 1
//
// a(i + 1) = 0.5 * (a(i) + b(i))
// b(i + 1) = sqrt(a(i + 1) * b(i))
//
dcNode *dcMathClass_atan(dcNode *_receiver, dcArray *_arguments)
{
    const dcNumber *one = dcNumber_getConstant(1);
    const dcNumber *two = dcNumber_getConstant(2);
    dcNumber *a = dcNumber_copy
        (dcNumberClass_getNumber(dcArray_get(_arguments, 0)), DC_DEEP);
    dcNumber_raise(a, a, two);
    dcNumber_add(a, a, one);
    dcNumber_raise(a, a, dcNumber_getNegativeOneHalf());
    dcNumber *b = dcNumber_copy(one, DC_DEEP);
    const dcNumber *oneHalf = dcNumber_getOneHalf();
    int i;

    // 30 is a nice round number
    for (i = 0; i < 30; i++)
    {
        dcNumber_add(a, a, b);
        dcNumber_multiply(a, a, oneHalf);
        dcNumber_multiply(b, b, a);
        dcNumber_squareRoot(b, b);
    }

    dcNumber *value = dcNumber_copy
        (dcNumberClass_getNumber(dcArray_get(_arguments, 0)), DC_DEEP);
    dcNumber *bottom = dcNumber_copy(value, DC_DEEP);
    dcNumber_raise(bottom, bottom, two);
    dcNumber_add(bottom, bottom, one);
    dcNumber_squareRoot(bottom, bottom);
    dcNumber_multiply(bottom, bottom, a);
    dcNumber_divide(value, value, bottom);
    dcNumber_free(&bottom, DC_DEEP);
    dcNumber_free(&b, DC_DEEP);
    dcNumber_free(&a, DC_DEEP);

    return dcNode_register(dcNumberClass_createObject(value));
}
