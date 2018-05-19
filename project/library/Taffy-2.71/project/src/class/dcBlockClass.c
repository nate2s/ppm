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

#include "dcBlockClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcGraphData.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcObjectClass.h"
#include "dcObjectStackList.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        SCOPE_DATA_PUBLIC,
        &dcBlockClass_asString,
        gCFunctionArgument_none
    },
    {
        "call",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH),
        &dcBlockClass_call,
        gCFunctionArgument_none
    },
    {
        "callWith:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH),
        &dcBlockClass_callWith,
        gCFunctionArgument_array
    },
    {
        "#operator(()):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH),
        &dcBlockClass_callWith,
        gCFunctionArgument_array
    },
    {
        0
    }
};

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcBlockClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (BLOCK_PACKAGE_NAME,                 // package name
          BLOCK_CLASS_NAME,                   // class name
          MAKE_FULLY_QUALIFIED(PROCEDURE),    // super type
          CLASS_ATOMIC,                       // class flags
          NO_FLAGS,                           // scope data flags
          NULL,                               // meta methods
          sMethodWrappers,                    // methods
          NULL,                               // initialization function
          NULL,                               // deinitialization function
          NULL,                               // allocate
          NULL,                               // deallocate
          NULL,                               // meta mark
          NULL,                               // mark
          NULL,                               // copy
          NULL,                               // free
          NULL,                               // register
          NULL,                               // marshall (in procedure class)
          NULL,                               // unmarshall (in procedure class)
          NULL));                             // set template
}

dcNode *dcBlockClass_createNode(dcNode *_body,
                                dcList *_arguments,
                                bool _object)
{
    return dcClass_createNode(sTemplate,
                              dcProcedureClass_createNode
                              (_body,
                               dcMethodHeader_create(NULL,
                                                     (_arguments == NULL
                                                      ? dcList_create()
                                                      : _arguments)),
                               _object),
                              NULL,
                              _object,
                              NULL);
}

dcNode *dcBlockClass_getBody(dcNode *_blockNode)
{
    return dcProcedureClass_getBody(dcClass_getSuperNode(_blockNode));
}

// taffy methods //
dcNode *dcBlockClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    dcString *display = dcString_create();
    dcString_appendString(display, "#Block {");
    dcNode *node = dcBlockClass_getBody(_receiver);
    bool exception = false;
    dcNode *result = NULL;

    dcList *arguments = (dcMethodHeader_getArguments
                         (dcProcedureClass_getMethodHeader
                          (dcClass_castNodeWithAssert
                           (_receiver,
                            dcProcedureClass_getTemplate(),
                            false,
                            true))));

    if (arguments != NULL && arguments->size > 0)
    {
        dcString_appendString(display, "<");
        dcListElement *that;

        for (that = arguments->head; that != NULL; that = that->next)
        {
            dcNode_print(that->object, display);

            if (that->next != NULL)
            {
                dcString_appendString(display, ", ");
            }
        }

        dcString_appendString(display, "> ");
    }

    while (node != NULL)
    {
        if (dcNode_print(node, display) == TAFFY_EXCEPTION)
        {
            exception = true;
            break;
        }

        node = dcGraphData_getNext(node);

        if (node != NULL)
        {
            dcString_appendString(display, " ; ");
        }
    }

    if (! exception)
    {
        dcString_appendCharacter(display, '}');
        result = dcNode_register
            (dcStringClass_createObject(display->string, false));
        dcString_free(&display, DC_SHALLOW);
    }
    else
    {
        dcString_free(&display, DC_DEEP);
    }

    return result;
}

dcNode *dcBlockClass_call(dcNode *_receiver, dcArray *_arguments)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcList *arguments = dcList_create();
    dcNode *result = (dcNodeEvaluator_evaluateProcedure
                      (evaluator,
                       _receiver, //dcNodeEvaluator_getCurrentSelf(evaluator),
                       dcClass_castNodeWithAssert
                       (_receiver,
                        dcProcedureClass_getTemplate(),
                        false,
                        true),
                       SCOPE_DATA_BREAKTHROUGH,
                       arguments));
    dcList_free(&arguments, DC_DEEP);
    return result;
}

dcNode *dcBlockClass_callWith(dcNode *_receiver, dcArray *_arguments)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    // fix me, derrr
    dcList *arguments = dcList_createFromArray(dcArrayClass_getObjects
                                               (dcArray_get(_arguments, 0)));
    dcNode *result = (dcNodeEvaluator_evaluateProcedure
                      (evaluator,
                       dcNodeEvaluator_getCurrentSelf(evaluator),
                       dcClass_castNodeWithAssert
                       (_receiver,
                        dcProcedureClass_getTemplate(),
                        false,
                        true),
                       SCOPE_DATA_BREAKTHROUGH,
                       arguments));
    dcList_free(&arguments, DC_SHALLOW);
    return result;
}

dcTaffy_createIsMeFunctionMacro(dcBlockClass);
