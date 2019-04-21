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

#include "dcClassManager.h"
#include "dcNode.h"
#include "dcGarbageCollector.h"
#include "dcIOClass.h"
#include "dcUnsignedInt32.h"
#include "dcList.h"
#include "dcMainClass.h"
#include "dcMemory.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcObjectStack.h"
#include "dcObjectStackList.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcSystem.h"
#include "dcTestClass.h"
#include "dcTestUtilities.h"

static dcNode *sTestObject = NULL;
static dcNodeEvaluator *sNodeEvaluator = NULL;
static dcObjectStackList *sObjectStackList = NULL;

static void testBasicGet(void)
{
    dcObjectStack *stack = dcObjectStack_create();
    dcObjectStack_pushScope(stack, dcSystem_getGlobalScope());
    dcTestUtilities_assert(dcObjectStack_getObject(stack, "io")
                              == dcIOClass_getInstance());
    dcObjectStack_free(&stack, DC_SHALLOW);
}

static void testGlobalGetAndObjectStack(void)
{
    dcObjectStack *stackOne = dcObjectStack_create();
    dcObjectStack_pushScope(stackOne, dcScope_createNode());
    dcObjectStackList *list = dcObjectStackList_create();
    dcObjectStackList_pushObjectStack(list, stackOne);
    dcTestUtilities_assert(dcObjectStackList_getObject(list, "io")
                              == dcIOClass_getInstance());
    dcObjectStackList_free(&list);
}

static void testTestClass(void)
{
    dcObjectStack *stack = dcObjectStack_create();
    dcObjectStack_setSelf(stack, sTestObject, false);
    dcNode *first = dcObjectStack_getObject(stack, "@variableA");
    dcTestUtilities_assert(first != NULL);
    dcNodeEvaluator_pushMark(sNodeEvaluator, first);
    dcNode *second = dcTestClass_getVariableA(sTestObject);
    dcTestUtilities_assert(second != NULL);
    dcNodeEvaluator_popMark(dcSystem_getNodeEvaluator());
    dcTestUtilities_assert(dcNodeEvaluator_evaluate(sNodeEvaluator, first)
                           == second);
    dcObjectStack_free(&stack, DC_SHALLOW);
}

static void testBadGetWithStack(dcObjectStack *_stack)
{
    dcTestUtilities_assert(dcObjectStack_getObject(_stack, "derDer")
                              == NULL);
    dcTestUtilities_assert(dcObjectStack_getObject(_stack, "@derDer")
                              == NULL);
    dcTestUtilities_assert(dcObjectStack_getObject(_stack, "@@derDer")
                              == NULL);
    dcTestUtilities_assert(dcObjectStack_getObject(_stack, "")
                              == NULL);
    dcTestUtilities_assert(dcObjectStack_getObject(_stack, " a")
                              == NULL);
}

static void testBadGet(void)
{
    dcObjectStack *stack = dcObjectStack_create();
    testBadGetWithStack(stack);
    dcObjectStack_setSelf(stack, sTestObject, false);
    testBadGetWithStack(stack);
    dcObjectStack_pushScope(stack, dcSystem_getGlobalScope());
    testBadGetWithStack(stack);
    dcObjectStack_free(&stack, DC_SHALLOW);
}

static void testTwoScopes(void)
{
    dcObjectStack *stack = dcObjectStack_create();
    dcScope *first = dcScope_create();
    dcScope *second = dcScope_create();
    dcNode *firstShell = dcScope_createShell(first);
    dcNode *secondShell = dcScope_createShell(second);

    dcObjectStack_pushScope(stack, firstShell);
    dcObjectStack_pushScope(stack, secondShell);

    //
    // test finding an object in a bottom scope
    //
    dcNode *one = dcUnsignedInt32_createNode(1);
    dcScope_setObject(first, one, "one", NO_FLAGS);
    dcTestUtilities_assert
        (dcObjectStack_getObject(stack, "one") == one);

    //
    // test finding an object in a top scope
    //
    dcNode *two = dcUnsignedInt32_createNode(2);
    dcScope_setObject(second, two, "two", NO_FLAGS);
    dcTestUtilities_assert
        (dcObjectStack_getObject(stack, "two") == two);

    //
    // test a shadowed object (doh)
    //
    dcNode *oneToo = dcUnsignedInt32_createNode(11);
    dcScope_setObject(second, oneToo, "one", NO_FLAGS);
    dcTestUtilities_assert
        (dcObjectStack_getObject(stack, "one") == oneToo);

    //
    // pop the scope and try again
    //
    dcObjectStack_popScope(stack, DC_SHALLOW);
    dcTestUtilities_assert
        (dcObjectStack_getObject(stack, "one") == one);
    dcObjectStack_popScope(stack, DC_SHALLOW);

    dcNode_free(&firstShell, DC_DEEP);
    dcNode_free(&secondShell, DC_DEEP);
    dcObjectStack_free(&stack, DC_SHALLOW);
}

