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
#include <string.h>

#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcHash.h"
#include "dcImport.h"
#include "dcIOClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcObjectClass.h"
#include "dcPackage.h"
#include "dcPackageContents.h"
#include "dcParser.h"
#include "dcProcedureClass.h"
#include "dcScope.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"
#include "dcTestClass.h"

#define CLASS_MANAGER_TEST_FILE "src/tests/dcClassManagerTest.c"

static dcPackage *sCorePackage = NULL;

static void testTestClass(void)
{
    dcClassTemplate *classTemplate = dcTestClass_getTemplate();
    dcNode *meta;
    dcTestUtilities_assert(dcClassManager_registerClassTemplate
                           (classTemplate, NULL, true, &meta)
                           == TAFFY_SUCCESS);
    dcNode *contents =
        dcClassManager_getPackageContents(classTemplate->package);
    dcNode *getResult = NULL;

    dcTestUtilities_assert
        (contents != NULL
         && (dcHash_getValueWithStringKey
             (CAST_PACKAGE_CONTENTS(contents)->classes,
              "TestClass",
              &getResult)
             == TAFFY_SUCCESS)
         && getResult == meta);
}

typedef struct
{
    const char *packageName;
    const char *name;
    const char *superName;
} ClassTemplateData;

static void testClasses(const ClassTemplateData *_data,
                        bool _checkGet,
                        dcList *_packageContentsList)
{
    size_t i;

    for (i = 0; _data[i].packageName != NULL; i++)
    {
        dcPackage *package = dcPackage_createFromString(_data[i].packageName);
        dcClassTemplate *classTemplate = dcClassTemplate_createSimple
            (_data[i].packageName,
             _data[i].name,
             _data[i].superName,
             NO_FLAGS,
             NO_FLAGS);
        dcNode *object;
        dcTestUtilities_assert(dcClassManager_registerClassTemplate
                               (classTemplate, NULL, true, &object)
                               == TAFFY_SUCCESS);
        dcNode *contents =
            dcClassManager_getPackageContents(package);
        dcNode *getResult = NULL;

        dcTestUtilities_assert
            (contents != NULL
             && (dcHash_getValueWithStringKey
                 (CAST_PACKAGE_CONTENTS(contents)->classes,
                  _data[i].name,
                  &getResult)
                 == TAFFY_SUCCESS)
             && getResult == object);

        dcPackage_free(&package);
    }
}

static void testLevelZero(void)
{
    const ClassTemplateData data[] =
    {
        {"", "MaCom", NULL},
        {"", "MaOrg", "MaCom"},
        {"", "MaNet", "MaOrg"},
        {NULL}
    };

    testClasses(data, true, NULL);
}

static void testLevelOne(void)
{
    const ClassTemplateData data[] =
    {
        {"com", "MaCom", NULL},
        {"org", "MaOrg", "com.MaCom"},
        {"net", "MaNet", "org.MaOrg"},
        {NULL}
    };

    testClasses(data, true, NULL);
}

static void testLevelN(void)
{
    const ClassTemplateData data[] =
    {
        {"com.taffy.core", "MaCom", NULL},
        {"org.taffy.core", "MaOrg", "com.taffy.core.MaCom"},
        {"net.taffy.core", "MaNet", "org.taffy.core.MaOrg"},
        {NULL}
    };

    testClasses(data, true, NULL);
}

static void populatePackageContentsList(dcList *_list, const char *_name)
{
    dcPackage *package = dcPackage_createFromString(_name);
    dcNode *contents = dcClassManager_getPackageContents(package);
    dcTestUtilities_assert(contents != NULL);
    dcList_push(_list, contents);
    dcPackage_free(&package);
}

//
// Test importing a class via a non-fully-qualified class name
// This uses a package contents list to hold data from "import" directives
//
static void testImport(void)
{
    const ClassTemplateData data[] =
    {
        {"com",            "ComDude",             NULL},
        {"com.taffy",      "ComTaffyDude",        "com.ComDude"},
        {"com.taffy.core", "ComTaffyCoreDudette", "com.taffy.ComTaffyDude"},
        {NULL}
    };

    dcList *packageContentsList = dcList_create();

    populatePackageContentsList(packageContentsList, "com");
    populatePackageContentsList(packageContentsList, "com.taffy");
    populatePackageContentsList(packageContentsList, "com.taffy.core");

    testClasses(data, false, packageContentsList);

    size_t i;

    for (i = 0; i < dcTaffy_countOf(data) - 1; i++)
    {
        dcTestUtilities_assert(dcClassManager_getClass
                               (data[i].name, NULL, packageContentsList, NULL)
                               != NULL);
    }

    dcList_free(&packageContentsList, DC_SHALLOW);
}

