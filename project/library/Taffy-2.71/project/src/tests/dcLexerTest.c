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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcCallStackData.h"
#include "dcClass.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcFileManagement.h"
#include "dcGarbageCollector.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcParser.h"
#include "dcStringEvaluator.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcThread.h"
#include "dcThreadInclude.h"
#include "dcTestUtilities.h"

#define LEXER_TEST_FILE_NAME "dcLexerTest.c"
static dcStringId sLexerTestFilenameId = 0;
static dcNodeEvaluator *sNodeEvaluator = NULL;

static void testExceptionLineNumber(const char *_program,
                                    uint32_t _wantedLine,
                                    bool _wantExceptionInParse)
{
    dcNode *head = dcParser_parseString(_program, LEXER_TEST_FILE_NAME, true);

    if (_wantExceptionInParse)
    {
        dcTestUtilities_assert(head == NULL);
    }
    else
    {
        dcTestUtilities_assert(head != NULL);
        dcTestUtilities_assert(dcNodeEvaluator_evaluate(sNodeEvaluator, head)
                               == NULL);
    }

    uint32_t exceptionLine =
        CAST_CALL_STACK_DATA
        (sNodeEvaluator->exceptionCallStack->tail->object)->lineNumber;

    if (exceptionLine != _wantedLine)
    {
        fprintf(stderr,
                "exceptionLine %u != wanted line %u\n",
                exceptionLine,
                _wantedLine);

        fprintf(stderr,
                "exception: %s\n",
                dcNode_synchronizedDisplay(sNodeEvaluator->exception));

        dcError_assert(false);
    }

    dcNode_free(&head, DC_DEEP);
    dcNodeEvaluator_clearException(sNodeEvaluator, DC_SHALLOW);
}

static void runLexerTest(const char *_program)
{
    dcMemory_useMemoryRegions();
    dcLexer *lexer = dcLexer_createWithInput("dcLexerTest.c",
                                             (char *)_program,
                                             false);
    TOKEN token = dcLexer_lex(lexer);

    while (token != EOF)
    {
        token = dcLexer_lex(lexer);
    }

    assert(lexer->states->size == 0);
    assert(! lexer->errorState);
    assert(! lexer->parseError);
    dcMemory_useMalloc();
    dcMemory_freeMemoryRegions(DC_DEEP);
}

static void testStringWithIdentifier(void)
{
    runLexerTest("\"'hiThere\"");
    runLexerTest("\"'#[self]\"");
    runLexerTest("[kernel eval: \"'#[self]\"]");
}

static void testMethodCallWithBlock(void)
{
    runLexerTest("[hi there: ^{ 1 + 1 }\n"
                 "     here: 2]");
}

static void testBlockWithArguments(void)
{
    runLexerTest("a = ^{ <one, two, three>\n"
                 "    one + two + three\n"
                 "}");

    runLexerTest("a = ^{ <one, two, three> "
                 "    one + two + three "
                 "}");
}

static void testMethodWithBlock(void)
{
    runLexerTest("(@) asFlatArray\n"
                 "#const,\n"
                 "#synchronizedRead\n"
                 "{\n"
                 "    result = [Array createWithSize: [self size] * 2]\n"
                 "    k = 0\n"
                 "         \n"
                 "    [self each: ^{ <pair>\n"
                 "        result[k] = [pair left]\n"
                 "        result[k + 1] = [pair right]\n"
                 "        k += 2\n"
                 "    }]\n"
                 "}\n");
}

static void testXor(void)
{
    runLexerTest("5 ^^ 6");
}

static void testXorEqual(void)
{
    runLexerTest("a ^^= 6");
}

static void testPrettyPrintFromString(void)
{
    runLexerTest("(@@) prettyPrint: _value\n"
                 "{\n"
                 "    result = _value\n"
                 "\n"
                 "    if ([_value kindOf?: String])\n"
                 "    {\n"
                 "        // double backslash needed for C conversion\n"
                 "        result = \"\\\"\" + _value + \"\\\"\"\n"
                 "    }\n"
                 "\n"
                 "    return (result)\n"
                 "}\n");
}

