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

#include "dcCallStackData.h"
#include "dcClassManager.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcCommandLineArguments.h"
#include "dcExceptions.h"
#include "dcFileEvaluator.h"
#include "dcGarbageCollector.h"
#include "dcHash.h"
#include "dcHashClass.h"
#include "dcIdentifier.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcScope.h"
#include "dcStringEvaluator.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcPackage.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestClass.h"
#include "dcTestUtilities.h"
#include "dcYesClass.h"

#define CLASS_TEST_FILE_NAME "src/tests/dcClassTest.c"

static dcNode *sObject = NULL;
static dcNode *sMetaObject = NULL;
static dcStringId sClassTestFilenameId = 0;
static dcNodeEvaluator *sNodeEvaluator = NULL;

static void marker(void)
{
    dcNode_mark(sObject);
}

static void testSetter(void)
{
    dcTestUtilities_assert(dcNodeEvaluator_callMethodWithArgument
                           (sNodeEvaluator,
                            sObject,
                            "setVariableA:",
                            dcYesClass_getInstance())
                           != NULL);
    dcTestUtilities_assert(dcTestClass_getVariableA(sObject)
                           == dcYesClass_getInstance());
}

static void testGetter(void)
{
    dcTestUtilities_assert(dcNodeEvaluator_callMethod
                           (sNodeEvaluator,
                            sObject,
                            "setVariableB")
                           == dcYesClass_getInstance());

    dcNode *variableB = dcNodeEvaluator_callMethod(sNodeEvaluator,
                                                   sObject,
                                                   "variableB");

    dcTestUtilities_assert(dcNodeEvaluator_callMethodWithArgument
                           (sNodeEvaluator,
                            variableB,
                            "#operator(==):",
                            dcNumberClass_getOneNumberObject())
                           == dcYesClass_getInstance());
}

static dcNode *testResult(const char *_name, dcNode *_wantedResult)
{
    dcNode *result =
        (dcNodeEvaluator_callMethodWithArgument
         (sNodeEvaluator,
          sObject, // receiver
          _name,
          sObject));
    dcTestUtilities_assert(result == _wantedResult);
    return result;
}

static void testOperatorEqualEqual(void)
{
    testResult(dcSystem_getOperatorName(TAFFY_EQUALS),
               dcKernelClass_getOrCreateSymbol("operatorEqualEqual"));
}

static void testOperatorPlus(void)
{
    testResult(dcSystem_getOperatorName(TAFFY_ADD),
               dcKernelClass_getOrCreateSymbol("operatorPlus"));
}

static void testInstanceFunction(void)
{
    testResult("instanceTestFunction",
               dcKernelClass_getOrCreateSymbol("instanceTestFunction"));
}

static void testMetaMethod(void)
{
    dcTestUtilities_assert
        (dcNodeEvaluator_callEvaluatedMethod
         (sNodeEvaluator,
          "org.taffy.tests.TestClass", // receiver
          "metaTestFunction", // function name
          NULL) // argument
         == dcKernelClass_getOrCreateSymbol("metaTestFunction"));

    dcTestUtilities_assert
        (dcNodeEvaluator_callEvaluatedMethod
         (sNodeEvaluator,
          "org.taffy.tests.TestClass", // receiver
          "taffyMetaTestFunction", // function name
          NULL) // argument
         == dcKernelClass_getOrCreateSymbol("taffyMetaTestFunction"));
}

static void testMetaInit(void)
{
    dcNode *result = dcStringEvaluator_evalString("class TestMetaInit {"
                                                  ""
                                                  "    @@value, @r"
                                                  ""
                                                  "    (@@) init\n"
                                                  "    {"
                                                  "        if (@@value == nil)"
                                                  "        {"
                                                  "            @@value = 1"
                                                  "        }"
                                                  "        else"
                                                  "        {"
                                                  "            @@value++"
                                                  "        }\n"
                                                  "    }\n"
                                                  "}\n"
                                                  "TestMetaInit value",
                                                  CLASS_TEST_FILE_NAME,
                                                  NO_STRING_EVALUATOR_FLAGS);
    dcTestUtilities_assert(result != NULL
                           && dcNumberClass_isMe(result)
                           && dcNumberClass_equalsInt32u(result, 1));

    // verify that an update does not call (@@) init
    result = dcStringEvaluator_evalString("class TestMetaInit {}\n"
                                          "TestMetaInit value",
                                          CLASS_TEST_FILE_NAME,
                                          STRING_EVALUATOR_ASSERT_NO_EXCEPTION);

    dcTestUtilities_assert(dcNumberClass_isMe(result)
                           && dcNumberClass_equalsInt32u(result, 1));

    // verify that an instantiation does not call (@@) init
    result = dcStringEvaluator_evalString("new TestMetaInit\n"
                                          "TestMetaInit value",
                                          CLASS_TEST_FILE_NAME,
                                          STRING_EVALUATOR_ASSERT_NO_EXCEPTION);

    dcTestUtilities_assert(dcNumberClass_isMe(result)
                           && dcNumberClass_equalsInt32u(result, 1));
}

