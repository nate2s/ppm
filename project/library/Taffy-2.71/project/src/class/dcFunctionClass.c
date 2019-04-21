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
#include <string.h>

#include "CompiledFunction.h"

#include "dcFunctionClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcCFunctionArgument.h"
#include "dcCharacterGraph.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcContainers.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcFlatArithmetic.h"
#include "dcGraphDatas.h"
#include "dcGraphDataTree.h"
#include "dcIdentifier.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMatrixClass.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcMutex.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcNumberClass.h"
#include "dcObjectStackList.h"
#include "dcProcedureClass.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSymbolClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcYesClass.h"

static dcFunctionMetaClassAux *sMetaAux;

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "createWithBlock:",
        SCOPE_DATA_PUBLIC,
        &dcFunctionMetaClass_createWithBlock,
        gCFunctionArgument_block
    },
    {
        "getDefaultMemorySize",
        SCOPE_DATA_PUBLIC,
        &dcFunctionMetaClass_getDefaultMemorySize,
        gCFunctionArgument_none
    },
    {
        "setDefaultMemorySize:",
        SCOPE_DATA_PUBLIC | SCOPE_DATA_SYNCHRONIZED,
        &dcFunctionMetaClass_setDefaultMemorySize,
        gCFunctionArgument_number
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "#operator(+):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_add,
        gCFunctionArgument_wild
    },
    {
        "addKey:value:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_addKeyValue,
        gCFunctionArgument_blockBlock
    },
    {
        "argumentCount",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_argumentCount,
        gCFunctionArgument_none
    },
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_asString,
        gCFunctionArgument_none
    },
    {
        "#operator(&):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_bitAnd,
        gCFunctionArgument_wild
    },
    {
        "#prefixOperator(~)",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_bitNot,
        gCFunctionArgument_none
    },
    {
        "#operator(|):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_bitOr,
        gCFunctionArgument_wild
    },
    {
        "compile",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_compile,
        gCFunctionArgument_none
    },
    {
        "compile!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_compileBang,
        gCFunctionArgument_none
    },
    {
        "compose:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_compose,
        gCFunctionArgument_function
    },
    {
        "composes:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_composes,
        gCFunctionArgument_array
    },
    {
        "id",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_id,
        gCFunctionArgument_none
    },
    {
        "call:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_parentheses,
        gCFunctionArgument_array
    },
    {
        "cancel",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_cancel,
        gCFunctionArgument_none
    },
    {
        "cancel!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_cancelBang,
        gCFunctionArgument_none
    },
    {
        "clearMemory",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_clearMemory,
        gCFunctionArgument_none
    },
    {
        "convertSubtractToAdd",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_convertSubtractToAdd,
        gCFunctionArgument_none
    },
    {
        "convertSubtractToAdd!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_convertSubtractToAddBang,
        gCFunctionArgument_none
    },
    {
        "degree",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_degree,
        gCFunctionArgument_none
    },
    {
        "derive:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONST),
        &dcFunctionClass_derive,
        gCFunctionArgument_symbol
    },
    {
        "distribute",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_distribute,
        gCFunctionArgument_none
    },
    {
        "distribute!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_distributeBang,
        gCFunctionArgument_none
    },
    {
        "#operator(/):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_divide,
        gCFunctionArgument_wild
    },
    {
        "#operator(==):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_equals,
        gCFunctionArgument_wild
    },
    {
        "#operator(~=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_deltaEquals,
        gCFunctionArgument_wild
    },
    {
        "#operator(^):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_raise,
        gCFunctionArgument_wild
    },
    {
        "expand",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_expand,
        gCFunctionArgument_none
    },
    {
        "expand!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_expandBang,
        gCFunctionArgument_none
    },
    {
        "factor",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_factor,
        gCFunctionArgument_none
    },
    {
        "factor!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_factorBang,
        gCFunctionArgument_none
    },
    {
        "#operator(!)",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_factorial,
        gCFunctionArgument_none
    },
    {
        "getMemorySize",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_getMemorySize,
        gCFunctionArgument_none
    },
    {
        "#operator(<<):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_leftShift,
        gCFunctionArgument_wild
    },
    {
        "integrate:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONST),
        &dcFunctionClass_integrate,
        gCFunctionArgument_symbol
    },
    {
        "#operator(%):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_modulus,
        gCFunctionArgument_wild
    },
    {
        "#operator(*):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_multiply,
        gCFunctionArgument_wild
    },
    {
        "#operator(()):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONST),
        &dcFunctionClass_parentheses,
        gCFunctionArgument_array
    },
    {
        "prettyPrint",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_prettyPrint,
        gCFunctionArgument_none
    },
    {
        "#operator(>>):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_rightShift,
        gCFunctionArgument_wild
    },
    {
        "setMemorySize:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_setMemorySize,
        gCFunctionArgument_number
    },
    {
        "simplify",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_simplify,
        gCFunctionArgument_none
    },
    {
        "simplify!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFunctionClass_simplifyBang,
        gCFunctionArgument_none
    },
    {
        "#operator(-):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcFunctionClass_subtract,
        gCFunctionArgument_wild
    },
    {
        0
    }
};

// standard functions //
ALLOCATE_FUNCTION(dcFunctionClass_allocateNode);
DEALLOCATE_FUNCTION(dcFunctionClass_deallocateNode);
DEINITIALIZE_FUNCTION(dcFunctionClass_deinitialize);
COPY_FUNCTION(dcFunctionClass_copyNode);
DO_GRAPH_OPERATION_FUNCTION(dcFunctionClass_doGraphOperation);
FREE_FUNCTION(dcFunctionClass_freeNode);
INITIALIZE_FUNCTION(dcFunctionClass_initialize);
MARK_FUNCTION(dcFunctionClass_markNode);
MARSHALL_FUNCTION(dcFunctionClass_marshallNode);
SET_TEMPLATE_FUNCTION(dcFunctionClass_setTemplate);
UNMARSHALL_FUNCTION(dcFunctionClass_unmarshallNode);

