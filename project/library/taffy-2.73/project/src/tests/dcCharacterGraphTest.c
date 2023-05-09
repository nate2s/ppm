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
#include "dcError.h"
#include "dcFlatArithmetic.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcParser.h"
#include "dcString.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"

static void assertEquals(dcCharacterGraph *_graph, const char *_wanted)
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

static void insertString(dcCharacterGraph *_graph,
                         const char *_string,
                         int32_t _x,
                         int32_t _y)
{
    dcString *string = dcString_createWithString(_string, true);
    dcCharacterGraph_insertString(_graph, string, _x, _y);
    dcString_free(&string, DC_DEEP);
}

static void testSimpleExpand(void)
{
    // test expand width
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcString *toInsert = dcString_createWithString("hi there", true);
        dcCharacterGraph_insertString(graph, toInsert, 0, 0);
        dcString *graphString = dcCharacterGraph_convertToString(graph);
        dcError_assert(dcString_equals(toInsert, graphString));
        dcCharacterGraph_free(&graph);
        dcString_free(&toInsert, DC_DEEP);
        dcString_free(&graphString, DC_DEEP);
    }

    // test expand height
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        insertString(graph, "hi there\nhow are you", 0, 0);
        assertEquals(graph,
                     "hi there\n"
                     "how are you");
        dcCharacterGraph_free(&graph);
    }
}

static void testAddString(void)
{
    dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
    insertString(graph, "x^", 0, 0);
    insertString(graph, "(y + z)", 2, -1);
    assertEquals(graph,
                 "  (y + z)\n"
                 "x^       ");
    dcCharacterGraph_free(&graph);
}

static void testInsertRow(void)
{
    // insert into new graph
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph_insertRow(graph, 5);
        insertString(graph, "hi there", 1, 2);
        assertEquals(graph,
                     "         \n"
                     "         \n"
                     " hi there\n"
                     "         \n"
                     "         \n"
                     "         ");
        dcCharacterGraph_free(&graph);
    }

    // insert into graph that's already created
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(2, 2);
        insertString(graph, "THE TOP", 0, 0);
        insertString(graph, "x", 0, 1);
        dcCharacterGraph_insertRow(graph, 1);
        insertString(graph, "(x + y)", 1, 1);
        assertEquals(graph,
                     "THE TOP \n"
                     " (x + y)\n"
                     "x       ");
        dcCharacterGraph_free(&graph);
    }

    // insert past beginning
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        insertString(graph, "THE BOTTOM", 0, 0);
        dcCharacterGraph_insertRow(graph, -1);
        insertString(graph, "THE TOP", 0, 0);
        assertEquals(graph,
                     "THE TOP   \n"
                     "THE BOTTOM");
        dcCharacterGraph_free(&graph);
    }
}

static void testInsertCharacterGraph(void)
{
    dcCharacterGraph *graphA = dcCharacterGraph_create(1, 5);
    dcCharacterGraph_insertCharacterString(graphA, "f(x", 0, 0);
    assertEquals(graphA, "f(x  ");
    dcCharacterGraph *graphB = dcCharacterGraph_create(1, 1);
    dcCharacterGraph_insertCharacterString(graphB, "2", 0, 0);
    assertEquals(graphB, "2");
    dcCharacterGraph_insertCharacterGraphUp(graphA, 3, -1, graphB);

    assertEquals(graphA,
                 "   2 \n"
                 "f(x  ");
    dcCharacterGraph_free(&graphA);
    dcCharacterGraph_free(&graphB);
}

