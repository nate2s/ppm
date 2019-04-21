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

#include "dcError.h"
#include "dcLexer.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"
#include "dcPieLineEvaluator.h"
#include "dcNodeEvaluator.h"
#include "dcString.h"

#include "src/org/taffy/help/CompiledHelpSystem.h"
#include "src/org/taffy/help/CompiledImportHelperHelpTopic.h"
#include "src/org/taffy/help/CompiledMainHelpTopic.h"
#include "src/org/taffy/help/warranty/CompiledGnuLesserTermsAndConditionsHelpTopic.h"
#include "src/org/taffy/help/warranty/CompiledGnuTermsAndConditionsHelpTopic.h"
#include "src/org/taffy/help/warranty/CompiledGnuWarrantyHelpTopic.h"

static uint32_t sLineNumber = 1;
static const char *sIgnoreResult = "this result shall be ignored";
static const char *sNoExceptionResult = "this result is no exception";
static dcPieLineEvaluator *sPieLineEvaluator = NULL;

static void expect(const char *_input,
                   const char *_expectedResult,
                   bool _newLine,
                   bool _continuation)
{
    char *result = dcPieLineEvaluator_evaluateLine
        (sPieLineEvaluator, _input, NULL, 0, NULL);

    if (_newLine || _continuation)
    {
        sLineNumber++;
    }

    char *prompt = dcPieLineEvaluator_getPrompt(sPieLineEvaluator, "pie", true);
    char *expectedPrompt =
        dcLexer_sprintf("pie.%u>%s ",
                        sLineNumber,
                        (_continuation
                         ? ">"
                         : ""));

    if ((_expectedResult == NULL
         && result == NULL)
        || _expectedResult == sIgnoreResult)
    {
        // pass
    }
    else if (result == NULL
             && _expectedResult == sNoExceptionResult)
    {
        dcError("** failed, got exception");
    }
    else if (_expectedResult == NULL
             && result != NULL)
    {
        dcError("** failed, expectedResult == NULL, result != NULL");
    }
    else if (_expectedResult != NULL
             && result == NULL)
    {
        dcError("** failed, NULL result\n");
    }
    else if (strcmp(result, _expectedResult) != 0)
    {
        dcError("** failed expect, wanted:\n'%s'\nbut got:\n'%s'\n",
                _expectedResult,
                result);
    }

    if (strcmp(prompt, expectedPrompt) != 0)
    {
        dcError("** failed prompt expect, wanted: '%s', but got: '%s'\n",
                expectedPrompt,
                prompt);
    }

    dcMemory_free(expectedPrompt);
    dcMemory_free(result);
    dcMemory_free(prompt);
}

static void testNewlineNoNewLine(void)
{
    expect("", NULL, false, false);
    expect("\n", NULL, false, false);
}

static void testSameLineAddition(void)
{
    expect("3+3", "6", true, false);
}

static void testMultiLineAddition(void)
{
    expect("3+", NULL, false, true);
    expect("3",  "6",  true,  false);

    expect("+", NULL, false, true);
    expect("3", "3",  true,  false);
}

static void testMultiLineSubtract(void)
{
    expect("3-", NULL, false, true);
    expect("2",  "1",  true,  false);

    expect("-", NULL, false, true);
    expect("3", "-3", true,  false);
}

static void testMultiLineMultiply(void)
{
    expect("3 *", NULL, false,  true);
    expect("4",   "12",  true,  false);
}

static void testMultiLineDivide(void)
{
    expect("3 /", NULL,   false,  true);
    expect("6",   "0.5",  true,   false);
}

static void testMultiLineEqual(void)
{
    expect("a =", NULL, false,  true);
    expect("3",   "3",  true,   false);
}

static void testMultiLineAnd(void)
{
    expect("3 &", NULL, false,  true);
    expect("1",   "1",  true,   false);
}

static void testMultiLineRaise(void)
{
    expect("3 ^", NULL, false,  true);
    expect("3",   "27", true,   false);
}

static void testMultiLineLessThan(void)
{
    expect("3 <", NULL,   false,  true);
    expect("4",   "true", true,   false);
}

static void testMultiLineGreaterThan(void)
{
    expect("3 >", NULL,    false,  true);
    expect("4",   "false", true,   false);
}

