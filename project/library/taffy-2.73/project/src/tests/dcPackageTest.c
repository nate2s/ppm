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

#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcImportAndPackageTest.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcPackage.h"
#include "dcParser.h"
#include "dcString.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"

#define PACKAGE_TEST_FILE_NAME "src/tests/dcPackageTest.c"

static void testPackage(void)
{
    const char *paths[] =
        {"org.taffy",
         "org.taffy.core",
         "org",
         "net",
         "org.org.taffy"};
    size_t i;

    for (i = 0; i < dcTaffy_countOf(paths); i++)
    {
        dcList *path = dcImportAndPackageTest_createPath(paths[i]);
        dcPackage *package = dcPackage_create(path);
        dcImportAndPackageTest_checkContents(package, paths[i]);
        dcPackage_free(&package);

        path = dcImportAndPackageTest_createPath(paths[i]);
        dcNode *packageNode = dcPackage_createNode(path);
        dcGraphData_assertType(packageNode, NODE_PACKAGE);
        dcImportAndPackageTest_checkContents(CAST_PACKAGE(packageNode), paths[i]);
        dcNode_free(&packageNode, DC_DEEP);
    }
}

static void testGetPathString(void)
{
    const char *names[] =
    {
        "",                 "",
        "*",                "*",
        "org",              "org",
        "org.taffy.core",   "org.taffy.core",
        "org.taffy.core.*", "org.taffy.core.*"
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(names); i += 2)
    {
        dcPackage *package = dcPackage_createFromString(names[i]);
        char *pathString = dcPackage_getPathString(package);
        dcTestUtilities_assert(strcmp(pathString, names[i + 1])
                               == 0);
        dcPackage_free(&package);
        dcMemory_free(pathString);
    }
}

static void testInvalidPackage(void)
{
    const char *invalidPackage = "org.taffy.core.Object";
    invalidPackage++;
}

static void testCopy(void)
{
    // with packages
    {
        dcPackage *first = dcPackage_createFromString("org.taffy.core.tests");
        dcPackage *copy = dcPackage_copy(first, DC_DEEP);
        dcTaffyOperator compareResult;

        dcTestUtilities_assert((dcPackage_compare(first,
                                                  copy,
                                                  &compareResult)
                                == TAFFY_SUCCESS)
                               && compareResult == TAFFY_EQUALS);

        dcPackage_free(&first);
        dcPackage_free(&copy);
    }

    // and with nodes
    {
        dcNode *first =
            dcPackage_createNode
            (dcImportAndPackageTest_createPath("org.taffy.core.tests"));
        dcNode *copy = dcNode_copy(first, DC_DEEP);
        dcTaffyOperator compareResult;

        dcTestUtilities_assert((dcPackage_compareNode(first,
                                                      copy,
                                                      &compareResult)
                                == TAFFY_SUCCESS)
                               && compareResult == TAFFY_EQUALS);

        dcNode_free(&first, DC_DEEP);
        dcNode_free(&copy, DC_DEEP);
    }
}

static void testPackageWithNodeEvaluator(void)
{
    dcNode *parseHead = dcParser_parseString("package org.taffy.core\n",
                                             PACKAGE_TEST_FILE_NAME,
                                             false);
    dcNode *realHead = dcGraphDataTree_getContents(parseHead);
    dcGraphData_assertType(realHead, NODE_PACKAGE);
    dcImportAndPackageTest_checkContents(CAST_PACKAGE(realHead),
                                         "org.taffy.core");
    dcNode_free(&parseHead, DC_DEEP);
}

static void testBlankPackage(void)
{
    dcPackage *blanky = dcPackage_createFromString("");
    dcTestUtilities_assert(blanky->path->size == 0);
    dcPackage_free(&blanky);
}

static void testDirectoryExtraction(void)
{
    // TODO: make these play nice with windows
    const char *names[] =
    {
        "",                 "",
        "*",                "",
        "org",              "org",
        "org.taffy.core",   "org/taffy/core",
        "org.taffy.core.*", "org/taffy/core"
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(names); i += 2)
    {
        dcPackage *package = dcPackage_createFromString(names[i]);
        char *directory = dcPackage_extractDirectoryName(package);
        dcTestUtilities_assert(strcmp(directory, names[i + 1])
                               == 0);
        dcPackage_free(&package);
        dcMemory_free(directory);
    }
}

static const dcTestFunctionMap sMap[] =
{
    {"Blank Package",               &testBlankPackage},
    {"Copy",                        &testCopy},
    {"Get Path String",             &testGetPathString},
    {"Invalid Package",             &testInvalidPackage},
    {"Package",                     &testPackage},
    {"Directory Extraction",        &testDirectoryExtraction},
    {NULL}
};

static const dcTestFunctionMap sRuntimeMap[] =
{
    {"Package with Node Evaluator", &testPackageWithNodeEvaluator},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcStringManager_create();
    dcLexer_initialize();
    dcTestUtilities_go("Package Test", _argc, _argv, NULL, sMap, false);

    if (dcTestUtilities_runSystem())
    {
        dcSystem_create();
        printf("\n");
        dcTestUtilities_go("Package Runtime Test",
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
