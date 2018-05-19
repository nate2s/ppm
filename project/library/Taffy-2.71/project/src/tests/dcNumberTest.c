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
#include <math.h>

#include "dcComplexNumber.h"
#include "dcComplexNumberClass.h"
#include "dcDecNumber.h"
#include "dcDefines.h"
#include "dcError.h"
#include "dcFloat.h"
#include "dcGarbageCollector.h"
#include "dcInt32.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcNumberClass.h"
#include "dcPair.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"

static void assertNumberEquals(const dcNumber *_left, const dcNumber *_right);

static void testPrintBytes(void)
{
    // don't run by default
    return;

    const char *numbers[] =
        {
            "1",
            "111",
            "111.111",
            "111.111222333444555",
            "111.111222333444555666",
            "999888777666.111222333444555666",
            "12345678999",
            NULL
        };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(numbers) - 1; i++)
    {
        decNumber *number = dcDecNumber_createFromString(numbers[i]);
        char *bytes = dcDecNumber_displayBytes(number);
        char *display = dcDecNumber_display(number);
        printf("bytes for: %s/%s: %s\n", display, numbers[i], bytes);
        free(display);
        free(bytes);
        dcDecNumber_free(&number);
    }
}

static void testIsWhole(void)
{
    struct
    {
        const char *value;
        bool isWhole;
    } tests[] =
          {
              {"1", true},
              {"1.1", false},
              {"1.0", true},
              {"1.10", false},
              {"1.100", false},
              {"1.001", false},
              {"1.0001", false},
              {"1.0000", true},
              {"123497234987324987.0000", true},
              {"12349723494.0000000000000001", false},
              {NULL}
          };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests) - 1; i++)
    {
        decNumber *number = dcDecNumber_createFromString(tests[i].value);
        dcTestUtilities_assert(dcDecNumber_isWhole(number) == tests[i].isWhole);
        dcDecNumber_free(&number);
    }
}

static void testComparison(void)
{
    struct
    {
        const char *left;
        const char *right;
        dcTaffyOperator expectedResult;
    } tests[] =
          {
              {"1.0",                      "2.0",         TAFFY_LESS_THAN},
              {"1.0",                      "1.1",         TAFFY_LESS_THAN},
              {"1.0",                      "1.01",        TAFFY_LESS_THAN},
              {"1.0", "1.000000000000000000000001",       TAFFY_LESS_THAN},
              {"2.0",                      "1.00",        TAFFY_GREATER_THAN},
              {"1.01",                     "1.00",        TAFFY_GREATER_THAN},
              {"1.0000000000000000000010", "1.00",        TAFFY_GREATER_THAN},
              {"1.0",                      "1.00",        TAFFY_EQUALS},
              {"1.00001",                  "1.00001",     TAFFY_EQUALS},
              {"1.123456789",              "1.123456789", TAFFY_EQUALS},
              {NULL}
          };

    size_t i;

    for (i = 0; tests[i].left != NULL; i++)
    {
        dcTaffyOperator compareResult;
        decNumber *left = dcDecNumber_createFromString(tests[i].left);
        decNumber *right = dcDecNumber_createFromString(tests[i].right);
        dcTestUtilities_assert((dcDecNumber_compare(left, right, &compareResult)
                                == TAFFY_SUCCESS)
                               && compareResult == tests[i].expectedResult);
        dcDecNumber_free(&left);
        dcDecNumber_free(&right);
    }
}

typedef struct
{
    const char *left;
    const char *right;
    const char *expected;
    size_t lsuSize;
} CombinationTest;

typedef dcNumberResult (NumberCombinationFunction)(decNumber *_result,
                                                   const decNumber *_left,
                                                   const decNumber *_right);

static void assertEquals(const decNumber *_left, const decNumber *_right)
{
    dcTaffyOperator compareResult;
    if ((dcDecNumber_compare(_left,
                             _right,
                             &compareResult)
         != TAFFY_SUCCESS)
        || compareResult != TAFFY_EQUALS)
    {
        char *leftDisplay = dcDecNumber_display(_left);
        char *rightDisplay = dcDecNumber_display(_right);
        fprintf(stderr,
                "Compare failed:\n"
                "left:  %s\n"
                "right: %s\n",
                leftDisplay,
                rightDisplay);
        dcMemory_free(leftDisplay);
        dcMemory_free(rightDisplay);
        dcError_assert(false);
    }
}

static void testCombination(const CombinationTest _tests[],
                            NumberCombinationFunction *_combinationFunction)
{
    size_t i;

    for (i = 0; _tests[i].left != NULL; i++)
    {
        uint32_t lsuSizeSave = dcDecNumber_getLsuSize();

        if (_tests[i].lsuSize != 0)
        {
            dcDecNumber_setLsuSize(_tests[i].lsuSize);
        }

        decNumber *left = dcDecNumber_createFromString(_tests[i].left);
        decNumber *right = dcDecNumber_createFromString(_tests[i].right);
        decNumber *expected = dcDecNumber_createFromString(_tests[i].expected);

        _combinationFunction(left, left, right);
        assertEquals(left, expected);

        dcDecNumber_free(&left);
        dcDecNumber_free(&right);
        dcDecNumber_free(&expected);

        if (_tests[i].lsuSize != 0)
        {
            dcDecNumber_setLsuSize(lsuSizeSave);
        }
    }
}

static void testAdd(void)
{
    const CombinationTest tests[] =
        {
            {"0", "0", "0", 0},
            {"0", "1", "1", 0},
            {"1", "0", "1", 0},
            {"1", "1", "2", 0},
            {"1", "2", "3", 0},
            {"10000000000", "1", "10000000001", 0},
            {"1.123456789", "0.000000001", "1.123456790", 0},
            {"1000000000000000000000000000000.1",
             "0.1",
             "1000000000000000000000000000000.2",
             64},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_add);
}

static void testSubtract(void)
{
    const CombinationTest tests[] =
        {
            {"0", "0", "0", 0},
            {"0", "1", "-1", 0},
            {"1", "0", "1", 0},
            {"1", "1", "0", 0},
            {"1", "2", "-1", 0},
            {"10000000000", "1", "9999999999", 0},
            {"1.123456789", "0.000000001", "1.123456788", 0},
            {"1000000000000000000000000000000000000000.1",
             "0.1",
             "1000000000000000000000000000000000000000.0",
             64},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_subtract);
}

static void testMultiply(void)
{
    const CombinationTest tests[] =
        {
            {"0", "0", "0", 0},
            {"0", "1", "0", 0},
            {"1", "0", "0", 0},
            {"1", "1", "1", 0},
            {"1", "2", "2", 0},
            {"10000000000", "1", "10000000000", 0},
            {"1.123456789", "0.000000001", "0.000000001123456789", 0},
            {"1000000000000000000000000000000.1",
             "2",
             "2000000000000000000000000000000.2",
             64},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_multiply);
}

static void testDivide(void)
{
    const CombinationTest tests[] =
        {
            {"0", "1", "0", 0},
            {"1", "1", "1", 0},
            {"1", "2", "0.5", 0},
            {"10000000000", "1", "10000000000", 0},
            {"1.123456789", "0.000000001", "1123456789", 0},
            {"1000000000000000000000000000000.2",
             "2",
             "500000000000000000000000000000.1",
             64},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_divide);
}

static decNumber *createFromLsuSize(size_t _lsuSize)
{
    return (dcDecNumber_createWithLsuSize
            (_lsuSize == 0
             ? dcDecNumber_getLsuSize()
             : _lsuSize));
}

typedef struct
{
    const char *value;
    const char *expected;
    size_t lsuSize;
} Log10Test;

static void testLog10(void)
{
    const Log10Test tests[] =
        {
            {"10", "1", 0},
            {"100", "2", 0},
            {"100", "2", 0},
            {"20",  "1.30102999566", 12},
            {"1000000000000000000000000000000000000000000000000",  "48", 52},
            {NULL}
        };

    size_t i;

    for (i = 0; tests[i].value != NULL; i++)
    {
        decNumber *value = dcDecNumber_createFromString(tests[i].value);
        decNumber *expected = dcDecNumber_createFromString(tests[i].expected);
        decNumber *logged = createFromLsuSize(tests[i].lsuSize);
        dcDecNumber_log10(logged, value);
        assertEquals(logged, expected);
        dcDecNumber_free(&value);
        dcDecNumber_free(&expected);
        dcDecNumber_free(&logged);
    }
}