static void testInsertCharacterGraphUp(void)
{
    // single line
    {
        dcCharacterGraph *graphA =
            dcCharacterGraph_createFromCharString
            ("x    \n"
             "y    ");

        dcCharacterGraph *graphB = dcCharacterGraph_createFromCharString("e");

        dcCharacterGraph_insertCharacterGraphUp(graphA, 1, 1, graphB);
        assertEquals(graphA,
                     "x    \n"
                     "ye   ");

        dcCharacterGraph_free(&graphA);
        dcCharacterGraph_free(&graphB);
    }

    // double line
    {
        dcCharacterGraph *graphA =
            dcCharacterGraph_createFromCharString
            ("x    \n"
             "y    ");

        dcCharacterGraph *graphB =
            dcCharacterGraph_createFromCharString
            ("e\n"
             "f");

        dcCharacterGraph_insertCharacterGraphUp(graphA, 1, 1, graphB);
        assertEquals(graphA,
                     "xe   \n"
                     "yf   ");

        dcCharacterGraph_free(&graphA);
        dcCharacterGraph_free(&graphB);
    }

    // three lines
    {
        dcCharacterGraph *graphA =
            dcCharacterGraph_createFromCharString
            ("x    \n"
             "y    ");

        dcCharacterGraph *graphB =
            dcCharacterGraph_createFromCharString
            ("g\n"
             "e\n"
             "f");

        dcCharacterGraph_insertCharacterGraphUp(graphA, 1, 1, graphB);
        assertEquals(graphA,
                     " g   \n"
                     "xe   \n"
                     "yf   ");

        dcCharacterGraph_free(&graphA);
        dcCharacterGraph_free(&graphB);
    }


    // three lines from 0th line
    {
        dcCharacterGraph *graphA =
            dcCharacterGraph_createFromCharString
            ("x    \n"
             "y    ");

        dcCharacterGraph *graphB =
            dcCharacterGraph_createFromCharString
            ("g\n"
             "e\n"
             "f");

        dcCharacterGraph_insertCharacterGraphUp(graphA, 1, 0, graphB);
        assertEquals(graphA,
                     " g   \n"
                     " e   \n"
                     "xf   \n"
                     "y    ");

        dcCharacterGraph_free(&graphA);
        dcCharacterGraph_free(&graphB);
    }

    // three lines from -1th line
    {
        dcCharacterGraph *graphA =
            dcCharacterGraph_createFromCharString
            ("x    \n"
             "y    ");

        dcCharacterGraph *graphB =
            dcCharacterGraph_createFromCharString
            ("g\n"
             "e\n"
             "f");

        dcCharacterGraph_insertCharacterGraphUp(graphA, 1, -1, graphB);
        assertEquals(graphA,
                     " g   \n"
                     " e   \n"
                     " f   \n"
                     "x    \n"
                     "y    ");

        dcCharacterGraph_free(&graphA);
        dcCharacterGraph_free(&graphB);
    }
}

static void testInsertCharacterGraphDown(void)
{
    // one line at a time
    {
        dcCharacterGraph *graphA = dcCharacterGraph_create(1, 5);
        dcCharacterGraph_insertCharacterString(graphA, "x", 0, 0);
        assertEquals(graphA, "x    ");

        dcCharacterGraph *graphB = dcCharacterGraph_create(1, 1);
        dcCharacterGraph_insertCharacterString(graphB, "-----", 0, 0);
        assertEquals(graphB, "-----");

        dcCharacterGraph *graphC = dcCharacterGraph_create(1, 1);
        dcCharacterGraph_insertCharacterString(graphC, "2", 0, 0);
        assertEquals(graphC, "2");

        dcCharacterGraph_insertCharacterGraphDown(graphA, 0, 1, graphB);
        dcCharacterGraph_insertCharacterGraphDown(graphA, 0, 2, graphC);

        assertEquals(graphA,
                     "x    \n"
                     "-----\n"
                     "2    ");

        dcCharacterGraph_free(&graphA);
        dcCharacterGraph_free(&graphB);
        dcCharacterGraph_free(&graphC);
    }

    // two lines inserted at 0
    {
        dcCharacterGraph *graphA =
            dcCharacterGraph_createFromCharString("x   ");
        dcCharacterGraph *graphB =
            dcCharacterGraph_createFromCharString("y\n"
                                                  "z");

        dcCharacterGraph_insertCharacterGraphDown(graphA, 1, 0, graphB);

        assertEquals(graphA,
                     "xy  \n"
                     " z  ");

        dcCharacterGraph_free(&graphA);
        dcCharacterGraph_free(&graphB);
    }

    // two lines inserted at -1
    {
        dcCharacterGraph *graphA =
            dcCharacterGraph_createFromCharString("x   ");
        dcCharacterGraph *graphB =
            dcCharacterGraph_createFromCharString("y\n"
                                                  "z");

        dcCharacterGraph_insertCharacterGraphDown(graphA, 1, -1, graphB);

        assertEquals(graphA,
                     " y  \n"
                     "xz  ");

        dcCharacterGraph_free(&graphA);
        dcCharacterGraph_free(&graphB);
    }

    // three lines inserted at -1
    {
        dcCharacterGraph *graphA =
            dcCharacterGraph_createFromCharString("x   ");
        dcCharacterGraph *graphB =
            dcCharacterGraph_createFromCharString("y\n"
                                                  "z\n"
                                                  "w");

        dcCharacterGraph_insertCharacterGraphDown(graphA, 1, -1, graphB);

        assertEquals(graphA,
                     " y  \n"
                     "xz  \n"
                     " w  ");

        dcCharacterGraph_free(&graphA);
        dcCharacterGraph_free(&graphB);
    }
}

