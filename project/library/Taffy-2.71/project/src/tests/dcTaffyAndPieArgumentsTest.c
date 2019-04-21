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

#include "CompiledPieUsage.h"
#include "CompiledTaffyUsage.h"

#include "dcError.h"
#include "dcGarbageCollector.h"
#include "dcIOClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcTestUtilities.h"
#include "dcTaffyApplication.h"
#include "dcPieApplication.h"
#include "dcNodeEvaluator.h"
#include "dcString.h"

static const char *sExpectedOutput = NULL;
static bool sGotOutput = false;

static char **createArguments(const char *_text, int *_argc)
{
    dcList *arguments = dcLexer_splitString(_text, ' ');
    *_argc = (int)arguments->size;
    char **result = (char **)dcMemory_allocate(sizeof(char *) * *_argc);
    int i;
    dcListElement *that;

    for (that = arguments->head, i = 0;
         i < *_argc;
         that = that->next, i++)
    {
        assert(that != NULL);
        result[i] = dcMemory_strdup(dcString_getString(that->object));
    }

    dcList_free(&arguments, DC_DEEP);
    return result;
}

static void assertPrintHelp(const char *_what)
{
    if (strcmp(sExpectedOutput, _what) != 0)
    {
        fprintf(stderr,
                "Excepected: '%s'\n"
                "But got:    '%s'\n",
                sExpectedOutput,
                _what);
        assert(false);
    }

    sGotOutput = true;
}

typedef int (*ApplicationGoFunction)(int argc, char **argv);

static void testInvalidCliArgumentProgram(const char *_arguments,
                                          ApplicationGoFunction _function,
                                          const char *_expectedOutput)
{
    int argc;
    sGotOutput = false;
    sExpectedOutput = _expectedOutput;
    dcIOClass_setOutputFunction(&assertPrintHelp);
    char **argv = createArguments(_arguments, &argc);
    assert(_function(argc, argv) == 1);
    int i;

    for (i = 0; i < argc; i++)
    {
        dcMemory_free(argv[i]);
    }

    assert(sGotOutput);
    dcMemory_free(argv);
    dcIOClass_resetOutputFunction();
}

void testInvalidCliArgumentsPrograms(const char **_arguments,
                                     ApplicationGoFunction _function,
                                     const char *_expectedOutput)
{
    int a;

    for (a = 0; _arguments[a] != NULL; a++)
    {
        testInvalidCliArgumentProgram(_arguments[a],
                                      _function,
                                      _expectedOutput);
    }
}

static void testInvalidCliArguments(void)
{
    const char *arguments[] =
    {
        "appName -",
        "appName --",
        "appName -asdf-",
        "appName -0-",
        "appName --0--",
        "appName ----------",
        "appName --logy \"hi there\"",
        "appName --00asdf \"hi there\"",
        "appName --asdf-hi",
        "appName -asdf--hi",
        NULL
    };

    testInvalidCliArgumentsPrograms(arguments,
                                    &dcTaffyApplication_go,
                                    __compiledTaffyUsage);
    testInvalidCliArgumentsPrograms(arguments,
                                    &dcPieApplication_go,
                                    __compiledPieUsage);
}

// logs are only valid in debug builds
#ifdef ENABLE_DEBUG
static void testInvalidLogTypeWithExpectedResult(const char *_arguments,
                                     const char *_expected)
{
    testInvalidCliArgumentProgram(_arguments,
                                  &dcTaffyApplication_go,
                                  _expected);
    testInvalidCliArgumentProgram(_arguments,
                                  &dcPieApplication_go,
                                  _expected);
}

static void testInvalidLogType(void)
{
    testInvalidLogTypeWithExpectedResult("appName asdf --log \"a\"",
                                         "Invalid log type: \"a\"\n");
    testInvalidLogTypeWithExpectedResult("appName asdf --log \"a-asdf\"",
                                         "Invalid log type: \"a-asdf\"\n");
}
#endif // ENABLE_DEBUG

static const dcTestFunctionMap sTests[] =
{
    {"Invalid CLI arguments", &testInvalidCliArguments},
#ifdef ENABLE_DEBUG
    {"Invalid Log Type",      &testInvalidLogType},
#endif // ENABLE_DEBUG
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcGarbageCollector_create();
    dcTestUtilities_go("Taffy and Pie Command Line Arguments Test",
                       _argc,
                       _argv,
                       NULL,
                       sTests,
                       false);
    dcGarbageCollector_free();
    return 0;
}
