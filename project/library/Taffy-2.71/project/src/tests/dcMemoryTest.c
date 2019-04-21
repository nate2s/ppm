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

#include "dcAnd.h"
#include "dcFalse.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"
#include "dcThread.h"
#include "dcThreadInclude.h"
#include "dcTrue.h"

#include <string.h>

extern dcTaffyThreadId parserSelf;

//
// Some tests require a memory validation tool like Valgrind to be of any use
//
static void testAllocateAndFree(void)
{
    void *data = dcMemory_allocate(10);
    dcMemory_free(data);

    data = dcMemory_allocateAndInitialize(10);
    uint8_t bytes[10] = {0};
    dcTestUtilities_assert(memcmp(data, bytes, sizeof(bytes)) == 0);
    dcMemory_free(data);
}

static void testDuplicate(void)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t *duplicate = (uint8_t *)dcMemory_duplicate(data, sizeof(data));
    dcTestUtilities_assert(memcmp(data, duplicate, sizeof(data)) == 0);
    dcMemory_free(duplicate);
}

static void testRealloc(void)
{
    uint8_t *data = (uint8_t *)dcMemory_allocate(10);
    data = (uint8_t *)dcMemory_realloc(data, 20);
    dcMemory_free(data);
}

static void testStrdup(void)
{
    const char *test = "This is a test";
    char *duplicate = dcMemory_strdup(test);
    dcTestUtilities_assert(strcmp(test, duplicate) == 0);
    dcMemory_free(duplicate);
}

dcTaffyThreadId parserSelf;

static void testMemoryRegions(void)
{
    dcMemory_useMemoryRegions();
    // pretend we're the parser
    parserSelf = dcThread_getSelfId();

    dcAnd_createNode(dcTrue_createNode(), dcFalse_createNode());

    void *area = dcMemory_allocate(10);
    area = dcMemory_realloc(area, 20);

    const char test[] = "this is a test";
    char *newString1 = (char *)dcMemory_duplicate(test, sizeof(test));
    char *newString2 = dcMemory_strdup(test);
    dcMemory_free(newString1);
    dcMemory_free(newString2);

    dcMemory_useMalloc();
    dcMemory_freeMemoryRegions(DC_DEEP);
}

static const dcTestFunctionMap sTests[] =
{
    {"Allocate and Free", &testAllocateAndFree},
    {"Duplicate",         &testDuplicate},
    {"Realloc",           &testRealloc},
    {"Strdup",            &testStrdup},
    {"Memory Regions",    &testMemoryRegions},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcTestUtilities_go("Memory Test", _argc, _argv, NULL, sTests, false);
    dcGarbageCollector_free();
    dcMemory_deinitialize();
    dcTestUtilities_assert(dcMemory_getAllocatedMemorySize() == 0);
    return 0;
}
