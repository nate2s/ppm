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

#include "CompiledString.h"

#include "dcClass.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcFilePackageData.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHash.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcPackage.h"
#include "dcPackageContents.h"
#include "dcStringEvaluator.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"
#include "dcYesClass.h"

#define STRING_EVALUATOR_TEST_FILE_NAME "src/tests/dcStringEvaluatorTest.c"

static void testReturnWithNoCallStack(void)
{
    dcNode *result = dcStringEvaluator_evalString
        ("import org.taffy.core.exception.*\n"
         "success = false \n"
         "try { return } "
         "catch (ReturnWithNoCallStackException _exception) { success = true }"
         "catch (Exception _exception) { }\n"
         "success",
         STRING_EVALUATOR_TEST_FILE_NAME,
         NO_STRING_EVALUATOR_FLAGS);
    dcError_assert(dcYesClass_isMe(result));
}

static void testImportSingleClass(void)
{
    const char *singleClassFileName = "ImportSingleClass.ty";

    // don't care as long as it's not an exception
    dcTestUtilities_assert(dcStringEvaluator_evalString
                           ("import org.taffy.core.exception.Exception",
                            singleClassFileName,
                            NO_STRING_EVALUATOR_FLAGS)
                           != NULL);

    dcFilePackageData *data =
        CAST_FILE_PACKAGE_DATA
        (dcSystem_getFilePackageData
         (dcStringManager_getStringId(singleClassFileName)));

    // don't really need this locking since this is basically a single threaded
    // test, but what the hey
    dcMutex_lock(data->mutex);

    dcNode *head = dcList_getHead(data->packageContents);

    dcTestUtilities_assert(data->packageContents->size == 1
                           && IS_CLASS(head)
                           && (dcClass_hasTemplate
                               (head,
                                dcExceptionClass_getTemplate(),
                                false)));
    dcMutex_unlock(data->mutex);
}

static void testFilePackageDataExists(void)
{
    const char *fileName = "TestFilePackageDataExists.ty";

    dcTestUtilities_assert(dcStringEvaluator_evalString
                           ("import org.taffy.core.exception.Exception",
                            fileName,
                            NO_STRING_EVALUATOR_FLAGS)
                           != NULL);

    dcError_assert(dcSystem_filePackageDataExists(fileName));
    dcError_assert(! dcSystem_filePackageDataExists("sillyFileName.ty"));
}

static void testImportWild(void)
{
    const char *fileName = "ImportWild.ty";

    // don't care as long as it's not an exception
    dcTestUtilities_assert(dcStringEvaluator_evalString
                           ("import org.taffy.core.io.*",
                            fileName,
                            NO_STRING_EVALUATOR_FLAGS)
                           != NULL);

    dcFilePackageData *data =
        CAST_FILE_PACKAGE_DATA
        (dcSystem_getFilePackageData
         (dcStringManager_getStringId(fileName)));

    // don't really need this locking since this is basically a single threaded
    // test, but what the hey
    dcMutex_lock(data->mutex);

    dcNode *head = dcList_getHead(data->packageContents);

    dcTestUtilities_assert(data->packageContents->size == 1
                           && head->type == NODE_PACKAGE_CONTENTS);

    dcPackageContents *contents = CAST_PACKAGE_CONTENTS(head);
    dcPackage *expectedPackage =
        dcPackage_createFromString("org.taffy.core.io");

    dcTestUtilities_assert(dcPackage_equals(contents->package,
                                            expectedPackage));

    dcPackage_free(&expectedPackage);
    dcMutex_unlock(data->mutex);
}

static const dcTestFunctionMap sRuntimeTests[] =
{
    {"Return With No Call Stack", &testReturnWithNoCallStack},
    {"Import Single Class",       &testImportSingleClass},
    {"Import Wild",               &testImportWild},
    {"File Package Data Exists",  &testFilePackageDataExists},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_create();
    dcTestUtilities_go("String Evaluator Test",
                       _argc,
                       _argv,
                       NULL,
                       sRuntimeTests,
                       true);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