static void testMultiLineLeftShift(void)
{
    expect("3 <<", NULL,   false,  true);
    expect("2",    "12",   true,   false);
}

static void testMultiLineRightShift(void)
{
    expect("4 >>", NULL, false,  true);
    expect("1",   "2",   true,   false);
}

static void testMultiLineArray(void)
{
    expect("[1,", NULL,           false, true);
    expect("2,",  NULL,           false, true);
    expect("3,",  NULL,           false, true);
    expect("4]",  "[1, 2, 3, 4]", true, false);

    expect("[1,", NULL,     false, true);
    expect("2",   NULL,     false, true);
    expect("]",   "[1, 2]", true,  false);

    // comment in the middle
    expect("[1,", NULL, false, true);
    expect("~(",          NULL,     false, true);
    expect("asdfasdf)~",  NULL,     false, true);
    expect("2]",          "[1, 2]", true,  false);
}

static void testMultiLineHash(void)
{
    expect("(1 => 2,", NULL, false, true);
    expect("3 =>",     NULL, false, true);
    expect("4,",       NULL, false, true);
    expect("5=>6)",    "#Hash(1=>2, 3=>4, 5=>6)", true,  false);

    expect("(1 =>", NULL,          false, true);
    expect("2",     NULL,          false, true);
    expect(")",     "#Hash(1=>2)", true,  false);

    expect("(",    NULL,          false, true);
    expect("1 =>", NULL,          false, true);
    expect("2",    NULL,          false, true);
    expect(")",    "#Hash(1=>2)", true,  false);

    expect("(",    NULL,          false, true);
    expect("1",    NULL,          false, true);
    expect("=> 2", NULL,          false, true);
    expect(")",    "#Hash(1=>2)", true,  false);

    expect("(",    NULL,          false, true);
    expect("1",    NULL,          false, true);
    expect("=>",   NULL,          false, true);
    expect("2", NULL,             false, true);
    expect(")",    "#Hash(1=>2)", true,  false);

    // comment in the middle
    expect("(1 => 2,",    NULL,                false, true);
    expect("~(",          NULL,                false, true);
    expect("asdfasdf)~",  NULL,                false, true);
    expect("3 => 4)",     "#Hash(1=>2, 3=>4)", true,  false);
}

static void testMultiLineString(void)
{
    expect("\"this is a test", NULL, false, true);
    expect("of the emergency broadcasting system\"",
           "\"this is a test\nof the emergency broadcasting system\"",
           true,
           false);

    expect("\"this also is a test", NULL, false, true);
    expect("of the emergency broadcasting system", NULL, false, true);
    expect("\"",
           "\"this also is a test\nof the emergency broadcasting system\n\"",
           true,
           false);

    expect("\"", NULL, false, true);
    expect("this seriously is a test", NULL, false, true);
    expect("of the emergency broadcasting system", NULL, false, true);
    expect("\"",
           "\"\n"
           "this seriously is a test\n"
           "of the emergency broadcasting system\n"
           "\"",
           true,
           false);
}

static void testMultiLineComment(void)
{
    expect("~(",     NULL,  false, true);
    expect("asdf)~", "nil", true,  false);

    expect("~(",  NULL,  false, true);
    expect("3",   NULL,  false, true);
    expect("4)~", "nil", true,  false);

    expect("~(", NULL,  false, true);
    expect("[",  NULL,  false, true);
    expect(")~", "nil", true,  false);

    expect("~(", NULL,  false, true);
    expect("]",  NULL,  false, true);
    expect(")~", "nil", true,  false);

    expect("~(", NULL,  false, true);
    expect("(",  NULL,  false, true);
    expect(")~", "nil", true,  false);

    expect("~(", NULL,  false, true);
    expect(")",  NULL,  false, true);
    expect(")~", "nil", true,  false);

    expect("~(", NULL,  false, true);
    expect("'",  NULL,  false, true);
    expect(")~", "nil", true,  false);

    expect("~(", NULL,  false, true);
    expect("\"", NULL,  false, true);
    expect(")~", "nil", true,  false);
}

