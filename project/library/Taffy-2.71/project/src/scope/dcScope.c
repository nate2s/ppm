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

#include "dcScope.h"
#include "dcClass.h"
#include "dcExceptions.h"
#include "dcGraphData.h"
#include "dcGarbageCollector.h"
#include "dcHash.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTaffyCMethodPointer.h"
#include "dcTaffyCMethodWrapper.h"

dcScope *dcScope_create(void)
{
    return dcScope_createWithValues(dcHash_create());
}

dcScope *dcScope_createWithValues(dcHash *_objects)
{
    dcScope *scope = (dcScope *)dcMemory_allocate(sizeof(dcScope));
    scope->objects = _objects;
    return scope;
}

dcNode *dcScope_createNode(void)
{
    dcNode *nodeScope = dcNode_create(NODE_SCOPE);
    CAST_SCOPE(nodeScope) = dcScope_create();
    return nodeScope;
}

dcNode *dcScope_createShell(dcScope *_scope)
{
    dcNode *nodeScope = dcNode_create(NODE_SCOPE);
    CAST_SCOPE(nodeScope) = _scope;
    return nodeScope;
}

dcScope *dcScope_createFromTaffyCMethodWrappers
    (const dcTaffyCMethodWrapper *_wraps)
{
    dcScope *retval = dcScope_create();
    dcScope_addTaffyCMethodWrappers(retval, _wraps);
    return retval;
}

void dcScope_free(dcScope **_scope, dcDepth _depth)
{
    if (*_scope != NULL)
    {
        dcScope *scope = *_scope;
        dcHash_free(&scope->objects, _depth);
        dcMemory_free(*_scope);
    }
}

dcScope *dcScope_copy(const dcScope *_scope, dcDepth _depth)
{
    return dcScope_createWithValues
        (dcHash_copy(_scope->objects, _depth));
}

static void doMerge(dcScope *_me,
                    const dcScope *_other,
                    dcScopeDataFlags _flags)
{
    dcHashIterator *it = dcHash_createIterator(_other->objects);
    dcNode *element;

    while ((element = dcHashIterator_getNextValue(it))
           != NULL)
    {
        assert(element->type == NODE_SCOPE_DATA);
        dcScopeData *data = CAST_SCOPE_DATA(element);

        if ((data->flags & _flags) != 0)
        {
            dcScope_set(_me,
                        dcNode_copy(data->object, DC_DEEP),
                        data->name,
                        data->flags);
        }
    }

    dcHashIterator_free(&it);
}

void dcScope_merge(dcScope *_me, const dcScope *_other)
{
    doMerge(_me, _other, SCOPE_DATA_METHOD | SCOPE_DATA_OBJECT);
}

void dcScope_mergeObjects(dcScope *_me, const dcScope *_other)
{
    doMerge(_me, _other, SCOPE_DATA_OBJECT);
}

// create dcScope_freeNode //
dcTaffy_createFreeNodeFunctionMacro(dcScope, CAST_SCOPE);

// create dcScope_copyNode //
dcTaffy_createCopyNodeFunctionMacro(dcScope, CAST_SCOPE);

// create dcScope_printNode
dcTaffy_createPrintNodeFunctionMacro(dcScope, CAST_SCOPE);

// create dcScope_display
dcTaffy_createDisplayFunctionMacro(dcScope);

// create dcScope_marshallNode //
dcTaffy_createMarshallNodeFunctionMacro(dcScope, CAST_SCOPE);

// create dcScope_unmarshallNode //
dcTaffy_createUnmarshallNodeFunctionMacro(dcScope, CAST_SCOPE);

dcResult dcScope_print(const dcScope *_scope, dcString *_stream)
{
    if (_scope != NULL)
    {
        dcString_append(_stream,
                        "Scope:\nObjects (size %u):\n",
                        _scope->objects->size);
        dcHashIterator *i = dcHash_createIterator(_scope->objects);
        dcNode *that = NULL;

        while ((that = dcHashIterator_getNextValue(i)))
        {
            dcString_append(_stream,
                            "%s (%s), ",
                            dcScopeData_getName(that),
                            ((dcScopeData_getFlags(that)
                              & SCOPE_DATA_METHOD) != 0
                             ? "method"
                             : "object"));
        }

        dcHashIterator_free(&i);
        dcString_appendCharacter(_stream, '\n');
    }
    else
    {
        dcString_appendString(_stream, "NULL");
    }

    return TAFFY_SUCCESS;
}

void dcScope_removeObject(dcScope *_scope, const char *_key, dcDepth _depth)
{
    dcHash_removeValueWithStringKey(_scope->objects, _key, NULL, _depth);
}

dcNode *dcScope_getScopeData(const dcScope *_scope,
                             const char *_key,
                             dcScopeDataFlags _flags)
{
    dcNode *foundObject;
    dcHash_getValueWithStringKey(_scope->objects,
                                 _key,
                                 &foundObject);
    if (foundObject != NULL)
    {
        // got it maybe
        dcScopeDataFlags flags = dcScopeData_getFlags(foundObject);

        if (_flags == NO_FLAGS
            || (flags & _flags) != 0)
        {
            // really got it
        }
        else
        {
            foundObject = NULL;
        }
    }

    return foundObject;
}

