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

#include "dcCommandLineArguments.h"
#include "dcGarbageCollector.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"
#include "dcTestUtilities.h"

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcTestUtilities_start("CommandLineArguemntsTest", _argc, _argv);
    dcCommandLineArguments *arguments = dcCommandLineArguments_create();

    //
    // define the arguments
    //
    const char * const positiveArguments[] =
        {"-a", "--aye",
         "-b", "--bay",
         "-c", "--cay",
         "-k", "--kay",
         "-s", "--stringList",
         "-z", "--ayeAye",
         "-q", "--ayea",
         NULL, "--nullio"};

    const char * const negativeArguments[] =
        {"-d", "--day",
         "-j", "--jayAllicious"};

    size_t i;

    //
    // populate the arguments
    //
    for (i = 0; i < dcTaffy_countOf(positiveArguments); i += 2)
    {
        dcCommandLineArguments_registerArguments(arguments,
                                                 positiveArguments[i],
                                                 positiveArguments[i + 1]);
    }

    for (i = 0; i < dcTaffy_countOf(negativeArguments); i += 2)
    {
        dcCommandLineArguments_registerArguments(arguments,
                                                 negativeArguments[i],
                                                 negativeArguments[i + 1]);
    }

    dcCommandLineArguments_setDefaultArgument(arguments, "--kay");

    //
    // construct the command line
    //
    const char *rawCommandLine =
        ("commandLineArgumentsTest 77 88 "
         "-a 1 2 3 "
         "--bay 4 5 6 7 "
         "-c 0 10 11 "
         "--stringList the dog jumped over the house "
         "--ayeAye fine fine "
         "--ayea doo doo "
         "--nullio nullbaby");

    dcList *spaces = dcLexer_splitString(rawCommandLine, ' ');
    dcContainerSizeType argumentsSize = spaces->size;
    char **commandLine =
        (char **)dcMemory_allocate(sizeof(char*) * argumentsSize);
    dcNode *head;

    for (i = 0, head = dcList_shift(spaces, DC_SHALLOW);
         i < argumentsSize;
         i++, head = dcList_shift(spaces, DC_SHALLOW))
    {
        commandLine[i] = dcString_getString(head);
        dcNode_free(&head, DC_SHALLOW);
    }

    dcList_free(&spaces, DC_DEEP);

    //
    // parse!
    //
    dcTestUtilities_assert(dcCommandLineArguments_parse
                           (arguments,
                            argumentsSize,
                            commandLine,
                            true));

    for (i = 0; i < argumentsSize; i++)
    {
        dcMemory_free(commandLine[i]);
    }

    dcMemory_free(commandLine);

    //
    // check hits
    //
    for (i = 0; i < dcTaffy_countOf(positiveArguments); i++)
    {
        if (positiveArguments[i] != NULL)
        {
            assert(dcCommandLineArguments_getHit
                   (arguments, positiveArguments[i]));
        }
    }

    for (i = 0; i < dcTaffy_countOf(negativeArguments); i++)
    {
        assert(! (dcCommandLineArguments_getHit
                  (arguments, negativeArguments[i])));
    }

    //
    // test dcCommandLineArguments_getValues()
    //
    dcList *aValues = dcCommandLineArguments_getValues(arguments, "-a");
    dcTestUtilities_checkListSize(aValues, 3);

    dcList *bValues = dcCommandLineArguments_getValues(arguments, "--bay");
    dcTestUtilities_checkListSize(bValues, 4);

    //
    // test dcCommandLineArguments_getInt()
    //
    dcTestUtilities_assert(dcCommandLineArguments_getInt(arguments, "-a", 44)
                           == 1);

    //
    // test dcCommandLineArguments_getIntList()
    //
    dcList *aIntList = dcCommandLineArguments_getIntList(arguments, "--aye");
    dcTestUtilities_checkListSize(aIntList, 3);
    dcTestUtilities_assert(CAST_INT(dcList_getHead(aIntList)) == 1);
    dcList_free(&aIntList, DC_DEEP);

    //
    // test dcCommandLineArguments_getStringList()
    //
    dcList *stringListValues = dcCommandLineArguments_getStringList
        (arguments, "--stringList");
    dcTestUtilities_checkListSize(stringListValues, 6);
    const char *strings[] = {"the", "dog", "jumped", "over", "the", "house"};
    dcTestUtilities_assert(dcTaffy_countOf(strings) == stringListValues->size);
    dcListElement *that = NULL;

    for (i = 0, that = stringListValues->head;
         that != NULL;
         i++, that = that->next)
    {
        dcTestUtilities_assert(strcmp
                               (strings[i], dcString_getString(that->object))
                               == 0);
    }

    dcList_free(&stringListValues, DC_DEEP);

    //
    // test default argument
    //
    dcList *kayValues = dcCommandLineArguments_getIntList(arguments, "--kay");
    dcTestUtilities_checkListSize(kayValues, 2);
    dcTestUtilities_assert(CAST_INT(dcList_getHead(kayValues)) == 77);
    dcTestUtilities_assert(CAST_INT(dcList_getTail(kayValues)) == 88);
    dcList_free(&kayValues, DC_DEEP);

    dcTestUtilities_end();
    dcGarbageCollector_free();
    dcCommandLineArguments_free(&arguments);
    dcMemory_deinitialize();
    return 0;
}
