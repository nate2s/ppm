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
#include <string.h>

#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcClass.h"
#include "dcError.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcFunctionClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcIdentifier.h"
#include "dcMemory.h"
#include "dcMethodCall.h"
#include "dcNode.h"
#include "dcParser.h"
#include "dcStringClass.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"

#define PARSER_TEST_FILE_NAME "src/tests/dcParserTest.c"

static void testSuccess(const char *_program, dcNodeType _type)
{
    dcNode *result = dcParser_parseString(_program,
                                          PARSER_TEST_FILE_NAME,
                                          false);
    if (result == NULL)
    {
        fprintf(stderr,
                "\n\nFor program:\n%s\nResult is NULL\n\n",
                _program);
        assert(false);
    }

    if (! dcGraphData_isType(result, NODE_GRAPH_DATA_TREE))
    {
        fprintf(stderr,
                "\n\nFor program:\n%s\nType %s is not NODE_GRAPH_DATA_TREE\n\n",
                _program,
                dcNode_getNodeTypeString(result));
        assert(false);
    }

    if (! dcGraphData_isType(dcGraphDataTree_getContents(result),
                             (dcGraphDataType)_type))
    {
        fprintf(stderr,
                "\n\nFor program:\n%s\nWanted type: %s but got type: %s\n\n",
                _program,
                dcNode_getTypeString(_type),
                dcGraphData_getTypeName(dcGraphData_getType
                                        (dcGraphDataTree_getContents(result))));
        assert(false);
    }

    dcNode_free(&result, DC_DEEP);
}

static void testBlank(void)
{
    testSuccess("", NODE_NIL);
}

static void testStartsWithReturn(void)
{
    testSuccess("\nnil", NODE_NIL);
}

static void testBunchOfReturns(void)
{
    testSuccess("\n\n\n\n\n\n\n\n\n\n\n", NODE_NIL);
}

static void testIdentifier(void)
{
    dcNode *result = dcParser_parseString("identifier",
                                          PARSER_TEST_FILE_NAME,
                                          false);
    dcTestUtilities_assert
        (dcGraphData_isType(result, NODE_GRAPH_DATA_TREE)
         && dcGraphData_isType(dcGraphDataTree_getContents(result),
                               NODE_IDENTIFIER));

    dcNode *contents = dcGraphDataTree_getContents(result);
    dcTestUtilities_assert(dcIdentifier_equalsString(contents, "identifier"));
    dcNode_free(&result, DC_DEEP);
}

static void testImportPlusGlobal(void)
{
    const char *input = ("import org.taffy.core.maths.*\n"
                         "global const math = new Math");
    dcNode *result = dcParser_parseString(input,
                                          PARSER_TEST_FILE_NAME,
                                          false);
    dcTestUtilities_assert(result != NULL);
    dcNode_free(&result, DC_DEEP);
}

static void testInvalidIdentifiers(void)
{
    const char *invalidIdentifiers[] =
        {"hi@there",
         "this#isATest",
         "invalid]",
         "invalid)",
         "class)",
         "class { (@ hi there }",
         "+)"};
    size_t i;

    for (i = 0; i < dcTaffy_countOf(invalidIdentifiers); i++)
    {
        dcTestUtilities_assert(dcParser_parseString(invalidIdentifiers[i],
                                                    PARSER_TEST_FILE_NAME,
                                                    false)
                               == NULL);
    }
}

static void testScopedIdentifier(void)
{
    const char *identifiers[] =
        {"org.ObjectDude"};
    //"org.taffy.core.Object",
    //     "org.taffy.core.exception.UnidentifiedObjectException"};
    size_t i = 0;

    for (i = 0; i < dcTaffy_countOf(identifiers); i++)
    {
        dcNode *graphDataTree = dcParser_parseString(identifiers[i],
                                                     PARSER_TEST_FILE_NAME,
                                                     false);
        dcTestUtilities_assert(graphDataTree != NULL);
        dcNode *result = dcGraphDataTree_getContents(graphDataTree);
        const char *name = dcIdentifier_getName(result);
        dcTestUtilities_assert(strcmp(name, identifiers[i]) == 0);
        dcNode_free(&graphDataTree, DC_DEEP);
    }
}