static void testLg(void)
{
    const Log10Test tests[] =
        {
            {"10", "1", 0},
            {"100", "2", 0},
            {"100", "2", 0},
            {"20",  "1.30102999566", 12},
            {"1000000000000000000000000000000000000000000000000",  "48", 52},
            {NULL}
        };

    size_t i;

    for (i = 0; tests[i].value != NULL; i++)
    {
        decNumber *value = dcDecNumber_createFromString(tests[i].value);
        decNumber *expected = dcDecNumber_createFromString(tests[i].expected);
        decNumber *logged = createFromLsuSize(tests[i].lsuSize);
        dcDecNumber_log10(logged, value);
        assertEquals(logged, expected);
        dcDecNumber_free(&value);
        dcDecNumber_free(&expected);
        dcDecNumber_free(&logged);
    }
}

static void testModulus(void)
{
    const CombinationTest tests[] =
        {
            {"2",  "2", "0", 0},
            {"3",  "2", "1", 0},
            {"20", "8", "4", 0},
            {"100000000000000000000000000000000", "2", "0", 46},
            {"2", "10000000000000000000000000000000", "2", 46},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_modulus);
}

static void testRaise(void)
{
    const CombinationTest tests[] =
        {
            {"10",  "0", "1",     0},
            {"1",   "1", "1",     0},
            {"1",   "2", "1",     0},
            {"100", "2", "10000", 0},
            {"1.5", "2", "2.25",  0},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_raise);
}

static void testOr(void)
{
    const CombinationTest tests[] =
        {
            {"0", "0", "0", 0},
            {"0", "1", "1", 0},
            {"1", "0", "1", 0},
            {"1", "1", "1", 0},
            {"3333333", "1", "3333333", 0},
            {"10000000000", "1", "10000000001", 0},
            {"10000000000000000000000000000000",
             "2",
             "10000000000000000000000000000002",
             64},
            {"11111111111111111111",
             "1111111111111111111",
             "11492057805447000519",
             64},
            {"0", "16777215", "16777215", 0},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_or);
}

static void testXOr(void)
{
    const CombinationTest tests[] =
        {
            {"0", "0", "0", 0},
            {"0", "1", "1", 0},
            {"1", "0", "1", 0},
            {"1", "1", "0", 0},
            {"3333333", "1", "3333332", 0},
            {"10000000000", "1", "10000000001", 0},
            {"10000000000000000000000000000000",
             "2",
             "10000000000000000000000000000002",
             64},
            {"324987234987234",
             "23423411144",
             "325006761807658"},
            {"11111111111111111111",
             "1111111111111111111",
             "10761893388671778816",
             64},
            {"0", "16777215", "16777215", 0},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_xor);
}

static void testAnd(void)
{
    const CombinationTest tests[] =
        {
            {"0", "0", "0", 0},
            {"0", "1", "0", 0},
            {"1", "0", "0", 0},
            {"1", "1", "1", 0},
            {"1", "2", "0", 0},
            {"1", "3", "1", 0},
            {"10000000000", "1", "0", 0},
            {"1000000000000000000000000000000000000000", "2", "0", 64},
            {"11111111111111111111",
             "1111111111111111111",
             "730164416775221703"},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_and);
}

static void testLeftShift(void)
{
    const CombinationTest tests[] =
        {
            {"0",  "1",   "0",  0},
            {"1",  "0",   "1",  0},
            {"7",  "2",   "28", 0},
            {"14", "1",   "28", 0},
            {"2",  "50",  "2251799813685248", 100},

            {"0",  "-1",   "0",  0},
            {"7",  "-2",   "1", 0},
            {"14", "-1",   "7", 0},
            {"14", "-100", "0", 0},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_leftShift);
}

static void testRightShift(void)
{
    const CombinationTest tests[] =
        {
            {"0",  "1",   "0",  0},
            {"1",  "0",   "1",  0},
            {"7",  "2",   "1", 0},
            {"14", "1",   "7", 0},
            {"14", "100", "0", 0},

            {"0",  "-1",   "0",    0},
            {"7",  "-2",   "28",   0},
            {"14", "-1",   "28",   0},
            {"2",  "-50",  "2251799813685248", 0},
            {NULL}
        };

    testCombination(tests, &dcDecNumber_rightShift);
}

typedef struct
{
    const char *value;
    const char *expected;
    uint32_t lsuSize;
} SingleOperandTest;

typedef dcNumberResult (SingleOperandFunction)(decNumber *_result,
                                               const decNumber *_value);

static void testSingleOperandFunction(const SingleOperandTest *_tests,
                                      SingleOperandFunction *_function)
{
    size_t i;

    for (i = 0; _tests[i].value != NULL; i++)
    {
        uint32_t lsuSizeSave = dcDecNumber_getLsuSize();

        if (_tests[i].lsuSize != 0)
        {
            dcDecNumber_setLsuSize(_tests[i].lsuSize);
        }

        decNumber *value = dcDecNumber_createFromString(_tests[i].value);
        decNumber *result = dcDecNumber_createFromInt32u(0);
        decNumber *expected = dcDecNumber_createFromString(_tests[i].expected);

        // perform two tests, one with result == value, and one with it not
        dcTestUtilities_assert(_function(result, value)
                               == TAFFY_NUMBER_SUCCESS);
        assertEquals(expected, result);

        dcTestUtilities_assert(_function(value, value) == TAFFY_NUMBER_SUCCESS);
        assertEquals(expected, value);

        dcDecNumber_free(&result);
        dcDecNumber_free(&value);
        dcDecNumber_free(&expected);

        if (_tests[i].lsuSize != 0)
        {
            dcDecNumber_setLsuSize(lsuSizeSave);
        }
    }
}

typedef dcNumberResult (NoOperandFunction)(decNumber *_result);

static void testNoOperandFunction(const SingleOperandTest *_tests,
                                  NoOperandFunction *_function)
{
    size_t i;

    for (i = 0; _tests[i].value != NULL; i++)
    {
        uint32_t lsuSizeSave = dcDecNumber_getLsuSize();

        if (_tests[i].lsuSize != 0)
        {
            dcDecNumber_setLsuSize(_tests[i].lsuSize);
        }

        decNumber *value = dcDecNumber_createFromString(_tests[i].value);
        decNumber *expected = dcDecNumber_createFromString(_tests[i].expected);

        _function(value);
        assertEquals(expected, value);

        dcDecNumber_free(&value);
        dcDecNumber_free(&expected);

        if (_tests[i].lsuSize != 0)
        {
            dcDecNumber_setLsuSize(lsuSizeSave);
        }
    }
}

typedef struct
{
    const char *value;
    const char *expected;
} NumberSingleOperandTest;

typedef dcNumberResult (NumberNoOperandFunction)(dcNumber *_result);

static void testNumberNoOperandFunction(const NumberSingleOperandTest *_tests,
                                        NumberNoOperandFunction *_function)
{
    size_t i;

    for (i = 0; _tests[i].value != NULL; i++)
    {
        dcNumber *value = dcNumber_createFromString(_tests[i].value);
        dcNumber *expected = dcNumber_createFromString(_tests[i].expected);

        _function(value);
        assertNumberEquals(expected, value);

        dcNumber_free(&value, DC_DEEP);
        dcNumber_free(&expected, DC_DEEP);
    }
}

static void testFloor(void)
{
    const SingleOperandTest tests[] =
    {
        {"0",              "0", 0},
        {"0.0000002",      "0", 0},
        {"0.0",            "0", 0},
        {"0.5",            "0", 0},
        {"0.555555",       "0", 0},
        {"1",              "1", 0},
        {"1.2",            "1", 0},
        {"1.234987234",    "1", 0},
        {"12345678999.1",  "12345678999", 0},
        {"-1.234987234",   "-2", 0},
        {"50.9",           "50", 0},
        {"0.00000000000000000000000000000000000", "0", 100},
        {"0.99999999999999999999999999999999999", "0", 100},
        {NULL}
    };

    testSingleOperandFunction(tests, &dcDecNumber_floor);
}

static void testCeiling(void)
{
    const SingleOperandTest tests[] =
    {
        {"0",             "0", 0},
        {"0.0000002",     "1", 0},
        {"0.0",           "0", 0},
        {"1",             "1", 0},
        {"1.2",           "2", 0},
        {"1.234987234",   "2", 0},
        {"12345678999.1", "12345679000", 0},
        {"-1.234987234",  "-1", 0},
        {NULL}
    };

    testSingleOperandFunction(tests, &dcDecNumber_ceiling);
}