static void testInheritedMetaMethod(void)
{
    dcStringEvaluator_evalString
        ("import src.tests.TestDirectory.inheritance.*\n",
         CLASS_TEST_FILE_NAME,
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);

    dcTestUtilities_assert
        (dcNodeEvaluator_callEvaluatedMethod
         (sNodeEvaluator,
          "src.tests.TestDirectory.inheritance.Class1",
          "init",
          NULL)
         != NULL);

    dcTestUtilities_assert
        (dcNodeEvaluator_callEvaluatedMethod
         (sNodeEvaluator,
          "src.tests.TestDirectory.inheritance.Class2",
          "init",
          NULL)
         != NULL);
}

static void testMetaObject(void)
{
    dcNode *identifier =
        dcIdentifier_createNode("org.taffy.tests.TestClass", NO_FLAGS);
    dcTestUtilities_assert(sMetaObject
                           == (dcNodeEvaluator_evaluate
                               (sNodeEvaluator, identifier)));
    dcNode_free(&identifier, DC_DEEP);
}

static void testUnidentifiedMethodException(void)
{
    dcNode *result = testResult("fail", NULL);
    dcTestUtilities_assertException(result,
                                    sNodeEvaluator,
                                    "UnidentifiedMethodException",
                                    "@exceptionMethodName",
                                    "\"fail\"",
                                    false);
    dcTestUtilities_assertException(result,
                                    sNodeEvaluator,
                                    "UnidentifiedMethodException",
                                    "@exceptionClassName",
                                    "\"TestClass\"",
                                    true);
}

static void testNoCastForReceiver(void)
{
    // implement me pulleeeasseeee
}

static void testCreateClassTemplate(void)
{
    dcClassTemplate *classTemplate =
        dcClassTemplate_createSimple("org.taffy.core.tests",
                                     "ClassTest",
                                     "org.taffy.core.Object",
                                     CLASS_ABSTRACT,
                                     NO_FLAGS);
    dcNode *node =
        dcClassTemplate_createSimpleNode("org.taffy.core.tests",
                                         "ClassTest",
                                         "org.taffy.core.Object",
                                         CLASS_ABSTRACT,
                                         NO_FLAGS);

    dcTestUtilities_assert(dcClassTemplate_equals
                           (classTemplate, CAST_CLASS_TEMPLATE(node)));

    dcClassTemplate_free(&classTemplate, DC_DEEP);
    dcNode_free(&node, DC_DEEP);
}

static void testCopyClassTemplate(void)
{
    dcClassTemplate *classTemplate =
        dcClassTemplate_createSimple("org.taffy.core.tests",
                                     "ClassTest",
                                     "org.taffy.core.Object",
                                     CLASS_ABSTRACT,
                                     NO_FLAGS);
    dcClassTemplate *copy = dcClassTemplate_copy(classTemplate, DC_DEEP);
    dcTestUtilities_assert(dcClassTemplate_equals(classTemplate, copy));
    dcClassTemplate_free(&classTemplate, DC_DEEP);
    dcClassTemplate_free(&copy, DC_DEEP);
}

static void testAssignmentInConstructorMemoryLeak(void)
{
    assert(dcStringEvaluator_evalString("org.taffy.core.exception."
                                        "LocalToGlobalConversionException "
                                        "createObjectWithName: \"foo\"",
                                        CLASS_TEST_FILE_NAME,
                                        STRING_EVALUATOR_ASSERT_NO_EXCEPTION)
           != NULL);
}