#define CAST_FUNCTION_AUX(_node_)                   \
    ((dcFunctionClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcFunctionClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (FUNCTION_PACKAGE_NAME,                   // package name
          FUNCTION_CLASS_NAME,                     // class name
          MAKE_FULLY_QUALIFIED(PROCEDURE),         // super type
          (CLASS_ATOMIC | CLASS_ABSTRACT),         // class flags
          NO_FLAGS,                                // scope data flags
          sMetaMethodWrappers,                     // meta methods
          sMethodWrappers,                         // methods
          &dcFunctionClass_initialize,             // initialization function
          &dcFunctionClass_deinitialize,           // deinitialization function
          &dcFunctionClass_allocateNode,           // allocate
          &dcFunctionClass_deallocateNode,         // deallocate
          NULL,                                    // meta mark
          &dcFunctionClass_markNode,               // mark
          &dcFunctionClass_copyNode,               // copy
          &dcFunctionClass_freeNode,               // free
          NULL,                                    // register
          &dcFunctionClass_marshallNode,           // marshall
          &dcFunctionClass_unmarshallNode,         // unmarshall
          NULL));                                  // set template
}

static void lockMetaMutex(void)
{
    dcMutex_lock(sMetaAux->mutex);
}

static void unlockMetaMutex(void)
{
    dcMutex_unlock(sMetaAux->mutex);
}

uint32_t dcFunctionMetaClass_getAndIncrementFunctionId(void)
{
    uint32_t result = sMetaAux->nextFunctionId;
    sMetaAux->nextFunctionId++;
    return result;
}

static dcFunctionClassAux *createAuxWithMemorySize(dcList *_specificValues,
                                                   size_t _memorySize)
{
    dcFunctionClassAux *aux =
        (dcFunctionClassAux *)dcMemory_allocate(sizeof(dcFunctionClassAux));
    aux->memory = dcHash_create();
    aux->memoryList = dcList_create();
    aux->specificValues = (_specificValues == NULL
                           ? dcList_create()
                           : _specificValues);
    lockMetaMutex();
    aux->memorySize = _memorySize;
    aux->functionId = dcFunctionMetaClass_getAndIncrementFunctionId();
    sMetaAux->nextFunctionId++;
    unlockMetaMutex();
    return aux;
}

static dcFunctionClassAux *createAux(dcList *_specificValues)
{
    return createAuxWithMemorySize(_specificValues,
                                   sMetaAux->defaultMemorySize);
}

#define FUNCTION_CLASS_FILE_NAME "dcFunctionClass.c"

