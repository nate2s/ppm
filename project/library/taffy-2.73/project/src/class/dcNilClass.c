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

#include "CompiledNil.h"
#include "dcNilClass.h"
#include "dcArray.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcObjectClass.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcYesClass.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "#operator(==):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcNilClass_equals,
        gCFunctionArgument_wild
    },
    {
        0
    }
};

INITIALIZE_FUNCTION(dcNilClass_initialize);

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcNilClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (NIL_PACKAGE_NAME,                  // package name
          NIL_CLASS_NAME,                    // class name
          MAKE_FULLY_QUALIFIED(OBJECT),      // super name
          NO_FLAGS,                          // class flags
          NO_FLAGS,                          // scope data flags
          NULL,                              // meta methods
          sMethodWrappers,                   // methods
          &dcNilClass_initialize,            // initialization function
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

static dcNode *sTheNil = NULL;

dcNode *dcNilClass_getInstance(void)
{
    return sTheNil;
}

void dcNilClass_resetInstance(void)
{
    sTheNil = dcScope_getObject(CAST_SCOPE(dcSystem_getGlobalScope()), "nil");
}

#define NIL_CLASS_TAFFY_FILE_NAME "src/class/Nil.ty"

void dcNilClass_initialize(void)
{
    sTheNil = dcNilClass_createObject();
    dcGraphData_setPosition
        (CAST_GRAPH_DATA(sTheNil),
         1,
         dcStringManager_getStringId(NIL_CLASS_TAFFY_FILE_NAME));
    dcSystem_addToGlobalScope(sTheNil, "nil");
    dcClassManager_registerSingleton(sTheNil, "nil");
    dcError_assert(dcStringEvaluator_evalString(__compiledNil,
                                                NIL_CLASS_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

dcNode *dcNilClass_createNode(bool _object)
{
    return dcClass_createBasicNode(sTemplate, _object);
}

dcNode *dcNilClass_createObject(void)
{
    return dcNilClass_createNode(true);
}

bool dcNilClass_isMe(const struct dcNode_t *_node)
{
    return dcClass_hasTemplate(_node, sTemplate, true);
}

//
// dcNilClass_equals()
//
// C implementation of a Nil object's #operator(==): method
//
dcNode *dcNilClass_equals(dcNode *_receiver, dcArray *_arguments)
{
    return (dcClass_hasTemplate(_receiver, sTemplate, true)
            && dcClass_hasTemplate(dcArray_get(_arguments, 0),
                                   sTemplate,
                                   true)
            ? dcYesClass_getInstance()
            : dcNoClass_getInstance());
}