static void testFactorial(void)
{
    const SingleOperandTest tests[] =
    {
        {"100", "9.3326215443944152681699238856266700490715968264381621468592963895217599993229920E+157", 80},
        {"0",  "1", 0},
        {"1",  "1", 0},
        {"2",  "2", 0},
        {"3",  "6", 0},
        {"4",  "24", 0},
        {"20", "2432902008176640000", 300},
        {"30", "265252859812191058636308480000000", 300},
        {"40", "815915283247897734345611269596115894272000000000", 300},
        {"50", "30414093201713378043612608166064768844377641568960512000000000000", 300},
        {"60", "8320987112741390144276341183223364380754172606361245952449277696409600000000000000", 300},
        {"70", "11978571669969891796072783721689098736458938142546425857555362864628009582789845319680000000000000000", 300},
        {"99", "933262154439441526816992388562667004907159682643816214685929638952175999932299156089414639761565182862536979208272237582511852109168640000000000000000000000", 300},
        {"100", "93326215443944152681699238856266700490715968264381621468592963895217599993229915608941463976156518286253697920827223758251185210916864000000000000000000000000", 300},
        {"100", "9.3326215443944152681699238856264E+157", 32},
        {NULL}
    };

    testSingleOperandFunction(tests, &dcDecNumber_factorial);
}

static void testAbsoluteValue(void)
{
    static SingleOperandTest tests[] =
        {
            {"0",                 "0",                0},
            {"1",                 "1",                0},
            {"-1",                "1",                0},
            {"-1234987234987234", "1234987234987234", 0},
            {"1234987234987234",  "1234987234987234", 0},
            {NULL}
        };

    testSingleOperandFunction(tests, &dcDecNumber_absoluteValue);
}

static void testIncrement(void)
{
    static SingleOperandTest tests[] =
        {
            {"0",                  "1",                  0},
            {"-1",                 "0",                  0},
            {"123456789123456789", "123456789123456790", 0},
            {NULL}
        };

    testNoOperandFunction(tests, &dcDecNumber_increment);
}

static void testDecrement(void)
{
    static SingleOperandTest tests[] =
        {
            {"0",                  "-1",                 0},
            {"-1",                 "-2",                 0},
            {"123456789123456789", "123456789123456788", 0},
            {"1.0",                "0.0",                0},
            {"-2147483648",        "-2147483649",        0},
            {NULL}
        };

    testNoOperandFunction(tests, &dcDecNumber_decrement);
}

static void testSquareRoot(void)
{
    const SingleOperandTest tests[] =
        {
            {"0",   "0",             0},
            {"1",   "1",             0},
            {"25",  "5",             0},
            {"100", "10",            0},
            {"10",  "3.16227766017", 12},
            {"10005", "100.024996876", 12},
            {"152415787532388367501905199875019052100",
             "12345678901234567890",
             100},
            {NULL}
        };

    testSingleOperandFunction(tests, &dcDecNumber_squareRoot);
}

typedef struct
{
    double value;
    const char *expected;
    uint32_t lsuSize;
} DoubleConversionTest;

static void testDoubleConversion(void)
{
    static DoubleConversionTest tests[] =
        {
            {0,        "0",        0},
            {-1,       "-1",       0},
            {1,        "1",        0},
            {3.14158,  "3.14158",  0},
            {-3.14158, "-3.14158", 0}
        };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        uint32_t lsuSizeSave = dcDecNumber_getLsuSize();

        if (tests[i].lsuSize != 0)
        {
            dcDecNumber_setLsuSize(tests[i].lsuSize);
        }

        decNumber *value = dcDecNumber_createFromDouble(tests[i].value);
        decNumber *expected = dcDecNumber_createFromString(tests[i].expected);

        assertEquals(expected, value);

        dcDecNumber_free(&value);
        dcDecNumber_free(&expected);

        if (tests[i].lsuSize != 0)
        {
            dcDecNumber_setLsuSize(lsuSizeSave);
        }
    }
}

typedef dcNumberResult (*TrigonometricFunction)(decNumber *_result,
                                                const decNumber *_Value);

typedef double (*CMathFunction)(double _value);

static void testComputePi(void)
{
    static SingleOperandTest tests[] =
    {
        {"1", "3.14159265358979323846264338327", 64},
        {NULL}
    };

    testSingleOperandFunction(tests, &dcDecNumber_pi);
}

typedef struct
{
    const char *left;
    const char *right;
    const char *expected;
    dcNumberType expectedType;
} TwoOperandNumberTest;

typedef struct
{
    const char *left;
    const char *right;
    dcNumberResult expectedException;
} TwoOperandNumberExceptionTest;

static void assertNumberEquals(const dcNumber *_left, const dcNumber *_right)
{
    if (! dcNumber_equals(_left, _right))
    {
        char *left = dcNumber_display(_left);
        char *right = dcNumber_display(_right);

        fprintf(stderr,
                "number equality check failed: \n"
                "left:  %s\n"
                "right: %s\n",
                left,
                right);

        dcMemory_free(left);
        dcMemory_free(right);

        //dcError_assert(false);
    }
}

static void testTwoOperandNumberOperation(const TwoOperandNumberTest _tests[],
                                          dcNumber_arithmeticFunction _function)
{
    size_t i;
    uint32_t lsuSizeSave = dcDecNumber_getLsuSize();
    dcDecNumber_setLsuSize(1024);

    for (i = 0; _tests[i].left != NULL; i++)
    {
        dcNumber *left = dcNumber_createFromString(_tests[i].left);
        dcNumber *right = dcNumber_createFromString(_tests[i].right);
        dcNumber *expected = dcNumber_createFromString(_tests[i].expected);
        dcNumber *result = dcNumber_createFromInt32u(0);

        // perform two tests
        dcTestUtilities_assert(_function(result, left, right)
                               == TAFFY_NUMBER_SUCCESS);
        assertNumberEquals(result, expected);
        dcError_assert(result->type == _tests[i].expectedType);

        dcTestUtilities_assert(_function(left, left, right)
                               == TAFFY_NUMBER_SUCCESS);
        assertNumberEquals(left, expected);
        dcError_assert(expected->type == _tests[i].expectedType);

        dcNumber_free(&result, DC_DEEP);
        dcNumber_free(&left, DC_DEEP);
        dcNumber_free(&right, DC_DEEP);
        dcNumber_free(&expected, DC_DEEP);
    }

    dcDecNumber_setLsuSize(lsuSizeSave);
}

static void testTwoOperandNumberExceptionOperation
    (const TwoOperandNumberExceptionTest _tests[],
     dcNumber_arithmeticFunction _function)
{
    size_t i;

    for (i = 0; _tests[i].left != NULL; i++)
    {
        dcNumber *left = dcNumber_createFromString(_tests[i].left);
        dcNumber *right = dcNumber_createFromString(_tests[i].right);
        dcNumber *result = dcNumber_createFromInt32u(0);

        dcTestUtilities_assert(_function(result, left, right)
                               == _tests[i].expectedException);
        dcTestUtilities_assert(_function(left, left, right)
                               == _tests[i].expectedException);

        dcNumber_free(&result, DC_DEEP);
        dcNumber_free(&left, DC_DEEP);
        dcNumber_free(&right, DC_DEEP);
    }
}

static void testEquality(void)
{
    struct
    {
        const char *left;
        const char *right;
        bool equal;
    } tests[] =
      {
          {"1",  "1.0",  true},
          {"-1", "-1.0", true},
          {"-1", "-1.1", false},

          {"1.0",  "1",  true},
          {"-1.0", "-1", true},
          {"-1.1", "-1", false}
      };

    uint16_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNumber *left = dcNumber_createFromString(tests[i].left);
        dcNumber *right = dcNumber_createFromString(tests[i].right);

        dcTestUtilities_assert(dcNumber_equals(left, right)
                               == tests[i].equal);

        dcNumber_free(&left, DC_DEEP);
        dcNumber_free(&right, DC_DEEP);
    }
}

static void testNumberAdd(void)
{
    const TwoOperandNumberTest tests[] =
        {
            // basic integer addition
            {"1",  "2",  "3",  NUMBER_INTEGER_TYPE},
            {"-1", "-2", "-3", NUMBER_INTEGER_TYPE},
            {"-1", "2",  "1",  NUMBER_INTEGER_TYPE},

            // integer to dec number
            {"1",   "2.5",    "3.5",    NUMBER_DEC_NUMBER_TYPE},
            {"1",   "65535",  "65536",  NUMBER_DEC_NUMBER_TYPE},
            {"1",   "65536",  "65537",  NUMBER_DEC_NUMBER_TYPE},
            {"-1",  "-65535", "-65536", NUMBER_DEC_NUMBER_TYPE},
            {"2.5", "2",      "4.5",    NUMBER_DEC_NUMBER_TYPE},

            // dec number addition
            {"1.5", "2.5", "4.0", NUMBER_DEC_NUMBER_TYPE},

            {"1.111111111111111",
             "2.111111111111111",
             "3.222222222222222",
             NUMBER_DEC_NUMBER_TYPE},

            {"33333333333333333333333333",
             "11111111111111111111111111",
             "44444444444444444444444444",
             NUMBER_DEC_NUMBER_TYPE},

            {"100000000000000000000.000000000003",
             "0.000000000003",
             "100000000000000000000.000000000006",
             NUMBER_DEC_NUMBER_TYPE},

            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_add);
}

