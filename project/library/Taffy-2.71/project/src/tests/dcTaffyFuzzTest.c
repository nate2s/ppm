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

#include "dcCommandLineArguments.h"
#include "dcError.h"
#include "dcLexer.h"
#include "dcFileEvaluator.h"
#include "dcGarbageCollector.h"
#include "dcIOClass.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"
#include "dcPieLineEvaluator.h"
#include "dcNodeEvaluator.h"
#include "dcString.h"

#include "dcTaffyApplication.h"

void outputIt(const char *_output)
{
    // silencio....
}

static void testIt(uint32_t _length)
{
    dcString *string = dcString_createWithLength(_length);
    uint32_t i = 0;

    for (i = 0; i < 10000; i++)
    {
        //fprintf(stderr, "i: %u\n", i);

        uint32_t j;

        for (j = 0; j < string->length - 1; j++)
        {
            string->string[j] = rand();
        }

        FILE *testFile = fopen("taffyFuzzTest.ty", "w");
        fwrite(string->string, 1, string->length, testFile);
        fclose(testFile);

        char *argv[2] = {"dcTaffyFuzzTest.c", "taffyFuzzTest.ty"};
        dcCommandLineArguments *arguments =
            dcTaffyCommandLineArguments_parseAndCreate(2, argv);

        dcFileEvaluator *evaluator =
            dcFileEvaluator_createWithArguments(arguments);
        dcFileEvaluator_execute(evaluator);
        dcFileEvaluator_free(&evaluator);
        dcCommandLineArguments_free(&arguments);
    }

    dcString_free(&string, DC_DEEP);
}

static void test2(void)
{
    testIt(2);
}

static void test5(void)
{
    testIt(5);
}

static void test10(void)
{
    testIt(10);
}

static void test50(void)
{
    testIt(50);
}

static void test100(void)
{
    testIt(100);
}

static const dcTestFunctionMap sTests[] =
{
    {"Test 2",   test2},
    {"Test 5",   test5},
    {"Test 10",  test10},
    {"Test 50",  test50},
    {"Test 100", test100},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_create();
    srand(0);
    dcIOClass_setOutputFunction(&outputIt);
    dcTestUtilities_go("Taffy Fuzz Test", _argc, _argv, NULL, sTests, false);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
