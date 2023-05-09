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

#include <string.h>

#include "dcScopeData.h"
#include "dcNode.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNodeEvaluator.h"
#include "dcString.h"

// creating //
dcScopeData *dcScopeData_create(const char *_name,
                                dcNode *_object,
                                dcScopeDataFlags _flags)
{
    dcScopeData *scopeData =
        (dcScopeData *)dcMemory_allocate(sizeof(dcScopeData));
    scopeData->name = dcMemory_strdup(_name);
    scopeData->object = NULL;
    scopeData->flags = _flags;
    dcScopeData_setObject(scopeData, _object);
    return scopeData;
}

dcNode *dcScopeData_createNode(const char *_name,
                               dcNode *_node,
                               dcScopeDataFlags _flags)
{
    return dcNode_createWithGuts(NODE_SCOPE_DATA,
                                 dcScopeData_create(_name, _node, _flags));
}

dcNode *dcScopeData_createBlankNode(void)
{
    return dcScopeData_createNode("", NULL, 0);
}

// freeing //
void dcScopeData_free(dcScopeData **_scopeData, dcDepth _depth)
{
    dcScopeData *scopeData = *_scopeData;
    dcNode_tryFree(&scopeData->object, _depth);
    dcMemory_free(scopeData->name);
    dcMemory_free(*_scopeData);
}

// create dcScopeData_freeNode
dcTaffy_createFreeNodeFunctionMacro(dcScopeData, CAST_SCOPE_DATA);

// create dcScopeData_copyNode
dcTaffy_createCopyNodeFunctionMacro(dcScopeData, CAST_SCOPE_DATA);

// create dcScopeData_markNode
dcTaffy_createMarkNodeFunctionMacro(dcScopeData, CAST_SCOPE_DATA);

dcResult dcScopeData_printNode(const dcNode *_node, dcString *_stream)
{
    dcScopeData *scopeData = CAST_SCOPE_DATA(_node);
    const char *objectDisplay = dcNode_display(scopeData->object);
    dcResult result = TAFFY_SUCCESS;

    if (objectDisplay != NULL)
    {
        dcString_append(_stream,
                        "#ScopeData[%d, %s, %s]",
                        scopeData->flags,
                        scopeData->name,
                        dcNode_display(scopeData->object));
    }
    else
    {
        result = TAFFY_EXCEPTION;
    }

    return result;
}

void dcScopeData_setName(dcScopeData *_scopeData, const char *_name)
{
    dcMemory_free(_scopeData->name);
    _scopeData->name = dcMemory_strdup(_name);
}

dcScopeData *dcScopeData_copy(const dcScopeData *_from, dcDepth _depth)
{
    return dcScopeData_create(_from->name,
                              dcNode_tryCopy(_from->object, _depth),
                              _from->flags);
}

// getting //
const char *dcScopeData_getName(const dcNode *_node)
{
    return CAST_SCOPE_DATA(_node)->name;
}

dcNode *dcScopeData_getObject(const dcNode *_node)
{
    return CAST_SCOPE_DATA(_node)->object;
}

dcScopeDataFlags dcScopeData_getFlags(const dcNode *_node)
{
    return CAST_SCOPE_DATA(_node)->flags;
}

void dcScopeData_setObject(dcScopeData *_scopeData, dcNode *_object)
{
    if (_scopeData->object != NULL)
    {
        dcNode_register(_scopeData->object);
    }

    _scopeData->object = _object;
}

void dcScopeData_setFlags(dcNode *_scopeData, dcScopeDataFlags _flags)
{
    CAST_SCOPE_DATA(_scopeData)->flags = _flags;
}

void dcScopeData_updateFlags(dcScopeData *_scopeData, dcScopeDataFlags _flags)
{
    _scopeData->flags |= _flags;
}

void dcScopeData_mark(dcScopeData *_scopeData)
{
    dcNode_mark(_scopeData->object);
}

void dcScopeData_registerNode(dcNode *_scopeNode)
{
    dcNode_register(CAST_SCOPE_DATA(_scopeNode)->object);
}

