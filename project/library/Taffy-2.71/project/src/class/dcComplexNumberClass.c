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

#include "CompiledComplexNumber.h"

#include "dcNumberClass.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcComplexNumber.h"
#include "dcComplexNumberClass.h"
#include "dcContainers.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcExceptions.h"
#include "dcFunctionClass.h"
#include "dcGraphData.h"
#include "dcMatrixClass.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
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
#include "dcYesClass.h"

#include "decNumber.h"

TAFFY_C_METHOD(dcComplexNumberMetaClass_createFromNumber);

TAFFY_C_METHOD(dcComplexNumberClass_add);
TAFFY_C_METHOD(dcComplexNumberClass_addEquals);
TAFFY_C_METHOD(dcComplexNumberClass_conjugate);
TAFFY_C_METHOD(dcComplexNumberClass_conjugateBang);
TAFFY_C_METHOD(dcComplexNumberClass_deltaEquals);
TAFFY_C_METHOD(dcComplexNumberClass_digitLimit);
TAFFY_C_METHOD(dcComplexNumberClass_divide);
TAFFY_C_METHOD(dcComplexNumberClass_divideEquals);
TAFFY_C_METHOD(dcComplexNumberClass_hash);
TAFFY_C_METHOD(dcComplexNumberClass_imaginary);
TAFFY_C_METHOD(dcComplexNumberClass_length);
TAFFY_C_METHOD(dcComplexNumberClass_modulus);
TAFFY_C_METHOD(dcComplexNumberClass_multiply);
TAFFY_C_METHOD(dcComplexNumberClass_multiplyEquals);
TAFFY_C_METHOD(dcComplexNumberClass_raise);
TAFFY_C_METHOD(dcComplexNumberClass_real);
TAFFY_C_METHOD(dcComplexNumberClass_setReal);
TAFFY_C_METHOD(dcComplexNumberClass_setImaginary);
TAFFY_C_METHOD(dcComplexNumberClass_setValue);
TAFFY_C_METHOD(dcComplexNumberClass_subtract);
TAFFY_C_METHOD(dcComplexNumberClass_subtractEquals);
TAFFY_C_METHOD(dcComplexNumberClass_trulyEquals);

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "createFromNumber:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberMetaClass_createFromNumber,
        gCFunctionArgument_number
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_asString,
        gCFunctionArgument_none
    },
    {
        "#operator(+):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_add,
        gCFunctionArgument_wild
    },
    {
        "#operator(+=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcComplexNumberClass_addEquals,
        gCFunctionArgument_wild
    },
    {
        "deltaEquals:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_deltaEquals,
        gCFunctionArgument_wild
    },
    {
        "digitLimit",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_digitLimit,
        gCFunctionArgument_none
    },
    {
        "#operator(/):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_divide,
        gCFunctionArgument_wild
    },
    {
        "#operator(/=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcComplexNumberClass_divideEquals,
        gCFunctionArgument_wild
    },
    {
        "conjugate",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_conjugate,
        gCFunctionArgument_none
    },
    {
        "conjugate!",
        SCOPE_DATA_PUBLIC,
        &dcComplexNumberClass_conjugateBang,
        gCFunctionArgument_none
    },
    {
        "hash",
        SCOPE_DATA_PUBLIC,
        &dcComplexNumberClass_hash,
        gCFunctionArgument_none
    },
    {
        "imaginary",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_imaginary,
        gCFunctionArgument_none
    },
    {
        "length",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_length,
        gCFunctionArgument_none
    },
    {
        "modulus",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_modulus,
        gCFunctionArgument_none
    },
    {
        "#operator(*):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_multiply,
        gCFunctionArgument_wild
    },
    {
        "#operator(*=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcComplexNumberClass_multiplyEquals,
        gCFunctionArgument_wild
    },
    {
        "real",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_real,
        gCFunctionArgument_none
    },
    {
        "trulyEquals:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_trulyEquals,
        gCFunctionArgument_wild
    },
    {
        "setReal:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED),
        &dcComplexNumberClass_setReal,
        gCFunctionArgument_number
    },
    {
        "setImaginary:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED),
        &dcComplexNumberClass_setImaginary,
        gCFunctionArgument_number
    },
    {
        "setValue:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED),
        &dcComplexNumberClass_setValue,
        gCFunctionArgument_complexNumber
    },
    {
        "#operator(-):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcComplexNumberClass_subtract,
        gCFunctionArgument_wild
    },
    {
        "#operator(-=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcComplexNumberClass_subtractEquals,
        gCFunctionArgument_wild
    },
    {
        0
    }
};

