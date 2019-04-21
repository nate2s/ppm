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

#ifndef __DC_ARRAY_CLASS_H__
#define __DC_ARRAY_CLASS_H__

#include "dcDefines.h"

/////////////////////
// dcArrayClassAux //
/////////////////////

struct dcArrayClassAux_t
{
    struct dcContainerData_t *containerData;
    struct dcArray_t *objects;
    bool initialized;
};

typedef struct dcArrayClassAux_t dcArrayClassAux;

//////////////////
// dcArrayClass //
//////////////////

// creating //
struct dcNode_t *dcArrayClass_createNode(struct dcArray_t *_objects,
                                         bool _initialized,
                                         bool _object);

struct dcNode_t *dcArrayClass_createObject(struct dcArray_t *_objects,
                                           bool _initialized);

struct dcNode_t *dcArrayClass_createEmptyObject(void);

// querying/getting //
bool dcArrayClass_isInitialized(const struct dcNode_t *_arrayNode);
void dcArrayClass_setInitialized(struct dcNode_t *_arrayNode);
struct dcArray_t *dcArrayClass_getObjects(const struct dcNode_t *_node);

// setting //
void dcArrayClass_setObjects(struct dcNode_t *_arrayNode,
                             struct dcArray_t *_objects,
                             dcDepth _depth);

void dcArrayClass_clearObjects(struct dcNode_t *_arrayNode);

// size verifying //
bool dcArrayClass_verifySizeWithThrow(struct dcNode_t *_arrayObject,
                                      dcContainerSizeType _size);

void dcInvalidArraySizeExceptionClass_throwObject
    (dcContainerSizeType _expected, int32_t _given);

// standard functions //
ALLOCATE_FUNCTION(dcArrayClass_allocateNode);
COPY_FUNCTION(dcArrayClass_copyNode);
DEALLOCATE_FUNCTION(dcArrayClass_deallocateNode);
FREE_FUNCTION(dcArrayClass_freeNode);
GET_TEMPLATE_FUNCTION(dcArrayClass_getTemplate);
INITIALIZE_FUNCTION(dcArrayClass_initialize);
MARK_FUNCTION(dcArrayClass_markNode);
MARSHALL_FUNCTION(dcArrayClass_marshallNode);
REGISTER_FUNCTION(dcArrayClass_registerNode);
UNMARSHALL_FUNCTION(dcArrayClass_unmarshallNode);

// taffy c methods //
TAFFY_C_METHOD(dcArrayMetaClass_createWithSize);

TAFFY_C_METHOD(dcArrayClass_asArray);
TAFFY_C_METHOD(dcArrayClass_asString);
TAFFY_C_METHOD(dcArrayClass_clear);
TAFFY_C_METHOD(dcArrayClass_collect);
TAFFY_C_METHOD(dcArrayClass_collectBang);
TAFFY_C_METHOD(dcArrayClass_concat);
TAFFY_C_METHOD(dcArrayClass_each);
TAFFY_C_METHOD(dcArrayClass_eachIndex);
TAFFY_C_METHOD(dcArrayClass_find);
TAFFY_C_METHOD(dcArrayClass_first);
TAFFY_C_METHOD(dcArrayClass_insertObjectAtIndex);
TAFFY_C_METHOD(dcArrayClass_isEmpty);
TAFFY_C_METHOD(dcArrayClass_last);
TAFFY_C_METHOD(dcArrayClass_objectAtIndex);
TAFFY_C_METHOD(dcArrayClass_objectAtIndexes);
TAFFY_C_METHOD(dcArrayClass_operatorEquals);
TAFFY_C_METHOD(dcArrayClass_reject);
TAFFY_C_METHOD(dcArrayClass_removeAll);
TAFFY_C_METHOD(dcArrayClass_reverse);
TAFFY_C_METHOD(dcArrayClass_reverseBang);
TAFFY_C_METHOD(dcArrayClass_select);
TAFFY_C_METHOD(dcArrayClass_selectBang);
TAFFY_C_METHOD(dcArrayClass_setObjectAtIndex);
TAFFY_C_METHOD(dcArrayClass_setObjectAtIndexes);
TAFFY_C_METHOD(dcArrayClass_size);
TAFFY_C_METHOD(dcArrayClass_subArrayFromTo);

void dcArrayClass_fillWithNil(struct dcArray_t *_array);

bool dcArrayClass_arrayContentsAsString(struct dcArray_t *_objects,
                                        struct dcString_t *_string);

struct dcNode_t *dcArrayClass_arrayOperation_helper
    (struct dcArray_t *_array1,
     struct dcArray_t *_array2,
     struct dcNode_t *_container1,
     struct dcNode_t *_container2,
     const char *_operation,
     struct dcNode_t *_extra);

struct dcString_t *dcArrayClass_asByteString_helper(struct dcArray_t *_array);

void dcArrayClass_printNode(const struct dcNode_t *_node,
                            struct dcString_t *_string);

#define ARRAY_PACKAGE_NAME CONTAINER_PACKAGE_NAME
#define ARRAY_CLASS_NAME "Array"

bool dcArrayClass_isMe(const struct dcNode_t *_node);

dcResult dcArrayClass_compileHelper(struct dcNode_t *_node,
                                    struct dcList_t *_symbols,
                                    bool *_changed);
#endif
