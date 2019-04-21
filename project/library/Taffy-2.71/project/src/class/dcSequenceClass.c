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

#include "CompiledSequence.h"

#include "dcSequenceClass.h"
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
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "createWithBlock:",
        SCOPE_DATA_PUBLIC,
        &dcSequenceMetaClass_createWithBlock,
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
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcSequenceClass_asString,
        gCFunctionArgument_none
    },
    {
        "#operator([...]):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONST),
        &dcSequenceClass_parenthesesDots,
        gCFunctionArgument_array
    },
    {
        0
    }
};

#define CAST_SEQUENCE_AUX(_node_)                   \
    ((dcSequenceClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcSequenceClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (SEQUENCE_PACKAGE_NAME,              // package name
          SEQUENCE_CLASS_NAME,                // class name
          MAKE_FULLY_QUALIFIED(PROCEDURE),    // super name
          CLASS_ATOMIC,                       // class flags
          NO_FLAGS,                           // scope data flags
          sMetaMethodWrappers,                // meta methods
          sMethodWrappers,                    // methods
          &dcSequenceClass_initialize,        // initialization function
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

#define SEQUENCE_CLASS_FILE_NAME "Sequence.ty"

void dcSequenceClass_initialize(void)
{
    dcError_assert(dcStringEvaluator_evalString(__compiledSequence,
                                                SEQUENCE_CLASS_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

dcNode *dcSequenceClass_createNode(dcNode *_body,
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

dcNode *dcSequenceClass_createObject(dcNode *_body, dcMethodHeader *_header)
{
    return dcSequenceClass_createNode(_body, _header, true);
}

dcNode *dcSequenceMetaClass_createWithBlock(dcNode *_receiver,
                                            dcArray *_arguments)
{
    dcNode *block = dcArray_get(_arguments, 0);
    return (dcNode_register
            (dcSequenceClass_createObject
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
}

dcNode *dcSequenceClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    return dcFunctionClass_asString(_receiver, _arguments);
}

TAFFY_HIDDEN bool sequenceExecuteFunction(dcNode *_sequence,
                                          dcNode *_result,
                                          dcNode **_token)
{
    if (*_token == NULL)
    {
        *_token = (dcNode_register
                   (dcArrayClass_createObject
                    (dcArray_createWithObjects(_result, NULL), true)));
        dcError_assert(dcClass_set(_sequence,
                                   "@token",
                                   *_token,
                                   NO_FLAGS,
                                   true));
    }
    else
    {
        dcArray_add(dcArrayClass_getObjects(*_token), _result);
    }

    return true;
}

dcNode *dcSequenceClass_parenthesesDots(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *tempResult = NULL;
    dcNode *result = NULL;

    if (dcFunctionRepeater_execute(_receiver,
                                   _arguments,
                                   NULL,
                                   &sequenceExecuteFunction,
                                   &tempResult))
    {
        result = tempResult;
    }

    dcClass_set(_receiver, "@token", dcNilClass_getInstance(), NO_FLAGS, true);
    return result;
}
