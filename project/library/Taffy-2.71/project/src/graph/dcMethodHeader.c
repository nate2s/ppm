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
#include <string.h>

#include "dcArray.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcGraphData.h"
#include "dcUnsignedInt32.h"
#include "dcIOClass.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNode.h"
#include "dcString.h"

dcNode *dcMethodHeader_createNode(const char *_name, dcList *_arguments)
{
    return (dcGraphData_createNodeWithGuts
            (NODE_METHOD_HEADER,
             dcMethodHeader_create(_name, _arguments)));
}

dcMethodHeader *dcMethodHeader_create(const char *_name, dcList *_arguments)
{
    dcMethodHeader *header =
        (dcMethodHeader *)dcMemory_allocate(sizeof(dcMethodHeader));
    header->type = METHOD_HEADER_TAFFY;
    header->data.arguments = _arguments;
    header->name = (_name == NULL
                    ? dcMemory_strdup("")
                    : dcMemory_strdup(_name));
    return header;
}

dcNode *dcMethodHeader_createCDefinitionNode
    (const char *_name,
     dcCFunctionArgument *_argumentTypes,
     const char **_suppliedArguments,
     uint8_t _suppliedArgumentsSize)
{
    return dcGraphData_createNodeWithGuts(NODE_METHOD_HEADER,
                                          dcMethodHeader_createCDefinition
                                          (_name,
                                           _argumentTypes,
                                           _suppliedArguments,
                                           _suppliedArgumentsSize));
}

static dcMethodHeader *createCDefinition(const char *_name,
                                         dcClassTemplate **_argumentTypes,
                                         uint8_t _argumentTypesSize,
                                         const char **_suppliedArguments,
                                         uint8_t _suppliedArgumentsSize)
{
    dcMethodHeaderCDefinition *cDefinition =
        (dcMethodHeaderCDefinition *)
        dcMemory_allocateAndInitialize(sizeof(dcMethodHeaderCDefinition));

    cDefinition->argumentTypes = _argumentTypes;
    cDefinition->argumentTypesSize = _argumentTypesSize;
    cDefinition->suppliedArgumentsSize = _suppliedArgumentsSize;

    cDefinition->suppliedArguments =
        (char **)dcMemory_allocate(sizeof(char *)
                                   * cDefinition->suppliedArgumentsSize);

    uint8_t i;

    for (i = 0; i < cDefinition->suppliedArgumentsSize; i++)
    {
        // copy //
        cDefinition->suppliedArguments[i] =
            dcMemory_strdup(_suppliedArguments[i]);
    }

    dcMethodHeader *header =
        (dcMethodHeader *)dcMemory_allocate(sizeof(dcMethodHeader));
    header->type = METHOD_HEADER_C;
    header->data.cDefinition = cDefinition;
    header->name = (_name == NULL
                    ? dcMemory_strdup("")
                    : dcMemory_strdup(_name));

    return header;
}

dcMethodHeader *dcMethodHeader_createCDefinition
    (const char *_name,
     dcCFunctionArgument *_argumentTypes,
     const char **_suppliedArguments,
     uint8_t _suppliedArgumentsSize)
{
    uint8_t argumentTypesSize;

    // set the argumentTypesSize variable
    for (argumentTypesSize = 0;
         _argumentTypes[argumentTypesSize] != NULL;
         argumentTypesSize++)
    {
    }

    dcClassTemplate **gotArgumentTypes =
        (dcClassTemplate **)dcMemory_allocate(sizeof(dcClassTemplate *)
                                              * argumentTypesSize);
    uint8_t i = 0;

    for (i = 0; i < argumentTypesSize; i++)
    {
        gotArgumentTypes[i] = dcClassManager_getClassTemplate
            (_argumentTypes[i], NULL, NULL, NULL);

        if (gotArgumentTypes[i] == NULL)
        {
            dcIOClass_printFormat("Error: dcMethodHeader_createCDefinition: "
                                  "can't find class template for '%s' for "
                                  "method: %s\n",
                                  _argumentTypes[i],
                                  _name);
            assert(gotArgumentTypes[i] != NULL);
        }
    }

    return createCDefinition(_name,
                             gotArgumentTypes,
                             argumentTypesSize,
                             _suppliedArguments,
                             _suppliedArgumentsSize);
}

void dcMethodHeader_free(dcMethodHeader **_methodHeader, dcDepth _depth)
{
    if (*_methodHeader != NULL)
    {
        dcMethodHeader *methodHeader = *_methodHeader;

        if (methodHeader->type == METHOD_HEADER_TAFFY)
        {
            if (_depth == DC_DEEP)
            {
                dcList_free(&methodHeader->data.arguments, DC_DEEP);
            }
        }
        else if (methodHeader->type == METHOD_HEADER_C)
        {
            dcMethodHeaderCDefinition *cDefinition =
                methodHeader->data.cDefinition;
            size_t i = 0;

            for (i = 0;
                 i < cDefinition->suppliedArgumentsSize;
                 i++)
            {
                dcMemory_free(cDefinition->suppliedArguments[i]);
            }

            dcMemory_free(cDefinition->suppliedArguments);
            dcMemory_free(cDefinition->argumentTypes);
            dcMemory_free(cDefinition);
        }

        dcMemory_free(methodHeader->name);
        dcMemory_free(*_methodHeader);
    }
}

