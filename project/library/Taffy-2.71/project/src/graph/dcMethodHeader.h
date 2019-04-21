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

#ifndef __DC_METHOD_H__
#define __DC_METHOD_H__

#include "dcDefines.h"

typedef enum
{
    METHOD_HEADER_TAFFY = 1,
    METHOD_HEADER_C     = 2
} dcMethodHeaderType;

typedef enum
{
    METHOD_HEADER_NODE_ARGUMENT     = 1,
    METHOD_HEADER_SUPPLIED_ARGUMENT = 2
} dcMethodHeaderArgumentTypes;

struct dcMethodHeaderCDefinition_t
{
    struct dcClassTemplate_t **argumentTypes;
    // there's a max of 255 arguments
    uint8_t argumentTypesSize;
    char **suppliedArguments;
    uint8_t suppliedArgumentsSize;
};

typedef struct dcMethodHeaderCDefinition_t dcMethodHeaderCDefinition;

struct dcMethodHeader_t
{
    union dcMethodHeaderData_t
    {
        struct dcList_t *arguments;
        dcMethodHeaderCDefinition *cDefinition;
    } data;

    dcMethodHeaderType type;

    //
    // the header name
    //
    // examples
    //
    //    for the code
    //
    //      'io put: "hello"'
    //
    //    name is "put:"
    //
    //    for the code
    //
    //      '1 upTo: 10 do: {}'
    //
    //    name is "upTo:do:"
    //
    char *name;
};

typedef struct dcMethodHeader_t dcMethodHeader;

// creating //
struct dcNode_t *dcMethodHeader_createNode(const char *_name,
                                           struct dcList_t *_arguments);

dcMethodHeader *dcMethodHeader_create(const char *_name,
                                      struct dcList_t *_arguments);

struct dcNode_t *dcMethodHeader_createCDefinitionNode
    (const char *_name,
     dcCFunctionArgument *_argumentTypes,
     const char **_suppliedArguments,
     uint8_t _suppliedArgumentsSize);

dcMethodHeader *dcMethodHeader_createCDefinition
    (const char *_name,
     dcCFunctionArgument *_argumentTypes,
     const char **_suppliedArguments,
     uint8_t _suppliedArgumentsSize);

// copying //
dcMethodHeader *dcMethodHeader_copy(const dcMethodHeader *_from,
                                    dcDepth _depth);

// getting //
struct dcClassTemplate_t **dcMethodHeader_getCDefinitionArgumentTypes
    (const dcMethodHeader *_methodHeader);

uint32_t dcMethodHeader_getCDefinitionArgumentTypesSize
    (const dcMethodHeader *_methodHeader);

const char **dcMethodHeader_getCDefinitionSuppliedArguments
    (const dcMethodHeader *_methodHeader);

uint32_t dcMethodHeader_getCDefinitionSuppliedArgumentsSize
    (const dcMethodHeader *_methodHeader);

struct dcList_t *dcMethodHeader_getArguments
    (const dcMethodHeader *_methodHeader);

size_t dcMethodHeader_getArgumentsSize(const dcMethodHeader *_methodHeader);

const char *dcMethodHeader_getName(const struct dcNode_t *_methodHeader);

// freeing //
void dcMethodHeader_free(dcMethodHeader **_methodHeader, dcDepth _depth);

// standard functions //
MARSHALL_FUNCTION(dcMethodHeader_marshallNode);
UNMARSHALL_FUNCTION(dcMethodHeader_unmarshallNode);
COPY_FUNCTION(dcMethodHeader_copyNode);
FREE_FUNCTION(dcMethodHeader_freeNode);

dcMethodHeader *dcMethodHeader_unmarshall(struct dcString_t *_stream);
struct dcString_t *dcMethodHeader_marshall(const dcMethodHeader *_header,
                                           struct dcString_t *_string);

#endif
