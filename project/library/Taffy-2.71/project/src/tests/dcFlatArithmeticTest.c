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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "dcArray.h"
#include "dcCharacterGraph.h"
#include "dcComplexNumber.h"
#include "dcComplexNumberClass.h"
#include "dcDefines.h"
#include "dcError.h"
#include "dcFlatArithmetic.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcHash.h"
#include "dcIdentifier.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcLog.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcMemory.h"
#include "dcParser.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"
#include "dcUnsignedInt32.h"
#include "dcYesClass.h"

#define FLAT_ARITHMETIC_TEST_FILE_NAME "src/tests/dcFlatArithmeticTest.c"

#define BOOL_TO_STRING(_value) (_value ? "yes" : "no")

typedef struct
{
    const char *program;
    const char *expectedResult;
    bool expectedModified;
} SingleTest;

static dcNode *createFlatArithmetic(const char *_program)
{
    if (_program == NULL)
    {
        return NULL;
    }

    dcNode *head = dcParser_parseString(_program,
                                        "dcFlatArithmetic.c",
                                        true);
    dcError_assert(head != NULL);
    dcNode *contents = dcGraphDataTree_getContents(head);
    dcNode *result = contents;
    dcNode_free(&head, DC_SHALLOW);
    return result;
}

static void assertContentsVaList(const dcNode *_node,
                                 dcTaffyOperator _operator,
                                 size_t _arithmeticCount,
                                 const char *_head,
                                 va_list _arguments)
{
    dcTestUtilities_assert(_node != NULL);
    const dcFlatArithmetic *flattery = CAST_FLAT_ARITHMETIC(_node);
    dcTestUtilities_assert(flattery->taffyOperator == _operator
                           && flattery->values->size == _arithmeticCount);
    dcListElement *that;
    const char *thatArgument = _head;
    uint32_t count = 0;

    for (that = flattery->values->head, thatArgument = _head;
         that != NULL && thatArgument != NULL;
         that = that->next, thatArgument = va_arg(_arguments, const char *))
     {
         count++;

         if (strcmp(thatArgument, "?") != 0)
         {
             dcNode *tree = dcParser_parseString(thatArgument,
                                                 FLAT_ARITHMETIC_TEST_FILE_NAME,
                                                 true);
             dcTestUtilities_assert
                 (thatArgument != NULL
                  && tree != NULL
                  && (dcNode_easyCompare(that->object,
                                         dcGraphDataTree_getContents(tree))
                      == TAFFY_EQUALS));
            dcNode_free(&tree, DC_DEEP);
        }
    }

    dcTestUtilities_assert(count == flattery->values->size);
}

static void assertCombineInto(const char *_program,
                              dcTaffyOperator _operator,
                              size_t _arithmeticCount,
                              const char *_head,
                              ...)
{
    dcNode *flatty = (dcFlatArithmetic_combine
                      (createFlatArithmetic(_program), NULL));
    va_list arguments;
    va_start(arguments, _head);
    assertContentsVaList(flatty, _operator, _arithmeticCount, _head, arguments);
    dcNode_free(&flatty, DC_DEEP);
}

static void assertContentsAndFree(const char *_program,
                                  dcTaffyOperator _operator,
                                  size_t _arithmeticCount,
                                  const char *_head,
                                  ...)
{
    dcNode *flatty = createFlatArithmetic(_program);
    va_list arguments;
    va_start(arguments, _head);
    assertContentsVaList(flatty, _operator, _arithmeticCount, _head, arguments);
    dcNode_free(&flatty, DC_DEEP);
}

static void assertContents(const dcNode *_node,
                           dcTaffyOperator _operator,
                           size_t _arithmeticCount,
                           const char *_head,
                           ...)
{
    va_list arguments;
    va_start(arguments, _head);
    assertContentsVaList(_node, _operator, _arithmeticCount, _head, arguments);
}

static void assertNodeContents(const dcNode *_node,
                               dcTaffyOperator _operator,
                               size_t _arithmeticCount,
                               dcNode *_head,
                               ...)
{
    dcTestUtilities_assert(_node != NULL);
    const dcFlatArithmetic *flattery = CAST_FLAT_ARITHMETIC(_node);
    dcTestUtilities_assert(flattery->taffyOperator == _operator
                           && flattery->values->size == _arithmeticCount);
    va_list arguments;
    va_start(arguments, _head);
    dcListElement *that;
    dcNode *thatArgument = _head;

    for (that = flattery->values->head, thatArgument = _head;
         that != NULL && thatArgument != NULL;
         that = that->next, thatArgument = va_arg(arguments, dcNode *))
     {
         dcTestUtilities_assert
             (dcNode_easyCompare(that->object, thatArgument) == TAFFY_EQUALS);
         dcNode_free(&thatArgument, DC_DEEP);
    }
}

