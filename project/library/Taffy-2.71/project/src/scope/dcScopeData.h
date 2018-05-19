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

#ifndef __DC_SCOPE_DATA_H__
#define __DC_SCOPE_DATA_H__

#include "dcDefines.h"

enum dcScopeDataFlags_e
{
    NO_SCOPE_DATA_FLAGS = 0,

    // is it a method? //
    SCOPE_DATA_METHOD = BITS(0),

    // is it an object? //
    SCOPE_DATA_OBJECT = BITS(1),

    // does the object have instance scope //
    SCOPE_DATA_INSTANCE = BITS(2),

    // does the object have meta scope //
    SCOPE_DATA_META = BITS(3),

    // is the object protected //
    SCOPE_DATA_PROTECTED = BITS(4),

    // is the object public //
    SCOPE_DATA_PUBLIC = BITS(5),

    // is the object constant //
    SCOPE_DATA_CONSTANT = BITS(6),

    // is the object local, fix me //
    SCOPE_DATA_LOCAL = BITS(7),

    // is the object global //
    SCOPE_DATA_GLOBAL = BITS(7),

    // is there a read accessor //
    SCOPE_DATA_READER = BITS(8),

    // is there a write accessor //
    SCOPE_DATA_WRITER = BITS(9),

    // does this method break through the parent's scope //
    SCOPE_DATA_BREAKTHROUGH = BITS(10),

    // is this method synchronized //
    SCOPE_DATA_SYNCHRONIZED = BITS(11),

    // is this method read synchronized //
    SCOPE_DATA_SYNCHRONIZED_READ = BITS(12),

    // is this method write synchronized //
    SCOPE_DATA_SYNCHRONIZED_WRITE = BITS(13),

    // does this method iterate its own container //
    SCOPE_DATA_CONTAINER_LOOP = BITS(14),

    // does this method modify its own container //
    SCOPE_DATA_MODIFIES_CONTAINER = BITS(15),

    // does the method modify no internal variables, or its arguments?
    SCOPE_DATA_CONST = BITS(16),

    // cast object? //
    SCOPE_DATA_NO_CAST = BITS(17),

    // is this a getter? //
    SCOPE_DATA_GETTER = BITS(18)
};

struct dcScopeData_t
{
    char *name;
    struct dcNode_t *object;
    dcScopeDataFlags flags;
};

typedef struct dcScopeData_t dcScopeData;

// creating //
dcScopeData *dcScopeData_create(const char *_name,
                                struct dcNode_t *_object,
                                dcScopeDataFlags _flags);

struct dcNode_t *dcScopeData_createNode(const char *_name,
                                        struct dcNode_t *_object,
                                        dcScopeDataFlags _flags);

struct dcNode_t *dcScopeData_createBlankNode(void);

// freeing //
void dcScopeData_free(dcScopeData **_scopeData, dcDepth _depth);

void dcScopeData_setName(dcScopeData *_scopeData, const char *_name);

// displaying //
char *dcScopeData_display(const dcScopeData *_scopeData);

// copying //
dcScopeData *dcScopeData_copy(const dcScopeData *_from, dcDepth _depth);

// getting //
const char *dcScopeData_getName(const struct dcNode_t *_scopeData);
struct dcNode_t *dcScopeData_getObject(const struct dcNode_t *_scopeData);
dcScopeDataFlags dcScopeData_getFlags(const struct dcNode_t *_scopeData);

// setting //
void dcScopeData_setObject(dcScopeData *_scopeData, struct dcNode_t *_object);
void dcScopeData_setFlags(struct dcNode_t *_scopeData, dcScopeDataFlags _flags);

// don't clobber the flags, just OR together //
void dcScopeData_updateFlags(dcScopeData *_scopeData, dcScopeDataFlags _flags);

// marking //
void dcScopeData_mark(dcScopeData *_scopeData);

// debugging hook //
void dcScopeData_printFlags(dcScopeDataFlags _flags);

// standard functions //
FREE_FUNCTION(dcScopeData_freeNode);
COPY_FUNCTION(dcScopeData_copyNode);
MARK_FUNCTION(dcScopeData_markNode);
PRINT_FUNCTION(dcScopeData_printNode);
MARSHALL_FUNCTION(dcScopeData_marshallNode);
REGISTER_FUNCTION(dcScopeData_registerNode);
UNMARSHALL_FUNCTION(dcScopeData_unmarshallNode);

#endif