static const char *sCompiledNil =
    "package org.taffy.core\n"
    "\n"
    "class Nil\n"
    "{\n"
    "    (@) hash\n"
    "    #const\n"
    "    {\n"
    "        return (4)\n"
    "    }\n"
    "\n"
    "    (@) asString\n"
    "    #const\n"
    "    {\n"
    "        return (\"nil\")\n"
    "    }\n"
    "}\n";

const char *sCompiledString =
    "package org.taffy.core\n"
    "\n"
    "class String\n"
    "{\n"
    "    (@@) prettyPrint: _value\n"
    "    {\n"
    "        result = _value\n"
    "\n"
    "        if ([_value kindOf?: String])\n"
    "        {\n"
    "            // double backslash needed for C conversion\n"
    "            result = \"\\\"\" + _value + \"\\\"\"\n"
    "        }\n"
    "\n"
    "        return (result)\n"
    "    }\n"
    "\n"
    "    (@) asString\n"
    "    #const\n"
    "    {\n"
    "        return (self)\n"
    "    }\n"
    "\n"
    "    (@) asSymbol\n"
    "    #const\n"
    "    {\n"
    "        return ([kernel eval: \"'#[self]\"])\n"
    "    }\n"
    "}\n";

static void testNil(void)
{
    runLexerTest(sCompiledNil);
}

static void testString(void)
{
    runLexerTest(sCompiledString);
}

static void testAsStringAsSymbolFromString(void)
{
    runLexerTest("(@) asString\n"
                 "#const\n"
                 "{\n"
                 "    return (self)\n"
                 "}\n"
                 "\n"
                 "(@) asSymbol\n"
                 "#const\n"
                 "{\n"
                 "    return ([kernel eval: \"'#[self]\"])\n"
                 "}\n");
}

static void testAsSymbolFromString(void)
{
    runLexerTest("(@) asSymbol\n"
                 "#const\n"
                 "{\n"
                 "    return ([kernel eval: \"'#[self]\"])\n"
                 "}");

}

static void testExceptionLineNumbers(void)
{
    testExceptionLineNumber("(\n"
                            "\n"
                            "class Test {}\n",
                            1,
                            true);

    testExceptionLineNumber("]\n"
                            "\n"
                            "class Test {}\n",
                            1,
                            true);

    testExceptionLineNumber("(\n"
                            "\n"
                            "\n"
                            "asdf\n",
                            4,
                            true);

    testExceptionLineNumber("slice Slicey {}\n"
                            "class Testy {}\n"
                            "\n"
                            "Testy attach: Slicey\n"
                            "\n"
                            "Test.Slicey\n",
                            6,
                            false);
}

static void testExceptionCallStackWithEach(void)
{
    dcList *expectedCallStack =
        dcList_createWithObjects
        (dcCallStackData_createNode("each:",
                                    sLexerTestFilenameId,
                                    11),
         dcCallStackData_createNode("addIt:",
                                    sLexerTestFilenameId,
                                    13),
         dcCallStackData_createNode("#operator(+):",
                                    sLexerTestFilenameId,
                                    5),
         NULL);

    dcTestUtilities_assertCallStack(dcStringEvaluator_evalString
                                    ("class B\n"
                                     "{\n"
                                     "    (@) addIt: _value\n"
                                     "    {\n"
                                     "        _value + \"b\"\n"
                                     "    }\n"
                                     "}\n"
                                     "\n"
                                     "b = new B\n"
                                     "array = [1,2,3,4]\n"
                                     "array each: ^{ <value> \n"
                                     "\n"
                                     "b addIt: value }\n",
                                     LEXER_TEST_FILE_NAME,
                                     NO_STRING_EVALUATOR_FLAGS),
                                    sNodeEvaluator,
                                    expectedCallStack);

}

