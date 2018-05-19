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

#include "dcGarbageCollector.h"
#include "dcNode.h"
#include "dcNumberClass.h"
#include "dcPair.h"
#include "dcTestUtilities.h"
#include "dcSystem.h"

int main(int _argc, char **_argv)
{
    dcGarbageCollector_create();
    dcTestUtilities_start("Pair Test", _argc, _argv);

    // test dcPair_createNode()
    //      dcPair_getLeft()
    //      dcPair_getRight()
    {
        dcNode *pair = dcPair_createNode(gTestNodes[1], gTestNodes[2]);
        dcTestUtilities_checkEqual(dcPair_getLeft(pair), gTestNodes[1]);
        dcTestUtilities_checkEqual(dcPair_getRight(pair), gTestNodes[2]);
        dcNode_free(&pair, DC_SHALLOW);
    }

    // test dcPair_create()
    {
        dcPair *pair = dcPair_create(gTestNodes[1], gTestNodes[2]);
        dcTestUtilities_checkEqual(pair->left, gTestNodes[1]);
        dcTestUtilities_checkEqual(pair->right, gTestNodes[2]);
        dcPair_free(&pair, DC_SHALLOW);
    }

    // test blank create
    //      dcPair_setLeft()
    //      dcPair_setRight()
    {
        dcPair *pair = dcPair_create(NULL, NULL);
        dcTestUtilities_checkEqual(pair->left, NULL);
        dcTestUtilities_checkEqual(pair->right, NULL);
        dcPair_setLeft(pair, gTestNodes[1]);
        dcPair_setRight(pair, gTestNodes[2]);
        dcTestUtilities_checkEqual(pair->left, gTestNodes[1]);
        dcTestUtilities_checkEqual(pair->right, gTestNodes[2]);
        dcPair_free(&pair, DC_SHALLOW);
    }

    // test dcPair_copy()
    {
        dcPair *pair = dcPair_create(gTestNodes[1], gTestNodes[2]);
        dcPair *shallowCopy = dcPair_copy(pair, DC_SHALLOW);
        dcTestUtilities_checkEqual(pair->left, shallowCopy->left);
        dcTestUtilities_checkEqual(pair->right, shallowCopy->right);
        dcPair *deepCopy = dcPair_copy(pair, DC_DEEP);

        // a deep copy will cause us to need dcNode_compare() instead
        // of a pointer comparison
        dcTaffyOperator compareResult;
        dcTestUtilities_assert(dcNode_compare(deepCopy->left,
                                              pair->left,
                                              &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_EQUALS);
        dcTestUtilities_assert(dcNode_compare(deepCopy->right,
                                              pair->right,
                                              &compareResult)
                               == TAFFY_SUCCESS
                               && compareResult == TAFFY_EQUALS);
        dcPair_free(&pair, DC_SHALLOW);
        dcPair_free(&shallowCopy, DC_SHALLOW);
        dcPair_free(&deepCopy, DC_DEEP);
    }

    // test dcPair_set()
    {
        dcPair *pair = dcPair_create(NULL, NULL);
        dcPair_set(pair, gTestNodes[1], gTestNodes[2]);
        dcTestUtilities_checkEqual(pair->left, gTestNodes[1]);
        dcTestUtilities_checkEqual(pair->right, gTestNodes[2]);
        dcPair_free(&pair, DC_SHALLOW);
    }

    // test dcPair_clear()
    {
        dcPair *pair = dcPair_create(gTestNodes[1], gTestNodes[2]);
        dcPair_clear(pair, DC_SHALLOW);
        dcTestUtilities_checkEqual(pair->left, NULL);
        dcTestUtilities_checkEqual(pair->right, NULL);
        dcPair_free(&pair, DC_SHALLOW);
    }

    // test dcPair_clearLeft()
    //      dcPair_clearRight()
    {
        dcPair *pair = dcPair_create(gTestNodes[1], gTestNodes[2]);
        dcPair_clearLeft(pair, DC_SHALLOW);
        dcTestUtilities_checkEqual(pair->left, NULL);
        dcTestUtilities_checkEqual(pair->right, gTestNodes[2]);
        dcPair_clearRight(pair, DC_SHALLOW);
        dcTestUtilities_checkEqual(pair->right, NULL);
        dcPair_free(&pair, DC_SHALLOW);
    }

    // test dcPair_mark()
    {
        dcPair *pair = dcPair_create(gTestNodes[1], gTestNodes[2]);
        dcTestUtilities_doMark(false);
        dcPair_mark(pair);
        dcTestUtilities_assert(dcNode_isMarked(gTestNodes[1]));
        dcTestUtilities_assert(dcNode_isMarked(gTestNodes[2]));
        dcPair_free(&pair, DC_SHALLOW);
    }

    dcTestUtilities_end();
    dcGarbageCollector_free();

    return 0;
}
