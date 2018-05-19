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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcDefines.h"
#include "dcCFunctionArgument.h"
#include "dcContainers.h"
#include "dcGarbageCollector.h"
#include "dcHash.h"
#include "dcUnsignedInt32.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcTestUtilities.h"

static dcContainerSizeType sScopeSize = 0;

static void reset(void)
{
    sScopeSize = 0;
}

static void testAdd(const char *_key,
                    dcScope *_scope,
                    dcScopeDataFlags _flags,
                    bool _new)
{
    dcNode *one = dcUnsignedInt32_createNode(1);
    dcNode_register(one);
    dcNode *scopeData = dcScope_set(_scope, one, _key, _flags);

    if (scopeData == NULL)
    {
        dcTestUtilities_assert(! _new);
    }
    else
    {
        dcTestUtilities_assert(dcScopeData_getFlags(scopeData)
                               == _flags);
        dcTestUtilities_assert(dcScopeData_getObject(scopeData) == one);
        dcTestUtilities_assert(strcmp(dcScopeData_getName(scopeData), _key)
                               == 0);
        if (_new)
        {
            sScopeSize++;
        }

        dcTestUtilities_assert(dcScope_getSize(_scope) == sScopeSize);
    }
}

static void testGet(const char *_key,
                    dcScope *_scope,
                    dcScopeDataFlags _flags)
{
    dcNode *scopeData = dcScope_getScopeData(_scope, _key, _flags);
    dcTestUtilities_assert(scopeData != NULL
                           && (dcScopeData_getFlags(scopeData) == _flags)
                           && (strcmp(dcScopeData_getName(scopeData), _key)
                               == 0));
}

static void testAddGet(const char *_key,
                       dcScope *_scope,
                       dcScopeDataFlags _flags,
                       bool _new)
{
    testAdd(_key, _scope, _flags, _new);
    testGet(_key, _scope, _flags);
}

static void testObjectAddAndGet(void)
{
    dcScope *scope = dcScope_create();
    testAddGet("one", scope, SCOPE_DATA_OBJECT, true);
    dcScope_free(&scope, DC_DEEP);
}

static void testMethodAddAndGet(void)
{
    dcScope *scope = dcScope_create();
    testAddGet("one", scope, SCOPE_DATA_METHOD, true);
    testAddGet("one:", scope, SCOPE_DATA_METHOD, true);
    testAddGet("one:two:three:", scope, SCOPE_DATA_METHOD, true);
    dcScope_free(&scope, DC_DEEP);
}

static void testOperatorAddAndGet(void)
{
    int i;
    dcScope *scope = dcScope_create();

    // first add them all
    for (i = (int)TAFFY_ADD; i < (int)TAFFY_LAST_OPERATOR; i++)
    {
        testAdd(dcSystem_getOperatorName((dcTaffyOperator)i),
                scope,
                SCOPE_DATA_METHOD,
                true);
    }

    // then get them all
    for (i = (int)TAFFY_ADD; i < (int)TAFFY_LAST_OPERATOR; i++)
    {
        testGet(dcSystem_getOperatorName((dcTaffyOperator)i),
                scope,
                SCOPE_DATA_METHOD);
    }

    dcScope_free(&scope, DC_DEEP);
}

static void testDuplicateObject(void)
{
    dcScope *scope = dcScope_create();
    testAddGet("one", scope, SCOPE_DATA_OBJECT, true);
    testAddGet("one", scope, SCOPE_DATA_OBJECT, false);
    testAddGet("one", scope, SCOPE_DATA_METHOD, false);
    dcScope_free(&scope, DC_DEEP);
}

static void testDuplicateMethod(void)
{
    dcScope *scope = dcScope_create();
    testAddGet("one", scope, SCOPE_DATA_METHOD, true);
    testAddGet("one", scope, SCOPE_DATA_METHOD, false);
    testAddGet("one", scope, SCOPE_DATA_OBJECT, false);
    dcScope_free(&scope, DC_DEEP);
}