void dcFunctionClass_initialize(void)
{
    sMetaAux = ((dcFunctionMetaClassAux *)
                dcMemory_allocate(sizeof(dcFunctionMetaClassAux)));
    sMetaAux->mutex = dcMutex_create(false);
    sMetaAux->defaultMemorySize = 3;
    sMetaAux->nextFunctionId = 1;
    dcError_assert(dcStringEvaluator_evalString(__compiledFunction,
                                                FUNCTION_CLASS_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

void dcFunctionClass_deinitialize(void)
{
    dcMutex_free(&sMetaAux->mutex);
    dcMemory_free(sMetaAux);
}

void dcFunctionClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = NULL;
}

void dcFunctionClass_deallocateNode(dcNode *_node)
{
    dcList_clear(CAST_FUNCTION_AUX(_node)->memoryList, DC_SHALLOW);
    dcHash_clear(CAST_FUNCTION_AUX(_node)->memory, DC_SHALLOW);
}

dcNode *dcFunctionClass_createObjectWithArguments(dcList *_identifiers,
                                                  dcNode *_body)
{
    return dcClass_createNode(sTemplate,
                              (dcProcedureClass_createNode
                               (dcGraphDataTree_createNode(_body),
                                dcMethodHeader_create("", _identifiers),
                                true)), // object
                              NULL,     // scope
                              true,     // object
                              createAux(NULL));
}

dcNode *dcFunctionClass_createNode(dcNode *_body,
                                   dcMethodHeader *_header,
                                   bool _object)
{
    return (dcClass_createNode
            (sTemplate,
             dcProcedureClass_createNode(_body, _header, _object),
             NULL, // scope
             _object,
             createAux(NULL)));
}

dcNode *dcFunctionClass_createObject(dcNode *_body, dcMethodHeader *_header)
{
    return dcFunctionClass_createNode(_body, _header, true);
}

void dcFunctionClass_freeNode(dcNode *_functionNode, dcDepth _depth)
{
    dcFunctionClassAux *aux = CAST_FUNCTION_AUX(_functionNode);

    if (aux != NULL)
    {
        dcList_free(&aux->memoryList, DC_SHALLOW);
        dcHash_free(&aux->memory, DC_SHALLOW);
        dcList_free(&aux->specificValues, DC_DEEP);
        dcMemory_free(aux);
    }
}

void dcFunctionClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    // the body of the function is copied in Procedure
    dcFunctionClassAux *fromAux = CAST_FUNCTION_AUX(_from);

    uint16_t memorySize = 0;

    if (dcNode_isTemplate(_from))
    {
        memorySize = sMetaAux->defaultMemorySize;
    }
    else
    {
        memorySize = fromAux->memorySize;
    }

    CAST_CLASS_AUX(_to) = (createAuxWithMemorySize
                           (dcList_copy(fromAux->specificValues, DC_DEEP),
                            memorySize));
}

void dcFunctionClass_markNode(dcNode *_function)
{
    dcHash_mark(CAST_FUNCTION_AUX(_function)->memory);
}

dcNode *dcFunctionMetaClass_createWithBlock(dcNode *_receiver,
                                            dcArray *_arguments)
{
    dcNode *procedure = (dcClass_castNodeWithAssert
                         (dcArray_get(_arguments, 0),
                          dcProcedureClass_getTemplate(),
                          false,
                          true));
    dcNode *body = dcProcedureClass_getBody(procedure);
    assert(dcGraphData_getType(body) == NODE_GRAPH_DATA_TREE);

    if (dcGraphDataTree_getSize(body) > 1)
    {
        dcBlockNotSingularExceptionClass_throwObject();
        return NULL;
    }

    return (dcNode_register
            (dcFunctionClass_createNode
             (dcNode_copy(body, DC_DEEP),
              dcMethodHeader_copy
              (dcProcedureClass_getMethodHeader(procedure), DC_DEEP),
              true)));
}

dcNode *dcFunctionMetaClass_setDefaultMemorySize(dcNode *_receiver,
                                                 dcArray *_arguments)
{
    dcNode *memorySizeObject = dcArray_get(_arguments, 0);
    int32_t memorySize = 0;
    dcNode *result = NULL;

    if (dcNumberClass_extractInt32s_withException(memorySizeObject,
                                                  &memorySize))
    {
        lockMetaMutex();
        sMetaAux->defaultMemorySize = memorySize;
        unlockMetaMutex();
        result = memorySizeObject;
    }

    return result;
}

dcNode *dcFunctionMetaClass_getDefaultMemorySize(dcNode *_function,
                                                 dcArray *_arguments)
{
    lockMetaMutex();
    dcNode *result = (dcNode_register
                      (dcNumberClass_createObjectFromInt32s
                       (sMetaAux->defaultMemorySize)));
    unlockMetaMutex();
    return result;
}

dcNode *dcFunctionClass_addSpecific(dcNode *_functionClassNode,
                                    const dcList *_arguments,
                                    const dcNode *_arithmetic)
{
    dcNode *pair =
        dcPair_createNode(dcList_createShell(dcList_copy(_arguments, DC_DEEP)),
                          dcNode_copy(_arithmetic, DC_DEEP));
    dcFunctionClassAux *aux = CAST_FUNCTION_AUX(_functionClassNode);

    FOR_EACH_IN_LIST(aux->specificValues, that)
    {
        dcTaffyOperator compareOperation;
        dcResult compareResult =
            dcNode_compareEqual(that->object, pair, &compareOperation);

        if (compareResult == TAFFY_EXCEPTION)
        {
            dcNode_free(&pair, DC_DEEP);
            return NULL;
        }
        else if (compareResult == TAFFY_SUCCESS
                 && compareOperation == TAFFY_EQUALS)
        {
            dcNode_free(&pair, DC_DEEP);
            return _functionClassNode;
        }
    }

    dcList_push(aux->specificValues, pair);
    return _functionClassNode;
}

dcNode *dcFunctionClass_addKeyValue(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *valueBody =
        (dcGraphDataTree_getContents
         (dcBlockClass_getBody(dcArray_get(_arguments, 0))));
    dcList *arguments =
        dcList_createWithObjects
        (dcNode_setTemplate(dcNode_copy(valueBody, DC_DEEP), true),
         NULL);
    dcList_push(CAST_FUNCTION_AUX(_receiver)->specificValues,
                dcPair_createNode
                (dcList_createShell(arguments),
                 dcNode_copy
                 (dcGraphDataTree_getContents
                  (dcBlockClass_getBody
                   (dcArray_get(_arguments, 1))),
                  DC_DEEP)));
    return _receiver;
}

TAFFY_HIDDEN dcNode *performArithmetic(dcNode *_receiver,
                                       dcArray *_arguments,
                                       dcTaffyOperator _type)
{
    dcNode *result = NULL;
    dcNode *argument = dcArray_get(_arguments, 0);
    dcNode *right = NULL;

    if ((right = dcClass_castNode(argument, sTemplate, false))
        != NULL)
    {
        result = (dcFunctionClass_numberOperation
                  (_receiver,
                   dcFunctionClass_getGraphDataBody(right),
                   _type,
                   true));
    }
    else if (((right = dcClass_castNode(argument,
                                        dcNumberClass_getTemplate(),
                                        false))
              != NULL)
             || ((right = dcClass_castNode(argument,
                                           dcMatrixClass_getTemplate(),
                                           false))
                 != NULL))
    {
        result = (dcFunctionClass_numberOperation
                  (_receiver, right, _type, true));
    }
    else
    {
        dcInvalidCastExceptionClass_throwObject(dcClass_getName(argument),
                                                "(Function or Number)");
    }

    return result;
}

dcNode *dcFunctionClass_add(dcNode *_receiver, dcArray *_arguments)
{
    return performArithmetic(_receiver, _arguments, TAFFY_ADD);
}

dcNode *dcFunctionClass_divide(dcNode *_receiver, dcArray *_arguments)
{
    return performArithmetic(_receiver, _arguments, TAFFY_DIVIDE);
}

dcNode *dcFunctionClass_multiply(dcNode *_receiver, dcArray *_arguments)
{
    return performArithmetic(_receiver, _arguments, TAFFY_MULTIPLY);
}

dcNode *dcFunctionClass_subtract(dcNode *_receiver, dcArray *_arguments)
{
    return performArithmetic(_receiver, _arguments, TAFFY_SUBTRACT);
}

dcNode *dcFunctionClass_raise(dcNode *_receiver, dcArray *_arguments)
{
    return performArithmetic(_receiver, _arguments, TAFFY_RAISE);
}

dcNode *dcFunctionClass_leftShift(dcNode *_receiver, dcArray *_arguments)
{
    return performArithmetic(_receiver, _arguments, TAFFY_LEFT_SHIFT);
}

dcNode *dcFunctionClass_rightShift(dcNode *_receiver, dcArray *_arguments)
{
    return performArithmetic(_receiver, _arguments, TAFFY_RIGHT_SHIFT);
}

dcNode *dcFunctionClass_bitAnd(dcNode *_receiver, dcArray *_arguments)
{
    return performArithmetic(_receiver, _arguments, TAFFY_BIT_AND);
}

dcNode *dcFunctionClass_bitOr(dcNode *_receiver, dcArray *_arguments)
{
    return performArithmetic(_receiver, _arguments, TAFFY_BIT_OR);
}

dcNode *dcFunctionClass_modulus(dcNode *_receiver, dcArray *_arguments)
{
    return performArithmetic(_receiver, _arguments, TAFFY_MODULUS);
}

static dcNode *flatArithmeticOperation(dcNode *_receiver,
                                       dcFlatArithmeticOperation _operation)
{
    dcNode *procedure = dcClass_getSuperNode(_receiver);
    dcNode *body = dcProcedureClass_getBody(procedure);
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNodeEvaluator_incrementAbortDelay(evaluator);

    CAST_GRAPH_DATA_TREE(body)->head =
        _operation(CAST_GRAPH_DATA_TREE(body)->head, NULL);

    FOR_EACH_IN_LIST(CAST_FUNCTION_AUX(_receiver)->specificValues, that)
    {
        CAST_PAIR(that->object)->right =
            _operation(CAST_PAIR(that->object)->right, NULL);
    }

    if (dcNodeEvaluator_decrementAbortDelay(evaluator))
    {
        return NULL;
    }

    return _receiver;
}

dcNode *dcFunctionClass_simplify(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNode_copy(_receiver, DC_DEEP);
    dcFunctionClass_simplifyBang(result, _arguments);
    return dcNode_register(result);
}

dcNode *dcFunctionClass_simplifyBang(dcNode *_receiver, dcArray *_arguments)
{
    return flatArithmeticOperation(_receiver, &dcFlatArithmetic_shrink);
}

dcNode *dcFunctionClass_factor(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNode_copy(_receiver, DC_DEEP);
    dcFunctionClass_factorBang(result, _arguments);
    return dcNode_register(result);
}

dcNode *dcFunctionClass_factorBang(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = (flatArithmeticOperation
                      (_receiver, &dcFlatArithmetic_multiFactor));

    if (result != NULL)
    {
        result = (flatArithmeticOperation
                  (result, &dcFlatArithmetic_undoConvertSubtractToAdd));
    }

    if (result != NULL)
    {
        result = flatArithmeticOperation(result, &dcFlatArithmetic_snip);
    }

    return result;
}

dcNode *dcFunctionClass_cancel(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNode_copy(_receiver, DC_DEEP);
    dcFunctionClass_cancelBang(result, _arguments);
    return dcNode_register(result);
}

dcNode *dcFunctionClass_cancelBang(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result =
        flatArithmeticOperation(_receiver, &dcFlatArithmetic_cancel);

    if (result != NULL)
    {
        result = flatArithmeticOperation(result, &dcFlatArithmetic_shrink);
    }

    return result;
}

dcNode *dcFunctionClass_distribute(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNode_copy(_receiver, DC_DEEP);
    dcFunctionClass_distributeBang(result, _arguments);
    return dcNode_register(result);
}

dcNode *dcFunctionClass_distributeBang(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *procedure = dcClass_getSuperNode(_receiver);
    dcNode *body = dcProcedureClass_getBody(procedure);
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNodeEvaluator_incrementAbortDelay(evaluator);

    CAST_GRAPH_DATA_TREE(body)->head =
        dcFlatArithmetic_distributeLikeAMadman(CAST_GRAPH_DATA_TREE(body)->head,
                                               NULL);
    CAST_GRAPH_DATA_TREE(body)->head =
        dcFlatArithmetic_distribute(CAST_GRAPH_DATA_TREE(body)->head, NULL);
    CAST_GRAPH_DATA_TREE(body)->head =
        dcFlatArithmetic_combine(CAST_GRAPH_DATA_TREE(body)->head, NULL);
    CAST_GRAPH_DATA_TREE(body)->head =
        dcFlatArithmetic_merge(CAST_GRAPH_DATA_TREE(body)->head,
                               NULL);

    FOR_EACH_IN_LIST(CAST_FUNCTION_AUX(_receiver)->specificValues, that)
    {
        CAST_PAIR(that->object)->right =
            dcFlatArithmetic_distributeLikeAMadman
            (CAST_PAIR(that->object)->right, NULL);
        CAST_PAIR(that->object)->right =
            dcFlatArithmetic_distribute(CAST_PAIR(that->object)->right, NULL);
        CAST_PAIR(that->object)->right =
            dcFlatArithmetic_combine(CAST_PAIR(that->object)->right, NULL);
        CAST_PAIR(that->object)->right =
            dcFlatArithmetic_merge(CAST_PAIR(that->object)->right,
                                   NULL);
    }

    if (dcNodeEvaluator_decrementAbortDelay(evaluator))
    {
        return NULL;
    }

    return _receiver;
}

dcNode *dcFunctionClass_expand(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNode_copy(_receiver, DC_DEEP);
    dcFunctionClass_expandBang(result, _arguments);
    return dcNode_register(result);
}

dcNode *dcFunctionClass_expandBang(dcNode *_receiver, dcArray *_arguments)
{
    return flatArithmeticOperation(_receiver, &dcFlatArithmetic_expand);
}

static dcNode *prefixOperation(dcNode *_receiver, dcTaffyOperator _operator)
{
    dcNode *result = dcNode_register(dcNode_copy(_receiver, DC_DEEP));
    dcNode *body = dcFunctionClass_getBody(result);
    dcNode *contents = dcGraphDataTree_getContents(body);
    dcNode *newBody = (dcMethodCall_createNodeWithArgument
                       (contents,
                        dcSystem_getOperatorName(_operator),
                        NULL));
    CAST_GRAPH_DATA_TREE(body)->head = newBody;
    // is this needed?
    dcProcedureClass_setBody(dcClass_getSuperNode(result), body);
    return result;
}

dcNode *dcFunctionClass_factorial(dcNode *_receiver, dcArray *_arguments)
{
    return prefixOperation(_receiver, TAFFY_FACTORIAL);
}

dcNode *dcFunctionClass_bitNot(dcNode *_receiver, dcArray *_arguments)
{
    return prefixOperation(_receiver, TAFFY_BIT_NOT);
}

TAFFY_HIDDEN dcNode *dcFunctionClass_calculusOperation
    (dcNode *_receiver,
     dcArray *_arguments,
     dcFlatArithmetic_calculusOperationFunction _function)
{
    const char *symbol = (dcSymbolClass_getString_helper
                          (dcArray_get(_arguments, 0)));

    dcNode *procedure = dcClass_getSuperNode(_receiver);

    if (strcmp(symbol, "__x__") == 0)
    {
        dcList *values = (dcMethodHeader_getArguments
                          (dcProcedureClass_getMethodHeader(procedure)));
        symbol = dcIdentifier_getName(values->head->object);
    }

    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNodeEvaluator_incrementAbortDelay(evaluator);
    dcNode *operationResult = _function(dcGraphDataTree_getContents
                                        (dcProcedureClass_getBody(procedure)),
                                        symbol);
    dcNode *result = NULL;

    if (evaluator->exception == NULL)
    {
        if (operationResult == NULL)
        {
            result = dcNilClass_getInstance();
        }
        else
        {
            result = (dcNode_register
                      (dcFunctionClass_createNode
                       (dcGraphDataTree_createNode(operationResult),
                        dcMethodHeader_copy
                        (dcProcedureClass_getMethodHeader(procedure),
                         DC_DEEP),
                        true)));

            if (dcNodeEvaluator_callMethod(evaluator, result, "init") == NULL)
            {
                result = NULL;
            }
        }
    }

    if (dcNodeEvaluator_decrementAbortDelay(evaluator))
    {
        result = NULL;
    }

    return result;
}

dcNode *dcFunctionClass_degree(dcNode *_receiver, dcArray *_arguments)
{
    const dcMethodHeader *header = dcFunctionClass_getMethodHeader(_receiver);
    dcList *arguments = dcMethodHeader_getArguments(header);
    int32_t degree = 0;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNodeEvaluator_incrementAbortDelay(evaluator);

    dcNode *result = (dcFlatArithmetic_degree
                      (dcFunctionClass_getGraphDataBody(_receiver),
                       arguments, // symbols
                       &degree)
                      ? (dcNode_register
                         (dcNumberClass_createObjectFromInt32s(degree)))
                      : dcNilClass_getInstance());

    if (dcNodeEvaluator_decrementAbortDelay(evaluator))
    {
        result = NULL;
    }

    return result;
}

dcNode *dcFunctionClass_derive(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;

    if (CAST_FUNCTION_AUX(_receiver)->specificValues->size > 0)
    {
        dcUnsupportedMathOperationExceptionClass_throwObject
            ("function has specific values");
    }
    else
    {
        result = dcFunctionClass_calculusOperation(_receiver,
                                                   _arguments,
                                                   &dcFlatArithmetic_derive);
    }

    return result;
}

dcNode *dcFunctionClass_id(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32u
             (CAST_FUNCTION_AUX(_receiver)->functionId)));
}

