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

#include "dcDefines.h"

#ifndef TAFFY_WINDOWS
#    include <dlfcn.h>
#    include <semaphore.h>
#endif

#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

#include "CompiledTaffyVersion.h"

#include "CompiledReentrantMutex.h"
#include "CompiledThreader.h"

#include "dcSystem.h"
#include "dcBootstrapClasses.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcCommandLineArguments.h"
#include "dcCondition.h"
#include "dcContainers.h"
#include "dcDecNumber.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcFileManagement.h"
#include "dcFilePackageData.h"
#include "dcFlatArithmetic.h"
#include "dcFutureManager.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcIOClass.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcLog.h"
#include "dcMainClass.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcParser.h"
#include "dcReadWriteLock.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcStringManager.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcThread.h"
#include "dcUnsignedInt32.h"
#include "dcVoid.h"

// for dcSystem_convertBoolToNode()
#include "dcNoClass.h"
#include "dcYesClass.h"

static dcSystem *sSystem = NULL;

#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcComplexNumberClass.h"
#include "dcConditionClass.h"
#include "dcConstantClass.h"
#include "dcContainerClass.h"
#include "dcDateClass.h"
#include "dcEquationClass.h"
#include "dcExceptionClass.h"
#include "dcFunctionClass.h"
#include "dcFutureClass.h"
#include "dcHashClass.h"
#include "dcHeapClass.h"
#include "dcIOClass.h"
#include "dcKernelClass.h"
#include "dcLineContainerClass.h"
#include "dcListClass.h"
#include "dcMainClass.h"
#include "dcMathClass.h"
#include "dcMatrixClass.h"
#include "dcMutexClass.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNumberClass.h"
#include "dcObjectClass.h"
#include "dcPairClass.h"
#include "dcParseErrorExceptionClass.h"
#include "dcProcedureClass.h"
#include "dcSeriesClass.h"
#include "dcSequenceClass.h"
#include "dcStringClass.h"
#include "dcSuppliedArgumentClass.h"
#include "dcSymbolClass.h"
#include "dcThreadClass.h"
#include "dcWildClass.h"
#include "dcYesClass.h"

#ifdef COMPILE_EXTERNAL_IO
#    include "dcFileClass.h"
#endif

const dcTaffy_getTemplatePointer gTaffyBootstrapClasses[] =
{
    &dcObjectClass_getTemplate,
    &dcYesClass_getTemplate,
    &dcNoClass_getTemplate,
    &dcProcedureClass_getTemplate,
    &dcFunctionClass_getTemplate,
    &dcEquationClass_getTemplate,
    &dcBlockClass_getTemplate,
    &dcNilClass_getTemplate,
    &dcStringClass_getTemplate,
    &dcKernelClass_getTemplate,
    &dcSymbolClass_getTemplate,
    &dcNumberClass_getTemplate,
    &dcMainClass_getTemplate,

    &dcSuppliedArgumentClass_getTemplate,
    &dcWildClass_getTemplate,

    &dcExceptionClass_getTemplate,
    &dcParseErrorExceptionClass_getTemplate,
    &dcContainerClass_getTemplate,
    &dcLineContainerClass_getTemplate,
    &dcArrayClass_getTemplate,
    &dcHashClass_getTemplate,
    &dcHeapClass_getTemplate,
    &dcIOClass_getTemplate,
    &dcListClass_getTemplate,
    &dcMatrixClass_getTemplate,
    &dcMutexClass_getTemplate,
    &dcConditionClass_getTemplate,
    &dcPairClass_getTemplate,
    &dcSeriesClass_getTemplate,
    &dcSequenceClass_getTemplate,
    &dcThreadClass_getTemplate,
    &dcConstantClass_getTemplate,
    &dcComplexNumberClass_getTemplate,
    &dcMathClass_getTemplate,
    &dcFutureClass_getTemplate,
    &dcDateClass_getTemplate,

#ifdef COMPILE_EXTERNAL_IO
    &dcFileClass_getTemplate,
#endif

    // THE end
    NULL
};

//
// The constructors
//
dcSystem *dcSystem_create(void)
{
    return dcSystem_createWithArguments(dcTaffyCommandLineArguments_create());
}

void dcSystem_addToGlobalScope(dcNode *_object, const char *_name)
{
    // place the object into the global scope
    dcScope_setObject(CAST_SCOPE(sSystem->globalScope),
                      _object,
                      _name,
                      NO_FLAGS);
    dcNode_register(_object);
}

#ifndef TAFFY_CYGWIN
static dcResult loadPlugin(const char *_pluginDirectory, void *_argument)
{
    return dcKernelClass_loadPlugin(_pluginDirectory);
}
#endif

