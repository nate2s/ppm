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

#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcImport.h"
#include "dcImportAndPackageTest.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcParser.h"
#include "dcStringEvaluator.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"
#include "dcYesClass.h"

#define IMPORT_TEST_FILE_NAME "src/tests/dcImportTest.c"

static dcNodeEvaluator *sNodeEvaluator = NULL;

static dcList *createPath(const char *_path)
{
    return dcLexer_splitString(_path, '.');
}

static void testImportSingleClass(void)
{
}

static void testImport(void)
{
    const char *paths[] =
        {"org.taffy.*",
         "org.taffy.core.Object",
         "org.*",
         "net.*",
         "org.org.Taffy"};
    size_t i;

    for (i = 0; i < dcTaffy_countOf(paths); i++)
    {
        dcList *path = createPath(paths[i]);
        dcNode *import = dcImport_createNode(dcList_copy(path, DC_DEEP));
        dcImportAndPackageTest_checkContents(CAST_IMPORT(import), paths[i]);
        dcList_free(&path, DC_DEEP);
        dcNode_free(&import, DC_DEEP);
    }
}

static void testImportWithHangingPeriod(void)
{
    dcTestUtilities_assert(dcParser_parseString("import org.",
                                                IMPORT_TEST_FILE_NAME,
                                                false)
                           == NULL);
}

static void testImportWithParser(void)
{
    const char *imports[] =
        {"import org",
         "import org.*",
         "import org.taffy.*",
         "import org.Object",
         "import org.taffy.core.Object"};
    size_t i;

    for (i = 0; i < dcTaffy_countOf(imports); i++)
    {
        dcNode *head = dcParser_parseString(imports[i],
                                            IMPORT_TEST_FILE_NAME,
                                            false);
        dcGraphData_assertType(dcGraphDataTree_getContents(head), NODE_IMPORT);
        dcNode_free(&head, DC_DEEP);
    }
}

