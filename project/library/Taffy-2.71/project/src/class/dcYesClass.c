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

#include "dcYesClass.h"
#include "dcArray.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcGraphData.h"
#include "dcKernelClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcObjectClass.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcStringClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcYesClass_asString,
        gCFunctionArgument_none
    },
    {
        "#operator(==):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcYesClass_equals,
        gCFunctionArgument_wild
    },
    {
        "#prefixOperator(!)",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcYesClass_negate,
        gCFunctionArgument_none
    },
    {
        0
    }
};

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcYesClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (YES_PACKAGE_NAME,                  // package name
          YES_CLASS_NAME,                    // class name
          MAKE_FULLY_QUALIFIED(OBJECT),      // super name
          CLASS_SINGLETON | CLASS_FINAL,     // class flags
          NO_FLAGS,                          // scope data flags
          NULL,                              // meta methods
          sMethodWrappers,                   // methods
          &dcYesClass_initialize,            // initialization function
          NULL,                              // deinitialization function
          NULL,                              // allocate
          NULL,                              // deallocate
          NULL,                              // meta mark
          NULL,                              // mark
          NULL,                              // copy
          NULL,                              // free
          NULL,                              // register
          NULL,                              // marshall
          NULL,                              // unmarshall
          NULL));                            // set template
}

static dcNode *sTheYes = NULL;

dcNode *dcYesClass_getInstance(void)
{
    return sTheYes;
}

void dcYesClass_resetInstance(void)
{
    sTheYes = dcScope_getObject(CAST_SCOPE(dcSystem_getGlobalScope()), "yes");
}

dcNode *dcYesClass_createNode(bool _object)
{
    return dcClass_createBasicNode(sTemplate, _object);
}

dcNode *dcYesClass_createObject(void)
{
    return dcYesClass_createNode(true);
}

void dcYesClass_initialize(void)
{
    sTheYes = dcYesClass_createObject();
    dcSystem_addToGlobalScope(sTheYes, "yes");
    dcClassManager_registerSingleton(sTheYes, "yes");
}

dcNode *dcYesClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    return dcNode_register(dcStringClass_createObject("true", true));
}

dcNode *dcYesClass_equals(dcNode *_receiver, dcArray *_arguments)
{
    return (dcClass_hasTemplate(dcArray_get(_arguments, 0),
                                sTemplate,
                                true)
            ? dcYesClass_getInstance()
            : dcNoClass_getInstance());
}

dcNode *dcYesClass_negate(dcNode *_receiver, dcArray *_arguments)
{
    return dcNoClass_getInstance();
}

bool dcYesClass_isMe(const dcNode *_node)
{
    return dcClass_hasTemplate(_node, sTemplate, true);
}