void dcSystem_outOfMemory(void)
{
    sSystem->outOfMemoryCallStack =
        dcSystem_getCurrentNodeEvaluator()->callStack;
    longjmp(sSystem->outOfMemoryBuffer, 1);
}

static void errorOutputFunction(const char *_output)
{
    fprintf(stderr, "%s", _output);
}

void dcSystem_setFinal(const char *_className, bool _yesno)
{
    dcClassTemplate *classTemplate = dcClassManager_getClassTemplate(_className,
                                                                     NULL,
                                                                     NULL,
                                                                     NULL);
    if (classTemplate == NULL)
    {
        fprintf(stderr,
                "Fatal error: class %s has NULL template\n",
                _className);
        assert(false);
    }

    if (_yesno)
    {
        classTemplate->classFlags |= CLASS_FINAL;
    }
    else
    {
        classTemplate->classFlags &= ~CLASS_FINAL;
    }
}

static void *runSynchronizedCreationTasks(void *_argument)
{
    dcSystem *result = sSystem;
    dcCommandLineArguments *arguments = (dcCommandLineArguments *)_argument;
    const dcTaffy_getTemplatePointer *finger;

    if (setjmp(sSystem->outOfMemoryBuffer) != 0)
    {
        assert(sSystem->outOfMemoryCallStack != NULL);
        dcIOClass_setOutputFunction(&errorOutputFunction);
        dcIOClass_printFormat("Aborted: Out of Memory:\n");
        // try to free up some memory so we can print the call stack
        dcIOClass_printFormat
            ("%s\n",
             dcList_display(sSystem->outOfMemoryCallStack));
        exit(1);
    }

    // perform the one-time lexer and parser initialization //
    dcLexer_initialize();
    dcParser_initialize();

    TAFFY_DEBUG(const dcString *stopBootstrapAfterClass =
                dcTaffyCommandLineArguments_getStopBootstrapAfterClass
                (arguments));

    //
    // register the classes
    //
    // first create them all,
    // then create the runtime values,
    // then initialize them
    //
    for (finger = gTaffyBootstrapClasses; *finger != NULL; finger++)
    {
        assert(dcClassManager_registerClassTemplate
               ((*finger)(), NULL, false, NULL)
               == TAFFY_SUCCESS);
    }

    // create the runtime values
    for (finger = gTaffyBootstrapClasses; *finger != NULL; finger++)
    {
        assert(dcClassTemplate_createRuntimeValues((*finger)(), NULL)
               == TAFFY_SUCCESS);
    }

    // initialize them
    for (finger = gTaffyBootstrapClasses; *finger != NULL; finger++)
    {
        dcClassTemplate *classTemplate = (*finger)();

        if (classTemplate->cTemplate != NULL
            && classTemplate->cTemplate->initializer != NULL)
        {
            classTemplate->cTemplate->initializer();
        }

        TAFFY_DEBUG(if (stopBootstrapAfterClass != NULL
                        && (dcString_equalsCharArray
                            (stopBootstrapAfterClass,
                             classTemplate->className)))
                    {
                        break;
                    });
    }

    // sanity
    assert(dcSystem_getCurrentNodeEvaluator()
           == sSystem->nodeEvaluator);

    //
    // sanity check: verify there are no exceptions at this point
    //
    {
        char *exception =
            dcNodeEvaluator_generateExceptionText(sSystem->nodeEvaluator);

        if (exception != NULL)
        {
            dcError_internal("%s", exception);
        }
        // /sanity
    }

    // TODO: find better place for these
    assert(dcStringEvaluator_evalString(__compiledThreader,
                                        "CompiledThreader.c",
                                        NO_STRING_EVALUATOR_FLAGS)
           != NULL);
    assert(dcStringEvaluator_evalString(__compiledReentrantMutex,
                                        "CompiledReentrantMutex.c",
                                        NO_STRING_EVALUATOR_FLAGS)
           != NULL);

    sSystem->abortExceptionClassTemplate =
        dcClassManager_getClassTemplate
        ("org.taffy.core.exception.AbortException",
         NULL,
         NULL,
         NULL);

    dcFlatArithmetic_initialize();

    // set the max future threads
    {
        int maxThreads =
            dcTaffyCommandLineArguments_getMaxFutureThreads(arguments);

        if (maxThreads == -1)
        {
            maxThreads = 10;
        }

        dcFutureManager_initialize(maxThreads);
    }

    sSystem->bootstrap = false;
    dcNodeEvaluator_initializeSelf(sSystem->nodeEvaluator);

#if ! (defined TAFFY_CYGWIN) && ! (defined TAFFY_WINDOWS)
    //
    // load the plugins
    //
    // first load all plugins in $(taffyHome)/plugins
    // then load plugins that are specified via the --plugins argument
    //
    dcResult pluginsResult = TAFFY_SUCCESS;

    if (sSystem->taffyHome != NULL)
    {
        char *pluginDirectory = dcLexer_sprintf
            ("%s%cplugins", sSystem->taffyHome, DIRECTORY_SEPARATOR);

        pluginsResult = dcFileManagement_iterateOverFilesInDirectory
            (pluginDirectory,
             "",
             &loadPlugin,
             NULL);

        dcMemory_free(pluginDirectory);
    }

    if (pluginsResult == TAFFY_SUCCESS)
    {
        dcListElement *plugin = NULL;

        for (plugin = sSystem->pluginDirectories->head;
             plugin != NULL && pluginsResult == TAFFY_SUCCESS;
             plugin = plugin->next)
        {
            pluginsResult = dcKernelClass_loadPlugin
                (dcString_getString(plugin->object));
        }
    }

    if (pluginsResult != TAFFY_SUCCESS)
    {
        char *exceptionText = dcNodeEvaluator_generateExceptionText
            (sSystem->nodeEvaluator);

        if (exceptionText != NULL)
        {
            dcIOClass_printFormat("Fatal Error: %s\n", exceptionText);
        }
        else
        {
            dcIOClass_printFormat("Unknown fatal error. "
                                  "This shouldn't happen. "
                                  "Exiting.\n");
        }

        dcMemory_free(exceptionText);
        result = NULL;
    }
#endif // TAFFY_CYGWIN

    // special: convert Number and ComplexNumber to final. There's a limitation
    // now to Taffy such that if you have a class that is defined in both .c
    // and .ty files, it cannot be final
    dcSystem_setFinal("org.taffy.core.maths.Number", true);
    dcSystem_setFinal("org.taffy.core.maths.ComplexNumber", true);

    dcGarbageCollector_logState();
    return result;
}