static void testClassCrash1(void)
{
    expect("a = \"hi\"", "\"hi\"", true, false);
    expect("class Test(a) {}", sIgnoreResult, true, false);
    expect("class A {}", "#A-meta", true, false);
}

static void testClassInvalidException(void)
{
    expect("a = \"hi there\"", "\"hi there\"", true, false);
    expect("class Test(a) {}", sIgnoreResult, true, false);
    expect("class A {}", "#A-meta", true, false);
    expect("class Test(A) {}", "#Test-meta", true, false);
}

static void testInvalidAsStringClass(void)
{
    expect("class Tester { (@) asString {} }", "#Tester-meta", true, false);
    expect("new Tester",
           "Uncaught Exception: "
           "InvalidCastException: \"invalid cast from: Nil to: String\"",
           true,
           false);
    expect("new Tester",
           "Uncaught Exception: "
           "InvalidCastException: \"invalid cast from: Nil to: String\"",
           true,
           false);
}

static void testHelp(void)
{
    expect("%help", "nil", true, false);
}

static void testResidualParseError(void)
{
    expect("f(x) = (x^2 + y 2x + 1) / (x + 1)",
           "Uncaught Exception: ParseErrorException: \"2\"\n"
           "    near <console>: 2, line 111",
           true,
           false);
    expect("f(x) = (x^2 + y+ 2x + 1) / (x + 1)",
           "F(x) = (x^2 + y + 2x + 1) / (x + 1)",
           true,
           false);
}

static void testLineEndingInBackslash(void)
{
    expect("3 \\", NULL, false,  true);
    expect("+ 4",  "7",  true,  false);

    expect("3 \\",  NULL, false,  true);
    expect("+ 4\\", NULL, false,  true);
    expect("+ 7",   "14", true,   false);
}

static const dcTestFunctionMap sTests[] =
{
    {"Help",                          &testHelp},
    {"Same Line Addition",            &testSameLineAddition},
    {"Multi-Line Addition",           &testMultiLineAddition},
    {"Multi-Line Subtract",           &testMultiLineSubtract},
    {"Multi-Line Multiply",           &testMultiLineMultiply},
    {"Multi-Line Divide",             &testMultiLineDivide},
    {"Multi-Line Equal",              &testMultiLineEqual},
    {"Multi-Line And",                &testMultiLineAnd},
    {"Multi-Line Raise",              &testMultiLineRaise},
    {"Multi-Line Less Than",          &testMultiLineLessThan},
    {"Multi-Line Greater Than",       &testMultiLineGreaterThan},
    {"Multi-Line Left Shift",         &testMultiLineLeftShift},
    {"Multi-Line Right Shift",        &testMultiLineRightShift},
    {"Line Ending in Backslash",      &testLineEndingInBackslash},
    {"Newline no New Line",           &testNewlineNoNewLine},
    {"Multi-Line Array",              &testMultiLineArray},
    {"Multi-Line Hash",               &testMultiLineHash},
    {"Multi-Line String",             &testMultiLineString},
    {"Multi-Line Comment",            &testMultiLineComment},
    {"Class Crash 1",                 &testClassCrash1},
    {"Class Crash Invalid Exception", &testClassInvalidException},
    {"Invalid asString Class",        &testInvalidAsStringClass},
    {"Residual Parse Error",          &testResidualParseError},
    {NULL}
};

static void loadHelpSystem(void)
{
    dcPieLineEvaluator_loadHelp();

    const char *helpStrings[] =
        {
            __compiledGnuLesserTermsAndConditionsHelpTopic,
            __compiledGnuTermsAndConditionsHelpTopic,
            __compiledImportHelperHelpTopic,
            __compiledMainHelpTopic,
            __compiledHelpSystem
        };

    assert(dcStringEvaluator_evalStringArray(helpStrings,
                                             dcTaffy_countOf(helpStrings))
           != NULL);
}

int main(int _argc, char **_argv)
{
    dcSystem_create();
    sPieLineEvaluator = dcPieLineEvaluator_create();

    loadHelpSystem();

    dcTestUtilities_go("Pie Test", _argc, _argv, NULL, sTests, true);
    dcPieLineEvaluator_free(&sPieLineEvaluator);
    dcSystem_free();
    return 0;
}