#define CAST_COMPLEX_NUMBER_AUX(node)                   \
    ((dcComplexNumberClassAux*)(CAST_CLASS_AUX(node)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcComplexNumberClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (COMPLEX_NUMBER_PACKAGE_NAME,              // package name
          COMPLEX_NUMBER_CLASS_NAME,                // class name
          MAKE_FULLY_QUALIFIED(OBJECT),             // super name
          CLASS_ATOMIC,                             // class flags
          NO_FLAGS,                                 // scope data flags
          sMetaMethodWrappers,                      // meta methods
          sMethodWrappers,                          // methods
          &dcComplexNumberClass_initialize,         // initialization function
          NULL,                                     // deinitialization function
          &dcComplexNumberClass_allocateNode,       // allocate
          NULL,                                     // deallocate
          NULL,                                     // meta mark
          NULL,                                     // mark
          &dcComplexNumberClass_copyNode,           // copy
          &dcComplexNumberClass_freeNode,           // free
          NULL,                                     // register
          &dcComplexNumberClass_marshallNode,       // marshall
          &dcComplexNumberClass_unmarshallNode,     // unmarshall
          NULL));                                   // set template
};

static dcComplexNumberClassAux *createAux(dcComplexNumber *_number)
{
    dcError_assert(_number != NULL);
    dcComplexNumberClassAux *aux =
        (dcComplexNumberClassAux *)dcMemory_allocate
        (sizeof(dcComplexNumberClassAux));
    aux->number = _number;
    return aux;
}

void dcComplexNumberClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) =
        createAux(dcComplexNumber_create(dcNumber_createFromInt32u(0),
                                         dcNumber_createFromInt32u(0)));
}

#define COMPLEX_NUMBER_TAFFY_FILE_NAME "src/class/ComplexNumber.ty"

void dcComplexNumberClass_initialize(void)
{
    dcError_assert(dcStringEvaluator_evalString(__compiledComplexNumber,
                                                COMPLEX_NUMBER_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

void dcComplexNumberClass_copyNode(dcNode *_to,
                                   const dcNode *_from,
                                   dcDepth _depth)
{
    CAST_CLASS_AUX(_to) =
        createAux(dcComplexNumber_copy
                  (CAST_COMPLEX_NUMBER_AUX(_from)->number, DC_DEEP));
}

void dcComplexNumberClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcComplexNumberClassAux *aux = CAST_COMPLEX_NUMBER_AUX(_node);
    dcComplexNumber_free(&aux->number, DC_DEEP);
    dcMemory_free(aux);
}

//
// Create and initialize a Number class
//
dcNode *dcComplexNumberClass_createNode(dcComplexNumberClassAux *_aux,
                                        bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL, // super
                              NULL, // scope
                              _object,
                              _aux);
}

dcNode *dcComplexNumberClass_createObject(dcComplexNumber *_number)
{
    dcComplexNumberClassAux *aux = createAux(_number);
    return dcComplexNumberClass_createNode(aux, true);
}


dcNode *dcComplexNumberClass_createObjectFromNumbers(dcNumber *_real,
                                                     dcNumber *_imaginary)
{
    return (dcComplexNumberClass_createObject
            (dcComplexNumber_create
             (_real,
              _imaginary)));
}

dcComplexNumber *dcComplexNumberClass_getNumber(const dcNode *_numberObject)
{
    dcComplexNumberClassAux *aux = CAST_COMPLEX_NUMBER_AUX(_numberObject);
    return aux->number;
}

static dcNode *inlineOperation(dcNode *_left,
                               dcNode *_right,
                               dcComplexNumber_arithmeticFunction _operation)
{
    dcComplexNumber *left = dcComplexNumberClass_getNumber(_left);
    _operation(left, left, dcComplexNumberClass_getNumber(_right));
    return _left;
}

dcNode *dcComplexNumberClass_inlineAdd(dcNode *_left, dcNode *_right)
{
    return inlineOperation(_left, _right, &dcComplexNumber_add);
}

dcNode *dcComplexNumberClass_inlineSubtract(dcNode *_left, dcNode *_right)
{
    return inlineOperation(_left, _right, &dcComplexNumber_subtract);
}

dcNode *dcComplexNumberClass_inlineMultiply(dcNode *_left, dcNode *_right)
{
    return inlineOperation(_left, _right, &dcComplexNumber_multiply);
}

dcNode *dcComplexNumberClass_inlineDivide(dcNode *_left, dcNode *_right)
{
    return inlineOperation(_left, _right, &dcComplexNumber_divide);
}

dcNode *dcComplexNumberClass_inlineRaise(dcNode *_left, dcNode *_right)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *result = (dcNodeEvaluator_callMethodWithArgument
                      (evaluator,
                       _left,
                       "#operator(^):",
                       _right));
    TAFFY_DEBUG(assert(evaluator->exception == NULL));

    if (dcComplexNumberClass_isMe(result)
        && dcComplexNumberClass_isWhole(result))
    {
        dcComplexNumberClass_chomp(result);
    }

    return result;
}