dcSystem *dcSystem_createWithArguments(dcCommandLineArguments *_arguments)
{
    dcGarbageCollector_createWithMemoryTip
        (dcTaffyCommandLineArguments_getGarbageCollectorObjectTip(_arguments));

#ifdef ENABLE_DEBUG
    if (! dcLog_configureFromCommandLineArguments(_arguments))
    {
        dcCommandLineArguments_free(&_arguments);
        return NULL;
    }
#endif

    assert(sSystem == NULL);
    sSystem = (dcSystem *)dcMemory_allocateAndInitialize(sizeof(dcSystem));
    sSystem->bootstrap = true;

    sSystem->up = false;
    sSystem->upMutex = dcMutex_create(false);
    sSystem->upCondition = dcCondition_create();
    sSystem->evaluatorsLock = dcReadWriteLock_create();

    sSystem->threads = dcList_create();
    sSystem->evaluators = dcHash_create();
    dcMutex_initialize();
    dcStringManager_create();

    srand((unsigned int)time(0));

    // set up the garbage collector debug state
    dcGarbageCollector_setEnabled
        (! dcTaffyCommandLineArguments_getDisableGarbageCollection(_arguments));
    dcGarbageCollector_setTrackRegistration
        (dcTaffyCommandLineArguments_getTrackRegistration(_arguments));
    dcGarbageCollector_setAlwaysGarbageCollect
        (dcTaffyCommandLineArguments_getAlwaysGarbageCollect(_arguments));

    dcGarbageCollector_addRoot(&dcSystem_mark);

    sSystem->filePackageDatas = dcHash_create();
    dcClassManager_create();

    sSystem->filePackageDatasMutex = dcMutex_create(false);
    sSystem->threadMutex = dcMutex_create(false);
    sSystem->threadCondition = dcCondition_create();
    sSystem->handles = dcList_create();
    sSystem->globalScope = dcScope_createNode();
    sSystem->nodeEvaluator = dcNodeEvaluator_create();
    sSystem->environmentVariables = dcHash_create();
    dcDecNumber_initialize();
    dcNumber_initialize();
    sSystem->registeredLibraryNames = dcList_create();
    sSystem->commandLineArguments = _arguments;
    sSystem->includeDirectories =
        dcTaffyCommandLineArguments_getIncludeDirectories(_arguments);
    sSystem->pluginDirectories =
        dcTaffyCommandLineArguments_getPluginDirectories(_arguments);
    sSystem->automaticFunctions = dcList_create();

    dcSystem_addAutomaticFunction("derive");
    dcSystem_addAutomaticFunction("integrate");
    dcSystem_addAutomaticFunction("simplify");

#ifdef TAFFY_WINDOWS
    dcSystem_addIncludeDirectory("");
#else
    dcSystem_addIncludeDirectory(".");
#endif

    //
    // <Taffy Home>
    //
    // first look in TAFFY_HOME file in current directory
    // then check $TAFFY_HOME global variable
    //
#ifdef TAFFY_WINDOWS
    FILE *taffyHomeFile = dcFileManagement_openFile("TAFFY_WINDOWS_HOME", "r");
#else
    FILE *taffyHomeFile = dcFileManagement_openFile("TAFFY_HOME", "r");
#endif

    if (taffyHomeFile == NULL)
    {
        sSystem->taffyHome = getenv("TAFFY_HOME");

        if (sSystem->taffyHome != NULL)
        {
            sSystem->taffyHome = dcMemory_strdup(sSystem->taffyHome);
            dcList_unshift
                (sSystem->includeDirectories,
                 dcString_createNodeWithString(sSystem->taffyHome, true));
        }
    }
    else
    {
        dcString *firstLine = dcFileManagement_getLine(taffyHomeFile);

        if (firstLine != NULL)
        {
            sSystem->taffyHome = dcMemory_strdup(firstLine->string);
            dcList_unshift(sSystem->includeDirectories,
                           dcString_createShell(firstLine));
        }

        dcFileManagement_closeFile(taffyHomeFile);
    }
    // </Taffy Home>

    // enable or disable memory freeing (for testing)
    dcMemory_setEnabled
        (! dcTaffyCommandLineArguments_getDisableFree(_arguments));
    dcMemory_setTrackCreations
        (dcTaffyCommandLineArguments_getTrackCreations(_arguments));

    dcGarbageCollector_execute();

    //
    // wait for the thread to come up and create its node evaluator
    // we need to wait for this so that it syncs correctly with the memory
    // regions system that's used later while parsing
    //
    dcMutex_lock(sSystem->upMutex);

    while (! sSystem->up)
    {
        dcCondition_wait(sSystem->upCondition, sSystem->upMutex);
    }

    dcMutex_unlock(sSystem->upMutex);
    // ^^ yay it's up!

    return (dcSystem *)(dcNodeEvaluator_synchronizeFunctionCall
                        (sSystem->nodeEvaluator,
                         &runSynchronizedCreationTasks,
                         _arguments));
}

