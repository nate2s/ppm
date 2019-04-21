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

#include <time.h>

#include "CompiledTaffyUsage.h"
#include "CompiledTaffyVersion.h"

#include "dcTaffyApplication.h"
#include "dcArrayClass.h"
#include "dcCommandLineArguments.h"
#include "dcDefines.h"
#include "dcError.h"
#include "dcFileEvaluator.h"
#include "dcGarbageCollector.h"
#include "dcIOClass.h"
#include "dcKernelClass.h"
#include "dcList.h"
#include "dcLog.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcStringClass.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"

static void dcTaffyApplication_displayHelp(void)
{
    dcIOClass_printFormat("%s", __compiledTaffyUsage);
}

static void *executeCommandLine(void *_commandLine)
{
    const dcString *commandLine = (const dcString *)_commandLine;
    dcNode *result = dcStringEvaluator_evalString(commandLine->string,
                                                  "taffy",
                                                  NO_STRING_EVALUATOR_FLAGS);
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    if (dcNodeEvaluator_hasException(evaluator))
    {
        char *text = dcNodeEvaluator_generateExceptionText(evaluator);
        dcIOClass_printFormat("==> %s\n", text);
        dcMemory_free(text);
    }
    else if (evaluator->exit)
    {
        dcIOClass_printFormat("==> true\n");
    }
    else
    {
        dcIOClass_printFormat("==> %s\n", dcNode_display(result));
    }

    return NULL;
}

int dcTaffyApplication_go(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcResult result = TAFFY_SUCCESS;
    const dcString *commandLine = NULL;
    bool displayTime = false;

    // parse the command-line arguments //
    dcCommandLineArguments *arguments =
        dcTaffyCommandLineArguments_parseAndCreate(_argc, _argv);

    if (arguments == NULL)
    {
        dcTaffyApplication_displayHelp();
        result = TAFFY_FAILURE;
        goto kickout;
    }

    commandLine = dcTaffyCommandLineArguments_getCommandLine(arguments);
    displayTime = dcTaffyCommandLineArguments_getDisplayTime(arguments);

    if (dcTaffyCommandLineArguments_displayVersion(arguments))
    {
        dcSystem_displayVersion("Taffy");
        goto kickout;
    }
    else if ((commandLine == NULL
              && dcTaffyCommandLineArguments_getFilename(arguments) == NULL)
             || dcTaffyCommandLineArguments_displayHelp(arguments))
    {
        if (! dcTaffyCommandLineArguments_displayHelp(arguments))
        {
            result = TAFFY_FAILURE;
        }

        dcTaffyApplication_displayHelp();
        goto kickout;
    }

    // initialize the system
    if (dcSystem_createWithArguments(arguments) == NULL)
    {
        result = TAFFY_FAILURE;
    }
    else
    {
        if (commandLine != NULL)
        {
            // execute the program specified on the command line
            dcNodeEvaluator_synchronizeFunctionCall
                (dcSystem_getCurrentNodeEvaluator(),
                 &executeCommandLine,
                 (void *)commandLine);
        }
        else
        {
            // execute the program specified by the filenames
            dcFileEvaluator *fileEvaluator =
                dcFileEvaluator_createWithArguments(arguments);
            clock_t start = clock();

            if (fileEvaluator->filename != NULL)
            {
                result = dcFileEvaluator_execute(fileEvaluator);
            }

            clock_t end = clock();
            double timeUsed = ((double)(end - start)) / CLOCKS_PER_SEC;

            if (displayTime)
            {
                dcIOClass_printFormat("> time elapsed: %f\n", timeUsed);
            }

            dcFileEvaluator_free(&fileEvaluator);
        }
    }

    // final cleanup //
    dcSystem_free();

kickout:
    dcMemory_deinitialize();

    // convert to a result that a shell will understand
    return (result == TAFFY_SUCCESS
            ? 0
            : 1);
}
