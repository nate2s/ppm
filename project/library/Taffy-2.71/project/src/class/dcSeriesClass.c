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

#include "CompiledSeries.h"

#include "dcSeriesClass.h"
#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcContainers.h"
#include "dcError.h"
#include "dcFunctionClass.h"
#include "dcFunctionRepeater.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcHashClass.h"
#include "dcIdentifier.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNilClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcNumberClass.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcYesClass.h"

// meta methods //
TAFFY_C_METHOD(dcSeriesMetaClass_createWithBlock);

// methods //
TAFFY_C_METHOD(dcSeriesClass_asString);
TAFFY_C_METHOD(dcSeriesClass_evaluateFromTo);
TAFFY_C_METHOD(dcSeriesClass_execute);

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "createWithBlock:",
        SCOPE_DATA_PUBLIC,
        &dcSeriesMetaClass_createWithBlock,
        gCFunctionArgument_block,
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        SCOPE_DATA_PUBLIC,
        &dcSeriesClass_asString,
        gCFunctionArgument_none
    },
    {
        "evaluateFrom:to:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH),
        &dcSeriesClass_evaluateFromTo,
        gCFunctionArgument_numberNumber
    },
    {
        "#operator([...]):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH),
        &dcSeriesClass_execute,
        gCFunctionArgument_array
    },
    {
        0
    }
};

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcSeriesClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (SERIES_PACKAGE_NAME,                // package name
          SERIES_CLASS_NAME,                  // class name
          MAKE_FULLY_QUALIFIED(PROCEDURE),    // super name
          CLASS_ATOMIC,                       // class flags
          NO_FLAGS,                           // scope data flags
          sMetaMethodWrappers,                // meta methods
          sMethodWrappers,                    // methods
          &dcSeriesClass_initialize,          // initialization sequence
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
};

#define SERIES_CLASS_FILE_NAME "Series.ty"

void dcSeriesClass_initialize(void)
{
    dcError_assert(dcStringEvaluator_evalString(__compiledSeries,
                                                SERIES_CLASS_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

dcNode *dcSeriesClass_createNode(dcNode *_body,
                                 dcMethodHeader *_header,
                                 bool _object)
{
    return (dcClass_createNode
            (sTemplate,
             dcProcedureClass_createNode(_body, _header, _object),
             NULL, // scope
             _object,
             NULL));
}

dcNode *dcSeriesClass_createObject(dcNode *_body, dcMethodHeader *_header)
{
    return dcSeriesClass_createNode(_body, _header, true);
}

dcNode *dcSeriesClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    return dcFunctionClass_asString(_receiver, _arguments);
}

dcNode *dcSeriesMetaClass_createWithBlock(dcNode *_receiver,
                                          dcArray *_arguments)
{
    dcNode *block = dcArray_get(_arguments, 0);
    dcNode *result = (dcNode_register
                      (dcSeriesClass_createObject
                       (dcGraphDataTree_createNode
                        (dcNode_setTemplate
                         (dcNode_copy(dcBlockClass_getBody(block), DC_DEEP),
                          true)),
                        dcMethodHeader_copy
                        (dcProcedureClass_getMethodHeader
                         (dcClass_castNode(block,
                                           dcProcedureClass_getTemplate(),
                                           true)),
                         DC_DEEP))));

    if (dcNodeEvaluator_callMethod(dcSystem_getCurrentNodeEvaluator(),
                                   result,
                                   "init")
        == NULL)
    {
        result = NULL;
    }

    return result;
}

static dcResult getMemory(dcNode *_series, dcHash **_memory)
{
    dcResult result = TAFFY_SUCCESS;
    *_memory = NULL;

    if (dcClass_getObject(_series, "@useMemory")
        == dcYesClass_getInstance())
    {
        dcNode *hashNode =
            dcClass_castNode
            (dcClass_getObject(_series, "@memory"),
             dcHashClass_getTemplate(),
             true);

        if (hashNode == NULL)
        {
            result = TAFFY_EXCEPTION;
        }
        else
        {
            *_memory = dcHashClass_getHash(hashNode);
        }
    }

    return result;
}

TAFFY_HIDDEN bool seriesExecuteFunction(dcNode *_series,
                                        dcNode *_result,
                                        dcNode **_token)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    bool result = true;

    if (*_token == NULL)
    {
        *_token = dcNode_register(dcNode_copy(_result, DC_DEEP));
        // for marking
        dcError_assert(dcClass_set(_series, "@token", *_token, NO_FLAGS, true));
    }
    else
    {
        if (dcNumberClass_isMe(*_token)
            && dcNumberClass_isMe(_result))
        {
            dcNumberClass_inlineAdd(*_token, _result);
        }
        else if (dcNodeEvaluator_callMethodWithArgument
                 (evaluator,
                  *_token,
                  dcSystem_getOperatorName(TAFFY_PLUS_EQUAL),
                  _result)
                 == NULL)
        {
            result = false;
        }
    }

    return result;
}

dcNode *dcSeriesClass_execute(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *tempResult = NULL;
    dcNode *result = NULL;
    dcHash *memory = NULL;

    if (getMemory(_receiver, &memory) != TAFFY_EXCEPTION
        && dcFunctionRepeater_execute(_receiver,
                                      _arguments,
                                      memory,
                                      &seriesExecuteFunction,
                                      &tempResult))
    {
        result = tempResult;
    }

    dcClass_set(_receiver, "@token", dcNilClass_getInstance(), NO_FLAGS, true);
    return result;
}

dcNode *dcSeriesClass_evaluateFromTo(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *tempResult = NULL;
    dcNode *result = NULL;
    dcHash *memory = NULL;

    if (getMemory(_receiver, &memory) != TAFFY_EXCEPTION
        && dcFunctionRepeater_executeHelper(_receiver,
                                            dcArray_get(_arguments, 0),
                                            dcArray_get(_arguments, 1),
                                            memory,
                                            &seriesExecuteFunction,
                                            &tempResult))
    {
        result = tempResult;
    }

    dcClass_set(_receiver, "@token", dcNilClass_getInstance(), NO_FLAGS, true);
    return result;
}