static void testParseErrorException(const char *_program, bool _expectedSuccess)
{
    dcNode *graphDataTree = dcParser_parseString(_program,
                                                 PARSER_TEST_FILE_NAME,
                                                 false);
    if (_expectedSuccess)
    {
        dcTestUtilities_assert(graphDataTree != NULL);
        dcNode_free(&graphDataTree, DC_DEEP);
    }
    else
    {
        dcTestUtilities_assert(graphDataTree == NULL);
    }
}

static void testCommentAtEnd(void)
{
    testParseErrorException("a = 1 // blerghi\n"
                            "b = 2",
                            true);
    testParseErrorException("[doo one: 1  // one\n"
                            "two: 2  // two\n"
                            "tre: 3] // tre\n",
                            true);
    testParseErrorException("(1 => 1,  // one\n"
                            " 2 => 2, // two\n"
                            " 3 => 3) // tre\n",
                            true);
    testParseErrorException("\"hi there #[a = 1  // one\n"
                            "]\"",
                            true);
    testParseErrorException("const addonTests = [\"FileTest\"] "
                            "//[\"DateTest\", \"FileTest\"]\n"
                            "const tests =\n"
                            "[]",
                            true);
}

static void testParseError(void)
{
    // test simple parse errors
    testParseErrorException(")~",        false);
    testParseErrorException(")",         false);

    // verify we can succeed again for sanity
    testParseErrorException("asdf ~()~", true);

    // test missing ]
    testParseErrorException("[[[this is] a]", false);

    // test parse errors in a class
    testParseErrorException("class UhOh"
                            "{"
                            "]",
                            false);

    // class with guts
    testParseErrorException("class UhOh"
                            "{"
                            "    (@) guts"
                            "    {"
                            "        [someFunction call]"
                            "    }"
                            "]",
                            false);

    // invalid method call
    testParseErrorException
        ("class UhOh"
         "{"
         "    (@) guts"
         "    {"
         "        \"this is a string #[[someFunction call] callAgain]\""
         "    }"
         "}",
         false);

    // class with readers and writers
    testParseErrorException
        ("class UhOh"
         "{"
         "    @ree, @rw"
         ""
         "    (@) guts"
         "    {"
         "        \"this is a string #[[someFunction call] callAgain]\""
         "    }"
         "}",
         false);

    // class with class
    testParseErrorException
        ("class UhOh"
         "{"
         "    class OhDear {}"
         ""
         "    (@) guts"
         "    {"
         "        \"this is a string #[[someFunction call] callAgain]\""
         "    }"
         "}",
         false);

    // class with meta methods
    testParseErrorException
        ("class UhOh"
         "{"
         "    (@@) metaDude {}"
         "    (@@) metaDudette {}"
         ""
         "    (@@) guts"
         "    {"
         "        \"this is a string #[[someFunction call] callAgain]\""
         "    }"
         "}",
         false);

    // class with class that has parse error
    testParseErrorException
        ("class UhOh"
         "{"
         "    @variable, @rw"
         "    (@) method {}"
         ""
         "    class Sup"
         "    {"
         "        der]"
         "    }"
         "}",
         false);

    // class with parse error that has a class
    testParseErrorException
        ("class UhOh"
         "{"
         "    class Sup"
         "    {"
         "        @variable, @rw"
         "        (@) method {}"
         "    }"
         "        der]"
         ""
         "}",
         false);

    // class with the goods
    testParseErrorException
        ("class UhOhOhDear"
         "{"
         "    @hiThere, @r;"
         "    @helloThere, @w;"
         "    @whatsUp, @rw;"
         ""
         "    (@) unMetaDude {}"
         "    (@@) metaDude {}"
         ""
         "    (@) guts"
         "    {"
         "        if (true)"
         "        {"
         "            io putLine: \"reee #[[someFunction call] callAgain]\""
         "        }"
         "    }"
         "}",
         false);

    // unfinished return statement
    testParseErrorException
        ("return ([ContainerAddedToItselfExceptionTester test: {\n"
         "   array = new Array\n"
         "   array push: array\n"
         " }]\n",
         false);

    // ending in backslash
    testParseErrorException("\\", false);
    // residual + parse error
    testParseErrorException("3x * 2 * x + )", false);

    testParseErrorException("3E-.", false);
    testParseErrorException("3E-+", false);
    testParseErrorException("3E++", false);
    testParseErrorException("3E+3", true);

    // unterminated symbol
    testParseErrorException("'", false);

    // unknown escape sequence
    testParseErrorException("\\m", false);

    // unknown escape sequence in string
    testParseErrorException("\"\\m\"", false);

    // tilde-equal with invalid contents
    testParseErrorException("1 ~= <-1> 2",     false);
    testParseErrorException("1 ~= <i> 2",      false);
    testParseErrorException("1 ~= <1 + 2i> 2", false);

    // expression ending in question-mark
    testParseErrorException("(1 < 2) ?", false);
    testParseErrorException("1 < 2 ?", false);

    testParseErrorException("^{ x } }", false);
    // TODO
    //testParseErrorException("{ { x }", false);
}

