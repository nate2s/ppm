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
#include "dcArrayClass.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHash.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcPair.h"
#include "dcParser.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcStringEvaluator.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcThreadClass.h"
#include "dcTestUtilities.h"
#include "dcUnsignedInt32.h"

#define THREAD_TEST_FILE_NAME "src/tests/dcThreadTest.c"

static dcNode **sThreads = NULL;
static size_t sThreadCount = 0;

static void threadRoot(void)
{
    size_t i;

    for (i = 0; i < sThreadCount; i++)
    {
        dcNode_mark(sThreads[i]);
    }
}

extern void bringUp(dcNodeEvaluator *_evaluator);
extern void takeDown(dcNodeEvaluator *_evaluator);

static void runThreads(const char *_program,
                       size_t _threadCount,
                       dcNode *_expectedResult)
{
    sThreadCount = _threadCount;
    sThreads = (dcNode **)(dcMemory_allocateAndInitialize
                           (sizeof(dcNode *) * sThreadCount));
    size_t i;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    char *program = dcLexer_sprintf("^{ %s }", _program);

    dcGarbageCollector_addRoot(&threadRoot);

    for (i = 0; i < sThreadCount; i++)
    {
        dcNode *body = dcParser_parseString(program,
                                            THREAD_TEST_FILE_NAME,
                                            true);
        dcTestUtilities_assert(body != NULL);
        sThreads[i] = dcNode_register(dcThreadClass_createNode(body, true));
        // set the position
        dcGraphData_setPosition
            (CAST_GRAPH_DATA(sThreads[i]),
             dcStringManager_getStringId(THREAD_TEST_FILE_NAME),
             1);
    }

    for (i = 0; i < sThreadCount; i++)
    {
        dcNode *result = dcNodeEvaluator_callMethod(evaluator,
                                                    sThreads[i],
                                                    "start");
        dcTestUtilities_assert(result == sThreads[i]);
    }

    for (i = 0; i < sThreadCount; i++)
    {
        dcNode *waitResult = dcNodeEvaluator_callMethod(evaluator,
                                                        sThreads[i],
                                                        "wait");
        dcTestUtilities_assert(waitResult == sThreads[i]);

        if (_expectedResult != NULL)
        {
            dcNode *threadResult = dcNodeEvaluator_callMethod(evaluator,
                                                              sThreads[i],
                                                              "result");
            int32_t value = 0;
            dcTestUtilities_assert(dcNumberClass_extractInt32s(threadResult,
                                                             &value));
            dcTestUtilities_assert(value == 100);
        }
    }

    dcMemory_free(program);
    dcGarbageCollector_removeRoot(&threadRoot);
    sThreadCount = 0;
    dcMemory_free(sThreads);
}

static void testThreadsWithIdentifier(void)
{
    const char *program =
        "^{ org.taffy.core.exception.UnidentifiedMethodException }";
    runThreads(program, 5, NULL);
}

static void testThreadsWithAdd(void)
{
    const char *program = "a = 1; a += 1;";
    runThreads(program, 5, NULL);
}

static void testThreadsWithExceptionGeneration(void)
{
    const char *program =
        ("[org.taffy.core.exception.UnidentifiedMethodException "
         "     methodName: \"#operator(++)\" "
         "      className: \"Nil\"]");
    runThreads(program, 2, NULL);
}

static void testThreadsWithNodeEvaluators(void)
{
    dcStringEvaluator_evalString("class Test"
                                 "{ "
                                 "    @@x, @rw; "
                                 ""
                                 "    (@@) initializeIt"
                                 "    {"
                                 "        @@x = 0\n"
                                 "    }\n"
                                 ""
                                 "    (@@) incrementIt"
                                 "    { "
                                 "        @@x++ "
                                 "    } "
                                 "}\n"
                                 "Test initializeIt",
                                 THREAD_TEST_FILE_NAME,
                                 STRING_EVALUATOR_ASSERT_NO_EXCEPTION);

    size_t iterations = 100;
    char *program = (dcLexer_sprintf
                     ("a = 0; while (a < %zu) { a++; [Test incrementIt] }",
                      iterations));
    size_t threadCount = 20;

    runThreads(program, threadCount, NULL);
    char *assertion = dcLexer_sprintf("kernel assert: ([Test x] == %zu)",
                                      threadCount * iterations);
    dcStringEvaluator_evalString(assertion,
                                 THREAD_TEST_FILE_NAME,
                                 STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
    dcMemory_free(assertion);
    dcMemory_free(program);
}

static const dcTestFunctionMap sTestMap[] =
{
    {"Threads with Identifier",           &testThreadsWithIdentifier},
    {"Threads with Add",                  &testThreadsWithAdd},
    {"Threads with Exception Generation", &testThreadsWithExceptionGeneration},
    {"Threads with Node Evaluators",      &testThreadsWithNodeEvaluators},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_createWithArguments
        (dcTaffyCommandLineArguments_parseAndCreateWithFailure(_argc,
                                                               _argv,
                                                               false));
    dcTestUtilities_go("ThreadTest", _argc, _argv, NULL, sTestMap, true);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
