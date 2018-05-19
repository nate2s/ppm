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
#include <time.h>

#include "CompiledKernel.h"

#include "dcKernelClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcCommandLineArguments.h"
#include "dcContainers.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcExceptions.h"
#include "dcFileEvaluator.h"
#include "dcFileManagement.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHashClass.h"
#include "dcIOClass.h"
#include "dcLexer.h"
#include "dcListClass.h"
#include "dcLog.h"
#include "dcMainClass.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcObjectClass.h"
#include "dcObjectStack.h"
#include "dcObjectStackList.h"
#include "dcPairClass.h"
#include "dcParser.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSymbolClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcYesClass.h"
#include "dcVoid.h"

#ifndef TAFFY_WINDOWS
    #include "dcSocket.h"
    #include <dlfcn.h>
#endif

#ifdef ENABLE_DEBUG
dcNode *garbageCollect(dcNode *_receiver, dcArray *_arguments);
#endif

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
#ifdef ENABLE_DEBUG
    {
        "garbageCollect",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &garbageCollect,
        gCFunctionArgument_none
    },
#endif
    {
        "addIncludeDirectory:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcKernelClass_addIncludeDirectory,
        gCFunctionArgument_string
    },
    {
        "allocatedMemory",
        SCOPE_DATA_PUBLIC,
        &dcKernelClass_allocatedMemory,
        gCFunctionArgument_none
    },
    {
        "arguments",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcKernelClass_arguments,
        gCFunctionArgument_none
    },
    {
        "assert:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcKernelClass_assert,
        gCFunctionArgument_wild
    },
    {
        "constify:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST
         | SCOPE_DATA_BREAKTHROUGH),
        &dcKernelClass_constify,
        gCFunctionArgument_string
    },
    {
        "eval:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH),
        &dcKernelClass_eval,
        gCFunctionArgument_string
    },
#ifdef ENABLE_EXECUTE_ON_SYSTEM
    {
        "executeOnSystem:",
        SCOPE_DATA_PUBLIC,
        &dcKernelClass_executeOnSystem,
        gCFunctionArgument_string
    },
#endif
    {
        "exit",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcKernelClass_exit,
        gCFunctionArgument_none
    },
    {
        "getSecondTime",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcKernelClass_getSecondTime,
        gCFunctionArgument_none
    },
    {
        "includeDirectories",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcKernelClass_includeDirectories,
        gCFunctionArgument_none
    },
#ifdef ENABLE_DEBUG
    {
        "loadState:",
        SCOPE_DATA_PUBLIC,
        &dcKernelClass_loadState,
        gCFunctionArgument_string
    },
#endif
    {
        "marshall:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcKernelClass_marshall,
        gCFunctionArgument_wild
    },
#ifdef ENABLE_DEBUG
    {
        "saveState:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcKernelClass_saveState,
        gCFunctionArgument_string
    },
#endif
    {
        "sleepSeconds:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcKernelClass_sleepSeconds,
        gCFunctionArgument_number
    },
#ifdef ENABLE_DEBUG
    {
        "symbols",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcKernelClass_symbols,
        gCFunctionArgument_none
    },
    {
        "systemSupportsPlugins",
        SCOPE_DATA_PUBLIC,
        &dcKernelClass_systemSupportsPlugins,
        gCFunctionArgument_none
    },
    {
        "testOutOfMemoryException",
        SCOPE_DATA_PUBLIC,
        &dcKernelClass_testOutOfMemoryException,
        gCFunctionArgument_none
    },
#endif
    {
        "unmarshall:",
        SCOPE_DATA_PUBLIC,
        &dcKernelClass_unmarshall,
        gCFunctionArgument_array
    },
    {
        0
    }
};

#define CAST_KERNEL_AUX(_node_) ((dcKernelClassAux*)(CAST_CLASS_AUX(_node_)))
#define DEFAULT_CONNECTOR_PORT 1777

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcKernelClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (KERNEL_PACKAGE_NAME,                  // package name
          KERNEL_CLASS_NAME,                    // class name
          MAKE_FULLY_QUALIFIED(OBJECT),         // super name
          CLASS_SINGLETON,                      // class flags
          NO_FLAGS,                             // scope data flags
          NULL,                                 // meta methods
          sMethodWrappers,                      // methods
          &dcKernelClass_initialize,            // initialization function
          NULL,                                 // deinitialization function
          &dcKernelClass_allocateNode,          // allocate
          &dcKernelClass_deallocateNode,        // deallocate
          NULL,                                 // meta mark
          &dcKernelClass_markNode,              // mark
          &dcKernelClass_copyNode,              // copy
          &dcKernelClass_freeNode,              // free (singleton has none)
          &dcKernelClass_registerNode,          // register
          &dcKernelClass_marshallNode,          // marshall
          &dcKernelClass_unmarshallNode,        // unmarshall
          NULL));                               // set template
}