dcString *dcScopeData_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcScopeData *scopeData = CAST_SCOPE_DATA(_node);
    return dcMarshaller_marshall(_stream,
                                 "usni",
                                 NODE_SCOPE_DATA,
                                 scopeData->name,
                                 scopeData->object,
                                 scopeData->flags);
}

bool dcScopeData_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    char *name = NULL;
    dcNode *object = NULL;
    uint32_t flags = NO_FLAGS;
    uint8_t type;

    if (dcMarshaller_unmarshallNoNull(_stream, "u", &type)
        && type == NODE_SCOPE_DATA
        && dcMarshaller_unmarshallNoNull(_stream,
                                         "sni",
                                         &name,
                                         &object,
                                         &flags))
    {
        result = true;
        CAST_SCOPE_DATA(_node) = dcScopeData_create(name, object, flags);
        dcMemory_free(name);
    }

    return result;
}

bool dcScopeData_isPublic(const dcScopeData *_scopeData)
{
    return FLAG_CHECK(_scopeData->flags & SCOPE_DATA_PUBLIC);
}

bool dcScopeData_isProtected(const dcScopeData *_scopeData)
{
    return FLAG_CHECK(_scopeData->flags & SCOPE_DATA_PROTECTED);
}

bool dcScopeData_hasReadAccessor(const dcScopeData *_scopeData)
{
    return FLAG_CHECK(_scopeData->flags & SCOPE_DATA_READER);
}

bool dcScopeData_hasWriteAccessor(const dcScopeData *_scopeData)
{
    return FLAG_CHECK(_scopeData->flags & SCOPE_DATA_WRITER);
}

struct ScopeDataMap_t
{
    dcScopeDataFlags flags;
    const char *name;
};

typedef struct ScopeDataMap_t ScopeDataMap;

static const ScopeDataMap scopeDataMap[] =
{
    {SCOPE_DATA_METHOD,             "SCOPE_DATA_METHOD"},
    {SCOPE_DATA_OBJECT,             "SCOPE_DATA_OBJECT"},
    {SCOPE_DATA_INSTANCE,           "SCOPE_DATA_INSTANCE"},
    {SCOPE_DATA_META,               "SCOPE_DATA_META"},
    {SCOPE_DATA_PROTECTED,          "SCOPE_DATA_PROTECTED"},
    {SCOPE_DATA_PUBLIC,             "SCOPE_DATA_PUBLIC"},
    {SCOPE_DATA_CONSTANT,           "SCOPE_DATA_CONSTANT"},
    {SCOPE_DATA_GLOBAL,             "SCOPE_DATA_GLOBAL"},
    {SCOPE_DATA_READER,             "SCOPE_DATA_READER"},
    {SCOPE_DATA_WRITER,             "SCOPE_DATA_WRITER"},
    {SCOPE_DATA_BREAKTHROUGH,       "SCOPE_DATA_BREAKTHROUGH"},
    {SCOPE_DATA_SYNCHRONIZED,       "SCOPE_DATA_SYNCHRONIZED"},
    {SCOPE_DATA_CONTAINER_LOOP,     "SCOPE_DATA_CONTAINER_LOOP"},
    {SCOPE_DATA_MODIFIES_CONTAINER, "SCOPE_DATA_MODIFIES_CONTAINER"},
    {SCOPE_DATA_NO_CAST,            "SCOPE_DATA_NO_CAST"},
    {0}
};

// debugging hook //
#ifdef TAFFY_DEBUG
#include <stdio.h>

void dcScopeData_printFlags(dcScopeDataFlags _flags)
{
    size_t i;
    printf("flags: %d: ", _flags);

    for (i = 0; i < dcTaffy_countOf(scopeDataMap); i++)
    {
        if ((_flags & scopeDataMap[i].flags) != 0)
        {
            printf("%s\n", scopeDataMap[i].name);
        }
    }

    printf("\n");
}
#endif
