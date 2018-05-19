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

#ifndef __DC_SCOPE_H__
#define __DC_SCOPE_H__

#include "dcDefines.h"

struct dcScope_t
{
    struct dcHash_t *objects;
};

typedef struct dcScope_t dcScope;

// meta //
void dcScope_initializeMutex(void);

// creating //
dcScope *dcScope_create(void);

dcScope *dcScope_createWithValues(struct dcHash_t *_objects);

dcScope *dcScope_createFromTaffyCMethodWrappers
    (const struct dcTaffyCMethodWrapper_t *_wrappers);

struct dcNode_t *dcScope_createNode(void);
struct dcNode_t *dcScope_createNodeFromScope(dcScope *_scope);
struct dcNode_t *dcScope_createShell(dcScope *_scope);

// freeing //
void dcScope_free(dcScope **_scope, const dcDepth _depth);

// printing //
dcResult dcScope_print(const struct dcScope_t *_scope,
                       struct dcString_t *_stream);
const char *dcScope_display(const dcScope *_scope);

// taffy c methods //
void dcScope_addTaffyCMethodWrappers
    (dcScope *_scope, const struct dcTaffyCMethodWrapper_t *_wrappers);

struct dcNode_t *dcScope_set(dcScope *_scope,
                             struct dcNode_t *_object,
                             const char *_key,
                             dcScopeDataFlags _flags);

struct dcNode_t *dcScope_setMethod(dcScope *_scope,
                                   struct dcNode_t *_object,
                                   const char *_key,
                                   dcScopeDataFlags _flags);

struct dcNode_t *dcScope_setObject(dcScope *_scope,
                                   struct dcNode_t *_object,
                                   const char *_key,
                                   dcScopeDataFlags _flags);

// getting //
dcContainerSizeType dcScope_getSize(const dcScope *_scope);
struct dcNode_t *dcScope_getScopeData(const dcScope *_scope,
                                      const char *_key,
                                      dcScopeDataFlags _flags);

#define dcScope_getScopeDataForObject(_scope, _key)         \
    dcScope_getScopeData(_scope, _key, SCOPE_DATA_OBJECT)

#define dcScope_getScopeDataForMethod(_scope, _key)         \
    dcScope_getScopeData(_scope, _key, SCOPE_DATA_METHOD)

struct dcNode_t *dcScope_getObject(const dcScope *_scope, const char *_key);
struct dcNode_t *dcScope_getMethod(const dcScope *_scope, const char *_key);

// removing //
void dcScope_clear(dcScope *_scope, dcDepth _depth);
void dcScope_removeObject(dcScope *_scope,
                          const char *_key,
                          dcDepth _depth);

// iterating //
struct dcHashIterator_t *dcScope_createIterator(const dcScope *_scope);
struct dcNode_t *dcScope_getNext(struct dcHashIterator_t *_iterator,
                                 dcScopeDataFlags _type);

// querying //
bool dcScope_hasObject(const dcScope *_scope, const char *_key);
bool dcScope_isModified(const dcScope *_scope);

// copying //
dcScope *dcScope_copy(const dcScope *_from, dcDepth _depth);

// merging //
void dcScope_merge(dcScope *_me, const dcScope *_other);
void dcScope_mergeObjects(dcScope *_me, const dcScope *_other);

// marking //
void dcScope_mark(dcScope *_scope);

// registering //
void dcScope_register(dcScope *_scope);

// standard functions //
COPY_FUNCTION(dcScope_copyNode);
FREE_FUNCTION(dcScope_freeNode);
MARK_FUNCTION(dcScope_markNode);
MARSHALL_FUNCTION(dcScope_marshallNode);
PRINT_FUNCTION(dcScope_printNode);
UNMARSHALL_FUNCTION(dcScope_unmarshallNode);

struct dcString_t *dcScope_marshall(const dcScope *_scope,
                                    struct dcString_t *_stream);
dcScope *dcScope_unmarshall(struct dcString_t *_stream);

// debug functions
bool dcScope_equals(const dcScope *_left, const dcScope *_right);

#endif