static void testImportWild(void)
{
    dcStringEvaluator_evalString
        ("import src.tests.TestDirectory.simpleFiles.*\n"
         "[kernel assert: [fileA getIt] == 1]",
         "testImportWild.c",
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
}

static void testImportString(void)
{
    dcStringEvaluator_evalString("import org.taffy.core.String",
                                 IMPORT_TEST_FILE_NAME,
                                 STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
}

static void testImportSelf(void)
{
    dcNode *result = (dcStringEvaluator_evalString
                      ("import src.tests.TestDirectory.ImportSelfClass",
                       IMPORT_TEST_FILE_NAME,
                       STRING_EVALUATOR_ASSERT_NO_EXCEPTION));
    dcTestUtilities_assert(dcYesClass_isMe(result));
}

static void testNoPackageImport(void)
{
    dcTestUtilities_assertException
        (dcStringEvaluator_evalString
         ("import src.tests.TestDirectory.ClassThatImportsClassWithoutPackage\n"
          "new ClassThatImportsClassWithoutPackage",
          IMPORT_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "ImportFailedException",
         "@packageName",
         "\"src.tests.TestDirectory.ClassWithoutPackage\"",
         true);
}

static void testParseErrorImportImport(void)
{
    dcTestUtilities_assertException
        (dcStringEvaluator_evalString
         ("import src.tests.TestDirectory.ClassThatImportsClassWithParseError\n"
          "new ClassThatImportsClassWithParseError",
          IMPORT_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "ParseErrorException",
         NULL,
         NULL,
         true);
}

static void testImportPlusImport(void)
{
    // TestClass's constructor instantiates a List object, who's package depends
    // on an import in TestClass.ty
    dcStringEvaluator_evalString
        ("import src.tests.TestDirectory.TestClass\n"
         "testClass = new TestClass\n"
         "kernel assert: [[testClass x] className] == \"List\"\n"
         "kernel assert: [[testClass y] className] == \"TestClassDeu\"\n",
         IMPORT_TEST_FILE_NAME,
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
}

static void testInvalidImport(void)
{
    dcTestUtilities_assertException
        (dcStringEvaluator_evalString
         ("import src.tests.TestDirectory.InvalidPackage",
          IMPORT_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "ImportFailedException",
         "@packageName",
         "\"src.tests.TestDirectory.InvalidPackage\"",
         true);

    dcTestUtilities_assertException
        (dcStringEvaluator_evalString
         ("import reeeeeee;"
          "class ThisIsATest {}",
          IMPORT_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "ImportFailedException",
         "@packageName",
         "\"reeeeeee\"",
         true);

    dcTestUtilities_assertException
        (dcStringEvaluator_evalString
         ("import src.tests.TestDirectory.InvalidImport;",
          IMPORT_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "ImportFailedException",
         "@packageName",
         "\"asdf\"",
         true);
}

static void testWildImport(void)
{
    dcStringEvaluator_evalString
        ("import src.tests.TestDirectory.WildImports1.*;"
         "new src.tests.TestDirectory.WildImports1.ClassA;"
         "new src.tests.TestDirectory.WildImports1.ClassB;"
         "new src.tests.TestDirectory.WildImports1.ClassC;",
         IMPORT_TEST_FILE_NAME,
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);

    dcStringEvaluator_evalString
        ("import src.tests.TestDirectory.WildImportsWithPackageImport.*;"
         "new src.tests.TestDirectory.WildImportsWithPackageImport.ClassA;"
         "new src.tests.TestDirectory.WildImportsWithPackageImport.ClassB;"
         "new src.tests.TestDirectory.WildImportsWithPackageImport.ClassC;",
         IMPORT_TEST_FILE_NAME,
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
}

static void testImportLoop(void)
{
    dcStringEvaluator_evalString
        ("import src.tests.TestDirectory.ImportLoop.PackageA.*;"
         "import src.tests.TestDirectory.ImportLoop.PackageB.*;"
         "import src.tests.TestDirectory.ImportLoop.PackageC.*;"
         "importLoopClassA = new ImportLoopClassA;"
         "importLoopClassB = new ImportLoopClassB;"
         "importLoopClassC = new ImportLoopClassC;",
         IMPORT_TEST_FILE_NAME,
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
}

//
// First we import everything from src.tests.TestDirectory.WildImports
// Then we import a class that imports a class inside of WildImports
// We verify that it succeeds
//
static void testImportAgain(void)
{
    dcStringEvaluator_evalString
        ("import src.tests.TestDirectory.WildImports2.ClassA;"
         "import src.tests.TestDirectory.WildImports2.*;"
         "new ClassA",
         IMPORT_TEST_FILE_NAME,
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);

     dcStringEvaluator_evalString
         ("import src.tests.TestDirectory.WildImports3.*;"
          "import src.tests.TestDirectory.WildImports3.ClassA;"
          "new ClassA;",
          IMPORT_TEST_FILE_NAME,
          STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
}

static void testImportInPackage(void)
{
    dcStringEvaluator_evalString
        ("import src.tests.TestDirectory.ImportInPackage.ClassZ;"
         "new ClassZ;",
         IMPORT_TEST_FILE_NAME,
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
}

static void testImportClassWithProtected(void)
{
    dcStringEvaluator_evalString
        ("import src.tests.TestDirectory.ClassWithProtected",
         IMPORT_TEST_FILE_NAME,
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
}

static const dcTestFunctionMap sTests[] =
{
    {"Import", &testImport},
    {NULL}
};

static const dcTestFunctionMap sRuntimeMap[] =
{
    {"Import With Hanging Period",  &testImportWithHangingPeriod},
    {"Import + Parser",             &testImportWithParser},
    {"Wiiiild Import",              &testWildImport},
    {"Import Single Class",         &testImportSingleClass},
    {"Import Wild",                 &testImportWild},
    {"Import String",               &testImportString},
    {"Import Self",                 &testImportSelf},
    {"No Package Import",           &testNoPackageImport},
    {"Parse Error Import Import",   &testParseErrorImportImport},
    {"Import + Import",             &testImportPlusImport},
    {"Invalid Import",              &testInvalidImport},
    {"Import Loop",                 &testImportLoop},
    {"Import Again",                &testImportAgain},
    {"Import in Package",           &testImportInPackage},
    {"Import Class With Protected", &testImportClassWithProtected},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcStringManager_create();
    dcLexer_initialize();

    dcTestUtilities_go("Import Test", _argc, _argv, NULL, sTests, false);

    if (dcTestUtilities_runSystem())
    {
        dcSystem_create();
        sNodeEvaluator = dcSystem_getCurrentNodeEvaluator();
        printf("\n");
        dcTestUtilities_go("Import Runtime Test",
                           _argc,
                           _argv,
                           NULL,
                           sRuntimeMap,
                           true);
        dcSystem_free();
    }

    dcLexer_cleanup();
    dcGarbageCollector_free();
    dcStringManager_free();
    dcMemory_deinitialize();
    return 0;
}