static void testNumberSubtract(void)
{
    const TwoOperandNumberTest tests[] =
        {
            // basic integer subtraction
            {"2",  "1",     "1",      NUMBER_INTEGER_TYPE},
            {"1",  "2",     "-1",     NUMBER_INTEGER_TYPE},
            {"1",  "10",    "-9",     NUMBER_INTEGER_TYPE},
            {"-1", "-10",   "9",      NUMBER_INTEGER_TYPE},
            {"1",  "-1000", "1001.0", NUMBER_DEC_NUMBER_TYPE},

            // integer to dec number
            {"1",   "2.5",    "-1.5",   NUMBER_DEC_NUMBER_TYPE},
            {"1",   "-65535", "65536",  NUMBER_DEC_NUMBER_TYPE},
            {"1",   "65536",  "-65535", NUMBER_DEC_NUMBER_TYPE},
            {"2.5", "-1",     "3.5",    NUMBER_DEC_NUMBER_TYPE},

            // dec number subtraction
            {"1.5", "-2.5", "4.0", NUMBER_DEC_NUMBER_TYPE},

            {"1.111111111111111",
             "-2.111111111111111",
             "3.222222222222222",
             NUMBER_DEC_NUMBER_TYPE},

            {"33333333333333333333333333",
             "-11111111111111111111111111",
             "44444444444444444444444444",
             NUMBER_DEC_NUMBER_TYPE},

            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_subtract);
}

static void testNumberMultiply(void)
{
    const TwoOperandNumberTest tests[] =
        {
            // basic integer multiplication
            {"1",  "2",  "2",  NUMBER_INTEGER_TYPE},
            {"-1", "-2", "2",  NUMBER_INTEGER_TYPE},
            {"1",  "-2", "-2", NUMBER_INTEGER_TYPE},
            {"-1", "2",  "-2", NUMBER_INTEGER_TYPE},

            // integer to dec number or not
            {"1",   "2.5",   "2.5",     NUMBER_DEC_NUMBER_TYPE},
            {"2",   "32768", "65536",   NUMBER_DEC_NUMBER_TYPE},
            {"2",   "32767", "65534.0", NUMBER_DEC_NUMBER_TYPE},
            {"2.5", "2",     "5.0",     NUMBER_DEC_NUMBER_TYPE},

            // dec number * dec number
            {"1.5", "2.5", "3.75", NUMBER_DEC_NUMBER_TYPE},
            {"333333333333333",
             "111111111111111",
             "37037037037036962962962962963",
             NUMBER_DEC_NUMBER_TYPE},
            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_multiply);
}

static void testNumberDivide(void)
{
    const TwoOperandNumberTest tests[] =
        {
            {"1",  "2",  "0.5",  NUMBER_DEC_NUMBER_TYPE},
            {"2",  "2",  "1.0",  NUMBER_DEC_NUMBER_TYPE},
            {"-1", "-2", "0.5",  NUMBER_DEC_NUMBER_TYPE},
            {"-1", "2",  "-0.5", NUMBER_DEC_NUMBER_TYPE},
            {"-1", "2",  "-0.5", NUMBER_DEC_NUMBER_TYPE},

            // dec number / integer
            {"1.0", "2", "0.5", NUMBER_DEC_NUMBER_TYPE},

            // integer / dec number
            {"8", "2.0", "4.0", NUMBER_DEC_NUMBER_TYPE},

            // dec number / dec number
            {"8.0", "2.0", "4.0", NUMBER_DEC_NUMBER_TYPE},
            {"2222222222222222222222222",
             "2",
             "1111111111111111111111111",
             NUMBER_DEC_NUMBER_TYPE},
            {"2222222222222222222222222.66666",
             "2",
             "1111111111111111111111111.33333",
             NUMBER_DEC_NUMBER_TYPE},

            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_divide);
}

static void testNumberRaise(void)
{
    const TwoOperandNumberTest tests[] =
        {
            //{"1",   "2",   "1",    NUMBER_DEC_NUMBER_TYPE},
            //{"2",   "2",   "4",    NUMBER_DEC_NUMBER_TYPE},
            {"1.5", "2",   "2.25", NUMBER_DEC_NUMBER_TYPE},
            {"2",   "2.0", "4.0",  NUMBER_DEC_NUMBER_TYPE},
            {"2.0", "2.0", "4.0",  NUMBER_DEC_NUMBER_TYPE},
            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_raise);
}

static void testNumberAnd(void)
{
    const TwoOperandNumberTest tests[] =
        {
            {"1", "1", "1", NUMBER_INTEGER_TYPE},
            {"3", "1", "1", NUMBER_INTEGER_TYPE},
            {"4", "1", "0", NUMBER_INTEGER_TYPE},
            {"11111111111111111111",
             "1111111111111111111",
             "730164416775221703",
             NUMBER_DEC_NUMBER_TYPE},
            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_bitAnd);

    const TwoOperandNumberExceptionTest exceptionTests[] =
        {
            {"1",               "2.1",            TAFFY_NUMBER_NEED_INTEGER},
            {"1.1",             "2",              TAFFY_NUMBER_NEED_INTEGER},
            {"1.1",             "2.1",            TAFFY_NUMBER_NEED_INTEGER},
            {"1111111111111.1", "2",              TAFFY_NUMBER_NEED_INTEGER},
            {"2",               "991111111111.1", TAFFY_NUMBER_NEED_INTEGER},
            {"991111111111.1",  "111111111111.1", TAFFY_NUMBER_NEED_INTEGER},
            {NULL}
        };

    testTwoOperandNumberExceptionOperation(exceptionTests, &dcNumber_bitAnd);
}

static void testNumberOr(void)
{
    const TwoOperandNumberTest tests[] =
        {
            {"1", "1", "1", NUMBER_INTEGER_TYPE},
            {"3", "1", "3", NUMBER_INTEGER_TYPE},
            {"4", "1", "5", NUMBER_INTEGER_TYPE},
            {"11111111111111111111",
             "1111111111111111111",
             "11492057805447000519",
             NUMBER_DEC_NUMBER_TYPE},
            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_bitOr);

    const TwoOperandNumberExceptionTest exceptionTests[] =
        {
            {"1",               "2.1",            TAFFY_NUMBER_NEED_INTEGER},
            {"1.1",             "2",              TAFFY_NUMBER_NEED_INTEGER},
            {"1.1",             "2.1",            TAFFY_NUMBER_NEED_INTEGER},
            {"1111111111111.1", "2",              TAFFY_NUMBER_NEED_INTEGER},
            {"2",               "991111111111.1", TAFFY_NUMBER_NEED_INTEGER},
            {"991111111111.1",  "111111111111.1", TAFFY_NUMBER_NEED_INTEGER},
            {NULL}
        };

    testTwoOperandNumberExceptionOperation(exceptionTests, &dcNumber_bitOr);
}

static void testNumberXOr(void)
{
    const TwoOperandNumberTest tests[] =
        {
            {"1", "1", "0", NUMBER_INTEGER_TYPE},
            {"3", "1", "2", NUMBER_INTEGER_TYPE},
            {"4", "1", "5", NUMBER_INTEGER_TYPE},
            {"11111111111111111111",
             "1111111111111111111",
             "10761893388671778816",
             NUMBER_DEC_NUMBER_TYPE},
            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_bitXOr);

    const TwoOperandNumberExceptionTest exceptionTests[] =
        {
            {"1",               "2.1",            TAFFY_NUMBER_NEED_INTEGER},
            {"1.1",             "2",              TAFFY_NUMBER_NEED_INTEGER},
            {"1.1",             "2.1",            TAFFY_NUMBER_NEED_INTEGER},
            {"1111111111111.1", "2",              TAFFY_NUMBER_NEED_INTEGER},
            {"2",               "991111111111.1", TAFFY_NUMBER_NEED_INTEGER},
            {"991111111111.1",  "111111111111.1", TAFFY_NUMBER_NEED_INTEGER},
            {NULL}
        };

    testTwoOperandNumberExceptionOperation(exceptionTests, &dcNumber_bitXOr);
}

static void testNumberSingleOperandFunction
    (const NumberSingleOperandTest *_tests,
     dcNumber_singleOperandFunction _function)
{
    size_t i;

    for (i = 0; _tests[i].value != NULL; i++)
    {
        dcNumber *value = dcNumber_createFromString(_tests[i].value);
        dcNumber *expected = dcNumber_createFromString(_tests[i].expected);
        dcNumber *result = dcNumber_createFromInt32u(0);

        dcTestUtilities_assert(_function(result, value)
                               == TAFFY_NUMBER_SUCCESS);
        dcTestUtilities_assert(_function(value, value)
                               == TAFFY_NUMBER_SUCCESS);
        assertNumberEquals(expected, value);

        dcNumber_free(&result, DC_DEEP);
        dcNumber_free(&value, DC_DEEP);
        dcNumber_free(&expected, DC_DEEP);
    }
}