static void testGets(void)
{
    dcScope *scope = dcScope_create();
    dcNode *one = dcUnsignedInt32_createNode(1);
    dcNode *method = dcUnsignedInt32_createNode(2); // lol

    dcTestUtilities_assert(dcScope_setObject(scope, one, "one", NO_FLAGS)
                           != NULL);
    dcTestUtilities_assert(dcScope_setMethod(scope, method, "method", NO_FLAGS)
                           != NULL);

    dcTestUtilities_assert(dcScope_getObject(scope, "one") == one);
    dcTestUtilities_assert(dcScope_getMethod(scope, "method") == method);
    dcScope_free(&scope, DC_DEEP);
}

static void testMerge(void)
{
    dcScope *to = dcScope_create();
    dcScope *from = dcScope_create();

    dcNode *one = dcUnsignedInt32_createNode(1);
    dcNode *two = dcUnsignedInt32_createNode(2);

    dcTestUtilities_assert(dcScope_setObject(from, one, "one", NO_FLAGS)
                           != NULL);
    dcTestUtilities_assert(dcScope_setObject(from, two, "two", NO_FLAGS)
                           != NULL);

    dcScope_merge(to, from);

    dcNode *toOne = dcScope_getObject(to, "one");
    dcNode_assertType(toOne, NODE_INT);
    dcTestUtilities_assert(dcUnsignedInt32_getInt(toOne) == 1);

    dcNode *toTwo = dcScope_getObject(to, "two");
    dcNode_assertType(toTwo, NODE_INT);
    dcTestUtilities_assert(dcUnsignedInt32_getInt(toOne) == 1);

    dcScope_free(&to, DC_DEEP);
    dcScope_free(&from, DC_DEEP);
}

static dcNode *asStringMethod(dcNode *_receiver, dcArray *_arguments)
{
    return NULL;
}

static dcNode *operatorEqualEqualMethod(dcNode *_receiver, dcArray *_arguments)
{
    return NULL;
}

static const dcTaffyCMethodWrapper wrappers[] =
{
    {
        "asString",
        SCOPE_DATA_PUBLIC,
        &asStringMethod,
        gCFunctionArgument_none
    },
    {
        "#operator(==):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH),
        &operatorEqualEqualMethod,
        gCFunctionArgument_wild
    },
    {
        NULL
    }
};

static void testTaffyCMethodIntegration(void)
{
    dcScope *scope = dcScope_createFromTaffyCMethodWrappers(wrappers);
    testGet("asString", scope, (SCOPE_DATA_PUBLIC | SCOPE_DATA_METHOD));
    testGet("#operator(==):",
            scope,
            (SCOPE_DATA_METHOD
             | SCOPE_DATA_PUBLIC
             | SCOPE_DATA_BREAKTHROUGH));
    dcScope_free(&scope, DC_DEEP);
}

static void testCopy(void)
{
    // copy blank
    {
        dcScope *scope = dcScope_create();
        dcScope *copy = dcScope_copy(scope, DC_DEEP);
        dcScope_free(&scope, DC_DEEP);
        dcScope_free(&copy, DC_DEEP);
    }

    // copy with one object
    // copy with three objects
}

static const dcTestFunctionMap map[] =
{
    {"Object Add + Get",   &testObjectAddAndGet},
    {"Method Add + Get",   &testMethodAddAndGet},
    {"Operator Add + Get", &testOperatorAddAndGet},
    {"Duplicate Object",   &testDuplicateObject},
    {"Duplicate Method",   &testDuplicateMethod},
    {"Gets",               &testGets},
    {"Merge",              &testMerge},
    {"Copy",               &testCopy},
    {NULL}
};

static const dcTestFunctionMap runtimeMap[] =
{
    {"Taffy C Method Integration", &testTaffyCMethodIntegration},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcTestUtilities_go("Scope Test", _argc, _argv, &reset, map, false);

    // investigate more about dcScope_merge()
    if (dcTestUtilities_runSystem())
    {
        dcSystem_create();
        dcTestUtilities_go("Scope Test Runtime",
                           _argc,
                           _argv,
                           &reset,
                           runtimeMap,
                           true);
        dcSystem_free();
    }

    dcGarbageCollector_free();
    dcMemory_deinitialize();
    return 0;
}
