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

static void testIt(uint32_t _argcLength)
{
    char **argv = (char **)dcMemory_allocate(sizeof(char *) * _argcLength);
    uint32_t i;

    // run the test 1000 times
    for (i = 0; i < 1000; i++)
    {
        uint32_t j;

        for (j = 0; j < _argcLength; j++)
        {
            // how long is each argument? it's random
            uint32_t limit = rand() % 1000 + 1;
            uint32_t k = 0;

            argv[j] = (char *)dcMemory_allocate(sizeof(char) * limit);

            for (k = 0; k < limit - 1; k++)
            {
                argv[j][k] = rand();
            }

            // null-terminate it
            argv[j][k] = 0;
        }

        dcCommandLineArguments *arguments = dcCommandLineArguments_create();
        // randomize the last argument, which is a boolean
        dcCommandLineArguments_parse(arguments, _argcLength, argv, rand() % 2);
        dcCommandLineArguments_free(&arguments);

        // free up the allocated argvs, but not the array, yet
        for (j = 0; j < _argcLength; j++)
        {
            dcMemory_free(argv[j]);
        }
    }

    // free up the allocated argvs
    for (i = 0; i < _argcLength; i++)
    {
        dcMemory_free(argv[i]);
    }

    dcMemory_free(argv);
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
    srand(0);
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcTestUtilities_go("Command Line Arguments Fuzz Test",
                       _argc,
                       _argv,
                       NULL,
                       sTests,
                       false);
    dcGarbageCollector_free();
    dcMemory_deinitialize();
    return 0;
}