static dcNode *performTwoOperandsOperation(dcNode *_left,
                                           dcNode *_candidate,
                                           dcTaffyOperator _operation,
                                           bool _isInline)
{
    dcNode *right = NULL;
    dcNode *result = NULL;
    dcComplexNumber_arithmeticFunction operation =
        dcComplexNumber_getArithmeticOperation(_operation);
    dcComplexNumber *left = dcComplexNumberClass_getNumber(_left);
    dcComplexNumber *complexy = NULL;
    bool exception = false;

    if ((right = dcClass_castNode(_candidate, sTemplate, false))
        != NULL)
    {
        complexy = (_isInline
                    ? left
                    : dcComplexNumber_create(NULL, NULL));
        operation(complexy,
                  left,
                  dcComplexNumberClass_getNumber(right));
    }
    else if (! _isInline
             && ((right = dcClass_castNode(_candidate,
                                           dcMatrixClass_getTemplate(),
                                           false))
                 != NULL))
    {
        result = dcMatrixClass_matrixAndNumberOperation
            (right, _left, _operation, false);
    }
    else if (! _isInline
             && ((right = dcClass_castNode(_candidate,
                                           dcFunctionClass_getTemplate(),
                                           false))
                 != NULL))
    {
        result = dcFunctionClass_numberOperation
            (right, _left, _operation, false);
    }
    else if ((right = dcClass_castNode(_candidate,
                                       dcMatrixClass_getTemplate(),
                                       false))
             != NULL)
    {
        result = dcMatrixClass_matrixAndNumberOperation
            (right, _left, _operation, true);
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
                                       dcNumberClass_getTemplate(),
                                       false))
             != NULL)
    {
        complexy = (_isInline
                    ? left
                    : dcComplexNumber_create(NULL, NULL));
        dcComplexNumber *rightComplexy = dcComplexNumber_create
            (dcNumber_copy(dcNumberClass_getNumber(right), DC_DEEP),
             NULL);
        operation(complexy, left, rightComplexy);
        dcComplexNumber_free(&rightComplexy, DC_DEEP);
    }
    else
    {
        exception = true;
        dcInvalidCastExceptionClass_throwObject
            (dcClass_getName(_candidate), "(ComplexNumber or Number)");
    }

    if (! exception && complexy != NULL)
    {
        if (_isInline)
        {
            result = _left;
        }
        else
        {
            // x + 0i ==> x
            if (dcNumber_equalsInt32u(complexy->imaginary, 0))
            {
                result = (dcNumberClass_createObject
                          (dcNumber_copy(complexy->real, DC_DEEP)));
                dcComplexNumber_free(&complexy, DC_DEEP);
            }
            else
            {
                result = dcComplexNumberClass_createObject(complexy);
            }
        }
    }

    dcNode_register(result);
    return result;
}