uint32_t dcSystem_getNodeEvaluatorCount(void)
{
    return sSystem->evaluators->size;
}

bool dcSystem_currentThreadHasNodeEvaluator(void)
{
    dcNode *selfId = dcThread_getSelf();
    bool result = (sSystem->evaluators != NULL
                   && (dcHash_getValue(sSystem->evaluators,
                                       selfId,
                                       NULL)
                       == TAFFY_SUCCESS));
    dcNode_free(&selfId, DC_DEEP);
    return result;
}

dcNodeEvaluator *dcSystem_getCurrentNodeEvaluator(void)
{
    dcNode *evaluatorNode;
    dcReadWriteLock_lockForRead(sSystem->evaluatorsLock);

    if (sSystem->evaluators->size == 2)
    {
        // the GC won't be calling this, we can safely return the system's
        // node evaluator
        dcReadWriteLock_unlock(sSystem->evaluatorsLock);
        return sSystem->nodeEvaluator;
    }

    {
        dcTaffyThreadId selfId = dcThread_getSelfId();

        dcError_check(sSystem != NULL,
                      "System is not initialized, "
                      "there is no current node evaluator.");

        if (dcHash_getValueWithKeys(sSystem->evaluators,
                                    NULL,
                                    (dcHashType)selfId,
                                    &evaluatorNode)
            != TAFFY_SUCCESS)
        {
            fprintf(stderr, "Fatal error: no node evaluator for this thread");
            assert(false);
        }

        dcReadWriteLock_unlock(sSystem->evaluatorsLock);
    }

    return (CAST_NODE_EVALUATOR(evaluatorNode));
}

