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

#include "dcError.h"
#include "dcUnsignedInt32.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcSystem.h"
#include "dcTree.h"

#define I_MAX 26

int main(int _argc, char **_argv)
{
    dcSystem_create();

    size_t iMax = 0;

    if (_argc == 1)
    {
        iMax = I_MAX;
    }
    else
    {
        iMax = atoi(_argv[1]);
    }

    assert(iMax > 0);

    dcTree *tree = dcTree_create(AVL_TREE);
    dcNode *numbers[iMax];

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    for (i = 0; i < iMax; i++)
    {
        numbers[i] = dcUnsignedInt32_createNode(rand() % iMax);
        printf("Created: %d (idx: %zu)\n", CAST_INT(numbers[i]), i);
    }

    i = 0;
    j = 0;
    k = 0;

    printf("INSERT\n");

    // SHALLOW //
    for (i = 0; i < iMax; i++)
    {
        dcTree_insert(tree, numbers[i], NULL);
        assert(dcTree_getSize(tree) == ++j);
    }

    char *treeDisplay = dcTree_display(tree);
    printf("%s\n", treeDisplay);
    DC_FREE(treeDisplay);

    dcTree_verifyBranchSizes(tree);

    // FIND //
    for (i = 0; i < iMax; i++)
    {
        assert(dcTree_find(tree, numbers[i], true, NULL));
    }

    printf("DELETE (shallow)\n");

    for (i = 0; i < iMax; i++)
    {
        dcTree_delete(tree, numbers[i], true, NULL, DC_SHALLOW);

        for (k = 0; k < i; k++)
        {
            assert(!dcTree_find(tree, numbers[k], true, NULL));
        }

        for (k = i + 1; k < iMax; k++)
        {
            assert(dcTree_find(tree, numbers[k], true, NULL));
        }

        assert(dcTree_getSize(tree) == --j);
        dcTree_verifyBranchSizes(tree);
    }

    printf("INSERT\n");

    // DEEP //
    for (i = 0; i < iMax; i++)
    {
        dcTree_insert(tree, numbers[i], NULL);
        assert(dcTree_getSize(tree) == ++j);
    }

    printf("DELETE (deep)\n");

    for (i = 0; i < iMax; i++)
    {
        dcTree_delete(tree, numbers[i], true, NULL, DC_DEEP);
        assert(dcTree_getSize(tree) == --j);
        dcTree_verifyBranchSizes(tree);
    }

    dcTree_free(&tree, DC_DEEP);
    dcSystem_free();

    return 0;
}
