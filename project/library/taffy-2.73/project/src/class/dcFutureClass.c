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

#include "CompiledFuture.h"

#include "dcFutureClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcCondition.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcFutureManager.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcSystem.h"
#include "dcMarshaller.h"
#include "dcMethodHeader.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNilClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcNumberClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcStringClass.h"
#include "dcTaffyCMethodPointer.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcThread.h"
#include "dcObjectClass.h"

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "createWithBlock:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcFutureMetaClass_createWithBlock,
        gCFunctionArgument_block
    },
    {
        "createWithBlock:withArguments:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcFutureMetaClass_createWithBlockWithArguments,
        gCFunctionArgument_blockArray
    },
    {
        0
    }
};

#define CAST_FUTURE_AUX(_node_) ((dcFutureClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcFutureClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (FUTURE_PACKAGE_NAME,                  // package name
          FUTURE_CLASS_NAME,                    // class name
          "Object",                             // super name
          NO_FLAGS,                             // class flags
          NO_FLAGS,                             // scope data flags
          sMetaMethodWrappers,                  // meta methods
          NULL,                                 // methods
          &dcFutureClass_initialize,            // initialization function
          NULL,                                 // deinitialization function
          &dcFutureClass_allocateNode,          // allocate
          &dcFutureClass_deallocateNode,        // deallocate
          NULL,                                 // meta mark
          &dcFutureClass_markNode,              // mark
          &dcFutureClass_copyNode,              // copy
          &dcFutureClass_freeNode,              // free
          &dcFutureClass_registerNode,          // register
          &dcFutureClass_marshallNode,          // marshall
          &dcFutureClass_unmarshallNode,        // unmarshall
          NULL));                               // set template
}

#define FUTURE_TAFFY_FILE_NAME "src/class/Future.ty"

void dcFutureClass_initialize(void)
{
    dcError_assert(dcStringEvaluator_evalString(__compiledFuture,
                                                FUTURE_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

static dcFutureClassAux *dcFutureClass_createAux(void)
{
    dcFutureClassAux *aux = ((dcFutureClassAux *)dcMemory_allocate
                             (sizeof(dcFutureClassAux)));
    aux->value = NULL;
    aux->valueSet = false;
    aux->valueCondition = dcCondition_create();
    aux->valueMutex = dcMutex_create(false);
    return aux;
}

void dcFutureClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = dcFutureClass_createAux();
}

void dcFutureClass_deallocateNode(dcNode *_node)
{
    CAST_FUTURE_AUX(_node)->value = NULL;
}

dcNode *dcFutureClass_createNode(bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL,    // super
                              NULL,    // scope
                              _object, // object?
                              dcFutureClass_createAux());
}

dcNode *dcFutureClass_createObject(void)
{
    return dcFutureClass_createNode(true);
}

void dcFutureClass_freeNode(dcNode *_futureNode, dcDepth _depth)
{
    dcFutureClassAux *aux = CAST_FUTURE_AUX(_futureNode);

    if (aux != NULL)
    {
        if (dcNode_isTemplate(_futureNode) && aux->value != NULL)
        {
            dcError_assert(! dcNode_isRegistered(aux->value));
            dcNode_tryFree(&aux->value, DC_DEEP);
        }

        dcCondition_free(&aux->valueCondition);
        dcMutex_free(&aux->valueMutex);
        dcMemory_free(aux);
    }
}

void dcFutureClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_CLASS_AUX(_to) = dcFutureClass_createAux();
}

void dcFutureClass_markNode(dcNode *_node)
{
    dcNode_mark(CAST_FUTURE_AUX(_node)->value);
}

void dcFutureClass_registerNode(dcNode *_node)
{
    dcFutureClassAux *aux = CAST_FUTURE_AUX(_node);
    dcNode_register(aux->value);
}

dcNode *dcFutureClass_waitForValue(dcNode *_futureNode)
{
    dcFutureClassAux *aux = CAST_FUTURE_AUX(_futureNode);
    dcMutex_lock(aux->valueMutex);

    while (! aux->valueSet)
    {
        dcGarbageCollector_nodeEvaluatorDown();
        dcCondition_wait(aux->valueCondition, aux->valueMutex);
        dcGarbageCollector_nodeEvaluatorBlockUp();
    }

    dcMutex_unlock(aux->valueMutex);
    return aux->value;
}

void dcFutureClass_setValue(dcNode *_futureNode, dcNode *_value)
{
    dcFutureClassAux *aux = CAST_FUTURE_AUX(_futureNode);
    dcMutex_lock(aux->valueMutex);
    aux->value = _value;
    aux->valueSet = true;
    dcCondition_signal(aux->valueCondition);
    dcMutex_unlock(aux->valueMutex);
}

bool dcFutureClass_isMe(const dcNode *_node)
{
    return dcClass_hasTemplate(_node, sTemplate, true);
}

// taffy methods //
dcNode *dcFutureMetaClass_createWithBlock(dcNode *_receiver,
                                          dcArray *_arguments)
{
    return dcFutureManager_schedule(dcArray_get(_arguments, 0), NULL);
}

dcNode *dcFutureMetaClass_createWithBlockWithArguments(dcNode *_receiver,
                                                       dcArray *_arguments)
{
    return dcFutureManager_schedule(dcArray_get(_arguments, 0),
                                    dcArray_get(_arguments, 1));
}

bool dcFutureClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcNode *value = NULL;

    if (dcMarshaller_unmarshall(_stream, "n", &value))
    {
        CAST_CLASS_AUX(_node) = dcFutureClass_createAux();
        CAST_FUTURE_AUX(_node)->value = value;
        result = true;
    }

    return result;
}

dcString *dcFutureClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    return dcMarshaller_marshall(_stream,
                                 "n",
                                 CAST_FUTURE_AUX(_node)->value);
}
