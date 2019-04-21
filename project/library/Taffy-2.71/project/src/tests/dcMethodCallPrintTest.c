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

#include "dcCharacterGraph.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcParser.h"
#include "dcString.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"

static void assertGraphEquals(dcCharacterGraph *_graph, const char *_wanted)
{
    dcString *graphString = dcCharacterGraph_convertToString(_graph);

    if (strcmp(graphString->string, _wanted) != 0)
    {
        fprintf(stderr,
                "assertion failure.\n"
                "wanted:\n"
                "[\n%s]\n"
                "but got:\n"
                "[\n%s]\n",
                _wanted,
                graphString->string);
        assert(false);
    }

    dcString_free(&graphString, DC_DEEP);
}

static void assertEqualsWithExpected(const char *_program,
                                     const char *_expected)
{
    dcNode *program = dcParser_parseString(_program,
                                           "src/tests/dcMethodCallPrintTest.c",
                                           false);
    char *display = NULL;

    if (program == NULL)
    {
        fprintf(stderr,
                "\n\nError: program: %s makes for NULL time\n\n",
                _program);
        assert(false);
    }

    display = dcNode_synchronizedDisplay(program);

    if (display == NULL
        || strcmp(display, _expected) != 0)
    {
        fprintf(stderr, "\n\nError: For program: %s\n", _program);
        fprintf(stderr,
                "assertion failure.\n"
                "wanted:"
                "\n%s\n"
                "but got:"
                "\n%s\n\n",
                _expected,
                (display == NULL
                 ? "NULL??"
                 : display));
        assert(false);
    }

    dcNode_free(&program, DC_DEEP);
    dcMemory_free(display);
}

static void assertEquals(const char *_program)
{
    assertEqualsWithExpected(_program, _program);
}

static void testOperators(void)
{
    assertEquals("a + 1");
    assertEquals("a - 1");
    assertEquals("a * 1");
    assertEquals("a / 1");
    assertEquals("a^1");
    assertEquals("a % 1");
    assertEquals("a < 1");
    assertEquals("a <= 1");
    assertEquals("a > 1");
    assertEquals("a >= 1");
    assertEquals("a << 1");
    assertEquals("a <<= 1");
    assertEquals("a >> 1");
    assertEquals("a > 1");
    assertEquals("a == 1");
    assertEquals("a++");
    assertEquals("a--");
    assertEquals("a & 1");
    assertEquals("a &= 1");
    assertEquals("a | 1");
    assertEquals("a |= 1");
    assertEquals("a ^^ 1");
    assertEquals("a ^^= 1");
    assertEqualsWithExpected("~a", "~ (a)");
    assertEqualsWithExpected("~ (a)", "~ (a)");
    assertEqualsWithExpected("a ~= 1", "a ~=<6> 1");
    assertEquals("a ~=<6> 1");
    assertEquals("a ~=<5> 1");
    assertEquals("a += 1");
    assertEquals("a -= 1");
    assertEquals("a *= 1");
    assertEquals("a /= 1");
    assertEquals("a ^= 1");
    assertEquals("a %= 1");
    assertEquals("f(x)");
    assertEquals("f(x, y, z)");
    assertEquals("array[x]");
    assertEquals("array[1] = 1");
    assertEquals("array[x, y, z]");
    assertEquals("array[1, 2, 3] = 2");
    assertEquals("-a");
    assertEquals("+a");
    assertEquals("!(a)");
    assertEquals("(a)!");
}

static void testNodeIt(const char *_program,
                       const char *_wantedCharacterGraph)
{
    dcNode *node = dcParser_parseString(_program,
                                        "dcCharacterGraphTest.c",
                                        true);
    if (node == NULL)
    {
        fprintf(stderr, "Program failed: %s\n", _program);
        assert(false);
    }

    dcCharacterGraph *graphResult = NULL;

    assert(dcNode_prettyPrint(node, &graphResult)
           == TAFFY_SUCCESS);

    assertGraphEquals(graphResult, _wantedCharacterGraph);

    dcNode_free(&node, DC_DEEP);
    dcCharacterGraph_free(&graphResult);
}

static void testIndexedMethodCallPrettyPrint(void)
{
    testNodeIt("f(x)", "f(x)");

    testNodeIt("f(x/2)",
               " /  x  \\\n"
               "f| --- |\n"
               " \\  2  /");

    testNodeIt("f(x, y / 2 / 3)",
               " /    /   y   \\ \\\n"
               " |    |  ---  | |\n"
               "f| x, |   2   | |\n"
               " |    | ----- | |\n"
               " \\    \\   3   / /");

    testNodeIt("f[x]", "f[x]");

    testNodeIt("f[x/2]",
               " +- x -+\n"
               "f| --- |\n"
               " +- 2 -+");

    testNodeIt("f[x, y/2]",
               " +-    y -+\n"
               "f| x, --- |\n"
               " +-    2 -+");

    testNodeIt("f[x, y/2/3, z]",
               " +-   /   y   \\   -+\n"
               " |    |  ---  |    |\n"
               "f| x, |   2   |, z |\n"
               " |    | ----- |    |\n"
               " +-   \\   3   /   -+");

    testNodeIt("x << e^x^y^2",
               "     /    2 \\\n"
               "     |   y  |\n"
               "x << |  x   |\n"
               "     \\ e    /");

    testNodeIt("~ e^x^y^2",
               "  /    2 \\\n"
               "  |   y  |\n"
               "~ |  x   |\n"
               "  \\ e    /");

    testNodeIt("! e^x^y^2",
               " /    2 \\\n"
               " |   y  |\n"
               "!|  x   |\n"
               " \\ e    /");

    testNodeIt("x ~=<5> e^x^y^2",
               "        /    2 \\\n"
               "        |   y  |\n"
               "x ~=<5> |  x   |\n"
               "        \\ e    /");

    testNodeIt("(e^x^y^2)!",
               "/    2 \\ \n"
               "|   y  | \n"
               "|  x   |!\n"
               "\\ e    / ");
}

static const dcTestFunctionMap sMap[] =
{
    {"Test Operators",     &testOperators},
    {"Indexed MethodCall Pretty Print",
     &testIndexedMethodCallPrettyPrint},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_create();
    dcTestUtilities_go("Method Call Print Test",
                       _argc,
                       _argv,
                       NULL,
                       sMap,
                       true);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