static void testInsertMultiLineCharacterGraph(void)
{
    dcCharacterGraph *graphA = dcCharacterGraph_create(1, 5);
    dcCharacterGraph_insertCharacterString(graphA, "f(x", 0, 0);
    assertEquals(graphA, "f(x  ");
    dcCharacterGraph *graphB = dcCharacterGraph_create(1, 1);
    dcCharacterGraph_insertCharacterString(graphB, "e", 0, 0);
    dcCharacterGraph_insertCharacterString(graphB, "2", 1, -1);
    assertEquals(graphB,
                 " 2\n"
                 "e ");
    dcCharacterGraph_insertCharacterGraphUp(graphA, 3, -1, graphB);
    assertEquals(graphA,
                 "    2\n"
                 "   e \n"
                 "f(x  ");
    dcCharacterGraph_free(&graphA);
    dcCharacterGraph_free(&graphB);
}

static dcNode *createFlatArithmetic(const char *_program)
{
    if (_program == NULL)
    {
        return NULL;
    }

    dcNode *head = dcParser_parseString(_program,
                                        "dcCharacterGraphTest.c",
                                        true);
    dcError_assert(head != NULL);
    dcNode *contents = dcGraphDataTree_getContents(head);
    dcNode *result = contents;
    dcNode_free(&head, DC_SHALLOW);
    return result;
}

static void testIt(const char *_program,
                   const char *_wantedCharacterGraph)
{
    dcNode *arithmetic = createFlatArithmetic(_program);
    dcCharacterGraph *graphResult = NULL;

    assert(dcNode_prettyPrint(arithmetic, &graphResult)
           == TAFFY_SUCCESS);

    assertEquals(graphResult, _wantedCharacterGraph);

    dcNode_free(&arithmetic, DC_DEEP);
    dcCharacterGraph_free(&graphResult);
}

static void bracesTestIt(const char *_graph,
                         const char *_wantedCharacterGraph)
{

    dcCharacterGraph *graph = dcCharacterGraph_createFromCharString(_graph);
    dcCharacterGraph_addBraces(graph);
    assertEquals(graph, _wantedCharacterGraph);
    dcCharacterGraph_free(&graph);
}

static void testFlatArithmeticAdd(void)
{
    // simple add
    testIt("1 + x + 3",
           "1 + x + 3");

    // add with multiply
    testIt("1 + x + (y * 3)",
           "1 + x + y * 3");

    // add with multi-line divide, gets parens
    testIt("1 + 3 / x / y",
           "    /   3   \\\n"
           "    |  ---  |\n"
           "1 + |   x   |\n"
           "    | ----- |\n"
           "    \\   y   /");
}