void dcSystem_free(void)
{
    if (sSystem == NULL)
    {
        return;
    }

    // wait for the threads to finish
    dcMutex_lock(sSystem->threadMutex);

    while (sSystem->threads->size > 0)
    {
        dcCondition_wait(sSystem->threadCondition, sSystem->threadMutex);
    }

    dcList_clear(sSystem->threads, DC_SHALLOW);

    dcMutex_unlock(sSystem->threadMutex);
    // all the threads are finished now

    // stop the garbage collection thread
    dcGarbageCollector_stop();
    sSystem->stopping = true;
    dcFlatArithmetic_deinitialize();

    bool displayGarbageCollectorStatistics =
        dcTaffyCommandLineArguments_displayGarbageCollectorStatistics
        (sSystem->commandLineArguments);

    // free the stacks etc //
    dcCommandLineArguments_free(&sSystem->commandLineArguments);
    dcNodeEvaluator_free(&sSystem->nodeEvaluator);
    dcScope_clear(CAST_SCOPE(sSystem->globalScope), DC_SHALLOW);

    //
    // deinitialize the classes as appropriate
    //
    const dcTaffy_getTemplatePointer *finger = gTaffyBootstrapClasses;

    for (finger = gTaffyBootstrapClasses; *finger != NULL; finger++)
    {
        const dcClassTemplate *that = (*finger)();

        // call the deinitializer //
        if (that->cTemplate != NULL
            && that->cTemplate->deinitializer != NULL)
        {
            that->cTemplate->deinitializer();
        }
    }

    dcHash_free(&sSystem->environmentVariables, DC_DEEP);

    dcHashIterator *iterator =
        dcHash_createIterator(sSystem->evaluators);
    dcNode *evaluator = NULL;

    while ((evaluator = dcHashIterator_getNextValue(iterator))
           != NULL)
    {
        dcNodeEvaluator_clearException(CAST_NODE_EVALUATOR(evaluator),
                                       DC_SHALLOW);
        dcNodeEvaluator_clearReturnValue(CAST_NODE_EVALUATOR(evaluator));
    }

    dcHashIterator_free(&iterator);
    dcNode_free(&sSystem->globalScope, DC_DEEP);
    dcHash_free(&sSystem->filePackageDatas, DC_DEEP);
    dcMutex_free(&sSystem->filePackageDatasMutex);
    dcClassManager_clearClassTemplates();
    dcGarbageCollector_freeWithStatistics(displayGarbageCollectorStatistics);
    dcFutureManager_free();
    dcClassManager_free();
    dcStringManager_free();
    dcList_free(&sSystem->threads, DC_DEEP);
    assert(sSystem->evaluators->size == 0);
    dcHash_free(&sSystem->evaluators, DC_DEEP);
    dcNumber_deinitialize();
    dcDecNumber_deinitialize();
    dcLexer_cleanup();
    dcList_free(&sSystem->registeredLibraryNames, DC_DEEP);
    dcMemory_free(sSystem->taffyHome);
    dcList_free(&sSystem->automaticFunctions, DC_DEEP);

    dcMutex_free(&sSystem->threadMutex);
    dcReadWriteLock_free(&sSystem->evaluatorsLock);
    dcCondition_free(&sSystem->threadCondition);
    dcMutex_free(&sSystem->upMutex);
    dcCondition_free(&sSystem->upCondition);

#ifndef TAFFY_WINDOWS
    // dlclose each handle
    while (sSystem->handles->size > 0)
    {
        dcNode *top = dcList_pop(sSystem->handles, DC_SHALLOW);
        dlclose(CAST_VOID(top));
        dcNode_free(&top, DC_SHALLOW);
    }
#endif

    dcList_free(&sSystem->handles, DC_DEEP);
    dcMemory_free(sSystem);
    sSystem = NULL;
}

bool dcSystem_isLive(void)
{
    return (sSystem != NULL);
}

void dcSystem_mark(void)
{
    dcHash_mark(sSystem->evaluators);

    // mark the containers
    dcMutex_lock(sSystem->threadMutex);
    dcList_mark(sSystem->threads);
    dcMutex_unlock(sSystem->threadMutex);
    dcHash_mark(sSystem->environmentVariables);

    // mark the global objects
    dcScope_mark(CAST_SCOPE(sSystem->globalScope));
}

void dcSystem_addHandle(void *handle)
{
    dcList_push(sSystem->handles, dcVoid_createNode(handle));
}

//
// Thread registering and unregistering
//
void dcSystem_registerThread(dcNode *_thread)
{
    dcMutex_lock(sSystem->threadMutex);
    dcList_push(sSystem->threads, _thread);
    dcMutex_unlock(sSystem->threadMutex);
}

void dcSystem_unregisterThread(dcNode *_thread)
{
    dcMutex_lock(sSystem->threadMutex);
    assert(dcList_removeWithComparisonFunction(sSystem->threads,
                                                       _thread,
                                                       DC_SHALLOW,
                                                       &dcNode_comparePointers)
                   == TAFFY_SUCCESS);
    dcCondition_signal(sSystem->threadCondition);
    dcMutex_unlock(sSystem->threadMutex);
}

bool dcSystem_libraryRegistered(const char *_libraryName)
{
    bool result = false;

    FOR_EACH_IN_LIST(sSystem->registeredLibraryNames, that)
    {
        dcString *libraryNameString = CAST_STRING(that->object);

        if (strcmp(libraryNameString->string, _libraryName) == 0)
        {
            result = true;
            break;
        }
    }

    return result;
}

dcNodeEvaluator *dcSystem_getNodeEvaluator(void)
{
    return sSystem->nodeEvaluator;
}

