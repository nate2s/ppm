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

#include "CompiledContainer.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcContainerClass.h"
#include "dcExceptions.h"
#include "dcGraphData.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcObjectClass.h"
#include "dcPairClass.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcContainerClass_asString,
        gCFunctionArgument_none
    },
    {
        0
    }
};

#define CAST_CONTAINER_CLASS_AUX(_node_)            \
    ((dcContainerClassAux*)CAST_CLASS_AUX(_node_))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcContainerClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (CONTAINER_PACKAGE_NAME,               // package name
          CONTAINER_CLASS_NAME,                 // class name
          MAKE_FULLY_QUALIFIED(OBJECT),         // super type
          CLASS_ABSTRACT,                       // class flags
          NO_FLAGS,                             // scope data flags
          NULL,                                 // meta methods
          sMethodWrappers,                      // methods
          &dcContainerClass_initialize,         // initialization function
          NULL,                                 // deinitialization function
          &dcContainerClass_allocateNode,       // allocate
          NULL,                                 // deallocate
          NULL,                                 // meta mark
          NULL,                                 // mark
          &dcContainerClass_copyNode,           // copy
          &dcContainerClass_freeNode,           // free
          NULL,                                 // register
          NULL,                                 // marshall
          NULL,                                 // unmarshall
          NULL));                               // set template
}

static dcContainerClassAux *createAux(void)
{
    return ((dcContainerClassAux *)dcMemory_allocateAndInitialize
            (sizeof(dcContainerClassAux)));
}

void dcContainerClass_allocateNode(dcNode *_object)
{
    CAST_CLASS_AUX(_object) = createAux();
}

dcNode *dcContainerClass_createNode(bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL, // super
                              NULL, // scope
                              _object,
                              createAux());
}

void dcContainerClass_initialize(void)
{
    assert(dcStringEvaluator_evalString(__compiledContainer,
                                        "src/class/ContainerClass.ty",
                                        NO_STRING_EVALUATOR_FLAGS)
           != NULL);
}

void dcContainerClass_copyNode(dcNode *_to,
                               const dcNode *_from,
                               dcDepth _depth)
{
    dcContainerClassAux *fromAux = CAST_CONTAINER_CLASS_AUX(_from);
    dcContainerClassAux *toAux = createAux();
    dcMemory_copy(toAux, fromAux, sizeof(dcContainerClassAux));
    CAST_CLASS_AUX(_to) = toAux;
}

void dcContainerClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcContainerClassAux *aux = CAST_CONTAINER_CLASS_AUX(_node);
    dcMemory_free(aux);
}

dcNode *dcContainerClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    dcString output;
    dcString_initialize(&output, 50);
    dcContainerClassAux *aux = CAST_CONTAINER_CLASS_AUX(_receiver);
    dcString_append(&output,
                    "#Container(modified=%u, loopCount: %lu)",
                    aux->modified,
                    aux->loopCount);
    return dcNode_register(dcStringClass_createObject(output.string, false));
}

bool dcContainerClass_isModified(dcNode *_object)
{
    dcNode *object = dcClass_castNodeWithAssert(_object,
                                                sTemplate,
                                                false,
                                                true);
    dcClass_lock(object);
    bool modified = CAST_CONTAINER_CLASS_AUX(object)->modified;
    dcClass_unlock(object);
    return modified;
}

dcNode *dcContainerClass_setModified(dcNode *_object, bool _modified)
{
    dcNode *object = dcClass_castNodeWithAssert(_object,
                                                sTemplate,
                                                false,
                                                true);
    dcClass_lock(object);
    dcContainerClassAux *aux = CAST_CONTAINER_CLASS_AUX(object);
    bool exception = false;

    if (aux->loopCount > 0)
    {
        exception = true;
        dcNonConstantUseOfConstantExceptionClass_throwObject
            (dcClass_getName(_object));
    }
    else
    {
        aux->modified = _modified;
    }

    dcClass_unlock(object);
    return (exception
            ? NULL
            : _object);
}

bool dcContainerClass_checkModified(dcNode *_object)
{
    bool result = false;

    if (dcContainerClass_isModified(_object))
    {
        // oh dear
        result = true;
        dcNonConstantUseOfConstantExceptionClass_throwObject
            (dcClass_getName(_object));
    }

    return result;
}

void dcContainerClass_startLoop(dcNode *_object)
{
    dcNode *object = dcClass_castNodeWithAssert(_object,
                                                sTemplate,
                                                false,
                                                true);
    dcClass_lock(object);
    dcContainerClassAux *aux = CAST_CONTAINER_CLASS_AUX(object);

    if (aux->loopCount == 0)
    {
        dcContainerClass_setModified(object, false);
    }

    assert(aux->loopCount < 0xFFFF);
    aux->loopCount++;
    dcClass_unlock(object);
}

void dcContainerClass_stopLoop(dcNode *_object)
{
    dcNode *object = dcClass_castNodeWithAssert(_object,
                                                sTemplate,
                                                false,
                                                true);
    dcClass_lock(object);
    dcContainerClassAux *aux = CAST_CONTAINER_CLASS_AUX(object);
    assert(aux->loopCount > 0);
    aux->loopCount--;

    if (aux->loopCount == 0)
    {
        dcContainerClass_setModified(object, false);
    }

    dcClass_unlock(object);
}
