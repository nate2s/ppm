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

#include "dcProcedureClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcSystem.h"
#include "dcMarshaller.h"
#include "dcMethodHeader.h"
#include "dcMemory.h"
#include "dcNilClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcNumberClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcTaffyCMethodPointer.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcObjectClass.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcProcedureClass_asString,
        gCFunctionArgument_none
    },
    {
        0
    }
};

#define CAST_PROCEDURE_AUX(_node_)                      \
    ((dcProcedureClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcProcedureClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (PROCEDURE_PACKAGE_NAME,                  // package name
          PROCEDURE_CLASS_NAME,                    // class name
          "Object",                                // super name
          CLASS_ABSTRACT,                          // class flags
          NO_FLAGS,                                // scope data flags
          NULL,                                    // meta methods
          sMethodWrappers,                         // methods
          NULL,                                    // initialization function
          NULL,                                    // deinitialization function
          &dcProcedureClass_allocateNode,          // allocate
          &dcProcedureClass_deallocateNode,        // deallocate
          NULL,                                    // meta mark
          &dcProcedureClass_markNode,              // mark
          &dcProcedureClass_copyNode,              // copy
          &dcProcedureClass_freeNode,              // free
          &dcProcedureClass_registerNode,          // register
          &dcProcedureClass_marshallNode,          // marshall
          &dcProcedureClass_unmarshallNode,        // unmarshall
          NULL));                                  // set template
}

static dcProcedureClassAux *createAux(dcNode *_body, dcMethodHeader *_header)
{
    dcProcedureClassAux *aux =
        (dcProcedureClassAux *)dcMemory_allocate(sizeof(dcProcedureClassAux));
    aux->body = _body;
    aux->methodHeader = (_header == NULL
                         ? dcMethodHeader_create("", NULL)
                         : _header);
    return aux;
}

void dcProcedureClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux(dcNilClass_getInstance(), NULL);
}

void dcProcedureClass_deallocateNode(dcNode *_node)
{
    CAST_PROCEDURE_AUX(_node)->body = NULL;
}

dcNode *dcProcedureClass_createNode(dcNode *_body,
                                    dcMethodHeader *_header,
                                    bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL,    // super
                              NULL,    // scope
                              _object, // object?
                              createAux(_body, _header));
}

dcNode *dcProcedureClass_createObject(dcNode *_body, dcMethodHeader *_header)
{
    return dcProcedureClass_createNode(_body, _header, true);
}

void dcProcedureClass_freeNode(dcNode *_procedureNode, dcDepth _depth)
{
    dcProcedureClassAux *aux = CAST_PROCEDURE_AUX(_procedureNode);

    if (aux != NULL)
    {
        if (dcNode_isTemplate(_procedureNode))
        {
            dcError_assert(! dcNode_isRegistered(aux->body));
            dcNode_tryFree(&aux->body, DC_DEEP);
        }

        dcMethodHeader_free(&aux->methodHeader, DC_DEEP);
        dcMemory_free(aux);
    }
}

void dcProcedureClass_copyNode(dcNode *_to,
                               const dcNode *_from,
                               dcDepth _depth)
{
    dcProcedureClassAux *aux = CAST_PROCEDURE_AUX(_from);
    CAST_CLASS_AUX(_to) =
        createAux(dcGraphData_copyTree(aux->body),
                  dcMethodHeader_copy(aux->methodHeader, DC_DEEP));
}

void dcProcedureClass_markNode(dcNode *_node)
{
    dcNode_mark(CAST_PROCEDURE_AUX(_node)->body);
}

void dcProcedureClass_registerNode(dcNode *_node)
{
    dcNode_register(dcNode_setTemplate(dcProcedureClass_getBody(_node), false));
}

dcMethodHeader *dcProcedureClass_getMethodHeader(const dcNode *_procedureNode)
{
    return CAST_PROCEDURE_AUX(_procedureNode)->methodHeader;
}

dcNode *dcProcedureClass_getBody(const dcNode *_procedureNode)
{
    return CAST_PROCEDURE_AUX(_procedureNode)->body;
}

void dcProcedureClass_setBody(dcNode *_procedureNode, dcNode *_body)
{
    CAST_PROCEDURE_AUX(_procedureNode)->body = _body;
}

// taffy methods //
dcNode *dcProcedureClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    dcString string;
    dcString_initialize(&string, 25);
    dcString_append(&string,
                    "(Procedure name: %s)",
                    dcProcedureClass_getMethodHeader(_receiver)->name);
    return dcNode_register(dcStringClass_createObject(string.string, false));
}

dcString *dcProcedureClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcProcedureClassAux *aux = CAST_PROCEDURE_AUX(_node);
    dcString *result = dcMarshaller_marshall(_stream, "H", aux->methodHeader);

    if (aux->body->type == NODE_TAFFY_C_METHOD_POINTER)
    {
        dcString_appendCharacter(result, 0xFF);
    }
    else
    {
        dcMarshaller_marshall(_stream,
                              "t",
                              aux->body);
    }

    return result;
}

bool dcProcedureClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcMethodHeader *methodHeader = NULL;
    dcNode *body = NULL;

    if (dcMarshaller_unmarshallNoNull(_stream,
                                      "Ht",
                                      &methodHeader,
                                      &body))
    {
        result = true;
        CAST_CLASS_AUX(_node) = createAux(body, methodHeader);
    }

    return result;
}

bool dcProcedureClass_isMe(const dcNode *_node)
{
    return dcClass_hasTemplate(_node, sTemplate, true);
}