static void testSingleNode(void)
{
    dcNode *flatty = createFlatArithmetic("1 + 2");
    assertContents(flatty, TAFFY_ADD, 2, "1", "2", NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void testTwoNodes(void)
{
    dcNode *flatty = createFlatArithmetic("1 + 2 + 3");
    assertContents(flatty, TAFFY_ADD, 3, "1", "2", "3", NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void testThreeNodes(void)
{
    dcNode *flatty = createFlatArithmetic("1 + 2 + 3 + 4 + 5");
    assertContents(flatty, TAFFY_ADD, 5, "1", "2", "3", "4", "5", NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void testAddAndMultiply(void)
{
    dcNode *flatty =
        createFlatArithmetic("1 + 2 * 3 + 4 + 5 + 6 * 7");
    dcFlatArithmetic *flattery = CAST_FLAT_ARITHMETIC(flatty);

    assertContents(flatty, TAFFY_ADD, 5, "1", "?", "4", "5", "?", NULL);
    assertContents(flattery->values->head->next->object,
                   TAFFY_MULTIPLY,
                   2,
                   "2",
                   "3",
                   NULL);
    assertContents(dcList_getTail(flattery->values),
                   TAFFY_MULTIPLY,
                   2,
                   "6",
                   "7",
                   NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void testMultiplyAndAdd(void)
{
    dcNode *flatty = createFlatArithmetic("1 * 2 * 3 + 4");
    dcFlatArithmetic *flattery = CAST_FLAT_ARITHMETIC(flatty);
    //assertContents(flatty, TAFFY_ADD, 2, "4", NULL);
    assertContents(flattery->values->head->object,
                   TAFFY_MULTIPLY,
                   3,
                   "1",
                   "2",
                   "3",
                   NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void testAddAndRaise(void)
{
    dcNode *flatty = createFlatArithmetic("1 + 2^4 + 3");
    dcFlatArithmetic *flattery = CAST_FLAT_ARITHMETIC(flatty);
    assertContents(flatty, TAFFY_ADD, 3, "1", "?", "3", NULL);
    assertContents(flattery->values->head->next->object,
                   TAFFY_RAISE,
                   2,
                   "2",
                   "4",
                   NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void assertSnipInto(const char *_program,
                           dcTaffyOperator _operator,
                           size_t _arithmeticCount,
                           const char *_head,
                           ...)
{
    dcNode *flatty = (dcFlatArithmetic_snip
                      (createFlatArithmetic(_program), NULL));
    va_list arguments;
    va_start(arguments, _head);
    assertContentsVaList(flatty, _operator, _arithmeticCount, _head, arguments);
    dcNode_free(&flatty, DC_DEEP);
}

static void testAddAndZeroSnip(void)
{
    assertSnipInto("1 + 2 + 0", TAFFY_ADD, 2, "1", "2", NULL);
    assertSnipInto("0 + 1 + 2", TAFFY_ADD, 2, "1", "2", NULL);
    assertSnipInto("1 + 0 + 2", TAFFY_ADD, 2, "1", "2", NULL);
}

static void testZeroPlusZeroSnip(void)
{
    dcNode *flatty = createFlatArithmetic("0 + 0");
    bool modified = false;
    flatty = dcFlatArithmetic_snip(flatty, &modified);
    dcTestUtilities_assert(modified && dcNumberClass_equalsInt32u(flatty, 0));
    dcNode_free(&flatty, DC_DEEP);
}

static void testMultiplyAndOneSnip(void)
{
    assertSnipInto("2 * 3 * 1", TAFFY_MULTIPLY, 2, "2", "3", NULL);
    assertSnipInto("1 * 2 * 3", TAFFY_MULTIPLY, 2, "2", "3", NULL);
    assertSnipInto("2 * 1 * 3", TAFFY_MULTIPLY, 2, "2", "3", NULL);
    assertSnipInto("2 * 1 * 1 * 3 * 1 * 1 * 8 * 7",
                   TAFFY_MULTIPLY,
                   4, "2", "3", "8", "7", NULL);
}

static void testDivideAndOneSnip(void)
{
    assertSnipInto("1 / 2",         TAFFY_DIVIDE, 2, "1", "2", NULL);
    assertSnipInto("2 / 2 / 1",     TAFFY_DIVIDE, 2, "2", "2", NULL);
    assertSnipInto("2 / 1 / 2 / 1", TAFFY_DIVIDE, 2, "2", "2", NULL);

    {
        dcNode *flatty = createFlatArithmetic("2 / 1");
        bool modified = false;
        flatty = dcFlatArithmetic_snip(flatty, &modified);
        dcTestUtilities_assert(modified
                               && dcNumberClass_equalsInt32u(flatty, 2));
        dcNode_free(&flatty, DC_DEEP);
    }
}

static void testRaiseAndOneSnip(void)
{
    assertSnipInto("1 ^ 2",         TAFFY_RAISE, 2, "1", "2", NULL);
    assertSnipInto("2 ^ 2 ^ 1",     TAFFY_RAISE, 2, "2", "2", NULL);

    // second
    {
        dcNode *flatty = createFlatArithmetic("2 ^ 1");
        bool modified = false;
        flatty = dcFlatArithmetic_snip(flatty, &modified);
        dcTestUtilities_assert(modified
                               && dcNumberClass_equalsInt32u(flatty, 2));
        dcNode_free(&flatty, DC_DEEP);
    }

    // third and fourth
    {
        dcNode *flatty = createFlatArithmetic("2 ^ 1 ^ 1");
        bool modified = false;
        flatty = dcFlatArithmetic_snip(flatty, &modified);
        dcTestUtilities_assert(modified
                               && dcNumberClass_equalsInt32u(flatty, 2));
        dcNode_free(&flatty, DC_DEEP);
    }
}

static void testAddAndMultiplyAndOneSnip(void)
{
    dcNode *flatty = createFlatArithmetic("1 + 2 * 1 * 2");
    bool modified = false;
    flatty = dcFlatArithmetic_snip(flatty, &modified);
    dcFlatArithmetic *flattery = CAST_FLAT_ARITHMETIC(flatty);
    dcError_assert(modified);
    assertContents(flatty, TAFFY_ADD, 2, "1", "?", NULL);
    assertContents(dcList_getTail(flattery->values),
                   TAFFY_MULTIPLY,
                   2,
                   "2",
                   "2",
                   NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void testAddAndIdentifier(void)
{
    dcNode *flatty = createFlatArithmetic("1 + a + 2");
    assertContents(flatty, TAFFY_ADD, 3, "1", "a", "2", NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void testAddAndMultiplyVanish(void)
{
    dcNode *flatty = createFlatArithmetic("1 + 2 * 1");
    bool modified = false;
    flatty = dcFlatArithmetic_snip(flatty, &modified);
    dcError_assert(modified);
    assertContents(flatty, TAFFY_ADD, 2, "1", "2", NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void testMultiplyAndAdd1Vanish(void)
{
    dcNode *flatty = createFlatArithmetic("2 * (0 + 1)");
    bool modified = false;
    flatty = dcFlatArithmetic_snip(flatty, &modified);
    dcError_assert(modified);
    dcTestUtilities_assert(dcNumberClass_equalsInt32u(flatty, 2));
    dcNode_free(&flatty, DC_DEEP);
}

static void testSubtract(void)
{
    dcNode *flatty = createFlatArithmetic("1 - 2");
    bool modified = false;
    flatty = dcFlatArithmetic_snip(flatty, &modified);
    dcError_assert(! modified);
    assertContents(flatty, TAFFY_SUBTRACT, 2, "1", "2", NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void testSubtractAndRaise(void)
{
    assertCombineInto("1 - x^2^2", TAFFY_SUBTRACT, 2, "1", "?", NULL);
}

static void testSubtractAndAdd(void)
{
    dcNode *flatty = createFlatArithmetic("1 - 2 + 3");
    flatty = dcFlatArithmetic_snip(flatty, NULL);
    assertContents(flatty, TAFFY_ADD, 2, "?", "3", NULL);
    dcFlatArithmetic *flattery = CAST_FLAT_ARITHMETIC(flatty);
    assertContents(flattery->values->head->object,
                   TAFFY_SUBTRACT,
                   2,
                   "1",
                   "2",
                   NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void testFiveMinusZeroSnip(void)
{
    dcNode *flatty = createFlatArithmetic("5 - 0");
    bool modified = false;
    flatty = dcFlatArithmetic_snip(flatty, &modified);
    dcTestUtilities_assert(modified && dcNumberClass_equalsInt32u(flatty, 5));
    dcNode_free(&flatty, DC_DEEP);
}

static void testZeroMinusFiveSnip(void)
{
    dcNode *flatty = createFlatArithmetic("0 - 5");
    bool modified = false;
    flatty = dcFlatArithmetic_snip(flatty, &modified);
    dcError_assert(! modified);
    assertContents(flatty, TAFFY_SUBTRACT, 2, "0", "5", NULL);
    dcNode_free(&flatty, DC_DEEP);
}

static void comparePrograms(const char *_program1,
                            const char *_program2,
                            dcTaffyOperator _wantedResult)
{
    dcNode *flatty1 = createFlatArithmetic(_program1);
    dcNode *flatty2 = createFlatArithmetic(_program2);
    dcTaffyOperator compareResult;
    dcResult result = dcNode_compare(flatty1, flatty2, &compareResult);
    dcTestUtilities_assert(result == TAFFY_SUCCESS
                           && compareResult == _wantedResult);
    dcNode_free(&flatty1, DC_DEEP);
    dcNode_free(&flatty2, DC_DEEP);
}

static void testEquality(const char *_program)
{
    comparePrograms(_program, _program, TAFFY_EQUALS);
}

static void testSingleNodeComparison(void)
{
    testEquality("1 + 2");
    testEquality("1 * 2");
    testEquality("1 - 2");
    testEquality("1 ^ 2");

    // negative test cases
    comparePrograms("1 + 2", "1 + 3", TAFFY_LESS_THAN);
    comparePrograms("1 * 2", "1 * 3", TAFFY_LESS_THAN);
    comparePrograms("1 - 2", "1 - 3", TAFFY_LESS_THAN);
    comparePrograms("1 ^ 2", "1 ^ 3", TAFFY_LESS_THAN);
}

static void testTwoNodeComparison(void)
{
    testEquality("1 + 2 + 3");
    testEquality("1 * 2 * 3");
    testEquality("1 - 2 - 3");
    testEquality("1 ^ 2 ^ 3");

    // negative test cases
    comparePrograms("1 + 2 + 3", "1 + 3 + 4", TAFFY_LESS_THAN);
    comparePrograms("1 * 2 * 3", "1 * 3 * 4", TAFFY_LESS_THAN);
    comparePrograms("1 - 2 - 3", "1 - 3 - 4", TAFFY_LESS_THAN);
    comparePrograms("1 ^ 2 ^ 3", "1 ^ 3 ^ 4", TAFFY_LESS_THAN);
}

static void testAddCombine(void)
{
    {
        bool modified = false;
        dcNode *flatty = dcFlatArithmetic_combine
            (createFlatArithmetic("1 + 2 + a + 4 + 5"), &modified);
        dcError_assert(modified);
        assertContents(flatty, TAFFY_ADD, 2, "12", "a", NULL);
        dcNode_free(&flatty, DC_DEEP);
    }

    {
        bool modified = false;
        dcNode *flatty = dcFlatArithmetic_combine
            (createFlatArithmetic("b + 1 + 2 + a + 4 + 5"), &modified);
        dcError_assert(modified);
        assertContents(flatty, TAFFY_ADD, 3, "b", "12", "a", NULL);
        dcNode_free(&flatty, DC_DEEP);
    }
}

static void assertPoppedInto(const char *_flatArithmeticProgram,
                             const char *_program)
{
    dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
              "Program: %s\n",
              _flatArithmeticProgram);

    dcNode *flatty = createFlatArithmetic(_flatArithmeticProgram);
    flatty = dcFlatArithmetic_shrink(flatty, NULL);
    dcTestUtilities_expectStringNodeEqual(_program, flatty);
    dcNode_free(&flatty, DC_DEEP);
}

static void testMultiplyCombine(void)
{
    assertCombineInto("b * 2 * a", TAFFY_MULTIPLY, 3, "b", "2", "a", NULL);
    assertCombineInto("b * 2 * a * (a + b)",
                      TAFFY_MULTIPLY,
                      4, "b", "2", "a", "?", NULL);
}

static void testRaiseCombine(void)
{
    assertCombineInto("a^2^2",   TAFFY_RAISE, 2, "a", "4", NULL);
    assertCombineInto("(a^2)^2", TAFFY_RAISE, 2, "a", "4", NULL);
}

static void testCombineEqual(const char *_program, const char *_expected)
{
    dcNode *flatty1 = createFlatArithmetic(_program);
    flatty1 = dcFlatArithmetic_combine(flatty1, NULL);
    dcNode *flatty2 = createFlatArithmetic(_expected);
    dcTaffyOperator compareResult;

    if (! ((dcNode_compare(flatty1, flatty2, &compareResult)
            == TAFFY_SUCCESS)
           && compareResult == TAFFY_EQUALS))
    {
        fprintf(stderr,
                "combine failure:\n"
                "got:      %s\n"
                "expected: %s",
                dcNode_display(flatty1),
                dcNode_display(flatty2));
        dcError_assert(false);
    }

    dcNode_free(&flatty1, DC_DEEP);
    dcNode_free(&flatty2, DC_DEEP);
}

static void testCombineFail(void)
{
    //const char *combineFail =
    //    "((((a * x) + b) * ln((a * x) + b)) - (a * x)) / a";
    const char *combineFail =
        "((((a * x) + b) * ln((a * x) + b)) - (a * x)) / a";
    testCombineEqual(combineFail, combineFail);
}

static void testMethodCallCombine(void)
{
    testCombineEqual("[a foo] * [a foo]", "[a foo]^2");
}

static void testCombineSnip(const char *_program, const char *_expected)
{
    dcNode *flatty1 = createFlatArithmetic(_program);
    flatty1 = dcFlatArithmetic_combine(flatty1, NULL);
    flatty1 = dcFlatArithmetic_snip(flatty1, NULL);
    dcNode *flatty2 = createFlatArithmetic(_expected);
    dcTaffyOperator compareResult;

    dcTestUtilities_assert((dcNode_compare(flatty1, flatty2, &compareResult)
                            == TAFFY_SUCCESS)
                           && compareResult == TAFFY_EQUALS);

    dcNode_free(&flatty1, DC_DEEP);
    dcNode_free(&flatty2, DC_DEEP);
}

static void testSort(const char *_program, const char *_expected)
{
    dcNode *flatty1 = createFlatArithmetic(_program);
    dcFlatArithmetic_sort(flatty1);
    dcNode *flatty2 = createFlatArithmetic(_expected);
    dcTaffyOperator compareResult;

    dcTestUtilities_assert((dcNode_compare(flatty1, flatty2, &compareResult)
                            == TAFFY_SUCCESS)
                           && compareResult == TAFFY_EQUALS);

    dcNode_free(&flatty1, DC_DEEP);
    dcNode_free(&flatty2, DC_DEEP);
}

static void testShrinkWithProgram(const char *_program, const char *_expected)
{
    dcLog_log(FLAT_ARITHMETIC_TEST_LOG, "Program: %s\n", _program);
    dcNode *flatty = (dcFlatArithmetic_shrink
                      (createFlatArithmetic(_program), NULL));
    dcTestUtilities_expectStringNodeEqual(_expected, flatty);
    dcNode_free(&flatty, DC_DEEP);
}

static void testAddWithTwoMultiplies(void)
{
    testCombineSnip("(2 * x) + (2 * x)", "4 * x");
    testCombineSnip("(y * x) + (y * x)", "2 * y * x");
}

static void testMultiplyWithTwoAdds(void)
{
    // test snip + combine
    testCombineSnip("(2 + x) * (2 + x)", "(2 + x)^2");
    testCombineSnip("(a + x) * (a + x)", "(a + x)^2");
}

static void testLeftShift(void)
{
    assertContentsAndFree("1 << 2", TAFFY_LEFT_SHIFT, 2, "1", "2", NULL);
}

static void testRightShift(void)
{
    assertContentsAndFree("1 >> 2", TAFFY_RIGHT_SHIFT, 2, "1", "2", NULL);
}

static void testBitAnd(void)
{
    assertContentsAndFree("1 & 2", TAFFY_BIT_AND, 2, "1", "2", NULL);
}

static void testBitOr(void)
{
    assertContentsAndFree("1 | 2", TAFFY_BIT_OR, 2, "1", "2", NULL);
}

static void testSorts(void)
{
    testSort("x + 3 + y", "3 + x + y");
    testSort("[x foo] + 3 + [y zoo]", "3 + [x foo] + [y zoo]");
}

typedef struct
{
    const char *program;
    const char *expected;
} Program;

static void testEqualities(void)
{
    struct
    {
        const char *left;
        const char *right;
        bool expectedEquality;
    } programs[] =
    {
        {"3 * x",       "x * 3",       true},
        {"3 + x",       "x + 3",       true},
        {"3 & x",       "x & 3",       true},
        {"3 | x",       "x | 3",       true},
        {"3 * x",       "3 + x",       false},
        {"3 - x",       "x - 3",       false},
        {"3 * (x + y)", "(x + y) * 3", true},
        {"3 + (x * y)", "(x * y) + 3", true},
        {"3 / (x * y)", "(x * y) / 3", false},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcNode *left = createFlatArithmetic(programs[i].left);
        dcNode *right = createFlatArithmetic(programs[i].right);
        dcTaffyOperator result = dcNode_easyCompare(left, right);

        if ((programs[i].expectedEquality && result != TAFFY_EQUALS)
            || (! programs[i].expectedEquality && result == TAFFY_EQUALS))
        {
            dcError_assert(false);
        }

        dcNode_free(&left, DC_DEEP);
        dcNode_free(&right, DC_DEEP);
    }
}

static void testComplexRaise(void)
{
    const Program tests[] =
    {
        {"i^0",        "1"},
        {"i^1",        "i"},
        {"i^2",        "-1"},
        {"i^3",        "-i"},
        {"i^4",        "1"},
        {"i^5",        "i"},
        {"(2 + 3i)^0", "1"},
        {"(2 + 3i)^1", "2 + 3i"},
        {"(2 + 3i)^2", "-5 + 12i"},

        {"i^i",               "0.20787957635"},
        {"i^(2i)",            "0.04321391826"},
        {"i^(1 + i)",         "0.20787957635i"},
        {"i^(2 + i)",         "-0.20787957635"},
        {"i^(3 + 3i)",        "-0.00898329102i"},
        {"(2i)^(3 + 3i)",     "0.0627684162 + 0.0349985007i"},
        {"(2 + 3i)^(3 + 3i)", "2.14140138 + 1.20518796i"},

        {"(-1)^(-0.2)",   "0.8090169943749492 - 0.5877852522924694i"},
        {"(-1)^(-0.3)",   "0.5877852522924694 - 0.8090169943749492i"},
        {"(-1)^(-0.9)",   "-0.9510565162951549 - 0.3090169943749516i"},
        {"(-1)^(-0.999)", "-0.9999950652018560 - 0.003141587485879687i"},

        {"(-1.5)^(-0.2)",   "0.7460009710363092 - 0.5420014313911695i"},
        {"(-1.5)^(-0.3)",   "0.5204647339435095 - 0.7163582500426922i"},
        {"(-1.5)^(-0.9)",   "-0.6602739943078416 - 0.2145360256609942i"},
        {"(-1.5)^(-0.999)", "-0.6669337403471723 - 0.002095241032177167i"},

        {"(-2)^(-0.2)",   "0.7042902001692496 - 0.5116967824803639i"},
        {"(-2)^(-0.3)",   "0.4774299797174131 - 0.6571259923739721i"},
        {"(-2)^(-0.9)",   "-0.5096585677686813 - 0.1655981070219000i"},
        {"(-2)^(-0.999)", "-0.5003442246213551 - 0.001571882911627500i"},

        {"(-1)^(-2.5)", "-i"},
        {"(-1)^(-3.5)", "i"},

        {"(-2)^(-1.5)", "0.3535533905932738i"},
        {"(-2)^(-2.5)", "-0.1767766952966369i"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG, "Program: %s\n", tests->program);

        dcNode *program = createFlatArithmetic(tests[i].program);
        dcNode *expected = createFlatArithmetic(tests[i].expected);
        program = dcFlatArithmetic_shrink(program, NULL);

        // a little convoluted
        char *programDisplay = dcNode_synchronizedDisplay(program);
        char *expectedDisplay = dcNode_synchronizedDisplay(expected);

        dcNode *result =
            dcStringEvaluator_evalFormat("dcFlatArithmeticTest.c",
                                         STRING_EVALUATOR_ASSERT_NO_EXCEPTION,
                                         "%s ~= %s",
                                         programDisplay,
                                         expectedDisplay);

        if (result != dcYesClass_getInstance())
        {
            fprintf(stderr,
                    "Test failed: %s ~= %s for program: %s\n",
                    programDisplay,
                    expectedDisplay,
                    tests[i].program);
        }

        dcMemory_free(programDisplay);
        dcMemory_free(expectedDisplay);
        dcNode_free(&program, DC_DEEP);
        dcNode_free(&expected, DC_DEEP);
    }
}

static void testShrink(void)
{
    const Program programs[] =
    {
        //
        // add
        //

        // simple
        {"1 + 1",             "2"},
        {"1 + 2 + 3 + 4 + 5", "15"},

        {"x + x",                       "2x"},
        {"x + x + x",                   "3x"},
        {"x + y + x + y",               "2x + 2y"},
        {"3 + x + 3",                   "x + 6"},
        {"1 + 2 + 3 + y + y + 5",       "2y + 11"}, //11 + 2y"},
        {"2 * (x + 3) + 3 * (x + 3)",   "5x + 15"},
        {"1 + 2 * (x + 3) + 3 * (y + 4) + 4 * (x + 3) + 5 * (y + 4)",
         "6x + 8y + 51"},

        // add with multiplication on left
        {"2x + x",                      "3x"},
        {"2x + 3x + x",                 "6x"},
        // add with multiplication on right
        {"x + 2x",                      "3x"},
        {"x + 2x + x",                  "4x"},
        // add with mixed multiplications
        {"2x + x + 2x",                 "5x"},
        {"x + 2x + x",                  "4x"},

        // add with multiply on right right
        {"x + x * 3",  "4x"},
        {"x + x * -3", "-2x"},
        {"1 + x * -2", "-2x + 1"},

        // add with multiply on left left
        {"x * 3 + x",  "4x"},
        {"x * -3 + x", "-2x"},

        // add with raise on left
        {"x^2 + x",                      "x^2 + x"},
        {"x^2 + x^3 + x",                "x^3 + x^2 + x"},
        // add with raise on right
        {"x + x^2",                      "x^2 + x"},
        {"x + x^2 + x^3",                "x^3 + x^2 + x"},
        // add with mixed raises
        {"x^2 + x + x^2",                "2x^2 + x"},
        {"x + x^2 + x",                  "x^2 + 2x"},

        // add with divide on left
        {"x/y + x",                      "(y * x + x) / y"},
        {"x/y + x/y + x",                "(x * y + 2x) / y"},
        {"x^3 / 2 + y * x",              "(x^3 + 2 * y * x) / 2"},
        // add with divide on right
        {"x + x/y",                      "(y * x + x) / y"},
        {"x + x/y + x/y",                "(x * y + 2x) / y"},
        // add with mixed divides
        {"x/y + x + x/y",                "(x * y + 2x) / y"},
        {"x + x/y + x",                  "(2 * y * x + x) / y"},

        {"(1 + 2x) - (2 + x)",           "x - 1"},
        {"(1 + 2x) - (1 + x)",           "x"},

        // add with subtract
        {"(a - 1) + 1",      "a"},
        {"1 + (a - 1)",      "a"},
        {"1 + (a - 1) + z",  "a + z"},

        {"10 / (5 / 2)", "4"},
        {"10 / 5 / 2",   "1"},

        {"10 - (2 - 5)", "13"},
        {"10 - 2 - 5",   "3"},

        // add with two multiplies
        {"x * 2 + 3 + x * 3",   "5x + 3"},
        {"x * 2 + 3 * (5 + x)", "5x + 15"},

        //
        // subtract
        //

        // to 0
        {"2 - 2", "0"},

        // to negative
        {"2 - 3 - 1",         "-2"},
        {"2 - 1 - 1 - 1 - 2", "-3"},

        {"x - x - x",                   "-x"},
        {"x - x - x - x",               "-2x"},
        {"x - 3 - 2",                   "x - 5"},
        {"x - 3 - 2 - x",               "-5"},
        {"x - x - x - x - x",           "-3x"},
        {"x - y - x - x - x",           "-y - 2x"},
        {"x - y - x - x",               "-y - x"},
        {"x - y - x",                   "-y"},
        {"3 - (x + 5)",                 "-x - 2"},
        {"3 - (x + 5 + y + z)",         "-x - y - z - 2"},
        {"x - y - x - x - x",           "-y - 2x"},
        {"x - y - y - x - x",           "-2y - x"},
        {"x + x - (x + x) - (x + x)",   "-2x"},
        {"y - (y - 1)",                 "1"},
        {"1 - (1 - 1)",                 "1"},
        {"x - -y",                      "x + y"},
        {"x - -2",                      "x + 2"},
        {"x - -2 - -y",                 "x + y + 2"},
        {"x - -2 - -y - -z",            "x + y + z + 2"},
        {"x - -2 - -y - -z - w - -xx",  "x + y + z + xx - w + 2"},
        {"x - -2y",                     "x + 2y"},

        // subtract with add on left
        {"(3 + x) - 5",                 "x - 2"},
        {"x + (3 - 3) - x - x",         "-x"},
        {"(y + z + 3 + x) - 5",         "y + z + x - 2"},
        {"(2 + x) - 5",                 "x - 3"},
        // subtract with add on right
        {"5 - (2 + x)",                 "-x + 3"},
        // subtract with mixed adds
        {"3 - (1 + x) - 3",             "-x - 1"},
        {"(1 + x) - 2 - (1 + x)",       "-2"},

        // subtract with multiplication on left
        {"2x - x",                      "x"},
        {"3x - 2x - x",                 "0"},
        // subtract with multiplication on right
        {"x - 2x",                      "-x"},
        {"x - 2x - 3x",                 "-4x"},
        // subtract with mixed multiplications
        {"2x - x - 2x",                 "-x"},
        {"x - 2x - x",                  "-2x"},

        // subtract with raise on left
        {"x^2 - x",                     "x^2 - x"},
        // subtract with raise on right
        {"x - x^2",                     "-x^2 + x"},

        // subtract with divide on left
        {"x/y - x",                     "(-y * x + x) / y"},
        // subtract with divide on right
        {"x - x/y",                     "(x * y - x) / y"},

        // subtract adds
        {"(x + 3 + y + z + zz) - (3 + y)", "x + z + zz"},

        // subtract something with a denominator
        {"x * y - (y * x) / 2", "(x * y) / 2"},
        {"x * y - (z * x) / 2", "(2 * y * x - z * x) / 2"},

        // subtract a divide that has a negative numerator
        {"x - (-3) / y",     "(x * y + 3) / y"},
        {"x - (-3 * x) / y", "(x * y + 3x) / y"},

        // subtract multiply
        {"x - x * 2",    "-x"},
        {"3x - x * 2",   "x"},
        {"3x - x * 4",   "-x"},
        {"x - (-x * 3)", "4x"},

        {"x * 4 - x",     "3x"},
        {"x * 1 - x",     "0"},

        {"x * 4 - x * 3", "x"},
        {"x * 4 - x * 6", "-2x"},

        // subtract with subtract on left
        {"(3 - x) - 4", "-x - 1"},
        {"(3 - x) - x", "-2x + 3"},

        //
        // multiply
        //

        // to 0
        {"x * 0",         "0"},
        {"3 * 0",         "0"},
        {"(3 + y) * 0",   "0"},
        {"(3 + y)^2 * 0", "0"},
        {"2 * 3 * 3 * 1 * 234876234876234 * 0",     "0"},
        {"2 * 3 * 3 * 1 * 234876234876234.234 * 0", "0"},

        // simple multiplication
        {"2 * 3", "6"},
        {"2 * 3 * 3 * 2", "36"},
        {"2 * 3 * 3 * 1", "18"},

        {"(x + x + x) * (x + x + x)",            "9x^2"},
        {"x * x + x * x",                        "2x^2"},
        {"x * x + x",                            "x^2 + x"},
        {"x * x + x * x + x",                    "2x^2 + x"},
        {"(3 * x * y * z) + (2 * x * y * z)",    "5 * x * y * z"},
        {"(x + 3) * (x + 3)",                    "(x + 3)^2"},
        {"(x + 3) * (y + 1) * (x + 3)",          "y * (x + 3)^2 + (x + 3)^2"},

        {"(3 * (x * 3))",                        "9x"},
        {"3 * (3/x)",                            "9 / x"},
        {"(3/x) * 3",                            "9 / x"},
        {"-1 * (1 / x^2)",                       "-1 / x^2"},
        {"x * (1/x^2)",                          "1 / x"},
        {"x * (a/x^2)",                          "a / x"},
        {"x * (a/x)",                            "a"},
        {"(x + y + z) * (y + 1) * (x + y + z)",
         "y * (x + y + z)^2 + (x + y + z)^2"},

        // multiplication with add on left
        {"(x + y) * x",                 "x^2 + y * x"},
        {"(x + y) * (x + y) * z",       "(x + y)^2 * z"},
        // multiplication with add on right
        {"x * (x + y)",                 "x^2 + y * x"},
        {"x * (x + y) * (x + y)",       "x * (x + y)^2"},
        // multiplication with mixed adds
        {"x * (x + y) * x",             "x^3 + y * x^2"},
        {"(x + y) * x * (x + y)",       "(x + y)^2 * x"},

        // multiplication with subtract on left
        {"(x - y) * x",                 "x^2 - y * x"},
        {"(x - y) * (x - y) * z",       "(x - y)^2 * z"},
        // multiplication with subtract on right
        {"x * (x - y)",                 "x^2 - y * x"},
        {"x * (x - y) * (x - y)",       "x * (x - y)^2"},
        // multiplication with mixed subtracts
        {"x * (x - y) * x",             "x^3 - y * x^2"},
        {"(x - y) * x * (x - y)",       "(x - y)^2 * x"},

        // multiplication with subtracts
        {"(a - b) * (c - 0.5 * z)",
         "c * a + 0.5 * z * b - 0.5 * z * a - c * b"},

        // multiplication with divide on left
        {"(x / y) * x",                 "x^2 / y"},
        {"(x / y) * (x / y) * z",       "(x^2 * z) / y^2"},
        {"(x / y) * y",                 "x"},
        {"(x / y) * (x / y) * y",       "x^2 / y"},
        // multiplication with divide on right
        {"x * (x / y)",                 "x^2 / y"},
        {"x * (x / y) * (x / y)",       "x^3 / y^2"},
        {"y * (x / y)",                 "x"},
        {"y * (x / y) * (x / y)",       "x^2 / y"},
        // multiplication with mixed divides
        {"x * (x / y) * x",             "x^3 / y"},
        {"(x / y) * x * (x / y)",       "x^3 / y^2"},
        {"y * (x / y) * y",             "x * y"},
        {"(x / y) * y * (x / y)",       "x^2 / y"},

        // multiplication with raise on left
        {"x^y * x",                     "x^(y + 1)"},
        {"x^y * x^y * z",               "x^(2y) * z"},
        // multiplication with raise on right
        {"x * x^y",                     "x^(y + 1)"},
        {"x * x^y * x^y",               "x^(2y + 1)"},
        // multiplication with mixed raises
        {"x * x^y * x",                 "x^(y + 2)"},
        {"x^y * x * x^y",               "x^(2y + 1)"},

        // multiply raise with number on right
        {"x * x^2", "x^3"},
        {"x * x^y", "x^(y + 1)"},

        {"0 * 2",                                "0"},
        {"x / 1",                                "x"},
        {"2 * (a / 2)",                          "a"},
        {"2 * (x / 2)",                          "x"},
        {"0 / 2",                                "0"},
        {"x^0.5 * x^2",                          "x^2.5"},

        //
        // divide
        //

        {"((3 + x) * x * y) / (x * y)",           "x + 3"},
        {"(x * y * z) / z",                       "x * y"},
        {"(3x) / (3y)",                           "x / y"},
        {"x / x / x",                             "1 / x"},
        {"(x + y) / (x + y) / (x + y)",           "1 / (x + y)"},
        {"(4 * x * y * z) / (2 * x * y * z * w)", "2 / w"},
        {"(x * y * z * w) / z",                   "x * y * w"},
        {"((x + 3) * y * z) / (x + 3)",           "y * z"},
        {"x^2/x",                                 "x"},
        {"x^3/x",                                 "x^2"},
        {"x^3/x^2",                               "x"},
        {"x^y/x^z",                               "x^(y - z)"},
        {"x^3/x^3",                               "1"},
        {"x^(y^w)/x^z",                           "x^(y^w - z)"},
        {"x^(2^x)/x",                             "x^(2^x - 1)"},
        {"((x^2) * (y^3))/x",                     "x * y^3"},
        {"2 / (4 * x)",                           "1 / (2x)"},
        {"4 / (2 * x)",                           "2 / x"},
        {"2 / (2 * x)",                           "1 / x"},
        {"x / x^2",                               "1 / x"},
        {"((2 * x) / x^2)",                       "2 / x"},
        {"x / x^x",                               "1 / x^(x - 1)"},
        {"0 / (2 * x)",                           "0"},

        // factor out GCD
        {"10 / 25", "2 / 5"},

        // divide GCD with add on top
        {"(20x + 20y) / 40", "(x + y) / 2"},
        {"(20x + 30y) / 40", "(20x + 30y) / 40"},

        // divide with add on left
        {"(x + y) / x",                 "(x + y) / x"},
        {"(x + y) / (x + y) / z",       "1 / z"},
        // divide with add on right
        {"x / (x + y)",                 "x / (x + y)"},
        {"x / (x + y) / (x + y)",       "x / (x + y)^2"},
        {"x / ((x + y) / (x + y))",     "x"},
        // divide with mixed adds
        {"x / (x + y) / x",             "1 / (x + y)"},
        {"(x + y) / x / (x + y)",       "1 / x"},

        // divide with subtract on left
        {"(x - y) / x",                 "(x - y) / x"},
        {"(x - y) / (x - y) / z",       "1 / z"},
        // divide with subtract on right
        {"x / (x - y)",                 "x / (x - y)"},
        {"x / (x - y) / (x - y)",       "x / (x - y)^2"},
        {"x / ((x - y) / (x - y))",     "x"},
        // divide with mixed subtracts
        {"x / (x - y) / x",             "1 / (x - y)"},
        {"(x - y) / x / (x - y)",       "1 / x"},

        // divide with multiply on left
        {"(x * y) / x",                 "y"},
        {"(x * y) / (x * y) / z",       "1 / z"},
        // divide with multiply on right
        {"x / (x * y)",                 "1 / y"},
        {"x / (x * y) / (x * y)",       "1 / (y^2 * x)"},
        // divide with mixed multiplies
        {"x / (x * y) / x",             "1 / (y * x)"},
        {"(x * y) / x / (x * y)",       "1 / x"},

        // divide with raise on left
        {"x^y / x",                 "x^(y - 1)"},
        {"x^y / x^y / z",           "1 / z"},
        // divide with raise on right
        {"x / x^y",                 "1 / x^(y - 1)"},
        {"x / x^y / x^y",           "1 / x^(2y - 1)"},
        // divide with mixed raises
        {"x / x^y / x",             "1 / x^y"},
        {"x^y / x / x^y",           "1 / x"},

        // divide a multiplication with canceling
        {"x / (y * x^2)", "1 / (y * x)"},

        // divide flips
        {"1 / (1 / y)",       "y"},
        {"1 / (x / y)",       "y / x"},
        {"1 / (x / (y + 1))", "(y + 1) / x"},
        {"z / (x / y / w)",   "(z * w * y) / x"},

        // keep negative on top
        {"-(1 / (4 * x^4))",  "-1 / (4x^4)"},

        // turn that frown upside down!
        {"x / -a",             "-x / a"},
        {"-x / a",             "-x / a"},
        {"-x / -a",            "x / a"},
        {"x / -(a + 1)",       "-x / (a + 1)"},
        {"(x + 1) / -(a + 1)", "(-x - 1) / (a + 1)"},

        // divide multiply by something
        {"((x + y) * z) / (x + y)", "z"},

        // divide 1 by negative multiply
        {"1 / (-4 * x^4)", "-1 / (4x^4)"},

        //
        // raise
        //

        // to 0
        //{"0^3^4",     "0"},
        {"0^(x + y)", "0"},

        // to 1
        {"3482304879^0",      "1"},
        //{"3482304879.1234^0", "1"},
        //{"1^3^4",             "1"},

        // simple
        //{"2^3^2", "512"},

        // raise number by divide
        {"8^(1/3)", "2"},
        {"4^(1/2)", "2"},

        // parentheses
        {"(2^2)^3", "64"},
        {"2^(2^3)", "256"},

        // multiply powers, some not
        {"(x^3)^(1/3)", "(x^3)^(1 / 3)"},
        {"(x^3)^0.5",   "(x^3)^0.5"},
        {"(x^3)^(2)",   "x^6"},
        {"(x^3)^(-2)",  "1 / x^6"},

        {"x^0.5 + x",                                   "x^0.5 + x"},
        {"x^2 + x",                                     "x^2 + x"},
        {"x^2 + 2 * x^2",                               "3x^2"},
        {"x^2 * x",                                     "x^3"},
        {"x^2 * x^3",                                   "x^5"},
        {"x^2^2",                                       "x^4"},
        {"x^2 * x^3 * x^8^2",                           "x^69"},
        {"(3 * x^2) + (2 * x^2)",                       "5x^2"},
        {"x^2 * x^3 + x^2 * x^3",                       "2x^5"},
        {"[x foo]^2 * [x foo]^3",                       "[x foo]^5"},
        {"[x foo]^2 * x^3",                             "[x foo]^2 * x^3"},
        {"(x^2 + 2 * x^2) * (2 * x^2 + 3 * x^2)",       "15x^4"},
        {"x^2^x^3^2",                                   "x^(2^(x^9))"},
        {"[x foo] * [x foo] + [x foo] * [x foo] + [x foo]",
         "2 * [x foo]^2 + [x foo]"},
        {"(x * y)^2 * (x * y)^3",                       "(x * y)^5"},
        {"(x + y + z)^3 * (x + y + z)^2",               "(x + y + z)^5"},
        {"2 * (x * 2)^1",                               "4x"},

        // raise two raises
        {"x^(y + 1) * x^y", "x^(2y + 1)"},

        // raise with add on bottom
        {"(x + y)^y",           "(x + y)^y"},
        // raise with add on top
        {"x^(x + y)",           "x^(x + y)"},
        // raise with subtract on bottom
        {"(x - y)^y",           "(x - y)^y"},
        // raise with subtract on top
        {"x^(x - y)",           "x^(x - y)"},
        // raise with multiply on bottom
        {"(x * y)^y",           "(x * y)^y"},
        // raise with multiply on top
        {"x^(x * y)",           "x^(x * y)"},
        // raise with divide on bottom
        {"(x / y)^y",           "(x / y)^y"},
        // raise with divide on top
        {"x^(x / y)",           "x^(x / y)"},

        // raise with subtract
        {"x * x^(y^z - 1)", "x^(y^z)"},
        {"x^(y^z - 1) * x", "x^(y^z)"},

        // raise with add
        {"x * x^(y^z + 1)", "x^(y^z + 2)"},
        {"x^(y^z + 1) * x", "x^(y^z + 2)"},

        // raise to i
        {"(-4)^0.5",   "2i"},
        {"(-4)^(1/2)", "2i"},

        // complex raise
        {"i^2", "-1"},
        {"i^3", "-i"},
        {"i^4", "1"},
        {"i^5", "i"},
        {"i^6", "-1"},
        {"i^7", "-i"},
        {"i^8", "1"},
        {"i^9", "i"},

        {"(-1)^(1)",  "-1"},
        {"(-1)^(-1)", "-1"},

        // complex raise due to fractional exponent
        {"(-1)^(-0.5)", "-i"},
        {"(-1)^(0.5)",  "i"},

        // others
        {"1 % 2",                                        "1 % 2"},
        {"1 and 2",                                      "1 and 2"},

        {"((1 / x^(x)) * (x^x * (1 + ln(x))))",          "1 + ln(x)"},
        {"((1 / (2 * x^(x))) * (x^x * (1 + ln(x))))",    "(1 + ln(x)) / 2"},

        // move over
        {"(1 / x) * y", "y / x"},
        {"y * (1 / x)", "y / x"},
        {"(2 / x) * y", "(2y) / x"},
        {"y * (2 / x)", "(2y) / x"},

        //
        // bit or
        //

        {"x | y", "x | y"},
        {"x | x", "x"},
        {"0 | y", "y"},
        {"x | 0", "x"},

        // or with adds
        {"(x + y) | x", "(x + y) | x"},
        {"x | (x + y)", "x | (x + y)"},
        // or with subtracts
        {"(x - y) | x", "(x - y) | x"},
        {"x | (x - y)", "x | (x - y)"},
        // or with multiply
        {"(x * y) | x", "(x * y) | x"},
        {"x | (x * y)", "x | (x * y)"},
        // or with divide
        {"(x / y) | x", "(x / y) | x"},
        {"x | (x / y)", "x | (x / y)"},
        // or with raises
        {"x^y | x", "x^y | x"},
        {"x | x^y", "x | x^y"},

        //
        // bit and
        //

        {"x & y", "x & y"},
        {"x & x", "x"},
        {"0 & y", "0"},
        {"x & 0", "0"},

        // or with adds
        {"(x + y) & x", "(x + y) & x"},
        {"x & (x + y)", "x & (x + y)"},
        // or with subtracts
        {"(x - y) & x", "(x - y) & x"},
        {"x & (x - y)", "x & (x - y)"},
        // or with multiply
        {"(x * y) & x", "(x * y) & x"},
        {"x & (x * y)", "x & (x * y)"},
        // or with divide
        {"(x / y) & x", "(x / y) & x"},
        {"x & (x / y)", "x & (x / y)"},
        // or with raises
        {"x^y & x", "x^y & x"},
        {"x & x^y", "x & x^y"},

        //
        // left shift
        //

        {"x << y", "x << y"},
        {"x << x", "x << x"},
        {"0 << y", "0"},
        {"x << 0", "x"},

        // or with adds
        {"(x + y) << x", "(x + y) << x"},
        {"x << (x + y)", "x << (x + y)"},
        // or with subtracts
        {"(x - y) << x", "(x - y) << x"},
        {"x << (x - y)", "x << (x - y)"},
        // or with multiply
        {"(x * y) << x", "(x * y) << x"},
        {"x << (x * y)", "x << (x * y)"},
        // or with divide
        {"(x / y) << x", "(x / y) << x"},
        {"x << (x / y)", "x << (x / y)"},
        // or with raises
        {"x^y << x", "x^y << x"},
        {"x << x^y", "x << x^y"},

        //
        // right shift
        //

        {"x >> y", "x >> y"},
        {"x >> x", "x >> x"},
        {"0 >> y", "0"},
        {"x >> 0", "x"},

        // or with adds
        {"(x + y) >> x", "(x + y) >> x"},
        {"x >> (x + y)", "x >> (x + y)"},
        // or with subtracts
        {"(x - y) >> x", "(x - y) >> x"},
        {"x >> (x - y)", "x >> (x - y)"},
        // or with multiply
        {"(x * y) >> x", "(x * y) >> x"},
        {"x >> (x * y)", "x >> (x * y)"},
        // or with divide
        {"(x / y) >> x", "(x / y) >> x"},
        {"x >> (x / y)", "x >> (x / y)"},
        // or with raises
        {"x^y >> x", "x^y >> x"},
        {"x >> x^y", "x >> x^y"},

        // with canceling
        {"(a * x + 3 * a * x) / a", "4x"},

        // negative negative, add to the rescue!!!!1
        {"x - (-y)",               "x + y"},
        {"x - (-y) - (-z)",        "x + y + z"},
        {"x - (-y) - (-z) - (-w)", "x + y + z + w"},

        // move negative to front
        {"x * (-1 * y)",     "-x * y"},
        {"x * y * (-1 * z)", "-x * y * z"},

        // bring -1 to top
        {"1 / -1", "-1"},
        {"x / -1", "-x"},
        {"(x + y) / -1", "-x - y"},

        // factor
        {"(y * x * (x^2 + a)) / (x^3 + x * a)", "y"},
        {"(y * (x^3 + x * a)) / (x * (x^2 + a))", "y"},
        {"1 / -x", "-1 / x"},

        // order subtract
        {"a + b",      "a + b"},
        {"a + 3 * b",  "a + 3b"},
        {"-a + b",     "b - a"},
        {"-3 + b",     "b - 3"},
        {"-3 * a + b", "b - 3a"},

        // already factored
        {"1 - x / y",       "(y - x) / y"},
        {"3 * (1 - x / y)", "(3y - 3x) / y"},

        {"0.500000 * (x * 0.5 * (x * (1 - cos(2x)) - sin(2x)) - cos(2x) "
         "* (0.250 - 0.5 * x^2)) - x * (0.2500000 * (x - sin(2x)) "
         "- 1.2500000 * cos(2x) * x)",
         "1.2500000 * cos(2x) * x^2 - 0.125000000 * cos(2x)"},

        {"0.500000 * (x * 0.5 * (x * (1 - cos(2x)) - sin(2x)) "
         "- cos(2x) * (0.250 - 0.5 * x^2))",
         "0.2500000 * x^2 - 0.2500000 * sin(2x) * x - 0.125000000 * cos(2x)"},

        // convert divide with non-whole to multiply with non-whole
        {"(6.5x) / 20",   "0.325 * x"},
        {"(6x) / 12.5",   "(12x) / 25"},
        {"(6.5x) / 12.5", "(13x) / 25"},

        {"(sec(x) * tan(x) * 1 * 2 + ln(sec(x) + tan(x)) * 1 * 2) / (2 * 2)",
         "(sec(x) * tan(x) + ln(sec(x) + tan(x))) / 2"},

        {"(x + 1)^(3.0 - 3.5) - (x + 1)^(2.0 - 3.5)",
         "x / (x + 1)^1.5"},

        {"(0.2500000 * x^2 * (x + 1)^3.0 + -1.5 * x^3 * (x + 1)^2.0 "
         "+ -2 * x^2 * (x + 1)^2.0) / (x + 1)^3.5",
         "(-1.2500000 * x^3 - 1.7500000 * x^2) / (x + 1)^1.5"},

        // trigonometric
        // more trigonometric tests are in testCombine()
        {"1 - x  -sin(x)^2", "-x + cos(x)^2"},
        {"-cosh(x)^2 + sinh(x)^2", "-1"},
        {"(3 * sin(x)) / cos(x)", "3 * tan(x)"},
        {"(sin(x) * 3) / cos(x)", "3 * tan(x)"},
        {"3 / csc(u)",            "3 * sin(u)"},
        {"(x + 1) / tanh(x^2)",   "x * coth(x^2) + coth(x^2)"},
        {"(x + 1) / cosh(x^2)",   "x * sech(x^2) + sech(x^2)"},
        {"x^2 / sinh(x)",         "x^2 * csch(x)"},
        {"x^2 / csc(x)",          "x^2 * sin(x)"},
        {"x^2 / sec(x)",          "x^2 * cos(x)"},
        {"x^2 / cot(x)",          "x^2 * tan(x)"},
        {"x^2 / sin(x)",          "x^2 * csc(x)"},
        {"x^2 / cos(x)",          "x^2 * sec(x)"},
        {"x^2 / tan(x)",          "x^2 * cot(x)"},

        {"1 + tan(x)^2 + 3",        "sec(x)^2 + 3"},
        {"1 + cot(x)^2 + 3",        "csc(x)^2 + 3"},
        {"sin(x)^2 + 1 + cos(x)^2", "2"},
        {"1 + cot(x)^2",            "csc(x)^2"},
        {"sec(x)^2 + 3",            "sec(x)^2 + 3"},

        {"3sin(x)^2 + 3cos(x)^2", "3"},
        {"sin(x) / (3 * cos(x))", "tan(x) / 3"},
        {"(3 * sin(x)) / cos(x)", "3 * tan(x)"},
        {"sin(x) / (3 * cos(x) * (1 + x))", "tan(x) / (3x + 3)"},

        {"ln(abs(a * b * x * d + c * d)) / (a * b * d)",
         "ln(abs(a * b * x * d + c * d)) / (a * b * d)"},

        {"(cos(x_substitution) * x * (3 * x^(2 - 1) * 1 + 2 * 1 * 1)) "
         "/ (3 * x^2 + 2x)",
         "cos(x_substitution)"},

        // rational roots
        {"(6x^4 - 11x^3 + 8x^2 - 33x - 30) / (-x^2 - 3)",
         "(-3x - 2) * (2x - 5)"},

        {"(x^5 + x^4 - 4 * x^3 - 4 * x^2) / (x^5 + 4 * x^4 + 4 * x^3)",
         "(-(x + 1) * (-x + 2)) / (x * (x + 2))"},

        // must factor out the GCD here
        {"2 * (x^5 + x^4 - 4 * x^3 - 4 * x^2) / (x^5 + 4 * x^4 + 4 * x^3)",
         "(-2 * (x + 1) * (-x + 2)) / (x * (x + 2))"},

        {"4x^3 + 8x^2 - 33x - 30 - 2 * x^2 * (2x - 5)",
         "3 * (2x - 5) * (3x + 2)"},

        // distribute like a madman
        {"(x + 1) * x^2 + (x + 1)^2 - (x^3 + 2x^2 + 2x + 1)", "0"},
        {"(x + 1) * x^2 + (x + 1)^2 - (x^3 + 2x^2 + 2x + 3)", "-2"},
        {"(x^3 - 12x^2) / (x - 1)^2 + -42 * (1 / (x - 1)^2)"
         "- (x - 10 + (-21x - 32) * (1 / (x^2 - 2x + 1)))",
         "0"},

        // e stuff
        {"(e^e)^x / ln(e^e) - e^(e * x - 1)", "0"},

        // factor by difference of squares
        {"(x^2 - 4) / (x + 2)",  "-(-x + 2)"},

        // factor by difference of cubes
        //{"(x^3 - 8) / (x^2 + 2x + 4)", "x - 2"},
        //{"(x^3 - 8) / (x - 2)",        "(x^2 + 2x + 4)"},

        // weird quadratics
        {"(x^4 - 2x^2 - 8) / (x^2 - 4)", "((x - 2) * (-x^2 - 2)) / (-x + 2)"},
        {"(x^6 + 6x^3 + 5) / (x + 1)",
         "x * (x * (x^2 * (x - 1) + x + 5) - 5) + 5"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        testShrinkWithProgram(programs[i].program, programs[i].expected);
    }
}

typedef struct
{
    const char *program;
    const char *symbol;
    const char *expected;
} OperationProgram;

typedef struct
{
    const char *left;
    const char *right;
    bool expectedEqual;
} EqualityProgram;

typedef dcNode *(*FlatArithmeticOperation)(dcNode *_arithmetic,
                                           const char *_symbol);

typedef bool (*FlatArithmeticNodeOperation)(dcNode **_arithmetic,
                                            dcNode *_yeah);

static void testEqual(const OperationProgram *_program,
                      dcNode *_operated)
{
    // TODO: remove me
    dcError_assert(! dcFlatArithmetic_containsIdentifier
                   (_operated, "!leftHandSideLoL!"));
    dcTestUtilities_expectStringNodeEqual(_program->expected, _operated);
    dcNodeEvaluator_clearException(dcSystem_getCurrentNodeEvaluator(), DC_DEEP);
}

static void testOperation(const OperationProgram *_program,
                          FlatArithmeticOperation _operationFunction)
{
    dcLog_log(FLAT_ARITHMETIC_TEST_LOG, "Program: %s\n", _program->program);
    dcNode *flatArithmetic = createFlatArithmetic(_program->program);
    dcNode *newFlatArithmetic = _operationFunction(flatArithmetic, _program->symbol);
    testEqual(_program, newFlatArithmetic);
    dcNode_free(&newFlatArithmetic, DC_DEEP);
    dcNode_free(&flatArithmetic, DC_DEEP);
}

static void testDerive(void)
{
    const OperationProgram programs[] =
    {
        {"x^2 * e^x",          "x", "2 * x * e^x + x^2 * e^x"},
        {"-x^2 * e^x",         "x", "-2 * x * e^x - x^2 * e^x"},
        {"x^2 * -e^x",         "x", "-2 * x * e^x - x^2 * e^x"},

        {"a", "x", "0"},
        {"(1 + x) * x^2", "x", "3x^2 + 2x"},

        {"0", "x", "0"},
        {"a", "x", "0"},
        {"y", "x", "0"},
        {"x", "x", "1"},

        // add rule
        {"x + 1",              "x", "1"},
        {"x + x",              "x", "2"},
        {"x + x^2",            "x", "2x + 1"},
        // add rule backwards
        {"1 + x",              "x", "1"},
        {"x^2 + x",            "x", "2x + 1"},
        // add rule with three
        {"x + 1 + x^2",        "x", "2x + 1"},
        {"x + x^2 + x^3",      "x", "3x^2 + 2x + 1"},

        // subtract rule
        {"x - x^2",       "x", "-2x + 1"},
        {"x^3 - x^2",     "x", "3x^2 - 2x"},
        {"x^3 - x^2 - x", "x", "(3x + 1) * (x - 1)"},

        // product rule
        {"x^2 * e^x",          "x", "2 * x * e^x + x^2 * e^x"},
        {"x * z",              "x", "z"},
        {"x * y",              "x", "y"},
        {"(1 + x) * x^2",      "x", "3x^2 + 2x"},
        {"x * (1 + x)",        "x", "2x + 1"},
        {"x * (1 + x) * x^2",  "x", "4x^3 + 3x^2"},

        // power rule
        {"x^2",                "x", "2x"},
        {"x^3",                "x", "3x^2"},
        {"x^2 + x^3",          "x", "3x^2 + 2x"},
        {"x^2",                "y", "0"},
        {"(x * 2)^2 + 1",      "x", "8x"},
        {"(x * 2 * 3)^3 + 3y", "x", "648x^2"},
        {"(x * 2 * 3)^3 + 3y", "y", "3"},
        {"x^(3 * (2 + 4))",    "x", "18x^17"},
        {"e^x",                "x", "e^x"},
        {"(x + 3) * (x + 4)",  "x", "2x + 7"},
        {"e^(5x)",             "x", "5e^(5x)"},
        {"x^x",                "x", "x^x + ln(x) * x^x"},
        {"2^x",                "x", "2^x * ln(2)"},
        {"y^x",                "x", "y^x * ln(y)"},
        // general power rule
        {"x^x^x",
         "x",
         "x^(x^x + x - 1) + ln(x) * x^(x^x + x) + ln(x)^2 * x^(x^x + x)"},

        // quotient rule
        {"1 / x",              "x", "-1 / x^2"},
        {"2 / (x + 1)",        "x", "-2 / (x + 1)^2"},
        {"3 / x",              "x", "-3 / x^2"},
        {"(1 + x) / (2 + x)",  "x", "1 / (x + 2)^2"},
        {"(1 + 2x) / (2 + x)", "x", "3 / (x + 2)^2"},
        {"(4x - 2) / (x^2 + 1)",
         "x",
         "(-4x^2 + 4x + 4) / (x^2 + 1)^2"},
        {"(1 + x) / (2 + x) / (3 + x)",
         "x",
         "(-x^2 - 2x + 1) / (x^2 + 5x + 6)^2"},
        {"((1 + x) / (2 + x)) / (3 + x)",
         "x",
         "(-x^2 - 2x + 1) / (x^2 + 5x + 6)^2"},
        {"(1 + x) / ((2 + x) * (3 + x))",
         "x",
         "(-x^2 - 2x + 1) / (x^2 + 5x + 6)^2"},
        {NULL}
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs) - 1; i++)
    {
        testOperation(&programs[i], &dcFlatArithmetic_derive);
    }

    const OperationProgram byStringPrograms[] =
    {
        // absolute value
        {"abs(x)",      "x", "x / abs(x)"},
        {"abs(x^2)",    "x", "2x"},
        {"abs(sin(x))", "x", "(sin(x) * cos(x)) / abs(sin(x))"},

        // logarithms
        {"log(3, y)",          "x", "0"},
        {"log(3, x)",          "x", "1 / (x * ln(3))"},
        {"log(3, x^2)",        "x", "2 / (x * ln(3))"},
        {"ln(2)",              "x", "0"},
        {"ln(2x)",             "x", "1 / x"},
        {"ln(x^2)",            "x", "2 / x"},
        {"ln(x^3)",            "x", "3 / x"},
        {"ln(x^x)",            "x", "1 + ln(x)"},
        {"ln(x)",              "x", "1 / x"},

        // trigonometric

        // sin
        {"sin(x)",    "x", "cos(x)"},
        {"sin(x^2)",  "x", "2 * x * cos(x^2)"},

        // cos
        {"cos(x)",      "x", "-sin(x)"},
        {"cos(x^2)",    "x", "-2 * x * sin(x^2)"},

        // tan
        {"tan(x)",   "x", "sec(x)^2"},
        {"tan(x^2)", "x", "2 * x * sec(x^2)^2"},

        // csc
        {"csc(x)",   "x", "-csc(x) * cot(x)"},
        {"csc(x^2)", "x", "-2 * x * csc(x^2) * cot(x^2)"},

        // cot
        {"cot(x)",   "x", "-csc(x)^2"},
        {"cot(x^2)", "x", "-2 * x * csc(x^2)^2"},

        // sec
        {"sec(x)",   "x", "sec(x) * tan(x)"},
        {"sec(x^2)", "x", "2 * x * sec(x^2) * tan(x^2)"},

        //
        // inverse trigonometric
        //
        {"asin(x)",   "x", "1 / (-x^2 + 1)^0.500000"},
        {"asin(x^2)", "x", "(2x) / (-x^4 + 1)^0.500000"},

        {"acos(x)",   "x", "-1 / (-x^2 + 1)^0.500000"},
        {"acos(x^2)", "x", "(-2x) / (-x^4 + 1)^0.500000"},

        {"atan(x)",   "x", "1 / (x^2 + 1)"},
        {"atan(x^2)", "x", "(2x) / (x^4 + 1)"},

        {"acsc(x)",   "x", "-1 / ((x^2 - 1)^0.500000 * x)"},
        {"acsc(x^2)", "x", "-2 / ((x^4 - 1)^0.500000 * x)"},

        {"asec(x)",   "x", "1 / ((x^2 - 1)^0.500000 * x)"},
        {"asec(x^2)", "x", "2 / ((x^4 - 1)^0.500000 * x)"},

        {"acot(x)",   "x", "-1 / (x^2 + 1)"},
        {"acot(x^2)", "x", "(-2x) / (x^4 + 1)"},

        //
        // hyperbolic functions
        //

        // sinh
        {"sinh(x)",     "x", "cosh(x)"},
        {"sinh(x^2)",   "x", "2 * x * cosh(x^2)"},
        {"sinh(x^2)^2", "x", "4 * x * cosh(x^2) * sinh(x^2)"},

        // asinh
        {"asinh(x)",     "x", "1 / (x^2 + 1)^0.500000"},
        {"asinh(x^2)",   "x", "(2x) / (x^4 + 1)^0.500000"},
        {"asinh(x^2)^2", "x", "(4 * x * asinh(x^2)) / (x^4 + 1)^0.500000"},

        // cosh
        {"cosh(x)",   "x", "sinh(x)"},
        {"cosh(x^2)", "x", "2 * x * sinh(x^2)"},

        // acosh
        {"acosh(x)",     "x", "1 / ((x - 1)^0.500000 * (x + 1)^0.500000)"},
        {"acosh(x^2)",
         "x",
         "(2x) / ((x^2 - 1)^0.500000 * (x^2 + 1)^0.500000)"},

        // tanh
        {"tanh(x)",   "x", "sech(x)^2"},
        {"tanh(x^2)", "x", "2 * x * sech(x^2)^2"},

        // atanh
        {"atanh(x)",     "x", "1 / (-x^2 + 1)"},
        {"atanh(x^2)",   "x", "(2x) / (-x^4 + 1)"},
        {"atanh(x^2)^2", "x", "(4 * x * atanh(x^2)) / (-x^4 + 1)"},

        // csch
        {"csch(x)",   "x", "-coth(x) * csch(x)"},
        {"csch(x^2)", "x", "-2 * x * coth(x^2) * csch(x^2)"},

        // acsch
        {"acsch(x)",     "x", "-1 / ((x^2 + 1)^0.500000 * x)"},
        {"acsch(x^2)",   "x", "-2 / ((x^4 + 1)^0.500000 * x)"},

        // sech
        {"sech(x)",   "x", "-tanh(x) * sech(x)"},
        {"sech(x^2)", "x", "-2 * x * tanh(x^2) * sech(x^2)"},

        // asech
        {"asech(x)",
         "x",
         "(-x + 1)^0.500000 / (x^2 * (x + 1)^0.500000 - (x + 1)^0.500000 * x)"},
        {"asech(x^2)",
         "x",
         "(2 * (-x^2 + 1)^0.500000) "
         "/ (x^3 * (x^2 + 1)^0.500000 - (x^2 + 1)^0.500000 * x)"},

        // coth
        {"coth(x)",     "x", "-csch(x)^2"},
        {"coth(x^2)",   "x", "-2 * x * csch(x^2)^2"},
        {"coth(x^2)^2", "x", "-4 * x * csch(x^2)^2 * coth(x^2)"},

        // acoth
        {"acoth(x)",     "x", "1 / (-x^2 + 1)"},
        {"acoth(x^2)",   "x", "(2x) / (-x^4 + 1)"},

        {NULL}
    };

    for (i = 0; i < dcTaffy_countOf(byStringPrograms) - 1; i++)
    {
        testOperation(&byStringPrograms[i], &dcFlatArithmetic_derive);
    }
}

static void testIntegrate(void)
{
    // need:
    // (ax + b)^n
    // f'(x)*e^(f(x))

    const OperationProgram programs[] =
    {
        // number
        {"0",   "x", "0"},
        {"1",   "x", "x"},
        {"2",   "x", "2x"},
        {"i",   "x", "i * x"},
        {"i^2", "x", "-x"},

        // identifier
        {"a", "x", "a * x"},

        // exponential functions
        {"e^x",       "x", "e^x"},
        {"e^x",       "y", "e^x * y"},
        {"x",         "x", "0.500000 * x^2"},
        {"2^x",       "x", "2^x / ln(2)"},
        {"x^a",       "x", "x^(a + 1) / (a + 1)"},
        {"x^2 * e^x", "x", "x^2 * e^x + 2e^x - 2 * x * e^x"},

        // programs of non-the-symbol
        {"log(x, y)",   "z", "(ln(y) * z) / ln(x)"},
        {"log(x, y^2)", "z", "(ln(y^2) * z) / ln(x)"},

        {"(e^e)^x", "x", "e^(e * x - 1)"},
        {"x^(e^e)", "x", "x^(e^e + 1) / (e^e + 1)"},

        // unfindable
        {"x^x",      "x", "NULL??"},
        {"e^(e^x)",  "x", "NULL??"},
        {"cos(x^2)", "x", "NULL??"},

        // one over x
        {"1 / x", "x", "ln(abs(x))"},
        {"3 / x", "x", "3 * ln(abs(x))"},
        {"1 / x", "y", "y / x"},

        // one over something more complicated
        {"x^(-3)",  "x", "-1 / (4x^4)"},
        {"1 / x^3", "x", "-1 / (4x^4)"},

        // division with expansion
        {"x^4 / x^3 + 7 / x^3", "x", "(2.000000x^6 - 7) / (4x^4)"},
        {"(x^4 + 7) / x^3",     "x", "(2.000000x^6 - 7) / (4x^4)"},

        // logarithms
        {"ln(x)",       "x", "ln(x) * x - x"},
        {"ln(y)",       "x", "ln(y) * x"},
        {"ln(x)^2",     "x", "2x + ln(x)^2 * x - 2 * x^2 * ln(x)"},
        {"ln(a*x + b)",
         "x",
         "(a * x * ln(a * x + b) + b * ln(a * x + b) - a * x - b) / a"},

        {"ln(a*b*x + c)",
         "x",
         "(a * b * x * ln(a * b * x + c) + c "
         "* ln(a * b * x + c) - a * b * x - c) / (a * b)"},
        {"ln(x*a + b)",
         "x",
         "(x * a * ln(x * a + b) + b * ln(x * a + b) - x * a - b) / a"},
        {"ln(b + a*x)",
         "x",
         "(a * x * ln(a * x + b) + b * ln(a * x + b) - a * x - b) / a"},
        {"ln(b + x*a)",
         "x",
         "(x * a * ln(x * a + b) + b * ln(x * a + b) - x * a - b) / a"},
        {"ln(a*x)",     "x", "ln(a * x) * x - x"},
        {"ln(x*a)",     "x", "ln(x * a) * x - x"},
        {"log(a, x)",   "x", "(ln(x) * x - x) / ln(a)"},
        {"log(a, b*x)", "x", "(ln(b * x) * x - x) / ln(a)"},
        {"log(a, x*b)", "x", "(ln(x * b) * x - x) / ln(a)"},
        {"log(a, b * x + c)",
         "x",
         "(b * x * ln(b * x + c) "
         "+ c * ln(b * x + c) - c - b * x) / (ln(a) * b)"},
        {"log(3, x)", "x", "(ln(x) * x - x) / ln(3)"},

        // exponential functions
        {"a^x", "x", "a^x / ln(a)"},
        {"2^x", "x", "2^x / ln(2)"},

        {"1 / sin(x)", "x", "ln(sin(x / 2)) - ln(cos(x / 2))"},
        {"2 / sin(x)", "x", "2 * ln(sin(x / 2)) - 2 * ln(cos(x / 2))"},

        // sqrt
        {"sqrt(x)", "x", "(2x^(3 / 2)) / 3"},

        // abs
        {"abs(x)",  "x", "0.500000 * x^2 * sgn(x)"},
        {"abs(-x)", "x", "-0.500000 * x^2 * sgn(-x)"},

        // parabola with y
        {"x^3 + y", "x", "(x^4 + 4 * y * x) / 4"},

        {"x^y + y", "x", "(x^(y + 1) + y^2 * x + y * x) / (y + 1)"},

        // reduction formulae
        {"sin(x)^3", "x", "(-sin(x)^2 * cos(x) - 2 * cos(x)) / 3"},
        {"cos(x)^3", "x", "(cos(x)^2 * sin(x) + 2 * sin(x)) / 3"},

        // uses polynomial division
        {"x / (b * x * a + c)",
         "x",
         "(x * a * b - ln(abs(b * x * a + c)) * c) / (a^2 * b^2)"},

        {"(a * b * x) / (a * b * x + c)",
         "x",
         "(x * b * a - ln(abs(a * b * x + c)) * c) / (b * a)"},

        {"(-c / (a * b)) * (1 / (a * b * x + c))",
         "x",
         "(-ln(abs(a * b * x + c)) * c) / (b^2 * a^2)"},

        {"1 / (x * a * b)", "x", "ln(abs(x)) / (a * b)"},
        {"1 / (x * a * b + c)", "x", "ln(abs(x * a * b + c)) / (a * b)"},

        {"(-c / (a * b)) * (1 / (a * b * x + c))",
         "x",
         "(-ln(abs(a * b * x + c)) * c) / (b^2 * a^2)"},

        {"ln(x*a)", "x", "ln(x * a) * x - x"},

        // unfindable
        {"x^x",                     "x", "NULL??"},
        {"3 * x^2 * e^(x^3 + x^2)", "x", "NULL??"},

        {"(2x + 5)^3", "x", "(2x + 5)^4 / 8"},

        // trigonometric functions
        {"sin(x)",  "x",  "-cos(x)"},
        {"sin(4x)", "x", "-cos(4x) / 4"},
        {"cos(x)",  "x", "sin(x)"},
        {"cos(4x)", "x", "sin(4x) / 4"},
        {"tan(x)",  "x", "-ln(abs(sec(x)))"},
        {"tan(4x)", "x", "-ln(abs(sec(4x))) / 4"},
        {"sec(x)",
         "x",
         "ln(sin(x / 2) + cos(x / 2)) "
         "- ln(cos(x / 2) - sin(x / 2))"},
        {"sec(4x)",
         "x",
         "(ln(sin(2x) + cos(2x)) - ln(cos(2x) - sin(2x))) / 4"},
        {"csc(x)",  "x", "ln(sin(x / 2)) - ln(cos(x / 2))"},
        {"csc(4x)",
         "x",
         "(ln(sin(2x)) - ln(cos(2x))) / 4"},
        {"cot(x)",  "x", "ln(sin(x))"},
        {"cot(4x)", "x", "ln(sin(4x)) / 4"},
        {"sec(x) * tan(x)",  "x", "sec(x)"},
        {"csc(x) * cot(x)", "x", "-csc(x)"},
        {"sin(x)^2", "x", "0.500000 * x - 0.500000 * sin(x) * cos(x)"},
        {"cos(x)^2", "x", "0.500000 * x + 0.500000 * sin(x) * cos(x)"},
        {"sec(x)^2", "x", "tan(x)"},
        {"sec(x)^3", "x", "(sec(x) * tan(x) + ln(sec(x) + tan(x))) / 2"},

        {"2* cos(x)", "x", "2 * sin(x)"},

        // inverse trigonemtric functions
        {"asin(x)", "x", "x * asin(x) + (-x^2 + 1)^0.500000"},
        {"acos(x)", "x", "x * acos(x) - (-x^2 + 1)^0.500000"},
        {"atan(x)", "x", "x * atan(x) - 0.500000 * ln(abs(x^2 + 1))"},

        // hyperbolic trigonometric functions
        // sinh
        {"sinh(x)",     "x", "cosh(x)"},
        {"sinh(a * x)", "x", "cosh(a * x) / a"},
        {"sinh(a * b * x + c)", "x", "cosh(a * b * x + c) / (a * b)"},
        // cosh
        {"cosh(x)",     "x", "sinh(x)"},
        {"cosh(a * x)", "x", "sinh(a * x) / a"},
        // tanh
        {"tanh(x)",     "x", "ln(cosh(x))"},
        {"tanh(a * x)", "x", "ln(cosh(a * x)) / a"},
        // coth
        {"coth(x)",     "x", "ln(sinh(x))"},
        {"coth(a * x)", "x", "ln(sinh(a * x)) / a"},
        // sech
        {"sech(x)",     "x", "2 * atan(tanh(x / 2))"},
        {"sech(a * x)", "x", "(2 * atan(tanh((a * x) / 2))) / a"},
        // csch
        {"csch(x)",     "x", "ln(tanh(x / 2))"},
        {"csch(a * x)", "x", "ln(tanh((a * x) / 2)) / a"},

        // inverse hyperbolic trigonometric functions
        // asinh
        {"asinh(x)",   "x", "x * asinh(x) - sqrt(x^2 + 1)"},
        {"asinh(a*x)",
         "x",
         "(a * x * asinh(a * x) - sqrt((a * x)^2 + 1)) / a"},
        // asosh
        {"acosh(x)",   "x", "x * acosh(x) - sqrt(x - 1) * sqrt(x + 1)"},
        {"acosh(a*x)",
         "x",
         "(a * x * acosh(a * x) - sqrt(a * x - 1) * sqrt(a * x + 1)) / a"},
        // atanh
        {"atanh(x)",     "x", "(ln(-x^2 + 1) + 2 * x * atanh(x)) / 2"},
        {"atanh(a * x)",
         "x",
         "(ln(-(a * x)^2 + 1) + 2 * a * x * atanh(a * x)) / (2a)"},
        // acoth
        {"acoth(x)",     "x", "(ln(-x^2 + 1) + 2 * x * acoth(x)) / 2"},
        {"acoth(a * x)",
         "x",
         "(ln(-(a * x)^2 + 1) + 2 * a * x * acoth(a * x)) / (2a)"},

        {"asech(x)",
         "x",
         "(-x^2 * asech(x) + x * asech(x) + 2 "
         "* sqrt((-x + 1) / (x + 1)) "
         "* sqrt(-x^2 + 1) * asin(sqrt(x + 1) / sqrt(2))) / (-x + 1)"},

        {"asech(a * x)",
         "x",
         "(2 * sqrt((-a * x + 1) / (a * x + 1)) * sqrt(-(a * x)^2 + 1) * asin(sqrt(a * x + 1) / sqrt(2)) - (a * x)^2 * asech(a * x) + a * x * asech(a * x)) / (-a^2 * x + a)"},

        {"acsch(x)",
         "x",
         "(sqrt((x^2 + 1) / x^2) * asinh(x) * x "
         "+ acsch(x) * sqrt(x^2 + 1) * x) / sqrt(x^2 + 1)"},
        {"acsch(a * x)",
         "x",
         "(sqrt(((a * x)^2 + 1) / (a * x)^2) * asinh(a * x) * x "
         "+ acsch(a * x) * sqrt((a * x)^2 + 1) * x) / sqrt((a * x)^2 + 1)"},

        // sums
        {"x + sin(x)",          "x", "0.500000 * x^2 - cos(x)"},
        {"x + cos(x)",          "x", "0.500000 * x^2 + sin(x)"},
        {"x + sin(x) + cos(x)", "x", "0.500000 * x^2 + sin(x) - cos(x)"},

        // differences
        {"x - sin(x)",          "x", "0.500000 * x^2 + cos(x)"},
        {"x - cos(x)",          "x", "0.500000 * x^2 - sin(x)"},
        {"x - sin(x) - cos(x)", "x", "0.500000 * x^2 + cos(x) - sin(x)"},

        // sums and differences
        {"x + sin(x) - cos(x)", "x", "0.500000 * x^2 - cos(x) - sin(x)"},

        {"x^i",            "x", "x^(1 + i) / (1 + i)"},
        {"x^(-1 + i) * i", "x", "x^i"},

        // use identity
        {"sin(x) * cos(3x)",
         "x",
         "0.250000 * cos(2x) - 0.125000 * cos(4x)"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        testOperation(&programs[i], &dcFlatArithmetic_integrate);
    }
}

static void testRepeatedDerivations(void)
{
    const char *program = "sin(x)";
    dcNode *arithmetic = createFlatArithmetic(program);
    dcNode *original = dcNode_copy(arithmetic, DC_DEEP);

    // cos(x)
    dcNode *derived1 = dcFlatArithmetic_derive(arithmetic, "x");
    // -sin(x)
    dcNode *derived2 = dcFlatArithmetic_derive(derived1, "x");
    // -cos(x)
    dcNode *derived3 = dcFlatArithmetic_derive(derived2, "x");
    // sin(x)
    dcNode *derived4 = dcFlatArithmetic_derive(derived3, "x");

    char *finalDisplay = dcNode_synchronizedDisplay(derived4);
    char *originalDisplay = dcNode_synchronizedDisplay(original);

    dcTestUtilities_assert(strcmp(finalDisplay, originalDisplay) == 0);

    dcNode_free(&derived1, DC_DEEP);
    dcNode_free(&derived2, DC_DEEP);
    dcNode_free(&derived3, DC_DEEP);
    dcNode_free(&derived4, DC_DEEP);
    dcNode_free(&arithmetic, DC_DEEP);
    dcMemory_free(finalDisplay);
    dcMemory_free(originalDisplay);
    dcNode_free(&original, DC_DEEP);
}

static void testAddComplexNumbers(void)
{
    {
        dcNode *flatty = dcFlatArithmetic_combine
            (createFlatArithmetic("1 + i + x + 2 + 3i"), NULL);
        assertNodeContents(flatty,
                           TAFFY_ADD,
                           2,
                           dcComplexNumberClass_createObject
                           (dcComplexNumber_createFromInt32s(3, 4)),
                           dcIdentifier_createNode("x", NO_FLAGS),
                           NULL);
        dcNode_free(&flatty, DC_DEEP);
    }

    assertPoppedInto("i + i", "2i");
    assertPoppedInto("i + i + i", "3i");
}

static void testSubtractComplexNumbers(void)
{
    assertPoppedInto("i - i",                 "0");
    assertPoppedInto("i - i - i",             "-i");
    assertPoppedInto("3 - (3 + 3i)",          "-3i");
    assertPoppedInto("3 - (3 + 3i) + 1 + 1i", "(1 - 2i)");
}

static void testMultiplyComplexNumbers(void)
{
    assertPoppedInto("i * i",       "-1");
    assertPoppedInto("(1 + i) * i", "(-1 + i)");
}

static void testDivideComplexNumbers(void)
{
    const Program programs[] =
    {
        {"(4 + 2i) / (3 - i)", "(1 + i)"},
        {"1 / (3 - i)", "(0.3 + 0.1i)"},
        {"(3 - i) / 2", "(1.5 - 0.5i)"}
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        assertPoppedInto(programs[i].program,
                         programs[i].expected);
    }
}

static void testPrettyPrintInto(const char *_flatArithmeticProgram,
                                const char *_wanted)
{
    dcNode *flatArithmetic = (createFlatArithmetic
                              (_flatArithmeticProgram));
    dcCharacterGraph *graph;
    dcError_assert(dcFlatArithmetic_prettyPrint
                   (CAST_FLAT_ARITHMETIC(flatArithmetic),
                    &graph)
                   == TAFFY_SUCCESS);
    dcString *string = dcCharacterGraph_convertToString(graph);

    if (strcmp(string->string, _wanted) != 0)
    {
        fprintf(stderr,
                "assertion failure.\n"
                "wanted:\n"
                "'%s'\n"
                "but got:\n"
                "'%s'\n",
                _wanted,
                string->string);
        dcError_assert(false);
    }

    dcString_free(&string, DC_DEEP);
    dcCharacterGraph_free(&graph);
    dcNode_free(&flatArithmetic, DC_DEEP);
}

static void testPrettyPrint(void)
{
    // raises
    testPrettyPrintInto("x + 2",
                        "x + 2");
    testPrettyPrintInto("x^2",
                        " 2\n"
                        "x ");
    testPrettyPrintInto("x^2^x",
                        "  x\n"
                        " 2 \n"
                        "x  ");
    testPrettyPrintInto("x^2^x^3",
                        "   3\n"
                        "  x \n"
                        " 2  \n"
                        "x   ");
    testPrettyPrintInto("x^(2+x)",
                        " (2 + x)\n"
                        "x       ");
    testPrettyPrintInto("x^(2+x)^(3 + y + z)^2",
                        "                   2\n"
                        "        (3 + y + z) \n"
                        " (2 + x)            \n"
                        "x                   ");

    // divides
    testPrettyPrintInto("x/2",
                        " x \n"
                        "---\n"
                        " 2 ");
    testPrettyPrintInto("x/(longy)",
                        "   x   \n"
                        "-------\n"
                        " longy ");
    testPrettyPrintInto("x/(x + 3)",
                        "   x   \n"
                        "-------\n"
                        " x + 3 ");
    testPrettyPrintInto("x/(x + 3)/(x + y + z)",
                        "     x     \n"
                        "  -------  \n"
                        "   x + 3   \n"
                        "-----------\n"
                        " x + y + z ");

    // divide and raise
    testPrettyPrintInto("x/(2^x^y)",
                        "  x  \n"
                        "-----\n"
                        "   y \n"
                        "  x  \n"
                        " 2   ");
}

static void testCompareNode(void)
{
    const EqualityProgram programs[] =
    {
        {"e^(3x + 2x)", "e^(3x + 2x)", true},
        {"e^(x^3 + x^2)", "e^(x^3 + x^2)", true}
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcNode *left = createFlatArithmetic(programs[i].left);
        dcNode *right = createFlatArithmetic(programs[i].right);
        dcTaffyOperator comparison;
        dcResult resulty = dcFlatArithmetic_compareNode(left,
                                                        right,
                                                        &comparison);
        assert(resulty == TAFFY_SUCCESS);

        if (programs[i].expectedEqual)
        {
            assert(comparison == TAFFY_EQUALS);
        }
        else
        {
            assert(comparison != TAFFY_EQUALS);
        }

        dcHash *cache = dcHash_create();
        dcNode *leftCopy = dcNode_copy(left, DC_DEEP);
        dcHash_setValue(cache, leftCopy, right);
        dcNode *candidate = NULL;
        assert(dcHash_getValue(cache, left, &candidate) == TAFFY_SUCCESS);
        dcNode_free(&left, DC_DEEP);
        dcHash_free(&cache, DC_DEEP);
    }
}

static void testIntegrateWithSubstitution(void)
{
    const OperationProgram programs[] =
    {
        {"e^(5x)",          "x", "e^(5x) / 5"},
        {"x * sqrt(x + 1)",
         "x",
         "(6 * (x + 1)^(5 / 2) - 10 * (x + 1)^(3 / 2)) / 15"},
        {"2x * asdf(x^2)",              "x", "NULL??"},
        {"asdf(x)",                     "x", "NULL??"},
        {"asdf(x^2)",                   "x", "NULL??"},
        {"2^ln(x) / x",                 "x", "2^ln(x) / ln(2)"},
        {"(4x^3) / (x^4 + 7)",          "x", "ln(abs(x^4 + 7))"},
        {"3/(x * ln(x))",               "x", "3 * ln(abs(ln(x)))"},
        {"(4x^3) / (x^4 + 7)",          "x", "ln(abs(x^4 + 7))"},
        //{"(3x^2 + 2x) * e^(x^3 + x^2)", "x", "e^(x^3 + x^2)"},
        {"x * cos(0.5 * x^2)",          "x", "sin(0.5 * x^2)"},
        {"cos(0.5 * x)",                "x", "sin(0.5 * x) / 0.5"},
        // mixups: 0.5 * x * cos(x^2)
        {"0.5 * x * cos(x^2)",          "x", "0.25 * sin(x^2)"},
        {"x * cos(x^2) * 0.5",          "x", "0.25 * sin(x^2)"},
        // mixups: x * cos(x^2)
        {"x * cos(x^2)",                "x", "sin(x^2) / 2"},
        {"cos(x^2) * x",                "x", "sin(x^2) / 2"},
        {"x * ln(x^4) * x * x",
         "x",
         "(ln(x^4) * x^4 - x^4) / 4"},
        {"x^3 * 2^(x^4)",
         "x",
         "2^(x^4) / (4 * ln(2))"},
        {"sqrt(4x + 4)",
         "x",
         "(4 * (x + 1)^(3 / 2)) / 3"},
        {"x / (1 + x^2)",   "x", "ln(abs(x^2 + 1)) / 2"},
        {"x^4 / (1 + x^5)", "x", "ln(abs(x^5 + 1)) / 5"},
        {"(2x + 5)^3",      "x", "(2x + 5)^4 / 8"},
        {"(3x^2 + 2x) * cos(x^3 + x^2)", "x", "sin(x^3 + x^2)"},
        {"(3x^2 + 2x) * e^(x^3 + x^2)", "x", "e^(x^3 + x^2)"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        testOperation(&programs[i], &dcFlatArithmetic_integrate);
    }
}

static void testIntegrateByParts(void)
{
    const OperationProgram programs[] =
    {
        {"e^x * sin(x)",    "x", "(sin(x) * e^x - cos(x) * e^x) / 2"},
        {"e^x * cos(x)",    "x", "(sin(x) * e^x + cos(x) * e^x) / 2"},
        {"sin(x) * e^x",    "x", "(sin(x) * e^x - cos(x) * e^x) / 2"},
        {"cos(x) * e^x",    "x", "(cos(x) * e^x + sin(x) * e^x) / 2"},

        {"x * cos(x)",      "x", "x * sin(x) + cos(x)"},
        {"cos(x) * x",      "x", "x * sin(x) + cos(x)"},
        {"1 / (x * ln(x))", "x", "ln(abs(ln(x)))"},
        {"ln(x * e^e)",     "x", "ln(x * e^e) * x - x"},

        {"x * (x + 1)^0.5",
         "x",
         "(30 * x * (x + 1)^1.5 - 20 * (x + 1)^1.5) / 75"},
        {"x * e^(5 * x)",
         "x",
         "(5 * x * e^(5x) - e^(5x)) / 25"},

        {"x * ln(x)",
         "x",
         "0.500000 * ln(x) * x^2 - 0.250000000000 * x^2"},

        {"ln(x) * x",
         "x",
         "0.500000 * ln(x) * x^2 - 0.250000000000 * x^2"},

        {"x * 0.500000 * (1 - cos(2x))",
         "x",
         "0.250000000000 * x^2 - 0.250000 * sin(2x) * x - 0.125000 * cos(2x)"},
        // "- 0.250000 * sin(2x) * x"},

        {"0.500000 * (1 - cos(2x)) * x",
         "x",
         "0.250000000000 * x^2 - 0.250000 * sin(2x) * x - 0.125000 * cos(2x)"},

        {"x * sin(x) * tan(x) * cos(x)",
         "x",
         "0.250000000000 * x^2 - 0.500000 "
         "* sin(x) * cos(x) * x - 0.125000000000 * cos(2x)"},

        {"x^3 * e^x^2", "x", "(x^2 * e^(x^2) - e^(x^2)) / 2"},
        {"e^x^2 * x^3", "x", "(x^2 * e^(x^2) - e^(x^2)) / 2"},

        {"cos(2x) * x", "x", "x * (sin(2x) / 2) - -cos(2x) / 4"},
        {"x * cos(2x)", "x", "(2 * sin(2x) * x + cos(2x)) / 4"},

        {"x * cos(2x) * 0.5",
         "x",
         "0.250 * sin(2x) * x + 0.125 * cos(2x)"},

        {"cos(2x) * 0.5 * x",
         "x",
         "0.250 * sin(2x) * x + 0.125 * cos(2x)"},

        {"x * sin(x)^2",
         "x",
         "0.250000000000 * x^2 - 0.500000 * sin(x) "
         "* cos(x) * x - 0.125000000000 * cos(2x)"},

        {"x * sin(x)^2 * cos(x) * tan(x)",
         "x",
         "(-6 * cos(x) * x + 6 * sin(x) - 3 * cos(x) "
         "* x * sin(x)^2 + sin(x)^3) / 9"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TICK_LOG, ".");

        testOperation(&programs[i], &dcFlatArithmetic_integrate);
    }
}

static void testFindIdentifier(void)
{
    struct
    {
        const char *program;
        const char *identifier;
        bool findIdentifier;
    } programs[] =
    {
        {"x",                        "x",              true},
        {"x",                        "y",              false},
        {"x / (2 * x)",              "x",              true},
        {"x / (2 * x)",              "y",              false},
        {"cos(x)",                   "x",              true},
        {"cos(y)",                   "y",              true},
        {"cos(y)",                   "x",              false},
        {"cos(x^2)",                 "x",              true},
        {"cos(3 + x^2)",             "x",              true},
        {"cos(3 + y^2)",             "x",              false},
        {"1 / cos(x)",               "x",              true},
        {"1 / (3 + y + z + cos(x))", "x",              true},
        {"ln(abs(x_substitution))",  "x_substitution", true},
        {"f(a, x)",                  "x",              true},
        {"f(a, b, x)",               "x",              true},
        {"sin(x)^2",                 "x",              true},
        {"sin(e^x)^3",               "x",              true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcNode *arithmetic = (createFlatArithmetic
                              (programs[i].program));
        bool findIdentifier = dcFlatArithmetic_findIdentifier
            (&arithmetic, programs[i].identifier, NULL);
        dcError_assert(findIdentifier == programs[i].findIdentifier);
        dcNode_free(&arithmetic, DC_DEEP);
    }
}

static void testFindAndReplaceIdentifier(void)
{
    struct
    {
        const char *program;
        const char *identifier;
        const char *replacement;
        const char *expectedResult;
    } tests[] =
    {
        {"x",           "x", "x^2",   "x^2"},
        {"x",           "y", "x^2",   NULL},
        {"x + x",       "x", "x^2",   "x^2 + x^2"},
        {"f(x)",        "x", "g(x)",  "f(g(x))"},
        {"f(g(x))",     "x", "e^x",   "f(g(e^x))"},
        {"x + f(x)",    "x", "g",     "g + f(g)"},
        {"x^2 + f(x)",  "x", "g",     "g^2 + f(g)"},
        {"x / y",       "y", "x^2",   "x / x^2"},
        {"x + (y * z)", "z", "x % 5", "x + y * (x % 5)"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: %s | identifier: %s\n",
                  tests[i].program,
                  tests[i].identifier);

        dcNode *arithmetic = createFlatArithmetic(tests[i].program);
        dcNode *replacement = createFlatArithmetic(tests[i].replacement);
        bool findIdentifier = dcFlatArithmetic_findIdentifier
            (&arithmetic, tests[i].identifier, replacement);
        assert(findIdentifier == (tests[i].expectedResult != NULL));
        char *display = dcNode_synchronizedDisplay(arithmetic);

        if (tests[i].expectedResult == NULL)
        {
            dcTestUtilities_expectStringEqual(tests[i].program, display);
        }
        else
        {
            dcTestUtilities_expectStringEqual(tests[i].expectedResult, display);
        }

        dcMemory_free(display);
        dcNode_free(&replacement, DC_DEEP);
        dcNode_free(&arithmetic, DC_DEEP);
    }
}

static void testSingleTestWithProgram(const SingleTest *_test,
                                      dcFlatArithmeticOperation _operation,
                                      bool _matchByString,
                                      dcNode *_program)
{
    dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
              "Program: %s | expected: %s | expected modified: %s\n",
              _test->program,
              _test->expectedResult,
              BOOL_TO_STRING(_test->expectedModified));

    bool modified = false;

    _program = _operation(_program, &modified);

    //printf("after: %s\n", dcFlatArithmetic_displayReal(program));

    if (_matchByString)
    {
        char *display = dcNode_synchronizedDisplay(_program);
        dcTestUtilities_expectStringEqual(_test->expectedResult, display);
        dcMemory_free(display);
    }
    else
    {
        dcNode *expected = createFlatArithmetic(_test->expectedResult);
        dcTestUtilities_expectEqual(&_program, &expected);
        dcNode_free(&expected, DC_DEEP);
    }

    if (modified != _test->expectedModified)
    {
        fprintf(stderr,
                "\n\nExpected modified: %s but got: %s, for program: %s\n",
                BOOL_TO_STRING(_test->expectedModified),
                BOOL_TO_STRING(modified),
                _test->program);
        //assert(false);
    }

    dcNode_free(&_program, DC_DEEP);
}

static void testSingleTest(const SingleTest *_test,
                           dcFlatArithmeticOperation _operation,
                           bool _matchByString)
{
    dcNode *program = createFlatArithmetic(_test->program);

    testSingleTestWithProgram(_test,
                              _operation,
                              _matchByString,
                              program);
}

static void testConvert(void)
{
    const SingleTest tests[] =
    {
        {"0 - a",                          "-a",                    true},
        {"0 - a - b",                      "-a - b",                true},
        {"0 - (a - b)",                    "-(a - b)",              true},
        {"a / b / c / d",                  "(a / (b * c * d))",     true},
        {"x / (y / z) / w",                "(x / ((y / z) * w))",   true},
        {"abs(x^2)",                       "x^2",                   true},
        {"1 / abs(x^2)",                   "1 / x^2",               true},
        {"sin(x) * cos(x) * tan(x)",       "sin(x)^2 * ()",         true},
        {"x * sin(x) * cos(x) * tan(x)",   "sin(x)^2 * (x)",        true},
        {"sin(x^2) * cos(x^2) * tan(x^2)", "sin(x^2)^2 * ()",       true},
        {"sin(x)^2 * cos(x) * tan(x)",     "sin(x)^2 * (sin(x)^1)", true},
        {"x * sin(x) * 3 * cos(x) * tan(x)", "sin(x)^2 * (x * 3)",  true},
        {"abs(sin(x))^2",                  "sin(x)^2",              true},
        {"abs(sin(x))^4",                  "sin(x)^4",              true},
        {"sin(x^2) * cos(x^2) * tan(x^3)",
         "sin(x^2) * cos(x^2) * tan(x^3)",
         false},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i], &dcFlatArithmetic_convert, true);
    }
}

static void testExpandDivide(void)
{
    const SingleTest programs[] =
    {
        {"x",                   "x",                             false},
        {"3",                   "3",                             false},
        {"3 + x",               "3 + x",                         false},
        {"3 * x",               "3x",                            false},
        {"3^x",                 "3^x",                           false},
        {"(3 + x) / y",         "3 / y + x / y",                 true},
        {"(3 - x) / y",         "3 / y - x / y",                 true},
        {"(3 - (x + 2)) / y",   "3 / y - (x + 2) / y",           true},
        {"(3 + (x - 2)) / y",   "3 / y + (x - 2) / y",           true},
        {"(x + y) / (z^3 + 2)", "x / (z^3 + 2) + y / (z^3 + 2)", true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        testSingleTest(&programs[i], &dcFlatArithmetic_expandDivide, true);
    }
}

static void testConvertDivideToMultiply(void)
{
    const SingleTest programs[] =
    {
        {"x",                       "x",                               false},
        {"x + y",                   "x + y",                           false},
        {"1 / x",                   "1 / x",                           true},
        {"1 / 1",                   "1 / 1",                           true},
        {"cos(x) / (3 * sin(x))",   "cos(x) * (1 / 3) * (1 / sin(x))", true},
        {"(3 * cos(x)) / sin(x)",   "(3 * cos(x)) * (1 / sin(x))",     true},
        {"1 / x / y",               "(1 / x) * (1 / y)",               true},
        {"(1 / x) * (1 / y)",       "(1 / x) * (1 / y)",       true}, // again
        {"1 / (x * y)",             "(1 / x) * (1 / y)",       true},
        {"x / y",                   "x * (1 / y)",             true},
        {"x * (1 / y)",             "x * (1 / y)",             true}, // again
        {"x / y / z",               "(x * (1 / y)) * (1 / z)", true},
        {"x * (1 / y) * (1 / z)",   "x * (1 / y) * (1 / z)",   true}, // again
        {"x / (y * z)",             "x * (1 / y) * (1 / z)",   true},
        {"x * (x^2 / 3 + y)",       "x * (x^2 * (1 / 3) + y)", true},
        {"x * (x^2 * (1 / 3) + y)", "x * (x^2 * (1 / 3) + y)", true}, // again
        {"(x^3) / 3 + y * x",       "x^3 * (1 / 3) + y * x",   true},
        {"x^3 * (1 / 3) + y * x",   "x^3 * (1 / 3) + y * x",   true}, // again
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        testSingleTest(&programs[i],
                       &dcFlatArithmetic_convertDivideToMultiply,
                       true);
    }
}

static void testExpandRaise(void)
{
    struct
    {
        const char *program;
        const char *expected;
    } programs[] =
    {
        {"x",             "x"},
        {"e^x",           "e^x"},
        {"e^(x + 1)",     "e^x * e^1"},
        {"e^(x + y)",     "e^x * e^y"},
        {"e^(x^2 + y)",   "e^(x^2) * e^y"},
        {"1 + e^(x + y)", "1 + e^x * e^y"},
        {"x / e^(x + y)", "x / (e^x * e^y)"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcNode *arithmetic = (createFlatArithmetic
                              (programs[i].program));
        arithmetic = dcFlatArithmetic_expandRaise(arithmetic, NULL);
        char *display = dcNode_synchronizedDisplay(arithmetic);
        dcTestUtilities_expectStringEqual(programs[i].expected, display);

        dcMemory_free(display);
        dcNode_free(&arithmetic, DC_DEEP);
    }
}

static void testConvertSubtractToAdd(void)
{
    const SingleTest programs[] =
    {
        {"-2 * x + 1",        "-2x + 1",          false},
        {"x",                 "x",                false},
        {"1 / 1",             "1 / 1",            false},
        {"1 - 2 * x",         "1 + -2x",          true},
        {"1 - a",             "1 + -a",           true},
        {"1 - 1",             "1 + -1",           true},
        {"1 - 2",             "1 + -2",           true},
        {"1 - (z - 1)",       "1 + -z + 1",       true},
        {"x^((z^y - 1) + 1)", "x^(z^y + -1 + 1)", true},
        {"x * (1 - (1 / y))", "x * (1 + -1 / y)", true},
        {"x - 2y",            "x + -2y",          true},
        {"-2 * x - y",        "-2x + -y",         true},
        {"x^3 - 5x^2 + 3x - 15",
         "x^3 + -5x^2 + 3x + -15",
         true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        testSingleTest(&programs[i],
                       &dcFlatArithmetic_convertSubtractToAdd,
                       true);
        testSingleTest(&programs[i],
                       &dcFlatArithmetic_convertSubtractToAdd,
                       false);
    }
}

static void testUndoConvertSubtractToAdd(void)
{
    const SingleTest programs[] =
    {
        {"-1 + x",               "-1 + x",                    false},
        {"x",                    "x",                         false},
        {"1 / 1",                "1 / 1",                     false},
        {"x * (1 - (1 / y))",    "x * (1 - 1 / y)",           false},
        {"-y + -x",              "-y - x",                    true},
        {"1 + -1",               "1 - 1",                     true},
        {"1 + -a",               "1 - a",                     true},
        {"1 + -z + 1",           "1 - z + 1",                 true},
        {"x^((z^y + -1) + 1)",   "x^(z^y - 1 + 1)",           true},
        {"x * (1 + -(1 / y))",   "x * (1 - 1 / y)",           true},
        {"1 + -1 / y",           "1 - 1 / y",                 true},
        {"x * (1 + -1 / y)",     "x * (1 - 1 / y)",           true},
        {"x + -2",               "x - 2",                     true},
        {"x + -2 + -y",          "x - 2 - y",                 true},
        {"x + -y",               "x - y",                     true},
        {"x + -y + -z",          "x - y - z",                 true},
        {"x + -y + -z - w + -2", "x - y - z - w - 2",         true},
        {"x + -y + -z + w + -2", "x - y - z + w - 2",         true},
        {"x + -2y",              "x - 2y",                    true},
        {"x + -2y + -3*x*z - w", "(x - 2y - 3 * x * z) - w",  true},
        {"x^3 + -5x^2 + 3x + -15",
         "x^3 - 5x^2 + 3x - 15",
         true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        testSingleTest(&programs[i],
                       &dcFlatArithmetic_undoConvertSubtractToAdd,
                       true);
    }
}

static void testRemoveForSubstitution(void)
{
    const OperationProgram programs[] =
    {
        {"(1 - 1 / w) * cos(w - ln(w))",
         "(w - 1) / w",
         "cos(w - ln(w))"},

        {"(1 - 1 / w) * cos(w - ln(w))",
         "1 - 1 / w",
         "cos(w - ln(w))"},
        {"e^(x^3 + x^2) * (3 * x^2 * 1 + 2 * x * 1) * 1",
         "3 * x^2 + 2x",
         "e^(x^3) * e^(x^2) * 1"},
        {"e^__x_sub__ * (3 * x^2 * 1 + 2 * x * 1) * 1",
         "3 * x^2 + 2x",
         "e^__x_sub__ * 1"},
        {"1 / (a * b * x + c)",
         "a * b",
         "(1 / (a * b * x + c)) * (1 / (a * b))"},

        {"f(x)",               "a * b", "f(x) * (1 / (a * b))"},
        {"x * y",              "x",     "y"},
        {"2 * x * y",          "x",     "2y"},
        {"2 * x * y",          "x * y", "2 * y * (1 / y)"},
        {"(2 * x) * y",        "x * y", "2 * y * (1 / y)"},
        {"(2 * x) * y",        "2 * x", "2 * y * (1 / 2)"},

        // out of order
        {"(x * y) / z",      "y * x",        "y * (1 / z) * (1 / y)"},
        {"(x * y * w) / z",  "y * x",        "y * w * (1 / z) * (1 / y)"},
        {"(x * y * w) / z",  "w * y * x",    "y * w * (1 / z) * (1 / (w * y))"},

        {"(3 * x^2 + 2x) * y", "3 * x^2 + 2x", "y"},
        {"(3 * x^2 + 2x) * y", "x^2 * 3 + 2x", "y"},

        {"x",           "2x",          "1 * (1 / 2)"},
        {"y / (2 * x)", "1 / (2 * x)", "y"},
        {"(2 * y) / x", "1 / x",       "2y"},
        {"(2 * y) / x", "2 / x",       "2 * y * (1 / 2)"},
        {"y / x",       "1 / x",       "y"},

        {"(3x^2 + 2x) * e", "3x^2 + 2x", "e"},
        {"3 / (x * y)",     "1 / x",     "3 * (1 / y)"},
        {"3 / (x * y * z)", "1 / x",     "3 * (1 / y) * (1 / z)"},

        // lots
        {"(x * y * (z + 1)) / (w + 2)", "x", "y * (z + 1) * (1 / (w + 2))"},
        {"(x * y * (z + 1)) / (w + 2)",
         "x * y",
         "y * (z + 1) * (1 / (w + 2)) * (1 / y)"},
        {"(x * y * (z + 1)) / (w + 2)",
         "x * y * (z + 1)",
         "y * (1 / (w + 2)) * (1 / y)"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: %s\n",
                  programs[i].program);

        dcNode *arithmetic = (createFlatArithmetic
                              (programs[i].program));
        dcNode *removeNode = createFlatArithmetic(programs[i].symbol);

        dcFlatArithmetic_remove(&arithmetic, removeNode, "x");

        if (arithmetic != NULL)
        {
            char *display = dcNode_synchronizedDisplay(arithmetic);

            if (strcmp(display, programs[i].expected) != 0)
            {
                fprintf(stderr,
                        "\n\nOperation compare failed.\n"
                        "program:  [%s]\n"
                        "operated: [%s]\n"
                        "expected: [%s]\n",
                        programs[i].program,
                        display,
                        programs[i].expected);
                dcError_assert(false);
            }

            dcMemory_free(display);
        }
        else
        {
            dcError_assert(programs[i].expected == NULL);
        }

        dcNode_free(&arithmetic, DC_DEEP);
        dcNode_free(&removeNode, DC_DEEP);
    }
}

static void testRemoveByParts(void)
{
    struct
    {
        const char *arithmetic;
        const char *remove;
        const char *expectedArithmetic;
    } programs[] =
    {
        {"x", "y", "x"},
        {"x", "x", "1"},

        {"x * y * (z + 1)", "z + 1",       "x * y"},
        {"x * y * (z + 1)", "(z + 1) * y", "x"},

        {"(x * y * (z + 1)) / (w + 2)", "x", "y * (z + 1) * (1 / (w + 2))"},
        {"(x * y * (z + 1)) / (w + 2)", "x * y", "(z + 1) * (1 / (w + 2))"},
        {"(x * y * (z + 1)) / (w + 2)", "x * y * (z + 1)", "(1 / (w + 2))"},

        {"e^x * sin(x)", "e^x",          "sin(x)"},
        {"e^x * sin(x)", "e^x * sin(x)", "1"},

        {"__x_sub__ * cos(x)", "e^x", "__x_sub__ * cos(x)"},

        {"-0.25 * sin(x) * x^4", "-0.25", "sin(x) * x^4"},

        {"sin(x) * __x_sub__",
         "4 * x^3",
         "sin(x) * __x_sub__"},

        {"__x_sub__ * x^5", "-sin(x)", "__x_sub__ * x^5"},

        {"-0.25 * sin(x) * x^4", "-0.25 * sin(x)", "x^4"},

        {"-0.050 * cos(x) * __x_sub__",
         "5 * x^4",
         "-0.050 * cos(x) * __x_sub__"},

        {"-4 * x^3 * cos(x)", "x^3 * cos(x)", "-4"},

        {"-0.25 * __x_sub__ * x^4",
         "cos(x)",
         "-0.25 * __x_sub__ * x^4"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: %s\n",
                  programs[i].arithmetic);

        dcNode *arithmetic = createFlatArithmetic(programs[i].arithmetic);
        dcNode *removeNode = createFlatArithmetic(programs[i].remove);
        char *beforeRemove = dcNode_synchronizedDisplay(removeNode);
        dcFlatArithmetic_remove(&arithmetic, removeNode, NULL);
        char *afterRemove = dcNode_synchronizedDisplay(removeNode);

        // verify remove wasn't modified
        assert(strcmp(beforeRemove, afterRemove) == 0);

        if (arithmetic != NULL)
        {
            char *display = dcNode_synchronizedDisplay(arithmetic);

            if ((arithmetic == NULL
                 && programs[i].expectedArithmetic != NULL)
                || (arithmetic != NULL
                    && programs[i].expectedArithmetic == NULL)
                || strcmp(display, programs[i].expectedArithmetic) != 0)
            {
                fprintf(stderr,
                        "\n\nOperation compare failed.\n"
                        "program:  [%s]\n"
                        "operated: [%s]\n"
                        "expected: [%s]\n",
                        programs[i].arithmetic,
                        display,
                        programs[i].expectedArithmetic);
                dcError_assert(false);
            }

            dcMemory_free(display);
        }
        else
        {
            dcError_assert(programs[i].expectedArithmetic == NULL);
        }

        dcMemory_free(beforeRemove);
        dcMemory_free(afterRemove);
        dcNode_free(&arithmetic, DC_DEEP);
        dcNode_free(&removeNode, DC_DEEP);
    }
}

static void testCancel(void)
{
    const SingleTest programs[] =
    {
        {"(2x) / x^2.000000",
         "(2 * 1) / x^(1.000000)",
         true},

        {"x / (y + z)", "x / (y + z)", false},

        {"(-7) / 3", "-7 / 3",   false},
        {"-7 / 3",   "-(7 / 3)", false},
        {"-(7.000000 / 3)", "-(7.000000 / 3)", false},
        {"-7.000000 / 3",   "-(7.000000 / 3)", false},
        {"(-3) / 7",        "-3 / 7",          false},
        {"3 / (-7)",        "-3 / 7",          true},

        //{"(y * x * (x^2 + a)) / (x^3 + x * a)",
        // "(y * 1 * (x^2 + a)) / (1 * (x^(3 - 1) + 1a))",
        // true},

        {"1 / 0.5", "2 / 1", true},

        {"(x^2 * y^4 + z * y^4) / y^5",
         "(1 * (x^2 + z)) / y^1",
         true},

        {"((x + 1)^1.5 - (x + 1)^0.5) / (x + 1)^2.0",
         "(1 * (x + 1 + -1)) / (x + 1)^1.5",
         true},

        {"x",                         "x",                         false},

        {"0 / 4",                     "0 / 4",                     false},
        {"0 / (2 * x)",               "0 / (2x)",                  false},
        {"4 / 0",                     "NaN / 1",                   true},

        // become 1
        {"1",                         "1",                         false},
        {"x / x",                     "1 / 1",                     true},
        {"(x - 1) / (x - 1)",         "1 / 1",                     true},
        {"x^2 / x^2",                 "1 / 1",                     true},
        {"(x * y) / (x * y)",         "1 / 1",                     true},

        // become 1 / 3
        {"(x * y) / (x * y) / 3",     "(1 / 1) / 3",               true},
        {"x^2 / x^2 / 3",             "(1 / 1) / 3",               true},

        // cancel out in multiply
        {"x / (y * x)",               "1 / (y * 1)",               true},
        {"(y * x) / x",               "(y * 1) / 1",               true},

        // cancel out function call
        {"ln(3) / (x * ln(3))",       "1 / (x * 1)",               true},

        // cancel out arithmetic
        {"((1 / x) * y) / (1 / x)",   "(1y) / 1",                  true},

        // raise
        {"x / x^2",                   "1 / x^(1)",                 true},
        {"x^2 / x",                   "x^(1) / 1",                 true},
        {"x^3 / x^2",                 "x^1 / 1",                   true},
        {"x^2 / x^3",                 "1 / x^1",                   true},
        {"((1 / x) * y) / (1 / x)^2", "(1y) / (1 / x)^(1)",        true},
        {"ln(3) / (x * ln(3)^2)",     "1 / (x * ln(3)^(1))",       true},
        {"x / (y * x^2)",             "1 / (y * x^(1))",           true},
        {"x / (y * x^z)",             "1 / (y * x^(z - 1))",       true},

        // false result
        {"3 / x", "3 / x", false},
        {"x / 3", "x / 3", false},
        {"x / 2", "x / 2", false},

        // subtract
        {"(x - y) / (2 * (x - y))", "1 / (2 * 1)", true},
        {"(x - y) / ((x - y) * 2)", "1 / (1 * 2)", true},

        // add
        {"(x + y) / (2 * (x + y))", "1 / (2 * 1)", true},
        {"(x + y) / ((x + y) * 2)", "1 / (1 * 2)", true},

        // numbers
        {"4 / 8",       "1 / 2",       true},
        {"(x * 4) / 8", "(x * 1) / 2", true},
        {"8 / 4",       "2 / 1",       true},
        {"(x * 8) / 4", "(x * 2) / 1", true},

        // divide a number by multiply
        {"6 / (2 * x)",               "3 / (1x)",                  true},
        {"6 / (2 * x * 3)",           "1 / (1 * x * 1)",           true},
        // again
        {"3 / (x * 3)",               "1 / (x * 1)",               true},
        {"6 / (2 * x * 8)",           "3 / (1 * x * 8)",           true},

        // add or subtract on top
        {"((x + y) * z) / (x + y)",   "(1z) / 1",                  true},
        {"(-(3x + x)) / x",            "(-1 * (3 + 1)) / 1",       true},
        {"(x + x^2) / x",              "(1 + x^(1)) / 1", true},
        {"(x + y^2) / x",              "(x + y^2) / x",            false},
        {"((x + y) * z) / (x + y)",   "(1z) / 1",                  true},
        {"(a * x - (x * a + a)) / a",
         "(1 * (x + -(x + 1))) / 1",
         true},
        {"((x * a) * ln(x * a) - x * a) / a",
         "(1 * x * (ln(x * a) + -1)) / 1",
         true},
        {"(x * y * 2 + x * y * 2) / 4",
         "(1 * y * (x + x)) / 2",
         true},
        {"(-2) / (4 * x^4)",    "-1 / (2x^4)",       true},

        // negatives going up
        {"(2) / (-4 * x^4)",    "-1 / (2x^4)",       true},
        {"1 / (-4 * x^4)",      "-1 / (4x^4)",       true},
        {"x / -4",              "-x / 4",            true},
        {"-x / -4",             "-(-x / 4)",         true},
        {"-(x * y) / -4",       "-(((-x) * y) / 4)", true},

        // various things that failed in other tests
        {"(ln(2) * 2^ln(x)) / x",     "(ln(2) * 2^ln(x)) / x",     false},

        {"(3 + x^2 * y) / (x * y)",   "(3 + x^2 * y) / (x * y)",   false},

        {"(3 * x * y + x^2 * y) / (x * y)",
         "(1 * 1 * (3 + x)) / (1 * 1)",
         true},

        {"(cos(x_sub) * x * (3 * x^(2 - 1) * 1 + 2 * 1 * 1)) "
         "/ (3 * x^2 + 2x)",
         "(cos(x_sub) * 1 * (3x^(2 - 1) + 2)) / (1 * (3x + 2))",
         true},

        {"(x + 1) / (x + 1)^2",
         "1 / (x + 1)^(1)",
         true},

        {"(x * (y + 1)) / (y + 1)^2",
         "(x * 1) / (y + 1)^(1)",
         true},

        {"((x + 1) * (-x + 2) * (x + 2) * -1) / (x * (x + 2)^2)",
         "((x + 1) * (-x + 2) * 1 * -1) / (x * (x + 2)^(1))",
         true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        testSingleTest(&programs[i], &dcFlatArithmetic_cancel, true);
    }
}

static void testFind(void)
{
    struct
    {
        const char *program;
        const char *toFind;
        bool expectedResult;
    }
    programs[] =
    {
        {"x",         "y",     false},
        {"x",         "y^2",   false},
        {"x",         "x^2",   false},
        {"x",         "x",     true},
        {"x^x",       "x^x",   true},
        {"x^x + 3",   "x^x",   true},
        {"e^(x + 1)", "e^x",   true},
        {"x / y",     "x",     true},
        {"x / y",     "1 / y", true},
        {"3x",        "6x",    false}, // really?
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: %s\n",
                  programs[i].program);

        dcNode *program = createFlatArithmetic(programs[i].program);
        dcNode *toFind = createFlatArithmetic(programs[i].toFind);
        bool found = dcFlatArithmetic_find(program, toFind);

        assert(found == programs[i].expectedResult);
        dcNode_free(&program, DC_DEEP);
        dcNode_free(&toFind, DC_DEEP);
    }
}

static void testCombine(void)
{
    const SingleTest tests[] =
    {
        {"7 / -3",   "7 / -3", false},
        {"-(7 / 3)", "-7 / 3", true},

        {"sin(x) * (1 / 3) * (1 / cos(x))", "tan(x) / 3", true},

        //
        // trigonometric
        //

        // to 1
        {"sin(x) * csc(x)",     "1", true},
        {"sin(x) * 3 * csc(x)", "3", true},
        {"cos(x) * sec(x)",     "1", true},
        {"cot(x) * tan(x)",     "1", true},

        // product identities
        {"tan(x) * cos(x)",     "sin(x)",     true},
        {"tan(x) * 3 * cos(x)", "sin(x) * 3", true},
        {"sin(x) * cot(x)",     "cos(x)",     true},
        {"sin(x) * 3 * cot(x)", "cos(x) * 3", true},
        {"cos(x) * csc(x)",     "cot(x)",     true},
        {"cot(x) * sec(x)",     "csc(x)",     true},
        {"csc(x) * tan(x)",     "sec(x)",     true},
        {"sec(x) * sin(x)",     "tan(x)",     true},

        // pythagorean identities

        //
        // sin(x)^2 + cos(x)^2 = 1
        //
        {"sin(x)^2 + cos(x)^2",   "1",         true},
        {"-sin(x)^2 + -cos(x)^2", "-1",        true},
        {"sin(x)^2 + -1",         "-cos(x)^2", true},
        {"cos(x)^2 + -1",         "-sin(x)^2", true},
        {"1 + -sin(x)^2",         "cos(x)^2", true},
        {"1 + -cos(x)^2",         "sin(x)^2", true},
        {"1 + -cos(y^2)^2", "sin(y^2)^2", true},

        //
        // 1 + cot(x)^2 = csc(x)^2
        //
        {"1 + cot(x)^2",         "csc(x)^2",  true},
        {"-1 + -cot(x)^2",       "-csc(x)^2", true},
        {"1 + -csc(x)^2",        "-cot(x)^2", true},
        {"cot(x)^2 + -csc(x)^2", "-1",        true},
        {"csc(x)^2 + -cot(x)^2", "1",         true},
        {"csc(x)^2 + -1",        "cot(x)^2",  true},

        //
        // tan(x)^2 + 1 = sec(x)^2
        //
        {"tan(x)^2 + 1",         "sec(x)^2",  true},
        {"-tan(x)^2 + -1",       "-sec(x)^2", true},
        {"tan(x)^2 + -sec(x)^2", "-1",        true},
        {"1 + -sec(x)^2",        "-tan(x)^2", true},
        {"sec(x)^2 + -1",        "tan(x)^2",  true},
        {"sec(x)^2 + -tan(x)^2", "1",         true},

        //
        // cosh(x)^2 - sinh(x)^2 = 1
        //
        {"cosh(x)^2 + -sinh(x)^2", "1",          true},
        {"-cosh(x)^2 + sinh(x)^2", "-1",         true},
        {"cosh(x)^2 + -1",         "-sinh(x)^2", true},
        {"-sinh(x)^2 + -1",        "-cosh(x)^2", true},
        {"1 + -cosh(x)^2",         "-sinh(x)^2", true},
        {"1 + sinh(x)^2",          "cosh(x)^2",  true},

        //
        // tanh(x)^2 + sech(x)^2 = 1
        //
        {"tanh(x)^2 + sech(x)^2",   "1",          true},
        {"-tanh(x)^2 + -sech(x)^2", "-1",         true},
        {"tanh(x)^2 + -1",          "-sech(x)^2", true},
        {"sech(x)^2 + -1",          "-tanh(x)^2", true},
        {"1 + -tanh(x)^2",          "sech(x)^2",  true},
        {"1 + -sech(x)^2",          "tanh(x)^2",  true},

        //
        // coth(x)^2 - csch(x)^2 = 1
        //
        {"coth(x)^2 + -csch(x)^2", "1",          true},
        {"-coth(x)^2 + csch(x)^2", "-1",         true},
        {"coth(x)^2 + -1",         "csch(x)^2",  true},
        {"-csch(x)^2 + -1",        "-coth(x)^2", true},
        {"1 + csch(x)^2",          "coth(x)^2",  true},
        {"1 + -coth(x)^2",         "-csch(x)^2", true},

        {"sin(x)^2 + 2 + cos(x)^2", "3",                 true},
        {"sin(x)^2 + cos(x)",       "sin(x)^2 + cos(x)", false},
        {"1 + cot(x)^2",            "csc(x)^2",          true},
        {"tan(x)^2 + 1",            "sec(x)^2",          true},


        {"1 + z + -sin(x)^2",   "cos(x)^2 + z",   true},
        {"1 + y + -cos(x)^2",   "sin(x)^2 + y",   true},
        {"1 + x + -cos(y^2)^2", "sin(y^2)^2 + x", true},

        {"csc(x)^2 + -1", "cot(x)^2", true},
        {"sec(x)^2 + -1", "tan(x)^2", true},

        //
        // hyperbolic
        //

        // to 1!
        {"cosh(x)^2 + -sinh(x)^2",     "1", true},
        {"tanh(x)^2 + sech(x)^2",      "1", true},
        {"tanh(x^y)^2 + sech(x^y)^2",  "1", true},
        {"coth(x)^2 + -csch(x)^2",     "1", true},
        {"coth(x^y)^2 + -csch(x^y)^2", "1", true},

        // to 1!!1
        {"cosh(x)^2 + 2 + -sinh(x)^2",      "3",       true},
        {"tanh(x)^2 + 3 + sech(x)^2",       "4",       true},
        {"tanh(x^y)^2 + z^2 + sech(x^y)^2", "1 + z^2", true},
        {"3 + coth(x)^2 + 4 + -csch(x)^2",  "8",       true},
        {"coth(x^y)^2 + r + -csch(x^y)^2",  "1 + r",   true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i], &dcFlatArithmetic_combine, true);
    }
}

static void testMerge(void)
{
    struct
    {
        const char *program;
        const char *expected;
        bool expectedModified;
    } programs[] =
    {
        {"(-3) / 7",          "(-3) / 7",           false},
        {"1",                 "1",                  false},
        {"x",                 "x",                  false},
        {"1 + x",             "1 + x",              false},
        {"1 + (x + y)",       "1 + x + y",          true},
        {"x * y",             "x * y",              false},
        {"x * (y * z)",       "x * y * z",          true},
        {"x * (y + z)",       "x * (y + z)",        false},
        {"x * (y + (z + 1))", "x * (y + z + 1)",    true},
        {"(x + (y + 1)) / z", "(x + y + 1) / z",    true},
        {"(x + (y + 1)) / z", "(x + y + 1) / z",    true},
        {"x / (y / z)",       "x / (y / z)",        false},
        {"x / (y * z)",       "x / (y * z)",        false},
        {"x / (y * (z * 2))", "x / (y * z * 2)",    true},
        {"x / (y + z)",        "x / (y + z)",       false},
        {"x / (y + (z + 2))",  "x / (y + z + 2)",   true},
        {"x / (y * (z + 2))",  "x / (y * (z + 2))", false},
        {"(x * y) / z",        "(x * y) / z",       false},
        {"((1 / x) * y) / (1 / x)",
         "((1 / x) * y) / (1 / x)",
         false},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcNode *program = (createFlatArithmetic
                           (programs[i].program));

        if (program != NULL)
        {
            dcNode *expected = (createFlatArithmetic
                                (programs[i].expected));

            if (dcNode_easyCompare(program, expected) != TAFFY_EQUALS)
            {
                fprintf(stderr,
                        "\n\nOperation compare failed.\n"
                        "program:  [%s]\n"
                        "operated: [%s]\n"
                        "expected: [%s]\n",
                        programs[i].program,
                        dcNode_display(program),
                        programs[i].expected);
                dcError_assert(false);
            }

            dcNode_free(&expected, DC_DEEP);
        }
        else
        {
            dcError_assert(programs[i].expected == NULL);
        }

        dcNode_free(&program, DC_DEEP);
    }
}

// run with --log "flat-arithmetic-choose" to see output
static void testChoose(void)
{
    struct ChooseTest
    {
        RemoveType chooseType;
        const char *program;
        uint8_t count;
        const char *symbol;
        const char *expectedResults;
    }
    tests[] =
    {
        // TODO:
        //{REMOVE_FOR_SUBSTITUTION,
        // "x^2 * (y / w^2) * z^3",
        // 2,
        // "w^2,x^2,y / w^2"},

        {REMOVE_FOR_SUBSTITUTION,
         "x^2 * a * b",
         1,
         "x",
         "x^2"},
        {REMOVE_FOR_SUBSTITUTION,
         "3 * (1 / x) * (1 / ln(x))",
         1,
         "x",
         "ln(x),1 / x,1 / ln(x)"},
        {REMOVE_FOR_SUBSTITUTION,
         "3 / (x * ln(x))",
         1,
         "x",
         "ln(x),x * ln(x)"},
        {REMOVE_FOR_SUBSTITUTION,
         "x^3 / y^4",
         1,
         "x",
         "x^3,y^4"},
        {REMOVE_FOR_SUBSTITUTION,
         "e^(x^2) * x^4",
         1,
         "x",
         "x^2,e^(x^2),x^2,x^4"},
        {REMOVE_FOR_SUBSTITUTION,
         "x^3 * f(x^2)",
         1,
         "x",
         "x^3,f(x^2),x^2"},

        {REMOVE_FOR_SUBSTITUTION,
         "f(x^2)",
         1,
         "x",
         "x^2"},
        {REMOVE_FOR_SUBSTITUTION,
         "e^(2x) * y^x",
         1,
         "x",
         "2x,e^(2x),2x,y^x"},
        {REMOVE_BY_PARTS,
         "e^(2x) * y^x",
         1,
         "x",
         "e^(2x),y^x"},
        {REMOVE_FOR_SUBSTITUTION,
         "1 / (a * b + c)",
         1,
         "x",
         "a * b + c"},
        {REMOVE_BY_PARTS,
         "1 / (a * b + c)",
         1,
         "x",
         ""},
        {REMOVE_FOR_SUBSTITUTION,
         "a / b",
         1,
         "x",
         ""},
        {REMOVE_BY_PARTS,
         "a / b",
         1,
         "x",
         ""},
        {REMOVE_FOR_SUBSTITUTION,
         "(2x + 5)^2",
         1,
         "x",
         "2x + 5"},
        {REMOVE_BY_PARTS,
         "(2x + 5)^2",
         1,
         "x",
         ""},
        {REMOVE_FOR_SUBSTITUTION,
         "1 / ln(x)",
         1,
         "x",
         "ln(x)"},
        {REMOVE_BY_PARTS,
         "1 / ln(x)",
         1,
         "x",
         ""},
        {REMOVE_FOR_SUBSTITUTION,
         "x^x",
         1,
         "x",
         ""},
        {REMOVE_BY_PARTS,
         "x^x",
         1,
         "x",
         ""},
        {REMOVE_FOR_SUBSTITUTION,
         "x * cos(x)",
         1,
         "x",
         "cos(x)"},
        {REMOVE_BY_PARTS,
         "x * cos(x)",
         1,
         "x",
         "x,cos(x)"},
        {REMOVE_FOR_SUBSTITUTION,
         "x^2 * cos(x)",
         1,
         "x",
         "x^2,cos(x)"},
        {REMOVE_BY_PARTS,
         "x^2 * cos(x)",
         1,
         "x",
         "x^2,cos(x)"},
        {REMOVE_FOR_SUBSTITUTION,
         "x^2 * cos(x) * sin(x)",
         1,
         "x",
         "x^2,cos(x),sin(x)"},
        {REMOVE_BY_PARTS,
         "x^2 * cos(x) * sin(x)",
         1,
         "x",
         "x^2,cos(x),sin(x)"},
        {REMOVE_FOR_SUBSTITUTION,
         "x^2 * cos(x^3) * sin(x)",
         1,
         "x",
         "x^2,cos(x^3),x^3,sin(x)"},
        {REMOVE_BY_PARTS,
         "x^2 * cos(x^3) * sin(x)",
         1,
         "x",
         "x^2,cos(x^3),sin(x)"},
        {REMOVE_FOR_SUBSTITUTION,
         "f(x^2)",
         1,
         "x",
         "x^2"},
        {REMOVE_BY_PARTS,
         "f(x^2)",
         1,
         "x",
         ""},
        {REMOVE_FOR_SUBSTITUTION,
         "f(x) * e^(x^2) * z^3 * y",
         1,
         "x",
        "x^2,f(x),e^(x^2),x^2,z^3"},
        {REMOVE_BY_PARTS,
         "f(x) * e^(x^2) * z^3 * y",
         1,
         "x",
        "f(x),e^(x^2),z^3,y"},
        {REMOVE_FOR_SUBSTITUTION,
         "3 * x^2 * e^(x^3 + x^2)",
         1,
         "x",
        "x^3 + x^2,x^2,e^(x^3 + x^2),x^3 + x^2"},
        {REMOVE_FOR_SUBSTITUTION,
         "(4x^3) / (x^4 + 7)",
         1,
         "x",
        "x^3,4x^3,x^4 + 7"},
        {REMOVE_BY_PARTS,
         "(4x^3) / (x^4 + 7)",
         1,
         "x",
         ""},
        {REMOVE_BY_PARTS,
         "(4x^3) * (1 / (x^4 + 7))",
         1,
         "x",
         "4,x^3,1 / (x^4 + 7)"},
        {REMOVE_FOR_SUBSTITUTION,
         "(4x^3) / f(x^2)",
         1,
         "x",
         "x^3,4x^3,f(x^2),x^2"},
        {REMOVE_FOR_SUBSTITUTION,
         "2^ln(x) * x^2",
         1,
         "x",
         "ln(x),2^ln(x),ln(x),x^2"},
        {REMOVE_FOR_SUBSTITUTION,
         "2^ln(x^3) * x^2",
         1,
         "x",
         "ln(x^3),x^3,2^ln(x^3),ln(x^3),x^2"},
        {REMOVE_BY_PARTS,
         "2^ln(x) * x^2",
         1,
         "x",
         "2^ln(x),x^2"},
        {REMOVE_FOR_SUBSTITUTION,
         "x^2 * (y / w^2) * z",
         1,
         "x",
         "w^2,x^2,y / w^2"},
        {REMOVE_BY_PARTS,
         "x^2 * (y / w^2) * z",
         1,
         "x",
         "x^2,y / w^2,z"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: %s type: %s\n",
                  tests[i].program,
                  (tests[i].chooseType == REMOVE_BY_PARTS
                   ? "by parts"
                   : (tests[i].chooseType == REMOVE_FOR_SUBSTITUTION
                      ? "for substitution"
                      : "????")));

        dcNode *program = createFlatArithmetic(tests[i].program);
        dcList *expectedResultsList =
            dcLexer_splitString(tests[i].expectedResults, ',');

        {
            FOR_EACH_IN_LIST(expectedResultsList, that)
            {
                that->object = dcList_createNodeWithObjects(that->object, NULL);
            }
        }

        dcList *results = dcList_create();
        dcHash *parents = dcHash_create();

        dcFlatArithmetic_choose(program,
                                tests[i].symbol,
                                tests[i].count,
                                tests[i].chooseType,
                                results,
                                parents);

        dcLog_log(FLAT_ARITHMETIC_CHOOSE_LOG,
                  "Choose %u of '%s' gets: '%s'\n",
                  tests[i].count,
                  dcNode_display(program),
                  dcList_display(results));

        if (expectedResultsList->size != results->size)
        {
            fprintf(stderr,
                    "expected list size: %u but got: %u, "
                    "expected list: '%s' results: '%s'\n",
                    expectedResultsList->size,
                    results->size,
                    dcList_display(expectedResultsList),
                    dcList_display(results));
            assert(false);
        }

        dcListElement *that;
        dcListElement *bat;

        for (that = expectedResultsList->head, bat = results->head;
             that != NULL && bat != NULL;
             that = that->next, bat = bat->next)
        {
            dcTestUtilities_expectNodeStringsEqual(that->object, bat->object);
        }

        dcHash_free(&parents, DC_DEEP);
        dcList_free(&expectedResultsList, DC_DEEP);
        dcList_free(&results, DC_DEEP);
        dcNode_free(&program, DC_DEEP);
    }
}

static void testSubstitution(void)
{
    struct
    {
        const char *program;
        const char *symbol;
        const char *expectedResult;
        const char *expectedSubstitution;
    }
    tests[] =
    {
        {"x^2 * sin(x^2)", "x", NULL, NULL},
        {"x * cos(x^3 + x^2) * (3 * x^1 * 1 + 2 * 1 * 1)",
         "x",
         "cos(x_substitution)",
         "x^3 + x^2"},
        {"x^3 * e^(x^2)",
         "x",
         "e^x_substitution * x_substitution * (1 / 2)",
         "x^2"},
        {"x^3 * e^(x^4)",
         "x",
         "e^x_substitution * (1 / 4)",
         "x^4"},
        {"(a^2 / d) / (a * b * x + c)",
         "x",
         "a^2 * (1 / d) * (1 / x_substitution) * (1 / (a * b))",
         "a * b * x + c"},
        {"3/(x * ln(x))",      "x", "3 * (1 / x_substitution)",        "ln(x)"},
        {"x * sqrt(x + 1)",
         "x",
         "(x_substitution - 1) * sqrt(x_substitution)",
         "x + 1"},
        {"1 / (a^2 * b^2 * x + c * a * b)",
         "x",
         "(1 / x_substitution) * (1 / (a^2 * b^2))",
         "a^2 * b^2 * x + c * a * b"},
        {"1 / (a^2 + x)",
         "x",
         "(1 / x_substitution) * (1 / 1)",
         "a^2 + x"},
        {"1 / (a * b * x + c)",
         "x",
         "(1 / x_substitution) * (1 / (a * b))",
         "a * b * x + c"},
        // not fair:
        {"(-c / (a * b)) * (1 / (a * b * x + c))",
         "x",
         "-c * (1 / a) * (1 / b) * (1 / x_substitution) * (1 / (a * b))",
         "a * b * x + c"},
        {"sinh(a * x)", "x", "sinh(x_substitution) * 1 * (1 / a)", "a * x"},
        {"ln(a*b*x + c)",
         "x",
         "ln(x_substitution) * 1 * (1 / (a * b))",
         "a * b * x + c"},
        {"cos(0.5 * x)", "x", "cos(x_substitution) * 1 * (1 / 0.5)", "0.5 * x"},
        {"2^ln(x) / x",        "x", "2^x_substitution",          "ln(x)"},
        {"4x^3 * e^(x^4)",     "x", "4 * e^x_substitution * (1 / 4)", "x^4"},
        {"(4x^3) / (x^4 + 7)",
         "x",
         "4 * (1 / x_substitution) * (1 / 4)",
         "x^4 + 7"},
        {"x * cos(x^2)",       "x", "cos(x_substitution) * (1 / 2)", "x^2"},
        {"cos(x^2) * x",       "x", "cos(x_substitution) * (1 / 2)", "x^2"},
        {"x / (1 + x^2)",
         "x",
         "(1 / x_substitution) * (1 / 2)",
         "x^2 + 1"},
        {"x^4 / (1 + x^5)",
         "x", "(1 / x_substitution) * (1 / 5)",
         "x^5 + 1"},
        {"(4x^3) / (x^4 + 7)",
         "x",
         "4 * (1 / x_substitution) * (1 / 4)",
         "x^4 + 7"},
        {"x * cos(0.5 * x^2)", "x", "cos(x_substitution)",       "0.5 * x^2"},
        {"e^(5x)",             "x", "e^x_substitution * 1 * (1 / 5)",  "5x"},
        {"x^3 * ln(x^4)",
         "x",
         "ln(x_substitution) * (1 / 4)",
         "x^4"},
        {"(3x^2 + 2x) * e^(x^3 + x^2)",
         "x",
         "e^x_substitution",
         "x^3 + x^2"},
        {"(1 - 1 / w) * cos(w - ln(w))",
         "w",
         "cos(x_substitution)",
         "w - ln(w)"},
        {"x * (x + 1)^0.5",
         "x",
         "(x_substitution - 1) * x_substitution^0.5",
         "x + 1"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG, "Program: %s\n", tests[i].program);
        dcNode *program = createFlatArithmetic(tests[i].program);

        // TODO: test failure case
        dcNode *substitution = NULL;
        dcNode *result = dcFlatArithmetic_substitute(program,
                                                     tests[i].symbol,
                                                     "x_substitution",
                                                     &substitution);

        if (result == NULL)
        {
            assert(tests[i].expectedResult == NULL);
        }
        else
        {
            dcTestUtilities_expectStringNodeEqual(tests[i].expectedResult,
                                                  result);
        }

        if (tests[i].expectedSubstitution == NULL)
        {
            assert(substitution == NULL);
        }
        else
        {
            dcTestUtilities_expectStringNodeEqual(tests[i].expectedSubstitution,
                                                  substitution);
        }

        dcNode_free(&program, DC_DEEP);
        dcNode_free(&substitution, DC_DEEP);
        dcNode_free(&result, DC_DEEP);
    }
}

static void testSolve(void)
{
    struct
    {
        const char *symbol;
        const char *leftHandSide;
        const char *rightHandSide;
        const char *expectedLeft;
        const char *expectedRight;
    } programs[] =
    {
        {"x",
         "x * y + x * 2",
         "3x",
         "1",
         "3 / (y + 2)"},

        {"x",
         "x * y + x * 2",
         "3x * y",
         "1",
         "(3y) / (y + 2)"},

        {"x",
         "x + 2",
         "y",
         "x",
         "y - 2"},

        {"x",
         "x",
         "y",
         "x",
         "y"},

        {"x",
         "x",
         "y + 2",
         "x",
         "y + 2"},

        /*
        {"x", "x", "y + ((2x + 3) - d)", "d - y - 3"},
        {"x", "x", "y + (2x - d)", "d - y"},

        {"y",
         "y",
         "e^x * sin(x) - (-e^x * cos(x) + y)",
         "(sin(x) * e^x + cos(x) * e^x) / 2"},

        {"y",
         "y",
         "-e^x * cos(x) + (e^x * sin(x) - y)",
         "(sin(x) * e^x - cos(x) * e^x) / 2"},

        {"x", "x",           "y + (2 - (d - 2x))", "d - y - 2"},
        {"x", "x",           "y + (2 - (2x - d))", "(y + d + 2) / 3"},
        {"x", "x",           "y + (2 - x)",        "(y + 2) / 2"},
        {"x", "x",           "y - (x - 2)",        "(y + 2) / 2"},

        // TODO: add tests for left hand side

        {"x", "x",           "y - (2 * x)", "y / 3"},
        {"x", "x^2",         "y",           "y^(1 / 2)"},
        {"x", "x",           "y",           "y"},
        {"x", "2x",          "y",           "y / 2"},
        {"x", "2 + x",       "y",           "y - 2"},
        {"x", "2 * (x + y)", "1",           "(-2y + 1) / 2"},
        */
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: %s, %s, %s, %s, %s\n",
                  programs[i].symbol,
                  programs[i].leftHandSide,
                  programs[i].rightHandSide,
                  programs[i].expectedLeft,
                  programs[i].expectedRight);

        dcNode *leftHandSide =
            createFlatArithmetic(programs[i].leftHandSide);
        dcNode *rightHandSide =
            createFlatArithmetic(programs[i].rightHandSide);
        dcFlatArithmetic_newSolve
            (programs[i].symbol, &leftHandSide, &rightHandSide);

        char *leftDisplayed = dcNode_synchronizedDisplay(leftHandSide);
        char *rightDisplayed = dcNode_synchronizedDisplay(rightHandSide);

        if (strcmp(leftDisplayed, programs[i].expectedLeft) != 0)
        {
            fprintf(stderr,
                    "\nExpected left: %s but got: %s, for "
                    "Program: %s, %s, %s\n",
                    programs[i].expectedLeft,
                    leftDisplayed,
                    programs[i].symbol,
                    programs[i].leftHandSide,
                    programs[i].rightHandSide);
            assert(false);
        }

        if (strcmp(rightDisplayed, programs[i].expectedRight) != 0)
        {
            fprintf(stderr,
                    "\nExpected right: %s but got: %s, for "
                    "Program: %s, %s, %s\n",
                    programs[i].expectedRight,
                    rightDisplayed,
                    programs[i].symbol,
                    programs[i].leftHandSide,
                    programs[i].rightHandSide);
            assert(false);
        }

        dcMemory_free(leftDisplayed);
        dcMemory_free(rightDisplayed);

        dcNode_free(&leftHandSide, DC_DEEP);
        dcNode_free(&rightHandSide, DC_DEEP);
    }
}

static void testContainsIdentifier(void)
{
    struct
    {
        const char *identifier;
        const char *program;
        bool expectedResult;
    } programs[] =
    {
        {"leftHandSideLoL",
         "sin(x) * (cos(x) * x - (x * cos(x) + sin(x))) - leftHandSideLoL",
         true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcNode *program = createFlatArithmetic(programs[i].program);
        bool result = dcFlatArithmetic_containsIdentifier
            (program, programs[i].identifier);
        dcError_assert(result == programs[i].expectedResult);
        dcNode_free(&program, DC_DEEP);
    }
}

static void testDisplay(void)
{
    struct
    {
        const char *arithmetic;
        const char *expectedDisplay;
    } tests[] =
    {
        {"1 / (-(a + b))", "1 / -(a + b)"},
        {"x^(-3)",         "x^(-3)"},
        {"-(x / y)",       "-(x / y)"},
        {"-((x + z) / y)", "-((x + z) / y)"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *arithmetic =
            createFlatArithmetic(tests[i].arithmetic);
        const char *display = dcNode_display(arithmetic);
        dcTestUtilities_expectStringEqual(tests[i].expectedDisplay, display);
        dcNode_free(&arithmetic, DC_DEEP);
    }
}

static void testFactor(void)
{
    const SingleTest tests[] =
    {
        {"x^0.5 + x",           "x^0.5 + x",                   false},
        {"x",                   "x",                           false},
        {"0 + x",               "0 + x",                       false},
        {"0 + 1",               "0 + 1",                       false},
        {"1 - x",               "1 - x",                       false},
        {"-1 - z",              "-1 - z",                      false},
        {"-a - 1",              "-a - 1",                      false},
        {"-(x / y - 1)",        "-(x / y - 1)",                false},
        {"y + x * (-1 + -z)",   "y + x * (-1 + -z)",           false},
        {"x + y",               "x + y",                       false},
        {"y - x",               "y - x",                       false},
        {"-x + -y",             "-x + -y",                     false},
        {"x^x",                 "x^x",                         false},
        {"-x + -y - 2",         "-x + -y - 2",                 false},
        {"1 * (x * 1 + y * 1)", "1 * (x * 1 + y * 1)",         false},
        {"x^y^z + y * x",       "x^(y^z) + y * x",             false},
        {"x * (x^(-2) + x)",    "x * (x^(-2) + x)",            false},
        {"x + x",               "x + x",                       false},
        {"x + y*x",             "x * (1 + y * 1)",             true},
        {"2 * (x / y) + x",     "x * (2 * 1 * (1 / y) + 1)",   true},
        {"x / y + x",           "x * (1 * (1 / y) + 1)",       true},
        {"2x + 3x*y",           "x * (2 * 1 + 3 * 1 * y)",     true},
        {"3x*y + 2x",           "x * (3 * 1 * y + 2 * 1)",     true},
        {"x + 2*x + y",         "y + x * (1 + 2 * 1)",         true},
        {"x - y*x",             "x * (1 + -y * 1)",            true},
        {"y - x - x * z",       "y + x * (-1 + -1 * z)",       true},
        {"x + x * z",           "x * (1 + 1z)",                true},
        {"y - x + x * z",       "y + x * (-1 + 1z)",           true},
        {"y - x - y*x",         "y + x * (-1 + -y * 1)",       true},
        {"x + (x - y*x)",       "x * (1 + 1 * (1 + -y * 1))",  true},
        {"2x + 2",              "2 * (1x + 1)",                true},
        {"(-3x) / y + 3",       "3 * (-x * (1 / y) + 1)",      true},
        {"(-3x) + 3",           "3 * (-x + 1)",                true},
        {"3 + (3x) / y",        "3 * (1 + 1 * x * (1 / y))",   true},
        {"sin(x) + x * sin(x)", "sin(x) * (1 + x * 1)",        true},
        {"3x + 6y",             "3 * (1x + 2y)",               true},
        {"3x + 6y + 5z",        "5z + 3 * (1x + 2y)",          true},
        {"x + y*x + y*z*x",     "x * (1 + y * 1 + y * z * 1)", true},
        {"5x + 15",             "5 * (1x + 3)",                true},
        {"x + y*x + x*z*y + z",
         "x + y * x + z * (x * 1 * y + 1)",
         true},
        {"(x^3) / 2 + y * x",
         "x * (x^2 * (1 / 2) + y * 1)",
         true},
        {"x * (1 + y * 1 + y * z * 1)",
         "x * (1 + y * (1 * 1 + 1 * z * 1))",
         true},
        {"x^2 + y * x^3",       "x^2 * (1 + y * x^1)", true},
        {"x^2 + y * x^4",       "x^2 * (1 + y * x^2)", true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i], &dcFlatArithmetic_factor, true);
    }
}

static void testDistribute(void)
{
    struct
    {
        const char *program;
        const char *expectedResult;
        bool expectedModified;
    } tests[] =
    {
        {"10 * (x + 2)",      "x * 10 + 20",   true},
        {"x * (1 - (1 - x))", "x - (x - x^2)", true},
        {"2 * (x + 1)",       "x * 2 + 2",     true},
        {"x * (x + 1)",       "x^2 + x",       true},
        {"x * (x - 1)",       "x^2 - x",       true},
        {"(x + 1) * 2",       "x * 2 + 2",     true},
        {"(x + 1) * x",       "x^2 + x",       true},
        {"(x - 1) * x",       "x^2 - x",       true},
        {"x + 1",             "x + 1",         false},
        {"x * sin(x)^2",      "x * sin(x)^2",  false},
        {"(1 + x) / y",       "(1 + x) / y",   false},
        {"0.500000 * (x * 0.5 * (x * (1 - cos(2x))))",
         "0.2500000 * x^2 - cos(2x) * 0.2500000 * x^2",
         true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: (%s)\n",
                  tests[i].program);

        dcNode *program = createFlatArithmetic(tests[i].program);
        dcNode *expectedResult =
            createFlatArithmetic(tests[i].expectedResult);
        bool modified = false;

        program = dcFlatArithmetic_distribute(program, &modified);
        program = dcFlatArithmetic_combine(program, NULL);
        program = dcFlatArithmetic_merge(program, NULL);

        char *display = dcNode_synchronizedDisplay(program);

        dcTestUtilities_expectStringEqual(tests[i].expectedResult, display);
        dcError_assert(modified == tests[i].expectedModified);

        dcMemory_free(display);
        dcNode_free(&program, DC_DEEP);
        dcNode_free(&expectedResult, DC_DEEP);
    }
}

static void testDistributeDivide(void)
{
    const SingleTest tests[] =
    {
        {"(1 + x) / y",       "1 / y + x / y",   true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i], &dcFlatArithmetic_distributeDivide, true);
    }
}

static void testSymbolCount(void)
{
    struct
    {
        const char *symbol;
        const char *program;
        size_t expectedCount;
    } tests[] =
    {
        {"x", "y",           0},
        {"x", "y^2",         0},
        {"x", "2 * (x + 1)", 1},
        {"x", "1 / x",       1},
        {"x", "ln(x)",       1},
        {"x", "1 / x^2",     2},
        {"x", "x^2",         2},
        {"x", "ln(x, x)",    2},
        {"x", "ln(x, x^2)",  3},
        {"x", "x^2 + x",     3},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *program = createFlatArithmetic(tests[i].program);
        size_t symbolCount =
            dcFlatArithmetic_symbolCount(program, tests[i].symbol);
        dcError_assert(symbolCount == tests[i].expectedCount);
        dcNode_free(&program, DC_DEEP);
    }
}

static void testExpand(void)
{
    const SingleTest tests[] =
    {
        {"x",            "x",              false},
        {"x + 1",        "x + 1",          false},
        {"x^2",          "x * x",          true},
        {"x^2 * sin(x)", "x * x * sin(x)", true},
        {"1 + x^2",      "1 + x * x",      true},
        {"1 / x^2",      "1 / (x * x)",    true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i], &dcFlatArithmetic_expand, true);
    }
}

static void testSimplifyMethodCall(void)
{
    const SingleTest tests[] =
    {
        {"abs(a * b * x * d + c * d)",
         "abs(a * b * x * d + c * d)",
         false},

        {"abs(2 / 2)", "abs(1)", true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i],
                       &dcFlatArithmetic_simplifyMethodCall,
                       true);
    }
}

static void testSimplifyTrigonometry(void)
{
    const SingleTest tests[] =
    {
        // hyperbolic
        {"(e^x - e^(-x)) / 2",  "sinh(x)",     true},
        {"1 / sinh(x)",         "csch(x)",     true},
        {"1 / sinh(x^2)",       "csch(x^2)",   true},
        {"2 / (e^x - e^(-x))",  "csch(x)",     true},
        {"1 / (e^x - e^(-x))",  "csch(x) / 2", true},
        {"(e^x + e^(-x)) / 2",  "cosh(x)",     true},
        {"(e^x + e^(-x)) / 1",  "cosh(x) * 2", true},
        {"1 / cosh(x)",         "sech(x)",     true},
        {"1 / cosh(x^2)",       "sech(x^2)",   true},
        {"2 / (e^x + e^(-x))",  "sech(x)",     true},
        {"1 / (e^x + e^(-x))",  "sech(x) / 2", true},
        {"sinh(x) / cosh(x)",   "tanh(x)",     true},
        {"1 / tanh(x)",         "coth(x)",     true},

        {"x", "x", false},

        {"log(a, b)", "ln(b) / ln(a)", true},

        // trig
        {"sin(-x)",             "-sin(x)",  true},
        {"cos(-x)",             "cos(x)",   true},
        {"tan(-x)",             "-tan(x)",  true},
        {"sin(-3x)",            "-sin(3x)", true},
        {"cos(-3x)",            "cos(3x)",  true},
        {"tan(-3x)",            "-tan(3x)", true},
        {"csc(-x)",             "-csc(x)",  true},
        {"sec(-x)",             "sec(x)",   true},
        {"cot(-x)",             "-cot(x)",  true},
        {"csc(-3x)",            "-csc(3x)", true},
        {"sec(-3x)",            "sec(3x)",  true},
        {"cot(-3x)",            "-cot(3x)", true},

        // a
        {"asin(-x)",         "-asin(x)",   true},
        {"atan(-x)",         "-atan(x)",   true},
        {"asin(-3x)",        "-asin(3x)",  true},
        {"atan(-3x)",        "-atan(3x)",  true},
        {"acsc(-x)",          "-acsc(x)",  true},
        {"acot(-x)",          "-acot(x)",  true},
        {"acsc(-3x)",         "-acsc(3x)", true},
        {"acot(-3x)",         "-acot(3x)", true},
        {"asec(-x)",          "asec(-x)",  false}, // no change

        // hyperbolic
        {"sinh(-x)",            "-sinh(x)",   true},
        {"cosh(-x)",            "cosh(x)",    true},
        {"tanh(-x)",            "-tanh(x)",   true},
        {"sinh(-3x)",           "-sinh(3x)",  true},
        {"cosh(-3x)",           "cosh(3x)",   true},
        {"tanh(-3x)",           "-tanh(3x)",  true},
        {"csch(-x)",             "-csch(x)",  true},
        {"sech(-x)",             "sech(x)",   true},
        {"coth(-x)",             "-coth(x)",  true},
        {"csch(-3x)",            "-csch(3x)", true},
        {"sech(-3x)",            "sech(3x)",  true},
        {"coth(-3x)",            "-coth(3x)", true},

        // a hyperbolic
        {"asinh(-x)",         "-asinh(x)",  true},
        {"acosh(-x)",         "acosh(-x)",  false}, // no change
        {"atanh(-x)",         "-atanh(x)",  true},
        {"asinh(-3x)",        "-asinh(3x)", true},
        {"acosh(-3x)",        "acosh(-3x)", false}, // no change
        {"atanh(-3x)",        "-atanh(3x)", true},
        {"acsch(-x)",         "-acsch(x)",  true},
        {"acoth(-x)",         "-acoth(x)",  true},
        {"acsch(-3x)",        "-acsch(3x)", true},
        {"acoth(-3x)",        "-acoth(3x)", true},
        {"asech(-x)",         "asech(-x)",  false}, // no change

        // clockwise identities
        {"sin(x) / cos(x)", "tan(x)", true},
        {"cos(x) / cot(x)", "sin(x)", true},
        {"cot(x) / csc(x)", "cos(x)", true},
        {"csc(x) / sec(x)", "cot(x)", true},
        {"sec(x) / tan(x)", "csc(x)", true},
        {"tan(x) / sin(x)", "sec(x)", true},

        // sin(x^3) / cos(x) != tan(x), bug check
        {"sin(x^3) / cos(x)", "sin(x^3) * sec(x)", true},
        {"sec(x^2) / tan(x)", "sec(x^2) * cot(x)", true},

        // counterclockwise identities
        {"sin(x) / tan(x)", "cos(x)", true},
        {"tan(x) / sec(x)", "sin(x)", true},
        {"sec(x) / csc(x)", "tan(x)", true},
        {"csc(x) / cot(x)", "sec(x)", true},
        {"cot(x) / cos(x)", "csc(x)", true},
        {"cos(x) / sin(x)", "cot(x)", true},

        // reciprocal identities
        {"1 / csc(x)",              "sin(x)",       true},
        {"1 / csc(x)^2",            "1 / csc(x)^2", false},
        {"1 / sec(x)",              "cos(x)",       true},
        {"1 / cot(x)",              "tan(x)",       true},
        {"1 / sin(x)",              "csc(x)",       true},
        {"1 / cos(x)",              "sec(x)",       true},
        {"1 / tan(x)",              "cot(x)",       true},

        {"2 / csc(x)",          "2 * sin(x)",   true},
        {"2 / csc(x)^2",        "2 / csc(x)^2", false},
        {"2 / sec(x)",          "2 * cos(x)",   true},
        {"2 / cot(x)",          "2 * tan(x)",   true},
        {"2 / sin(x)",          "2 * csc(x)",   true},
        {"2 / cos(x)",          "2 * sec(x)",   true},
        {"2 / tan(x)",          "2 * cot(x)",   true},
        {"2 + tan(a)^2",        "2 + tan(a)^2", false},
        {"2 + cot(a)^2",        "2 + cot(a)^2", false},
        {"4 + tan(a)^2",        "4 + tan(a)^2", false},
        {"4 + cot(a)^2",        "4 + cot(a)^2", false},

        {"(2 + y) / csc(x)",    "(2 + y) * sin(x)",   true},
        {"(2 + y) / csc(x)^2",  "(2 + y) / csc(x)^2", false},
        {"(2 + y) / sec(x)",    "(2 + y) * cos(x)",   true},
        {"(2 + y) / cot(x)",    "(2 + y) * tan(x)",   true},
        {"(2 + y) / sin(x)",    "(2 + y) * csc(x)",   true},
        {"(2 + y) / cos(x)",    "(2 + y) * sec(x)",   true},
        {"(2 + y) / tan(x)",    "(2 + y) * cot(x)",   true},
        {"(2 + y) + tan(a)^2",  "(2 + y) + tan(a)^2", false},
        {"(2 + y) + cot(a)^2",  "(2 + y) + cot(a)^2", false},

        {"sin(0)", "0", true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i],
                       &dcFlatArithmetic_simplifyTrigonometry,
                       false);
    }
}

static void testIsInverseTrigonometric(void)
{
    struct
    {
        const char *program;
        const char *symbol;
        bool expectedResult;
    } tests[] =
    {
        {"asin(x)", "x", true},
        {"asin(x)", "y", false},
        {"acos(x)", "x", true},
        {"acos(x)", "y", false},
        {"atan(x)", "x", true},
        {"atan(x)", "y", false},
        {"acsc(x)", "x", true},
        {"acsc(x)", "y", false},
        {"asec(x)", "x", true},
        {"asec(x)", "y", false},
        {"acot(x)", "x", true},
        {"acot(x)", "y", false},

        {"asin(3x)", "x", true},
        {"asin(3x)", "y", false},

        {"asin(x^3)", "x", true},
        {"asin(x^3)", "y", false},

        {"asdf(x)",  "x", false},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *program = createFlatArithmetic(tests[i].program);
        bool result = dcFlatArithmetic_isInverseTrigonometric
            (program, tests[i].symbol);
        dcError_assert(result == tests[i].expectedResult);
        dcNode_free(&program, DC_DEEP);
    }
}

static void testIsTrigonometric(void)
{
    struct
    {
        const char *program;
        const char *symbol;
        bool expectedResult;
    } tests[] =
    {
        {"sin(x)", "x", true},
        {"sin(x)", "y", false},
        {"cos(x)", "x", true},
        {"cos(x)", "y", false},
        {"tan(x)", "x", true},
        {"tan(x)", "y", false},
        {"csc(x)", "x", true},
        {"csc(x)", "y", false},
        {"sec(x)", "x", true},
        {"sec(x)", "y", false},
        {"cot(x)", "x", true},
        {"cot(x)", "y", false},

        {"sin(3x)", "x", true},
        {"sin(3x)", "y", false},

        {"sin(x^3)", "x", true},
        {"sin(x^3)", "y", false},

        {"asdf(x)",  "x", false},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *program = createFlatArithmetic(tests[i].program);
        bool result = dcFlatArithmetic_isTrigonometric
            (program, tests[i].symbol);
        dcError_assert(result == tests[i].expectedResult);
        dcNode_free(&program, DC_DEEP);
    }
}

static void testIsExponential(void)
{
    struct
    {
        const char *program;
        const char *symbol;
        bool expectedResult;
    } tests[] =
    {
        {"x",     "x", false},
        {"2^x",   "x", true},
        {"2^x",   "y", false},
        {"2^y^x", "y", false},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *program = createFlatArithmetic(tests[i].program);
        bool result = dcFlatArithmetic_isExponential(program, tests[i].symbol);
        dcError_assert(result == tests[i].expectedResult);
        dcNode_free(&program, DC_DEEP);
    }
}

static void testIsAlgebraic(void)
{
    struct
    {
        const char *program;
        const char *symbol;
        bool expectedResult;
    } tests[] =
    {
        {"x",     "x", true},
        {"x",     "y", false},
        {"x^2",   "x", true},
        {"x^2",   "y", false},
        {"2^y^x", "y", false},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *program = createFlatArithmetic(tests[i].program);
        bool result = dcFlatArithmetic_isAlgebraic(program, tests[i].symbol);
        dcError_assert(result == tests[i].expectedResult);
        dcNode_free(&program, DC_DEEP);
    }
}

static void testIsLogarithmic(void)
{
    struct
    {
        const char *program;
        const char *symbol;
        bool expectedResult;
    } tests[] =
    {
        {"ln(x)",    "x", true},
        {"ln(x^2)",  "x", true},
        {"log(x)",   "x", true},
        {"log(x^2)", "x", true},
        {"log(x)",   "y", false},
        {"asdf(x)",  "x", false},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *program = createFlatArithmetic(tests[i].program);
        bool result = dcFlatArithmetic_isLogarithmic(program, tests[i].symbol);
        dcError_assert(result == tests[i].expectedResult);
        dcNode_free(&program, DC_DEEP);
    }
}

static void testOrderForIlate(void)
{
    struct
    {
        const char *left;
        const char *right;
        const char *symbol;
        bool expectedResult;
    } tests[] =
    {
        {"cos(2x)", "x",     "x", false},
        {"cos(x)",  "x^2",   "x", false},
        {"x^2",    "cos(x)", "x", true},
        {"e^x",    "x^2",    "x", false},
        {"x^2",    "e^x",    "x", true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *left = createFlatArithmetic(tests[i].left);
        dcNode *right = createFlatArithmetic(tests[i].right);
        bool result = dcFlatArithmetic_orderForIlate
            (left, right, tests[i].symbol);
        dcError_assert(result == tests[i].expectedResult);
        dcNode_free(&left, DC_DEEP);
        dcNode_free(&right, DC_DEEP);
    }
}

static void testEquals(void)
{
    struct
    {
        const char *left;
        const char *right;
        bool expectedResult;
    } tests[] =
    {
        {"x", "x",     true},
        {"x", "x + 1", false},

        {"-2 -x", "-x -2",  true},
        {"-2 -x", "-x + 2", false},

        {"-1 -2x", "-2x - 1", true},
        {"1 -2x",  "-2x - 1", false},

        {"e^(x - 2)", "e^(-1 * (-x + 2))", true},
        {"e^(x - 2)", "e^(-x + 2)",        false},

        {"x / (y + z)", "x / (-(-z - y))",     true},
        {"x / (y + z)", "x / (-2 * (-z - y))", false},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: (%s) / (%s)\n",
                  tests[i].left,
                  tests[i].right);
        dcNode *left = createFlatArithmetic(tests[i].left);
        dcNode *right = createFlatArithmetic(tests[i].right);
        bool result = dcFlatArithmetic_equals(left, right);
        dcError_assert(result == tests[i].expectedResult);
        dcNode_free(&left, DC_DEEP);
        dcNode_free(&right, DC_DEEP);
    }
}

static dcList *createIdentifiers(const char *_symbols)
{
    dcList *stringSymbols = dcLexer_splitString(_symbols, ' ');
    dcList *identifiers = dcList_create();

    FOR_EACH_IN_LIST(stringSymbols, that)
    {
        dcList_push(identifiers,
                    dcIdentifier_createNode
                    (dcString_getString(that->object), NO_FLAGS));
    }

    dcList_free(&stringSymbols, DC_DEEP);
    return identifiers;
}

static void testQuotientAndRemainder(void)
{
    struct
    {
        const char *left;
        const char *right;
        const char *symbols;
        const char *expectedQuotient;
        const char *expectedRemainder;
    } tests[] =
    {
        {"x^3 + 2x^2 + 2x + 1",
         "x + 1",
         "x",
         "x^2 + x + 1",
         "0"},

        {"x",
         "a * b * x + c",
         "x",
         "1 / (a * b)",
         "-c / (a * b)"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: (%s) / (%s)\n",
                  tests[i].left,
                  tests[i].right);
        dcNode *left = createFlatArithmetic(tests[i].left);
        dcNode *right = createFlatArithmetic(tests[i].right);
        dcList *symbols = createIdentifiers(tests[i].symbols);
        dcNode *quotient = NULL;
        dcNode *remainder = NULL;
        bool result = dcFlatArithmetic_dividePolynomials(left,
                                                         right,
                                                         symbols,
                                                         &quotient,
                                                         &remainder);

        if (! result)
        {
            assert(tests[i].expectedQuotient == NULL
                   && tests[i].expectedRemainder == NULL);
        }
        else
        {
            bool error = false;
            if (tests[i].expectedQuotient == NULL)
            {
                fprintf(stderr,
                        "Expected NULL quotient but got: '%s'\n",
                        dcNode_display(quotient));
                error = true;
            }

            if (tests[i].expectedRemainder == NULL)
            {
                fprintf(stderr,
                        "Expected NULL remainder but got: '%s'\n",
                        dcNode_display(remainder));
                error = true;
            }

            assert(! error);
            assert(quotient != NULL);
            assert(remainder != NULL);
            dcTestUtilities_expectStringNodeEqual(tests[i].expectedQuotient,
                                                  quotient);
            dcTestUtilities_expectStringNodeEqual(tests[i].expectedRemainder,
                                                  remainder);
        }

        dcNode_free(&quotient, DC_DEEP);
        dcNode_free(&remainder, DC_DEEP);
        dcList_free(&symbols, DC_DEEP);
        dcList_free(&symbols, DC_DEEP);
        dcNode_free(&left, DC_DEEP);
        dcNode_free(&right, DC_DEEP);
    }
}

static void testPolynomialDivision(void)
{
    struct
    {
        const char *left;
        const char *right;
        const char *symbols;
        const char *expectedResult;
    } tests[] =
    {
        {"1", "a * b * x + c", "x", "NULL??"},

        {"x",
         "a * b * x + c",
         "x",
         "1 / (a * b) + (-c / (a * b)) * (1 / (a * b * x + c))"},

        {"x^x",   "x + 1", "x", "NULL??"},
        {"x + 1", "x^x",   "x", "NULL??"},

        {"x + 1", "x + 1", "x", "1 + 0 * (1 / (x + 1))"},

        {"x", "x", "x", "1 + 0 * (1 / x)"},
        {"x^2", "x", "x", "x + 0 * (1 / x)"},
        {"x^2 + 3", "x", "x", "x + 3 * (1 / x)"},
        {"x^3 + 3", "x", "x", "x^2 + 3 * (1 / x)"},

        {"2x^3 - 3x - 5",
         "x + 2",
         "x",
         "2x^2 - 4x + 5 + -15 * (1 / (x + 2))"},

        {"4x^4 - 10x^2 + 1",
         "x - 6",
         "x",
         "4x^3 + 24x^2 + 134x + 804 + 4825 * (1 / (x - 6))"},

        {"x^3 - 2x^2 - 4", "x - 3", "x", "x^2 + x + 3 + 5 * (1 / (x - 3))"},

        {"5x^3 - x^2 + 6",
         "x - 4",
         "x",
         "5x^2 + 19x + 76 + 310 * (1 / (x - 4))"},

        {"x^2 + 3", "x + 1", "x", "x - 1 + 4 * (1 / (x + 1))"},
        {"x^3 - 12x^2 - 42",
         "x^2 - 2x + 1",
         "x",
         "(x^3 - 12x^2) / (x * x - x - x * 1 + 1) + -42 * (1 / (x - 1)^2)"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: (%s) / (%s)\n",
                  tests[i].left,
                  tests[i].right);
        dcNode *left = createFlatArithmetic(tests[i].left);
        dcNode *right = createFlatArithmetic(tests[i].right);
        dcList *symbols = createIdentifiers(tests[i].symbols);
        bool result = dcFlatArithmetic_dividePolynomials(left,
                                                         right,
                                                         symbols,
                                                         NULL,
                                                         NULL);
        assert(result);
        //char *display = dcNode_synchronizedDisplay(result);

        //dcTestUtilities_expectStringEqual(tests[i].expectedResult, display);

        dcList_free(&symbols, DC_DEEP);
        //dcMemory_free(display);
        //dcNode_free(&result, DC_DEEP);
        dcNode_free(&left, DC_DEEP);
        dcNode_free(&right, DC_DEEP);
    }
}

static void testOrderPolynomial(void)
{
    const SingleTest programs[] =
    {
        {"4x^2 + x^(4 - 1)",        "4x^2 + x^(4 - 1)",    false},
        {"x + x",                   "x + x",               false},
        {"x",                       "x",                   false},
        {"x^x + x^y",               "x^x + x^y",           false},
        {"6x^3 + 4x^2 + x^(3 - 1)",
         "6x^3 + 4x^2 + x^(3 - 1)",
         false},
        {"x^2 + x",                  "x^2 + x",           false},
        {"x^x^2 / y",                "x^(x^2) / y",       false},
        {"x - x^3 + 3x^2",           "-x^3 + 3x^2 + x",   true},
        {"1 + x * -2",               "x * -2 + 1",        true},
        {"x - x^3 - 3x^2",           "-x^3 - 3x^2 + x",   true},
        {"x + x^3 + 3x^2",           "x^3 + 3x^2 + x",    true},
        {"x + x^2",                  "x^2 + x",           true},
        {"(x^2 + x^3) / y",          "(x^3 + x^2) / y",   true},
        {"e^(1 + x^3 + x^2)",        "e^(x^3 + x^2 + 1)", true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        testSingleTest(&programs[i], dcFlatArithmetic_orderPolynomial, true);
    }
}

static void testOrderSubtract(void)
{
    const SingleTest programs[] =
    {
        {"a + b",      "a + b",      false},
        {"a + 3 * b",  "a + 3b",     false},
        {"-a + b",     "b + -a",     true},
        {"-3 + b",     "b + -3",     true},
        {"-3 * a + b", "b + -3a",    true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        testSingleTest(&programs[i], dcFlatArithmetic_orderSubtract, true);
    }
}

static void testMultiplyByDenominator(void)
{
    const SingleTest tests[] =
    {
        {"3",                 "3",                               false},
        {"3 + x",             "3 + x",                           false},
        {"3 / x + x",         "(3 + x * x) / x",                 true},
        {"3 / x + x + 4 / y", "(3y + x * x * y + 4x) / (y * x)", true},
        {"3 / (x + y) + 4",   "(3 + 4 * (x + y)) / (x + y)",     true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i],
                       &dcFlatArithmetic_multiplyByDenominator,
                       true);
    }
}

static void testDegree(void)
{
    struct
    {
        const char *program;
        const char *symbols;
        bool expectedPolynomial;
        int32_t expectedDegree;
    } tests[] =
    {
        {"1 % x",                "x",    false, 0},
        {"x^x",                  "x",    false, 0},
        {"y^(x + 1)",            "x",    false, 0},
        {"(x - 1)^2",            "x",    true,  2},
        {"(x^2 + 2)^2",          "x",    true,  4},
        {"(x^2 + y + 2)^2",      "x y",  true,  4},
        {"3 / x + x",            "x",    true,  1},
        {"0",                    "x",    true,  -1},
        {"x",                    "x",    true,  1},
        {"x - x",                "x",    true,  -1},
        {"x^2",                  "x",    true,  2},
        {"x^4",                  "x",    true,  4},
        {"x^4 + 2x^2",           "x",    true,  4},
        {"x^4 * y^3 + 2x^2",     "x",    true,  4},
        {"1 / x",                "x",    true,  -1},
        {"x / x^2",              "x",    true,  -1},
        {"x / (x^2 + 1)",        "x",    true,  -1},
        {"3 * x^4 * y^3 + 2x^2", "x",    true,  4},
        {"3 * x^4 * y^3 + x",    "x",    true,  4},
        {"3 - 5x + 2x^5 - 7x^9", "x",    true,  9},
        {"3x + 3y",              "x y",  true,  1},
        {"3x * 3y",              "x y",  true,  2},
        {"(y - 3) * (2y + 6) * (-4y - 21)",
         "y",
         true,
         3},
        {"(3z^8 + z^5 - 4z^2 + 6) + (-3z^8 + 8z^4 + 2z^3 + 14z)",
         "z",
         true,
         5},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: (%s)\n",
                  tests[i].program);
        dcNode *program = createFlatArithmetic(tests[i].program);
        dcList *symbols = createIdentifiers(tests[i].symbols);
        int32_t degree = 0;
        bool isPolynomial = dcFlatArithmetic_degree(program,
                                                    symbols,
                                                    &degree);

        if (isPolynomial != tests[i].expectedPolynomial
            || degree != tests[i].expectedDegree)
        {
            fprintf(stderr,
                    "\n\nExpected degree of %d for '%s' but got %d\n"
                    "is polynomial: %s expected polynomial: %s\n",
                    tests[i].expectedDegree,
                    tests[i].program,
                    degree,
                    BOOL_TO_STRING(isPolynomial),
                    BOOL_TO_STRING(tests[i].expectedPolynomial));
            assert(false);
        }

        dcList_free(&symbols, DC_DEEP);
        dcNode_free(&program, DC_DEEP);
    }
}

//static const dcTestFunctionMap sTestMap[] =
//{
//    {NULL}
//};

static void testUnfindableSubstitution(void)
{
    struct
    {
        const char *program;
        const char *symbol;
        bool expectedResult;
    } tests[] =
    {
        {"(a * x * b^3 + c * (1 * b^2 + -1)) "
         "/ (a * b * (b^1 * x * a^1 + c * 1 * 1))",
         "x",
         true // unfindable
        }
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: (%s) | expectedResult: %s\n",
                  tests[i].program,
                  BOOL_TO_STRING(tests[i].expectedResult));
        dcNode *program = createFlatArithmetic(tests[i].program);
        assert(dcFlatArithmetic_unfindableSubstitution(program, tests[i].symbol)
               == tests[i].expectedResult);
        dcNode_free(&program, DC_DEEP);
    }
}

static void testMoveNumberToFront(void)
{
    const SingleTest tests[] =
    {
        {"3",         "3",      false},
        {"3 + x",     "3 + x",  false},
        {"3x",        "3x",     false},
        {"x * 3",     "3x",     true},
        {"x + y * 3", "x + 3y", true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i],
                       &dcFlatArithmetic_moveNumberToFront,
                       true);
    }
}

typedef struct
{
    const char *top;
    const char *bottom;
    bool expectedModified;
    const char *expectedTop;
    const char *expectedBottom;
} TopAndBottomTest;

static void printTopAndBottomTest(const TopAndBottomTest *_test)
{
    fprintf(stderr,
            ("top: %s | "
             "bottom: %s | "
             "modified: %s | "
             "expectedTop: %s | "
             "expectedBottom: %s\n"),
            _test->top,
            _test->bottom,
            BOOL_TO_STRING(_test->expectedModified),
            _test->expectedTop,
            _test->expectedBottom);
}

static void testCancelTopAndBottom(void)
{
    const TopAndBottomTest tests[] =
        {
            // cancel case 1
            {"x",     "x",     true, "1",             "1"},
            // cancel case 2
            // ?
            // cancel case 3
            {"x^3",   "x",     true, "x^(2)",         "1"},
            // cancel case 4
            {"x",     "x^3",   true, "1",             "x^(2)"},
            // cancel case 5
            {"x^2",   "x^3",   true, "1",             "x^1"},
            // cancel case 6
            {"x^3",   "x^2",   true, "x^1",           "1"},
            // cancel case 7
            {"x^y^x", "x^3",   true, "x^((y^x) - 3)", "1"},
            {"x^3",   "x^y^x", true, "x^(3 - (y^x))", "1"},
            // cancel case 8
            {"1",     "1.48",  true, "100",           "148"},
            // cancel case 9
            {"10",    "20",    true, "1",             "2"},
            // cancel case 10
            {"12",    "15",    true, "4",             "5"},
            // cancel case 11
            {"12",    "0",     true, "NaN",           "1"},
            // cancel case 16.1
            {"3",     "-x",    true, "-3", "1x"},
            {"-1 * x", "-y",   true, "1x", "1y"},
            // cancel case 16.2.2
            {"-3", "-y", true, "3", "1y"},
            // cancel case 16.3
            {"y",     "-x",     true, "-y",            "1x"},
            // cancel case 16.4
            {"x", "-3", true, "-x", "3"},
            // cancel case 16.5
            {"x", "-1 * y", true, "-x", "1y"},
            // cancel case 16.6
            {"y",     "-3 * x", true, "-y",            "3x"},
        };

    uint8_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG,
                  "Program: %s\n",
                  tests[i].top);

        dcNode *top = createFlatArithmetic(tests[i].top);
        dcNode *bottom = createFlatArithmetic(tests[i].bottom);
        bool modified = dcFlatArithmetic_cancelTopAndBottom(&top,
                                                            &bottom,
                                                            true);
        char *topDisplay = NULL;
        char *bottomDisplay = NULL;
        bool failure = false;

        if (modified != tests[i].expectedModified)
        {
            fprintf(stderr,
                    "Expected modified: %s but got: %s for test: ",
                    BOOL_TO_STRING(tests[i].expectedModified),
                    BOOL_TO_STRING(modified));
            printTopAndBottomTest(&tests[i]);
            failure = true;
            goto kickout;
        }

        topDisplay = dcNode_synchronizedDisplay(top);
        bottomDisplay = dcNode_synchronizedDisplay(bottom);

        if (strcmp(topDisplay, tests[i].expectedTop) != 0)
        {
            fprintf(stderr,
                    "Expected top: %s but got: %s for test: ",
                    tests[i].expectedTop,
                    topDisplay);
            failure = true;
            goto kickout;
        }

        if (strcmp(bottomDisplay, tests[i].expectedBottom) != 0)
        {
            fprintf(stderr,
                    "Expected bottom: %s but got: %s for test: ",
                    tests[i].expectedBottom,
                    bottomDisplay);
            failure = true;
            goto kickout;
        }

    kickout:
        dcNode_free(&top, DC_DEEP);
        dcNode_free(&bottom, DC_DEEP);
        dcMemory_free(topDisplay);
        dcMemory_free(bottomDisplay);

        if (failure)
        {
            assert(false);
        }
    }
}

/*
static void testIntegrateWithException(void)
{
    const char *programs[] =
    {
        "cos(x) * x",
        //"x * 0.500000 * (1 - cos(2x))",
        //"e^x * sin(x)",
        //"x * sin(x) * tan(x) * cos(x)",
        //"x^3 * e^x^2",
        //"e^x^2 * x^3",
        //"cos(2x) * x",
        //"x * cos(2x) * 0.5",
        //"x * sin(x)^2",
        //"x * sin(x)^2 * cos(x) * tan(x)",
    };

    extern uint32_t exceptionForceCount;
    extern uint32_t exceptionIndex;
    extern bool exceptionForceEnabled;
    size_t i;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    exceptionForceEnabled = true;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcLog_log(FLAT_ARITHMETIC_TEST_LOG, "Program: %s\n", programs[i]);

        size_t j;

        for (j = 0; j < 100; j++)
        {
            exceptionForceEnabled = true;
            exceptionForceCount = j;
            exceptionIndex = 0;

            dcNode *flatArithmetic = createFlatArithmetic(programs[i]);
            dcLog_log(FLAT_ARITHMETIC_TICK_LOG, ".");
            dcNode *result = dcFlatArithmetic_integrate(flatArithmetic, "x");
            assert(result == NULL);
            dcNode_free(&flatArithmetic, DC_DEEP);
            evaluator->exception = NULL;
            dcNodeEvaluator_resetState(evaluator);
        }
    }
}
*/

static void testIsPolynomial(void)
{
    struct
    {
        const char *program;
        bool expectedResult;
    } tests[] =
          {
              {"x",                     true},
              {"x^2",                   true},
              {"x * y",                 true},
              {"sqrt(7)",               true},
              {"x^2 + x",               true},
              {"x^2 + x + 2",           true},
              {"x^2 + (3 + i) * x",     true},
              {"2x^5 - 5*x^3 - 10x + 9", true},
              {"x^y",               false},
              {"e^x",               false},
              {"7^1.5",             false},
              {"x^1.5",             false},
              {"x / y",             false},
              {"x << 3",            false},
              {"x^(-2)",            false},
              {"sqrt(e^x)",         false},
              {"sqrt(e/x)",         false},
              {"sqrt(e + x)",       false},
              {"x^2 + x / y + 2",   false},
              {"x^3 + 2x^1.5 - 7",  false},
              {NULL,                false}
          };

    size_t i;

    for (i = 0; tests[i].program != NULL; i++)
    {
        dcNode *arithmetic = createFlatArithmetic(tests[i].program);
        bool got = dcFlatArithmetic_isPolynomial(arithmetic);

        if (got != tests[i].expectedResult)
        {
            fprintf(stderr,
                    "Got: %u but expected: %u for program: %s\n",
                    got,
                    tests[i].expectedResult,
                    tests[i].program);
            assert(false);
        }

        dcNode_free(&arithmetic, DC_DEEP);
    }
}

static void testFactorQuadratic(void)
{
    struct
    {
        const char *polynomial;
        const char *expectedFactored;
    } tests[] =
          {
              {"9x^2 + 42x + 49",        "(3x + 7)^2"},
              {"9y^2 + 42y + 49",        "(3y + 7)^2"},
              {"4 * x^2 + 12x + 9",      "(2x + 3)^2"},
              {"x^2 + 2x + 1",           "(x + 1)^2"},
              {"x^2 - 2x + 1",           "(x - 1)^2"},
              {"2x + x^2 + 1",           "(x + 1)^2"},
              {"9x^2 + 36x + 36",        "9 * (x + 2)^2"},
              {"361x^2 + 646x + 289",    "(19x + 17)^2"},
              {"x^2 + 4x + 1",           NULL},
              {"x",                      NULL},
              {"x * y",                  NULL},
              {"x^2 - 7x",               NULL},
              {"5x^3 - 3x + 3",          NULL},
              {"9y^2 + 42 * y * x + 49", NULL},
              {NULL}
          };

    size_t i;

    for (i = 0; tests[i].polynomial != NULL; i++)
    {
        dcNode *polynomial = createFlatArithmetic(tests[i].polynomial);
        bool expectedModified = (tests[i].expectedFactored != NULL);
        bool modified = false;
        polynomial = dcFlatArithmetic_factorQuadratic(polynomial, &modified);

        if (modified != expectedModified)
        {
            fprintf(stderr,
                    ("\n\nExpected modified: %s but got: %s "
                     "for polynomial: %s\n\n"),
                    BOOL_TO_STRING(expectedModified),
                    BOOL_TO_STRING(modified),
                    tests[i].polynomial);
            assert(false);
        }

        char *factoredDisplay = dcNode_synchronizedDisplay(polynomial);

        if ((! expectedModified
             && strcmp(factoredDisplay, tests[i].polynomial) != 0)
            || (expectedModified
                && strcmp(factoredDisplay, tests[i].expectedFactored) != 0))
        {
            fprintf(stderr,
                    "\n\nExpected: %s but got: %s for program: %s\n\n",
                    tests[i].expectedFactored,
                    factoredDisplay,
                    tests[i].polynomial);
            assert(false);
        }

        dcMemory_free(factoredDisplay);
        dcNode_free(&polynomial, DC_DEEP);
    }
}

static void testFactorQuadraticMessy(void)
{
    struct
    {
        const char *polynomial;
        const char *expectedFactored;
        bool expectedModified;
    } tests[] =
          {
              {"9.8596 * x^2 + 484.188 * x + 5944.41",
               "0.0004 * (157x + 3855)^2",
               true},

              {"x^2 + 5x - 4",
               ("(x - 0.7015621187164245) * "
                "(x + 5.701562118716425)"),
               true},

              {"11.86389136 * x^2 - 306.55160 * x + 1980.25",
               "1.6E-7 * (8611x - 111250)^2",
               true},

              {NULL}
          };

    size_t i;

    for (i = 0; tests[i].polynomial != NULL; i++)
    {
        dcNode *polynomial = createFlatArithmetic(tests[i].polynomial);
        bool modified = false;
        polynomial =
            dcFlatArithmetic_factorQuadraticWhatever(polynomial, &modified);

        if (modified != tests[i].expectedModified)
        {
            fprintf(stderr,
                    ("\n\nExpected modified: %s but got: %s "
                     "for polynomial: %s\n\n"),
                    BOOL_TO_STRING(tests[i].expectedModified),
                    BOOL_TO_STRING(modified),
                    tests[i].polynomial);
            assert(false);
        }

        char *factoredDisplay = dcNode_synchronizedDisplay(polynomial);

        if ((! tests[i].expectedModified
             && strcmp(factoredDisplay, tests[i].polynomial) != 0)
            || (tests[i].expectedModified
                && strcmp(factoredDisplay, tests[i].expectedFactored) != 0))
        {
            fprintf(stderr,
                    "\n\nExpected: %s but got: %s for program: %s\n\n",
                    tests[i].expectedFactored,
                    factoredDisplay,
                    tests[i].polynomial);
            assert(false);
        }

        dcMemory_free(factoredDisplay);
        dcNode_free(&polynomial, DC_DEEP);
    }
}

static void testHasSingleIdentifier(void)
{
    struct
    {
        const char *polynomial;
        const char *expectedIdentifier;
        bool expectedResult;
    } tests[] =
          {
              {"x",                 "x",  true},
              {"x^2",               "x",  true},
              {"e^x",               "x",  true},
              {"PI^x",              "x",  true},
              {"x^PI + 3x^e",       "x",  true},
              {"y + 2y + 44y^234",  "y",  true},
              {"x^2 + 2x + x",      "x",  true},
              {"x + sin(x)",        "x",  true},
              {"x + sin(x^2)",      "x",  true},
              {"x + sin(2^x)",      "x",  true},
              {"x * y",             NULL, false},
              {"x^2 + 2y + x",      NULL, false},
              {"x + sin(y)",        NULL, false},
              {"x + sin(2^y)",      NULL, false},
              {NULL}
          };

    size_t i;

    for (i = 0; tests[i].polynomial != NULL; i++)
    {
        dcNode *polynomial = createFlatArithmetic(tests[i].polynomial);
        dcNode *identifier = NULL;
        bool result = dcFlatArithmetic_hasSingleIdentifier(polynomial,
                                                           &identifier);

        if (result != tests[i].expectedResult)
        {
            fprintf(stderr,
                    ("\n\nExpected result: %s but got: %s "
                     "for polynomial: %s\n\n"),
                    BOOL_TO_STRING(tests[i].expectedResult),
                    BOOL_TO_STRING(result),
                    tests[i].polynomial);
            assert(false);
        }

        if (result)
        {
            assert(identifier != NULL);
            assert(IS_IDENTIFIER(identifier));
        }
        else
        {
            assert(identifier == NULL);
        }

        if (identifier == NULL
            && tests[i].expectedIdentifier == NULL)
        {
            // pass
        }
        else if (identifier == NULL
                 || tests[i].expectedIdentifier == NULL
                 || ! dcIdentifier_equalsString(identifier,
                                                tests[i].expectedIdentifier))
        {
            fprintf(stderr,
                    "\n\nExpected: %s but got: %s for program: %s\n\n",
                    tests[i].expectedIdentifier,
                    dcIdentifier_getName(identifier),
                    tests[i].polynomial);
            assert(false);
        }

        dcNode_free(&polynomial, DC_DEEP);
    }
}

static void testCollectPowers(void)
{
    SingleTest tests[] =
    {
        {"x",                     "x",               false},
        {"x / y",                 "x / y",           false},
        {"x^2 / y",               "x^2 / y",         false},
        {"x^(2 + z) / y^2",       "x^(2 + z) / y^2", false},
        {"x^2 / y^2",             "(x / y)^2",       true},
        {"x^(2 + z) / y^(2 + z)", "(x / y)^(2 + z)", true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i], &dcFlatArithmetic_collectPowers, true);
    }
}

static void testGetOrderedPolynomialCoefficients(void)
{
    struct
    {
        const char *arithmetic;
        const char *expectedCoefficients;
    } tests[] =
      {
          {"x^3 + 2x + 1",           "1 0 2 1"},
          {"x^3 + 2x^2 + 11",        "1 2 0 11"},
          {"x^3 + 11",               "1 0 0 11"},
          {"11",                     "11"},
          {"11.55",                  "11.55"},
          {"y^5 + 33y",              "1 0 0 0 33 0"},
          {"y^5 - 33y",              "1 0 0 0 -33 0"},
          {"y^5 - 33.1y",            "1 0 0 0 -33.1 0"},
          {"5y^5 - 4y^3 + 33y - 12", "5 0 -4 0 33 -12"}
      };

    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *arithmetic = createFlatArithmetic(tests[i].arithmetic);
        arithmetic = dcFlatArithmetic_convertSubtractToAdd(arithmetic, NULL);

        dcArray *coefficients =
            dcFlatArithmetic_getOrderedPolynomialCoefficients(arithmetic,
                                                              false);
        dcList *expectedCoefficientsList =
            dcLexer_splitString(tests[i].expectedCoefficients, ' ');

        if (coefficients->size != expectedCoefficientsList->size)
        {
            fprintf(stderr,
                    "\n\nError: wanted size: %u but got size: %u\n",
                    expectedCoefficientsList->size,
                    coefficients->size);
            fprintf(stderr,
                    "Expected: %s but got: %s\n\n",
                    tests[i].expectedCoefficients,
                    dcArray_display(coefficients));
            assert(false);
        }

        dcArray *expectedCoefficients =
            dcArray_createFromList(expectedCoefficientsList, DC_SHALLOW);
        dcList_free(&expectedCoefficientsList, DC_SHALLOW);

        uint32_t j;

        for (j = 0; j < coefficients->size; j++)
        {
            char *got = dcNode_synchronizedDisplay(coefficients->objects[j]);
            char *wanted = (dcString_getString
                            (expectedCoefficients->objects[coefficients->size
                                                           - j
                                                           - 1]));
            if (strcmp(got, wanted) != 0)
            {
                fprintf(stderr,
                        "\n\nError: got coefficient: %s but wanted: %s, "
                        "index %u\n\n",
                        got,
                        wanted,
                        j);
                fprintf(stderr,
                        "Expected: %s but got: %s\n\n",
                        tests[i].expectedCoefficients,
                        dcArray_display(expectedCoefficients));
                assert(false);
            }

            dcMemory_free(got);
        }

        dcArray_free(&coefficients, DC_DEEP);
        dcArray_free(&expectedCoefficients, DC_DEEP);
        dcNode_free(&arithmetic, DC_DEEP);
    }
}

static void testGetCoefficientsFail(void)
{
    struct
    {
        const char *arithmetic;
    } tests[] =
      {
          {"3.5 * x^3 + 2x + 1"},
          {"1.1"},
          {"x^3 + 1.5x"},
      };

    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *arithmetic = createFlatArithmetic(tests[i].arithmetic);
        dcArray *coefficients =
            dcFlatArithmetic_getOrderedPolynomialCoefficients(arithmetic, true);
        assert(coefficients == NULL);
        dcNode_free(&arithmetic, DC_DEEP);
    }
}

static void testFactorPolynomialByRationalRoots(void)
{
    SingleTest tests[] =
    {
        {"24 * x^3 + 12 * x^2 + 8 * x + 4",
         "(2x + 1) * (12x^2 + 4)",
         true},
        {"x^3 + 2x^2 + 2x + 1",
         "(x + 1) * (x^2 + x + 1)",
        true},
        {"2x^3 + 3x - 5",
         "(-x + 1) * (-2x^2 - 2x - 5)",
        true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *program = createFlatArithmetic(tests[i].program);
        program = dcFlatArithmetic_convertSubtractToAdd(program, NULL);

        testSingleTestWithProgram
            (&tests[i],
             &dcFlatArithmetic_factorPolynomialByRationalRoots,
             true,
             program);
    }
}

static void testDistributeLikeAMadman(void)
{
    SingleTest tests[] =
    {
        {"x",                     "x",               false},
        {"x / y",                 "x / y",           false},
        {"x^(2 + z) / y^2",       "x^(2 + z) / y^2", false},
        {"x^2 / y",               "x^2 / y",         false},
        {"x^3",                   "x^3",             false},
        {"(x + 1)^4",
         "(x + 1) * (x + 1) * (x + 1) * (x + 1)",
         true},
        {"(x + 1) * x^2 + (x + 1)^2",
         "(x + 1) * x^2 + (x + 1) * (x + 1)",
         true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i],
                       &dcFlatArithmetic_distributeLikeAMadman,
                       true);
    }
}

static void testMaxPower(void)
{
    struct
    {
        const char *program;
        uint32_t expectedPower;
        bool expectedResult;
    } tests[] =
      {
          {"x * (x^2 + y)",       2, true},
          {"x * (x^2 * x^3 + y)", 3, true},
          {"x / x^3",             3, true},
          {"x / x",               0, false}
      };

    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *program = createFlatArithmetic(tests[i].program);
        uint32_t power = 0;
        bool gotIt = dcFlatArithmetic_maxPower(program, &power);

        assert(gotIt == tests[i].expectedResult);

        if (gotIt
            && power != tests[i].expectedPower)
        {
            fprintf(stderr, "Wanted power: %u but got: %u for program: %s\n",
                    tests[i].expectedPower,
                    power,
                    tests[i].program);
        }

        dcNode_free(&program, DC_DEEP);
    }
}

static void testMoveLeftAndRight(void)
{
    struct
    {
        const char *arithmetic;
        const char *symbol;
        const char *expectedLeft;
        const char *expectedRight;
    } tests[] =
          {
              {"x + 2",     "x", "x",   "0 - 2"},
              {"x + y",     "x", "x",   "0 - y"},
              {"x - y",     "x", "x",   "0 - -y"},
              {"x / y",     "x", "x",   "1 / (1 / y)"},
              // fix me
              //{"x + y / x", "x", "x^2", "0 - y"},
          };

    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *arithmetic = createFlatArithmetic(tests[i].arithmetic);
        dcNode *left = NULL;
        dcNode *right = NULL;
        dcFlatArithmetic_moveLeftAndRight(arithmetic,
                                          tests[i].symbol,
                                          &left,
                                          &right);
        char *leftDisplay = dcNode_synchronizedDisplay(left);
        char *rightDisplay = dcNode_synchronizedDisplay(right);

        if (strcmp(leftDisplay, tests[i].expectedLeft) != 0)
        {
            fprintf(stderr,
                    "Expected left: %s but got: %s for test: %s\n",
                    tests[i].expectedLeft,
                    leftDisplay,
                    tests[i].arithmetic);
            assert(false);
        }

        if (strcmp(rightDisplay, tests[i].expectedRight) != 0)
        {
            fprintf(stderr,
                    "Expected right: %s but got: %s for test: %s\n",
                    tests[i].expectedRight,
                    rightDisplay,
                    tests[i].arithmetic);
            assert(false);
        }

        dcMemory_free(leftDisplay);
        dcMemory_free(rightDisplay);

        dcNode_free(&arithmetic, DC_DEEP);
        dcNode_free(&left, DC_DEEP);
        dcNode_free(&right, DC_DEEP);
    }
}

static void testFactorPolynomialByGcd(void)
{
    const SingleTest tests[] =
    {
        {"x^2 + 1", "x^2 + 1",      false},
        {"4x + -2", "2 * (2x - 1)", true},

        {"-4 * x^2 + 4x + 4",
         "4 * (-x^2 + x + 1)",
         true},

        {"2.0 * x^2 + 8.1x + 4",
         "2.0x^2 + 8.1 * x + 4",
         false},

        {"x^2 + 2x + 1", "x^2 + 2x + 1", false},
        {"2 * x^2 + 2x + 2",
         "2 * (x^2 + x + 1)",
         true},
        {"3x^3 - 6x^2 + 15x - 30",
         "3 * (x^3 - 2x^2 + 5x - 10)",
         true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i],
                       &dcFlatArithmetic_factorPolynomialByGcd,
                       true);
    }
}

static void testFactorPolynomialByGrouping(void)
{
    const SingleTest tests[] =
    {
        {"x^3 - x^2 - 2x - 1",
         "x^3 - x^2 - 2x - 1",
         false},
        {"x^2 + a * b - a * x - b * x",
         "-(x - b) * (a - x)",
         true},
        {"x^3 + 2x^2 - 9x - 18",
         "(x^2 - 9) * (x + 2)",
         true},
        {"4x^2 + 20x - 3x * y - 15y",
         "(4x - 3y) * (x + 5)",
         true},
        {"x^3 - 5x^2 + 3x - 15",
         "(x^2 + 3) * (x - 5)",
         true},
        {"3x^3 - 6x^2 + 15x - 30",
         "3 * (x^2 + 5) * (x - 2)",
         true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i],
                       &dcFlatArithmetic_factorPolynomialByGrouping,
                       true);
    }
}

static void testPrint(void)
{
    Program tests[] =
    {
        {"2x^2",          "2x^2"},
        {"2 * [x foo]^2", "2 * [x foo]^2"}
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *program = createFlatArithmetic(tests[i].program);
        char *displayed = dcNode_synchronizedDisplay(program);
        dcTestUtilities_expectStringEqual(displayed, tests[i].expected);
        dcNode_free(&program, DC_DEEP);
        dcMemory_free(displayed);
    }
}

static void testFactorDifferenceOfSquares(void)
{
    const SingleTest tests[] =
    {
        {"x^3 - 16", "x^3 - 16",              false},
        {"x^2 + 16", "x^2 + 16",              false},
        {"x^2 - 4",  "(x + 2) * (x - 2)",     true},
        {"x^2 - 16", "(x + 4) * (x - 4)",     true},
        {"x^4 - 16", "(x^2 + 4) * (x^2 - 4)", true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i],
                       &dcFlatArithmetic_factorDifferenceOfSquares,
                       true);
    }
}

static void testFactorDifferenceOfCubes(void)
{
    const SingleTest tests[] =
    {
        {"27x^3 + 1", "(3x + 1) * (9x^2 - 3x + 1)", true},
        {"x^3 - 8", "(x - 2) * (x^2 + 2x + 4)", true},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        testSingleTest(&tests[i],
                       &dcFlatArithmetic_factorDifferenceOfCubes,
                       true);
    }
}

static const dcTestFunctionMap sTodoTests[] =
{
    {"Factor",                       &testFactor},
    {"Polynomial Division",          &testPolynomialDivision},
    {"Factor Difference of Cubes",   &testFactorDifferenceOfCubes}
};

static const dcTestFunctionMap sMap[] =
{
    {"Solve",                        &testSolve},
    {"Single Node",                   &testSingleNode},
    {"Two Nodes",                     &testTwoNodes},
    {"Three Nodes",                   &testThreeNodes},
    {"Add + Multiply",                &testAddAndMultiply},
    {"Multiply + Add",                &testMultiplyAndAdd},
    {"Add + Raise",                   &testAddAndRaise},
    {"Add + 0 Snip",                  &testAddAndZeroSnip},
    {"0 + 0 Snip",                    &testZeroPlusZeroSnip},
    {"Multiply + 1 Snip",             &testMultiplyAndOneSnip},
    {"Divide + 1 Snip",               &testDivideAndOneSnip},
    {"Raise + 1 Snip",                &testRaiseAndOneSnip},
    {"Add + Multiply + 1 Snip",       &testAddAndMultiplyAndOneSnip},
    {"Add + Identifier",              &testAddAndIdentifier},
    {"Add + Multiply Vanish",         &testAddAndMultiplyVanish},
    {"Multiply + Add 1 Vanish",       &testMultiplyAndAdd1Vanish},
    {"Subtract",                      &testSubtract},
    {"Subtract and Raise",            &testSubtractAndRaise},
    {"Subtract + Add",                &testSubtractAndAdd},
    {"5 - 0 Snip",                    &testFiveMinusZeroSnip},
    {"0 - 5 Snip",                    &testZeroMinusFiveSnip},
    {"Single Node Comparison",        &testSingleNodeComparison},
    {"Two Node Comparison",           &testTwoNodeComparison},
    {"Add Combine",                   &testAddCombine},
    {"Multiply Combine",              &testMultiplyCombine},
    {"Raise Combine",                 &testRaiseCombine},
    {"Method Call Combine",           &testMethodCallCombine},
    {"Combine Fail",                  &testCombineFail},
    {"Convert",                       &testConvert},
    {"Expand Raise",                  &testExpandRaise},
    {"Distribute Divide",             &testDistributeDivide},
    {"Distribute",                    &testDistribute},
    {"Distribute like a Madman",      &testDistributeLikeAMadman},
    {"Equals",                        &testEquals},
    {"Merge",                         &testMerge},
    {"Add Complex Numbers",           &testAddComplexNumbers},
    {"Subtract Complex Numbers",      &testSubtractComplexNumbers},
    {"Multiply Complex Numbers",      &testMultiplyComplexNumbers},
    {"Divide Complex Numbers",        &testDivideComplexNumbers},
    {"Add with Two Multiplies",       &testAddWithTwoMultiplies},
    {"Multiply with Two Adds",        &testMultiplyWithTwoAdds},
    {"Left Shift",                    &testLeftShift},
    {"Right Shift",                   &testRightShift},
    {"Bit And",                       &testBitAnd},
    {"Bit Or",                        &testBitOr},
    {"Sorts",                         &testSorts},
    {"Print",                         &testPrint},
    {"Pretty Print",                  &testPrettyPrint},
    {"Symbol Count",                  &testSymbolCount},
    {"Find Identifier",               &testFindIdentifier},
    {"Expand Divide",                 &testExpandDivide},
    {"Multiply by Denominator",       &testMultiplyByDenominator},
    {"Display",                       &testDisplay},
    {"Contains Identifier",           &testContainsIdentifier},
    {"Find",                          &testFind},
    {"Move Number to Front",          &testMoveNumberToFront},
    {"Order Subtract",                &testOrderSubtract},
    {"Order Polynomial",              &testOrderPolynomial},
    {"Move Left and Right",           &testMoveLeftAndRight},
    {"Max Power",                     &testMaxPower},
    {"Degree",                        &testDegree},
    {"Quotient and Remainder",        &testQuotientAndRemainder},
    {"Convert Divide to Multiply",    &testConvertDivideToMultiply},
    {"Convert Subtract to Add",       &testConvertSubtractToAdd},
    {"Undo Convert Subtract to Add",  &testUndoConvertSubtractToAdd},
    {"Compare Node",                  &testCompareNode},
    {"Combine",                       &testCombine},
    {"Simplify Method Call",          &testSimplifyMethodCall},
    {"Cancel!",                       &testCancel},
    {"Cancel Top and Bottom",         &testCancelTopAndBottom},
    {"Get Ordered Polynomial Coefficients",
     &testGetOrderedPolynomialCoefficients},
    {"Get Coefficients Fail",         &testGetCoefficientsFail},
    {"Collect Powers",                &testCollectPowers},
    {"Has Single Identifier",         &testHasSingleIdentifier},
    {"Factor Quadratic",              &testFactorQuadratic},
    {"Factor Quadratic Messy",        &testFactorQuadraticMessy},
    {"Factor Polynomial by Grouping", &testFactorPolynomialByGrouping},
    {"Factor Difference of Squares",  &testFactorDifferenceOfSquares},
    {"Factor Polynomial by GCD",      &testFactorPolynomialByGcd},
    {"Factor Polynomial by Rational Roots",
    &testFactorPolynomialByRationalRoots},
    {"Is Polynomial",                 &testIsPolynomial},
    {"Shrink!",                       &testShrink},
    {"Complex Raise",                 &testComplexRaise},
    {"Choose",                        &testChoose},
    {"Equalities",                    &testEqualities},
    {"Derive!",                       &testDerive},
    {"Repeated derivations",          &testRepeatedDerivations},
    {"Expand",                        &testExpand},
    {"Is Exponential",                &testIsExponential},
    {"Is Algebraic",                  &testIsAlgebraic},
    {"Is Inverse Trigonometric",      &testIsInverseTrigonometric},
    {"Is Logarithmic",                &testIsLogarithmic},
    {"Is Trigonometric",              &testIsTrigonometric},
    {"Simplify Trigonometry",         &testSimplifyTrigonometry},
    {"Order for ILATE",               &testOrderForIlate},
    {"Remove for Substitution",       &testRemoveForSubstitution},
    {"Unfindable Substitution",       &testUnfindableSubstitution},
    {"Find And Replace Identifier",   &testFindAndReplaceIdentifier},
    {"Substitution",                  &testSubstitution},
    {"Integrate!",                    &testIntegrate},
    {"Integrate with Substitution",   &testIntegrateWithSubstitution},
    {"Remove by Parts",               &testRemoveByParts},
    {"Integrate by Parts",            &testIntegrateByParts},
    //{"Integrate with Exception",    &testIntegrateWithException}, // TODO
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();

    if (dcSystem_createWithArguments
        (dcTaffyCommandLineArguments_parseAndCreateWithFailure(_argc,
                                                               _argv,
                                                               false))
        != NULL)
    {
        dcTestUtilities_go("Flat Arithmetic Test",
                           _argc,
                           _argv,
                           NULL,
                           sMap,
                           true);
        dcSystem_free();
    }

    dcMemory_deinitialize();
    return 0;
}