static dcNode *sInstance = NULL;

static void resetInstance(void)
{
    sInstance = dcScope_getObject(CAST_SCOPE(dcSystem_getGlobalScope()),
                                  "kernel");
}

dcString *dcKernelClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    //
    // Pack the symbols
    //
    dcHash_marshall(CAST_KERNEL_AUX(_node)->symbols, _stream);
    return _stream;
}

static dcKernelClassAux *createAux(dcHash *_symbols)
{
    dcKernelClassAux *result = (dcKernelClassAux *)(dcMemory_allocate
                                                    (sizeof(dcKernelClassAux)));
    result->symbols = _symbols;
    return result;
}

bool dcKernelClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    //
    // Unpack the symbols
    //
    dcHash *symbols = dcHash_unmarshall(_stream);
    bool result = false;

    if (symbols != NULL)
    {
        CAST_CLASS_AUX(_node) = createAux(symbols);
        result = true;
    }

    return result;
}

void dcKernelClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_CLASS_AUX(_to) = createAux(dcHash_copy(CAST_KERNEL_AUX(_from)->symbols,
                                                DC_DEEP));
}

void dcKernelClass_registerNode(dcNode *_kernelNode)
{
    dcHash_register(CAST_KERNEL_AUX(_kernelNode)->symbols);
}

void dcKernelClass_deallocateNode(dcNode *_node)
{
    dcHash_clear(CAST_KERNEL_AUX(_node)->symbols, DC_SHALLOW);
}

void dcKernelClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcKernelClassAux *aux = CAST_KERNEL_AUX(_node);
    dcHash_free(&aux->symbols, DC_DEEP);
    dcMemory_free(aux);
}

dcNode *dcKernelClass_getInstance(void)
{
    return sInstance;
}

#define KERNEL_TAFFY_FILE_NAME "src/class/Kernel.ty"

static void evalAndSet(dcNode **_nodeLoc, const char *_name)
{
    *_nodeLoc = dcStringEvaluator_evalString
        (_name, KERNEL_TAFFY_FILE_NAME, NO_STRING_EVALUATOR_FLAGS);
    dcError_assert(*_nodeLoc != NULL);
    dcSystem_addToGlobalScope(*_nodeLoc, _name);
}

void dcKernelClass_initialize(void)
{
    assert(sInstance == NULL);
    assert(dcStringEvaluator_evalString(__compiledKernel,
                                        KERNEL_TAFFY_FILE_NAME,
                                        NO_STRING_EVALUATOR_FLAGS)
           != NULL);
    sInstance = dcKernelClass_createObject();
    dcSystem_addToGlobalScope(sInstance, "kernel");
    dcClassManager_registerSingleton(sInstance, "kernel");

    dcKernelClassAux *aux = CAST_KERNEL_AUX(sInstance);
    evalAndSet(&aux->lessThanSymbol, "'lessThan");
    evalAndSet(&aux->equalSymbol, "'equal");
    evalAndSet(&aux->greaterThanSymbol, "'greaterThan");
    evalAndSet(&aux->uncomparableSymbol, "'uncomparable");
}

void dcKernelClass_allocateNode(dcNode *_node)
{
    // singleton
    CAST_CLASS_AUX(_node) = CAST_CLASS_AUX(sInstance);
}

dcNode *dcKernelClass_getLessThanSymbol(void)
{
    return CAST_KERNEL_AUX(sInstance)->lessThanSymbol;
}

dcNode *dcKernelClass_getEqualSymbol(void)
{
    return CAST_KERNEL_AUX(sInstance)->equalSymbol;
}

dcNode *dcKernelClass_getGreaterThanSymbol(void)
{
    return CAST_KERNEL_AUX(sInstance)->greaterThanSymbol;
}