dcNode *dcSystem_getGlobalScope(void)
{
    return sSystem->globalScope;
}

void dcSystem_printGlobalScope(void)
{
    char *display = dcSystem_displayGlobalScope();
    printf("%s\n", display);
    dcMemory_free(display);
}

char *dcSystem_displayGlobalScope(void)
{
    return dcLexer_sprintf
        ("--dcSystem printing global scope--\n"
         "%s"
         "--dcSystem done printing global scope--",
         dcNode_display(sSystem->globalScope));
}

const dcList *dcSystem_getIncludeDirectories(void)
{
    return sSystem->includeDirectories;
}

const dcList *dcSystem_getPluginDirectories(void)
{
    return sSystem->pluginDirectories;
}

const dcList *dcSystem_getArguments(void)
{
    return (sSystem->commandLineArguments != NULL
            ? (dcTaffyCommandLineArguments_getArguments
               (sSystem->commandLineArguments))
            : NULL);
}

dcHash *dcSystem_getEnvironmentVariables(void)
{
    return sSystem->environmentVariables;
}

void dcSystem_setEnvironmentVariable(const char *_key, const char *_value)
{
}

char *dcSystem_getEnvironmentVariable(const char *_name)
{
    char **localEnviron = NULL;
    char *result = NULL;

    if (localEnviron != NULL)
    {
        result = getenv(_name);
    }

    return result;
}

dcResult dcSystem_refreshEnvironmentVariables(void)
{
    char **localEnviron = NULL;
    dcResult result = TAFFY_FAILURE;

    /////////////////////////////////////
    //
    // Set up the environment variables
    //
    //   example
    //
    //   if the environment variable is:
    //
    //   PATH = /my/home
    //
    //   then left = "PATH"
    //   and right = "/my/home"
    //
    /////////////////////////////////////

    if (localEnviron != NULL)
    {
        size_t i = 0;
        dcHash_clear(sSystem->environmentVariables, DC_DEEP);
        result = TAFFY_SUCCESS;

        for (i = 0; localEnviron[i] != NULL; i++)
        {
            dcList *values = dcLexer_splitString(localEnviron[i], '=');

            if (values->size != 2)
            {
                dcWarning("failure during environment set");
                result = TAFFY_EXCEPTION;
                break;
            }

            dcNode *leftString = dcList_getHead(values);

            dcHash_setValue(sSystem->environmentVariables,
                            leftString,
                            dcPair_createNode
                            (leftString, dcList_get(values, 1)));

            dcList_free(&values, DC_SHALLOW);
        }
    }

    return result;
}

void dcSystem_addIncludeDirectory(const char *_directory)
{
    dcList_push(sSystem->includeDirectories,
                dcString_createNodeWithString(_directory, true));
}

struct dcOperatorPair_t
{
    const char *symbol;
    const char *operatorName;
};

typedef struct dcOperatorPair_t dcOperatorPair;

#define ARGUMENT_OPERATOR_PAIR(operator)        \
    #operator, "#operator("#operator"):"

#define OPERATOR_PAIR(operator)                 \
    #operator, "#operator("#operator")"

#define PREFIX_OPERATOR_PAIR(operator)          \
    #operator, "#prefixOperator("#operator")"

//
// the string definitions for the dcTaffyOperator enum in dcDefines.h
//
const dcOperatorPair sOperatorPairs[] =
{
    {ARGUMENT_OPERATOR_PAIR(+)},
    {ARGUMENT_OPERATOR_PAIR(-)},
    {ARGUMENT_OPERATOR_PAIR(*)},
    {ARGUMENT_OPERATOR_PAIR(/)},
    {ARGUMENT_OPERATOR_PAIR(^)},
    {ARGUMENT_OPERATOR_PAIR(%)},
    {ARGUMENT_OPERATOR_PAIR(<)},
    {ARGUMENT_OPERATOR_PAIR(<=)},
    {ARGUMENT_OPERATOR_PAIR(>)},
    {ARGUMENT_OPERATOR_PAIR(>=)},
    {ARGUMENT_OPERATOR_PAIR(<<)},
    {ARGUMENT_OPERATOR_PAIR(<<=)},
    {ARGUMENT_OPERATOR_PAIR(>>)},
    {ARGUMENT_OPERATOR_PAIR(>>=)},
    {"and", "and:"},
    {"or",  "or:"},
    {ARGUMENT_OPERATOR_PAIR(==)},
    {OPERATOR_PAIR(++)},
    {OPERATOR_PAIR(--)},
    {ARGUMENT_OPERATOR_PAIR(&)},
    {ARGUMENT_OPERATOR_PAIR(&=)},
    {ARGUMENT_OPERATOR_PAIR(|)},
    {ARGUMENT_OPERATOR_PAIR(|=)},
    {ARGUMENT_OPERATOR_PAIR(^^)},
    {ARGUMENT_OPERATOR_PAIR(^^=)},
    {PREFIX_OPERATOR_PAIR(~)},
    {ARGUMENT_OPERATOR_PAIR(~=)},
    {ARGUMENT_OPERATOR_PAIR(+=)},
    {ARGUMENT_OPERATOR_PAIR(-=)},
    {ARGUMENT_OPERATOR_PAIR(*=)},
    {ARGUMENT_OPERATOR_PAIR(/=)},
    {ARGUMENT_OPERATOR_PAIR(^=)},
    {ARGUMENT_OPERATOR_PAIR(%=)},
    {ARGUMENT_OPERATOR_PAIR(())},
    {ARGUMENT_OPERATOR_PAIR([])},
    {ARGUMENT_OPERATOR_PAIR([]=)},
    {ARGUMENT_OPERATOR_PAIR([...])},
    {ARGUMENT_OPERATOR_PAIR([...]=)},
    {PREFIX_OPERATOR_PAIR(-)},
    {PREFIX_OPERATOR_PAIR(+)},
    {OPERATOR_PAIR(!)},
    {"?", "?"}, // unknown operator
};

