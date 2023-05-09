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

#include "CompiledHelpSystem.h"
#include "CompiledImportHelperHelpTopic.h"
#include "CompiledMainHelpTopic.h"
#include "CompiledGnuLesserTermsAndConditionsHelpTopic.h"
#include "CompiledGnuTermsAndConditionsHelpTopic.h"
#include "CompiledGnuWarrantyHelpTopic.h"

static dcPieLineEvaluator *sPieLineEvaluator = NULL;

static void testIt(uint32_t _length)
{
    dcString *string = dcString_createWithLength(_length);
    uint32_t i = 0;

    for (i = 0; i < 1000000; i++)
    {
        //fprintf(stderr, "** i: %u\n", i);
        uint32_t j;

        for (j = 0; j < string->length - 1; j++)
        {
            string->string[j] = rand();
        }

        // null terminate
        string->string[j] = 0;

        char *result = dcPieLineEvaluator_evaluateLine(sPieLineEvaluator,
                                                       string->string,
                                                       NULL,
                                                       0,
                                                       NULL);
        dcMemory_free(result);
    }

    dcString_free(&string, DC_DEEP);
}

static void test2(void)
{
    testIt(2);
}

static void test5(void)
{
    testIt(5);
}

static void test10(void)
{
    testIt(10);
}

static void test50(void)
{
    testIt(50);
}

static void test100(void)
{
    testIt(100);
}

static const dcTestFunctionMap sTests[] =
{
    {"Test 2",   test2},
    {"Test 5",   test5},
    {"Test 10",  test10},
    {"Test 50",  test50},
    {"Test 100", test100},
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
    dcMemory_initialize();
    dcSystem_create();
    sPieLineEvaluator = dcPieLineEvaluator_create();
    srand(0);
    loadHelpSystem();
    dcTestUtilities_go("Pie Fuzz Test", _argc, _argv, NULL, sTests, true);
    dcPieLineEvaluator_free(&sPieLineEvaluator);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
