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

#include "dcFileEvaluator.h"
#include "dcCommandLineArguments.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcFileManagement.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcIOClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcParser.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"

dcFileEvaluator *dcFileEvaluator_createWithArguments
    (dcCommandLineArguments *_arguments)
{
    dcFileEvaluator *fileEvaluator =
        (dcFileEvaluator *)dcMemory_allocate(sizeof(dcFileEvaluator));
    fileEvaluator->filename =
        dcTaffyCommandLineArguments_getFilename(_arguments);
    return fileEvaluator;
}

dcFileEvaluator *dcFileEvaluator_create(void)
{
    return (dcFileEvaluator *)(dcMemory_allocateAndInitialize
                               (sizeof(dcFileEvaluator)));
}

void dcFileEvaluator_free(dcFileEvaluator **_evaluator)
{
    dcMemory_free(*_evaluator);
}

dcResult dcFileEvaluator_execute(dcFileEvaluator *_fileEvaluator)
{
    if (_fileEvaluator->filename != NULL)
    {
        return ((dcFileEvaluator_evaluateFile(_fileEvaluator->filename->string)
                 == NULL)
                ? TAFFY_EXCEPTION
                : TAFFY_SUCCESS);
    }

    return TAFFY_FAILURE;
}

dcNode *dcFileEvaluator_evaluateFile(const char *_fileName)
{
    return dcFileEvaluator_evaluateFileWithExceptionCatch(_fileName, true);
}

typedef struct
{
    const char *fileName;
    bool handleException;
} EvaluateFileArguments;

static void *evaluateFileSynchronized(void *_argument)
{
    EvaluateFileArguments *arguments = (EvaluateFileArguments *)_argument;
    dcNode *result = NULL;
    FILE *file = dcFileManagement_openFile(arguments->fileName, "r");
    dcNodeEvaluator *nodeEvaluator = dcSystem_getCurrentNodeEvaluator();

    if (file == NULL)
    {
        if (arguments->handleException)
        {
            dcFileOpenExceptionClass_throwObject(arguments->fileName);
        }
    }
    else
    {
        dcLexer *lexer = dcLexer_createFromFile(arguments->fileName, file);

        if (lexer != NULL)
        {
            dcNode *parseHead = dcParser_synchronizedParse(lexer, true, NULL);

            if (parseHead != NULL)
            {
                result = dcNodeEvaluator_evaluate(nodeEvaluator, parseHead);
            }

            dcGarbageCollector_nodeEvaluatorDown();
            dcGarbageCollector_lock();

            if (result == parseHead)
            {
                nodeEvaluator->returnValue = NULL;
            }

            dcGarbageCollector_unlock();
            dcGarbageCollector_nodeEvaluatorUp();

            dcLexer_free(&lexer);
            dcNode_free(&parseHead, DC_DEEP);
        }

        dcFileManagement_closeFile(file);
    }

    if (arguments->handleException)
    {
        char *exceptionDisplay =
            dcNodeEvaluator_generateExceptionText(nodeEvaluator);

        if (exceptionDisplay != NULL)
        {
            result = NULL;
            dcIOClass_printFormat("==> %s\n", exceptionDisplay);
            dcMemory_free(exceptionDisplay);
        }
    }

    return result;
}

dcNode *dcFileEvaluator_evaluateFileWithExceptionCatch(const char *_fileName,
                                                       bool _handleException)
{
    EvaluateFileArguments arguments = {0};
    arguments.fileName = _fileName;
    arguments.handleException = _handleException;
    return (dcNode *)(dcNodeEvaluator_synchronizeFunctionCall
                      (dcSystem_getCurrentNodeEvaluator(),
                       (dcGenericFunction)(&evaluateFileSynchronized),
                       &arguments));
    return NULL;
}
