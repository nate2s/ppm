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
#include <stdarg.h>

#include "CompiledIO.h"

#include "dcIOClass.h"
#include "dcArray.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcExceptions.h"
#include "dcGarbageCollector.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcMemory.h"
#include "dcNilClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcObjectClass.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcYesClass.h"

static void genericOutputFunction(const char *_output)
{
    printf("%s", _output);
}

static OutputFunction sTheOutputFunction = genericOutputFunction;

void dcIOClass_setOutputFunction(OutputFunction _function)
{
    sTheOutputFunction = _function;
}

void dcIOClass_resetOutputFunction(void)
{
    sTheOutputFunction = &genericOutputFunction;
}

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "flush",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcIOClass_flush,
        gCFunctionArgument_none
    },
    {
        "put:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcIOClass_put,
        gCFunctionArgument_wild
    },
    {
        "put:width:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcIOClass_putWidth,
        gCFunctionArgument_wildNumber
    },
    {
        "putLine:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcIOClass_putLine,
        gCFunctionArgument_wild
    },
    {
        0
    }
};

static dcNode *sInstance = NULL;
static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcIOClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (IO_PACKAGE_NAME,                      // package name
          IO_CLASS_NAME,                        // class name
          MAKE_FULLY_QUALIFIED(OBJECT),         // super name
          CLASS_SINGLETON,                      // class flags
          NO_FLAGS,                             // scope data flags
          NULL,                                 // meta methods
          sMethodWrappers,                      // methods
          &dcIOClass_initialize,                // initialization function
          NULL,                                 // deinitialization function
          NULL,                                 // allocate
          NULL,                                 // deallocate
          NULL,                                 // meta mark
          NULL,                                 // mark
          NULL,                                 // copy
          NULL,                                 // free
          NULL,                                 // register
          NULL,                                 // marshall
          NULL,                                 // unmarshall
          NULL));                               // set template
}

dcNode *dcIOClass_createNode(bool _object)
{
    return dcClass_createBasicNode(sTemplate, _object);
}

dcNode *dcIOClass_createObject(void)
{
    return dcIOClass_createNode(true);
}

void dcIOClass_initialize(void)
{
    assert(dcStringEvaluator_evalString(__compiledIO,
                                        "src/class/dcIOClass.c",
                                        NO_STRING_EVALUATOR_FLAGS)
           != NULL);

    assert(dcStringEvaluator_evalString("global putLine(x) = [io putLine: x]",
                                        "src/class/dcIOClass.c",
                                        NO_STRING_EVALUATOR_FLAGS)
           != NULL);

    assert(dcStringEvaluator_evalString("global put(x) = [io put: x]",
                                        "src/class/dcIOClass.c",
                                        NO_STRING_EVALUATOR_FLAGS)
           != NULL);

    sInstance = dcIOClass_createObject();
    dcSystem_addToGlobalScope(sInstance, "io");
    dcClassManager_registerSingleton(sInstance, "io");
}

void dcIOClass_resetInstance(void)
{
    sInstance = dcScope_getObject(CAST_SCOPE(dcSystem_getGlobalScope()), "io");
}

dcNode *dcIOClass_getInstance(void)
{
    return sInstance;
}

// uses the output function to print!
size_t dcIOClass_printFormat(const char *_format, ...)
{
    // create the output
    size_t result;
    va_list arguments;
    va_start(arguments, _format);
    char *output = dcLexer_sprintfWithVaList(_format, arguments, &result);

    // output the output
    sTheOutputFunction(output);

    // we're done
    dcMemory_free(output);
    return result;
}

typedef enum
{
    NO_PUT_FLAGS = 0,
    PUT_NEWLINE  = 1,
    PUT_WIDTH    = 2
} PutFlags;

static dcNode *put(dcNode *_receiver,
                   dcArray *_arguments,
                   PutFlags _flags)
{
    dcNode *object = dcArray_get(_arguments, 0);
    dcNode *result = NULL;

    // convert the argument to char* //
    char *display = dcStringClass_asString_noQuotes_helper(object);

    if (display != NULL)
    {
        uint32_t width = 0;
        bool exception = false;

        if ((_flags & PUT_WIDTH) != 0)
        {
            exception = ! (dcNumberClass_extractInt32u_withException
                           (dcArray_get(_arguments, 1),
                            &width));
        }

        if (! exception)
        {
            if ((_flags & PUT_WIDTH) == 0)
            {
                sTheOutputFunction(display);
            }
            else
            {
                char *output = dcLexer_sprintf("%-*s", width, display);
                sTheOutputFunction(output);
                dcMemory_free(output);
            }

            // TODO: speed this up to just one call to sTheOutputFunction()
            // but we can't modify display since it's garbage collected already
            if ((_flags & PUT_NEWLINE) != 0)
            {
                sTheOutputFunction("\n");
            }

            result = sInstance;
        }
        // else exception is already set from
        // dcNumberClass_extractInt32u_withException
    }
    // else exception is already set from dcStringClass_asString_helper //

    dcMemory_free(display);
    return result;
}

// taffy methods //
dcNode *dcIOClass_flush(dcNode *_receiver, dcArray *_arguments)
{
    fflush(stdout);
    return sInstance;
}

dcNode *dcIOClass_put(dcNode *_receiver, dcArray *_arguments)
{
    return put(_receiver, _arguments, NO_PUT_FLAGS);
}

dcNode *dcIOClass_putLine(dcNode *_receiver, dcArray *_arguments)
{
    return put(_receiver, _arguments, PUT_NEWLINE);
}

dcNode *dcIOClass_putWidth(dcNode *_receiver, dcArray *_arguments)
{
    return put(_receiver, _arguments, PUT_WIDTH);
}
