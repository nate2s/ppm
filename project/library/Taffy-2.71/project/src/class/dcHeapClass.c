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

#include "CompiledHeap.h"

#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcExceptionClass.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHeap.h"
#include "dcHeapClass.h"
#include "dcKernelClass.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNilClass.h"
#include "dcNumberClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcObjectClass.h"
#include "dcPairClass.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcYesClass.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "insert:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcHeapClass_insert,
        gCFunctionArgument_wild
    },
    {
        "pop",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcHeapClass_pop,
        gCFunctionArgument_none
    },
    {
        "size",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcHeapClass_size,
        gCFunctionArgument_none
    },
    {
        "verify",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcHeapClass_verify,
        gCFunctionArgument_none
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "newMin",
        SCOPE_DATA_PUBLIC,
        &dcHeapMetaClass_newMin,
        gCFunctionArgument_none
    },
    {
        "newMax",
        SCOPE_DATA_PUBLIC,
        &dcHeapMetaClass_newMax,
        gCFunctionArgument_none
    },
    {
        0
    }
};

#define CAST_HEAP_AUX(_node_) ((dcHeapClassAux*)CAST_CLASS_AUX(_node_))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcHeapClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (HEAP_PACKAGE_NAME,                      // package name
          HEAP_CLASS_NAME,                        // class name
          MAKE_FULLY_QUALIFIED(OBJECT),           // super type
          CLASS_HAS_READ_WRITE_LOCK,              // class flags
          NO_FLAGS,                               // scope data flags
          sMetaMethodWrappers,                    // meta methods
          sMethodWrappers,                        // methods
          &dcHeapClass_initialize,                // initialization function
          NULL,                                   // deinitialization function
          &dcHeapClass_allocateNode,              // allocate
          &dcHeapClass_deallocateNode,            // deallocate
          NULL,                                   // meta mark
          &dcHeapClass_markNode,                  // mark
          &dcHeapClass_copyNode,                  // copy
          &dcHeapClass_freeNode,                  // free
          &dcHeapClass_registerNode,              // register
          NULL,                                   // marshall
          NULL,                                   // unmarshall
          NULL));                                 // set template
};

#define HEAP_TAFFY_FILE_NAME "src/class/Heap.ty"

void dcHeapClass_initialize(void)
{
    assert(dcStringEvaluator_evalString(__compiledHeap,
                                        HEAP_TAFFY_FILE_NAME,
                                        NO_STRING_EVALUATOR_FLAGS)
           != NULL);
}

static dcHeapClassAux *createAux(dcHeap *_objects, bool _initialized)
{
    dcHeapClassAux *aux = (dcHeapClassAux *)(dcMemory_allocate
                                             (sizeof(dcHeapClassAux)));
    aux->objects = (_objects != NULL
                    ? _objects
                    : dcHeap_createWithCapacity(HEAP_MIN, 20));
    aux->initialized = _initialized;
    return aux;
}

void dcHeapClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux(dcHeap_create(HEAP_MIN), true);
}

void dcHeapClass_deallocateNode(dcNode *_node)
{
    dcHeap_clear(dcHeapClass_getObjects(_node), DC_SHALLOW);
}

void dcHeapClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcHeapClassAux *aux = CAST_HEAP_AUX(_node);

    if (aux != NULL)
    {
        dcHeap_free(&aux->objects, DC_DEEP);
        dcMemory_free(aux);
    }
}

void dcHeapClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_CLASS_AUX(_to) = (createAux
                           (dcHeap_copy(CAST_HEAP_AUX(_from)->objects, _depth),
                            CAST_HEAP_AUX(_from)->initialized));
}

dcNode *dcHeapClass_createNode(dcHeap *_objects,
                               bool _initialized,
                               bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL, // super
                              NULL, // scope
                              _object,
                              createAux(_objects, _initialized));
}

dcNode *dcHeapClass_createObject(dcHeap *_objects, bool _initialized)
{
    return dcHeapClass_createNode(_objects, _initialized, true);
}

dcNode *dcHeapClass_createEmptyObject(void)
{
    return dcHeapClass_createNode(dcHeap_create(HEAP_MIN), true, true);
}

void dcHeapClass_markNode(dcNode *_classNode)
{
    dcHeap_mark(dcHeapClass_getObjects(_classNode));
}

void dcHeapClass_registerNode(dcNode *_classNode)
{
    dcHeap_register(dcHeapClass_getObjects(_classNode));
}

dcNode *dcHeapClass_buildEmptyObject(void)
{
    return dcHeapClass_createNode(NULL, false, true);
}

bool dcHeapClass_isInitialized(const dcNode *_heapNode)
{
    return CAST_HEAP_AUX(_heapNode)->initialized;
}

void dcHeapClass_setInitialized(dcNode *_heapNode)
{
    CAST_HEAP_AUX(_heapNode)->initialized = true;
}

dcHeap *dcHeapClass_getObjects(const dcNode *_heapNode)
{
    return CAST_HEAP_AUX(_heapNode)->objects;
}

void dcHeapClass_setObjects(dcNode *_heapNode,
                            dcHeap *_objects,
                            dcDepth _depth)
{
    dcHeapClassAux *aux = CAST_HEAP_AUX(_heapNode);
    dcHeap_free(&(aux->objects), _depth);
    aux->objects = _objects;
}

dcNode *dcHeapMetaClass_newMin(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcHeapClass_createObject
             (dcHeap_create(HEAP_MIN), true)));
}

dcNode *dcHeapMetaClass_newMax(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcHeapClass_createObject
             (dcHeap_create(HEAP_MAX), true)));
}

dcNode *dcHeapClass_insert(dcNode *_receiver, dcArray *_arguments)
{
    dcHeap_insert(dcHeapClass_getObjects(_receiver),
                  dcArray_get(_arguments, 0));
    return (dcNodeEvaluator_hasException(dcSystem_getCurrentNodeEvaluator())
            ? NULL
            : _receiver);
}

dcNode *dcHeapClass_pop(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *popResult;
    dcResult result = dcHeap_pop(dcHeapClass_getObjects(_receiver), &popResult);
    return (result == TAFFY_SUCCESS
            ? popResult
            : (result == TAFFY_FAILURE
               ? dcNilClass_getInstance()
               : NULL)); // exception
}

dcNode *dcHeapClass_size(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32u
             (dcHeapClass_getObjects(_receiver)->objects->size)));
}

dcNode *dcHeapClass_verify(dcNode *_receiver, dcArray *_arguments)
{
    return (dcHeap_verify(dcHeapClass_getObjects(_receiver))
            ? dcYesClass_getInstance()
            : dcNoClass_getInstance());
}
