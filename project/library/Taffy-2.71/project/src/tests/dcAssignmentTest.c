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

#include "dcArray.h"
#include "dcAssignment.h"
#include "dcClass.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcIdentifier.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcObjectStackList.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcStringEvaluator.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"

#define ASSIGNMENT_TEST_FILE_NAME "src/tests/dcAssignmentTest.c"

static dcNodeEvaluator *sNodeEvaluator = NULL;

static dcNode *testAssignmentResult(dcScopeDataFlags _identifierFlags,
                                    dcScopeDataFlags _assignmentFlags,
                                    const char *_name,
                                    const char *_exceptionPackageName,
                                    const char *_exceptionClassName,
                                    uint32_t _value)
{
    dcNode *value = dcNode_setTemplate
        (dcNumberClass_createObjectFromInt32s(_value), true);
    dcNode *assignment = dcAssignment_createNode
        (dcIdentifier_createNode(_name, _identifierFlags),
         value,
         _assignmentFlags);

    size_t markCount = dcNodeEvaluator_pushMark(sNodeEvaluator, value);

    // sanity, verify that value is not freed
    dcNode_setFreeTrapped(value, true);

    dcGraphData_setPosition(CAST_GRAPH_DATA(value),
                            dcStringManager_getStringId("dcAssignmentTest.c"),
                            1);

    dcNode *assignedValue =
        dcNodeEvaluator_evaluate(sNodeEvaluator, assignment);

    if (assignedValue != NULL)
    {
        dcTaffyOperator comparison;
        dcResult compareResult =
            dcNode_compare(value, assignedValue, &comparison);
        dcTestUtilities_assert(compareResult == TAFFY_SUCCESS
                               && comparison == TAFFY_EQUALS);
    }

    // garbage collect
    dcGarbageCollector_go();
    dcNode_setFreeTrapped(value, false);

    if (assignedValue == NULL)
    {
        dcTestUtilities_assert(_exceptionClassName != NULL);

        // check that an exception was thrown
        dcTestUtilities_assert
            (dcClass_isType
             (sNodeEvaluator->exception,
              _exceptionPackageName,
              _exceptionClassName));
    }
    else
    {
        // verify key wasn't deleted
        dcTestUtilities_assert
            (dcObjectStackList_getObject(sNodeEvaluator->objectStackList, _name)
             == assignedValue);
    }

    dcNodeEvaluator_popMarks(sNodeEvaluator, markCount);

    // we're done
    dcNode_free(&assignment, DC_DEEP);
    return assignedValue;
}

static dcNode *testAssignment(dcScopeDataFlags _identifierFlags,
                              dcScopeDataFlags _assignmentFlags,
                              const char *_name)
{
    return testAssignmentResult(_identifierFlags,
                                _assignmentFlags,
                                _name,
                                NULL,
                                NULL,
                                31337);
}

static void testLocalScopeAssignment(void)
{
    testAssignment(NO_FLAGS, NO_FLAGS, "testLocalScopeAssignment");
}

static void testGlobalScopeAssignment(void)
{
    const char *variableName = "testGlobalScopeAssignment";
    dcNode *assignedNode =
        testAssignment(NO_FLAGS, SCOPE_DATA_GLOBAL, variableName);

    // get the scope data from the global scope
    dcNode *scopeData = dcScope_getScopeDataForObject
        (CAST_SCOPE(dcSystem_getGlobalScope()), variableName);

    // make sure they're equal
    dcTestUtilities_assert(scopeData != NULL
                           && (dcScopeData_getObject(scopeData)
                               == assignedNode));
}

static void assertException(const dcNode *_assignmentResult,
                            const char *_exceptionString)
{
    // assert that an exception was thrown
    dcTestUtilities_assert
        (_assignmentResult == NULL
         && (dcClass_isType(sNodeEvaluator->exception,
                            EXCEPTION_PACKAGE_NAME,
                            _exceptionString)));
    dcNodeEvaluator_clearException(sNodeEvaluator, DC_DEEP);
}