const int sOperatorSizeChecker[(dcTaffy_countOf(sOperatorPairs)
                                == TAFFY_LAST_OPERATOR)
                               ? 1
                               : -1] = {0};

const char *dcSystem_getOperatorName(dcTaffyOperator _type)
{
    assert(_type < sizeof(sOperatorPairs) / sizeof(dcOperatorPair));
    return sOperatorPairs[_type].operatorName;
}

const char *dcSystem_getOperatorSymbol(dcTaffyOperator _type)
{
    assert(_type < sizeof(sOperatorPairs) / sizeof(dcOperatorPair));
    return sOperatorPairs[_type].symbol;
}

void dcSystem_propagateFlagToEvaluators(bool _yesno, uint16_t _flag)
{
    dcGarbageCollector_lock();
    dcHashIterator *i = dcHash_createIterator(sSystem->evaluators);
    dcNode *node = NULL;

    // send the flags to all running node evaluators!
    while ((node = dcHashIterator_getNextValue(i))
           != NULL)
    {
        dcNodeEvaluator_setFlag(CAST_NODE_EVALUATOR(node), _yesno, _flag);
    }

    dcHashIterator_free(&i);
    dcGarbageCollector_unlock();
}

void dcSystem_abortExecution(int _signal)
{
    dcGarbageCollector_lock();
    dcHashIterator *i = dcHash_createIterator(sSystem->evaluators);
    dcNode *node = NULL;

    // send an abort signal to all running node evaluators!
    while ((node = dcHashIterator_getNextValue(i))
           != NULL)
    {
        dcNodeEvaluator_abortExecutionFromSignal
            (CAST_NODE_EVALUATOR(node), _signal);
    }

    dcHashIterator_free(&i);
    dcGarbageCollector_unlock();
}

void dcSystem_exit(void)
{
    dcGarbageCollector_lock();
    dcHashIterator *i = dcHash_createIterator(sSystem->evaluators);
    dcNode *node = NULL;

    while ((node = dcHashIterator_getNextValue(i))
           != NULL)
    {
        dcNodeEvaluator_setExiting(CAST_NODE_EVALUATOR(node));
    }

    dcHashIterator_free(&i);
    dcGarbageCollector_unlock();
}

void dcSystem_addNodeEvaluator(dcNodeEvaluator *_evaluator)
{
    // this is called at the start of a thread, so the GC won't
    // be aware of it yet. we must synchronize with the GC here
    dcGarbageCollector_lock();
    dcReadWriteLock_lockForWrite(sSystem->evaluatorsLock);
    dcNode *shell = dcNodeEvaluator_createShell(_evaluator);

    //
    // assert that the node evaluator isn't already registered
    //
    size_t preSize = sSystem->evaluators->size;
    dcHash_setValueWithHashValue(sSystem->evaluators,
                                 NULL,
                                 (dcHashType)_evaluator->simpleThreadId,
                                 shell);
    assert(sSystem->evaluators->size == preSize + 1);
    dcReadWriteLock_unlock(sSystem->evaluatorsLock);
    dcGarbageCollector_unlock();
}

void dcSystem_removeNodeEvaluator(dcNodeEvaluator *_evaluator)
{
    dcGarbageCollector_lock();
    dcNode *removed = NULL;
    assert(dcHash_removeValueWithHashValue
           (sSystem->evaluators,
            NULL,
            (dcHashType)_evaluator->simpleThreadId,
            &removed,
            DC_SHALLOW)
           == TAFFY_SUCCESS);
    dcNode_freeShell(&removed);
    dcGarbageCollector_unlock();
}

