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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcArray.h"
#include "dcClass.h"
#include "dcCallStackData.h"
#include "dcClassTemplate.h"
#include "dcCommandLineArguments.h"
#include "dcError.h"
#include "dcFlatArithmetic.h"
#include "dcGarbageCollector.h"
#include "dcHash.h"
#include "dcUnsignedInt32.h"
#include "dcList.h"
#include "dcLog.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"
#include "dcString.h"
#include "dcSystem.h"

#define OUTPUT(...) fprintf(stderr, __VA_ARGS__)

#define TEST_NODES_SIZE 32
static char *sTestName = NULL;
dcContainerSizeType gTestNodesSize = 0;
dcNode **gTestNodes = NULL;
static bool sDoTick = true;
static dcList *sTestIndexList = NULL;
static bool sRunSystem = false;
static size_t sStopAt = 0;
static bool sLogTime = false;
static int sOnlyTestNumber = -1;
static dcString *sOnlyTestName = NULL;
static dcString *sStopAtName = NULL;

static void *runSynchronizedTest(void *_realTest)
{
    dcTestUtilities_testFunction function =
        (dcTestUtilities_testFunction )_realTest;
    (*function)();
    return NULL;
}

void dcTestUtilities_stopAt(size_t _testNumber)
{
    sStopAt = _testNumber;
}

void dcTestUtilities_go(const char *_name,
                        int _argc,
                        char **_argv,
                        dcTestUtilities_testFunction _initializer,
                        const dcTestFunctionMap *_map,
                        bool _synchronized)
{
    dcTestUtilities_start(_name, _argc, _argv);
    clock_t start = clock();
    dcTestUtilities_runTests(_initializer, _map, _synchronized);
    clock_t end = clock();

    if (sLogTime)
    {
        fprintf(stderr,
                "> time elapsed: %f\n",
                ((double)(end - start)) / CLOCKS_PER_SEC);
    }

    dcTestUtilities_end();
}

void dcTestUtilities_start(const char *_name, int _argc, char **_argv)
{
    dcTestUtilities_startAndCreateNodes(_name, _argc, _argv, true);
}

void dcTestUtilities_startAndCreateNodes(const char *_name,
                                         int _argc,
                                         char **_argv,
                                         bool _createNodes)
{
    dcMemory_free(sTestName);
    sTestName = dcMemory_strdup(_name);
    gTestNodesSize = 0;

    dcCommandLineArguments *arguments = dcCommandLineArguments_create();

    dcCommandLineArguments_registerArguments(arguments, "-l", "--log");
    dcCommandLineArguments_registerArguments(arguments, "-t", "--test-indexes");
    dcCommandLineArguments_registerArguments(arguments, "-r", "--run-system");
    dcCommandLineArguments_registerArguments(arguments,
                                             "-g",
                                             "--garbage-collector-object-tip");
    dcCommandLineArguments_registerArguments(arguments,
                                             "-s",
                                             "--stop-bootstrap-after-class");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--only-test");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--only-test-number");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--stop-at");
    dcCommandLineArguments_registerArguments(arguments, NULL, "--time");
    dcCommandLineArguments_registerArguments(arguments, NULL, "--rand-seed");

    if (! dcCommandLineArguments_parse(arguments, _argc, _argv, false))
    {
        fprintf(stderr, "Failure parsing command line.");
        exit(1);
    }

    OUTPUT("[%s]", sTestName);

    dcLog_configureFromCommandLineArguments(arguments);
    sTestIndexList = dcCommandLineArguments_getIntList
        (arguments, "--test-indexes");
    sOnlyTestNumber =
        dcCommandLineArguments_getInt(arguments, "--only-test-number", -1);
    const dcString *onlyTestName =
        dcCommandLineArguments_getString(arguments, "--only-test");

    if (onlyTestName != NULL)
    {
        sOnlyTestName = dcString_copy(onlyTestName, DC_DEEP);
    }

    if (dcCommandLineArguments_getHit(arguments, "--rand-seed"))
    {
        srand(dcCommandLineArguments_getInt(arguments, "--rand-seed", 0));
    }

    sRunSystem = dcCommandLineArguments_getHit(arguments, "--run-system");
    sLogTime = dcCommandLineArguments_getHit(arguments, "--time");
    const dcString *stopAtName =
        dcCommandLineArguments_getString(arguments, "--stop-at");

    if (stopAtName != NULL)
    {
        sStopAtName = dcString_copy(stopAtName, DC_DEEP);
    }

    dcCommandLineArguments_free(&arguments);
    dcGarbageCollector_addRoot(&dcTestUtilities_markNodes);

    if (_createNodes)
    {
        dcContainerSizeType i;
        dcError_assert(gTestNodes == NULL);
        gTestNodesSize = TEST_NODES_SIZE;
        gTestNodes = (dcNode **)(dcMemory_allocateAndInitialize
                                 (sizeof(dcNode **) * gTestNodesSize));

        for (i = 0; i < gTestNodesSize; i++)
        {
            gTestNodes[i] = dcNode_register(dcUnsignedInt32_createNode(i));
            dcNode_setFreeTrapped(gTestNodes[i], true);
        }

        dcGarbageCollector_addRoot(&dcTestUtilities_markNodes);
    }
}

