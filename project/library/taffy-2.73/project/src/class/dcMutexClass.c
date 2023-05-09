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

#include "dcMutexClass.h"
#include "dcArray.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcNumberClass.h"
#include "dcObjectClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcYesClass.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcMutexClass_asString,
        gCFunctionArgument_none
    },
    {
        "init",
        SCOPE_DATA_PROTECTED,
        &dcMutexClass_init,
        gCFunctionArgument_none
    },
    {
        "initWithReentrant",
        SCOPE_DATA_PROTECTED,
        &dcMutexClass_initWithReentrant,
        gCFunctionArgument_none
    },
    {
        "lock",
        SCOPE_DATA_PUBLIC,
        &dcMutexClass_lock,
        gCFunctionArgument_none
    },
    {
        "unlock",
        SCOPE_DATA_PUBLIC,
        &dcMutexClass_unlock,
        gCFunctionArgument_none
    },
    {
        0
    }
};

#define CAST_MUTEX_AUX(_node_) ((dcMutexClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcMutexClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (MUTEX_PACKAGE_NAME,                  // package name
          MUTEX_CLASS_NAME,                    // class name
          MAKE_FULLY_QUALIFIED(OBJECT),        // super type
          NO_FLAGS,                            // class flags
          NO_FLAGS,                            // scope data flags
          NULL,                                // meta methods
          sMethodWrappers,                     // methods
          NULL,                                // initialization function
          NULL,                                // deinitialization function
          &dcMutexClass_allocateNode,          // allocate
          NULL,                                // deallocate
          NULL,                                // meta mark
          NULL,                                // mark
          &dcMutexClass_copyNode,              // copy
          &dcMutexClass_freeNode,              // free
          NULL,                                // register
          NULL,                                // marshall
          NULL,                                // unmarshall
          NULL));                              // set template
};

static dcMutexClassAux *createAux(void)
{
    dcMutexClassAux *aux = (dcMutexClassAux *)(dcMemory_allocate
                                               (sizeof(dcMutexClassAux)));
    aux->mutex = NULL;
    return aux;
}

void dcMutexClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux();
}

dcNode *dcMutexClass_createNode(bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL, // super
                              NULL, // scope
                              _object,
                              createAux());
}

dcNode *dcMutexClass_createObject(void)
{
    return dcMutexClass_createNode(true);
}

void dcMutexClass_freeNode(dcNode *_mutexNode, dcDepth _depth)
{
    dcMutexClassAux *aux = CAST_MUTEX_AUX(_mutexNode);
    dcMutex_free(&aux->mutex);
    dcMemory_free(aux);
}

void dcMutexClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_CLASS_AUX(_to) = createAux();
}

// taffy methods //
dcNode *dcMutexClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcStringClass_createObject
             (dcLexer_sprintf
              ("#Mutex(%lu)",
               (unsigned long)(&CAST_MUTEX_AUX(_receiver)->mutex)),
              false)));
}

dcNode *dcMutexClass_lock(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcMutexClassAux *aux = CAST_MUTEX_AUX(_receiver);
    dcGarbageCollector_nodeEvaluatorDown();
    result = dcSystem_convertBoolToNode(dcMutex_lock(aux->mutex));
    dcGarbageCollector_nodeEvaluatorBlockUp();
    return result;
}

dcNode *dcMutexClass_unlock(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcMutexClassAux *aux = CAST_MUTEX_AUX(_receiver);
    dcGarbageCollector_nodeEvaluatorDown();
    result = dcSystem_convertBoolToNode(dcMutex_unlock(aux->mutex));
    dcGarbageCollector_nodeEvaluatorBlockUp();
    return result;
}

dcMutex *dcMutexClass_getMutex(dcNode *_node)
{
    return CAST_MUTEX_AUX(_node)->mutex;
}

static dcNode *init(dcNode *_receiver, bool _reentrant)
{
    dcMutexClassAux *aux = CAST_MUTEX_AUX(_receiver);
    assert(aux->mutex == NULL);
    aux->mutex = dcMutex_create(_reentrant);
    return _receiver;
}

dcNode *dcMutexClass_initWithReentrant(dcNode *_receiver, dcArray *_arguments)
{
    return init(_receiver, true);
}

dcNode *dcMutexClass_init(dcNode *_receiver, dcArray *_arguments)
{
    return init(_receiver, false);
}