dcNode *dcFunctionClass_integrate(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;

    if (CAST_FUNCTION_AUX(_receiver)->specificValues->size > 0)
    {
        dcUnsupportedMathOperationExceptionClass_throwObject
            ("function has specific values");
    }
    else
    {
        result = dcFunctionClass_calculusOperation(_receiver,
                                                   _arguments,
                                                   &dcFlatArithmetic_integrate);
    }

    return result;
}

dcNode *dcFunctionClass_getMemorySize(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32s
             (CAST_FUNCTION_AUX(_receiver)->memorySize)));
}

dcNode *dcFunctionClass_setMemorySize(dcNode *_receiver, dcArray *_arguments)
{
    dcFunctionClassAux *aux = CAST_FUNCTION_AUX(_receiver);
    dcNode *memorySizeObject = dcArray_get(_arguments, 0);
    int32_t memorySize;
    dcNode *result = NULL;

    if (dcNumberClass_extractInt32s_withException(memorySizeObject,
                                                  &memorySize))
    {
        aux->memorySize = memorySize;
        result = _receiver;
    }

    return result;
}

dcNode *dcFunctionClass_clearMemory(dcNode *_receiver, dcArray *_arguments)
{
    dcHash_clear(CAST_FUNCTION_AUX(_receiver)->memory, DC_SHALLOW);
    return dcYesClass_getInstance();
}