bool dcSystem_isInBootstrap(void)
{
    return sSystem->bootstrap;
}

dcNode *dcSystem_convertBoolToNode(bool _yesno)
{
    return (_yesno
            ? dcYesClass_getInstance()
            : dcNoClass_getInstance());
}

dcClassTemplate *dcSystem_getAbortExceptionClassTemplate(void)
{
    return sSystem->abortExceptionClassTemplate;
}

void dcSystem_removeFilePackageData(dcStringId _filenameId)
{
    dcNode *key = dcUnsignedInt32_createNode(_filenameId);
    assert(dcHash_removeValue(sSystem->filePackageDatas,
                              key,
                              NULL,
                              DC_DEEP)
           == TAFFY_SUCCESS);
    dcNode_free(&key, DC_DEEP);
}

// this function creates a file package data if it doesn't already exist
dcNode *dcSystem_getFilePackageData(dcStringId _filenameId)
{
    dcNode *key = dcUnsignedInt32_createNode(_filenameId);
    dcNode *result = NULL;
    dcMutex_lock(sSystem->filePackageDatasMutex);

    //
    // first check if a file package data already exists
    //
    assert(dcHash_getValue(sSystem->filePackageDatas,
                           key,
                           &result)
           != TAFFY_EXCEPTION);

    if (result == NULL)
    {
        //
        // one doesn't already exist, create a new one
        //
        result = dcFilePackageData_createNode();
        assert(dcHash_setValue(sSystem->filePackageDatas,
                               key,
                               result)
               == TAFFY_SUCCESS);
    }
    else
    {
        dcNode_free(&key, DC_DEEP);
    }

    dcMutex_unlock(sSystem->filePackageDatasMutex);
    return result;
}

bool dcSystem_filePackageDataExists(const char *_filename)
{
    dcNode *key = dcUnsignedInt32_createNode
        (dcStringManager_getStringId(_filename));
    dcNode *dummy = NULL;
    bool result = (dcHash_getValue(sSystem->filePackageDatas,
                                   key,
                                   &dummy)
                   == TAFFY_SUCCESS);
    dcNode_free(&key, DC_DEEP);
    return result;
}

dcList *dcSystem_getPackageContentsFromGraphDataNode(dcNode *_node)
{
    return (CAST_FILE_PACKAGE_DATA
            (dcSystem_getFilePackageData
             (CAST_GRAPH_DATA(_node)->filenameId))
            ->packageContents);
}

bool dcSystem_isDebugEnabled(void)
{
#ifdef ENABLE_DEBUG
    return true;
#else
    return false;
#endif
}

void dcSystem_resetMarkCount(void)
{
    sSystem->markCount = 0;
}

void dcSystem_logAndResetMarkCount(const char *_name)
{
    dcLog_log(GARBAGE_COLLECTOR_LOG,
              "[%s] marked %u\n",
              _name,
              sSystem->markCount);
    sSystem->markCount = 0;
}

void dcSystem_nodeWasMarked(void)
{
    // test code may not always initialize dcSystem
    if (sSystem != NULL)
    {
        sSystem->markCount++;
    }
}

void dcSystem_printBytes(const uint8_t *_bytes,
                         uint32_t _size)
{
    uint32_t i;

    for (i = 0; i < _size; i++)
    {
        printf("%X ", _bytes[i]);
    }

    printf("\n");

    for (i = 0; i < _size; i++)
    {
        printf("%X ", _bytes[i]);
    }

    printf("\n");
}

bool dcSystem_silencePluginWarnings(void)
{
    return dcTaffyCommandLineArguments_silencePluginWarnings
        (sSystem->commandLineArguments);
}

void dcSystem_garbageCollectorIsUp(void)
{
    dcMutex_lock(sSystem->upMutex);
    sSystem->up = true;
    dcCondition_signal(sSystem->upCondition);
    dcMutex_unlock(sSystem->upMutex);
}

void dcSystem_displayVersion(const char *_programName)
{
    dcIOClass_printFormat("%s %s", _programName, __compiledTaffyVersion);
}

void dcSystem_addAutomaticFunction(const char *_name)
{
    dcList_push(sSystem->automaticFunctions,
                dcString_createNodeWithString(_name, true));
}

bool dcSystem_isAutomaticFunction(const char *_name)
{
#ifdef TAFFY_TEST
    if (sSystem == NULL)
    {
        return false;
    }
#endif

    FOR_EACH_IN_LIST(sSystem->automaticFunctions, that)
    {
        if (dcString_equalsCharArray(CAST_STRING(that->object), _name))
        {
            return true;
        }
    }

    return false;
}
