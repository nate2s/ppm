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

#ifndef __DC_FUNCTION_CLASS_H__
#define __DC_FUNCTION_CLASS_H__

#include "dcDefines.h"

struct dcFunctionMetaClassAux_t
{
    struct dcMutex_t *mutex;
    uint16_t defaultMemorySize;
    uint32_t nextFunctionId;
};

typedef struct dcFunctionMetaClassAux_t dcFunctionMetaClassAux;

////////////////////////
// dcFunctionClassAux //
////////////////////////

struct dcFunctionClassAux_t
{
    struct dcHash_t *memory;
    struct dcList_t *memoryList;
    uint16_t memorySize;
    struct dcList_t *specificValues;
    uint32_t functionId;
};

typedef struct dcFunctionClassAux_t dcFunctionClassAux;

/////////////////////
// dcFunctionClass //
/////////////////////

uint32_t dcFunctionMetaClass_getAndIncrementFunctionId(void);

// creating //
struct dcNode_t *dcFunctionClass_createNode(struct dcNode_t *_body,
                                            struct dcMethodHeader_t *_header,
                                            bool _object);

struct dcNode_t *dcFunctionClass_createObjectWithArguments
    (struct dcList_t *_identifiers,
     struct dcNode_t *_body);

struct dcNode_t *dcFunctionClass_createObject(struct dcNode_t *_body,
                                              struct dcMethodHeader_t *_header);

// specific values //
struct dcNode_t *dcFunctionClass_addSpecific
    (struct dcNode_t *_functionClassNode,
     const struct dcList_t *_arguments,
     const struct dcNode_t *_arithmetic);

// testing //
bool dcFunctionClass_argumentsMatch(const struct dcArray_t *_arguments,
                                      const struct dcList_t *_specificArguments,
                                      const struct dcList_t *_headerArguments);

struct dcNode_t *dcFunctionClass_numberOperation(struct dcNode_t *_function,
                                                 struct dcNode_t *_number,
                                                 dcTaffyOperator _operation,
                                                 bool _left);

struct dcNode_t *dcFunctionClass_getComposedBody(struct dcNode_t *_receiver,
                                                 struct dcArray_t *_blocks,
                                                 bool _fromTaffy);

struct dcNode_t *dcFunctionClass_reallyCompose(struct dcNode_t *_function,
                                               struct dcArray_t *_blocks,
                                               bool _fromTaffy);

struct dcMethodHeader_t *dcFunctionClass_getMethodHeader
    (struct dcNode_t *_function);

#define FUNCTION_CLASS_MARSHALL_SIZE 5

TAFFY_C_METHOD(dcFunctionMetaClass_createWithBlock);
TAFFY_C_METHOD(dcFunctionMetaClass_setDefaultMemorySize);
TAFFY_C_METHOD(dcFunctionMetaClass_getDefaultMemorySize);

struct dcNode_t *dcFunctionClass_getBody(struct dcNode_t *_receiver);
struct dcNode_t *dcFunctionClass_getGraphDataBody(struct dcNode_t *_receiver);

dcResult dcFunctionClass_compileHelper(struct dcNode_t *_function,
                                       bool *_changed);

uint32_t dcFunctionClass_getFunctionId(struct dcNode_t *_function);
uint32_t dcFunctionClass_argumentCountHelper(struct dcNode_t *_receiver);

GET_TEMPLATE_FUNCTION(dcFunctionClass_getTemplate);

// external standard functions //
IS_ME_FUNCTION(dcFunctionClass_isMe);

// taffy methods
TAFFY_C_METHOD(dcFunctionClass_add);
TAFFY_C_METHOD(dcFunctionClass_addKeyValue);
TAFFY_C_METHOD(dcFunctionClass_argumentCount);
TAFFY_C_METHOD(dcFunctionClass_asString);
TAFFY_C_METHOD(dcFunctionClass_bitAnd);
TAFFY_C_METHOD(dcFunctionClass_bitNot);
TAFFY_C_METHOD(dcFunctionClass_bitOr);
TAFFY_C_METHOD(dcFunctionClass_call);
TAFFY_C_METHOD(dcFunctionClass_cancel);
TAFFY_C_METHOD(dcFunctionClass_cancelBang);
TAFFY_C_METHOD(dcFunctionClass_clearMemory);
TAFFY_C_METHOD(dcFunctionClass_compile);
TAFFY_C_METHOD(dcFunctionClass_compileBang);
TAFFY_C_METHOD(dcFunctionClass_compose);
TAFFY_C_METHOD(dcFunctionClass_composes);
TAFFY_C_METHOD(dcFunctionClass_convertSubtractToAdd);
TAFFY_C_METHOD(dcFunctionClass_convertSubtractToAddBang);
TAFFY_C_METHOD(dcFunctionClass_deltaEquals);
TAFFY_C_METHOD(dcFunctionClass_degree);
TAFFY_C_METHOD(dcFunctionClass_derive);
TAFFY_C_METHOD(dcFunctionClass_divide);
TAFFY_C_METHOD(dcFunctionClass_distribute);
TAFFY_C_METHOD(dcFunctionClass_distributeBang);
TAFFY_C_METHOD(dcFunctionClass_equals);
TAFFY_C_METHOD(dcFunctionClass_expand);
TAFFY_C_METHOD(dcFunctionClass_expandBang);
TAFFY_C_METHOD(dcFunctionClass_factorial);
TAFFY_C_METHOD(dcFunctionClass_factor);
TAFFY_C_METHOD(dcFunctionClass_factorBang);
TAFFY_C_METHOD(dcFunctionClass_getMemorySize);
TAFFY_C_METHOD(dcFunctionClass_id);
TAFFY_C_METHOD(dcFunctionClass_integrate);
TAFFY_C_METHOD(dcFunctionClass_leftShift);
TAFFY_C_METHOD(dcFunctionClass_modulus);
TAFFY_C_METHOD(dcFunctionClass_multiply);
TAFFY_C_METHOD(dcFunctionClass_parentheses);
TAFFY_C_METHOD(dcFunctionClass_prettyPrint);
TAFFY_C_METHOD(dcFunctionClass_raise);
TAFFY_C_METHOD(dcFunctionClass_rightShift);
TAFFY_C_METHOD(dcFunctionClass_setMemorySize);
TAFFY_C_METHOD(dcFunctionClass_simplify);
TAFFY_C_METHOD(dcFunctionClass_simplifyBang);
TAFFY_C_METHOD(dcFunctionClass_subtract);

#define FUNCTION_PACKAGE_NAME MATHS_PACKAGE_NAME
#define FUNCTION_CLASS_NAME "Function"

#endif