bool dcTestUtilities_runSystem(void)
{
    return sRunSystem;
}

void dcTestUtilities_markNodes(void)
{
    dcTestUtilities_doMark(true);
}

void dcTestUtilities_doMark(bool _yesno)
{
    dcContainerSizeType i;

    for (i = 0; i < gTestNodesSize; i++)
    {
        dcNode_markYesNo(gTestNodes[i], _yesno);
    }
}

void dcTestUtilities_checkMarks(bool _yesno)
{
    dcContainerSizeType i;

    for (i = 0; i < gTestNodesSize; i++)
    {
        dcTestUtilities_assert(dcNode_isMarked(gTestNodes[i]) == _yesno);
    }
}

void dcTestUtilities_end(void)
{
    dcGarbageCollector_lock();
    dcContainerSizeType i;

    for (i = 0; i < gTestNodesSize; i++)
    {
        dcNode_setFreeTrapped(gTestNodes[i], false);
    }

    dcGarbageCollector_removeRoot(&dcTestUtilities_markNodes);
    dcList_free(&sTestIndexList, DC_DEEP);
    dcMemory_free(sTestName);
    dcMemory_free(gTestNodes);
    gTestNodesSize = 0;
    dcString_free(&sStopAtName, DC_DEEP);
    dcString_free(&sOnlyTestName, DC_DEEP);

    dcGarbageCollector_unlock();

    OUTPUT("[done]\n");
}

static void tick(void)
{
    if (sDoTick)
    {
        OUTPUT(".");
    }
}

void dcTestUtilities_checkEqual(const dcNode *_first,
                                const dcNode *_second)
{
    dcError_check(_first == _second,
                  "pointers are unequal for: '%s'/%p and '%s'/%p\n",
                  dcNode_display(_first),
                  _first,
                  dcNode_display(_second),
                  _second);
    tick();
}

void dcTestUtilities_checkIntEqual(int _first, int _second)
{
    dcError_check(_first == _second,
                  "ints %d and %d are unequal",
                  _first,
                  _second);
    tick();
}

void dcTestUtilities_checkArraySize(const dcArray *_array,
                                    dcContainerSizeType _wanted)
{
    dcError_check(_array->size == _wanted,
                  "wanted array size: %u, but got: %u\n",
                  _wanted,
                  _array->size);
    tick();
}

void dcTestUtilities_checkListSize(const dcList *_list,
                                   dcContainerSizeType _wanted)
{
    dcError_check(_list->size == _wanted,
                  "wanted list size: %u, but got: %u\n",
                  _wanted,
                  _list->size);
    tick();
}

void dcTestUtilities_checkBool(bool _got, const char *_description)
{
    if (! _got)
    {
        fprintf(stderr, "boolean check failed for: %s\n", _description);
        abort();
    }

    tick();
}

void dcTestUtilities_checkNonNull(const void *_pointer)
{
    dcError_check(_pointer != NULL, "_pointer is NULL but shouldn't be");
    tick();
}