dcNode *dcKernelClass_getUncomparableSymbol(void)
{
    return CAST_KERNEL_AUX(sInstance)->uncomparableSymbol;
}

dcNode *dcKernelClass_createNode(bool _object)
{
    dcKernelClassAux *aux = (dcKernelClassAux *)(dcMemory_allocate
                                                 (sizeof(dcKernelClassAux)));
    aux->symbols = dcHash_create();
    return dcClass_createNode(sTemplate,
                              NULL,         // super
                              NULL,         // scope
                              _object,      // object ?
                              aux);
}

dcNode *dcKernelClass_createObject(void)
{
    return dcKernelClass_createNode(true);
}

void dcKernelClass_markNode(dcNode *_node)
{
    dcHash_mark(CAST_KERNEL_AUX(_node)->symbols);
}

dcNode *dcKernelClass_getOrCreateSymbol(const char *_symbolName)
{
    // symbols start with a ' in taffy-land, but are '-less in C-land
    assert(_symbolName != NULL
           && _symbolName[0] != '\'');

    dcHash *symbols = CAST_KERNEL_AUX(sInstance)->symbols;
    dcNode *result;

    if (dcHash_getValueWithStringKey(symbols, _symbolName, &result)
        == TAFFY_FAILURE)
    {
        result = dcSymbolClass_createNode(_symbolName, true);
        dcHash_setValueWithStringKey(symbols, _symbolName, result);
        dcNode_register(result);
    }

    return result;
}

#ifdef ENABLE_DEBUG
dcNode *dcKernelClass_systemSupportsPlugins(dcNode *_receiver,
                                            dcArray *_arguments)
{
#if (defined TAFFY_APPLE) || (defined TAFFY_LINUX)
    return dcYesClass_getInstance();
#else
    return dcNoClass_getInstance();
#endif
}

dcNode *dcKernelClass_symbols(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *symbolsArray =
        dcHash_getValues(CAST_KERNEL_AUX(_receiver)->symbols);
    return dcNode_register(dcArrayClass_createObject(symbolsArray, true));
}
#endif // ENABLE_DEBUG

#ifdef ENABLE_EXECUTE_ON_SYSTEM
dcNode *dcKernelClass_executeOnSystem(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *commandNode = dcArray_get(_arguments, 0);
    int intResult = system(dcStringClass_getString(commandNode));
    return dcNode_register(dcNumberClass_createObjectFromInt32s(intResult));
}
#endif // ENABLE_EXECUTE_ON_SYSTEM

dcNode *dcKernelClass_assert(dcNode *_receiver, dcArray *_arguments)
{
    if (! dcClass_hasTemplate(dcArray_get(_arguments, 0),
                              dcYesClass_getTemplate(),
                              true))
    {
        dcAssertFailedExceptionClass_throwObject();
        return NULL;
    }

    return dcYesClass_getInstance();
}

dcNode *dcKernelClass_constify(dcNode *_receiver, dcArray *_arguments)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    const char *name = dcStringClass_getString(dcArray_get(_arguments, 0));
    dcNode *scopeDataNode =
        dcObjectStackList_getScopeDataForObject(evaluator->objectStackList,
                                                name,
                                                NULL);
    if (scopeDataNode == NULL)
    {
        dcUnidentifiedObjectExceptionClass_throwObject(name);
        return NULL;
    }

    CAST_SCOPE_DATA(scopeDataNode)->flags |= SCOPE_DATA_CONSTANT;
    return dcYesClass_getInstance();
}

static dcScope *getLocalScope(dcNodeEvaluator *_evaluator)
{
    return (CAST_SCOPE
            (dcList_getHead
             (CAST_OBJECT_STACK
              (dcList_getHead(_evaluator->objectStackList->objectStacks))
              ->scopes)));
}