dcNode *dcFunctionClass_convertSubtractToAddBang(dcNode *_receiver,
                                                 dcArray *_arguments)
{
    return (flatArithmeticOperation
            (_receiver,
             &dcFlatArithmetic_convertSubtractToAdd));
}

dcNode *dcFunctionClass_convertSubtractToAdd(dcNode *_receiver,
                                             dcArray *_arguments)
{
    dcNode *result = dcNode_copy(_receiver, DC_DEEP);
    dcFunctionClass_convertSubtractToAddBang(result, _arguments);
    return dcNode_register(result);
}

dcNode *dcFunctionClass_getGraphDataBody(dcNode *_receiver)
{
    return (dcGraphDataTree_getContents
            (dcProcedureClass_getBody(dcClass_getSuperNode(_receiver))));
}

dcNode *dcFunctionClass_getBody(dcNode *_receiver)
{
    return dcProcedureClass_getBody(dcClass_getSuperNode(_receiver));
}

static void printArguments(dcList *_arguments, dcString *_string)
{
    dcListElement *that;

    for (that = _arguments->head; that != NULL; that = that->next)
    {
        dcNode_print(that->object, _string);

        if (that->next != NULL)
        {
            dcString_appendString(_string, ", ");
        }
    }
}

dcNode *dcFunctionClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    dcString display;
    dcString_initialize(&display, 100);
    dcNode *procedure = dcClass_getOrInstantiateSuperNode(_receiver);
    const dcMethodHeader *header = dcFunctionClass_getMethodHeader(_receiver);
    dcList *arguments = dcMethodHeader_getArguments(header);
    dcList *specificValues = NULL;

    if (dcFunctionClass_isMe(_receiver))
    {
        specificValues = CAST_FUNCTION_AUX(_receiver)->specificValues;

        if (specificValues->size > 0)
        {
            dcString_appendString(&display, "{");
        }

        dcString_appendString(&display, "F(");
    }
    else
    {
        dcString_append(&display, "#%s(", dcClass_getName(_receiver));
    }

    if (arguments != NULL)
    {
        printArguments(arguments, &display);
    }

    dcString_appendString(&display, ") = ");
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    evaluator->printingFunction++;
    dcNode_print(dcProcedureClass_getBody(procedure), &display);
    evaluator->printingFunction--;

    if (specificValues != NULL && specificValues->size > 0)
    {
        dcString_appendString(&display, ", ");

        FOR_EACH_IN_LIST(specificValues, that)
        {
            dcPair *pair = CAST_PAIR(that->object);

            //
            // print the <arguments>
            //
            dcString_appendString(&display, "(");
            FOR_EACH_IN_LIST(CAST_LIST(pair->left), here)
            {
                dcString_appendString(&display, dcNode_display(here->object));

                if (here->next != NULL)
                {
                    dcString_appendString(&display, ", ");
                }
            }

            dcString_appendString(&display, ")");
            //
            // </arguments>
            //

            dcString_append(&display, " = %s", dcNode_display(pair->right));

            if (that->next != NULL)
            {
                dcString_appendString(&display, ", ");
            }
        }

        dcString_appendString(&display, "}");
    }

    return dcNode_register(dcStringClass_createObject(display.string, false));
}