void dcTestUtilities_checkHashSize(const dcHash *_hash,
                                   dcContainerSizeType _wanted)
{
    dcError_check(_hash->size == _wanted,
                  "wanted hash size: %u, but got: %u\n",
                  _wanted,
                  _hash->size);
    tick();
}

static void printHeaderLine(size_t _width)
{
    size_t i;

    for (i = 0; i < _width; i++)
    {
        OUTPUT("-");
    }

    OUTPUT("\n");
}

void dcTestUtilities_runTests(dcTestUtilities_testFunction _initializer,
                              const dcTestFunctionMap *_map,
                              bool _synchronized)
{
    OUTPUT("\n");
    const char testString[] = "Test";
    const char resultString[] = "Result";
    int maxWidth = 0;

    sDoTick = false;
    const dcTestFunctionMap *finger = _map;
    int smudge = 10;

    if (_map != NULL)
    {
        for (finger = _map; finger->name != NULL; finger++)
        {
            size_t width = strlen(finger->name);

            if ((int)width > maxWidth)
            {
                maxWidth = width;
            }
        }

        maxWidth += smudge;

        size_t headerWidth = maxWidth + sizeof(resultString);

        // print the test header //
        printHeaderLine(headerWidth);
        OUTPUT("%s %*s\n", testString, maxWidth + 2, resultString);
        printHeaderLine(headerWidth);
        dcListElement *thisTestIndex = sTestIndexList->head;

        int testNumber = 0;

        for (finger = _map; finger->name != NULL; finger++, testNumber++)
        {
            if (sStopAt != 0
                && (size_t)(finger - _map) >= sStopAt)
            {
                break;
            }

            if (sStopAtName != NULL
                && strcmp(finger->name, sStopAtName->string) == 0)
            {
                break;
            }

            if (sOnlyTestNumber != -1
                && sOnlyTestNumber != testNumber)
            {
                continue;
            }

            if (sOnlyTestName != NULL
                && strcmp(finger->name, sOnlyTestName->string) != 0)
            {
                continue;
            }

            if (sTestIndexList->size == 0
                || (thisTestIndex != NULL
                    && ((uint32_t)(finger - _map)
                        == dcUnsignedInt32_getInt(thisTestIndex->object))))
            {
                if (_initializer != NULL)
                {
                    _initializer();
                }

                OUTPUT("%-*s", maxWidth, finger->name);

                if (_synchronized)
                {
                    dcNodeEvaluator_synchronizeFunctionCall
                        (dcSystem_getCurrentNodeEvaluator(),
                         &runSynchronizedTest,
                         (void *)finger->function);
                }
                else
                {
                    (*finger->function)();
                }

                OUTPUT(" [PASS]\n");

                if (thisTestIndex != NULL)
                {
                    thisTestIndex = thisTestIndex->next;
                }
            }
        }
    }
}

void dcTestUtilities_checkMark(const dcNode *_node, bool _marked)
{
    dcTestUtilities_assert(dcNode_isMarked(_node) == _marked);
}

void dcTestUtilities_assertException(dcNode *_result,
                                     dcNodeEvaluator *_evaluator,
                                     const char *_exceptionName,
                                     const char *_objectName,
                                     const char *_expectedValue,
                                     bool _clearException)
{
    dcTestUtilities_assert(_result == NULL && _evaluator->exception != NULL);
    dcTestUtilities_assert
        (strcmp(dcClass_getTemplate(_evaluator->exception)->className,
                _exceptionName)
         == 0);

    if (_objectName != NULL)
    {
        dcNode *object = dcClass_getObject(_evaluator->exception, _objectName);
        dcTestUtilities_assert(object != NULL);
        dcTestUtilities_assert(strcmp(dcNode_display(object),
                                      _expectedValue)
                               == 0);
    }

    if (_clearException)
    {
        dcNodeEvaluator_clearException(_evaluator, DC_SHALLOW);
    }
}

static bool callStackDataEquals(const dcCallStackData *_left,
                                const dcCallStackData *_right)
{
    return (_left->filenameId == _right->filenameId
            && ((_left->methodName == NULL
                 && _right->methodName == NULL)
                || (_left->methodName != NULL
                    && _right->methodName != NULL
                    && (strcmp(_left->methodName, _right->methodName) == 0))));
}