static void testNumberDeltaEqual(void)
{
    struct
    {
        const char *left;
        const char *right;
        uint32_t delta;
        bool expectedResult;
    } tests[] =
          {
              {"0", "0",   0, true},
              {"0", "1",   0, true},
              {"0", "2",   0, false},
              {"0", "-1",  0, true},
              {"0", "-2",  0, false},
              {"0", "1",   1, false},
              {"0", "2",   1, false},
              {"0", "-1",  1, false},
              {"0", "-2",  1, false},

              {"0", "0.1", 0, true},

              {"0", "0.1", 1, true},
              {"0", "0.1", 2, false},
              {"0", "0.1", 3, false},

              {"0.1", "0", 1, true},
              {"0.1", "0", 2, false},
              {"0.1", "0", 3, false},

              {"333333333333333333333333333",
               "333333333333333333333333333",
               0,
               true},

              {"333333333333333333333333333",
               "333333333333333333333333333.01",
               1,
               true},

              {"333333333333333333333333333",
               "333333333333333333333333333.01",
               2,
               true},

              {"333333333333333333333333333",
               "333333333333333333333333333.01",
               3,
               false},

              {"0.9092974268256816953960201260866784483672715732544267001315360707",
               "0.9092974268256816953960198659117450289251091622455175277076082870",
               6,
              true},

              {NULL}
          };

    uint32_t i;

    for (i = 0; tests[i].left != NULL; i++)
    {
        dcNumber *left = dcNumber_createFromString(tests[i].left);
        dcNumber *right = dcNumber_createFromString(tests[i].right);

        dcTestUtilities_assert(dcNumber_deltaEqual(left, right, tests[i].delta)
                               == tests[i].expectedResult);

        dcNumber_free(&left, DC_DEEP);
        dcNumber_free(&right, DC_DEEP);
    }
}

static void testNumberModulus(void)
{
    const TwoOperandNumberTest tests[] =
        {
            // basic integer addition
            {"1",   "2",     "1",   NUMBER_INTEGER_TYPE},
            {"-1",  "-2",    "-1",  NUMBER_INTEGER_TYPE},
            {"-1",  "2",     "-1",  NUMBER_INTEGER_TYPE},
            {"1",   "2.5",   "1.0", NUMBER_DEC_NUMBER_TYPE},
            {"30",  "1",     "0",   NUMBER_INTEGER_TYPE},
            {"30",  "29",    "1",   NUMBER_INTEGER_TYPE},
            {"30",   "10.1", "9.8", NUMBER_DEC_NUMBER_TYPE},
            {"30.0", "10.1", "9.8", NUMBER_DEC_NUMBER_TYPE},
            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_modulus);
}

static void testNumberLeftShift(void)
{
    const TwoOperandNumberTest tests[] =
        {
            {"0",  "1",   "0.0",  NUMBER_DEC_NUMBER_TYPE},
            {"1",  "0",   "1.0",  NUMBER_DEC_NUMBER_TYPE},
            {"7",  "2",   "28.0", NUMBER_DEC_NUMBER_TYPE},
            {"14", "1",   "28.0", NUMBER_DEC_NUMBER_TYPE},
            {"2",  "50",  "2251799813685248", NUMBER_DEC_NUMBER_TYPE},

            {"100000000000000000",
             "1",
             "200000000000000000",
             NUMBER_DEC_NUMBER_TYPE},

            {"0",  "-1",   "0.0", NUMBER_DEC_NUMBER_TYPE},
            {"7",  "-2",   "1.0", NUMBER_DEC_NUMBER_TYPE},
            {"14", "-1",   "7.0", NUMBER_DEC_NUMBER_TYPE},
            {"2",  "-50",  "0.0", NUMBER_DEC_NUMBER_TYPE},
            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_leftShift);
}

static void testNumberRightShift(void)
{
    const TwoOperandNumberTest tests[] =
        {
            {"0",  "1",   "0.0", NUMBER_DEC_NUMBER_TYPE},
            {"1",  "0",   "1.0", NUMBER_DEC_NUMBER_TYPE},
            {"7",  "2",   "1.0", NUMBER_DEC_NUMBER_TYPE},
            {"14", "1",   "7.0", NUMBER_DEC_NUMBER_TYPE},
            {"2",  "-50", "2251799813685248", NUMBER_DEC_NUMBER_TYPE},

            {"0",  "-1",   "0.0",  NUMBER_DEC_NUMBER_TYPE},
            {"7",  "-2",   "28.0", NUMBER_DEC_NUMBER_TYPE},
            {"14", "-1",   "28.0", NUMBER_DEC_NUMBER_TYPE},
            {"2",  "50",   "0.0",  NUMBER_DEC_NUMBER_TYPE},
            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_rightShift);
}

static void testNumberSquareRoot(void)
{
    const NumberSingleOperandTest tests[] =
        {
            {"0",   "0"},
            {"1",   "1"},
            {"25",  "5"},
            {"100", "10"},
            {"10",  "3.162277660168379"},
            {NULL}
        };

    testNumberSingleOperandFunction(tests, &dcNumber_squareRoot);
}

static void testNumberAbsoluteValue(void)
{
    const NumberSingleOperandTest tests[] =
        {
            {"0",    "0"},
            {"1",    "1"},
            {"-1",   "1"},
            {"1.0",  "1.0"},
            {"-1.0", "1.0"},
            {"-1.1", "1.1"},
            {NULL}
        };

    testNumberSingleOperandFunction(tests, &dcNumber_absoluteValue);
}

static void testNumberFloor(void)
{
    const NumberSingleOperandTest tests[] =
    {
        {"0",             "0"},
        {"0.0000002",     "0"},
        {"0.0",           "0"},
        {"1",             "1"},
        {"1.2",           "1"},
        {"1.234987234",   "1"},
        {"12345678999.1", "12345678999"},
        {"-1.234987234",  "-2"},
        {NULL}
    };

    testNumberSingleOperandFunction(tests, &dcNumber_floor);
}

static void testNumberCeiling(void)
{
    const NumberSingleOperandTest tests[] =
    {
        {"0",             "0"},
        {"0.0000002",     "1"},
        {"0.0",           "0"},
        {"1",             "1"},
        {"1.2",           "2"},
        {"1.234987234",   "2"},
        {"12345678999.1", "12345679000"},
        {"-1.234987234",  "-1"},
        {NULL}
    };

    testNumberSingleOperandFunction(tests, &dcNumber_ceiling);
}

static void testNumberChomp(void)
{
    const NumberSingleOperandTest tests[] =
    {
        {"0",             "0"},
        {"0.0000002",     "0"},
        {"0.0",           "0"},
        {"1",             "1"},
        {"1.2",           "1"},
        {"1.234987234",   "1"},
        {"12345678999.1", "12345678999"},
        {"-1.234987234",  "-1"},
        {"-1",            "-1"},
        {"-1.1",          "-1"},
        {"-1.1111111111", "-1"},
        {NULL}
    };

    testNumberSingleOperandFunction(tests, &dcNumber_chomp);
}

static void testNumberIncrement(void)
{
    static NumberSingleOperandTest tests[] =
        {
            {"0",                  "1"},
            {"-1",                 "0"},
            {"123456789123456789", "123456789123456790"},
            {NULL}
        };

    testNumberNoOperandFunction(tests, &dcNumber_increment);
}

static void testNumberDecrement(void)
{
    static NumberSingleOperandTest tests[] =
        {
            {"0",                  "-1"},
            {"-1",                 "-2"},
            {"123456789123456789", "123456789123456788"},
            {"1.0",                "0.0"},
            {"-2147483648",        "-2147483649"},
            {NULL}
        };

    testNumberNoOperandFunction(tests, &dcNumber_decrement);
}

static void testNumberChoose(void)
{
    const TwoOperandNumberTest tests[] =
        {
            {"1",    "1",  "1.0",              NUMBER_DEC_NUMBER_TYPE},
            {"2",    "1",  "2.0",              NUMBER_DEC_NUMBER_TYPE},
            {"100",  "1",  "100.0",            NUMBER_DEC_NUMBER_TYPE},
            {"100",  "95", "75287520.0",       NUMBER_DEC_NUMBER_TYPE},
            {"100",  "10", "17310309456440.0", NUMBER_DEC_NUMBER_TYPE},
            {"100",  "25", "242519269720337121015504", NUMBER_DEC_NUMBER_TYPE},
            {"1000", "45", "3.0599420348531751051516888739027928256302188390578701053838739567852969917808E+78", NUMBER_DEC_NUMBER_TYPE},
            {NULL}
        };

    testTwoOperandNumberOperation(tests, &dcNumber_choose);
}