static dcNode *extractValue(dcNode *_scopeDataNode)
{
    return (_scopeDataNode == NULL
            ? NULL
            : dcScopeData_getObject(_scopeDataNode));
}

dcNode *dcScope_getObject(const dcScope *_scope, const char *_key)
{
    return extractValue(dcScope_getScopeDataForObject(_scope, _key));
}

dcNode *dcScope_getMethod(const dcScope *_scope, const char *_key)
{
    return extractValue(dcScope_getScopeDataForMethod(_scope, _key));
}

dcNode *dcScope_set(dcScope *_scope,
                    dcNode *_object,
                    const char *_key,
                    dcScopeDataFlags _flags)
{
    // scope can't take containers, they will be improperly freed during clear()
    assert(! dcNode_isContainer(_object));

    dcNode *scopeData = dcScope_getScopeData(_scope, _key, NO_FLAGS);

    if (scopeData != NULL)
    {
        // update the scope data //
        dcScopeData_setObject(CAST_SCOPE_DATA(scopeData), _object);
        dcScopeData_setFlags(scopeData, _flags);
    }
    else
    {
        // create new scope data //
        scopeData = dcScopeData_createNode(_key, _object, _flags);

        // stuff it //
        dcHash_setValueWithStringKey(_scope->objects, _key, scopeData);
    }

    return scopeData;
}

dcNode *dcScope_setMethod(dcScope *_scope,
                          dcNode *_method,
                          const char *_key,
                          dcScopeDataFlags _flags)
{
    return dcScope_set(_scope, _method, _key, _flags | SCOPE_DATA_METHOD);
}

dcNode *dcScope_setObject(dcScope *_scope,
                          dcNode *_object,
                          const char *_key,
                          dcScopeDataFlags _flags)
{
    return dcScope_set(_scope, _object, _key, _flags | SCOPE_DATA_OBJECT);
}

dcContainerSizeType dcScope_getSize(const dcScope *_scope)
{
    return _scope->objects->size;
}

void dcScope_addTaffyCMethodWrappers(dcScope *_scope,
                                     const dcTaffyCMethodWrapper *_wrappers)
{
    if (_wrappers != NULL)
    {
        size_t i;

        for (i = 0; _wrappers[i].name != NULL; i++)
        {
            const dcTaffyCMethodWrapper *wrapper = &_wrappers[i];

            dcNode *procedure = dcNode_setTemplate
                (dcProcedureClass_createObject
                 (dcNode_setTemplate
                  (dcTaffyCMethodPointer_createNode(wrapper->method),
                   true),
                  dcMethodHeader_createCDefinition(wrapper->name,
                                                   wrapper->argumentTypes,
                                                   NULL,
                                                   0)),
                 true); // is a template, yup yup

            // derr?
            dcGraphData_setPosition
                (CAST_GRAPH_DATA(procedure),
                 1,
                 dcStringManager_getStringId("CMethodWrapper.ty"));

            dcScope_setMethod(_scope, procedure, wrapper->name, wrapper->flags);
        }
    }
}

void dcScope_clear(dcScope *_scope, dcDepth _depth)
{
    if (_scope != NULL)
    {
        dcHash_clear(_scope->objects, _depth);
    }
}

bool dcScope_hasObject(const dcScope *_scope, const char *_key)
{
    return (dcScope_getScopeDataForObject(_scope, _key) != NULL);
}

bool dcScope_isModified(const dcScope *_scope)
{
    return (_scope->objects->size > 0);
}

void dcScope_mark(dcScope *_scope)
{
    dcHash_mark(_scope->objects);
}

void dcScope_markNode(dcNode *_scopeNode)
{
    return dcScope_mark(CAST_SCOPE(_scopeNode));
}

void dcScope_register(dcScope *_scope)
{
    dcHash_register(_scope->objects);
}

dcString *dcScope_marshall(const dcScope *_scope, dcString *_stream)
{
    return dcMarshaller_marshall(_stream, "uh", NODE_SCOPE, _scope->objects);
}

dcScope *dcScope_unmarshall(dcString *_stream)
{
    dcScope *scope = NULL;
    dcHash *objects;
    uint8_t type;

    if (dcMarshaller_unmarshallNoNull(_stream, "u", &type)
        && type == NODE_SCOPE
        && dcMarshaller_unmarshallNoNull(_stream, "h", &objects))
    {
        scope = dcScope_createWithValues(objects);
    }

    return scope;
}

dcHashIterator *dcScope_createIterator(const dcScope *_scope)
{
    return dcHash_createIterator(_scope->objects);
}

dcNode *dcScope_getNext(dcHashIterator *_iterator, dcScopeDataFlags _type)
{
    dcNode *that = dcHashIterator_getNextValue(_iterator);

    while (that != NULL
           && ((dcScopeData_getFlags(that) & _type) == 0))
    {
        that = dcHashIterator_getNextValue(_iterator);
    }

    return that;
}