bool dcKernelClass_unmarshallState(dcString *_input,
                                   dcNodeEvaluator *_evaluator)
{
    dcScope *newGlobalScope = NULL;
    dcScope *newLocalScope = NULL;
    bool result = false;

    if (dcMarshaller_unmarshallNoNull(_input,
                                      "SS",
                                      &newGlobalScope,
                                      &newLocalScope))
    {
        result = true;
        dcGarbageCollector_blockOtherNodeEvaluators();
        dcScope *globalScope = CAST_SCOPE(dcSystem_getGlobalScope());
        dcScope_merge(globalScope, newGlobalScope);
        dcScope_register(globalScope);
        dcScope *localScope = getLocalScope(_evaluator);
        dcScope_merge(localScope, newLocalScope);
        dcScope_register(localScope);
        dcGarbageCollector_unblockOtherNodeEvaluators();

        resetInstance();
        dcNilClass_resetInstance();
        dcIOClass_resetInstance();
        dcYesClass_resetInstance();
        dcNoClass_resetInstance();
        dcMainClass_resetInstance();
    }

    dcScope_free(&newGlobalScope, DC_DEEP);
    dcScope_free(&newLocalScope, DC_DEEP);

    return result;
}

#ifdef ENABLE_DEBUG
dcNode *dcKernelClass_loadState(dcNode *_receiver, dcArray *_arguments)
{
    const char *filename = dcStringClass_getString(dcArray_get(_arguments, 0));
    FILE *file = fopen(filename, "r");
    bool success = false;

    if (file == NULL)
    {
        // failure
        dcExceptions_throwObject
            (dcFileOpenExceptionClass_createObject(filename));
    }
    else
    {
        dcString *input = dcFileManagement_extractAllInputFromFile(file);

        if (input == NULL)
        {
            // failure
            dcExceptions_throwObject
                (dcFileOpenExceptionClass_createObject(filename));
        }
        else
        {
            success = (dcKernelClass_unmarshallState
                       (input,
                        dcSystem_getCurrentNodeEvaluator()));
        }

        dcString_free(&input, DC_DEEP);
        fclose(file);
    }

    return (success
            ? dcYesClass_getInstance()
            : dcNoClass_getInstance());
}

dcString *dcKernelClass_marshallState(dcString *_string,
                                      dcNodeEvaluator *_evaluator)
{
    dcGarbageCollector_blockOtherNodeEvaluators();
    dcScope *localScope = getLocalScope(_evaluator);
    fprintf(stderr, "local scope is: %s\n", dcScope_display(localScope));

    dcString *result = (dcMarshaller_marshall
                        (_string,
                         "SS",
                         CAST_SCOPE(dcSystem_getGlobalScope()),
                         localScope));
    dcGarbageCollector_unblockOtherNodeEvaluators();
    return result;
}

dcNode *dcKernelClass_saveState(dcNode *_receiver, dcArray *_arguments)
{
    const char *filename = dcStringClass_getString(dcArray_get(_arguments, 0));
    FILE *file = fopen(filename, "w");
    dcNode *result = NULL;

    if (file != NULL)
    {
        dcString *marshalled =
            dcKernelClass_marshallState(NULL,
                                        dcSystem_getCurrentNodeEvaluator());

        if (marshalled != NULL)
        {
            size_t written = fwrite(marshalled->string,
                                    1,
                                    marshalled->length,
                                    file);

            if (written > 0 && ferror(file) == 0)
            {
                result = dcNode_register
                    (dcNumberClass_createObjectFromInt64u(written));
            }
            // else throw an exception?

            dcString_free(&marshalled, DC_DEEP);
        }

        fclose(file);
    }

    if (result == NULL)
    {
        dcFileWriteExceptionClass_throwObject
            (filename,
             (file == NULL
              ? "invalid permissions to write file"
              : strerror(ferror(file))));
    }

    return result;
}
#endif // ENABLE_DEBUG

dcNode *dcKernelClass_sleepSeconds(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *sleepNumberNode = dcArray_get(_arguments, 0);
    int32_t sleepNumber = 0;
    dcNode *result = NULL;

    if (dcNumberClass_extractInt32s_withException(sleepNumberNode,
                                                  &sleepNumber))
    {
        result = dcNilClass_getInstance();

#ifdef TAFFY_WINDOWS
        Sleep(sleepNumber * 1000); // convert to ms
#else
        sleep(sleepNumber);
#endif
    }

    return result;
}

dcNode *dcKernelClass_allocatedMemory(dcNode *_receiver, dcArray *_arguments)
{
    char *display = dcLexer_sprintf("%lld", dcMemory_getAllocatedMemorySize());
    dcNode *result =
        dcNode_register(dcNumberClass_createObjectFromString(display));
    dcMemory_free(display);
    return result;
}

