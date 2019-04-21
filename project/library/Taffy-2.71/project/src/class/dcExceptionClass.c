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

#include "dcArray.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcExceptionClass.h"
#include "dcExceptions.h"
#include "dcNode.h"
#include "dcGraphData.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcListClass.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNodeEvaluator.h"
#include "dcNilClass.h"
#include "dcObjectClass.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_NO_CAST
         | SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcExceptionClass_asString,
        gCFunctionArgument_none
    },
    {
        "data",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcExceptionClass_data,
        gCFunctionArgument_none
    },
    {
        "init",
        SCOPE_DATA_PUBLIC,
        &dcExceptionClass_init,
        gCFunctionArgument_none
    },
    {
        "setData:",
        (SCOPE_DATA_NO_CAST
         | SCOPE_DATA_PROTECTED),
        &dcExceptionClass_setData,
        gCFunctionArgument_string
    },
    {
        0
    }
};

#define CAST_EXCEPTION_AUX(_node_)                      \
    ((dcExceptionClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcExceptionClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (EXCEPTION_PACKAGE_NAME,             // package name
          EXCEPTION_CLASS_NAME,               // class name
          MAKE_FULLY_QUALIFIED(OBJECT),       // super type
          NO_FLAGS,                           // class flags
          NO_FLAGS,                           // scope data flags
          NULL,                               // meta methods
          sMethodWrappers,                    // methods
          &dcExceptionClass_initialize,       // class initialization function
          NULL,                               // class deinitialization function
          &dcExceptionClass_allocateNode,     // allocate
          NULL,                               // deallocate
          NULL,                               // meta mark
          &dcExceptionClass_markNode,         // mark
          &dcExceptionClass_copyNode,         // copy
          &dcExceptionClass_freeNode,         // free
          &dcExceptionClass_registerNode,     // register
          &dcExceptionClass_marshallNode,     // marshall
          &dcExceptionClass_unmarshallNode,   // unmarshall
          NULL));                             // set template
}

void dcExceptionClass_initialize(void)
{
    dcExceptions_create();
}

static dcExceptionClassAux *createAux(dcNode *_data)
{
    dcExceptionClassAux *aux =
        (dcExceptionClassAux *)dcMemory_allocate(sizeof(dcExceptionClassAux));
    aux->data = _data;
    return aux;
}

void dcExceptionClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux(dcNilClass_getInstance());
}

void dcExceptionClass_copyNode(dcNode *_to,
                               const dcNode *_from,
                               dcDepth _depth)
{
    dcExceptionClassAux *toAux =
        (dcExceptionClassAux *)dcMemory_allocate(sizeof(dcExceptionClassAux));
    dcExceptionClassAux *fromAux = CAST_EXCEPTION_AUX(_from);
    toAux->data = fromAux->data;
    CAST_CLASS_AUX(_to) = toAux;
}

void dcExceptionClass_freeNode(dcNode *_exception, dcDepth _depth)
{
    dcExceptionClassAux *aux = CAST_EXCEPTION_AUX(_exception);

    if (dcNode_isTemplate(_exception)
        && aux->data != NULL
        && dcNode_isTemplate(aux->data))
    {
        dcNode_free(&aux->data, DC_DEEP);
    }

    dcMemory_free(aux);
}

dcNode *dcExceptionClass_createNode(dcNode *_data, bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL, // super
                              NULL, // scope
                              _object,
                              createAux(_data));
}

dcNode *dcExceptionClass_createObjectFromString(const char *_data)
{
    return dcExceptionClass_createObject
        (dcStringClass_createObject(_data, true));
}

dcNode *dcExceptionClass_getData(const dcNode *_exception)
{
    return CAST_EXCEPTION_AUX(_exception)->data;
}

void dcExceptionClass_markNode(dcNode *_exceptionNode)
{
    dcNode_mark(dcExceptionClass_getData(_exceptionNode));
}

void dcExceptionClass_registerNode(dcNode *_object)
{
    dcNode_register(dcExceptionClass_getData(_object));
}

//
// dcExceptionClass_asString
//
// the 'asString Taffy method for the Exception class
//
dcNode *dcExceptionClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *casted = dcClass_castNode(_receiver, sTemplate, true);
    dcNode *result = NULL;

    if (casted != NULL)
    {
        dcExceptionClassAux *aux = CAST_EXCEPTION_AUX(casted);
        const char *exceptionString = NULL;

        if (aux->data != NULL)
        {
            exceptionString = dcStringClass_asString_helper(aux->data);
        }

        char *display = dcLexer_sprintf("%s%s%s",
                                        dcClass_getName(_receiver),
                                        (exceptionString == NULL
                                         ? ""
                                         : ": "),
                                        (exceptionString == NULL
                                         ? ""
                                         : exceptionString));
        result = dcNode_register(dcStringClass_createObject(display, false));
    }
    // else an exception is already generated //

    return result;
}

// Taffy c functions //
dcNode *dcExceptionClass_data(dcNode *_receiver, dcArray *_arguments)
{
    return dcExceptionClass_getData(_receiver);
}

dcNode *dcExceptionClass_init(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *casted = dcClass_castNodeWithAssert(_receiver,
                                                sTemplate,
                                                false,
                                                true);
    dcExceptionClassAux *aux = CAST_EXCEPTION_AUX(casted);

    if (aux == NULL)
    {
        aux = ((dcExceptionClassAux *)dcMemory_allocate
               (sizeof(dcExceptionClassAux)));
        CAST_CLASS_AUX(casted) = aux;
    }

    aux->data = dcNode_register(dcStringClass_createObject(":)", true));
    return _receiver;
}

dcNode *dcExceptionClass_setData(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *receiver = dcClass_castNode(_receiver, sTemplate, true);

    if (receiver != NULL)
    {
        dcExceptionClassAux *aux = CAST_EXCEPTION_AUX(receiver);

        // set the data //
        aux->data = dcArray_get(_arguments, 0);
    }

    return receiver;
}

dcNode *dcExceptionMetaClass_newWithData(dcNode *_receiver, dcArray *_arguments)
{
    return dcExceptionClass_createObject(dcArray_get(_arguments, 0));
}

bool dcExceptionClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    dcNode *data;
    bool result = false;

    if (dcMarshaller_unmarshallNoNull(_stream, "g", &data))
    {
        CAST_CLASS_AUX(_node) = createAux(data);
        result = true;
    }

    return result;
}

dcString *dcExceptionClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcClass_lock((dcNode *)_node);
    dcString *result = dcMarshaller_marshall(_stream,
                                             "n",
                                             CAST_EXCEPTION_AUX(_node)->data);
    dcClass_unlock((dcNode *)_node);
    return result;
}