TAFFY_HIDDEN dcNode *computeValue(dcNode *_receiver, dcNode *_keys)
{
    dcNode *procedureNode =
        dcClass_castNodeWithAssert(_receiver,
                                   dcProcedureClass_getTemplate(),
                                   false,
                                   true);
    dcMethodHeader *header = dcProcedureClass_getMethodHeader(procedureNode);
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *result = NULL;
    dcArray *keys = dcArrayClass_getObjects(_keys);
    dcNode *body = dcProcedureClass_getBody(procedureNode);
    const dcList *headerArguments = dcMethodHeader_getArguments(header);

    //
    // find the body to use
    //
    FOR_EACH_IN_LIST(CAST_FUNCTION_AUX(_receiver)->specificValues, that)
    {
        dcNode *pairNode = that->object;
        dcList *specificArguments = CAST_LIST(dcPair_getLeft(pairNode));

        // do the arguments match? //
        bool match = (dcFunctionClass_argumentsMatch
                      (keys,
                       specificArguments,
                       headerArguments));

        if (dcNodeEvaluator_hasException(evaluator))
        {
           return NULL;
        }

        if (match)
        {
            body = dcPair_getRight(pairNode);
            break;
        }
    }

    //
    // evaluate it!
    //
    dcList *arguments = dcList_createFromArray(keys);
    uint32_t pushCount = 0;
    pushCount += dcNodeEvaluator_pushMark(evaluator, _receiver);
    pushCount += dcNodeEvaluator_pushListToMark(evaluator, arguments);
    result = dcNodeEvaluator_evaluateProcedureGuts(evaluator,
                                                   body,
                                                   header,
                                                   arguments);
    pushCount += dcNodeEvaluator_pushMark(evaluator, result);
    dcNodeEvaluator_setReturning(evaluator, false);
    dcList_free(&arguments, DC_SHALLOW);

    //
    // add result to memory, if we can
    //
    if (result != NULL)
    {
        dcFunctionClassAux *aux = CAST_FUNCTION_AUX(_receiver);

        if (aux->memorySize > 0)
        {
            if (dcHash_setValue(aux->memory, _keys, result)
                == TAFFY_EXCEPTION)
            {
                result = NULL;
            }

            if (result != NULL)
            {
                dcList_push(aux->memoryList, _keys);

                if (aux->memoryList->size > aux->memorySize)
                {
                    dcNode *head = dcList_shift(aux->memoryList, DC_SHALLOW);

                    if (dcHash_removeValue(aux->memory, head, NULL, DC_SHALLOW)
                        == TAFFY_EXCEPTION)
                    {
                        result = NULL;
                    }
                }
            }
        }
    }

    dcNodeEvaluator_popMarks(evaluator, pushCount);
    return result;
}

dcNode *dcFunctionClass_parentheses(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *keys = dcArray_get(_arguments, 0);
    dcNode *result = NULL;
    bool exception = false;
    dcFunctionClassAux *aux = CAST_FUNCTION_AUX(_receiver);

    // look in the memory first
    if (aux->memorySize > 0
        && dcHash_getValue(aux->memory, keys, &result) == TAFFY_EXCEPTION)
    {
        exception = true;
    }

    if (! exception && result == NULL)
    {
        //fprintf(stderr,
        //        "didn't get from memory: %s\n",
        //        dcArray_display(_arguments));
        result = computeValue(_receiver, keys);
    }
    else if (! exception && result != NULL)
    {
        //fprintf(stderr, "got from memory!\n");
    }

    return result;
}

dcResult dcFunctionClass_compileHelper(dcNode *_function, bool *_changed)
{
    dcNode *graphDataTree = dcFunctionClass_getBody(_function);

    if (dcGraphDataTree_getSize(graphDataTree) > 1)
    {
        dcUnsupportedMathOperationExceptionClass_throwObject
            ("function body must be a single statement");
        return TAFFY_EXCEPTION;
    }

    dcNode *body = dcGraphDataTree_getContents(graphDataTree);
    dcNode *backup = dcNode_copy(body, DC_DEEP);
    dcList *arguments = (dcMethodHeader_getArguments
                         (dcFunctionClass_getMethodHeader(_function)));

    if (dcFlatArithmetic_compile(&body, arguments, _changed) == TAFFY_EXCEPTION)
    {
        assert(! dcNode_isRegistered(body));
        dcNode_free(&body, DC_DEEP);
        CAST_GRAPH_DATA_TREE(graphDataTree)->head = backup;
        return TAFFY_EXCEPTION;
    }

    dcNode_free(&backup, DC_DEEP);
    body = dcFlatArithmetic_merge(body, NULL);
    CAST_GRAPH_DATA_TREE(graphDataTree)->head = body;
    return TAFFY_SUCCESS;
}

dcNode *dcFunctionClass_compile(dcNode *_receiver, dcArray *_arguments)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *copy = dcNode_register(dcNode_copy(_receiver, DC_DEEP));
    dcNodeEvaluator_pushMark(evaluator, copy);
    dcNode *result = dcFunctionClass_compileBang(copy, _arguments);
    dcNodeEvaluator_popMark(evaluator);
    return result;
}

dcNode *dcFunctionClass_compileBang(dcNode *_receiver, dcArray *_arguments)
{
    if (dcFunctionClass_compileHelper(_receiver, NULL) == TAFFY_EXCEPTION)
    {
        return NULL;
    }

    return _receiver;
}

dcNode *dcFunctionClass_compose(dcNode *_receiver, dcArray *_arguments)
{
    dcMethodHeader *header = dcFunctionClass_getMethodHeader(_receiver);
    dcList *arguments = dcMethodHeader_getArguments(header);
    dcNode *body = (dcNode_copy
                    (dcFunctionClass_getGraphDataBody(_receiver),
                     DC_DEEP));

    if (arguments->size != 1)
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject(1, arguments->size);
        return NULL;
    }

    dcNode *function = dcArray_get(_arguments, 0);

    if (CAST_FUNCTION_AUX(function)->specificValues->size > 0)
    {
        dcUnsupportedMathOperationExceptionClass_throwObject
            ("function body cannot be composed "
             "(function has specific values)");
        return NULL;
    }

    assert(IS_IDENTIFIER(arguments->head->object));

    dcFlatArithmetic_findIdentifier
        (&body,
         dcIdentifier_getName(arguments->head->object),
         dcFunctionClass_getGraphDataBody(function));

    return dcNode_register(dcFunctionClass_createObject
                           (dcGraphDataTree_createNode(body),
                            dcMethodHeader_copy(header, DC_DEEP)));
}