dcNode *dcKernelClass_marshall(dcNode *_receiver, dcArray *_arguments)
{
    uint32_t i;
    dcString *marshalled = dcMarshaller_marshall(NULL,
                                                 "n",
                                                 dcArray_get(_arguments, 0));

    // marshalled must be non-NULL
    dcError_check(marshalled != NULL, "marshalled is NULL but must not be");

    if (marshalled->length > (uint32_t)-1)
    {
        // TODO: add exception
        assert(false);
    }

    dcArray *array = dcArray_createWithSize((uint32_t)marshalled->index);

    for (i = 0; i < marshalled->index; i++)
    {
        dcNode *number = dcNumberClass_createObjectFromInt32u
            ((uint8_t)marshalled->string[i]);
        dcArray_set(array, number, i);
    }

    dcString_free(&marshalled, DC_DEEP);
    return dcNode_register(dcArrayClass_createObject(array, true));
}

dcNode *dcKernelClass_unmarshall(dcNode *_receiver, dcArray *_arguments)
{
    uint32_t i;
    dcNode *arrayObject = dcArray_get(_arguments, 0);
    dcArray *array = dcArrayClass_getObjects(arrayObject);
    dcNode *result = NULL;
    dcString *stream = dcString_createWithLength(array->size);
    bool exception = false;

    // TODO: remove this in deference to a ByteArray class
    for (i = 0; i < array->size; i++)
    {
        uint8_t value;

        if (dcNumberClass_extractInt8u_withException(dcArray_get(array, i),
                                                     &value))
        {
            dcString_appendCharacter(stream, value);
        }
        else
        {
            // failure
            exception = true;
            break;
        }
    }

    dcString_resetIndex(stream);

    if (! exception)
    {
        result = dcNode_unmarshall(stream);

        // if it's an identifier, it's for a singleton
        if (result != NULL
            && IS_IDENTIFIER(result))
        {
            // any exceptions will happen here
            dcNode *save = result;
            result = (dcNodeEvaluator_evaluateIdentifier
                      (dcSystem_getCurrentNodeEvaluator(),
                       result));
            dcNode_free(&save, DC_DEEP);
        }
        else if (result == NULL
                 || ! IS_CLASS(result))
        {
            dcNode_free(&result, DC_DEEP);
            dcUnmarshallFailureExceptionClass_throwObject(arrayObject);
        }
        else
        {
            dcNode_register(result);
        }
    }

    dcString_free(&stream, DC_DEEP);
    return result;
}

dcNode *dcKernelClass_arguments(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNilClass_getInstance();
    const dcList *arguments = dcSystem_getArguments();

    if (arguments != NULL)
    {
        dcArray *newArray =
            dcArray_createWithSize(arguments->size);
        uint32_t i = 0;

        FOR_EACH_IN_LIST(arguments, that)
        {
            dcArray_set
                (newArray,
                 dcStringClass_createObject
                 (dcString_getString(that->object),
                  true),
                 i);
            i++;
        }

        dcNode *arrayNode = dcArrayClass_createObject(newArray, true);
        dcNode_register(arrayNode);
        result = arrayNode;
    }

    return result;
}

dcNode *dcKernelClass_addIncludeDirectory(dcNode *_receiver,
                                          dcArray *_arguments)
{
    dcNode *includeDirectoryNode = dcArray_get(_arguments, 0);
    dcSystem_addIncludeDirectory(dcStringClass_getString(includeDirectoryNode));
    return dcYesClass_getInstance();
}

dcNode *dcKernelClass_includeDirectories(dcNode *_receiver, dcArray *_arguments)
{
    const dcList *systemIncludeDirectories = dcSystem_getIncludeDirectories();
    dcArray *includeDirectories =
        dcArray_createWithSize(systemIncludeDirectories->size);
    dcListIterator *i = dcList_createHeadIterator(systemIncludeDirectories);
    dcNode *stringNode = NULL;

    while ((stringNode = dcListIterator_getNext(i)))
    {
        dcArray_add(includeDirectories,
                    dcStringClass_createObject
                    (dcString_getString(stringNode),
                     true));
    }

    dcNode *includeDirectoriesObject =
            dcArrayClass_createObject(includeDirectories, true);
    dcNode_register(includeDirectoriesObject);
    return includeDirectoriesObject;
}