void dcTestUtilities_assertCallStack(dcNode *_result,
                                     dcNodeEvaluator *_evaluator,
                                     dcList *_expectedCallStack)
{
    dcTestUtilities_assert(_result == NULL && _evaluator->exception != NULL);
    dcTestUtilities_assert(_evaluator->exceptionCallStack->size
                           == _expectedCallStack->size);
    dcListElement *that;
    dcListElement *thatExpected;

    for (that = _evaluator->exceptionCallStack->head,
             thatExpected = _expectedCallStack->head;
         that != NULL;
         that = that->next,
             thatExpected = thatExpected->next)
    {
        const dcCallStackData *wanted = CAST_CALL_STACK_DATA(that->object);
        const dcCallStackData *expected =
            CAST_CALL_STACK_DATA(thatExpected->object);

        dcError_check(callStackDataEquals(wanted, expected),
                      "call stack datas are unequal\ngot:\n%s\nwanted:\n%s\n",
                      dcCallStackData_display(wanted),
                      dcCallStackData_display(expected));
    }

    dcList_free(&_expectedCallStack, DC_DEEP);
    dcNodeEvaluator_clearException(_evaluator, DC_SHALLOW);
}

static void notEqual(const char *_expected, const char *_got)
{
    fprintf(stderr,
            "\n\n"
            "Expected [%s]\n"
            "but got: [%s]\n\n",
            _expected,
            _got);
    //dcError_assert(false);
}

void dcTestUtilities_expectEqual(dcNode **_node, dcNode **_expected)
{
    if ((*_node == NULL && *_expected != NULL)
        || (*_node != NULL && *_expected == NULL))
    {
        char *expectedDisplay = dcNode_synchronizedDisplay(*_expected);
        char *nodeDisplay = dcNode_synchronizedDisplay(*_node);
        notEqual(expectedDisplay, nodeDisplay);
        dcMemory_free(expectedDisplay);
        dcMemory_free(nodeDisplay);
    }
    else if (dcNode_easyCompare(*_node, *_expected) != TAFFY_EQUALS)
    {
        // TODO: do not necessitate me
        *_node = dcFlatArithmetic_shrink(*_node, NULL);
        *_expected = dcFlatArithmetic_shrink(*_expected, NULL);

        char *expectedDisplay = dcNode_synchronizedDisplay(*_expected);
        char *nodeDisplay = dcNode_synchronizedDisplay(*_node);

        if (dcNode_easyCompare(*_node, *_expected) != TAFFY_EQUALS)
        {
            notEqual(expectedDisplay, nodeDisplay);
        }

        dcMemory_free(expectedDisplay);
        dcMemory_free(nodeDisplay);
    }

}

void dcTestUtilities_expectStringEqual(const char *_expected, const char *_got)
{
    if (((_expected == NULL && _got != NULL)
         || (_expected != NULL && _got == NULL))
        || strcmp(_expected, _got) != 0)
    {
        notEqual(_expected, _got);
    }
}

void dcTestUtilities_expectStringNodeEqual(const char *_expected, dcNode *_node)
{
    char *display = dcNode_synchronizedDisplay(_node);
    dcTestUtilities_expectStringEqual(_expected, display);
    dcMemory_free(display);
}

void dcTestUtilities_expectNodeStringsEqual(dcNode *_expected, dcNode *_node)
{
    char *nodeDisplay = dcNode_synchronizedDisplay(_node);
    char *expectedDisplay = dcNode_synchronizedDisplay(_expected);
    dcTestUtilities_expectStringEqual(expectedDisplay, nodeDisplay);
    dcMemory_free(nodeDisplay);
    dcMemory_free(expectedDisplay);
}

void dcTestUtilities_printBytes(uint8_t *_bytes, uint32_t _length)
{
    uint32_t i;

    for (i = 0; i < _length; i++)
    {
        fprintf(stderr, "%u ", _bytes[i]);
    }

    fprintf(stderr, "\n");
}