static void testMetaIncrement(void)
{
    dcStringEvaluator_evalString("class Test"
                                 "{ "
                                 "    @@x, @rw; "
                                 ""
                                 "    (@@) initializeIt"
                                 "    {"
                                 "        @@x = 1"
                                 "    }"
                                 ""
                                 "    (@@) incrementIt"
                                 "    { "
                                 "        @@x++ "
                                 "    } "
                                 "}\n"
                                 "[Test initializeIt]\n"
                                 "[kernel assert: [Test incrementIt] == 2]\n",
                                 CLASS_TEST_FILE_NAME,
                                 STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
}

static void testAncestorMethodLookup(void)
{
    assert(dcStringEvaluator_evalString("import org.taffy.core.container.*\n"
                                        "a = new Array\n"
                                        "a init\n",
                                        CLASS_TEST_FILE_NAME,
                                        STRING_EVALUATOR_ASSERT_NO_EXCEPTION)
           != NULL);
}

static void testBadParentExceptionThrow(void)
{
    dcTestUtilities_assertException
        (dcStringEvaluator_evalString
         ("import src.tests.TestDirectory.HasBadParent",
          CLASS_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "UnidentifiedClassException",
         "@className",
         "\"DerDer\"",
         true);
}

static void testNonMetaAsParent(void)
{
    dcTestUtilities_assertException
        (dcStringEvaluator_evalString("A = \"hi there\";"
                                      "class TesterTester(A) {}",
                                      CLASS_TEST_FILE_NAME,
                                      NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "UnidentifiedClassException",
         "@className",
         "\"A\"",
         true);
}

static void testAbstractClassInstantiation(void)
{
    dcTestUtilities_assertException
        (dcStringEvaluator_evalString("abstract class Ree {};"
                                      "new Ree",
                                      CLASS_TEST_FILE_NAME,
                                      NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "AbstractClassInstantiationException",
         "@objectName",
         "\"Ree\"",
         true);
}

static void testFinalClassExtension(void)
{
    dcTestUtilities_assertException
        (dcStringEvaluator_evalString("final class Finally {};"
                                      "class OhNo(Finally) {};",
                                      CLASS_TEST_FILE_NAME,
                                      NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "InvalidSuperClassException",
         "@superName",
         "\"Finally\"",
         true);
}

static void testParseErrorDoubleImport(void)
{
    dcTestUtilities_assertException
        (dcStringEvaluator_evalString
         ("import src.tests.TestDirectory.ParseErrorClass",
          CLASS_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "ParseErrorException",
         NULL,
         NULL,
         true);

    dcTestUtilities_assertException
        (dcStringEvaluator_evalString
         ("import src.tests.TestDirectory.ParseErrorClass",
          CLASS_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "ParseErrorException",
         NULL,
         NULL,
         true);
}

static void testInvalidAsStringClass(void)
{
    dcNode *result = (dcStringEvaluator_evalString
                      ("class InvalidAsStringClass { (@) asString {} };"
                       "io put: \"#[new InvalidAsStringClass]\"",
                       CLASS_TEST_FILE_NAME,
                       NO_STRING_EVALUATOR_FLAGS));
    dcTestUtilities_assertException(result,
                                    sNodeEvaluator,
                                    "InvalidCastException",
                                    "@from",
                                    "\"Nil\"",
                                    false);
    dcTestUtilities_assertException(result,
                                    sNodeEvaluator,
                                    "InvalidCastException",
                                    "@to",
                                    "\"String\"",
                                    true);
}

static void testExceptionDuringInit(void)
{
    dcTestUtilities_assertException
        (dcStringEvaluator_evalString
         ("class ExceptionDuringInit { (@) init { asdf } };"
          "new ExceptionDuringInit",
          CLASS_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "UnidentifiedObjectException",
         "@objectName",
         "\"asdf\"",
         true);
}

static void testProtectedMembers(void)
{
    dcNode *result = dcStringEvaluator_evalString
        ("class BaseClass\n"
         "{\n"
         "    @public\n"
         "    @publicMember, @rw\n"
         "    (@) init\n"
         "    {\n"
         "        new BaseClass.ProtectedClass\n"
         "        new BaseClass.ProtectedClass.AnotherProtectedClass\n"
         "    }\n"
         "\n"
         "    (@) getValue\n"
         "    {\n"
         "        left = new BaseClass.ProtectedClass.AnotherProtectedClass\n"
         "        return ([left getValue])\n"
         "    }\n"
         "\n"
         "    @protected\n"
         "    @protectedMember, @rw\n"
         "\n"
         "    class ProtectedClass\n"
         "    {\n"
         "        @protected\n"
         "        @anotherProtectedMember, @rw\n"
         "        class AnotherProtectedClass\n"
         "        {\n"
         "            (@) getValue\n"
         "            {\n"
         "                return (5)\n"
         "            }\n"
         "        }\n"
         "    }\n"
         "}\n"
         "baseClass = new BaseClass\n"
         "baseClass getValue\n",
         CLASS_TEST_FILE_NAME,
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
    dcTestUtilities_assert(dcNumberClass_equalsInt32u(result, 5));
}

static void testClassWithinClass(void)
{
    dcNode *result =
        dcStringEvaluator_evalString("class Base\n"
                                     "{\n"
                                     "    class Within\n"
                                     "    {\n"
                                     "        (@) getIt\n"
                                     "        {\n"
                                     "            return 5\n"
                                     "        }\n"
                                     "    }\n"
                                     ""
                                     "    (@) doIt\n"
                                     "    {\n"
                                     "        within = new Within\n"
                                     "        return [within getIt]\n"
                                     "    }\n"
                                     "}\n"
                                     "base = new Base\n"
                                     "base doIt\n",
                                     CLASS_TEST_FILE_NAME,
                                     STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
    assert(dcNumberClass_isMe(result)
           && dcNumberClass_equalsInt32u(result, 5));
}

static void testProtectedSuperclass(void)
{
    dcTestUtilities_assertException
        (dcStringEvaluator_evalString
         ("class PublicClass"
          "{\n"
          "    @protected\n"
          "    class ProtectedClass {}\n"
          "}\n"
          ""
          "class ClassSucceed(PublicClass) {}\n"
          "class ClassFail(PublicClass.ProtectedClass) {}\n"
          ""
          "new ClassSucceed\n"
          "new ClassFail\n",
          CLASS_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         "UnidentifiedClassException",
         "@className",
         "\"PublicClass.ProtectedClass\"",
         true);
}

static void testInheritedMethodCallStackSameFile(void)
{
    dcList *expectedCallStack =
        dcList_createWithObjects
        (dcCallStackData_createNode("validMethod",
                                    sClassTestFilenameId,
                                    15),
         dcCallStackData_createNode("anotherValidMethod",
                                    sClassTestFilenameId,
                                    10),
         dcCallStackData_createNode(NULL,
                                    sClassTestFilenameId,
                                    5),
         NULL);

    dcTestUtilities_assertCallStack(dcStringEvaluator_evalString
                                    ("class B\n"
                                     "{\n"
                                     "    (@) anotherValidMethod\n"
                                     "    {\n"
                                     "        [self invalidMethod]\n"
                                     "    }\n"
                                     "\n"
                                     "    (@) validMethod\n"
                                     "    {\n"
                                     "        [self anotherValidMethod]\n"
                                     "    }\n"
                                     "}\n"
                                     "\n"
                                     "b = new B\n"
                                     "b validMethod\n",
                                     CLASS_TEST_FILE_NAME,
                                     NO_STRING_EVALUATOR_FLAGS),
                                    sNodeEvaluator,
                                    expectedCallStack);
}

static void testInheritedMethodCallStackDifferentFiles(void)
{
    dcStringId inheritedExceptionStringId =
        dcStringManager_getStringId
        ("src/tests/TestDirectory/InheritedException.ty");

    dcStringId ohDearStringId =
        dcStringManager_getStringId("src/tests/TestDirectory/OhDear.ty");

    dcList *expectedCallStack =
        dcList_createWithObjects
        (dcCallStackData_createNode("new InheritedException",
                                    sClassTestFilenameId,
                                    2),
         dcCallStackData_createNode("init",
                                    sClassTestFilenameId,
                                    2),
         dcCallStackData_createNode("init",
                                    inheritedExceptionStringId,
                                    11),
         dcCallStackData_createNode("asdf",
                                    ohDearStringId,
                                    16),
         NULL);

    dcTestUtilities_assertCallStack
        (dcStringEvaluator_evalString
         ("import src.tests.TestDirectory.InheritedException\n"
          "new InheritedException\n",
          CLASS_TEST_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS),
         sNodeEvaluator,
         expectedCallStack);
}

static void testImplicitImports(void)
{
    dcStringEvaluator_evalString
        ("import src.tests.TestDirectory.ImplicitImport.ImplicitImportB\n"
         "import src.tests.TestDirectory.ImplicitImport.ImplicitImportC\n"
         "import src.tests.TestDirectory.ImplicitImport.ImplicitImportD\n"
         "new ImplicitImportB;"
         "new ImplicitImportC;"
         "new ImplicitImportD;",
         CLASS_TEST_FILE_NAME,
         STRING_EVALUATOR_ASSERT_NO_EXCEPTION);
}

static void resetTest(void)
{
    dcTestUtilities_assert(sNodeEvaluator->exception == NULL);
}

static const dcTestFunctionMap sMap[] =
{
    {"Create Class Template",     &testCreateClassTemplate},
    {"Copy Class Template",       &testCopyClassTemplate},
    {NULL}
};

static const dcTestFunctionMap sRuntimeMap[] =
{
    {"Setter",                       &testSetter},
    {"Getter",                       &testGetter},
    {"Operator ==",                  &testOperatorEqualEqual},
    {"Operator +",                   &testOperatorPlus},
    {"Instance Function",            &testInstanceFunction},
    {"Meta Method",                  &testMetaMethod},
    {"Meta Init",                    &testMetaInit},
    {"Abstract Class Instantiation", &testAbstractClassInstantiation},
    {"Final Class Extension",        &testFinalClassExtension},
    {"Inherited Meta Method",        &testInheritedMetaMethod},
    {"Meta-Object",                  &testMetaObject},
    {"UnidentifiedMethodException",  &testUnidentifiedMethodException},
    {"No Cast for Receiver",         &testNoCastForReceiver},
    {"Assignment in Constructor",    &testAssignmentInConstructorMemoryLeak},
    {"Meta increment",               &testMetaIncrement},
    {"Ancestor Method Lookup",       &testAncestorMethodLookup},
    {"Bad Parent Exception Throw",   &testBadParentExceptionThrow},
    {"Non-Meta as Parent",           &testNonMetaAsParent},
    {"Parse Error Double Import",    &testParseErrorDoubleImport},
    {"Invalid asString Class",       &testInvalidAsStringClass},
    {"Exception During init",        &testExceptionDuringInit},
    {"Protected Members",            &testProtectedMembers},
    {"Protected Superclass",         &testProtectedSuperclass},
    {"Class within Class",           &testClassWithinClass},
    {"Implicit Imports",             &testImplicitImports},
    {"Inherited Method Call Stack Same File",
     &testInheritedMethodCallStackSameFile},
    {"Inherited Method Call Stack Different Files",
     &testInheritedMethodCallStackDifferentFiles},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcStringManager_create();
    sClassTestFilenameId = dcStringManager_getStringId(CLASS_TEST_FILE_NAME);
    dcTestUtilities_go("Class Test", _argc, _argv, NULL, sMap, false);

    if (dcTestUtilities_runSystem())
    {
        dcSystem_createWithArguments
            (dcTaffyCommandLineArguments_parseAndCreateWithFailure
             (_argc,
              _argv,
              false));
        dcTestUtilities_start("ClassTest", _argc, _argv);

        dcTestUtilities_assert
            (dcClassManager_registerClassTemplate
             (dcTestClass_getTemplate(), NULL, true, &sMetaObject)
             == TAFFY_SUCCESS);

        // test setter
        sObject = dcTestClass_createObject();
        sNodeEvaluator = dcSystem_getNodeEvaluator();

        dcGarbageCollector_addRoot(&marker);
        dcNode_register(sObject);

        dcTestUtilities_runTests(&resetTest, sRuntimeMap, true);
        dcTestUtilities_end();
        dcSystem_free();
    }

    dcGarbageCollector_free();
    dcStringManager_free();
    dcMemory_deinitialize();
    return 0;
}