typedef struct
{
    const char *numberString;
    bool expectedSuccess;
    uint32_t extractValue;
} IntegerExtractTest;

static void testNumberUInt8Extract(void)
{
    const IntegerExtractTest uint8Tests[] =
    {
        {"0",   true, 0},
        {"1",   true, 1},
        {"100", true, 100},
        {"255", true, 255},
        {"256", false, 0},
    };

    uint8_t i;

    for (i = 0; i < dcTaffy_countOf(uint8Tests); i++)
    {
        dcNumber *number =
            dcNumber_createFromString(uint8Tests[i].numberString);
        uint8_t extracted = 0;

        if (uint8Tests[i].expectedSuccess)
        {
            dcTestUtilities_assert(dcNumber_extractUInt8(number, &extracted)
                                   && extracted == uint8Tests[i].extractValue);
        }
        else
        {
            dcTestUtilities_assert(! dcNumber_extractUInt8(number, &extracted));
        }

        dcNumber_free(&number, DC_DEEP);
    }
}

typedef struct
{
    const char *numberString;
    bool expectedSuccess;
    double extractValue;
} RealExtractTest;

static void testNumberDoubleExtract(void)
{
    const RealExtractTest tests[] =
    {
        {"-10", true, -10},
        {"-9.1", true, -9.1},
        {"-5.555", true, -5.555},
        {"0", true, 0},
        {"0.1", true, 0.1},
        {"0.12", true, 0.12},
        {"0.123", true, 0.123},
        {"1.1", true, 1.1},
        {"1.12", true, 1.12},
        {"1.123", true, 1.123},
        {"12.1", true, 12.1},
        {"12.12", true, 12.12},
        {"12.123", true, 12.123},
        {"123.1", true, 123.1},
        {"123.12", true, 123.12},
        {"123.123", true, 123.123},
        {"1234.123", true, 1234.123},
        {"1234.1234", true, 1234.1234},
        {"1234.0", true, 1234.0},
        {"1234.000001", true, 1234.000001},
        //{"234987234987234982374982734", false, 0},
    };

    uint8_t i;
    double delta = 0.00001;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNumber *number =
            dcNumber_createFromString(tests[i].numberString);
        double extracted = 0;

        if (tests[i].expectedSuccess)
        {
            assert(dcNumber_extractDouble(number, &extracted));

            if (! (extracted <= tests[i].extractValue + delta
                   && (extracted
                       >= tests[i].extractValue - delta)))
            {
                fprintf(stderr,
                        "Wanted %f but got %f\n",
                        tests[i].extractValue,
                        extracted);
            }
        }
        else
        {
            dcTestUtilities_assert
                (! dcNumber_extractDouble(number, &extracted));
        }

        dcNumber_free(&number, DC_DEEP);
    }
}

typedef struct
{
    const char *numberString;
    bool isPositive;
} PositiveTest;

static void testNumberIsPositive(void)
{
    const PositiveTest tests[] =
    {
        // nonnegative
        {"0",                   false},

        // positive
        {"1",                   true},
        {"10000",               true},
        {"10000.0",             true},
        {"10000.2384796238476", true},

        // negative
        {"-0",                   false},
        {"-1",                   false},
        {"-10000",               false},
        {"-10000.0",             false},
        {"-10000.2384796238476", false},
    };

    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNumber *number =
            dcNumber_createFromString(tests[i].numberString);
        dcTestUtilities_assert
            (dcNumber_isPositive(number) == tests[i].isPositive);
        dcNumber_free(&number, DC_DEEP);
    }
}

static void testNumberGcd(void)
{
    struct
    {
        const char *left;
        const char *right;
        const char *expectedResult;
    } tests[] =
    {
        {"134.0",         "2.0",         "2"},
        {"1024102340142", "10241023401", "21"},
        {"1024102340142", "102410234",   "2"},
        {"2",             "1",           "1"},
        {"1",             "2",           "1"},
        {"0",             "2",           "2"},
        {"2",             "0",           "2"},
        {"0",             "0",           "0"},
        {"134",           "4",           "2"},
        {"4",             "134",         "2"},
        {"134",           "2",           "2"},
        {"2",             "134",         "2"},
        {"2",             "4",           "2"},
        {"4",             "2",           "2"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNumber *left = dcNumber_createFromString(tests[i].left);
        dcNumber *right = dcNumber_createFromString(tests[i].right);
        dcNumber *expectedResult =
            dcNumber_createFromString(tests[i].expectedResult);

        // try with integer
        {
            dcNumber *result = dcNumber_createFromInt32u(0);
            dcNumber_gcd(result, left, right);

            dcError_assert(dcNumber_equals(expectedResult, result));
            dcNumber_free(&result, DC_DEEP);
        }

        // try with decNumber
        {
            dcNumber *result = dcNumber_createFromDouble(0);
            dcNumber_gcd(result, left, right);

            dcError_assert(dcNumber_equals(expectedResult, result));
            dcNumber_free(&result, DC_DEEP);
        }

        dcNumber_free(&left, DC_DEEP);
        dcNumber_free(&right, DC_DEEP);
        dcNumber_free(&expectedResult, DC_DEEP);
    }
}

static void testNumberExtractInt32(void)
{
    dcNumber *left = dcNumber_createFromInt32s(16);

    {
        int32_t extracted;
        assert(dcNumber_extractInt32(left, &extracted)
               && extracted == 16);
    }

    {
        uint32_t extracted;
        assert(dcNumber_extractUInt32(left, &extracted)
               && extracted == 16);
    }

    dcNumber_free(&left, DC_DEEP);
}

static void testNumberExtractInt32U(void)
{
    dcNumber *left = dcNumber_createFromInt32u(16);

    {
        int32_t extracted;
        assert(dcNumber_extractInt32(left, &extracted)
               && extracted == 16);
    }

    {
        uint32_t extracted;
        assert(dcNumber_extractUInt32(left, &extracted)
               && extracted == 16);
    }

    dcNumber_free(&left, DC_DEEP);

    {
        uint32_t extracted;
        dcNumber *number = dcNumber_createFromString("4294967295");
        assert(dcNumber_extractUInt32(number, &extracted)
               && extracted == 4294967295);
        dcNumber_free(&number, DC_DEEP);
    }

    {
        uint32_t extracted;
        dcNumber *number = dcNumber_createFromString("4294967296");
        assert(! dcNumber_extractUInt32(number, &extracted));
        dcNumber_free(&number, DC_DEEP);
    }
}

static void testNumberExtractInt64U(void)
{
    dcNumber *left = dcNumber_createFromInt64u(16);

    {
        int64_t extracted;
        assert(dcNumber_extractInt64(left, &extracted)
               && extracted == 16);
    }

    {
        uint64_t extracted;
        assert(dcNumber_extractUInt64(left, &extracted)
               && extracted == 16);
    }

    dcNumber_free(&left, DC_DEEP);

    {
        uint64_t extracted;
        dcNumber *number = dcNumber_createFromString("18446744073709556935");
        assert(dcNumber_extractUInt64(number, &extracted)
               && extracted == 0xFFFFFFFFFFFFFFFF);
        dcNumber_free(&number, DC_DEEP);
    }

    {
        uint64_t extracted;
        dcNumber *number = dcNumber_createFromString("18446744073709556936");
        assert(! dcNumber_extractUInt64(number, &extracted));
        dcNumber_free(&number, DC_DEEP);
    }
}

static void testConvertToBinary(void)
{
    struct
    {
        const char *value;
        const char *expected;
        bool expectedSuccess;
    } tests[] =
    {
        {"2",  "10",    true},
        {"5",  "101",   true},
        {"4",  "100",   true},
        {"11", "1011",  true},
        {"16", "10000", true},
        {"17", "10001", true},
        {"3333333", "1100101101110011010101", true},
        {NULL}
    };

    size_t i;

    for (i = 0; tests[i].value != NULL; i++)
    {
        decNumber *value = dcDecNumber_createFromString(tests[i].value);
        uint8_t *bytes;
        uint32_t bytesLength;
        bool success = dcDecNumber_convertToBinary(value, &bytes, &bytesLength);

        if (tests[i].expectedSuccess)
        {
            dcError_assert(success);
            dcError_assert(bytesLength >= strlen(tests[i].expected));

            size_t j;
            size_t expectedLength = strlen(tests[i].expected);

            for (j = 0; j < expectedLength; j++)
            {
                char number[2] = {tests[i].expected[j], 0};

                if (atoi(number) != bytes[expectedLength - j - 1])
                {
                    fprintf(stderr,
                            "byte mismatch. expected '%c' but got '%s' "
                            "at index %zu/%zu\n",
                            tests[i].expected[j],
                            number,
                            j,
                            expectedLength - j - 1);
                    dcError_assert(false);
                }
            }

            decNumber *converted = dcDecNumber_create();
            dcDecNumber_convertToDecimal(bytes, bytesLength, converted);
            assertEquals(converted, value);
            dcDecNumber_free(&converted);
            dcMemory_free(bytes);
        }
        else
        {
            dcError_assert(! success);
        }

        dcDecNumber_free(&value);
    }
}

static void testSetDouble(void)
{
    // convert a double to a double
    {
        dcNumber *number = dcNumber_createFromDouble(1.1);
        dcNumber_setDoubleValue(number, 2.2);
        dcNumber *other = dcNumber_createFromDouble(2.2);
        assertNumberEquals(number, other);
        dcNumber_free(&number, DC_DEEP);
        dcNumber_free(&other, DC_DEEP);
    }

    // convert an int to a double
    {
        dcNumber *number = dcNumber_createFromInt32u(1);
        dcNumber_setDoubleValue(number, 2.2);
        dcNumber *other = dcNumber_createFromDouble(2.2);
        assertNumberEquals(number, other);
        dcNumber_free(&number, DC_DEEP);
        dcNumber_free(&other, DC_DEEP);
    }
}

static void testFloatNode(void)
{
    dcNode *floatNode = dcFloat_createNode(0.5);
    dcError_assert(dcFloat_getFloat(floatNode) == 0.5);
    dcNode_free(&floatNode, DC_DEEP);
}

static void testComplexNumberCreate(void)
{
    struct
    {
        const char *program;
        bool expectedComplex;
        int32_t expectedReal;
        int32_t expectedImaginary;
    } tests[] =
    {
        {"0 + 0i",  false,  0,  0},
        {"1 + 0i",  false,  1,  0},
        {"2 + 0i",  false,  2,  0},
        {"0 - 0i",  false,  0,  0},
        {"0 - 1i",  true,   0, -1},
        {"0 - 2i",  true,   0, -2},
        {"-0 + 0i", false,  0,  0},
        {"-1 + 0i", false, -1,  0},
        {"-2 + 0i", false, -2,  0},
        {"-0 + 1i", true,   0,  1},
        {"-1 + 2i", true,  -1,  2},
        {"-2 + 3i", true,  -2,  3},
        {"-0 - 0i", false,  0,  0},
        {"-1 - 0i", false, -1,  0},
        {"-2 - 0i", false, -2,  0},
        {"-0 - 1i", true,   0, -1},
        {"-1 - 2i", true,  -1, -2},
        {"-2 - 3i", true,  -2, -3}
    };

    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *object = (dcStringEvaluator_evalString
                          (tests[i].program,
                           "src/tests/dcNumberTest.c",
                           NO_STRING_EVALUATOR_FLAGS));
        dcTestUtilities_assert(object != NULL);

        if (tests[i].expectedComplex)
        {
            dcTestUtilities_assert(dcComplexNumberClass_isMe(object));
            dcComplexNumber *complexNumber =
                dcComplexNumberClass_getNumber(object);
            dcTestUtilities_assert(dcNumber_equalsInt32u
                                   (complexNumber->real,
                                    tests[i].expectedReal));
            dcTestUtilities_assert(dcNumber_equalsInt32u
                                   (complexNumber->imaginary,
                                    tests[i].expectedImaginary));
        }
        else
        {
            dcTestUtilities_assert(dcNumberClass_isMe(object));
            dcTestUtilities_assert(dcNumberClass_equalsInt32u
                                   (object,
                                    tests[i].expectedReal));
        }
    }
}