static void testObjectStackListFail(dcObjectStackList *_list)
{
    // for fun
    dcTestUtilities_assert
        (dcObjectStackList_getObject(_list, "der") == NULL);
    dcTestUtilities_assert
        (dcObjectStackList_getObject(_list, "@der") == NULL);
    dcTestUtilities_assert
        (dcObjectStackList_getObject(_list, "@@der") == NULL);
}

static void testSimpleObjectStackList(void)
{
    dcObjectStackList *list = dcObjectStackList_create();
    dcNode *one = dcUnsignedInt32_createNode(1);
    dcNode *scope = dcScope_createNode();
    dcObjectStack *stack = dcObjectStack_create();
    dcScope_setObject(CAST_SCOPE(scope), one, "one", NO_FLAGS);
    dcObjectStack_pushScope(stack, scope);
    dcObjectStackList_pushObjectStack(list, stack);

    dcTestUtilities_assert
        (dcObjectStackList_getObject(list, "one") == one);

    testObjectStackListFail(list);

    dcObjectStack_popScope(stack, DC_DEEP);
    dcObjectStackList_popObjectStack(list, DC_SHALLOW);
    dcObjectStack_free(&stack, DC_SHALLOW);
    dcObjectStackList_free(&list);
}

static void testBreakthrough(void)
{
    dcObjectStack *breakthrough = dcObjectStack_create();
    breakthrough->breakthrough = true;
    dcObjectStack *stack = dcObjectStack_create();

    // create an object and add it to the base scope
    dcNode *one = dcUnsignedInt32_createNode(1);
    dcNode_register(one);
    dcNode *scope = dcScope_createNode();
    dcScope_setObject(CAST_SCOPE(scope), one, "one", NO_FLAGS);

    // add the scope to the object stack
    dcObjectStack_pushScope(stack, scope);

    dcObjectStackList *list = dcObjectStackList_create();
    dcObjectStackList_pushObjectStack(list, stack);
    dcObjectStackList_pushObjectStack(list, breakthrough);

    dcTestUtilities_assert
        (dcObjectStackList_getObject(list, "one") == one);

    // test with a self in the way
    dcObjectStack_setSelf(stack, sTestObject, false);
    dcTestUtilities_assert
        (dcObjectStackList_getObject(list, "one") == one);

    // test with another object stack in the way, fail
    dcObjectStack *secondStack = dcObjectStack_create();
    dcObjectStackList_pushObjectStack(list, secondStack);
    dcTestUtilities_assert
        (dcObjectStackList_getObject(list, "one") == NULL);

    // now succeed
    secondStack->breakthrough = true;
    dcTestUtilities_assert
        (dcObjectStackList_getObject(list, "one") == one);

    testObjectStackListFail(list);

    dcObjectStack_popScope(stack, DC_DEEP);
    dcObjectStackList_free(&list);
}

static void testBreakthroughPlusObject(void)
{
    //assert(false);
}