// taffy methods //
dcNode *dcKernelClass_eval(dcNode *_receiver, dcArray *_arguments)
{
    // extract the input //
    return (dcStringEvaluator_evalString
            (dcStringClass_getString(dcArray_get(_arguments, 0)),
             dcNodeEvaluator_getCurrentFileName
             (dcSystem_getCurrentNodeEvaluator()),
             NO_STRING_EVALUATOR_FLAGS));
}

dcNode *dcKernelClass_exit(dcNode *_receiver, dcArray *_arguments)
{
    dcSystem_exit();
    return dcYesClass_getInstance();
}

dcNode *dcKernelClass_getSecondTime(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register(dcNumberClass_createObjectFromInt32u
                            ((uint32_t)time(NULL))));
}

bool dcKernelClass_equals_helper(dcNode *_left, dcNode *_right)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *result = dcNodeEvaluator_callMethodWithArgument
        (evaluator, _left, dcSystem_getOperatorName(TAFFY_EQUALS), _right);

    // in case #operator(==): wasn't found...
    dcNodeEvaluator_clearException(evaluator, DC_DEEP);

    return (result == dcYesClass_getInstance());
}

#ifdef ENABLE_DEBUG
dcNode *dcKernelClass_testOutOfMemoryException(dcNode *_receiver,
                                               dcArray *_arguments)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *list = dcNode_register(dcListClass_createEmptyObject());
    dcNodeEvaluator_pushMark(evaluator, list);

    while (dcNodeEvaluator_canContinueEvaluating(evaluator))
    {
        dcList_push(dcListClass_getObjects(list),
                    dcNode_register
                    (dcNumberClass_createObjectFromInt32u(123456789)));
    }

    dcNodeEvaluator_popMark(evaluator);
    return (evaluator->exception != NULL
            ? NULL
            : list);
}
#endif

static dcResult loadLibrary(const char *_libraryNameWithPath,
                            const char *_templateGetter)
{
    //fprintf(stderr, "attempting to load library: %s\n", _libraryNameWithPath);

#ifdef TAFFY_WINDOWS
    return TAFFY_FAILURE;
#else
    void *handle = dlopen(_libraryNameWithPath, RTLD_NOW);
    dcResult result = TAFFY_EXCEPTION;
    char *exception = NULL;

    if (handle != NULL)
    {
        dcTaffy_getTemplatePointer getTemplate = ((dcTaffy_getTemplatePointer)
                                                  dlsym(handle,
                                                        _templateGetter));

        if (getTemplate != NULL)
        {
            if (dcClassManager_registerClassTemplate
                (getTemplate(),
                 NULL,
                 true,
                 NULL)
                == TAFFY_SUCCESS)
            {
                result = TAFFY_SUCCESS;
            }
            // else exception is already set
        }
        else
        {
            exception = dcLexer_sprintf
                ("can't get template from function %s", _templateGetter);
        }
    }
    else
    {
        result = TAFFY_FAILURE;

        if (! dcSystem_silencePluginWarnings())
        {
            dcIOClass_printFormat("Warning: Unable to load plugin: %s. "
                                  "Debug steps: \n"
                                  "0) Verify your DYLD_LIBRARY_PATH (on Mac) "
                                  "or LD_LIBRARY_PATH (on Linux) is set "
                                  "appropriately.\n"
                                  "1) Verify your TAFFY_HOME "
                                  "environment variable is set. \n"
                                  "2) Verify a .taffyHome file exists.\n"
                                  "3) Verify the plugin was compiled and "
                                  "installed\n"
                                  "4) Verify (1) and (2) do not exist "
                                  "together\n"
                                  "Squelch this warning with "
                                  "--silence-plugin-warnings\n\n",
                                  _libraryNameWithPath);
        }

        exception = dcLexer_sprintf("dlopen() error: '%s'", dlerror());
    }

    if (exception == NULL)
    {
        dcSystem_addHandle(handle);
    }
    else
    {
        TAFFY_DEBUG(dcError_assert(result != TAFFY_SUCCESS));
        dcPluginInstantiationExceptionClass_throwObject(exception);
        dcMemory_free(exception);

        if (handle != NULL)
        {
            dlclose(handle);
        }
    }

    return result;
#endif // TAFFY_WINDOWS
}