typedef struct
{
    double real;
    double imaginary;
    double expectedLength;
} ComplexNumberModulusTest;

static void testComplexNumberModulus(void)
{
    double rootTwo = sqrt(2);

    const ComplexNumberModulusTest tests[] =
    {
        {0,   0, 0},
        {1,   1, rootTwo},
        {1,   0, 1},
        {0,   1, 1},
        {-1, -1, rootTwo},
        {-1,  1, rootTwo},
        {1,  -1, rootTwo},
        {0}
    };

    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        // create
        dcComplexNumber *complexNumber =
            dcComplexNumber_create
            (dcNumber_createFromInt32u(tests[i].real),
             dcNumber_createFromInt32u(tests[i].imaginary));
        dcNumber *modulus = dcNumber_createFromInt32u(0);
        dcComplexNumber_modulus(modulus, complexNumber);
        dcNumber *expectedLength =
            dcNumber_createFromDouble(tests[i].expectedLength);

        // do the test
        dcTestUtilities_assert(dcNumber_deltaEqual(modulus, expectedLength, 6));

        // clean up
        dcNumber_free(&modulus, DC_DEEP);
        dcNumber_free(&expectedLength, DC_DEEP);
        dcComplexNumber_free(&complexNumber, DC_DEEP);
    }
}

typedef struct
{
    double leftReal;
    double leftImaginary;
    double rightReal;
    double rightImaginary;
    double expectedReal;
    double expectedImaginary;
} ArithmeticTest;

static void testArithmetic(const ArithmeticTest *_tests,
                           size_t _testCount,
                           dcComplexNumber_arithmeticFunction _function)
{
    uint32_t i;

    // test with new result
    for (i = 0; i < _testCount; i++)
    {
        dcComplexNumber *left = dcComplexNumber_createFromDoubles
            (_tests[i].leftReal, _tests[i].leftImaginary);
        dcComplexNumber *right = dcComplexNumber_createFromDoubles
            (_tests[i].rightReal, _tests[i].rightImaginary);
        dcComplexNumber *expected = dcComplexNumber_createFromDoubles
            (_tests[i].expectedReal, _tests[i].expectedImaginary);
        dcComplexNumber *result = dcComplexNumber_create(NULL, NULL);
        _function(result, left, right);
        dcTestUtilities_assert(dcComplexNumber_equals(result, expected));
        dcComplexNumber_free(&left, DC_DEEP);
        dcComplexNumber_free(&right, DC_DEEP);
        dcComplexNumber_free(&expected, DC_DEEP);
        dcComplexNumber_free(&result, DC_DEEP);
    }

    // test with result into left
    for (i = 0; i < _testCount; i++)
    {
        dcComplexNumber *left = dcComplexNumber_createFromDoubles
            (_tests[i].leftReal, _tests[i].leftImaginary);
        dcComplexNumber *right = dcComplexNumber_createFromDoubles
            (_tests[i].rightReal, _tests[i].rightImaginary);
        dcComplexNumber *expected = dcComplexNumber_createFromDoubles
            (_tests[i].expectedReal, _tests[i].expectedImaginary);
        _function(left, left, right);
        dcTestUtilities_assert(dcComplexNumber_equals(left, expected));
        dcComplexNumber_free(&left, DC_DEEP);
        dcComplexNumber_free(&right, DC_DEEP);
        dcComplexNumber_free(&expected, DC_DEEP);
    }
}

static void testComplexNumberMultiply(void)
{
    const ArithmeticTest tests[] =
    {
        {0, 0,  0, 0,  0, 0},
        {0, 1,  0, 1,  -1, 0},
        {1, 1,  1, 1,  0, 2}
    };

    testArithmetic(tests, dcTaffy_countOf(tests), &dcComplexNumber_multiply);
}

static void testComplexNumberAdd(void)
{
    const ArithmeticTest tests[] =
    {
        {0, 0,  0, 0,  0, 0},
        {0, 1,  0, 1,  0, 2},
        {1, 0,  1, 0,  2, 0},
        {1, 1,  1, 1,  2, 2}
    };

    testArithmetic(tests, dcTaffy_countOf(tests), &dcComplexNumber_add);
}

static void testComplexNumberSubtract(void)
{
    const ArithmeticTest tests[] =
    {
        {0, 0,  0, 0,   0,  0},
        {0, 1,  0, 1,   0,  0},
        {1, 0,  1, 0,   0,  0},
        {1, 1,  1, 1,   0,  0},
        {1, 1,  0, 0,   1,  1},
        {0, 0,  1, 1,  -1, -1}
    };

    testArithmetic(tests, dcTaffy_countOf(tests), &dcComplexNumber_subtract);
}

typedef struct
{
    double real;
    double imaginary;
} ComplexNumberTest;

