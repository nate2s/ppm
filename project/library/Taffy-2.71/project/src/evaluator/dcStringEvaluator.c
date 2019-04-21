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

#include <string.h>

#include "dcError.h"
#include "dcIOClass.h"
#include "dcLexer.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcParser.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"

typedef struct
{
    const char *inputString;
    const char *fileName;
    const char *format;
    dcStringEvaluator_evalFlag flags;
    dcNodeEvaluator *evaluator;
    va_list *argumentPointer;
} EvalArguments;

static void *evalString(void *_argument)
{
    EvalArguments *arguments = (EvalArguments*)_argument;
    dcNode *parseHead = dcParser_parseString(arguments->inputString,
                                             arguments->fileName,
                                             true);
    dcNode *result = NULL;

    if (parseHead != NULL)
    {
        // evaluate the parse head
        // result will be already registered with el garbage collector
        result = dcNodeEvaluator_evaluate(arguments->evaluator, parseHead);

        // if the result is a template, parseHead probably points into it
        result = dcNode_copyIfTemplate(result);
        dcNodeEvaluator_setReturnValue(arguments->evaluator, result);
        dcNode_free(&parseHead, DC_DEEP);
    }

    if (result == NULL
        && (arguments->flags & STRING_EVALUATOR_ASSERT_NO_EXCEPTION) != 0)
    {
        dcError_assert(false);
    }

    if ((arguments->flags & STRING_EVALUATOR_HANDLE_EXCEPTION) != 0)
    {
        char *exceptionText =
            dcNodeEvaluator_generateExceptionText(arguments->evaluator);

        if (exceptionText != NULL)
        {
            dcIOClass_printFormat("%s\n", exceptionText);
        }

        dcMemory_free(exceptionText);
    }

    return result;
}

static void *evalFormat(void *_argument)
{
    EvalArguments *arguments = (EvalArguments *)_argument;
    char *buffer = dcLexer_sprintfWithVaList
        (arguments->format, *arguments->argumentPointer, NULL);
    arguments->inputString = buffer;
    dcNode *result = (dcNode *)evalString(arguments);
    dcMemory_free(buffer);
    return result;
}

dcNode *dcStringEvaluator_evalFormat(const char *_fileName,
                                     dcStringEvaluator_evalFlag _flags,
                                     const char *_format,
                                     ...)
{
    va_list argumentPointer;
    va_start(argumentPointer, _format);
    EvalArguments arguments = {0};
    arguments.argumentPointer = &argumentPointer;
    arguments.format = _format;
    arguments.fileName = _fileName;
    arguments.evaluator = dcSystem_getCurrentNodeEvaluator();

    dcNode *result = NULL;

    if (_flags & STRING_EVALUATOR_SYNCHRONIZE)
    {
        result = (dcNode *)(dcNodeEvaluator_synchronizeFunctionCall
                            (arguments.evaluator, &evalFormat, &arguments));
    }
    else
    {
        result = (dcNode *)evalFormat(&arguments);
    }

    return result;
}

dcNode *dcStringEvaluator_evalString(const char *_inputString,
                                     const char *_fileName,
                                     dcStringEvaluator_evalFlag _flags)
{
    EvalArguments arguments = {0};
    arguments.inputString = _inputString;
    arguments.fileName = _fileName;
    arguments.evaluator = dcSystem_getCurrentNodeEvaluator();
    arguments.flags = _flags;
    dcNode *result = NULL;

    dcError_assert(arguments.evaluator->exception == NULL);

    if (_flags & STRING_EVALUATOR_SYNCHRONIZE)
    {
        result = (dcNode *)(dcNodeEvaluator_synchronizeFunctionCall
                            (arguments.evaluator,
                             &evalString,
                             &arguments));
    }
    else
    {
        result = (dcNode *)evalString(&arguments);
    }

    return result;
}

static long int stringArrayI = 0;

dcNode *dcStringEvaluator_evalStringArray(const char *_array[],
                                          size_t _count)
{
    size_t i;
    dcNode *result = NULL;

    for (i = 0; i < _count; i++)
    {
        char *filename =
            dcLexer_sprintf("string evaluator file #%u", stringArrayI);
        stringArrayI++;
        result = dcStringEvaluator_evalString(_array[i],
                                              filename,
                                              NO_STRING_EVALUATOR_FLAGS);
        dcMemory_free(filename);
        if (result == NULL)
        {
            break;
        }
    }

    return result;
}