static void testConstantRedefinition(void)
{
    const char *variableName = "testConstantRedefinition";
    const char *exceptionString = "ConstantRedefinitionException";

    dcTestUtilities_assert
        (testAssignment(NO_FLAGS, SCOPE_DATA_CONSTANT, variableName)
         != NULL);

    // redefine the constant object (exception time, baby!)
    dcTestUtilities_assert(testAssignmentResult(NO_FLAGS,
                                                NO_FLAGS,
                                                variableName,
                                                EXCEPTION_PACKAGE_NAME,
                                                exceptionString,
                                                31337)
                           == NULL);

    dcNodeEvaluator_clearException(sNodeEvaluator, DC_SHALLOW);
    dcNode *assignmentResult =
        dcStringEvaluator_evalString("^{ const ree = 1; ree = 2 } call",
                                     ASSIGNMENT_TEST_FILE_NAME,
                                     NO_STRING_EVALUATOR_FLAGS);

    assertException(assignmentResult, exceptionString);
}

static void testLocalToGlobalConversion(void)
{
    const char *variableName = "testLocalToGlobalConversion";
    const char *exceptionString = "LocalToGlobalConversionException";

    // create a local variable
    dcTestUtilities_assert(testAssignment(NO_FLAGS, NO_FLAGS, variableName)
                           != NULL);

    // create a global variable of the same name (exception time, baby!)
    dcTestUtilities_assert
        (testAssignmentResult(NO_FLAGS,
                              SCOPE_DATA_GLOBAL,
                              variableName,
                              EXCEPTION_PACKAGE_NAME,
                              exceptionString,
                              31337)
         == NULL);

    dcNodeEvaluator_clearException(sNodeEvaluator, DC_SHALLOW);
    dcNode *assignmentResult =
        dcStringEvaluator_evalString("^{ ree = 1; global ree = 2 } call",
                                     ASSIGNMENT_TEST_FILE_NAME,
                                     NO_STRING_EVALUATOR_FLAGS);

    assertException(assignmentResult, exceptionString);
}

// Set an object, push a scope, update the object,
// pop the scope, read the object
static void testStackedScopeUpdate(void)
{
    const char *variableName = "testStackedScopeUpdate";
    dcTestUtilities_assert(testAssignmentResult(NO_FLAGS,
                                                NO_FLAGS,
                                                variableName,
                                                NULL,
                                                NULL,
                                                1)
                           != NULL);

    // push a Re: stacked scope
    dcObjectStackList_pushScope(sNodeEvaluator->objectStackList,
                                dcScope_createNode());


    dcTestUtilities_assert(testAssignmentResult(NO_FLAGS,
                                                NO_FLAGS,
                                                variableName,
                                                NULL,
                                                NULL,
                                                2)
                           != NULL);

    // pop the Re: stacked scope, the variable should stay 2
    dcObjectStackList_popScope(sNodeEvaluator->objectStackList, DC_DEEP);

    dcNode *object = dcNodeEvaluator_findObject(sNodeEvaluator,
                                                variableName,
                                                false);
    dcTestUtilities_assert
        (object != NULL
         && dcClass_hasTemplate(object, dcNumberClass_getTemplate(), true)
         && dcNumberClass_equalsInt32u(object, 2));
}

static const dcTestFunctionMap sMap[] =
{
    {"Local Scope Assignment",     &testLocalScopeAssignment},
    {"Global Scope Assignment",    &testGlobalScopeAssignment},
    {"Stacked-scope Update",       &testStackedScopeUpdate},
    {"Constant Redefinition",      &testConstantRedefinition},
    {"Local to Global Conversion", &testLocalToGlobalConversion},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_createWithArguments
        (dcTaffyCommandLineArguments_parseAndCreateWithFailure(_argc,
                                                               _argv,
                                                               false));
    sNodeEvaluator = dcSystem_getCurrentNodeEvaluator();
    dcTestUtilities_go("Assignment Test", _argc, _argv, NULL, sMap, false);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
