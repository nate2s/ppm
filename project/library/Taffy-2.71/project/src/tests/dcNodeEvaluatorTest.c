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

#include "dcClass.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcFilePackageData.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHash.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcPackage.h"
#include "dcPackageContents.h"
#include "dcStringEvaluator.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"
#include "dcYesClass.h"

#define NODE_EVALUATOR_TEST_FILE_NAME "src/tests/dcNodeEvaluatorTest.c"

static dcNodeEvaluator *sNodeEvaluator = NULL;

// TODOOOOO
static const dcTestFunctionMap sTests[] =
{
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_createWithArguments
        (dcTaffyCommandLineArguments_parseAndCreateWithFailure
         (_argc, _argv, false));

    sNodeEvaluator = dcSystem_getCurrentNodeEvaluator();
    dcTestUtilities_go("Node Evaluator Test", _argc, _argv, NULL, sTests, true);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
