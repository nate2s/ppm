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
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcFunctionUpdate.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcIdentifier.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcParser.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"

//
// create a tree of _size iterators
//
static dcNode *createTree(size_t _size)
{
    size_t i;
    dcNode *head = NULL;
    dcNode *iterator = NULL;

    for (i = 0; i < _size; i++)
    {
        char *name = dcLexer_sprintf("identifier %u", i);
        dcNode *identifier = dcIdentifier_createNode(name, NO_FLAGS);
        dcMemory_free(name);

        if (iterator != NULL)
        {
            dcGraphData_setNext(iterator, identifier);
        }

        iterator = identifier;

        if (head == NULL)
        {
            head = iterator;
        }
    }

    return head;
}

static void testIdentifierTree(void)
{
    size_t size = 10;
    dcNode *head = createTree(size);
    dcNode *tree = dcGraphDataTree_createNode(head);
    dcTestUtilities_assert(dcGraphDataTree_getSize(tree) == size);
    dcNode_free(&tree, DC_DEEP);
}

static void testIdentifierTreeCopy(void)
{
    size_t size = 10;
    dcNode *head = createTree(size);
    dcNode *tree = dcGraphDataTree_createNode(dcGraphData_copyTree(head));
    dcTestUtilities_assert(dcGraphDataTree_getSize(tree) == size);
    dcNode_free(&head, DC_DEEP);
    dcNode_free(&tree, DC_DEEP);
}

static void testIdentifierTreeRegister(void)
{
    dcNode_register(createTree(20));
}

static const dcTestFunctionMap sMap[] =
{
    {"Identifier Tree",          &testIdentifierTree},
    {"Identifier Tree Copy",     &testIdentifierTreeCopy},
    {"Identifier Tree Register", &testIdentifierTreeRegister},
    {NULL}
};

static const dcTestFunctionMap sRuntimeMap[] =
{
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcLexer_initialize();
    dcStringManager_create();
    dcTestUtilities_go("Graph Data Test", _argc, _argv, NULL, sMap, false);
    dcLexer_cleanup();
    dcStringManager_free();
    dcGarbageCollector_free();

    if (dcTestUtilities_runSystem())
    {
        dcSystem_create();
        dcTestUtilities_go("Package Runtime Test",
                           _argc,
                           _argv,
                           NULL,
                           sRuntimeMap,
                           true);
        dcSystem_free();
    }

    dcMemory_deinitialize();
    return 0;
}