static void testVerbatimTextWithExpectedResult
    (const char *_text, const char *_expectedResult)
{
    dcNode *graphDataTree = dcParser_parseString(_text,
                                                 PARSER_TEST_FILE_NAME,
                                                 false);
    dcTestUtilities_assert
        (graphDataTree != NULL
         && (dcGraphData_isType(graphDataTree, NODE_GRAPH_DATA_TREE)));

    // assert that we got a string object!
    dcNode *contents = dcGraphDataTree_getContents(graphDataTree);
    dcTestUtilities_assert(dcGraphData_isType(contents, NODE_CLASS)
                           && dcClass_hasTemplate(contents,
                                                  dcStringClass_getTemplate(),
                                                  true)
                           && (strcmp(dcStringClass_getString(contents),
                                      _expectedResult)
                               == 0));

    dcNode_free(&graphDataTree, DC_DEEP);
}

static void testVerbatim(const char *_text)
{
    char *expectedResult = dcMemory_strdup(_text + 2);
    expectedResult[strlen(_text) - 4] = 0;
    testVerbatimTextWithExpectedResult(_text, expectedResult);
    dcMemory_free(expectedResult);
}

static void testVerbatimText(void)
{
    testVerbatim("'{'}");
    testVerbatim("'{ '}");
    testVerbatim("'{hi there how are you'}");
    testVerbatim("'{hi there how\n are you'}");
    testVerbatim("'{hi there how\n "
                 "#[this should show up!] are you'}");
    testVerbatim("'{hi there how\n "
                 "#[this should show up!] are you\n"
                 "'}");
    testVerbatim("'{\n"
                 "hi there how\n "
                 "#[this should show up!] are you\n"
                 "'}");
    testVerbatimTextWithExpectedResult("'{ \\'} '}",
                                       " '} ");
    testVerbatimTextWithExpectedResult("'{ \\'} \n"
                                       "\\'}'}",
                                       " '} \n"
                                       "'}");
}

static void testEmptyIf(void)
{
    testSuccess("if (true) {}", NODE_IF);
    testSuccess("if (true)\n {}", NODE_IF);
    testSuccess("if (true)\n {}", NODE_IF);
    testSuccess("if (\ntrue\n)\n {}", NODE_IF);
}

static void testEmptyWhile(void)
{
    testSuccess("while (true) {}", NODE_WHILE);
    testSuccess("while (true)\n {}", NODE_WHILE);
    testSuccess("while (\ntrue\n)\n {}", NODE_WHILE);
}

static void testFor(void)
{
    // standard
    testSuccess("for (a = 0; a < 100; a++) {}", NODE_FOR);

    // multi cases
    testSuccess("for (a = 0, b = 0; a < 100; a++) {}", NODE_FOR);
    testSuccess("for (a = 0, b = 0, c = 10; a < 100; a++) {}", NODE_FOR);
    testSuccess("for (a = 0; a < 100; a++, b += 2) {}", NODE_FOR);
    testSuccess("for (a = 0, b = 0; a < 100; a++, b += 2) {}", NODE_FOR);
    testSuccess("for (a = 0, b = 0; "
                "a < 100; "
                "a++, b += 2, c += 3) {}",
                NODE_FOR);
    testSuccess("for (a = 0, b = 0, c = 10; "
                "a < 100 and b < 1000; "
                "a++, b += 2, c += 3) {}",
                NODE_FOR);

    // blank cases
    testSuccess("for ( ; a < 100; a++) {}", NODE_FOR);
    testSuccess("for ( ; a < 100; ) {}", NODE_FOR);
    testSuccess("for (a = 0; a < 100; ) {}", NODE_FOR);
    testSuccess("for (a = 0, b = 2; a < 100; ) {}", NODE_FOR);
}

