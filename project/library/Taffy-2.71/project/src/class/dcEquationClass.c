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

#include "dcFunctionClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcEquationClass.h"
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
#include "dcObjectClass.h"
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

TAFFY_C_METHOD(dcEquationMetaClass_createWithLeftRight);

TAFFY_C_METHOD(dcEquationClass_asString);
TAFFY_C_METHOD(dcEquationClass_left);
TAFFY_C_METHOD(dcEquationClass_right);
TAFFY_C_METHOD(dcEquationClass_setLeft);
TAFFY_C_METHOD(dcEquationClass_setRight);
TAFFY_C_METHOD(dcEquationClass_solve);
TAFFY_C_METHOD(dcEquationClass_solveFor);

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "createWithLeft:right:",
        SCOPE_DATA_PUBLIC,
        &dcEquationMetaClass_createWithLeftRight,
        gCFunctionArgument_functionFunction
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
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcEquationClass_asString,
        gCFunctionArgument_none
    },
    {
        "left",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcEquationClass_left,
        gCFunctionArgument_none
    },
    {
        "right",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcEquationClass_right,
        gCFunctionArgument_none
    },
    {
        "setLeft",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcEquationClass_setLeft,
        gCFunctionArgument_function
    },
    {
        "setRight",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcEquationClass_setRight,
        gCFunctionArgument_function
    },
    {
        "solve",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ),
        &dcEquationClass_solve,
        gCFunctionArgument_none
    },
    {
        "solve:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ),
        &dcEquationClass_solveFor,
        gCFunctionArgument_symbol
    },
    {
        0
    }
};

// standard functions //
ALLOCATE_FUNCTION(dcEquationClass_allocateNode);
DEINITIALIZE_FUNCTION(dcEquationClass_deinitialize);
COPY_FUNCTION(dcEquationClass_copyNode);
FREE_FUNCTION(dcEquationClass_freeNode);
INITIALIZE_FUNCTION(dcEquationClass_initialize);
MARSHALL_FUNCTION(dcEquationClass_marshallNode);
UNMARSHALL_FUNCTION(dcEquationClass_unmarshallNode);

#define CAST_EQUATION_AUX(_node_)                       \
    ((dcEquationClassAux *)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcEquationClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (EQUATION_PACKAGE_NAME,                   // package name
          EQUATION_CLASS_NAME,                     // class name
          MAKE_FULLY_QUALIFIED(OBJECT),            // super type
          (CLASS_ATOMIC
           | CLASS_ABSTRACT
           | CLASS_HAS_READ_WRITE_LOCK),           // class flags
          NO_FLAGS,                                // scope data flags
          sMetaMethodWrappers,                     // meta methods
          sMethodWrappers,                         // methods
          NULL,                                    // initialization function
          NULL,                                    // deinitialization function
          &dcEquationClass_allocateNode,           // allocate
          NULL,                                    // deallocate
          NULL,                                    // meta mark
          NULL,                                    // mark
          &dcEquationClass_copyNode,               // copy
          &dcEquationClass_freeNode,               // free
          NULL,                                    // register
          &dcEquationClass_marshallNode,           // marshall
          &dcEquationClass_unmarshallNode,         // unmarshall
          NULL));                                  // set template
}

dcTaffy_createIsMeFunctionMacro(dcEquationClass);

static dcEquationClassAux *createAux(dcNode *_left, dcNode *_right)
{
    dcEquationClassAux *aux =
        (dcEquationClassAux *)dcMemory_allocate(sizeof(dcEquationClassAux));
    aux->left = _left;
    aux->right = _right;
    return aux;
}

dcNode *dcEquationClass_createObject(dcNode *_left, dcNode *_right)
{
    return (dcClass_createNode
            (sTemplate,
             NULL, // supernode
             NULL, // scope
             true, // object
             createAux(_left, _right)));
}

void dcEquationClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = (createAux
                             (dcNumberClass_createObjectFromInt32u(0),
                              dcNumberClass_createObjectFromInt32u(0)));
}

void dcEquationClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcEquationClassAux *aux = CAST_EQUATION_AUX(_from);
    CAST_CLASS_AUX(_to) = createAux(dcNode_copy(aux->left, DC_DEEP),
                                    dcNode_copy(aux->right, DC_DEEP));
}

void dcEquationClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcEquationClassAux *aux = CAST_EQUATION_AUX(_node);
    dcNode_free(&aux->left, DC_DEEP);
    dcNode_free(&aux->right, DC_DEEP);
    dcMemory_free(aux);
}

dcString *dcEquationClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    return dcMarshaller_marshall(_stream,
                                 "nn",
                                 CAST_EQUATION_AUX(_node)->left,
                                 CAST_EQUATION_AUX(_node)->right);
}

bool dcEquationClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcNode *left;
    dcNode *right;

    if (dcMarshaller_unmarshallNoNull(_stream,
                                      "nn",
                                      &left,
                                      &right))
    {
        result = true;
        CAST_CLASS_AUX(_node) = createAux(left, right);
    }

    return result;
}

/////////////////////
//                 //
// Taffy C Methods //
//                 //
/////////////////////

dcNode *dcEquationMetaClass_createWithLeftRight(dcNode *_receiver,
                                                dcArray *_arguments)
{
    dcNode *leftBody =
        dcFunctionClass_getGraphDataBody(dcArray_get(_arguments, 0));
    dcNode *rightBody =
        dcFunctionClass_getGraphDataBody(dcArray_get(_arguments, 1));
    return (dcNode_register
            (dcEquationClass_createObject(dcNode_copy(leftBody, DC_DEEP),
                                          dcNode_copy(rightBody, DC_DEEP))));
}

dcNode *dcEquationClass_left(dcNode *_receiver, dcArray *_arguments)
{
    return CAST_EQUATION_AUX(_receiver)->left;
}

dcNode *dcEquationClass_right(dcNode *_receiver, dcArray *_arguments)
{
    return CAST_EQUATION_AUX(_receiver)->right;
}

dcNode *dcEquationClass_setLeft(dcNode *_receiver, dcArray *_arguments)
{
    CAST_EQUATION_AUX(_receiver)->left = dcArray_get(_arguments, 0);
    return _receiver;
}

dcNode *dcEquationClass_setRight(dcNode *_receiver, dcArray *_arguments)
{
    CAST_EQUATION_AUX(_receiver)->right = dcArray_get(_arguments, 0);
    return _receiver;
}

dcNode *dcEquationClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    dcString display;
    dcString_initialize(&display, 50);
    dcString_append(&display, "#equation(");

    dcEquationClassAux *aux = CAST_EQUATION_AUX(_receiver);
    dcString_append(&display, "%s", dcNode_display(aux->left));
    dcString_append(&display, " = ");
    dcString_append(&display, "%s)", dcNode_display(aux->right));

    return dcNode_register(dcStringClass_createObject(display.string, false));
}

static dcNode *solveHelper(dcNode *_receiver, const char *_symbol)
{
    dcEquationClassAux *aux = CAST_EQUATION_AUX(_receiver);
    dcNode *result = dcFlatArithmetic_solve(_symbol,
                                            aux->left,
                                            aux->right);
    if (result == NULL)
    {
        result = dcNoClass_getInstance();
    }
    else
    {
        dcList *arguments = dcList_create();
        dcList_push(arguments, dcIdentifier_createNode(_symbol, NO_FLAGS));
        result = (dcNode_register
                  (dcFunctionClass_createObjectWithArguments(arguments,
                                                             result)));
    }

    return result;
}

dcNode *dcEquationClass_solve(dcNode *_receiver, dcArray *_arguments)
{
    return solveHelper(_receiver, "x");
}

dcNode *dcEquationClass_solveFor(dcNode *_receiver, dcArray *_arguments)
{
    return solveHelper(_receiver,
                       dcSymbolClass_getString_helper
                       (dcArray_get(_arguments, 0)));
}
