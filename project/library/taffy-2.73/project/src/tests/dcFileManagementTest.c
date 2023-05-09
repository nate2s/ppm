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

#include <stdlib.h>
#include <string.h>

#include "dcDefines.h"
#include "dcFileManagement.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcNode.h"
#include "dcMemory.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"

static dcList *sWantedFileNames = NULL;

static dcResult eachFile(const char *_fileName, void *_token)
{
    dcNode *fileName = dcString_createNodeWithString(_fileName, true);
    dcTestUtilities_assert(dcList_remove(sWantedFileNames, fileName, DC_DEEP)
                           == TAFFY_SUCCESS);
    dcNode_free(&fileName, DC_DEEP);
    return TAFFY_SUCCESS;
}

static void testIterateOverDirectory(void)
{
    const char *wantedSuffixes[] = {"", "ty", "txt"};
    size_t i = 0;
    sWantedFileNames = dcList_create();

    for (i = 0; i < dcTaffy_countOf(wantedSuffixes); i++)
    {
        char ids[3] = {'A', 'B', 'C'};
        size_t j = 0;

        for (j = 0; j < dcTaffy_countOf(ids); j++)
        {
            char *name = dcLexer_sprintf
                ("src/tests/TestDirectory/simpleFiles/file%c%s%s",
                 ids[j],
                 (strcmp(wantedSuffixes[i], "")
                  != 0
                  ? "."
                  : ""),
                 wantedSuffixes[i]);
            dcList_push(sWantedFileNames,
                        dcString_createNodeWithString(name, false));
        }

        dcTestUtilities_assert((dcFileManagement_iterateOverFilesInDirectory
                                ("src/tests/TestDirectory/simpleFiles",
                                 wantedSuffixes[i],
                                 &eachFile,
                                 NULL))
                               && sWantedFileNames->size == 0);
    }

    dcList_free(&sWantedFileNames, DC_DEEP);
}

static const dcTestFunctionMap sMap[] =
{
    {"Iterate Over Directory", &testIterateOverDirectory},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_create();
    dcTestUtilities_go("File Management Test", _argc, _argv, NULL, sMap, true);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
