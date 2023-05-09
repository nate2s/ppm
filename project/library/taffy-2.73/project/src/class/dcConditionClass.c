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
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcCondition.h"
#include "dcConditionClass.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcLexer.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcMutexClass.h"
#include "dcNode.h"
#include "dcNoClass.h"
#include "dcScopeData.h"
#include "dcStringClass.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcObjectClass.h"
#include "dcYesClass.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcConditionClass_asString,
        gCFunctionArgument_none
    },
    {
        "broadcast",
        SCOPE_DATA_PUBLIC,
        &dcConditionClass_broadcast,
        gCFunctionArgument_none
    },
    {
        "signal",
        SCOPE_DATA_PUBLIC,
        &dcConditionClass_signal,
        gCFunctionArgument_none
    },
    {
        "wait:",
        SCOPE_DATA_PUBLIC,
        &dcConditionClass_wait,
        gCFunctionArgument_mutex
    },
    {
        0
    }
};

#define CAST_CONDITION_AUX(_node_)                      \
    ((dcConditionClassAux *)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcConditionClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (CONDITION_PACKAGE_NAME,              // package name
          CONDITION_CLASS_NAME,                // class name
          MAKE_FULLY_QUALIFIED(OBJECT),        // super type
          NO_FLAGS,                            // class flags
          NO_FLAGS,                            // scope data flags
          NULL,                                // meta methods
          sMethodWrappers,                     // methods
          NULL,                                // initialization function
          NULL,                                // deinitialization function
          &dcConditionClass_allocateNode,      // allocate
          NULL,                                // deallocate
          NULL,                                // meta mark
          NULL,                                // mark
          &dcConditionClass_copyNode,          // copy
          &dcConditionClass_freeNode,          // free
          NULL,                                // register
          NULL,                                // marshall
          NULL,                                // unmarshall
          NULL));                              // set template
};

static dcConditionClassAux *createAux(void)
{
    dcConditionClassAux *aux =
        (dcConditionClassAux *)dcMemory_allocate(sizeof(dcConditionClassAux));
    aux->condition = dcCondition_create();
    return aux;
}

void dcConditionClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux();
}

dcNode *dcConditionClass_createNode(bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL, // super
                              NULL, // scope
                              _object,
                              createAux());
}

dcNode *dcConditionClass_createObject(void)
{
    return dcConditionClass_createNode(true);
}

void dcConditionClass_freeNode(dcNode *_conditionNode, dcDepth _depth)
{
    dcConditionClassAux *aux = CAST_CONDITION_AUX(_conditionNode);
    dcCondition_free(&aux->condition);
    dcMemory_free(aux);
}

void dcConditionClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_CLASS_AUX(_to) = createAux();
}

// taffy methods //
dcNode *dcConditionClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcStringClass_createObject
             (dcLexer_sprintf
              ("#Condition(%lu)",
               (unsigned long)(&CAST_CONDITION_AUX(_receiver)->condition)),
              false)));
}

dcNode *dcConditionClass_signal(dcNode *_receiver, dcArray *_arguments)
{
    dcCondition_signal(dcConditionClass_getCondition(_receiver));
    return dcYesClass_getInstance();
}

dcNode *dcConditionClass_broadcast(dcNode *_receiver, dcArray *_arguments)
{
    dcCondition_broadcast(dcConditionClass_getCondition(_receiver));
    return dcYesClass_getInstance();
}

dcNode *dcConditionClass_wait(dcNode *_receiver, dcArray *_arguments)
{
    dcGarbageCollector_nodeEvaluatorDown();
    dcCondition_wait(dcConditionClass_getCondition(_receiver),
                     dcMutexClass_getMutex(dcArray_get(_arguments, 0)));
    dcGarbageCollector_nodeEvaluatorBlockUp();
    return dcYesClass_getInstance();
}

dcCondition *dcConditionClass_getCondition(dcNode *_node)
{
    return CAST_CONDITION_AUX(_node)->condition;
}