#ifdef TAFFY_WINDOWS
dcResult dcKernelClass_loadPlugin(const char *_directory)
{
    return TAFFY_FAILURE;
}
#else
dcResult dcKernelClass_loadPlugin(const char *_directory)
{
    dcResult result = TAFFY_SUCCESS;
    char *filename = dcLexer_sprintf("%s%cpluginInfo.ty",
                                     _directory,
                                     DIRECTORY_SEPARATOR);

    // first import TaffyPlugin for convenience
    dcError_assert(dcStringEvaluator_evalString
                   ("import org.taffy.core.TaffyPlugin",
                    filename,
                    NO_STRING_EVALUATOR_FLAGS)
                   != NULL);

    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *pluginInfo =
        dcFileEvaluator_evaluateFileWithExceptionCatch(filename, false);
    uint32_t marks = 0;
    marks += dcNodeEvaluator_pushMark(evaluator, pluginInfo);

    if (pluginInfo == NULL)
    {
        result = TAFFY_EXCEPTION;

        if (evaluator->exception == NULL)
        {
            char *reason =
                dcLexer_sprintf("can't open plugin file %s", filename);
            dcPluginInstantiationExceptionClass_throwObject(reason);
            dcMemory_free(reason);
        }
    }
    else
    {
        if (! dcClass_isType(pluginInfo, "org.taffy.core", "TaffyPlugin"))
        {
            result = TAFFY_EXCEPTION;
            char *reason = dcLexer_sprintf
                ("%s must instantiate a TaffyPlugin", filename);
            dcPluginInstantiationExceptionClass_throwObject(reason);
            dcMemory_free(reason);
        }
        else
        {
            //
            // try to add the taffy source directory
            //
            char *srcDirectory = dcLexer_sprintf
                ("%s%csrc", _directory, DIRECTORY_SEPARATOR);

            if (dcFileManagement_fileExists(srcDirectory))
            {
                dcSystem_addIncludeDirectory(srcDirectory);
            }

            dcMemory_free(srcDirectory);

            //
            // try to load c libraries
            //
            char *libDirectory = dcLexer_sprintf
                ("%s%clib", _directory, DIRECTORY_SEPARATOR);

            //printf("searching lib directory: %s for files ending in %s\n",
            //       libDirectory,
            //       LIBRARY_SUFFIX);

            dcNode *cLibraries = dcClass_getObject(pluginInfo, "@cLibraries");
            dcNode *hashObject = NULL;

            if (cLibraries != NULL
                && dcGraphData_getType(cLibraries) != NODE_NIL
                && ((hashObject = dcClass_castNode(cLibraries,
                                                   dcHashClass_getTemplate(),
                                                   true))
                    != NULL))
            {
                dcHash *hash = dcHashClass_getHash(hashObject);
                dcHashIterator *i = dcHash_createIterator(hash);
                dcNode *that = NULL;
                dcNode *key = NULL;
                dcNode *value = NULL;

                // extract key-value pairs out of the hash
                // each key and value must be a string,
                // else an exception is thrown
                while ((that = dcHashIterator_getNext(i))
                       != NULL
                       && ((key = (dcClass_castNode
                                   (CAST_HASH_ELEMENT
                                    (that)->key.keyUnion.nodeKey,
                                    dcStringClass_getTemplate(),
                                    true)))
                           != NULL)
                       && ((value = (dcClass_castNode
                                     (CAST_HASH_ELEMENT(that)->value,
                                      dcStringClass_getTemplate(),
                                      true)))
                           != NULL)
                       && result == TAFFY_SUCCESS)
                {
                    char *realLibrary =
                        dcLexer_sprintf("%s/%s%s.%s",
                                        libDirectory,
                                        LIBRARY_PREFIX,
                                        dcStringClass_getString(key),
                                        LIBRARY_SUFFIX);
                    result = loadLibrary(realLibrary,
                                         dcStringClass_getString(value));
                    dcMemory_free(realLibrary);
                }

                dcHashIterator_free(&i);
            }

            result = (evaluator->exception == NULL
                      ? TAFFY_SUCCESS
                      : TAFFY_EXCEPTION);

            dcMemory_free(libDirectory);
        }
    }

    dcNodeEvaluator_popMarks(evaluator, marks);
    dcMemory_free(filename);
    return result;
}
#endif // TAFFY_WINDOWS

#ifdef ENABLE_DEBUG
dcNode *garbageCollect(dcNode *_receiver, dcArray *_arguments)
{
    dcGarbageCollector_forceExecution();
    return dcYesClass_getInstance();
}
#endif