static void testObjectStackListMark(void)
{
    dcNode *one = dcUnsignedInt32_createNode(1);
    dcNode *two = dcUnsignedInt32_createNode(2);
    dcNode *three = dcUnsignedInt32_createNode(3);
    dcObjectStackList *list = dcObjectStackList_create();
    dcObjectStack *stackOne = dcObjectStack_create();
    dcObjectStack *stackTwo = dcObjectStack_create();
    dcNode *scopeOne = dcScope_createNode();
    dcNode *scopeTwo = dcScope_createNode();

    dcNode_register(one);
    dcNode_register(two);
    dcNode_register(three);

    dcScope_setObject(CAST_SCOPE(scopeOne), one, "one", NO_FLAGS);
    dcScope_setObject(CAST_SCOPE(scopeTwo), two, "two", NO_FLAGS);

    dcObjectStack_setSelf(stackOne, three, false);
    dcObjectStack_pushScope(stackOne, scopeOne);
    dcObjectStack_pushScope(stackTwo, scopeTwo);
    dcObjectStackList_pushObjectStack(list, stackOne);
    dcObjectStackList_pushObjectStack(list, stackTwo);

    dcTestUtilities_checkMark(one, false);
    dcTestUtilities_checkMark(two, false);
    dcTestUtilities_checkMark(three, false);

    dcObjectStackList_mark(list);

    dcTestUtilities_checkMark(one, true);
    dcTestUtilities_checkMark(two, true);
    dcTestUtilities_checkMark(three, true);

    sObjectStackList = list;

    // check that the garbage collector marks them
    dcNode_setGarbageCollectionTrapped(one, true);
    dcNode_setGarbageCollectionTrapped(two, true);
    dcNode_setGarbageCollectionTrapped(three, true);
    dcGarbageCollector_go();
    dcGarbageCollector_go();
    dcNode_setGarbageCollectionTrapped(one, false);
    dcNode_setGarbageCollectionTrapped(two, false);
    dcNode_setGarbageCollectionTrapped(three, false);

    dcObjectStackList_free(&list);
    sObjectStackList = NULL;
}

static void testConstSelf(void)
{
    // test a const/non-const self on the bottom of a three node stack
    {
        dcNode *self = dcUnsignedInt32_createNode(1);
        dcObjectStack *stack = dcObjectStack_create();
        dcObjectStack_setSelf(stack, self, true);

        dcObjectStackList *list = dcObjectStackList_create();
        dcObjectStackList_pushObjectStack(list, stack);
        dcObjectStackList_pushObjectStack(list, dcObjectStack_create());
        dcObjectStackList_pushObjectStack(list, dcObjectStack_create());

        dcTestUtilities_assert(dcObjectStackList_isObjectConst(list, self));
        stack->selfIsConst = false;
        dcTestUtilities_assert(! dcObjectStackList_isObjectConst(list, self));

        dcObjectStackList_free(&list);
        dcNode_free(&self, DC_DEEP);
    }

    // test a const/non-const self in the middle of a three node stack
    {
        dcObjectStackList *list = dcObjectStackList_create();
        dcObjectStackList_pushObjectStack(list, dcObjectStack_create());

        dcNode *self = dcUnsignedInt32_createNode(1);
        dcObjectStack *stack = dcObjectStack_create();
        dcObjectStack_setSelf(stack, self, true);

        dcObjectStackList_pushObjectStack(list, stack);
        dcObjectStackList_pushObjectStack(list, dcObjectStack_create());

        dcTestUtilities_assert(dcObjectStackList_isObjectConst(list, self));
        stack->selfIsConst = false;
        dcTestUtilities_assert(! dcObjectStackList_isObjectConst(list, self));

        dcObjectStackList_free(&list);
        dcNode_free(&self, DC_DEEP);
    }
}

static void marker(void)
{
    dcNode_mark(sTestObject);
    dcObjectStackList_mark(sObjectStackList);
}

static const dcTestFunctionMap map[] =
{
    {"Basic Get",                        &testBasicGet},
    {"Global Get + ObjectStack",         &testGlobalGetAndObjectStack},
    {"Test Class",                       &testTestClass},
    {"Bad Get",                          &testBadGet},
    {"Two Scopes",                       &testTwoScopes},
    {"Simple ObjectStackList",           &testSimpleObjectStackList},
    {"Breakthrough",                     &testBreakthrough},
    {"Breakthrough + object",            &testBreakthroughPlusObject},
    {"ObjecStackList Mark",              &testObjectStackListMark},
    {"Const Self",                       &testConstSelf},
    {NULL}
};

int main(int _argc, char **_argv)
{
    // set everything up
    dcMemory_initialize();
    dcSystem_create();
    dcTestUtilities_start("Object Stack Test", _argc, _argv);
    sNodeEvaluator = dcSystem_getCurrentNodeEvaluator();

    // register the test class and create an instance of it
    dcTestUtilities_assert(dcClassManager_registerClassTemplate
                           (dcTestClass_getTemplate(), NULL, true, NULL)
                           == TAFFY_SUCCESS);

    sTestObject = dcTestClass_createObject();
    dcNode_register(sTestObject);
    dcGarbageCollector_addRoot(&marker);

    // do the tests
    dcTestUtilities_runTests(NULL, map, false);

    // power down
    dcTestUtilities_end();
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