dcNode *dcFunctionClass_composes(dcNode *_receiver, dcArray *_arguments)
{
    dcMethodHeader *header = dcFunctionClass_getMethodHeader(_receiver);
    dcList *arguments = dcMethodHeader_getArguments(header);
    dcNode *body = (dcNode_copy
                    (dcFunctionClass_getGraphDataBody(_receiver),
                     DC_DEEP));
    dcArray *replacements = dcArrayClass_getObjects(dcArray_get(_arguments, 0));
    dcNode **functions = ((dcNode **)dcMemory_allocate(sizeof(dcNode *)
                                                       * replacements->size));

    if (arguments->size != replacements->size)
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject
            (arguments->size, replacements->size);
        return NULL;
    }

    size_t i;

    for (i = 0; i < replacements->size; i++)
    {
        functions[i] = dcClass_castNode(replacements->objects[i],
                                     dcFunctionClass_getTemplate(),
                                     true);

        if (functions[i] == NULL)
        {
            dcMemory_free(functions);
            return NULL;
        }

        if (CAST_FUNCTION_AUX(functions[i])->specificValues->size > 0)
        {
            dcUnsupportedMathOperationExceptionClass_throwObject
                ("function body cannot be composed "
                 "(function has specific values)");
            dcMemory_free(functions);
            return NULL;
        }
    }

    i = 0;

    FOR_EACH_IN_LIST(arguments, that)
    {
        assert(IS_IDENTIFIER(that->object));
        dcFlatArithmetic_findIdentifier
            (&body,
             dcIdentifier_getName(that->object),
             dcFunctionClass_getGraphDataBody(functions[i]));
        i++;
    }

    dcMemory_free(functions);
    return dcNode_register(dcFunctionClass_createObject
                           (dcGraphDataTree_createNode(body),
                            dcMethodHeader_copy(header, DC_DEEP)));
}

dcNode *dcFunctionClass_prettyPrint(dcNode *_receiver, dcArray *_arguments)
{
    dcCharacterGraph *graph = dcCharacterGraph_createFromCharString("F(");
    dcNode *result = NULL;
    const dcMethodHeader *header = dcFunctionClass_getMethodHeader(_receiver);
    dcList *arguments = dcMethodHeader_getArguments(header);
    dcList *specificValues = CAST_FUNCTION_AUX(_receiver)->specificValues;

    if (arguments != NULL)
    {
        dcString *display = dcString_create();
        printArguments(arguments, display);
        dcCharacterGraph_appendString(graph, display, 0);
        dcString_free(&display, DC_DEEP);
    }

    dcCharacterGraph_appendCharString(graph, ") = ", 0);

    dcCharacterGraph *rightHandSide = NULL;
    dcResult printResult = dcNode_prettyPrint
        (dcFunctionClass_getBody(_receiver), &rightHandSide);

    if (printResult == TAFFY_SUCCESS)
    {
        if (specificValues != NULL && specificValues->size > 0)
        {
            FOR_EACH_IN_LIST(specificValues, that)
            {
                dcCharacterGraph_addRows(rightHandSide, 1);

                dcPair *pair = CAST_PAIR(that->object);
                dcCharacterGraph *specificGraph;
                assert(dcNode_prettyPrint(pair->right, &specificGraph)
                       == TAFFY_SUCCESS);
                int32_t midY = specificGraph->height / 2;

                dcCharacterGraph_appendCharString(specificGraph, " if ", midY);

                //
                // print the <arguments>
                //
                FOR_EACH_IN_LIST(CAST_LIST(pair->left), here)
                {
                    dcCharacterGraph *argumentGraph;

                    dcResult argumentResult =
                        dcNode_prettyPrint(here->object, &argumentGraph);

                    if (argumentResult == TAFFY_EXCEPTION)
                    {
                        // TODO: handle this
                        assert(false);
                    }

                    dcCharacterGraph_insertCharacterGraphUp
                        (specificGraph,
                         specificGraph->width,
                         midY + argumentGraph->height / 2,
                         argumentGraph);

                    midY = argumentGraph->height / 2;

                    if (here->next != NULL)
                    {
                        dcCharacterGraph_appendCharString
                            (specificGraph, " and ", midY);
                    }

                    dcCharacterGraph_free(&argumentGraph);
                }
                //
                // </arguments>
                //

                if (specificGraph->height > 1)
                {
                    dcCharacterGraph_addParens(specificGraph);
                }

                dcCharacterGraph_insertCharacterGraphDown(rightHandSide,
                                                          0,
                                                          rightHandSide->height,
                                                          specificGraph);

                dcCharacterGraph_free(&specificGraph);
            }
        }

        if (rightHandSide->height > 1
            && specificValues != NULL
            && specificValues->size > 0)
        {
            dcCharacterGraph_addParens(rightHandSide);
        }

        dcCharacterGraph_insertCharacterGraphUp(graph,
                                                graph->width,
                                                rightHandSide->height / 2,
                                                rightHandSide);

        dcString *string = dcCharacterGraph_convertToString(graph);
        result = dcNode_register
            (dcStringClass_createObject(string->string, false));
        dcString_free(&string, DC_SHALLOW);
    }

    dcCharacterGraph_free(&graph);
    dcCharacterGraph_free(&rightHandSide);
    return result;
}

bool dcFunctionClass_argumentsMatch(const dcArray *_arguments,
                                    const dcList *_specificArguments,
                                    const dcList *_headerArguments)
{
    bool match = true;
    dcListElement *specificArgumentsIt = _specificArguments->head;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    if (_arguments->size == _specificArguments->size)
    {
        uint32_t i = 0;

        for (i = 0; i < _arguments->size; i++)
        {
            //
            // eg:
            //
            // for a toy function f defined with the
            // variables a, b, and the values 0 and 1:
            //
            // f(a, b) = f(a - 1) + f(b - 1) + 1
            //   ^^^^ a and b are falseT specific arguments
            //
            // f(0, 1) = 0
            //   ^^^^ 0 and 1 ARE specific arguments
            //
            // call in tin: main> f(10)
            //                      ^^ given argument
            //
            // we want to compare the given argument with the
            // specific arguments and find a match
            //
            // left refers to the given argument
            // right refers to the specific argument
            //

            dcNode *left = dcArray_get(_arguments, i);
            dcNode *right = specificArgumentsIt->object;

            // they are either dcIdentifier or not //
            if (! IS_IDENTIFIER(left) && ! IS_IDENTIFIER(right))
            {
                dcNode *evaluateResult = NULL;

                if (dcClass_hasTemplate(right,
                                        dcNumberClass_getTemplate(),
                                        true))
                {
                    evaluateResult = dcNodeEvaluator_callMethodWithArgument
                        (evaluator,
                         left,
                         dcSystem_getOperatorName(TAFFY_EQUALS),
                         right);
                }
                else
                {
                    // create the scope //
                    dcScope *scope = dcScope_create();
                    dcNode *scopeShell = dcScope_createShell(scope);

                    // <pull> the identifier name out of the header arguments //
                    dcNode *identifierNode =
                        dcList_get(_headerArguments, i);
                    const char *identifierName =
                        dcIdentifier_getName(identifierNode);
                    // </pull> //

                    // populate the scope //
                    dcScope_setObject(scope, left, identifierName, NO_FLAGS);

                    // <call> the method //
                    dcObjectStackList_pushScope(evaluator->objectStackList,
                                                scopeShell);

                    // evaluatedResult should be yes or no //
                    evaluateResult = dcNodeEvaluator_evaluate(evaluator, right);

                    dcObjectStackList_popScope(evaluator->objectStackList,
                                               DC_DEEP);
                    // </call> //
                }

                if (evaluateResult != dcYesClass_getInstance())
                {
                    match = false;
                    break;
                }
            }

            specificArgumentsIt = specificArgumentsIt->next;
        }
    }
    else
    {
        match = false;
    }

    return match;
}