static void testFlatArithmeticRaise(void)
{
    // simple raise
    testIt("e^x",
           " x\n"
           "e ");

    // raise with divide
    testIt("e^(x / y)",
           " /  x  \\\n"
           " | --- |\n"
           " \\  y  /\n"
           "e       ");

    // raise with add
    testIt("e^(3 + x)",
           " (3 + x)\n"
           "e       ");

    // three raise
    testIt("e^x^y",
           "  y\n"
           " x \n"
           "e  ");
}

static void testFlatArithmeticDivide(void)
{
    // simple divide
    testIt("1 / x",
           " 1 \n"
           "---\n"
           " x ");

    // three divide
    testIt("1 / x / (y + 2)",
           "   1   \n"
           "  ---  \n"
           "   x   \n"
           "-------\n"
           " y + 2 ");

    // add and divide
    testIt("1 + (x / y)",
           "     x \n"
           "1 + ---\n"
           "     y ");

    // add and bigger divide
    testIt("1 + ((x * 2) / y)",
           "     x * 2 \n"
           "1 + -------\n"
           "       y   ");

    // bigger on top
    testIt("(x + y + z) / 2",
           " x + y + z \n"
           "-----------\n"
           "     2     ");
}

static void testAddParens(void)
{
    // one line
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph_insertCharacterString(graph, "x", 0, 0);
        dcCharacterGraph_addParens(graph);
        assertEquals(graph, "(x)");
        dcCharacterGraph_free(&graph);
    }

    // two lines
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph_insertCharacterString(graph, "x", 0, 0);
        dcCharacterGraph_insertCharacterString(graph, "y", 0, 1);
        dcCharacterGraph_addParens(graph);
        assertEquals(graph,
                     ("/ x \\\n"
                      "\\ y /"));
        dcCharacterGraph_free(&graph);
    }

    // three lines
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph_insertCharacterString(graph, "x", 0, 0);
        dcCharacterGraph_insertCharacterString(graph, "y", 0, 1);
        dcCharacterGraph_insertCharacterString(graph, "z", 2, 2);
        dcCharacterGraph_addParens(graph);
        assertEquals(graph,
                     ("/ x   \\\n"
                      "| y   |\n"
                      "\\   z /"));
        dcCharacterGraph_free(&graph);
    }
}

static void testAddBraces(void)
{
    // one line
    bracesTestIt("x", "[x]");

    // two lines
    bracesTestIt(" x \n"
                 "---\n"
                 " y ",
                 "+- x -+\n"
                 "| --- |\n"
                 "+- y -+");

    // three lines
    bracesTestIt("x \n"
                 "y \n"
                 " z",
                 "+-x -+\n"
                 "| y  |\n"
                 "+- z-+");
}

static void testPrependRows(void)
{
    // 1
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        insertString(graph, "x", 0, 0);
        dcCharacterGraph_prependRows(graph, 1);
        assertEquals(graph,
                     " \n"
                     "x");
        dcCharacterGraph_free(&graph);
    }

    // 2
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        insertString(graph, "x", 0, 0);
        dcCharacterGraph_prependRows(graph, 2);
        assertEquals(graph,
                     " \n"
                     " \n"
                     "x");
        dcCharacterGraph_free(&graph);
    }
}

static void testPrependColumns(void)
{
    // 1
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        insertString(graph, "x", 0, 0);
        dcCharacterGraph_prependColumns(graph, 1);
        assertEquals(graph, " x");
        dcCharacterGraph_free(&graph);
    }
}