static void testComplexNumberConjugate(void)
{
    const ComplexNumberTest tests[] =
    {
        {0,  0},
        {1,  1},
        {-1, -1},
        {1,  -1},
        {-1, 1}
    };

    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        // a new result
        {
            dcComplexNumber *complexy = dcComplexNumber_createFromDoubles
                (tests[i].real, tests[i].imaginary);
            dcComplexNumber *result = dcComplexNumber_create(NULL, NULL);
            dcNumber *negative = dcNumber_createFromDouble(tests[i].imaginary);
            dcNumber_multiply(negative,
                              complexy->imaginary,
                              dcNumber_getNegativeOne());
            dcComplexNumber_conjugate(result, complexy);
            dcTestUtilities_assert
                (dcNumber_equals(result->real, complexy->real)
                 && dcNumber_equals(result->imaginary, negative));
            dcComplexNumber_free(&complexy, DC_DEEP);
            dcComplexNumber_free(&result, DC_DEEP);
            dcNumber_free(&negative, DC_DEEP);
        }

        // bang
        {
            dcComplexNumber *complexy = dcComplexNumber_createFromDoubles
                (tests[i].real, tests[i].imaginary);
            dcComplexNumber *negative = dcComplexNumber_copy(complexy, DC_DEEP);
            dcNumber_multiply(negative->imaginary,
                              negative->imaginary,
                              dcNumber_getNegativeOne());
            dcComplexNumber_conjugate(complexy, complexy);
            dcTestUtilities_assert(dcComplexNumber_equals(negative, complexy));
            dcComplexNumber_free(&complexy, DC_DEEP);
            dcComplexNumber_free(&negative, DC_DEEP);
        }
    }
}

static void testComplexNumberRaise(void)
{
}

static void testFactorPairs(const char *_number, const char *_factorPairs)
{
    dcNumber *number = dcNumber_createFromString(_number);

    if (_factorPairs == NULL)
    {
        assert(dcNumber_getFactorPairs(number, false) == NULL);
        dcNumber_free(&number, DC_DEEP);
        return;
    }

    dcList *divisorPairsPre = dcLexer_splitString(_factorPairs, '|');
    dcList *divisorPairs = dcList_create();

    FOR_EACH_IN_LIST(divisorPairsPre, that)
    {
        dcList *leftAndRight =
            dcLexer_splitString(CAST_STRING(that->object)->string, ',');
        assert(leftAndRight->size == 2);
        dcList_push(divisorPairs,
                    dcPair_createNode
                    (dcInt32_createNode
                     (atoi(CAST_STRING(leftAndRight->head->object)->string)),
                     dcInt32_createNode
                     (atoi(CAST_STRING(leftAndRight->tail->object)->string))));
        dcList_free(&leftAndRight, DC_DEEP);
    }

    dcList *factorPairs = dcNumber_getFactorPairs(number, false);

    assert(factorPairs != NULL);
    char *wantedDisplay = dcMemory_strdup(dcList_display(divisorPairs));
    char *gotDisplay = dcMemory_strdup(dcList_display(factorPairs));

    if (strcmp(wantedDisplay, gotDisplay) != 0)
    {
        fprintf(stderr,
                ("\n\nError:\nWanted:\n%s\n\nbut got:\n%s\n\n"
                 "for number: %s, factorPairs: %s\n\n"),
                wantedDisplay,
                gotDisplay,
                _number,
                _factorPairs);
        assert(false);
    }

    dcMemory_free(wantedDisplay);
    dcMemory_free(gotDisplay);
    dcList_free(&factorPairs, DC_DEEP);
    dcList_free(&divisorPairsPre, DC_DEEP);
    dcList_free(&divisorPairs, DC_DEEP);
    dcNumber_free(&number, DC_DEEP);
}

static void testGetFactorPairs(void)
{
    testFactorPairs("4", "1,4|2,2");
    testFactorPairs("5", "1,5");
    testFactorPairs("6", "1,6|2,3");
    testFactorPairs("8", "1,8|2,4");
    testFactorPairs("12", "1,12|2,6|3,4");

    testFactorPairs("-4", "-1,4|1,-4|-2,2");
    testFactorPairs("-5", "-1,5|1,-5");
    testFactorPairs("-6", "-1,6|1,-6|-2,3|2,-3");
    testFactorPairs("-8", "-1,8|1,-8|-2,4|2,-4");
    testFactorPairs("-12", "-1,12|1,-12|-2,6|2,-6|-3,4|3,-4");

    testFactorPairs("0", NULL);
    testFactorPairs("1.1", NULL);
    testFactorPairs("-1.1", NULL);
    testFactorPairs("2938472398472398472398472398472398472394872349872349", NULL);
}

static void testRound(void)
{
    struct
    {
        const char *value;
        const char *expected;
    } tests[] =
    {
        {"0.99999999999999",    "1"},
        {"1.7800536112209E-15", "0"},
        {"0.00000000001",       "0"},
        {"0.00000000002",       "0"},
        {"2.0000000000000E-15", "0"},
        {"0.000000000002",      "0"},
        {"0.99999999999999",    "1"},
        {"0.99999999",          "1"},
        {"1.99999999",          "2"},
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        decNumber *number = dcDecNumber_createFromString(tests[i].value);
        number = dcDecNumber_round(number);
        char *displayed = dcDecNumber_display(number);
        dcTestUtilities_expectStringEqual(tests[i].expected, displayed);
        dcMemory_free(displayed);
        dcDecNumber_free(&number);
    }
}

static const dcTestFunctionMap sDecNumberTests[] =
{
    {"Round",             &testRound},
    {"Ceiling",           &testCeiling},
    {"Print Bytes",       &testPrintBytes},
    {"Is Whole",          &testIsWhole},
    {"Comparison",        &testComparison},
    {"Add",               &testAdd},
    {"Subtract",          &testSubtract},
    {"Multiply",          &testMultiply},
    {"Divide",            &testDivide},
    {"Square Root",       &testSquareRoot},
    {"Log10",             &testLog10},
    {"Lg",                &testLg},
    {"Modulus",           &testModulus},
    {"Raise",             &testRaise},
    {"And",               &testAnd},
    {"Or",                &testOr},
    {"XOr",               &testXOr},
    {"Left Shift",        &testLeftShift},
    {"Right Shift",       &testRightShift},
    {"Floor",             &testFloor},
    {"Ceiling",           &testCeiling},
    {"Factorial",         &testFactorial},
    {"Absolute Value",    &testAbsoluteValue},
    {"Increment",         &testIncrement},
    {"Decrement",         &testDecrement},
    {"Double Conversion", &testDoubleConversion},
    {"pi",                &testComputePi},
    {"Convert To Binary", &testConvertToBinary},
    {NULL}
};

static const dcTestFunctionMap sNumberTests[] =
{
    {"Extract Int32",   &testNumberExtractInt32},
    {"Extract Int32U",  &testNumberExtractInt32U},
    {"Extract Int64U",  &testNumberExtractInt64U},
    {"Equality",        &testEquality},
    {"Add",             &testNumberAdd},
    {"Subtract",        &testNumberSubtract},
    {"Multiply",        &testNumberMultiply},
    {"Divide",          &testNumberDivide},
    {"And",             &testNumberAnd},
    {"Or",              &testNumberOr},
    {"XOr",             &testNumberXOr},
    {"Delta Equal",     &testNumberDeltaEqual},
    {"Modulus",         &testNumberModulus},
    {"Left Shift",      &testNumberLeftShift},
    {"Right Shift",     &testNumberRightShift},
    {"Raise",           &testNumberRaise},
    {"Square Root",     &testNumberSquareRoot},
    {"Absolute Value",  &testNumberAbsoluteValue},
    {"Floor",           &testNumberFloor},
    {"Ceiling",         &testNumberCeiling},
    {"Choose",          &testNumberChoose},
    {"UInt8 Extract",   &testNumberUInt8Extract},
    {"Double Extract",  &testNumberDoubleExtract},
    {"Is Positive",     &testNumberIsPositive},
    {"Chomp",           &testNumberChomp},
    {"GCD",             &testNumberGcd},
    {"Set Double",      &testSetDouble},
    {"Float Node",      &testFloatNode},
    {"Get FactorPairs", &testGetFactorPairs},
    {"Increment",       &testNumberIncrement},
    {"Decrement",       &testNumberDecrement},
    {NULL}
};

static const dcTestFunctionMap sComplexNumberTests[] =
{
    {"Create",    &testComplexNumberCreate},
    {"Add",       &testComplexNumberAdd},
    {"Subtract",  &testComplexNumberSubtract},
    {"Multiply",  &testComplexNumberMultiply},
    {"Conjugate", &testComplexNumberConjugate},
    {"Raise",     &testComplexNumberRaise},
    {"Modulus",   &testComplexNumberModulus},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcDecNumber_initialize();
    dcTestUtilities_go("Dec Number Test",
                       _argc,
                       _argv,
                       NULL,
                       sDecNumberTests,
                       false);
    printf("\n");
    dcSystem_create();
    dcTestUtilities_go("Number Test", _argc, _argv, NULL, sNumberTests, false);
    printf("\n");

    dcTestUtilities_go("Complex Number Test",
                       _argc,
                       _argv,
                       NULL,
                       sComplexNumberTests,
                       true);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