dcNode *dcComplexNumberClass_addHelper(dcNode *_left, dcNode *_right)
{
    return performTwoOperandsOperation(_left, _right, TAFFY_ADD, false);
}

dcNode *dcComplexNumberClass_subtractHelper(dcNode *_left, dcNode *_right)
{
    return performTwoOperandsOperation(_left, _right, TAFFY_SUBTRACT, false);
}

dcNode *dcComplexNumberClass_multiplyHelper(dcNode *_left, dcNode *_right)
{
    return performTwoOperandsOperation(_left, _right, TAFFY_MULTIPLY, false);
}

dcNode *dcComplexNumberClass_divideHelper(dcNode *_left, dcNode *_right)
{
    return performTwoOperandsOperation(_left, _right, TAFFY_DIVIDE, false);
}

dcNode *dcComplexNumberMetaClass_createFromNumber(dcNode *_receiver,
                                                  dcArray *_arguments)
{
    return (dcNode_register
            (dcComplexNumberClass_createObject
             (dcComplexNumber_create
              (dcNumber_copy
               (dcNumberClass_getNumber
                (dcArray_get(_arguments, 0)),
                DC_DEEP),
               NULL))));
}

///////////////////////////////////////////////////////
//
// ComplexNumber#+(prefix)
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
dcNode *dcComplexNumberClass_antiNegate(dcNode *_receiver, dcArray *_arguments)
{
    return _receiver;
}

bool dcComplexNumberClass_equalsNumber(dcNode *_left, dcNode *_right)
{
    return dcComplexNumber_equals(dcComplexNumberClass_getNumber(_left),
                                  dcComplexNumberClass_getNumber(_right));
}

//////////////////////////////////////////
//
// ComplexNumber#operator(+):
//
// Returns the sum of two ComplexNumber objects
//
// Example
//
//   a = 1 + 2i
//   b = 2 + 3i
//   a + b
//   ==> 3 + 5i
//
//////////////////////////////////////////
dcNode *dcComplexNumberClass_add(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_ADD,
                                       false);
}

//////////////////////////////////////////
//
// ComplexNumber#/
//
// Returns the quotient of the division of two ComplexNumber objects
//
// Example
//
//   a = 5 + 2i
//   b = 7 + 4i
//   a / b
//   ==> (43 - 6i) / 65
//
//////////////////////////////////////////
dcNode *dcComplexNumberClass_divide(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_DIVIDE,
                                       false);
}

//////////////////////////////////////////
//
// ComplexNumber#digitLimit
//
// Returns the max digit limit
//
//////////////////////////////////////////
dcNode *dcComplexNumberClass_digitLimit(dcNode *_receiver, dcArray *_arguments)
{
    dcComplexNumber *number = dcComplexNumberClass_getNumber(_receiver);
    return dcNode_register(dcNumberClass_createObjectFromInt32u
                           (dcTaffy_max
                            (dcNumber_getDigitLimit(number->real),
                             dcNumber_getDigitLimit(number->imaginary))));
}

dcNode *dcComplexNumberClass_addEquals(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_ADD,
                                       true);
}

dcNode *dcComplexNumberClass_divideEquals(dcNode *_receiver,
                                          dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_DIVIDE,
                                       true);
}

///////////////////////////////////////
//
// ComplexNumber#*
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
dcNode *dcComplexNumberClass_multiply(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_MULTIPLY,
                                       false);
}

dcNode *dcComplexNumberClass_multiplyEquals(dcNode *_receiver,
                                            dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_MULTIPLY,
                                       true);
}

//////////////////////////////////////////////////
//
// ComplexNumber#-
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
dcNode *dcComplexNumberClass_subtract(dcNode *_receiver, dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_SUBTRACT,
                                       false);
}

dcNode *dcComplexNumberClass_subtractEquals(dcNode *_receiver,
                                            dcArray *_arguments)
{
    return performTwoOperandsOperation(_receiver,
                                       dcArray_get(_arguments, 0),
                                       TAFFY_SUBTRACT,
                                       true);
}