static void testAppendCharacterGraph(void)
{
    dcCharacterGraph *top = dcCharacterGraph_create(1, 1);
    insertString(top, "x", 0, 0);

    dcCharacterGraph *lines = dcCharacterGraph_create(1, 1);
    insertString(lines, "------", 0, 0);

    dcCharacterGraph *bottom = dcCharacterGraph_create(1, 1);
    insertString(bottom, "y + 2", 0, 0);

    dcCharacterGraph *twoLinesBottom = dcCharacterGraph_create(1, 1);
    insertString(twoLinesBottom, "e", 0, 0);
    insertString(twoLinesBottom, "2", 1, -1);
    assertEquals(twoLinesBottom,
                 " 2\n"
                 "e ");

    dcCharacterGraph_appendCharacterGraph(top, lines);
    assertEquals(top,
                 "x     \n"
                 "------");

    dcCharacterGraph_appendCharacterGraph(top, bottom);
    assertEquals(top,
                 "x     \n"
                 "------\n"
                 "y + 2 ");

    dcCharacterGraph_appendCharacterGraph(top, lines);
    assertEquals(top,
                 "x     \n"
                 "------\n"
                 "y + 2 \n"
                 "------");

    dcCharacterGraph_appendCharacterGraph(top, twoLinesBottom);
    assertEquals(top,
                 "x     \n"
                 "------\n"
                 "y + 2 \n"
                 "------\n"
                 " 2    \n"
                 "e     ");

    dcCharacterGraph_free(&top);
    dcCharacterGraph_free(&bottom);
    dcCharacterGraph_free(&lines);
    dcCharacterGraph_free(&twoLinesBottom);
}

static dcCharacterGraph *createGraph(const char *_string)
{
    return dcCharacterGraph_createFromCharString(_string);
}

static void testAppendMiddle(void)
{
    // single line
    {
        dcCharacterGraph *graph = createGraph("x");
        dcCharacterGraph *right = createGraph("y");
        dcCharacterGraph_appendMiddle(graph, right);
        assertEquals(graph, "y");
        dcCharacterGraph_free(&graph);
        dcCharacterGraph_free(&right);
    }

    // append double line
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph *left = createGraph("x");
        dcCharacterGraph *right = createGraph("y\n"
                                              "z");
        dcCharacterGraph_appendMiddle(graph, left);
        dcCharacterGraph_appendMiddle(graph, right);
        assertEquals(graph,
                     " y\n"
                     "xz");
        dcCharacterGraph_free(&graph);
        dcCharacterGraph_free(&left);
        dcCharacterGraph_free(&right);
    }

    // append double line then triple
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph *left = createGraph("x");
        dcCharacterGraph *right = createGraph("y\n"
                                              "z");
        dcCharacterGraph *righty = createGraph("y\nz\nw");
        dcCharacterGraph_appendMiddle(graph, left);
        dcCharacterGraph_appendMiddle(graph, right);
        dcCharacterGraph_appendMiddle(graph, righty);
        assertEquals(graph,
                     " yy\n"
                     "xzz\n"
                     "  w");
        dcCharacterGraph_free(&graph);
        dcCharacterGraph_free(&left);
        dcCharacterGraph_free(&right);
        dcCharacterGraph_free(&righty);
    }

    // double line + single
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph *left = createGraph("x\ny");
        dcCharacterGraph *right = createGraph("z");
        dcCharacterGraph_appendMiddle(graph, left);
        dcCharacterGraph_appendMiddle(graph, right);
        assertEquals(graph,
                     "x \n"
                     "yz");
        dcCharacterGraph_free(&graph);
        dcCharacterGraph_free(&left);
        dcCharacterGraph_free(&right);
    }

    // double line + triple
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph *left = createGraph("x\ny");
        dcCharacterGraph *right = createGraph("x\ny\nz");
        dcCharacterGraph_appendMiddle(graph, left);
        dcCharacterGraph_appendMiddle(graph, right);
        assertEquals(graph,
                     "xx\n"
                     "yy\n"
                     " z");
        dcCharacterGraph_free(&graph);
        dcCharacterGraph_free(&left);
        dcCharacterGraph_free(&right);
    }

    // double line + triple + single
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph *left = createGraph("x\ny");
        dcCharacterGraph *right = createGraph("x\ny\nz");
        dcCharacterGraph *righty = createGraph("11");
        dcCharacterGraph_appendMiddle(graph, left);
        dcCharacterGraph_appendMiddle(graph, right);
        dcCharacterGraph_appendMiddle(graph, righty);
        assertEquals(graph,
                     "xx  \n"
                     "yy11\n"
                     " z  ");
        dcCharacterGraph_free(&left);
        dcCharacterGraph_free(&graph);
        dcCharacterGraph_free(&right);
        dcCharacterGraph_free(&righty);
    }

    // append triple line then single
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph *left = createGraph("x");
        dcCharacterGraph *right = createGraph("y\n"
                                              "z\n"
                                              "w");
        dcCharacterGraph *righty = createGraph("11");
        dcCharacterGraph_appendMiddle(graph, left);
        dcCharacterGraph_appendMiddle(graph, right);
        dcCharacterGraph_appendMiddle(graph, righty);
        assertEquals(graph,
                     " y  \n"
                     "xz11\n"
                     " w  ");
        dcCharacterGraph_free(&graph);
        dcCharacterGraph_free(&left);
        dcCharacterGraph_free(&right);
        dcCharacterGraph_free(&righty);
    }

    // append triple line then single
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        dcCharacterGraph *left = createGraph("x");
        dcCharacterGraph *right = createGraph("y\n"
                                              "z\n"
                                              "w");
        dcCharacterGraph *righty = createGraph("11");
        dcCharacterGraph_appendMiddle(graph, left);
        dcCharacterGraph_appendMiddle(graph, right);
        dcCharacterGraph_appendMiddle(graph, righty);
        assertEquals(graph,
                     " y  \n"
                     "xz11\n"
                     " w  ");
        dcCharacterGraph_free(&graph);
        dcCharacterGraph_free(&left);
        dcCharacterGraph_free(&right);
        dcCharacterGraph_free(&righty);
    }
}