dcNode *dcFunctionClass_numberOperation(dcNode *_function,
                                        dcNode *_number,
                                        dcTaffyOperator _type,
                                        bool _left)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNodeEvaluator_incrementAbortDelay(evaluator);

    dcNode *result = dcNode_copy(_function, DC_DEEP);
    dcNode *body = dcFunctionClass_getBody(result);
    dcNode *contents = dcGraphDataTree_getContents(body);
    dcNode *first = dcNode_setTemplate(contents, true);
    dcNode *second = dcNode_setTemplate(dcNode_copy(_number, DC_DEEP), true);
    dcNode *newBody = (dcNode_setTemplate
                       (dcFlatArithmetic_snip
                        (dcFlatArithmetic_createNodeWithValues
                         (_type,
                          (_left ? first : second),
                          (_left ? second : first),
                          NULL),
                         NULL),
                        true));
    CAST_GRAPH_DATA_TREE(body)->head = newBody;
    dcProcedureClass_setBody(dcClass_getSuperNode(result), body);

    // registering after body is set is very important
    dcNode_register(result);

    if (dcNodeEvaluator_decrementAbortDelay(evaluator))
    {
        result = NULL;
    }

    return result;
}

dcString *dcFunctionClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    return dcMarshaller_marshall(_stream,
                                 "l",
                                 CAST_FUNCTION_AUX(_node)->specificValues);
}

bool dcFunctionClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcList *specificValues = NULL;

    if (dcMarshaller_unmarshallNoNull(_stream, "l", &specificValues))
    {
        result = true;
        CAST_CLASS_AUX(_node) = createAux(specificValues);
    }

    return result;
}

dcNode *equals(dcNode *_receiver,
               dcNode *_rightObject,
               dcNode *_delta)
{
    dcNode *result = dcNoClass_getInstance();

    dcClass_lock(_rightObject);

    if (_rightObject == _receiver)
    {
        result = dcYesClass_getInstance();
    }
    else if (dcFunctionClass_isMe(_rightObject))
    {
        dcTaffyOperator listComparison = TAFFY_UNKNOWN_OPERATOR;
        dcNode *left = dcFunctionClass_getGraphDataBody(_receiver);
        dcNode *right = dcFunctionClass_getGraphDataBody(_rightObject);
        dcTaffyOperator comparison = TAFFY_LESS_THAN;
        dcList *leftArguments = (dcMethodHeader_getArguments
                                 (dcFunctionClass_getMethodHeader(_receiver)));
        dcList *rightArguments = (dcMethodHeader_getArguments
                                  (dcFunctionClass_getMethodHeader
                                   (_rightObject)));

        if ((leftArguments == NULL && rightArguments != NULL)
            || (leftArguments != NULL && rightArguments == NULL))
        {
            result = dcNoClass_getInstance();
        }
        else
        {
            dcResult headerResult = TAFFY_SUCCESS;
            comparison = TAFFY_EQUALS;

            if (leftArguments != NULL && rightArguments != NULL)
            {
                headerResult = dcList_compare(leftArguments,
                                              rightArguments,
                                              &comparison);
            }

            if (headerResult == TAFFY_EXCEPTION)
            {
                return NULL;
            }

            dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
            dcNodeEvaluator_incrementAbortDelay(evaluator);

            if (headerResult == TAFFY_SUCCESS
                && comparison == TAFFY_EQUALS
                && ((_delta != NULL
                     && dcFlatArithmetic_deltaEquals(left, right, _delta))
                    || (_delta == NULL
                        && dcFlatArithmetic_equals(left, right)))
                && (dcList_compare(CAST_FUNCTION_AUX(_receiver)->specificValues,
                                   CAST_FUNCTION_AUX
                                   (_rightObject)->specificValues,
                                   &listComparison)
                    == TAFFY_SUCCESS)
                && listComparison == TAFFY_EQUALS)
            {
                result = dcYesClass_getInstance();
            }

            if (dcNodeEvaluator_decrementAbortDelay(evaluator))
            {
                result = NULL;
            }
        }
    }

    dcClass_unlock(_rightObject);
    return result;
}

dcNode *dcFunctionClass_equals(dcNode *_receiver, dcArray *_arguments)
{
    return equals(_receiver, dcArray_get(_arguments, 0), NULL);
}

dcNode *dcFunctionClass_deltaEquals(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *array = dcArrayClass_getObjects(dcArray_get(_arguments, 0));
    return equals(_receiver, array->objects[0], array->objects[1]);
}

dcMethodHeader *dcFunctionClass_getMethodHeader(dcNode *_function)
{
    return (dcProcedureClass_getMethodHeader
            (dcClass_getOrInstantiateSuperNode(_function)));
}

dcNode *dcFunctionClass_argumentCount(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32u
             (dcMethodHeader_getArguments
              (dcFunctionClass_getMethodHeader(_receiver))
              ->size)));
}

uint32_t dcFunctionClass_argumentCountHelper(dcNode *_receiver)
{
    return (dcMethodHeader_getArguments
            (dcFunctionClass_getMethodHeader(_receiver))
            ->size);
}

uint32_t dcFunctionClass_getFunctionId(dcNode *_function)
{
    return CAST_FUNCTION_AUX(_function)->functionId;
}

dcTaffy_createIsMeFunctionMacro(dcFunctionClass);