//////////////////////////////////////////////
//
// ComplexNumber#hash
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
dcNode *dcComplexNumberClass_hash(dcNode *_receiver, dcArray *_arguments)
{
    dcHashType hashConversionValue = 139707145;
    dcComplexNumber *number = dcComplexNumberClass_getNumber(_receiver);
    dcHashType realHashValue = 0;
    dcHashType imaginaryHashValue = 0;
    dcError_assert(dcNumber_hash(number->real, &realHashValue)
                   == TAFFY_SUCCESS);
    realHashValue *= hashConversionValue;
    dcError_assert(dcNumber_hash(number->imaginary, &imaginaryHashValue)
                   == TAFFY_SUCCESS);
    imaginaryHashValue *= hashConversionValue;
    return (dcNode_register
            (dcNumberClass_createObjectFromInt64u(realHashValue
                                                  + imaginaryHashValue)));
}

//////////////////////////////////////////
//
// Number#trulyEquals
//
// Returns the equality of two ComplexNumber objects
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
dcNode *dcComplexNumberClass_trulyEquals(dcNode *_receiver,
                                         dcArray *_arguments)
{
    bool equals = false;
    dcNode *right = dcClass_castNode(dcArray_get(_arguments, 0),
                                     sTemplate,
                                     false);
    if (right != NULL)
    {
        equals = (dcComplexNumber_equals
                  (dcComplexNumberClass_getNumber(_receiver),
                   dcComplexNumberClass_getNumber(right)));
    }

    return dcSystem_convertBoolToNode(equals);
}

// TODO: Factor with dcNumberClass_deltaEquals
dcNode *dcComplexNumberClass_deltaEquals(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();
    dcArray *arguments = dcArrayClass_getObjects(dcArray_get(_arguments, 0));
    int32_t precision = 0;
    dcComplexNumber *left = dcComplexNumberClass_getNumber(_receiver);
    dcComplexNumber *right = NULL;
    dcNode *candidate = dcClass_castNode(dcArray_get(arguments, 0),
                                         sTemplate,
                                         false);

    if (candidate != NULL)
    {
        right = dcComplexNumberClass_getNumber(candidate);
        dcNode *precisionNode = dcClass_castNode(dcArray_get(arguments, 1),
                                                 dcNumberClass_getTemplate(),
                                                 false);

        if (precisionNode != NULL)
        {
            if (dcNumberClass_extractInt32s(precisionNode, &precision))
            {
                if (dcNumber_deltaEqual(left->real, right->real, precision)
                    && dcNumber_deltaEqual(left->imaginary,
                                           right->imaginary,
                                           precision))
                {
                    result = dcYesClass_getInstance();
                }
            }
            else
            {
                result = NULL;
                dcNeedIntegerExceptionClass_throwObject(precisionNode);
            }
        }
        // else result is no
    }
    // else result is no

    return result;
}

///////////////////////////////
//
// ComplexNumber#asString
//
// Returns self as a String
//
///////////////////////////////
dcNode *dcComplexNumberClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    return dcNode_register
        (dcStringClass_createObject
         (dcComplexNumber_display(dcComplexNumberClass_getNumber(_receiver)),
          false));
}

dcNode *dcComplexNumberClass_conjugate(dcNode *_receiver, dcArray *_arguments)
{
    dcComplexNumber *newwie =
        dcComplexNumber_copy(dcComplexNumberClass_getNumber(_receiver),
                             DC_DEEP);
    dcComplexNumber_conjugate(newwie, newwie);
    return dcNode_register(dcComplexNumberClass_createObject(newwie));
}

dcNode *dcComplexNumberClass_conjugateBang(dcNode *_receiver,
                                           dcArray *_arguments)
{
    dcComplexNumber *me = dcComplexNumberClass_getNumber(_receiver);
    dcComplexNumber_conjugate(me, me);
    return _receiver;
}

dcString *dcComplexNumberClass_marshallNode(const dcNode *_node,
                                            dcString *_stream)
{
    dcComplexNumberClassAux *aux = CAST_COMPLEX_NUMBER_AUX(_node);
    return dcComplexNumber_marshall(aux->number, _stream);
}

