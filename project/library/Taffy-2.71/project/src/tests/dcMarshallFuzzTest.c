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

#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcComplexNumber.h"
#include "dcComplexNumberClass.h"
#include "dcContainers.h"
#include "dcError.h"
#include "dcFutureClass.h"
#include "dcGarbageCollector.h"
#include "dcGraphDatas.h"
#include "dcHashClass.h"
#include "dcListClass.h"
#include "dcMarshaller.h"
#include "dcMatrix.h"
#include "dcMatrixClass.h"
#include "dcMemory.h"
#include "dcNilClass.h"
#include "dcNumber.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcPairClass.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcStringClass.h"
#include "dcSymbolClass.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"
#include "dcUnsignedInt32.h"
#include "dcYesClass.h"

static void testItWithSetValues(uint32_t _length,
                                const uint8_t *_values,
                                uint32_t _valuesLength)
{
    dcString *string = dcString_createWithLength(_length);
    uint32_t i = 0;

    for (i = 0; i < 20000; i++)
    {
        uint32_t j;

        for (j = 0; j < string->length; j++)
        {
            if (j < _valuesLength)
            {
                string->string[j] = _values[j];
            }
            else
            {
                string->string[j] = rand();
            }
        }

        dcString_resetIndex(string);
        dcNode *node = dcNode_unmarshall(string);

        if (node != NULL)
        {
            dcNode_tryFree(&node, DC_DEEP);
        }
    }

    dcString_free(&string, DC_DEEP);
}

static void testIt(uint32_t _length)
{
    testItWithSetValues(_length, NULL, 0);
}

static void test1(void)
{
    testIt(1);
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

static void testNodeTypes(void)
{
    dcNodeType type;

    for (type = 0; type < NODE_LAST; type++)
    {
        uint8_t value[1] = {type};
        testItWithSetValues(10, value, 1);
    }
}

static void testGraphDataTypes(void)
{
    dcNodeType type;

    for (type = 0; type < NODE_GRAPH_DATA_LAST; type++)
    {
        uint8_t values[2] = {NODE_GRAPH_DATA, type};
        testItWithSetValues(10, values, 2);
    }
}

static void testClasses(void)
{
    uint8_t templateType = 0;

    for (templateType = 0; templateType < 50; templateType++)
    {
        uint8_t values[3] = {NODE_GRAPH_DATA, NODE_CLASS, templateType};
        testItWithSetValues(10, values, 3);
    }
}

static const dcTestFunctionMap sTests[] =
{
    {"Test 1",           test1},
    {"Test 5",           test5},
    {"Test 10",          test10},
    {"Test 50",          test50},
    {"Test 100",         test100},
    {"Node Types",       testNodeTypes},
    {"Graph Data Types", testGraphDataTypes},
    {"Classes",          testClasses},
    {NULL,               NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_create();
    srand(0);
    dcTestUtilities_go("Marshall Fuzz Test", _argc, _argv, NULL, sTests, false);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