// create dcMethodHeader_freeNode
dcTaffy_createFreeNodeFunctionMacro(dcMethodHeader, CAST_METHOD_HEADER);

// create dcMethodHeader_copyNode
dcTaffy_createCopyNodeFunctionMacro(dcMethodHeader, CAST_METHOD_HEADER);

dcMethodHeader *dcMethodHeader_copy(const dcMethodHeader *_fromHeader,
                                    dcDepth _depth)
{
    dcMethodHeader *toHeader = NULL;

    if (_fromHeader != NULL)
    {
        if (_fromHeader->type == METHOD_HEADER_TAFFY)
        {
            toHeader = dcMethodHeader_create
                (_fromHeader->name,
                 dcList_copy(_fromHeader->data.arguments, DC_DEEP));
        }
        else if (_fromHeader->type == METHOD_HEADER_C)
        {
            dcClassTemplate **argumentTypes =
                (dcClassTemplate **)
                dcMemory_duplicate
                (_fromHeader->data.cDefinition->argumentTypes,
                 sizeof(dcClassTemplate*)
                 * _fromHeader->data.cDefinition->argumentTypesSize);

            toHeader = createCDefinition
                (_fromHeader->name,
                 argumentTypes,
                 _fromHeader->data.cDefinition->argumentTypesSize,
                 (const char **)
                 _fromHeader->data.cDefinition->suppliedArguments,
                 _fromHeader->data.cDefinition->suppliedArgumentsSize);
        }
    }

    return toHeader;
}

dcClassTemplate **dcMethodHeader_getCDefinitionArgumentTypes
    (const dcMethodHeader *_methodHeader)
{
    return _methodHeader->data.cDefinition->argumentTypes;
}

const char **dcMethodHeader_getCDefinitionSuppliedArguments
    (const dcMethodHeader *_methodHeader)
{
    return (const char **)_methodHeader->data.cDefinition->suppliedArguments;
}

uint32_t dcMethodHeader_getCDefinitionSuppliedArgumentsSize
    (const dcMethodHeader *_methodHeader)
{
    return _methodHeader->data.cDefinition->suppliedArgumentsSize;
}

uint32_t dcMethodHeader_getCDefinitionArgumentTypesSize
    (const dcMethodHeader *_methodHeader)
{
    return ((_methodHeader->data.cDefinition->argumentTypes
             == NULL)
            ? 0
            : _methodHeader->data.cDefinition->argumentTypesSize);
}

dcList *dcMethodHeader_getArguments(const dcMethodHeader *_methodHeader)
{
    return _methodHeader->data.arguments;
}

size_t dcMethodHeader_getArgumentsSize(const dcMethodHeader *_methodHeader)
{
    return _methodHeader->data.arguments->size;
}

const char *dcMethodHeader_getName(const dcNode *_methodHeader)
{
    return CAST_METHOD_HEADER(_methodHeader)->name;
}

bool dcMethodHeader_unmarshallNode(dcNode *_node, dcString *_stream)
{
    CAST_METHOD_HEADER(_node) = dcMethodHeader_unmarshall(_stream);
    return (CAST_METHOD_HEADER(_node) != NULL);
}

dcMethodHeader *dcMethodHeader_unmarshall(dcString *_stream)
{
    dcMethodHeader *result = NULL;
    char *methodName = NULL;
    uint8_t headerType;

    if (dcMarshaller_unmarshallNoNull(_stream,
                                      "su",
                                      &methodName,
                                      &headerType))
    {
        if (headerType == METHOD_HEADER_TAFFY)
        {
            dcList *arguments = NULL;

            if (dcMarshaller_unmarshall(_stream, "l", &arguments))
            {
                result = dcMethodHeader_create(methodName, arguments);
            }
        }
        // else do nothing //
    }

    dcMemory_free(methodName);
    return result;
}

// create dcMethodHeader_marshallNode
dcTaffy_createMarshallNodeFunctionMacro(dcMethodHeader, CAST_METHOD_HEADER);

dcString *dcMethodHeader_marshall(const dcMethodHeader *_header,
                                  dcString *_stream)
{
    dcString *result = dcMarshaller_marshall(_stream,
                                             "su",
                                             _header->name,
                                             _header->type);
    if (_header->type == METHOD_HEADER_TAFFY)
    {
        dcMarshaller_marshall(result,
                              "l",
                              dcMethodHeader_getArguments(_header));
    }
    // else do nothing

    return result;
}