bool dcComplexNumberClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcComplexNumber *number = dcComplexNumber_unmarshall(_stream);

    if (number != NULL)
    {
        result = true;
        CAST_CLASS_AUX(_node) = createAux(number);
    }

    return result;
}

bool dcComplexNumberClass_isMe(const dcNode *_node)
{
    return (IS_CLASS(_node)
            && dcClass_hasTemplate(_node, sTemplate, true));
}

bool dcComplexNumberClass_isWhole(const dcNode *_node)
{
    dcComplexNumber *number = dcComplexNumberClass_getNumber(_node);
    return (dcNumber_isWhole(number->real)
            && dcNumber_isWhole(number->imaginary));
}

void dcComplexNumberClass_chomp(const dcNode *_node)
{
    dcComplexNumber *number = dcComplexNumberClass_getNumber(_node);
    dcNumber_chomp(number->real, number->real);
    dcNumber_chomp(number->imaginary, number->imaginary);
}

dcNode *dcComplexNumberClass_numberOperation(dcNode *_left,
                                             dcNode *_right,
                                             dcTaffyOperator _operation,
                                             bool _complexOnLeft)
{
    dcNode *result = NULL;

    if (_complexOnLeft)
    {
        result = performTwoOperandsOperation(_left, _right, _operation, false);
    }
    else
    {
        // _left is of an unknown type currently
        dcNode *left;

        if ((left = dcClass_castNode(_left, sTemplate, false))
            != NULL)
        {
            // two complex numbers
            performTwoOperandsOperation(left, _right, _operation, false);
        }
        else if ((left = dcClass_castNode(_left,
                                          dcNumberClass_getTemplate(),
                                          false))
                 != NULL)
        {
            // left is Number, right is ComplexNumber
            // promote Number to ComplexNumber, and perform operation
            dcComplexNumber_arithmeticFunction operation =
                dcComplexNumber_getArithmeticOperation(_operation);
            dcComplexNumber *complexy = dcComplexNumber_create(NULL, NULL);
            dcComplexNumber *leftComplexy = dcComplexNumber_create
                (dcNumber_copy(dcNumberClass_getNumber(left), DC_DEEP), NULL);
            operation(complexy,
                      leftComplexy,
                      dcComplexNumberClass_getNumber(_right));

            if (dcNumber_equals(complexy->imaginary, dcNumber_getConstant(0)))
            {
                // it's registered later
                result = dcNumberClass_createObject(complexy->real);
                complexy->real = NULL;
                dcComplexNumber_free(&complexy, DC_DEEP);
            }
            else
            {
                // it's registered later
                result = dcComplexNumberClass_createObject(complexy);
            }

            dcComplexNumber_free(&leftComplexy, DC_DEEP);
        }
        else
        {
            dcInvalidCastExceptionClass_throwObject
                (dcClass_getName(_left), "(ComplexNumber or Number)");
        }
    }

    return result;
}

dcNode *dcComplexNumberClass_inlineAddReal(dcNode *_left,
                                           dcNode *_right,
                                           bool _complexOnLeft)
{
    if (_complexOnLeft)
    {
        dcComplexNumber *left = dcComplexNumberClass_getNumber(_left);
        dcComplexNumber_addReal(left, left, dcNumberClass_getNumber(_right));
    }
    else
    {
        dcComplexNumber *right = dcComplexNumberClass_getNumber(_right);
        dcComplexNumber_addReal(right, right, dcNumberClass_getNumber(_left));
    }

    return (_complexOnLeft
            ? _left
            : _right);
}

dcNode *dcComplexNumberClass_inlineSubtractReal(dcNode *_left,
                                                dcNode *_right,
                                                bool _complexOnLeft)
{
    if (_complexOnLeft)
    {
        dcComplexNumber *left = dcComplexNumberClass_getNumber(_left);
        dcComplexNumber_subtractReal(left,
                                     left,
                                     dcNumberClass_getNumber(_right));
    }
    else
    {
        dcComplexNumber *right = dcComplexNumberClass_getNumber(_right);
        dcNumber *left = dcNumberClass_getNumber(_left);
        dcComplexNumber_multiplyReal(right,
                                     right,
                                     dcNumber_getNegativeOne());
        dcNumber_add(right->real,
                     left,
                     right->real);
    }

    return (_complexOnLeft
            ? _left
            : _right);
}