static void testAddRows(void)
{
    // 1
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        insertString(graph, "x", 0, 0);
        dcCharacterGraph_addRows(graph, 1);
        assertEquals(graph,
                     "x\n"
                     " ");
        dcCharacterGraph_free(&graph);
    }

    // 2
    {
        dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
        insertString(graph, "x", 0, 0);
        dcCharacterGraph_addRows(graph, 2);
        assertEquals(graph,
                     "x\n"
                     " \n"
                     " ");
        dcCharacterGraph_free(&graph);
    }
}

static const dcTestFunctionMap sMap[] =
{
    {"Prepend Rows",           &testPrependRows},
    {"Prepend Columns",        &testPrependColumns},
    {"Add Rows",               &testAddRows},
    {"Simple Expand",          &testSimpleExpand},
    {"Insert Row",             &testInsertRow},
    {"Add String",             &testAddString},
    {"Append Character Graph", &testAppendCharacterGraph},
    {"Append Middle",          &testAppendMiddle},
    {"Insert Character Graph", &testInsertCharacterGraph},
    {"Insert Character Graph Up", &testInsertCharacterGraphUp},
    {"Insert Character Graph Down", &testInsertCharacterGraphDown},
    {"Insert Multi-Line Character Graph", &testInsertMultiLineCharacterGraph},
    {"Add Parens",             &testAddParens},
    {"Add Braces",             &testAddBraces},
    {NULL}
};

static const dcTestFunctionMap sRuntimeMap[] =
{
    {"Flat Arithmetic Raise",  &testFlatArithmeticRaise},
    {"Flat Arithmetic Divide", &testFlatArithmeticDivide},
    {"Flat Arithmetic Add",    &testFlatArithmeticAdd},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcStringManager_create();
    dcTestUtilities_go("Character Graph Test", _argc, _argv, NULL, sMap, false);

    if (dcTestUtilities_runSystem())
    {
        dcSystem_createWithArguments
            (dcTaffyCommandLineArguments_parseAndCreateWithFailure
             (_argc,
              _argv,
              false));
        dcTestUtilities_go("Character Graph Runtime Test",
                           _argc,
                           _argv,
                           NULL,
                           sRuntimeMap,
                           true);
        dcSystem_free();
    }

    dcGarbageCollector_free();
    dcStringManager_free();
    dcMemory_deinitialize();
    return 0;
}