static void testSplitString(void)
{
    {
        const char *string = "a";
        dcList *splits = dcLexer_splitString(string, ' ');
        assert(splits->size == 1);
        assert(strcmp(dcString_getString(splits->head->object), "a") == 0);
        dcList_free(&splits, DC_DEEP);
    }

    {
        const char *string = "a     b";
        dcList *splits = dcLexer_splitString(string, ' ');
        assert(splits->size == 2);
        assert(strcmp(dcString_getString(splits->head->object), "a") == 0);
        assert(strcmp(dcString_getString(splits->tail->object), "b") == 0);
        dcList_free(&splits, DC_DEEP);
    }

    {
        const char *string = "a     ";
        dcList *splits = dcLexer_splitString(string, ' ');
        assert(splits->size == 1);
        assert(strcmp(dcString_getString(splits->head->object), "a") == 0);
        dcList_free(&splits, DC_DEEP);
    }

    {
        const char *string = "      ";
        dcList *splits = dcLexer_splitString(string, ' ');
        assert(splits->size == 0);
        dcList_free(&splits, DC_DEEP);
    }

    {
        const char *string = "      a";
        dcList *splits = dcLexer_splitString(string, ' ');
        assert(splits->size == 1);
        assert(strcmp(dcString_getString(splits->head->object), "a") == 0);
        dcList_free(&splits, DC_DEEP);
    }

    {
        const char *string = "";
        dcList *splits = dcLexer_splitString(string, ' ');
        assert(splits->size == 0);
        dcList_free(&splits, DC_DEEP);
    }

    {
        const char *string = "a  b   c";
        dcList *splits = dcLexer_splitString(string, ' ');
        assert(splits->size == 3);
        assert(strcmp(dcString_getString(splits->head->object), "a") == 0);
        assert(strcmp(dcString_getString(splits->head->next->object), "b")
               == 0);
        assert(strcmp(dcString_getString(splits->tail->object), "c") == 0);
        dcList_free(&splits, DC_DEEP);
    }

    {
        const char *string = "a,b,c";
        dcList *splits = dcLexer_splitString(string, ',');
        assert(splits->size == 3);
        assert(strcmp(dcString_getString(splits->head->object), "a") == 0);
        assert(strcmp(dcString_getString(splits->head->next->object), "b")
               == 0);
        assert(strcmp(dcString_getString(splits->tail->object), "c") == 0);
        dcList_free(&splits, DC_DEEP);
    }

    {
        const char *string = "a/b";
        dcList *splits = dcLexer_splitString(string, '/');
        assert(splits->size == 2);
        assert(strcmp(dcString_getString(splits->head->object), "a") == 0);
        assert(strcmp(dcString_getString(splits->tail->object), "b") == 0);
        dcList_free(&splits, DC_DEEP);
    }

    {
        const char *string = ("234987234987234987234982374982734987234"
                              "/ 234987234987234987234987239487239487234");
        dcList *splits = dcLexer_splitString(string, '/');
        assert(splits->size == 2);
        dcList_free(&splits, DC_DEEP);
    }
}

static const dcTestFunctionMap sTests[] =
{
    {"xor",                     &testXor},
    {"xor equal",               &testXorEqual},
    {"Nil",                     &testNil},
    {"String",                  &testString},
    {"Split String",            &testSplitString},
    {"asString, asSymbol",      &testAsStringAsSymbolFromString},
    {"asSymbol from String",    &testAsSymbolFromString},
    {"prettyPrint from String", &testPrettyPrintFromString},
    {"String with Identifier",  &testStringWithIdentifier},
    {"Block with Arguments",    &testBlockWithArguments},
    {"Method with Block",       &testMethodWithBlock},
    {"Method Call with Block",  &testMethodCallWithBlock},
    {NULL}
};

static const dcTestFunctionMap sRuntimeTests[] =
{
    {"Exception Line Numbers",         &testExceptionLineNumbers},
    {"Exception Call Stack with Each", &testExceptionCallStackWithEach},
    {NULL}
};

extern dcTaffyThreadId parserSelf;

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcStringManager_create();
    parserSelf = dcThread_getSelfId();
    dcLexer_initialize();

    dcTestUtilities_go("Lexer Test", _argc, _argv, NULL, sTests, false);
    printf("\n");

    if (dcTestUtilities_runSystem())
    {
        dcSystem_createWithArguments
            (dcTaffyCommandLineArguments_parseAndCreateWithFailure(_argc,
                                                                   _argv,
                                                                   false));
        sLexerTestFilenameId =
            dcStringManager_getStringId(LEXER_TEST_FILE_NAME);
        sNodeEvaluator = dcSystem_getCurrentNodeEvaluator();
        dcTestUtilities_go("Lexer Runtime Test",
                           _argc,
                           _argv,
                           NULL,
                           sRuntimeTests,
                           true);
        dcSystem_free();
    }

    dcGarbageCollector_free();
    dcStringManager_free();
    dcMemory_deinitialize();
    return 0;
}
