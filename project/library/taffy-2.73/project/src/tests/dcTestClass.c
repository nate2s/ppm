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

#include "dcArray.h"
#include "dcClassManager.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcCFunctionArgument.h"
#include "dcCommandLineArguments.h"
#include "dcFileEvaluator.h"
#include "dcGraphData.h"
#include "dcKernelClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcObjectClass.h"
#include "dcScopeData.h"
#include "dcStringManager.h"
#include "dcSymbolClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"
#include "dcYesClass.h"
#include "dcTestClass.h"

static dcNode *operatorEqualEqual(dcNode *_reciever, dcArray *_arguments);
static dcNode *operatorPlus(dcNode *_reciever, dcArray *_arguments);
static dcNode *instanceTestFunction(dcNode *_reciever, dcArray *_arguments);
static dcNode *metaTestFunction(dcNode *_reciever, dcArray *_arguments);

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "metaTestFunction",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_META),
        &metaTestFunction,
        gCFunctionArgument_none
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "#operator(==):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_INSTANCE
         | SCOPE_DATA_BREAKTHROUGH),
        &operatorEqualEqual,
        gCFunctionArgument_wild
    },
    {
        "#operator(+):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_INSTANCE
         | SCOPE_DATA_BREAKTHROUGH),
        &operatorPlus,
        gCFunctionArgument_wild
    },
    {
        "instanceTestFunction",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_INSTANCE
         | SCOPE_DATA_BREAKTHROUGH),
        &instanceTestFunction,
        gCFunctionArgument_wild
    },
    {
        0
    }
};

static void initialize(void)
{
    assert(dcFileEvaluator_evaluateFile("src/tests/TestClass.ty")
           != NULL);
}

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcTestClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         ("org.taffy.tests",                    // package name
          "TestClass",                          // class name
          MAKE_FULLY_QUALIFIED(OBJECT),         // super name
          NO_FLAGS,                             // class flags
          NO_FLAGS,                             // scope data flags
          sMetaMethodWrappers,                  // meta methods
          sMethodWrappers,                      // methods
          &initialize,                          // initialization function
          NULL,                                 // deinitialization function
          NULL,                                 // allocate
          NULL,                                 // deallocate
          NULL,                                 // meta mark
          NULL,                                 // mark
          NULL,                                 // copy
          NULL,                                 // free
          NULL,                                 // register
          NULL,                                 // marshall
          NULL,                                 // unmarshall
          NULL));                               // set template
}

dcNode *dcTestClass_createObject(void)
{
    dcNode *result = dcClass_createBasicNode(sTemplate, true);
    dcGraphData_setPosition
        (CAST_GRAPH_DATA(result),
         dcStringManager_getStringId("src/tests/dcTestClass.c"),
         1);
    return result;
}

dcNode *dcTestClass_getVariableA(dcNode *_testObject)
{
    return dcNodeEvaluator_callMethod(dcSystem_getCurrentNodeEvaluator(),
                                      _testObject,
                                      "variableA");
}

static dcNode *operatorEqualEqual(dcNode *_receiver, dcArray *_arguments)
{
    return dcKernelClass_getOrCreateSymbol("operatorEqualEqual");
}

static dcNode *operatorPlus(dcNode *_receiver, dcArray *_arguments)
{
    return dcKernelClass_getOrCreateSymbol("operatorPlus");
}

static dcNode *instanceTestFunction(dcNode *_receiver, dcArray *_arguments)
{
    return dcKernelClass_getOrCreateSymbol("instanceTestFunction");
}

static dcNode *metaTestFunction(dcNode *_receiver, dcArray *_arguments)
{
    return dcKernelClass_getOrCreateSymbol("metaTestFunction");
}