static void testCollision(void)
{
    size_t i;

    for (i = 0; i < 3; i++)
    {
        dcClassTemplate *classTemplate =
            dcClassTemplate_createSimple("org.taffy.core.tests",
                                         "IWillCollide",
                                         "org.taffy.core.Object",
                                         NO_FLAGS,
                                         NO_FLAGS);
        dcNode *metaResult = NULL;
        dcResult result = dcClassManager_registerClassTemplate
            (classTemplate, NULL, true, &metaResult);

        if (i == 0)
        {
            dcTestUtilities_assert(result == TAFFY_SUCCESS);
            dcGraphData_assertType(metaResult, NODE_CLASS);
        }
        else
        {
            dcTestUtilities_assert(result == TAFFY_FAILURE
                                   && metaResult == NULL);
            dcClassTemplate_free(&classTemplate, DC_DEEP);
        }
    }
}

static dcNode *createImport(const char *_import)
{
    return dcImport_createNode(dcLexer_splitString(_import, '.'));
}

static void testCollisionFromCopy(void)
{
    size_t i;
    dcClassTemplate *classTemplate =
        dcClassTemplate_createSimple("org.taffy.core.tests",
                                     "IWillCollideAgain",
                                     "org.taffy.core.Object",
                                     NO_FLAGS,
                                     NO_FLAGS);

    dcNode *import1 = createImport("org.taffy.core");
    dcNode *import2 = createImport("org.taffy.core.exception");
    dcList *deferredImports = dcList_createWithObjects(import1, import2, NULL);
    dcList *imports = dcList_create();
    dcClassTemplate_createRuntimeValues(classTemplate, NULL);

    for (i = 0; i < 1; i++)
    {
        dcClassTemplate *copy = dcClassTemplate_copy(classTemplate, DC_DEEP);
        dcNode *metaResult = NULL;
        dcResult result = dcClassManager_registerClassTemplate
            (copy, NULL, true, &metaResult);

        if (i == 0)
        {
            dcTestUtilities_assert(result == TAFFY_SUCCESS);
            dcGraphData_assertType(metaResult, NODE_CLASS);
        }
        else
        {
            dcTestUtilities_assert(result == TAFFY_FAILURE
                                   && metaResult == NULL);
            dcClassTemplate_free(&copy, DC_DEEP);
        }
    }

    dcList_free(&deferredImports, DC_DEEP);
    dcList_free(&imports, DC_DEEP);
    dcClassTemplate_free(&classTemplate, DC_DEEP);
}

static void testCopyTemplate(void)
{
    dcClassTemplate *classTemplate =
        dcClassTemplate_createSimple("org.taffy.core.tests",
                                     "IWillCollideAgain",
                                     "org.taffy.core.Object",
                                     NO_FLAGS,
                                     NO_FLAGS);
    dcClassTemplate *copy = dcClassTemplate_copy(classTemplate, DC_SHALLOW);
    dcTestUtilities_assert(strcmp(copy->className,
                                  classTemplate->className) == 0
                           && (strcmp(copy->superName, classTemplate->superName)
                               == 0));
    dcClassTemplate_free(&classTemplate, DC_DEEP);
    dcClassTemplate_free(&copy, DC_DEEP);
}

static void testDeepCopyTemplate(void)
{
    dcClassTemplate *classTemplate =
        dcClassTemplate_createSimple("org.taffy.core.tests",
                                     "IWillCollideAgain",
                                     "org.taffy.core.Object",
                                     NO_FLAGS,
                                     NO_FLAGS);
    dcClassTemplate *copy = dcClassTemplate_copy(classTemplate, DC_DEEP);
    dcTestUtilities_assert(strcmp(copy->className,
                                  classTemplate->className) == 0
                           && (strcmp(copy->superName, classTemplate->superName)
                               == 0));
    dcClassTemplate_free(&classTemplate, DC_DEEP);
    dcClassTemplate_free(&copy, DC_DEEP);
}

static void testClass(const char *_className, const char *_program)
{
    dcNode *graphDataTree =
        dcParser_parseString(_program, CLASS_MANAGER_TEST_FILE, true);
    dcGraphData_assertType(graphDataTree, NODE_GRAPH_DATA_TREE);
    dcNode *klass = dcGraphDataTree_getContents(graphDataTree);
    dcGraphData_assertType(klass, NODE_CLASS);
    dcClassTemplate *classTemplate = dcClass_getTemplate(klass);
    dcTestUtilities_assert(strcmp(classTemplate->className, _className) == 0);
    dcNode_free(&graphDataTree, DC_DEEP);
}

static void testParseClass(void)
{
    testClass("TestParseClass",
              "class TestParseClass { }");
}