dcNode *dcComplexNumberClass_inlineMultiplyReal(dcNode *_left,
                                                dcNode *_right,
                                                bool _complexOnLeft)
{
    if (_complexOnLeft)
    {
        dcComplexNumber *left = dcComplexNumberClass_getNumber(_left);
        dcComplexNumber_multiplyReal(left,
                                     left,
                                     dcNumberClass_getNumber(_right));
    }
    else
    {
        dcComplexNumber *right = dcComplexNumberClass_getNumber(_right);
        dcComplexNumber_multiplyReal(right,
                                     right,
                                     dcNumberClass_getNumber(_left));
    }

    return (_complexOnLeft
            ? _left
            : _right);
}

// the result is stored into the complex number (either _left or _right,
// depending on _complexOnLeft) !
dcNode *dcComplexNumberClass_inlineDivideReal(dcNode *_left,
                                              dcNode *_right,
                                              bool _complexOnLeft)
{
    if (_complexOnLeft)
    {
        dcComplexNumber *left = dcComplexNumberClass_getNumber(_left);
        dcComplexNumber_divideReal(left, left, dcNumberClass_getNumber(_right));
    }
    else
    {
        dcNumber *left = dcNumberClass_getNumber(_left);
        dcComplexNumber *lol =
            dcComplexNumber_create(dcNumber_copy(left, DC_DEEP), NULL);
        dcComplexNumber *right = dcComplexNumberClass_getNumber(_right);
        dcComplexNumber_divide(right, lol, right);
        dcComplexNumber_free(&lol, DC_DEEP);
    }

    return (_complexOnLeft
            ? _left
            : _right);
}

dcNode *dcComplexNumberClass_real(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObject
             (dcNumber_copy
              (dcComplexNumberClass_getNumber(_receiver)->real,
               DC_DEEP))));
}

dcNode *dcComplexNumberClass_imaginary(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObject
             (dcNumber_copy
              (dcComplexNumberClass_getNumber(_receiver)->imaginary,
               DC_DEEP))));
}

dcNode *dcComplexNumberClass_length(dcNode *_receiver, dcArray *_arguments)
{
    dcNumber *number = dcNumber_createFromInt32u(0);
    dcComplexNumber_modulus(number, dcComplexNumberClass_getNumber(_receiver));
    return dcNode_register(dcNumberClass_createObject(number));
}

dcNode *dcComplexNumberClass_modulus(dcNode *_receiver, dcArray *_arguments)
{
    return dcComplexNumberClass_length(_receiver, _arguments);
}

dcNode *dcComplexNumberClass_maybeConvertToReal(dcNode *_object)
{
    dcNode *result =
        (dcNumber_equalsInt32u
         (dcComplexNumberClass_getNumber(_object)->imaginary, 0)
         ? (dcNumberClass_createObject
            (dcNumber_copy
             (dcComplexNumberClass_getNumber(_object)->real, DC_DEEP)))
         : _object);

    if (dcNode_isTemplate(_object))
    {
        dcNode_setTemplate(result, true);
    }

    return result;
}

dcNode *dcComplexNumberClass_setReal(dcNode *_receiver, dcArray *_arguments)
{
    dcNumber_inlineCopy
        (dcComplexNumberClass_getNumber(_receiver)->real,
         dcNumberClass_getNumber(dcArray_get(_arguments, 0)));
    return _receiver;
}

dcNode *dcComplexNumberClass_setImaginary(dcNode *_receiver,
                                          dcArray *_arguments)
{
    dcNumber_inlineCopy
        (dcComplexNumberClass_getNumber(_receiver)->imaginary,
         dcNumberClass_getNumber(dcArray_get(_arguments, 0)));
    return _receiver;
}

dcNode *dcComplexNumberClass_setValue(dcNode *_receiver, dcArray *_arguments)
{
    dcComplexNumber_inlineCopy
        (dcComplexNumberClass_getNumber(_receiver),
         dcComplexNumberClass_getNumber(dcArray_get(_arguments, 0)));
    return _receiver;
}