static void testEmptyFor(void)
{
    testSuccess("for (a = true; a != false; a = false) {}", NODE_FOR);
    testSuccess("for (a = true; a != false; a = false)\n {}", NODE_FOR);
    testSuccess("for (a = true; \n a != false; a = false)\n {}", NODE_FOR);
    testSuccess("for (a = true; a != false; \n a = false)\n {}", NODE_FOR);
    testSuccess("for (\na = true;\na != false; \n a = false\n)\n {}", NODE_FOR);
}

static void testEmptyTry(void)
{
    testSuccess("try {} catch (Exception _exception) {}", NODE_TRY_BLOCK);
    testSuccess("try\n {} catch (Exception _exception) {}", NODE_TRY_BLOCK);
    testSuccess("try {} \n catch (Exception _exception) {}", NODE_TRY_BLOCK);
    testSuccess("try {} \n catch (Exception _exception)\n {}", NODE_TRY_BLOCK);
    testSuccess("try\n{}\ncatch (Exception _exception)\n{}", NODE_TRY_BLOCK);
}

static void testEscapeSequence(void)
{
    // escaped quote inside of string, pretty eh?
    testSuccess("\"\\\"\"", NODE_CLASS);

    // tab inside string
    testSuccess("\"\t\"", NODE_CLASS);

    // carriage return inside string
    testSuccess("\"\n\"", NODE_CLASS);

    // escaped #, followed by [, inside string
    testSuccess("\"\\#[\"", NODE_CLASS);

    // carriage return inside expression
    testSuccess("3\\n4", NODE_CLASS);

    // carriage return inside expression
    testSuccess("a = 3\\nb = 4", NODE_ASSIGNMENT);
}

static void testAutomaticFunction(void)
{
    dcNode *result = dcParser_parseString("derive x^2",
                                          PARSER_TEST_FILE_NAME,
                                          false);
    dcGraphData_assertType(result, NODE_GRAPH_DATA_TREE);
    dcNode *contents = dcGraphDataTree_getContents(result);
    dcGraphData_assertType(contents, NODE_METHOD_CALL);

    assert(strcmp(dcMethodCall_getMethodName(contents), "#operator(()):") == 0);
    dcNode *receiver = dcMethodCall_getReceiver(contents);
    dcGraphData_assertType(receiver, NODE_IDENTIFIER);
    assert(strcmp(dcIdentifier_getName(receiver), "derive") == 0);

    dcList *arguments = dcMethodCall_getArguments(contents);
    assert(arguments->size == 1);
    assert(dcArrayClass_isMe(arguments->head->object));
    dcArray *objects = dcArrayClass_getObjects(arguments->head->object);
    assert(objects->size == 1);

    dcNode *first = objects->objects[0];
    assert(dcFunctionClass_isMe(first));

    dcNode_free(&result, DC_DEEP);
}

static const dcTestFunctionMap sMap[] =
{
    {"Starts With Return",  &testStartsWithReturn},
    {"Bunch of Returns",    &testBunchOfReturns},
    {"Scoped Identifier",   &testScopedIdentifier},
    {"Invalid Identifiers", &testInvalidIdentifiers},
    {"Blank",               &testBlank},
    {"Import plus Global",  &testImportPlusGlobal},
    {"Identifier",          &testIdentifier},
    {"Empty if",            &testEmptyIf},
    {"Empty while",         &testEmptyWhile},
    {"For",                 &testFor},
    {"Empty for",           &testEmptyFor},
    {"Empty try",           &testEmptyTry},
    {"Verbatim Text",       &testVerbatimText},
    {"Parse Error",         &testParseError},
    {"Comment at End",      &testCommentAtEnd},
    {"Escape Sequence",     &testEscapeSequence},
    {"Automatic Function",  &testAutomaticFunction},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_create();
    dcTestUtilities_go("Parser Test", _argc, _argv, NULL, sMap, false);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