static void testParseClassWithGuts(void)
{
    testClass("TestParseClass",
              "class TestParseClass \n"
              "{ (@) hiThere: _one { } }");
}

static void testPackageContentsCopy(void)
{
    dcPackage *package = dcPackage_createFromString("org.taffy.core");
    dcNode *packageContents = dcClassManager_getPackageContents(package);
    dcTestUtilities_assert(packageContents != NULL);
    dcNode *copy = dcNode_copy(packageContents, DC_SHALLOW);
    dcNode_free(&copy, DC_SHALLOW);
    dcPackage_free(&package);
}

static void testClassWithClass(void)
{
    const char *program =
        "package org.taffy.core.tests\n"
        "class Tester { class TesterBaby {} }";

    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *head = dcParser_parseString(program, CLASS_MANAGER_TEST_FILE, true);
    dcTestUtilities_assert(head != NULL);
    dcTestUtilities_assert(dcNodeEvaluator_evaluate(evaluator, head)
                           != NULL);

    // now verify that we can get both class Tester and its baby TesterBaby
    dcPackage *testsPackage =
        dcPackage_createFromString("org.taffy.core.tests");
    dcPackage *babyPackage =
        dcPackage_createFromString("org.taffy.core.tests.Tester");

    dcClassTemplate *testerTemplate =
        dcClassManager_getClassTemplate("Tester", testsPackage, NULL, NULL);
    assert(testerTemplate != NULL
           && strcmp(testerTemplate->className, "Tester") == 0);

    // test with TesterBaby class name and org.taffy.core.tests.Tester package
    dcClassTemplate *babyTemplate =
        dcClassManager_getClassTemplate("TesterBaby", babyPackage, NULL, NULL);
    assert(babyTemplate != NULL
           && strcmp(babyTemplate->className, "TesterBaby") == 0);

    // test with Tester.TesterBaby class name and org.taffy.core.tests package
    assert(dcClassManager_getClassTemplate("Tester.TesterBaby",
                                           testsPackage,
                                           NULL,
                                           NULL)
           == babyTemplate);

    // test with tests.Tester.TesterBaby class name and org.taffy.core package
    dcPackage *corePackage = dcPackage_createFromString("org.taffy.core");
    assert(dcClassManager_getClassTemplate("tests.Tester.TesterBaby",
                                           corePackage,
                                           NULL,
                                           NULL)
           == babyTemplate);

    // evaluate the parent to expose a bug where we registered its baby twice
    // (that shouldn't happen)
    dcNode *momma = dcClassManager_getClass("Tester", testsPackage, NULL, NULL);
    assert(dcNodeEvaluator_evaluate(evaluator, momma) != NULL);

    dcPackage_free(&corePackage);
    dcPackage_free(&testsPackage);
    dcPackage_free(&babyPackage);
    dcNode_free(&head, DC_DEEP);
}

static const dcTestFunctionMap sTestMap[] =
{
    {"Level Zero",               &testLevelZero},
    {"Level One",                &testLevelOne},
    {"Level N",                  &testLevelN},
    {"Import",                   &testImport},
    {"Copy Template",            &testCopyTemplate},
    {"Deep Copy Template",       &testDeepCopyTemplate},
    {NULL}
};

static const dcTestFunctionMap sRuntimeTestMap[] =
{
    {"Parse Class",              &testParseClass},
    {"Parse Class With Guts",    &testParseClassWithGuts},
    {"TestClass",             &testTestClass},
    {"Collision",             &testCollision},
    {"Collision from Copy",   &testCollisionFromCopy},
    {"Package Contents Copy", &testPackageContentsCopy},
    {"Class with Class",      &testClassWithClass},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcStringManager_create();
    dcMutex_initialize();
    dcClassManager_create();
    dcLexer_initialize();
    sCorePackage = dcPackage_createFromString(CORE_PACKAGE_NAME);
    dcClassTemplate *procedureTemplate = dcProcedureClass_getTemplate();
    dcTestUtilities_go("ClassManager Test",
                       _argc,
                       _argv,
                       NULL,
                       sTestMap,
                       false);

    if (dcTestUtilities_runSystem())
    {
        dcSystem_create();
        printf("\n");
        dcTestUtilities_go("ClassManager System Test",
                           _argc,
                           _argv,
                           NULL,
                           sRuntimeTestMap,
                           true);
        dcSystem_free();
    }
    else
    {
        dcStringManager_free();
        dcClassManager_free();
        dcGarbageCollector_free();
        dcClassTemplate_free(&procedureTemplate, DC_DEEP);
    }

    dcLexer_cleanup();
    dcPackage_free(&sCorePackage);
    dcMemory_deinitialize();
    return 0;
}
