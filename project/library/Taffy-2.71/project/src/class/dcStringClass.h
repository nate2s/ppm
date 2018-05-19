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

#ifndef __DC_STRING_CLASS_H__
#define __DC_STRING_CLASS_H__

#include "dcDefines.h"

//////////////////////
// dcStringClassAux //
//////////////////////

struct dcStringClassAux_t
{
    union dcStringClassAux_types_t
    {
        struct dcList_t *objects;
        char *string;
    } types;

    bool initialized;
};

typedef struct dcStringClassAux_t dcStringClassAux;

///////////////////
// dcStringClass //
///////////////////

// creating //
struct dcNode_t *dcStringClass_createNode(const char *_string,
                                          bool _object,
                                          bool _copy);

struct dcNode_t *dcStringClass_createNodeFromList(struct dcList_t *_objects,
                                                  bool _object,
                                                  bool _initialized);

// creating //
struct dcNode_t *
dcStringClass_createInitializeObject(struct dcNode_t *_object,
                                     struct dcArray_t *_evaluatedString);

struct dcNode_t *dcStringClass_createObject(const char *_string, bool _copy);

struct dcNode_t *
dcStringClass_createObjectFromList(struct dcList_t *_objects,
                                   bool _initialized);

// initializing //
bool dcStringClass_initializeObject(struct dcNode_t *_stringNode,
                                    struct dcList_t *_objects);

// displaying //
char *dcStringClass_displayNodeWithoutQuotes(const struct dcNode_t *_string);

// getting/querying //
bool dcStringClass_isInitialized(const struct dcNode_t *_string);
bool dcStringClass_isMe(const struct dcNode_t *_string);
struct dcList_t *dcStringClass_getObjects(const struct dcNode_t *_node);
const char *dcStringClass_getString(const struct dcNode_t *_node);
struct dcList_t *dcStringClass_findSpaces(const char *_string);

const char *dcStringClass_asString_helper(struct dcNode_t *_object);
char *dcStringClass_asString_noQuotes_helper(struct dcNode_t *_object);
struct dcArray_t *dcStringClass_split_helper(const char *_splitString,
                                             const char *_splitterString);

// standard functions //
ALLOCATE_FUNCTION(dcStringClass_allocateNode);
COPY_FUNCTION(dcStringClass_copyNode);
GET_TEMPLATE_FUNCTION(dcStringClass_getTemplate);
PRINT_FUNCTION(dcStringClass_printNode);
FREE_FUNCTION(dcStringClass_freeNode);
INITIALIZE_FUNCTION(dcStringClass_initialize);
MARSHALL_FUNCTION(dcStringClass_marshallNode);
UNMARSHALL_FUNCTION(dcStringClass_unmarshallNode);

#define STRING_MARSHALL_LENGTH 2
#define STRING_CLASS_INITIALIZED 1
#define STRING_CLASS_UNINITIALIZED 0

// taffy c methods //
TAFFY_C_METHOD(dcStringClass_append);
TAFFY_C_METHOD(dcStringClass_appendBang);
TAFFY_C_METHOD(dcStringClass_clear);
TAFFY_C_METHOD(dcStringClass_compare);
TAFFY_C_METHOD(dcStringClass_downcase);
TAFFY_C_METHOD(dcStringClass_downcaseBang);
TAFFY_C_METHOD(dcStringClass_eachCharacter);
TAFFY_C_METHOD(dcStringClass_eachWord);
TAFFY_C_METHOD(dcStringClass_equals);
TAFFY_C_METHOD(dcStringClass_eval);
TAFFY_C_METHOD(dcStringClass_greaterThan);
TAFFY_C_METHOD(dcStringClass_greaterThanOrEqual);
TAFFY_C_METHOD(dcStringClass_hash);
TAFFY_C_METHOD(dcStringClass_indexOf);
TAFFY_C_METHOD(dcStringClass_insertStringAtIndex);
TAFFY_C_METHOD(dcStringClass_isNumeric);
TAFFY_C_METHOD(dcStringClass_lastIndexOf);
TAFFY_C_METHOD(dcStringClass_length);
TAFFY_C_METHOD(dcStringClass_lessThan);
TAFFY_C_METHOD(dcStringClass_lessThanOrEqual);
TAFFY_C_METHOD(dcStringClass_objectAtIndex);
TAFFY_C_METHOD(dcStringClass_reverse);
TAFFY_C_METHOD(dcStringClass_reverseBang);
TAFFY_C_METHOD(dcStringClass_setStringAtIndex);
TAFFY_C_METHOD(dcStringClass_split);
TAFFY_C_METHOD(dcStringClass_substring);
TAFFY_C_METHOD(dcStringClass_substringFromTo);
TAFFY_C_METHOD(dcStringClass_swapcase);
TAFFY_C_METHOD(dcStringClass_swapcaseBang);
TAFFY_C_METHOD(dcStringClass_upcase);
TAFFY_C_METHOD(dcStringClass_upcaseBang);
TAFFY_C_METHOD(dcStringClass_withWidth);

#define STRING_PACKAGE_NAME CORE_PACKAGE_NAME
#define STRING_CLASS_NAME "String"

#endif
