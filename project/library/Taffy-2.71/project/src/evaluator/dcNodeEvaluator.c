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
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>

// evaluation //
#include "dcCallStackData.h"
#include "dcFileEvaluator.h"
#include "dcNodeEvaluator.h"

// misc //
#include "dcContainers.h"
#include "dcError.h"
#include "dcFilePackageData.h"
#include "dcFileManagement.h"
#include "dcFutureManager.h"
#include "dcGraphDatas.h"
#include "dcLog.h"
#include "dcNode.h"
#include "dcStringManager.h"
#include "dcSystem.h"

// threading
#include "dcMutex.h"
#include "dcThread.h"

// maths
#include "dcMatrix.h"
#include "dcNumber.h"
#include "dcUnsignedInt32.h"

// memory management //
#include "dcGarbageCollector.h"
#include "dcMemory.h"

// scope //
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcObjectStack.h"
#include "dcObjectStackList.h"

// class //
#include "dcArrayClass.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcFunctionClass.h"
#include "dcFutureClass.h"
#include "dcHashClass.h"
#include "dcHeapClass.h"
#include "dcIOClass.h"
#include "dcKernelClass.h"
#include "dcMainClass.h"
#include "dcMatrixClass.h"
#include "dcMutexClass.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNumberClass.h"
#include "dcPairClass.h"
#include "dcProcedureClass.h"
#include "dcStringClass.h"
#include "dcYesClass.h"

// special class
#include "dcWildClass.h"
#include "dcSuppliedArgumentClass.h"

// exceptions //
#include "dcExceptionClass.h"
#include "dcExceptions.h"

#include "dcPackageContents.h"

#ifndef TAFFY_WINDOWS
    #include <dlfcn.h>
#endif

static const dcNodeEvaluator_evaluateHelperPointer sEvaluateHelpers[] =
{
    &dcNodeEvaluator_evaluateAnd,
    &dcNodeEvaluator_evaluateAssignment,
    &dcNodeEvaluator_evaluateBreak,
    NULL, // catch block
    &dcNodeEvaluator_evaluateClass,
    &dcNodeEvaluator_evaluateExit,
    &dcNodeEvaluator_evaluateFalse,
    &dcNodeEvaluator_evaluateFlatArithmetic,
    &dcNodeEvaluator_evaluateFor,
    &dcNodeEvaluator_evaluateFunctionUpdate,
    &dcNodeEvaluator_evaluateGraphDataList,
    NULL, // graph data node
    NULL, // graph data pair
    &dcNodeEvaluator_evaluateGraphDataTree,
    &dcNodeEvaluator_evaluateIdentifier,
    &dcNodeEvaluator_evaluateIf,
    &dcNodeEvaluator_evaluateImport,
    &dcNodeEvaluator_evaluateIn,
    &dcNodeEvaluator_evaluateMethodCall,
    NULL, // method header
    &dcNodeEvaluator_evaluateNew,
    &dcNodeEvaluator_evaluateNil,
    &dcNodeEvaluator_evaluateNotEqual,
    &dcNodeEvaluator_evaluateOr,
    &dcNodeEvaluator_evaluatePackage,
    &dcNodeEvaluator_evaluateReturn,
    &dcNodeEvaluator_evaluateSelf,
    &dcNodeEvaluator_evaluateSuper,
    &dcNodeEvaluator_evaluateSymbol,
    &dcNodeEvaluator_evaluateSynchronized,
    &dcNodeEvaluator_evaluateThrow,
    &dcNodeEvaluator_evaluateTrue,
    &dcNodeEvaluator_evaluateTryBlock,
    &dcNodeEvaluator_evaluateUpSelf,
    &dcNodeEvaluator_evaluateWhile
};

GRAPH_DATA_SIZE_CHECKER(sEvaluateHelpers);

static uint32_t sSimpleId = 0;

dcNodeEvaluator *dcNodeEvaluator_create(void)
{
    dcNodeEvaluator *evaluator =
        (dcNodeEvaluator *)(dcMemory_allocateAndInitialize
                            (sizeof(dcNodeEvaluator)));
    dcObjectStack *stack = dcObjectStack_create();

    evaluator->callStack = dcList_create();
    evaluator->objectStackList = dcObjectStackList_create();

    // set the max stack depth, a bit arbitrarily //
    evaluator->maxStackDepth = 400;
    evaluator->evaluatedToMark = dcList_create();

    evaluator->threadId = dcThread_getSelf();
    evaluator->simpleThreadId = dcThread_getSelfId();
    evaluator->simpleId = sSimpleId++;

    evaluator->deferredImports = dcList_create();
    evaluator->importReferences = dcList_create();
    evaluator->importReferencesCallStacks = dcHash_create();
    evaluator->asStringObjects = dcHash_create();
    evaluator->readLockedObjects = dcHash_create();
    evaluator->writeLockedObjects = dcHash_create();
    evaluator->evaluatorMutex = dcMutex_create(true);

    // add the initial local scope //
    dcObjectStack_pushScope(stack, dcScope_createNode());
    dcObjectStackList_pushObjectStack(evaluator->objectStackList, stack);

    if (! dcSystem_isInBootstrap())
    {
        dcNodeEvaluator_initializeSelf(evaluator);
    }

    TAFFY_DEBUG(evaluator->upCounter = 0);

    dcSystem_addNodeEvaluator(evaluator);
    return evaluator;
}

dcNode *dcNodeEvaluator_createShell(dcNodeEvaluator *_evaluator)
{
    return dcNode_createWithGuts(NODE_EVALUATOR, _evaluator);
}

void dcNodeEvaluator_freeNode(dcNode *_node, dcDepth _depth)
{
    dcNodeEvaluator_free(&(CAST_NODE_EVALUATOR(_node)));
}

dcTaffy_createMarkNodeFunctionMacro(dcNodeEvaluator, CAST_NODE_EVALUATOR);

static void clearImportReferences(dcNodeEvaluator *_evaluator)
{
    while (_evaluator->importReferences->size > 0)
    {
        dcNode *top = dcList_pop(_evaluator->importReferences, DC_SHALLOW);
        dcNode_freeShell(&top);
    }
}

void dcNodeEvaluator_free(dcNodeEvaluator **_evaluator)
{
    dcNodeEvaluator *evaluator = *_evaluator;
    dcSystem_removeNodeEvaluator(evaluator);

    dcNodeEvaluator_clearException(evaluator, DC_SHALLOW);
    dcList_free(&evaluator->evaluatedToMark, DC_SHALLOW);

    // delete the scopes
    dcError_assert(evaluator->objectStackList->objectStacks->size == 1);
    dcObjectStack *stack = dcObjectStackList_popObjectStack
        (evaluator->objectStackList, DC_SHALLOW);
    // delete the initial local scope
    dcObjectStack_popScope(stack, DC_DEEP);
    dcObjectStackList_free(&evaluator->objectStackList);
    dcObjectStack_free(&stack, DC_DEEP);

    dcList_free(&evaluator->deferredImports, DC_DEEP);

    clearImportReferences(evaluator);
    dcList_free(&evaluator->importReferences, DC_DEEP);
    dcHash_free(&evaluator->importReferencesCallStacks, DC_DEEP);
    dcHash_free(&evaluator->asStringObjects, DC_SHALLOW);
    dcHash_free(&evaluator->readLockedObjects, DC_SHALLOW);
    dcHash_free(&evaluator->writeLockedObjects, DC_SHALLOW);
    dcMutex_free(&evaluator->evaluatorMutex);
    dcNode_free(&evaluator->threadId, DC_DEEP);

    dcList_free(&evaluator->callStack, DC_SHALLOW);
    dcList_free(&evaluator->exceptionCallStack, DC_SHALLOW);
    dcMemory_free(*_evaluator);
}

void dcNodeEvaluator_setPosition(dcNodeEvaluator *_evaluator,
                                 dcNode *_graphDataNode)
{
    dcGraphData *graphData = CAST_GRAPH_DATA(_graphDataNode);

    if (graphData->lineNumber > 0 && graphData->filenameId > 0)
    {
        _evaluator->lineNumber = graphData->lineNumber;
        _evaluator->filenameId = graphData->filenameId;
    }
}

static void lock(dcNodeEvaluator *_evaluator)
{
    dcMutex_lock(_evaluator->evaluatorMutex);
}

static void unlock(dcNodeEvaluator *_evaluator)
{
    dcMutex_unlock(_evaluator->evaluatorMutex);
}

dcList *dcNodeEvaluator_getPackageContents(dcNodeEvaluator *_evaluator)
{
    return dcNodeEvaluator_getCurrentFilePackageData
        (_evaluator)->packageContents;
}

uint32_t dcNodeEvaluator_getEvaluatedToMarkSize
    (const dcNodeEvaluator *_evaluator)
{
    return _evaluator->evaluatedToMark->size;
}

// may be used by tests
void bringUp(dcNodeEvaluator *_evaluator)
{
    TAFFY_DEBUG(assert(_evaluator->upCounter >= 0));

    if (_evaluator->evaluationCount == 0)
    {
        // we're back from the dead
        dcGarbageCollector_nodeEvaluatorBlockUp();
    }

    _evaluator->evaluationCount++;
    dcGarbageCollector_blockNodeEvaluator();

    TAFFY_DEBUG(_evaluator->upCounter++);
}

// may be used by tests
void takeDown(dcNodeEvaluator *_evaluator)
{
    TAFFY_DEBUG(assert(_evaluator->upCounter > 0));

    _evaluator->evaluationCount--;
    dcGarbageCollector_blockNodeEvaluator();

    if (_evaluator->evaluationCount == 0)
    {
        dcGarbageCollector_nodeEvaluatorDown();
    }

    TAFFY_DEBUG(_evaluator->upCounter--);
}

void dcNodeEvaluator_setReturnValue(dcNodeEvaluator *_evaluator,
                                    dcNode *_returnValue)
{
    TAFFY_DEBUG(if (_returnValue != NULL)
                {
                    dcError_assert(! dcNode_isContainer(_returnValue));
                });

    _evaluator->returnValue = _returnValue;
}

void dcNodeEvaluator_clearReturnValue(dcNodeEvaluator *_evaluator)
{
    bringUp(_evaluator);
    _evaluator->returnValue = NULL;
    takeDown(_evaluator);
}

dcNode *dcNodeEvaluator_getReturnValue(dcNodeEvaluator *_evaluator)
{
    return _evaluator->returnValue;
}

void dcNodeEvaluator_clearCallStack(dcNodeEvaluator *_evaluator)
{
    dcList_clear(_evaluator->callStack, DC_SHALLOW);
}

dcScope *dcNodeEvaluator_getCurrentScope(dcNodeEvaluator *_evaluator,
                                         uint16_t _scopeFlags)
{
    return dcObjectStackList_getTailScope(_evaluator->objectStackList);
}

const char *dcNodeEvaluator_getFilename(dcNodeEvaluator *_evaluator)
{
    return dcStringManager_getStringFromId(_evaluator->filenameId);
}

bool dcNodeEvaluator_isBreaking(dcNodeEvaluator *_evaluator)
{
    return READ_BITS(_evaluator->flags, NODE_EVALUATOR_BREAK);
}

void dcNodeEvaluator_resetState(dcNodeEvaluator *_evaluator)
{
    _evaluator->abortExecutionState = NODE_EVALUATOR_ABORT_NOT;
    _evaluator->abortExecutionSignal = 0;
}

void dcNodeEvaluator_setFlag(dcNodeEvaluator *_evaluator,
                             bool _yesno,
                             int _value)
{
    lock(_evaluator);
    SET_BITS(_evaluator->flags, _value, _yesno);
    unlock(_evaluator);
}

void dcNodeEvaluator_setExiting(dcNodeEvaluator *_evaluator)
{
    _evaluator->exit = true;
    dcNodeEvaluator_setFlag(_evaluator, true, NODE_EVALUATOR_EXIT);
}

void dcNodeEvaluator_setReturning(dcNodeEvaluator *_evaluator, bool _yesno)
{
    dcNodeEvaluator_setFlag(_evaluator, _yesno, NODE_EVALUATOR_RETURN);
}

void dcNodeEvaluator_setBreaking(dcNodeEvaluator *_evaluator, bool _yesno)
{
    dcNodeEvaluator_setFlag(_evaluator, _yesno, NODE_EVALUATOR_BREAK);
}

void dcNodeEvaluator_abortExecutionFromSignal(dcNodeEvaluator *_evaluator,
                                              int _signal)
{
    _evaluator->abortExecutionState = NODE_EVALUATOR_ABORT_RECEIVED;
    _evaluator->abortExecutionSignal = _signal;
}

bool dcNodeEvaluator_abortReceived(dcNodeEvaluator *_evaluator)
{
    lock(_evaluator);
    bool result = ((_evaluator->abortExecutionState
                    == NODE_EVALUATOR_ABORT_RECEIVED)
                   || (_evaluator->abortExecutionState
                       == NODE_EVALUATOR_ABORT_PROCESSING)
                   || (_evaluator->abortExecutionState
                       == NODE_EVALUATOR_ABORT_PLEASE));
    unlock(_evaluator);
    return result;
}

typedef struct
{
    uint32_t flag;
    const char *string;
}  FlagString;

static const FlagString flagStrings[] =
{
    {NODE_EVALUATOR_RETURN,         "NODE_EVALUATOR_RETURN"},
    {NODE_EVALUATOR_BREAK,          "NODE_EVALUATOR_BREAK"},
    {NODE_EVALUATOR_EXIT,           "NODE_EVALUATOR_EXIT"}
};

// debugging hook
void dcNodeEvaluator_printFlags(const dcNodeEvaluator *_evaluator)
{
    bool printed = false;
    size_t i;

    for (i = 0; i < dcTaffy_countOf(flagStrings); i++)
    {
        if ((_evaluator->flags & flagStrings[i].flag) != 0)
        {
            printed = true;
            printf("%s\n", flagStrings[i].string);
        }
    }

    if (! printed)
    {
        printf("None\n");
    }
}

static dcNode *previousPrint = NULL;

static void processAbort(dcNodeEvaluator *_evaluator)
{
    const char *abortString = NULL;

#ifdef TAFFY_WINDOWS
    abortString = "User generated abort";
#else
    abortString = strsignal(_evaluator->abortExecutionSignal);
#endif

    _evaluator->abortExecutionState = NODE_EVALUATOR_ABORT_PROCESSING;
    dcUserGeneratedAbortSignalExceptionClass_throwObject(abortString);
    assert(_evaluator->exception != NULL);
    _evaluator->abortExecutionState = NODE_EVALUATOR_ABORT_PLEASE;
}

//
// dcNodeEvaluator_evaluate()
//
// The main entry point to evaluating a tree
//
dcNode *dcNodeEvaluator_evaluate(dcNodeEvaluator *_evaluator, dcNode *_node)
{
    dcNode *result = NULL;
    dcNode *nodeIterator = _node;
    uint32_t pushedCount = dcNodeEvaluator_pushMark(_evaluator, nodeIterator);
    dcError_assert(_node != NULL);
    bringUp(_evaluator);

    TAFFY_DEBUG(dcError_assert
                (dcSystem_isInBootstrap()
                 || dcNodeEvaluator_getCurrentSelf(_evaluator) != NULL));

    // try to process an abort
    if (_evaluator->exception == NULL
        && _evaluator->abortExecutionState == NODE_EVALUATOR_ABORT_RECEIVED
        && _evaluator->abortDelay == 0)
    {
        processAbort(_evaluator);
    }
    else if (dcNodeEvaluator_canContinueEvaluating(_evaluator))
    {
        while (nodeIterator != NULL)
        {
            pushedCount += dcNodeEvaluator_pushMark(_evaluator, nodeIterator);

            if (! dcSystem_isInBootstrap()
                && dcLog_isEnabled(NODE_EVALUATOR_LOG)
                && _node != previousPrint)
            {
                previousPrint = _node;

                dcLog_log(NODE_EVALUATOR_LOG,
                          "evaluating: [%s:%u] %s\n",
                          dcStringManager_getStringFromId
                          (_evaluator->filenameId),
                          _evaluator->lineNumber,
                          dcNode_display(nodeIterator));

                previousPrint = NULL;
            }

            // if it's a GraphData node, grab its line and filename //
            if (nodeIterator->type == NODE_GRAPH_DATA)
            {
                dcGraphDataType type = dcGraphData_getType(nodeIterator);

                // graph datas giveth a line number and filename
                dcNodeEvaluator_setPosition(_evaluator, nodeIterator);

                if (_evaluator->onlyEvaluateClasses > 0)
                {
                    // we're evaluating an import, so we only evaluate class-
                    // like-things until the import is complete

                    if ((type == NODE_CLASS
                         && ! dcClass_isObject(nodeIterator))
                        || type == NODE_IMPORT
                        || type == NODE_PACKAGE
                        || type == NODE_GRAPH_DATA_TREE)
                    {
                        result = sEvaluateHelpers[type]
                            (_evaluator, nodeIterator);
                    }
                    else
                    {
                        result = dcNilClass_getInstance();
                    }
                }
                else
                {
                    result = sEvaluateHelpers[type](_evaluator, nodeIterator);
                }

                if (result == NULL)
                {
                    // stooooooooooooooooooppppppppppp!!
                    nodeIterator = NULL;
                }
                else
                {
                    dcNodeEvaluator_setReturnValue(_evaluator, result);
                    // go to the next node in the tree
                    nodeIterator = dcGraphData_getNext(nodeIterator);
                }
            }
            else
            {
                // not interested //
                result = nodeIterator;
                break;
            }

            uint32_t markCount = dcNodeEvaluator_pushMark(_evaluator, result);

            if (! dcNodeEvaluator_canContinueEvaluating(_evaluator))
            {
                nodeIterator = NULL;
            }

            dcNodeEvaluator_popMarks(_evaluator, markCount);
        }
    }
    else
    {
        result = _evaluator->exception;
    }

    dcNodeEvaluator_popMarks(_evaluator, pushedCount);
    dcError_assert(_evaluator->evaluationCount > 0);
    takeDown(_evaluator);

#ifdef ENABLE_DEBUG
    if (_evaluator->evaluationCount == 0)
    {
        //
        // <sanity>
        //
        if (result == NULL)
        {
            dcError_assert(_evaluator->exception != NULL);
        }
        else if (_evaluator->exception != NULL)
        {
            dcError_assert(result == NULL);
        }
        // </sanity>
    }
#endif

    return result;
}

bool dcNodeEvaluator_hasException(dcNodeEvaluator *_evaluator)
{
    return (_evaluator->exception != NULL);
}

bool dcNodeEvaluator_iHaveAnException(void)
{
    return (dcNodeEvaluator_hasException(dcSystem_getCurrentNodeEvaluator()));
}

bool dcNodeEvaluator_canContinueEvaluating(dcNodeEvaluator *_evaluator)
{
    bool canContinue = true;
    lock(_evaluator);

    if (_evaluator->exception != NULL
        || READ_BITS(_evaluator->flags, NODE_EVALUATOR_RETURN)
        || READ_BITS(_evaluator->flags, NODE_EVALUATOR_EXIT)
        || READ_BITS(_evaluator->flags, NODE_EVALUATOR_BREAK)
        || _evaluator->abortExecutionState == NODE_EVALUATOR_ABORT_PLEASE)
    {
        canContinue = false;
    }

    unlock(_evaluator);
    return canContinue;
}

char *dcNodeEvaluator_displayExceptionCallStack(dcNodeEvaluator *_evaluator)
{
    dcString exceptionString;
    dcString_initialize(&exceptionString, 100);

    FOR_EACH_IN_LIST_REVERSE(_evaluator->exceptionCallStack, that)
    {
        // append the callstack display onto the exception string
        dcString_append(&exceptionString,
                        "%s%s",
                        dcNode_display(that->object),
                        (that->previous == NULL
                         ? ""
                         : "\n"));
    }

    return exceptionString.string;
}

static void *generateExceptionText(void *_evaluator)
{
    char *result = NULL;
    dcNodeEvaluator *evaluator = (dcNodeEvaluator *)_evaluator;

    if (dcNodeEvaluator_hasException(evaluator))
    {
        const char *display =
            dcStringClass_asString_helper(evaluator->exception);
        char *callStackDisplay =
            dcNodeEvaluator_displayExceptionCallStack(evaluator);

        if (dcClass_isKindOfTemplate(evaluator->exception,
                                     dcSystem_getAbortExceptionClassTemplate()))
        {
            result = dcLexer_sprintf("Execution aborted: %s%s%s",
                                     display,
                                     (strlen(callStackDisplay) > 0
                                      ? "\n"
                                      : ""),
                                     callStackDisplay);
        }
        else
        {
            result = dcLexer_sprintf("Uncaught Exception: %s%s%s",
                                     display,
                                     (strlen(callStackDisplay) > 0
                                      ? "\n"
                                      : ""),
                                     callStackDisplay);
        }

        dcMemory_free(callStackDisplay);
        dcNodeEvaluator_clearException(evaluator, DC_DEEP);
    }

    return result;
}

char *dcNodeEvaluator_generateExceptionText(dcNodeEvaluator *_evaluator)
{
    return ((char*)dcNodeEvaluator_synchronizeFunctionCall
            (_evaluator,
             &generateExceptionText,
             _evaluator));
}

static dcNode *findMultiScopedObject(dcNodeEvaluator *_evaluator,
                                     const dcList *_identifiers)
{
    dcListElement *that = _identifiers->head;
    bool crossedClassBoundary = false;
    bool first = true;
    dcNode *result = NULL;
    dcScope *currentScope =
        dcNodeEvaluator_getCurrentScope(_evaluator, NO_FLAGS);

    while (that != NULL)
    {
        const char *identifierString = dcString_getString(that->object);
        dcNode *scopeDataNode = NULL;

        if (first)
        {
            //
            // for the first time around, zero-in the first identifier.
            // once this is completed, the second search, and beyond,
            // can be executed using the currentScope
            //
            //   eg
            //     class TestClass { class PublicClass {} }
            //
            // TestClass may be in the local scope, may be global, etc,
            // but PublicClass is always in TestClass' scope (currentScope)
            //
            scopeDataNode = dcObjectStackList_getScopeDataForObject
                (_evaluator->objectStackList,
                 identifierString,
                 NULL);

            if (scopeDataNode == NULL)
            {
                result = NULL;
                break;
            }

            // just do this once //
            if (dcScope_getScopeDataForObject(currentScope, identifierString)
                == NULL)
            {
                crossedClassBoundary = true;
            }

            first = false;
        }
        else
        {
            scopeDataNode = dcScope_getScopeDataForObject
                (currentScope, identifierString);

            if (scopeDataNode == NULL)
            {
                result = NULL;
                break;
            }
        }

        // check for invalid access to private or protected members //
        if (crossedClassBoundary
            && ((dcScopeData_getFlags(scopeDataNode) & SCOPE_DATA_PROTECTED)
                != 0))
        {
            // uh oh //
            result = NULL;
            break;
        }

        result = dcScopeData_getObject(scopeDataNode);
        currentScope = dcClass_getUsedScope(result);
        that = that->next;
    }

    return result;
}

static dcNode *evaluateImport(dcNodeEvaluator *_evaluator,
                              const char *_packageName,
                              const char *_className,
                              const dcClassTemplate *_requestorTemplate,
                              bool _throwException);

static dcNode *evaluateImportFromCurrentPackage(dcNodeEvaluator *_evaluator,
                                                const char *_identifierName,
                                                bool _throwException)
{
    char *pathString = (dcPackage_getPathString
                        (dcNodeEvaluator_getCurrentFilePackageData
                         (_evaluator)->package));
    dcNode *result = (evaluateImport
                      (_evaluator,
                       pathString,
                       _identifierName,
                       dcNodeEvaluator_getCurrentSelfTemplate(_evaluator),
                       _throwException));
    dcMemory_free(pathString);
    return result;
}

dcNode *dcNodeEvaluator_findObject(dcNodeEvaluator *_evaluator,
                                   const char *_identifierName,
                                   bool _enableExceptionThrow)
{
    dcNode *result = dcObjectStackList_getObject(_evaluator->objectStackList,
                                                 _identifierName);
    if (result != NULL
        && (dcGraphData_isType(result, NODE_NIL)
            || (dcGraphData_isType(result, NODE_CLASS)
                && dcClass_isObject(result))))
    {
        result = dcNodeEvaluator_evaluate(_evaluator, result);
    }
    else if (result == NULL && _enableExceptionThrow)
    {
        dcNodeEvaluator_pushCallStack(_evaluator, _identifierName);
        dcUnidentifiedObjectExceptionClass_throwObject(_identifierName);
        dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
        result = NULL;
    }

    return result;
}

static dcResult evaluateFile(dcNodeEvaluator *_evaluator, const char *_fileName)
{
    _evaluator->onlyEvaluateClasses++;
    dcNode *result = dcFileEvaluator_evaluateFileWithExceptionCatch(_fileName,
                                                                    false);
    _evaluator->onlyEvaluateClasses--;
    return (result == NULL
            ? TAFFY_EXCEPTION
            : TAFFY_SUCCESS);
}

const char *dcNodeEvaluator_getCurrentFileName(dcNodeEvaluator *_evaluator)
{
    return (dcStringManager_getStringFromId(_evaluator->filenameId));
}

dcFilePackageData *dcNodeEvaluator_getCurrentFilePackageData
    (dcNodeEvaluator *_evaluator)
{
    dcError_assert(_evaluator->filenameId != 0);
    return CAST_FILE_PACKAGE_DATA
        (dcSystem_getFilePackageData(_evaluator->filenameId));
}

static dcResult importFile(const char *_fileName,
                           void *_token,
                           bool _throwException)
{
    char *fileNameWithoutTy = dcMemory_strdup(_fileName);
    TAFFY_DEBUG(dcError_assert(strlen(fileNameWithoutTy) >= 3));
    // chomp off .ty
    fileNameWithoutTy[strlen(fileNameWithoutTy) - 3] = 0;

    dcStringId fileNameId = dcStringManager_getStringId(_fileName);
    dcFilePackageData *importFilePackageData =
        CAST_FILE_PACKAGE_DATA(dcSystem_getFilePackageData(fileNameId));

    dcResult result = TAFFY_SUCCESS;
    dcPackage *package = dcPackage_create
        (dcLexer_splitString(fileNameWithoutTy, DIRECTORY_SEPARATOR));
    dcMutex_lock(importFilePackageData->mutex);

    if (importFilePackageData->state == FILE_PACKAGE_DATA_NONE)
    {
        importFilePackageData->state = FILE_PACKAGE_DATA_IMPORTING;
        dcMutex_unlock(importFilePackageData->mutex);

        result = evaluateFile((dcNodeEvaluator *)_token, _fileName);
        dcMutex_lock(importFilePackageData->mutex);

        if (result == TAFFY_SUCCESS)
        {
            if (dcClassManager_getClassFromPackage(package,
                                                   NULL,
                                                   NULL,
                                                   NULL)
                == NULL)
            {
                result = TAFFY_EXCEPTION;

                if (_throwException)
                {
                    char *path = dcPackage_getPathString(package);
                    dcImportFailedExceptionClass_throwObject(path);
                    dcMemory_free(path);
                }
            }
            else
            {
                importFilePackageData->state = FILE_PACKAGE_DATA_IMPORTED;
            }
        }
    }

    dcPackage_free(&package);
    dcMemory_free(fileNameWithoutTy);
    dcMutex_unlock(importFilePackageData->mutex);

    if (result != TAFFY_SUCCESS)
    {
        dcSystem_removeFilePackageData(fileNameId);
    }

    return result;
}

static dcResult importFileWithName(const char *_fileName, void *_token)
{
    return importFile(_fileName, _token, true);
}

static dcNode *callMetaInit(dcNodeEvaluator *_evaluator, dcNode *_node)
{
    dcError_assert(! dcClass_isObject(_node));
    return dcNodeEvaluator_forceCallMethod(_evaluator, _node, "init");
}

static dcNode *evaluateImportNode(dcNodeEvaluator *_evaluator,
                                  dcNode *_importNode,
                                  const dcClassTemplate *_requestorTemplate,
                                  bool _throwException);

static dcNode *evaluateImport(dcNodeEvaluator *_evaluator,
                              const char *_packageName,
                              const char *_className,
                              const dcClassTemplate *_requestorTemplate,
                              bool _throwException)
{
    char *importString =
        dcLexer_sprintf("%s%s%s",
                        (_packageName != NULL
                         && _packageName[0] != 0
                         ? _packageName
                         : ""),
                        (_packageName != NULL
                         && _packageName[0] != 0
                         ? "."
                         : ""),
                        _className);
    dcNode *import = dcImport_createNode
        (dcLexer_splitString(importString, '.'));
    dcNodeEvaluator_setPosition(_evaluator, import);
    dcNode *result = evaluateImportNode(_evaluator,
                                        import,
                                        _requestorTemplate,
                                        _throwException);
    dcNode_free(&import, DC_DEEP);
    dcMemory_free(importString);
    return result;
}

static dcNode *evaluateImportNode(dcNodeEvaluator *_evaluator,
                                  dcNode *_importNode,
                                  const dcClassTemplate *_requestorTemplate,
                                  bool _throwException)
{
    dcNode *result = dcYesClass_getInstance();
    char *directoryName =
        dcPackage_extractDirectoryName(CAST_PACKAGE(_importNode));
    char *fileName = dcLexer_sprintf
        ("%s.%s", directoryName, TAFFY_FILE_NAME_SUFFIX);
    dcFilePackageData *importFilePackageData =
        CAST_FILE_PACKAGE_DATA
        (dcSystem_getFilePackageData
         (dcStringManager_getStringId(fileName)));
    dcPackage *importedPackage = CAST_IMPORT(_importNode);
    dcNode *contentsNode = NULL;
    bool freePackage = false;
    char *pathString = dcPackage_getPathString(importedPackage);

    dcNodeEvaluator_pushCallStack(_evaluator, dcNode_display(_importNode));

    dcNode *currentFilePackageDataNode =
        dcSystem_getFilePackageData(_evaluator->filenameId);
    dcFilePackageData *currentFilePackageData =
        CAST_FILE_PACKAGE_DATA(currentFilePackageDataNode);

    // would an exception have been thrown if _enableExceptions was true
    bool mutedException = false;

    _evaluator->importDepth++;
    currentFilePackageData->state = FILE_PACKAGE_DATA_IMPORTING;

    if (importFilePackageData->state == FILE_PACKAGE_DATA_IMPORTING
        && ! importFilePackageData->deferred)
    {
        importFilePackageData->deferred = true;

        // this package is currently busy, try again a little later
        dcList_push(_evaluator->deferredImports,
                    dcNode_copy(_importNode, DC_DEEP));
    }
    else
    {
        if (importedPackage->isWild)
        {
            // the package is like org.taffy.core.*

            if (importFilePackageData->state == FILE_PACKAGE_DATA_NONE)
            {
                // it hasn't been imported yet, so read the files!

                importFilePackageData->state = FILE_PACKAGE_DATA_IMPORTING;

                if (dcFileManagement_iterateOverFilesInDirectory
                    (directoryName,
                     TAFFY_FILE_NAME_SUFFIX,
                     &importFileWithName,
                     _evaluator)
                    == TAFFY_SUCCESS)
                {
                    contentsNode =
                        dcClassManager_getPackageContents(importedPackage);
                }

                if (_evaluator->exception != NULL
                    && ! _throwException)
                {
                    result = NULL;
                    dcNodeEvaluator_clearException(_evaluator, DC_DEEP);
                    mutedException = true;
                }

                if (contentsNode != NULL)
                {
                    importFilePackageData->state = FILE_PACKAGE_DATA_IMPORTED;
                }
            }
            else
            {
                // it's already been imported, get it from the class manager
                contentsNode =
                    dcClassManager_getPackageContents(importedPackage);
            }
        }
        else
        {
            contentsNode = dcClassManager_getClassFromPackage
                (importedPackage,
                 NULL,
                 NULL,
                 dcNodeEvaluator_getCurrentSelfTemplate(_evaluator));

            if (contentsNode == NULL
                && (importFile(fileName, _evaluator, _throwException)
                    == TAFFY_SUCCESS))
            {
                contentsNode = dcClassManager_getClassFromPackage
                    (importedPackage, NULL, NULL, _requestorTemplate);
            }
        }

        if (contentsNode == NULL)
        {
            // :(
            result = NULL;

            // dcFileEvaluator_evaluateFileWithExceptionCatch() can generate
            // an exception, so verify we're not clobbering an
            // already-set exception
            if (_evaluator->exception == NULL)
            {
                if (_throwException)
                {
                    dcImportFailedExceptionClass_throwObject(pathString);
                }
                else
                {
                    mutedException = true;
                }
            }
        }
        else
        {
            result = dcYesClass_getInstance();
            dcMutex_lock(currentFilePackageData->mutex);
            dcList_push(currentFilePackageData->packageContents, contentsNode);
            dcMutex_unlock(currentFilePackageData->mutex);
        }

        if (freePackage)
        {
            dcPackage_free(&importedPackage);
        }
    }

    // don't decrement importDepth here, otherwise deferred importing
    // below might get into an infinite loop

    if (_evaluator->exception != NULL)
    {
        dcList_clear(_evaluator->deferredImports, DC_DEEP);
        clearImportReferences(_evaluator);
    }
    else if (! mutedException && _evaluator->importDepth == 1)
    {
        // we're done importing!

        // re-import those that were busy during the original import
        if (_evaluator->deferredImports->size > 0)
        {
            dcListElement *that;

            for (that = _evaluator->deferredImports->head;
                 that != NULL;
                 that = that->next)
            {
                if (dcNodeEvaluator_evaluate(_evaluator, that->object) == NULL)
                {
                    result = NULL;
                    break;
                }
            }

            dcList_clear(_evaluator->deferredImports, DC_DEEP);
        }

        // resolve all the dangling references that have accumulated during
        // the import
        while (_evaluator->importReferences->size > 0
               && result != NULL)
        {
            dcNode *topClassTemplate = dcList_pop
                (_evaluator->importReferences, DC_SHALLOW);
            dcClassTemplate *classTemplate =
                CAST_CLASS_TEMPLATE(topClassTemplate);
            // get the meta class so we can get the package contents list
            dcNode *meta = dcClassManager_getClass(classTemplate->className,
                                                   classTemplate->package,
                                                   NULL,
                                                   classTemplate);

            dcError_assert(meta != NULL);
            dcList *packageContents =
                dcSystem_getPackageContentsFromGraphDataNode(meta);

            //
            // if the parent isn't defined yet, try to import it
            //
            if ((dcClassManager_getClassTemplate(classTemplate->superName,
                                                 classTemplate->package,
                                                 packageContents,
                                                 NULL)
                 == NULL)
                && (evaluateImport(_evaluator,
                                   classTemplate->packageName,
                                   classTemplate->superName,
                                   classTemplate,
                                   _throwException)
                    == NULL))
            {
                // TODO: fix me, we don't want to generate an exception
                // just to clear it
                if (_throwException
                    && (strcmp(dcClass_getTemplate
                               (_evaluator->exception)->className,
                               "ImportFailedException")
                        == 0))
                {
                    // the call stack for this import was saved
                    dcNodeEvaluator_clearException(_evaluator, DC_SHALLOW);
                    dcList *callStackSave = _evaluator->callStack;
                    dcNode *callStack = NULL;
                    dcError_assert(dcHash_getValueWithKeys
                                   (_evaluator->importReferencesCallStacks,
                                    NULL,
                                    (size_t)(void *)topClassTemplate,
                                    &callStack)
                                   == TAFFY_SUCCESS);
                    _evaluator->callStack = CAST_LIST(callStack);
                    dcUnidentifiedClassExceptionClass_throwObject
                        (classTemplate->superName);
                    _evaluator->callStack = callStackSave;
                    dcError_assert(dcHash_removeValueWithHashValue
                                   (_evaluator->importReferencesCallStacks,
                                    NULL,
                                    (size_t)(void *)topClassTemplate,
                                    NULL,
                                    DC_DEEP)
                                   == TAFFY_SUCCESS);
                }

                dcNode_freeShell(&topClassTemplate);
                result = NULL;
                break;
            }

            dcResult runtimeResult = dcClassTemplate_createRuntimeValues
                (classTemplate, packageContents);

            if (runtimeResult == TAFFY_SUCCESS)
            {
                //
                // call the meta init: (@@) init
                //
                if (callMetaInit(_evaluator, meta) == NULL)
                {
                    //  result = NULL;
                }
            }
            else if (runtimeResult == TAFFY_EXCEPTION)
            {
                // exception
                result = NULL;
            }
            else
            {
                dcError_assert(runtimeResult == TAFFY_SUCCESS
                               || runtimeResult == TAFFY_EXCEPTION);
            }

            dcNode_freeShell(&topClassTemplate);
        }

        clearImportReferences(_evaluator);
        dcHash_clear(_evaluator->importReferencesCallStacks, DC_DEEP);
    }

    dcMemory_free(pathString);
    _evaluator->importDepth--;
    dcMemory_free(fileName);

    if (result != NULL)
    {
        currentFilePackageData->state = FILE_PACKAGE_DATA_IMPORTED;
    }

    dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
    dcMemory_free(directoryName);
    return result;
}

dcNode *dcNodeEvaluator_evaluateImport(dcNodeEvaluator *_evaluator,
                                       dcNode *_importNode)
{
    return evaluateImportNode(_evaluator, _importNode, NULL, true);
}

dcNode *dcNodeEvaluator_evaluateIn(dcNodeEvaluator *_evaluator, dcNode *_in)
{
    dcIn *in = CAST_IN(_in);
    dcNode *result = NULL;
    dcNode *object = dcNodeEvaluator_evaluate(_evaluator, in->left);

    if (object != NULL)
    {
        result = dcNoClass_getInstance();

        FOR_EACH_IN_ARRAY(in->array, i, that)
        {
            dcNode *success = (dcNodeEvaluator_callMethodWithArgument
                               (_evaluator,
                                object,
                                dcSystem_getOperatorName(TAFFY_EQUALS),
                                that));

            if (success == NULL)
            {
                // exception, bail
                result = NULL;
                break;
            }
            else if (success == dcYesClass_getInstance())
            {
                result = dcYesClass_getInstance();
                break;
            }
        }
    }

    return result;
}

dcNode *dcNodeEvaluator_getCurrentSelf(dcNodeEvaluator *_evaluator)
{
    return dcObjectStackList_getTailSelf(_evaluator->objectStackList);
}

dcClassTemplate *dcNodeEvaluator_getCurrentSelfTemplate
    (dcNodeEvaluator *_evaluator)
{
    dcNode *currentSelf = dcNodeEvaluator_getCurrentSelf(_evaluator);

    return (currentSelf == NULL
            ? NULL
            : dcClass_getTemplate(currentSelf));
}

dcNode *dcNodeEvaluator_evaluateGraphDataTree(dcNodeEvaluator *_evaluator,
                                              dcNode *_treeNode)
{
    return dcNodeEvaluator_evaluate
        (_evaluator, dcGraphDataTree_getContents(_treeNode));
}

static void pushPosition(dcNodeEvaluator *_evaluator, dcNode *_graphDataNode)
{
    dcGraphData *graphData = CAST_GRAPH_DATA(_graphDataNode);
    dcList_push(_evaluator->callStack,
                dcCallStackData_createNode
                (NULL,
                 graphData->filenameId,
                 graphData->lineNumber));
}

static dcNode *maybePromoteArithmetic(dcNode *_result,
                                      dcNode *_node,
                                      dcNodeEvaluator *_evaluator)
{
    if (_result == NULL
        && dcClass_isType(_evaluator->exception,
                          "org.taffy.core.exception",
                          "UnidentifiedObjectException"))
    {
        // promote arithmetic to function
        dcNodeEvaluator_clearException(_evaluator, DC_DEEP);
        dcNode *function = (dcParser_createFunctionFromGuts
                            (dcNode_copy(_node, DC_DEEP)));
        dcNode_setTemplate(function, false);
        dcNode_register(function);
        _result = dcNodeEvaluator_evaluate(_evaluator, function);
    }

    return _result;
}

dcNode *dcNodeEvaluator_evaluateIdentifier(dcNodeEvaluator *_evaluator,
                                           dcNode *_identifierNode)
{
    const char *identifierName = dcIdentifier_getName(_identifierNode);
    // TODO: call findObject here, move the guts below into it

    dcFilePackageData *data =
        dcNodeEvaluator_getCurrentFilePackageData(_evaluator);

    // it wasn't a class, try an identifier
    dcNode *result = dcNodeEvaluator_findObject
        (_evaluator, identifierName, false);

    if (! dcNodeEvaluator_canContinueEvaluating(_evaluator))
    {
        return NULL;
    }

    if (result == NULL)
    {
        dcMutex_lock(data->mutex);
        // it wasn't an identifier, try a class
        result = dcClassManager_getClass
            (identifierName,
             data->package,
             data->packageContents,
             dcNodeEvaluator_getCurrentSelfTemplate(_evaluator));
        dcMutex_unlock(data->mutex);
    }

    if (result == NULL)
    {
        // try importing via the current package
        result = evaluateImportFromCurrentPackage
            (_evaluator, identifierName, false);
    }

    // don't overwrite an already-existing exception
    if (result == NULL && _evaluator->exception == NULL)
    {
        // couldn't find a match
        dcNodeEvaluator_pushCallStack(_evaluator, identifierName);
        dcUnidentifiedObjectExceptionClass_throwObject(identifierName);
        dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
    }

    if (result != NULL && dcFutureClass_isMe(result))
    {
        result = dcFutureClass_waitForValue(result);
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateAssignment(dcNodeEvaluator *_evaluator,
                                           dcNode *_assignment)
{
    // the return value //
    dcNode *result = NULL;

    // extract the information out of the assignment //
    dcNode *identifier = dcAssignment_getIdentifier(_assignment);
    dcNode *value = dcAssignment_getValue(_assignment);
    const char *identifierName = dcIdentifier_getName(identifier);

    //
    // evaluate the value
    //
    // eg
    //
    // > a = 37
    //
    // value == 37
    //

    dcNode *evaluatedValue = dcNodeEvaluator_evaluate(_evaluator, value);

    if (evaluatedValue != NULL)
    {
        if (dcClassTemplate_isAtomic(dcClass_getTemplate(evaluatedValue))
            || dcNode_isTemplate(evaluatedValue))
        {
            bool isTemplate = dcNode_isTemplate(evaluatedValue);

            evaluatedValue = dcNode_register
                (dcNode_copy(evaluatedValue, DC_DEEP));

            if (isTemplate)
            {
                // functions created via f(x) syntax are not new'd,
                // so simulate that here
                // TODO: account for inherited classes
                if (dcFunctionClass_isMe(evaluatedValue))
                {
                    if (dcNodeEvaluator_forceCallMethod(_evaluator,
                                                        evaluatedValue,
                                                        "init")
                        == NULL)
                    {
                        return NULL;
                    }
                }
            }
        }

        dcScopeDataFlags assignmentFlags = dcAssignment_getFlags(_assignment);
        dcScope *foundScope = NULL;
        dcNode *scopeDataNode =
            dcObjectStackList_getScopeDataForObject(_evaluator->objectStackList,
                                                    identifierName,
                                                    &foundScope);

        // it's a parse bug if the length of identifierName == 0
        dcError_assert(strlen(identifierName) > 0);

        if (identifierName[0] == '@' && scopeDataNode == NULL)
        {
            // don't allow assignment of class variables that weren't
            // declared
            dcNodeEvaluator_pushCallStack(_evaluator, identifierName);
            dcUnidentifiedObjectExceptionClass_throwObjectWithReason
                (identifierName,
                 "class-scoped variables must first be declared in the class "
                 "header");
            dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
        }
        // do not allow redefinitions of constant objects //
        else if (scopeDataNode != NULL
                 && ((dcScopeData_getFlags(scopeDataNode) & SCOPE_DATA_CONSTANT)
                     != 0))
        {
            dcNodeEvaluator_pushCallStackFromGraphDataNode(_evaluator,
                                                           value,
                                                           "");
            dcConstantRedefinitionExceptionClass_throwObject(identifierName);
            dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
        }
        else
        {
            dcScope *scopeToUse = NULL;

            if ((assignmentFlags & SCOPE_DATA_GLOBAL) != 0)
            {
                scopeToUse = CAST_SCOPE(dcSystem_getGlobalScope());
            }
            else
            {
                scopeToUse = dcObjectStackList_getTailScope
                    (_evaluator->objectStackList);
            }

            dcError_assert(scopeToUse != NULL);
            dcScopeDataFlags identifierFlags =
                dcIdentifier_getScopeDataFlags(identifier);

            // propagate the assignment flags to the identifier flags
            identifierFlags |= assignmentFlags;

            if (scopeDataNode != NULL)
            {
                //
                // error if the node was not global and now is
                //
                if ((assignmentFlags & SCOPE_DATA_GLOBAL) != 0
                    && ((dcScopeData_getFlags(scopeDataNode)
                         & SCOPE_DATA_GLOBAL)
                        == 0))
                {
                    dcLocalToGlobalConversionExceptionClass_throwObject
                        (identifierName);
                }
                else
                {
                    //
                    // the object already exists, update it from whence it came
                    //
                    dcError_assert(foundScope != NULL);

                    dcScope_setObject(foundScope,
                                      evaluatedValue,
                                      identifierName,
                                      identifierFlags);
                    result = evaluatedValue;
                }
            }
            else
            {
                //
                // the object does not already exist, update the scope
                //
                dcScope_setObject(scopeToUse,
                                  evaluatedValue,
                                  identifierName,
                                  identifierFlags);
                result = evaluatedValue;
            }
        }
    }

    return result;
}

//
// evaluate a method call list like:
// [[[myObject foo] bar] baz]
//
TAFFY_HIDDEN dcNode *evaluateMethodCallList(dcNodeEvaluator *_evaluator,
                                            dcNode *_listNode)
{
    dcListElement *that = CAST_GRAPH_DATA_LIST(_listNode)->head;
    dcNode *receiver = dcNodeEvaluator_evaluate(_evaluator, that->object);

    if (receiver != NULL)
    {
        uint32_t pushedCount = dcNodeEvaluator_pushMark(_evaluator, receiver);

        for (that = that->next; that != NULL; that = that->next)
        {
            dcError_assert(dcGraphData_getType(that->object)
                           == NODE_METHOD_CALL);

            dcMethodCall *methodCall = CAST_METHOD_CALL(that->object);
            pushedCount += dcNodeEvaluator_pushMark(_evaluator, receiver);

            // set the receiver //
            receiver = (dcNodeEvaluator_callMethodWithArguments
                        (_evaluator,
                         receiver,
                         methodCall->methodName,
                         methodCall->arguments,
                         false));

            if (receiver == NULL)
            {
                // exception occurred, and is already set //
                break;
            }
        }

        dcNodeEvaluator_popMarks(_evaluator, pushedCount);
    }

    return receiver;
}

dcNode *dcNodeEvaluator_evaluateGraphDataList(dcNodeEvaluator *_evaluator,
                                              dcNode *_listNode)
{
    dcListElement *head = CAST_GRAPH_DATA_LIST(_listNode)->head;
    dcNode *result = NULL;

    dcError_assert(head->object->type == NODE_GRAPH_DATA);

    if (dcGraphData_getType(head->object) == NODE_METHOD_CALL)
    {
        result = evaluateMethodCallList(_evaluator, _listNode);
    }
    else if (dcGraphData_getType(head->object) == NODE_IDENTIFIER)
    {
        result = findMultiScopedObject
            (_evaluator, CAST_GRAPH_DATA_LIST(_listNode));
    }
    else
    {
        dcError_assert(false);
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateMethodCall(dcNodeEvaluator *_evaluator,
                                           dcNode *_methodCallNode)
{
    TAFFY_DEBUG(dcList_verifyTemplate
                (dcMethodCall_getArguments(_methodCallNode)));

    return dcNodeEvaluator_callMethodWithArguments
        (_evaluator,
         dcMethodCall_getReceiver(_methodCallNode),
         dcMethodCall_getMethodName(_methodCallNode),
         dcMethodCall_getArguments(_methodCallNode),
         false);
}

dcNode *dcNodeEvaluator_evaluateIf(dcNodeEvaluator *_evaluator, dcNode *_ifNode)
{
    // the return value //
    dcNode *result = NULL;

    // extract the data out of _ifNode //
    dcNode *condition = dcIf_getCondition(_ifNode);
    dcNode *statement = dcIf_getStatement(_ifNode);

    // create a scope for this block //
    dcObjectStackList_pushScope(_evaluator->objectStackList,
                                dcScope_createNode());

    if (condition == NULL)
    {
        // is an 'else' //
        result = dcNodeEvaluator_evaluate(_evaluator, statement);
    }
    else
    {
        dcNode *evaluatedCondition =
            dcNodeEvaluator_evaluate(_evaluator, condition);
        dcNode *next = dcIf_getNext(_ifNode);
        result = evaluatedCondition;

        if (evaluatedCondition == dcYesClass_getInstance())
        {
            result = dcNodeEvaluator_evaluate(_evaluator, statement);
        }
        else if (evaluatedCondition != NULL
                 && next != NULL)
        {
            // is an 'else' or 'else if' //
            result = dcNodeEvaluator_evaluate(_evaluator, next);
        }
        // else if evaluatedCondition == NULL, then there is an exception
    }

    // pop the if's scope, and free it //
    dcObjectStackList_popScope(_evaluator->objectStackList, DC_DEEP);

    return result;
}

void dcNodeEvaluator_startLoop(dcNodeEvaluator *_evaluator)
{
    dcObjectStackList_getTailObjectStack
        (_evaluator->objectStackList)->loopCount++;
}

void dcNodeEvaluator_stopLoop(dcNodeEvaluator *_evaluator)
{
    dcObjectStackList_getTailObjectStack
        (_evaluator->objectStackList)->loopCount--;
    dcNodeEvaluator_setBreaking(_evaluator, false);
}

static dcNode *evaluateLoop(dcNodeEvaluator *_evaluator,
                            dcNode *_initial,
                            dcNode *_condition,
                            dcNode *_increment,
                            dcNode *_statement)
{
    // create a scope for the while statement //
    dcNode *scope = dcScope_createNode();
    bool isNewScope = true;
    bool pushed = false;
    int pushCount = 0;
    int markCount = 0;
    dcNode *result = dcNilClass_getInstance();
    dcNode *condition = NULL;

    markCount += dcNodeEvaluator_pushMark(_evaluator, _initial);
    markCount += dcNodeEvaluator_pushMark(_evaluator, _condition);
    markCount += dcNodeEvaluator_pushMark(_evaluator, _increment);
    markCount += dcNodeEvaluator_pushMark(_evaluator, _statement);

    dcObjectStack *objectStack =
        dcObjectStackList_getTailObjectStack(_evaluator->objectStackList);
    dcObjectStack_pushScope(objectStack, dcScope_createNode());

    if (_initial != NULL)
    {
        if (_initial->type == NODE_LIST)
        {
            FOR_EACH_IN_LIST(CAST_LIST(_initial), that)
            {
                result = dcNodeEvaluator_evaluate(_evaluator, that->object);

                if (result == NULL)
                {
                    break;
                }
            }
        }
        else
        {
            result = dcNodeEvaluator_evaluate(_evaluator, _initial);
        }
    }

    markCount += dcNodeEvaluator_pushMark(_evaluator, result);
    dcNodeEvaluator_startLoop(_evaluator);

    // evaluate the loop statement's condition
    // we must check if we can continue evaluating before and after
    // evaluating the condition
    while (result != NULL
           && dcNodeEvaluator_canContinueEvaluating(_evaluator)
           && ((condition = dcNodeEvaluator_evaluate(_evaluator, _condition))
               != NULL)
           && condition != dcNilClass_getInstance()
           && condition != dcNoClass_getInstance())
    {
        if (isNewScope)
        {
            if (scope == NULL)
            {
                scope = dcScope_createNode();
            }

            // push the scope onto the scope stack //
            dcObjectStack_pushScope(objectStack, scope);
            pushed = true;
            pushCount++;
            isNewScope = false;
        }

        markCount += dcNodeEvaluator_pushMark(_evaluator, condition);
        result = dcNodeEvaluator_evaluate(_evaluator, _statement);

        if (dcScope_isModified(CAST_SCOPE(scope)))
        {
            // pop the scope, and free it //
            dcObjectStack_popScope(objectStack, DC_DEEP);
            scope = NULL;
            isNewScope = true;
            pushCount--;
        }

        if (! dcNodeEvaluator_canContinueEvaluating(_evaluator))
        {
            break;
        }
        else if (result != NULL
                 && _increment != NULL)
        {
            if (_increment->type == NODE_LIST)
            {
                FOR_EACH_IN_LIST(CAST_LIST(_increment), that)
                {
                    result = dcNodeEvaluator_evaluate(_evaluator, that->object);

                    if (result == NULL)
                    {
                        break;
                    }
                }
            }
            else
            {
                result = dcNodeEvaluator_evaluate(_evaluator, _increment);
            }
        }
    }

    dcNodeEvaluator_stopLoop(_evaluator);
    dcNodeEvaluator_popMarks(_evaluator, markCount);

    if (condition == NULL)
    {
        // propagate the exception
        result = NULL;
    }

    if (pushCount > 0)
    {
        dcObjectStackList_popScope(_evaluator->objectStackList, DC_DEEP);
    }
    else if (!pushed)
    {
        dcNode_free(&scope, DC_DEEP);
    }

    dcObjectStackList_popScope(_evaluator->objectStackList, DC_DEEP);
    return result;
}

dcNode *dcNodeEvaluator_evaluateFor(dcNodeEvaluator *_evaluator,
                                    dcNode *_forNode)
{
    return evaluateLoop(_evaluator,
                        dcFor_getInitial(_forNode),
                        dcFor_getCondition(_forNode),
                        dcFor_getIncrement(_forNode),
                        dcFor_getStatement(_forNode));
}

dcNode *dcNodeEvaluator_evaluateWhile(dcNodeEvaluator *_evaluator,
                                      dcNode *_whileNode)
{
    return evaluateLoop(_evaluator,
                        NULL,
                        dcWhile_getCondition(_whileNode),
                        NULL,
                        dcWhile_getStatement(_whileNode));
}

dcNode *dcNodeEvaluator_evaluateOr(dcNodeEvaluator *_evaluator,
                                   dcNode *_orNode)
{
    // extract the data from _node //
    dcNode *leftNode = dcOr_getLeft(_orNode);
    dcNode *rightNode = dcOr_getRight(_orNode);

    // evaluate the left side of the or //
    dcNode *left = dcNodeEvaluator_evaluate(_evaluator, leftNode);

    return (left == NULL
            // an exception occurred
            ? NULL
            // no exception occurred
            : (left == dcYesClass_getInstance()
               // short circuit
               ? left
               // no short circuit :(
               : dcNodeEvaluator_evaluate(_evaluator, rightNode)));
}


dcNode *dcNodeEvaluator_evaluateFlatArithmetic(dcNodeEvaluator *_evaluator,
                                               dcNode *_node)
{
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    const char *operatorName = (char *)(dcSystem_getOperatorName
                                        (arithmetic->taffyOperator));
    dcNode *result = NULL;

    if (arithmetic->taffyOperator == TAFFY_RAISE)
    {
        dcError_assert(arithmetic->values->size > 1);
        result = (dcNodeEvaluator_callMethodWithArgument
                  (_evaluator,
                   arithmetic->values->tail->previous->object,
                   operatorName,
                   arithmetic->values->tail->object));

        dcError_assert(arithmetic->values->size > 0);
        dcListElement *that;

        for (that = arithmetic->values->tail->previous->previous;
             that != NULL && result != NULL;
             that = that->previous)
        {
            result = dcNodeEvaluator_callMethodWithArgument(_evaluator,
                                                            that->object,
                                                            operatorName,
                                                            result);
        }
    }
    else
    {
        result = dcList_getHead(arithmetic->values);
        dcError_assert(arithmetic->values->size > 0);
        dcListElement *that;

        for (that = arithmetic->values->head->next;
             that != NULL && result != NULL;
             that = that->next)
        {
            result = dcNodeEvaluator_callMethodWithArgument(_evaluator,
                                                            result,
                                                            operatorName,
                                                            that->object);
        }
    }

    return maybePromoteArithmetic(result, _node, _evaluator);
}

dcNode *dcNodeEvaluator_evaluateTrue(dcNodeEvaluator *_evaluator,
                                     dcNode *_trueNode)
{
    return dcYesClass_getInstance();
}

dcNode *dcNodeEvaluator_evaluateFunctionUpdate(dcNodeEvaluator *_evaluator,
                                               dcNode *_functionUpdate)
{
    dcNode *result = dcNodeEvaluator_evaluateIdentifier
        (_evaluator, dcFunctionUpdate_getIdentifier(_functionUpdate));

    if (result != NULL)
    {
        result = (dcFunctionClass_addSpecific
                  (result,
                   dcFunctionUpdate_getArguments(_functionUpdate),
                   dcFunctionUpdate_getArithmetic(_functionUpdate)));
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateFalse(dcNodeEvaluator *_evaluator,
                                      dcNode *_NONode)
{
    return dcNoClass_getInstance();
}

dcNode *dcNodeEvaluator_evaluateNil(dcNodeEvaluator *_evaluator,
                                    dcNode *_nilNode)
{
    return dcNilClass_getInstance();
}

dcNode *dcNodeEvaluator_evaluateAnd(dcNodeEvaluator *_evaluator,
                                    dcNode *_andNode)
{
    // evaluate the left side of the <and> //
    dcNode *evaluatedLeft = dcNodeEvaluator_evaluate(_evaluator,
                                                     dcAnd_getLeft(_andNode));
    dcNode *result = NULL;

    if (evaluatedLeft == dcYesClass_getInstance())
    {
        uint32_t marked = dcNodeEvaluator_pushMark(_evaluator, evaluatedLeft);
        // no short-circuiting can be performed, evaluate the right //
        result = dcNodeEvaluator_evaluate(_evaluator, dcAnd_getRight(_andNode));
        dcNodeEvaluator_popMarks(_evaluator, marked);
    }
    else
    {
        result = dcNoClass_getInstance();
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateNew(dcNodeEvaluator *_evaluator,
                                    dcNode *_newNode)
{
    dcNode *result = NULL;
    dcNode *identifier = CAST_NEW(_newNode);
    dcFilePackageData *filePackageData =
        dcNodeEvaluator_getCurrentFilePackageData(_evaluator);
    const char *identifierName = dcIdentifier_getName(identifier);
    dcNode *theClass = (dcClassManager_getClass
                        (identifierName,
                         filePackageData->package,
                         filePackageData->packageContents,
                         dcNodeEvaluator_getCurrentSelfTemplate(_evaluator)));

    if (theClass == NULL)
    {
        theClass = evaluateImportFromCurrentPackage
            (_evaluator, identifierName, false);

        if (theClass == dcYesClass_getInstance())
        {
            theClass = (dcClassManager_getClass
                        (identifierName,
                         filePackageData->package,
                         filePackageData->packageContents,
                         dcNodeEvaluator_getCurrentSelfTemplate(_evaluator)));
        }
    }

    if (theClass == NULL)
    {
        // an exception must not already exist
        assert(_evaluator->exception == NULL);
        result = NULL;
        dcNodeEvaluator_pushCallStack(_evaluator, identifierName);
        dcUnidentifiedClassExceptionClass_throwObject(identifierName);
        dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
    }
    else
    {
        dcClassTemplate *classTemplate = dcClass_getTemplate(theClass);
        // create the call stack entry
        dcNodeEvaluator_pushCallStack(_evaluator, dcNew_display(_newNode));

        if ((classTemplate->classFlags & CLASS_SINGLETON) != 0
            && classTemplate->singletonId > 0)
        {
            dcSingletonInstantiationExceptionClass_throwObject
                (classTemplate->className);
            result = NULL;
        }
        else
        {
            if (dcClassTemplate_isAbstract(classTemplate))
            {
                dcAbstractClassInstantiationExceptionClass_throwObject
                    (classTemplate->className);
            }
            else
            {
                result = (dcNode_register
                          (dcClass_createBasicNode(classTemplate, true)));

                dcGraphData_copyPosition(theClass, result);
                uint32_t marked = dcNodeEvaluator_pushMark(_evaluator, result);

                if (dcNodeEvaluator_forceCallMethod(_evaluator, result, "init")
                    == NULL)
                {
                    result = NULL;
                }

                dcNodeEvaluator_popMarks(_evaluator, marked);
                dcNodeEvaluator_setReturnValue(_evaluator, result);
            }
        }

        dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateNotEqual(dcNodeEvaluator *_evaluator,
                                         dcNode *_notEqualNode)
{
    dcNode *result = dcNodeEvaluator_evaluate
        (_evaluator, CAST_NOTEQUALCALL(_notEqualNode));
    return (result == NULL
            ? NULL
            : ((result == dcNilClass_getInstance()
                || result == dcNoClass_getInstance())
               ? dcYesClass_getInstance()
               : dcNoClass_getInstance()));
}

static dcNode *evaluateMetaClass(dcNodeEvaluator *_evaluator, dcNode *_meta)
{
    dcClassTemplate *templateTemplate = dcClass_getTemplate(_meta);
    dcNode *result = _meta;
    bool newClass = false;

    if (dcNode_isTemplate(_meta))
    {
        dcFilePackageData *data =
            dcNodeEvaluator_getCurrentFilePackageData(_evaluator);

        dcMutex_lock(data->mutex);
        dcNode *klass = (dcClassManager_getClass
                         (templateTemplate->className,
                          templateTemplate->package,
                          data->packageContents,
                          dcNodeEvaluator_getCurrentSelfTemplate(_evaluator)));
        dcMutex_unlock(data->mutex);

        if (klass == NULL)
        {
            if (dcNodeEvaluator_findObject(_evaluator,
                                           templateTemplate->superName,
                                           false)
                != NULL)
            {
                // the superclass already exists as a non-meta object
                dcUnidentifiedClassExceptionClass_throwObject
                    (templateTemplate->superName);
                result = NULL;
            }
            else
            {
                newClass = true;

                // the class hasn't been created before
                dcClassTemplate *copy = dcClassTemplate_copy
                    (templateTemplate, DC_DEEP);
                dcScope_register(copy->scope);
                pushPosition(_evaluator, _meta);

                dcMutex_lock(data->mutex);
                dcResult registerResult =
                    (dcClassManager_registerClassTemplate
                     (copy,
                      data->packageContents,
                      // don't initialize the class template if we're importing
                      // since its super class might not be defined yet
                      (_evaluator->importDepth == 0),
                      &result));
                dcMutex_unlock(data->mutex);

                if (_evaluator->importDepth > 0)
                {
                    // wait till later to initialize this template since
                    // we're in the middle of an import
                    dcNode *classTemplateShell =
                        dcClassTemplate_createShell(copy);
                    dcList_push(_evaluator->importReferences,
                                classTemplateShell);
                    dcHash_setValueWithHashValue
                        (_evaluator->importReferencesCallStacks,
                         NULL,
                         (size_t)((void *)classTemplateShell),
                         dcList_createShell
                         (dcList_copy(_evaluator->callStack, DC_DEEP)));
                }

                dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);

                if (registerResult == TAFFY_EXCEPTION)
                {
                    dcClassTemplate_free(&copy, DC_DEEP);
                    result = NULL;
                }
                else
                {
                    dcError_assert(registerResult == TAFFY_SUCCESS);
                    dcGraphData_copyPosition(_meta, result);
                }
            }
        }
        else
        {
            dcClassFlags metaFlags = dcClass_getFlags(_meta);
            dcClassFlags classFlags = dcClass_getFlags(klass);

            // don't care about read write lock, or singelton in this context
            metaFlags &= ~(CLASS_HAS_READ_WRITE_LOCK | CLASS_SINGLETON);
            classFlags &= ~(CLASS_HAS_READ_WRITE_LOCK | CLASS_SINGLETON);

            if (metaFlags != classFlags)
            {
                result = NULL;
                dcInconsistentClassUpdateExceptionClass_throwObject();
            }
            else if ((metaFlags & CLASS_FINAL) != 0)
            {
                result = NULL;
                dcFinalClassUpdateExceptionClass_throwObject();
            }
            else
            {
                // it's already created, update!
                result = klass;
                dcClassTemplate_update(dcClass_getTemplate(klass),
                                       templateTemplate);
            }
        }

        // finally, evaluate all composited classes
        if (result != NULL)
        {
            dcHashIterator *iterator =
                dcScope_createIterator(dcClass_getMetaScope(result));
            dcNode *that = NULL;

            while ((that = dcScope_getNext(iterator, SCOPE_DATA_OBJECT))
                   != NULL)
            {
                dcNode *object = dcScopeData_getObject(that);

                if (IS_CLASS(object)
                    && ! dcClass_isObject(object))
                {
                    dcClass_getTemplate(object)->parentNode = result;

                    if (dcNodeEvaluator_evaluate(_evaluator, object) == NULL)
                    {
                        result = NULL;
                        break;
                    }
                }
            }

            dcHashIterator_free(&iterator);

            //
            // call the meta init: (@@) init
            //
            if (newClass
                && result != NULL
                && _evaluator->importDepth == 0)
            {
                result = callMetaInit(_evaluator, result);
            }
        }
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateClass(dcNodeEvaluator *_evaluator,
                                      dcNode *_classNode)
{
    dcNode *result = _classNode;
    const dcClassTemplate *classTemplate = dcClass_getTemplate(_classNode);

    //
    // we're interested in evaluating an object if it's
    //   an uninitialized array, string, hash, or pair
    //

    if (! dcClass_isObject(_classNode))
    {
        result = evaluateMetaClass(_evaluator, _classNode);
    }
    else
    {
        if (classTemplate == dcArrayClass_getTemplate()
            && ! dcArrayClass_isInitialized(_classNode))
        {
            result = dcNodeEvaluator_evaluateUninitializedArrayClass
                (_evaluator, _classNode);
        }
        else if (classTemplate == dcStringClass_getTemplate()
                 && ! dcStringClass_isInitialized(_classNode))
        {
            result = dcNodeEvaluator_evaluateUninitializedStringClass
                (_evaluator, _classNode);
        }
        else if (classTemplate == dcHashClass_getTemplate()
                 && ! dcHashClass_isInitialized(_classNode))
        {
            result = dcNodeEvaluator_evaluateUninitializedHashClass
                (_evaluator, _classNode);
        }
        else if (classTemplate == dcPairClass_getTemplate()
                 && ! dcPairClass_isInitialized(_classNode))
        {
            result = dcNodeEvaluator_evaluateUninitializedPairClass
                (_evaluator, _classNode);
        }
        else if (classTemplate == dcMatrixClass_getTemplate()
                 && ! dcMatrixClass_isInitialized(_classNode))
        {
            result = dcNodeEvaluator_evaluateUninitializedMatrixClass
                (_evaluator, _classNode);
        }
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateUninitializedArrayClass
    (dcNodeEvaluator *_evaluator, dcNode *_arrayNode)
{
    dcArray *evaluatedArray =
        dcNodeEvaluator_evaluateUninitializedArray
        (_evaluator, dcArrayClass_getObjects(_arrayNode));

    return (evaluatedArray == NULL
            ? NULL
            : (dcNode_register
               (dcArrayClass_createObject(evaluatedArray, true))));
}

dcNode *dcNodeEvaluator_evaluateUninitializedMatrixClass
    (dcNodeEvaluator *_evaluator, dcNode *_matrixNode)
{
    dcMatrix *matrix = dcMatrixClass_getMatrix(_matrixNode);
    dcArray *evaluatedArray =
        dcNodeEvaluator_evaluateUninitializedArray(_evaluator,
                                                   matrix->objects);
    return (evaluatedArray == NULL
            ? NULL
            : dcNode_register(dcMatrixClass_createObject
                              (dcMatrix_create(evaluatedArray,
                                               matrix->rowCount,
                                               matrix->columnCount),
                               true)));
}

dcNode *dcNodeEvaluator_evaluateUninitializedStringClass
    (dcNodeEvaluator *_evaluator, dcNode *_stringNode)
{
    dcNode *result =
        dcNode_register
        (dcStringClass_createObjectFromList
         (dcList_copy(dcStringClass_getObjects(_stringNode), DC_DEEP),
          true));

    if (result != NULL)
    {
        dcGraphData_copyPosition(_stringNode, result);
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateUninitializedHashClass
    (dcNodeEvaluator *_evaluator, dcNode *_hashNode)
{
    dcNode *result = NULL;
    dcList *tempKeys = dcHashClass_getTempKeys(_hashNode);
    dcList *tempValues = dcHashClass_getTempValues(_hashNode);
    dcList *evaluatedKeys = dcNodeEvaluator_evaluateUninitializedList
        (_evaluator, tempKeys, true);
    uint32_t markCount = 0;

    if (evaluatedKeys != NULL)
    {

        markCount += dcNodeEvaluator_pushListToMark(_evaluator, evaluatedKeys);
        dcList *evaluatedValues =
            dcNodeEvaluator_evaluateUninitializedList(_evaluator,
                                                      tempValues,
                                                      true);
        markCount += dcNodeEvaluator_pushListToMark
            (_evaluator, evaluatedValues);

        if (evaluatedValues != NULL)
        {
            dcNode *exception = NULL;
            dcNode *hashClass =
                dcNode_register
                (dcHashClass_createObjectFromInitializedLists
                 (evaluatedKeys,
                  evaluatedValues,
                  exception,
                  _evaluator));
            dcList_free(&evaluatedValues, DC_SHALLOW);

            if (exception != NULL)
            {
                dcInvalidHashValueExceptionClass_throwObject(exception);
                result = NULL;
            }
            else
            {
                result = hashClass;
            }
        }
        // else exception occurred //

        dcList_free(&evaluatedKeys, DC_SHALLOW);
    }

    dcNodeEvaluator_popMarks(_evaluator, markCount);
    return result;
}

dcNode *dcNodeEvaluator_evaluateUninitializedPairClass
    (dcNodeEvaluator *_evaluator, dcNode *_pairNode)
{
    dcNode *result = NULL;
    dcNode *left = dcPairClass_getLeft(_pairNode);
    dcNode *right = dcPairClass_getRight(_pairNode);
    dcNode *evaluatedLeft = dcNodeEvaluator_evaluate(_evaluator, left);

    if (evaluatedLeft != NULL)
    {
        dcNode *evaluatedRight =
            dcNodeEvaluator_evaluate(_evaluator, right);

        if (evaluatedRight != NULL)
        {
            result = dcPairClass_createObject(evaluatedLeft,
                                              evaluatedRight,
                                              true);
        }
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluatePackage(dcNodeEvaluator *_evaluator,
                                        dcNode *_package)
{
    dcNode *result = NULL;
    dcFilePackageData *data =
        dcNodeEvaluator_getCurrentFilePackageData(_evaluator);
    dcMutex_lock(data->mutex);

    if (data->package == NULL)
    {
        result = dcYesClass_getInstance();
        data->package = dcPackage_copy(CAST_PACKAGE(_package), DC_DEEP);
        dcNode *contents = NULL;

        if (dcPackage_isWild(_package))
        {
            contents = dcClassManager_getPackageContents
                (CAST_PACKAGE(_package));
        }
        else
        {
            contents = dcClassManager_getClassFromPackage
                (CAST_PACKAGE(_package),
                 NULL,
                 NULL,
                 NULL);
        }

        if (contents != NULL)
        {
            dcList_push(data->packageContents, contents);
        }
    }
    else
    {
        dcNodeEvaluator_pushCallStack(_evaluator, dcNode_display(_package));
        char *pathString = dcPackage_getPathString(CAST_PACKAGE(_package));
        dcMoreThanOnePackageExceptionClass_throwObject(pathString);
        dcMemory_free(pathString);
        dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
    }

    dcMutex_unlock(data->mutex);
    return result;
}

dcNode *dcNodeEvaluator_evaluateReturn(dcNodeEvaluator *_evaluator,
                                       dcNode *_returnNode)
{
    // get the return value: "return yes" ==> returnValue == yes //
    dcNode *returnValue = dcReturn_getValueFromNode(_returnNode);
    dcNode *result = NULL;

    if (_evaluator->callStack->size == 0)
    {
        // can't perform return when not in a procedure
        dcReturnWithNoCallStackExceptionClass_throwObject();
    }
    else
    {
        if (returnValue == NULL)
        {
            result = dcNilClass_getInstance();
        }
        else
        {
            uint32_t marked = dcNodeEvaluator_pushMark(_evaluator, returnValue);
            result =
                dcNodeEvaluator_evaluate(_evaluator, returnValue);
            dcNodeEvaluator_popMarks(_evaluator, marked);
        }

        if (result != NULL)
        {
            dcNodeEvaluator_setReturning(_evaluator, true);
        }
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateBreak(dcNodeEvaluator *_evaluator,
                                      dcNode *_breakNode)
{
    // see if we can find an object stack that is either
    // breakthrough and has no loop count
    // ! breakthrough and has a loop count
    dcListElement *that = NULL;
    bool found = false;

    for (that = _evaluator->objectStackList->objectStacks->tail;
         that != NULL;
         that = that->previous)
    {
        dcObjectStack *objectStack = CAST_OBJECT_STACK(that->object);

        if (objectStack->loopCount > 0)
        {
            found = true;
            break;
        }
        else if (! objectStack->breakthrough)
        {
            break;
        }
    }

    dcNode *result = NULL;

    if (! found)
    {
        dcBreakWithoutALoopExceptionClass_throwObject();
    }
    else
    {
        dcNodeEvaluator_setBreaking(_evaluator, true);
        result = dcNilClass_getInstance();
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateSelf(dcNodeEvaluator *_evaluator,
                                     dcNode *_selfNode)
{
    return dcNodeEvaluator_getCurrentSelf(_evaluator);
}

dcNode *dcNodeEvaluator_evaluateUpSelf(dcNodeEvaluator *_evaluator,
                                       dcNode *_selfNode)
{
    return dcObjectStackList_getUpSelf(_evaluator->objectStackList);
}

dcNode *dcNodeEvaluator_evaluateSuper(dcNodeEvaluator *_evaluator,
                                      dcNode *_superNode)
{
    return dcClass_getOrInstantiateSuperNode
        (dcNodeEvaluator_getCurrentSelf(_evaluator));
}

dcNode *dcNodeEvaluator_evaluateSymbol(dcNodeEvaluator *_evaluator,
                                       dcNode *_symbolNode)
{
    return dcKernelClass_getOrCreateSymbol(dcSymbol_getSymbol(_symbolNode));
}

dcNode *dcNodeEvaluator_evaluateSynchronized(dcNodeEvaluator *_evaluator,
                                             dcNode *_synchronized)
{
    dcNode *synchronized = dcNodeEvaluator_evaluate
        (_evaluator, dcSynchronized_getIdentifier(_synchronized));
    uint32_t marked = dcNodeEvaluator_pushMark(_evaluator, synchronized);
    dcNode *result = NULL;

    if (synchronized != NULL)
    {
        dcClass_lock(synchronized);
        result = dcNodeEvaluator_evaluate
            (_evaluator, dcSynchronized_getStatement(_synchronized));
        dcClass_unlock(synchronized);
    }

    dcNodeEvaluator_popMarks(_evaluator, marked);
    return result;
}

static dcNode *evaluateCatches(dcNodeEvaluator *_evaluator,
                               dcList *_catches,
                               dcNode *_exception,
                               bool *_foundBlock)
{
    dcListElement *that = NULL;
    dcNode *result = NULL;

    for (that = _catches->head; that != NULL; that = that->next)
    {
        dcCatchBlock *catchBlock = CAST_CATCHBLOCK(that->object);

        if (catchBlock->type != NULL)
        {
            // do need to check types //
            dcNode *type =
                dcNodeEvaluator_evaluate(_evaluator, catchBlock->type);

            if (type == NULL)
            {
                // exception occurred
                result = NULL;
                break;
            }
            else
            {
                //
                // check if the exception type matches
                //
                //   eg
                //     catch(MyException e)
                //
                // verify that the thrown exception is of type 'MyException'
                //
                if (dcClass_isKindOfTemplate(_exception,
                                             dcClass_getTemplate(type)))
                {
                    *_foundBlock = true;
                }
            }
        }
        else
        {
            //
            // don't need to check types
            //
            //   eg
            //     catch (e)
            //
            *_foundBlock = true;
        }

        if (*_foundBlock)
        {
            //
            // <stuff> the scope
            //
            //   eg, for:
            //     catch (e)
            //
            //   stuff the exception into a variable named 'e'
            //
            dcNode *scopeToPush = dcScope_createNode();
            const char *identifierName =
                dcIdentifier_getName(catchBlock->identifier);

            dcScope_setObject(CAST_SCOPE(scopeToPush),
                              _exception,
                              identifierName,
                              NO_FLAGS);
            // </stuff> //

            dcObjectStackList_pushScope(_evaluator->objectStackList,
                                        scopeToPush);
            result = dcNodeEvaluator_evaluate
                (_evaluator, catchBlock->statement);

            dcObjectStackList_popScope(_evaluator->objectStackList, DC_DEEP);
            break;
        }
    }

    return result;
}

dcNode *dcNodeEvaluator_evaluateTryBlock(dcNodeEvaluator *_evaluator,
                                         dcNode *_tryNode)
{
    dcTryBlock *tryBlock = CAST_TRYBLOCK(_tryNode);
    dcNode *result = dcNodeEvaluator_evaluate(_evaluator, tryBlock->statement);
    uint32_t pushed = dcNodeEvaluator_pushMark(_evaluator, result);
    dcNode *exception = _evaluator->exception;

    if (exception != NULL)
    {
        pushed += dcNodeEvaluator_pushMark(_evaluator, exception);
        dcNodeEvaluator_clearException(_evaluator, DC_SHALLOW);
        dcList *catches = tryBlock->catches;
        bool foundBlock = false;

        if (! (dcClass_isKindOfTemplate
               (exception, dcSystem_getAbortExceptionClassTemplate())))
        {
            result = evaluateCatches(_evaluator,
                                     catches,
                                     exception,
                                     &foundBlock);
        }

        // check if the catch blocks themselves produced an exception
        if (! foundBlock
            && _evaluator->exception == NULL)
        {
            // no suitable catch block was found, set the exception again //
            dcNodeEvaluator_setException(_evaluator, exception, false);
        }
    }
    else
    {
        dcError_assert(result != NULL);
    }

    dcNodeEvaluator_popMarks(_evaluator, pushed);
    return result;
}

#ifdef TAFFY_WINDOWS
dcNode *dcNodeEvaluator_evaluateHandle(dcNodeEvaluator *_evaluator,
                                       void *_handle,
                                       const char *_libraryName)
{
    return NULL;
}
#else
dcNode *dcNodeEvaluator_evaluateHandle(dcNodeEvaluator *_evaluator,
                                       void *_handle,
                                       const char *_libraryName)
{
    dcNode *result = dcYesClass_getInstance();
    char *getTemplateFunctionString =
        dcLexer_sprintf("dc%sClass_getTemplate", _libraryName);
    dcClass_getTemplateFunction function =
        (dcClass_getTemplateFunction)dlsym(_handle, getTemplateFunctionString);
    bool invalidLibrary = false;
    char *error = dlerror();

    if (error != NULL)
    {
        invalidLibrary = true;
    }
    else
    {
        dcClassTemplate *classTemplate = function();

        if (classTemplate == NULL)
        {
            invalidLibrary = true;
        }
        else
        {
            // got good data, register the template //
            dcError_assert(dcClassManager_registerClassTemplate
                           (classTemplate, NULL, true, &result)
                           == TAFFY_SUCCESS);
        }
    }

    if (invalidLibrary)
    {
        // exception time, baby //
        dcInvalidLibraryExceptionClass_throwObject(_libraryName);
        result = NULL;
        dlclose(_handle);
    }
    // else don't close the handle, as it invalidates the memory //

    dcMemory_free(getTemplateFunctionString);
    return result;
}
#endif // TAFFY_WINDOWS

dcNode *dcNodeEvaluator_evaluateThrow(dcNodeEvaluator *_evaluator,
                                      dcNode *_throwNode)
{
    dcNode *evaluateResult =
        dcNodeEvaluator_evaluate(_evaluator, CAST_THROW(_throwNode));

    if (evaluateResult != NULL)
    {
        uint32_t marked = dcNodeEvaluator_pushMark(_evaluator, evaluateResult);
        dcNode *copyOrNot = dcNode_copyIfTemplate(evaluateResult);
        dcNodeEvaluator_popMarks(_evaluator, marked);
        dcNodeEvaluator_setException(_evaluator, copyOrNot, true);
    }
    // else, another exception occurred //

    // we indicate an exception occurred by returning NULL
    return NULL;
}

dcNode *dcNodeEvaluator_evaluateExit(dcNodeEvaluator *_evaluator,
                                     dcNode *_exitNode)
{
    SET_BITS(_evaluator->flags, NODE_EVALUATOR_EXIT, true);
    return NULL;
}

static dcNode *evaluateTaffyCMethodProcedure(dcNodeEvaluator *_evaluator,
                                             dcNode *_receiver,
                                             dcNode *_procedure,
                                             dcList *_arguments)
{
    dcNode *result = NULL;
    dcMethodHeader *header = dcProcedureClass_getMethodHeader(_procedure);
    dcClassTemplate **argumentTypes =
        dcMethodHeader_getCDefinitionArgumentTypes(header);
    const char **suppliedArguments =
        dcMethodHeader_getCDefinitionSuppliedArguments(header);
    bool failure = false;
    dcArray *methodArguments = NULL;

    if (argumentTypes == NULL)
    {
        // blank, use 0?
        methodArguments = dcArray_createWithSize(1);
    }
    else
    {
        uint32_t argumentTypesSize =
            dcMethodHeader_getCDefinitionArgumentTypesSize(header);
        methodArguments = dcArray_createWithSize(argumentTypesSize);
        uint32_t i = 0;
        uint32_t j = 0; // for the supplied arguments
        dcListElement *that = _arguments->head;

        for (i = 0; i < argumentTypesSize; i++)
        {
            dcClassTemplate *type = argumentTypes[i];
            dcError_assert(type != NULL);

            if (type == dcSuppliedArgumentClass_getTemplate())
            {
                dcNode *suppliedArgument =
                    dcNode_register(dcStringClass_createObject
                                    (suppliedArguments[j], true));
                dcGraphData_copyPosition(_receiver, suppliedArgument);
                dcArray_add(methodArguments, suppliedArgument);
                j++;
            }
            else
            {
                if (that == NULL)
                {
                    dcNodeEvaluator_pushCallStackPosition(_evaluator);
                    dcInvalidNumberArgumentsExceptionClass_throwObject
                        (argumentTypesSize,
                         _arguments->size);
                    dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
                    failure = true;
                    break;
                }

                dcNode *casted = dcClass_castNode(that->object, type, true);

                if (casted != NULL)
                {
                    dcArray_add(methodArguments, casted);
                    that = that->next;
                }
                else
                {
                    failure = true;
                    break;
                }
            }
        }
    }

    if (failure)
    {
        dcArray_free(&methodArguments, DC_SHALLOW);
    }
    else
    {
        uint32_t markSize =
            dcNodeEvaluator_pushArrayToMark(_evaluator, methodArguments);

        dcTaffyCMethodPointer methodPointer =
            CAST_TAFFY_C_METHOD_POINTER(dcProcedureClass_getBody(_procedure));

        // finally call the method!
        result = methodPointer(_receiver, methodArguments);

        // if the C method returned something like an identifier
        // or dcNil, evaluate it before returning
        if (result != NULL && ! IS_CLASS(result))
        {
            result = dcNodeEvaluator_evaluate(_evaluator, result);
        }

        if (result != NULL
            && dcClass_isObject(result)
            && dcClassTemplate_isAtomic(dcClass_getTemplate(result)))
        {
            result = dcNode_register(dcNode_copy(result, DC_DEEP));
        }

        // set the return value so it is marked //
        dcNodeEvaluator_setReturnValue(_evaluator, result);
        dcNodeEvaluator_popMarks(_evaluator, markSize);
    }

    dcArray_free(&methodArguments, DC_SHALLOW);
    return result;
}

void dcNodeEvaluator_initializeSelf(dcNodeEvaluator *_evaluator)
{
    dcNode *newSelf = dcMainClass_getInstance();
    dcError_assert(newSelf != NULL);
    dcObjectStack_setSelf
        (dcObjectStackList_getTailObjectStack(_evaluator->objectStackList),
         newSelf,
         true);
}

dcNode *dcNodeEvaluator_evaluateProcedure(dcNodeEvaluator *_evaluator,
                                          dcNode *_receiver,
                                          dcNode *_procedure,
                                          dcScopeDataFlags _flags,
                                          dcList *_arguments)
{
    dcNode *result = NULL;
    dcMethodHeader *header =
        dcProcedureClass_getMethodHeader(_procedure);
    int markCount = 0;
    markCount += dcNodeEvaluator_pushMark(_evaluator, _receiver);
    markCount += dcNodeEvaluator_pushMark(_evaluator, _procedure);
    markCount += dcNodeEvaluator_pushListToMark(_evaluator, _arguments);
    dcList *evaluatedArguments = NULL;
    uint32_t lineNumberSave = _evaluator->lineNumber;
    dcStringId filenameIdSave = _evaluator->filenameId;
    bool isConst = false;
    dcListElement *that;

    dcNode *receiver =
        (_receiver == NULL
         ? dcNodeEvaluator_getCurrentSelf(_evaluator)
         : (_evaluator->printingFunction > 0
            ? _receiver
            : dcNodeEvaluator_evaluate(_evaluator, _receiver)));

    if (receiver == NULL)
    {
        // exception is already set
        goto kickout;
    }

    markCount += dcNodeEvaluator_pushMark(_evaluator, receiver);

    evaluatedArguments = (dcNodeEvaluator_evaluateUninitializedList
                          (_evaluator, _arguments, false));

    if (evaluatedArguments == NULL)
    {
        // exception is already set
        goto kickout;
    }

    for (that = evaluatedArguments->head; that != NULL; that = that->next)
    {
        if (IS_CLASS(that->object) && dcClass_isObject(that->object))
        {
            that->object = dcClass_copyIfAtomic(that->object);
        }
    }

    markCount += dcNodeEvaluator_pushListToMark(_evaluator, evaluatedArguments);
    _evaluator->lineNumber = lineNumberSave;
    _evaluator->filenameId = filenameIdSave;

    if ((_flags & SCOPE_DATA_CONST) != 0)
    {
        isConst = true;
    }

    if (! isConst
        && (dcObjectStackList_isObjectConst
            (_evaluator->objectStackList, receiver)))
    {
        dcNonConstantUseOfConstantExceptionClass_throwObject
            (dcClass_getName(receiver));
    }
    // make sure we didn't run out of "stack space"
    else if (_evaluator->callStack->size > _evaluator->maxStackDepth)
    {
        dcStackOverflowExceptionClass_throwObject();
    }
    else
    {
        bool exception = false;
        // update the object stack //
        bool locked = false;
        dcObjectStack *stack = dcObjectStack_create();
        dcObjectStackList_pushObjectStack(_evaluator->objectStackList, stack);
        dcObjectStack_pushScope(stack, dcScope_createNode());
        dcObjectStack_setSelf(stack, receiver, isConst);
        stack->breakthrough = (_flags & SCOPE_DATA_BREAKTHROUGH) != 0;

        if ((_flags & SCOPE_DATA_SYNCHRONIZED) != 0)
        {
            locked = true;
            dcGarbageCollector_nodeEvaluatorDown();
            dcClass_lock(receiver);
            dcGarbageCollector_nodeEvaluatorBlockUp();
        }

        if ((_flags & SCOPE_DATA_SYNCHRONIZED_READ) != 0)
        {
            dcResult hashResult =
                dcHash_getValueWithKeys(_evaluator->writeLockedObjects,
                                        NULL,
                                        (dcHashType)(size_t)receiver,
                                        NULL);
            assert(hashResult != TAFFY_EXCEPTION);

            if (hashResult == TAFFY_SUCCESS)
            {
                dcDeadlockExceptionClass_throwObject();
                exception = true;
            }
            else
            {
                dcGarbageCollector_nodeEvaluatorDown();
                dcClass_lockForRead(receiver);
                assert(dcHash_setValueWithHashValue
                       (_evaluator->readLockedObjects,
                        NULL,
                        (dcHashType)(size_t)receiver,
                        receiver)
                       == TAFFY_SUCCESS);
                dcGarbageCollector_nodeEvaluatorBlockUp();
            }
        }
        else if ((_flags & SCOPE_DATA_SYNCHRONIZED_WRITE) != 0)
        {
            dcResult hashResult =
                dcHash_getValueWithKeys(_evaluator->readLockedObjects,
                                        NULL,
                                        (dcHashType)(size_t)receiver,
                                        NULL);
            assert(hashResult != TAFFY_EXCEPTION);

            if (hashResult == TAFFY_SUCCESS)
            {
                dcDeadlockExceptionClass_throwObject();
                exception = true;
            }
            else
            {
                dcGarbageCollector_nodeEvaluatorDown();
                assert(dcHash_setValueWithHashValue
                       (_evaluator->writeLockedObjects,
                        NULL,
                        (dcHashType)(size_t)receiver,
                        receiver)
                       == TAFFY_SUCCESS);
                dcClass_lockForWrite(receiver);
                dcGarbageCollector_nodeEvaluatorBlockUp();
            }
        }

        if (exception)
        {
            result = NULL;
        }
        else
        {
            // determine if this is a c or taffy function,
            // and take appropriate action
            if (header->type == METHOD_HEADER_TAFFY)
            {
                result = dcNodeEvaluator_evaluateProcedureGuts
                    (_evaluator,
                     dcProcedureClass_getBody(_procedure),
                     dcProcedureClass_getMethodHeader(_procedure),
                     evaluatedArguments);
            }
            else
            {
                result = evaluateTaffyCMethodProcedure(_evaluator,
                                                       _receiver,
                                                       _procedure,
                                                       evaluatedArguments);
            }
        }

        dcNodeEvaluator_setReturnValue(_evaluator, result);

        // propagate the return if we're in a breakthrough method
        if ((_flags & SCOPE_DATA_BREAKTHROUGH) == 0)
        {
            dcNodeEvaluator_setReturning(_evaluator, false);
        }

        dcObjectStack_popScope(stack, DC_DEEP);
        dcObjectStackList_popObjectStack(_evaluator->objectStackList,
                                         DC_DEEP);

        TAFFY_DEBUG(dcError_assert
                    (dcSystem_isInBootstrap()
                     || (dcNodeEvaluator_getCurrentSelf(_evaluator)
                         != NULL)));

        if (locked)
        {
            dcClass_unlock(_receiver);
        }

        if ((_flags & SCOPE_DATA_SYNCHRONIZED_READ) != 0
            || (_flags & SCOPE_DATA_SYNCHRONIZED_WRITE) != 0)
        {
            dcGarbageCollector_nodeEvaluatorDown();
            dcParser_lock();
            dcHash_removeValueWithHashValue(_evaluator->readLockedObjects,
                                            NULL,
                                            (dcHashType)(size_t)receiver,
                                            NULL,
                                            DC_SHALLOW);
            dcHash_removeValueWithHashValue(_evaluator->writeLockedObjects,
                                            NULL,
                                            (dcHashType)(size_t)receiver,
                                            NULL,
                                            DC_SHALLOW);
            dcParser_unlock();
            dcClass_unlockReadWriteLock(receiver);
            dcGarbageCollector_nodeEvaluatorBlockUp();
        }
    }

    // un-update the receivers stack, don't free //
kickout:
    dcNodeEvaluator_popMarks(_evaluator, markCount);
    dcList_free(&evaluatedArguments, DC_SHALLOW);
    return result;
}

dcNode *dcNodeEvaluator_callMethod(dcNodeEvaluator *_evaluator,
                                   dcNode *_receiver,
                                   const char *_methodName)
{
    dcList *arguments = dcList_create();
    dcNode *result = dcNodeEvaluator_callMethodWithArguments(_evaluator,
                                                             _receiver,
                                                             _methodName,
                                                             arguments,
                                                             false);
    dcList_free(&arguments, DC_SHALLOW);
    return result;
}

dcNode *dcNodeEvaluator_forceCallMethod(dcNodeEvaluator *_evaluator,
                                        dcNode *_receiver,
                                        const char *_methodName)
{
    dcList *arguments = dcList_create();
    dcNode *result = dcNodeEvaluator_callMethodWithArguments(_evaluator,
                                                             _receiver,
                                                             _methodName,
                                                             arguments,
                                                             true);
    dcList_free(&arguments, DC_SHALLOW);
    return result;
}

dcNode *dcNodeEvaluator_callEvaluatedMethod(dcNodeEvaluator *_evaluator,
                                            const char *_receiverName,
                                            const char *_functionName,
                                            dcNode *_argument,
                                            ...)
{
    dcNode *result = NULL;
    bringUp(_evaluator);

    if (dcNodeEvaluator_canContinueEvaluating(_evaluator))
    {
        dcNode *identifier = dcIdentifier_createNode(_receiverName, NO_FLAGS);
        dcNode *receiver = dcNodeEvaluator_evaluate(_evaluator, identifier);
        va_list vaList;
        va_start(vaList, _argument);
        dcList *arguments = dcList_create();
        dcNode *iterator = va_arg(vaList, dcNode*);

        if (_argument != NULL)
        {
            dcList_push(arguments, _argument);

            while (iterator != NULL)
            {
                dcList_push(arguments, iterator);
                iterator = va_arg(vaList, dcNode*);
            }
        }

        if (receiver != NULL)
        {
            result = dcNodeEvaluator_callMethodWithArguments(_evaluator,
                                                             receiver,
                                                             _functionName,
                                                             arguments,
                                                             false);
        }

        dcList_free(&arguments, DC_FLOATING);
        dcNode_free(&identifier, DC_DEEP);
    }

    takeDown(_evaluator);
    return result;
}

dcNode *dcNodeEvaluator_callMethodWithArgument(dcNodeEvaluator *_evaluator,
                                               dcNode *_receiver,
                                               const char *_methodName,
                                               dcNode *_argument)
{
    dcList *arguments = dcList_createWithObjects(_argument, NULL);
    dcNode *result = dcNodeEvaluator_callMethodWithArguments(_evaluator,
                                                             _receiver,
                                                             _methodName,
                                                             arguments,
                                                             false);
    dcList_free(&arguments, DC_SHALLOW);
    return result;
}

dcNode *dcNodeEvaluator_callMethodWithArguments(dcNodeEvaluator *_evaluator,
                                                dcNode *_receiver,
                                                const char *_methodName,
                                                dcList *_arguments,
                                                bool _force)
{
    dcNode *result = NULL;
    dcClassTemplate *foundTemplate = NULL;
    dcNode *method = NULL;
    uint32_t marks = dcNodeEvaluator_pushListToMark(_evaluator, _arguments);
    marks += dcNodeEvaluator_pushMark(_evaluator, _receiver);

    bringUp(_evaluator);
    uint32_t lineNumberSave = _evaluator->lineNumber;
    dcStringId filenameIdSave = _evaluator->filenameId;
    dcNode *receiver = (_evaluator->printingFunction > 0
                        ? _receiver
                        : dcNodeEvaluator_evaluate(_evaluator, _receiver));
    _evaluator->lineNumber = lineNumberSave;
    _evaluator->filenameId = filenameIdSave;

    if (receiver != NULL)
    {
        // attempt to look up the method //
        dcGarbageCollector_nodeEvaluatorDown();
        dcParser_lock();
        dcGarbageCollector_nodeEvaluatorBlockUp();

        method = dcClass_getScopeDataForMethod
            (receiver,
             _methodName,
             true, // search up
             (_force
              ? receiver
              : dcNodeEvaluator_getCurrentSelf(_evaluator)), // the requestor
             &foundTemplate,
             NULL);

        dcParser_unlock();

        if (method == NULL)
        {
            // FAILURE //
            if (_evaluator->exception == NULL)
            {
                // push the current position and set the exception
                dcNodeEvaluator_pushCallStackPosition(_evaluator);
                dcUnidentifiedMethodExceptionClass_throwObject
                    (dcClass_getName(receiver), _methodName);
                dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
            }
        }
        else
        {
            dcNode *procedure = dcScopeData_getObject(method);
            dcScopeDataFlags flags = dcScopeData_getFlags(method);

            if ((flags & SCOPE_DATA_NO_CAST) == 0
                && (dcProcedureClass_getMethodHeader(procedure)->type
                    == METHOD_HEADER_C))
            {
                receiver = dcClass_castNode(receiver, foundTemplate, true);
            }

            dcError_assert(receiver != NULL);

            dcNodeEvaluator_pushCallStack(_evaluator,
                                          dcProcedureClass_getMethodHeader
                                          (procedure)->name);
            result = dcNodeEvaluator_evaluateProcedure
                (_evaluator, receiver, procedure, flags, _arguments);
            dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
        }
    }

    dcNodeEvaluator_setReturning(_evaluator, false);
    takeDown(_evaluator);
    dcNodeEvaluator_popMarks(_evaluator, marks);

    return result;
}

uint32_t dcNodeEvaluator_pushMark(dcNodeEvaluator *_evaluator,
                                  dcNode *_nodeToMark)
{
    uint32_t result = 0;

    if (_nodeToMark != NULL && dcNode_isRegistered(_nodeToMark))
    {
        result = 1;
        //
        // only one thread should be accessing this at one time
        // but do this to squelch helgrind
        //
        lock(_evaluator);
        dcList_push(_evaluator->evaluatedToMark, _nodeToMark);
        unlock(_evaluator);
    }

    return result;
}

void dcNodeEvaluator_popMark(dcNodeEvaluator *_evaluator)
{
    //
    // only one thread should be accessing this at one time
    // but do this to squelch helgrind
    //
    lock(_evaluator);
    dcList_pop(_evaluator->evaluatedToMark, DC_FLOATING);
    unlock(_evaluator);
}

void dcNodeEvaluator_popMarkShell(dcNodeEvaluator *_evaluator)
{
    lock(_evaluator);
    dcNode *shell = dcList_getTail(_evaluator->evaluatedToMark);
    dcList_pop(_evaluator->evaluatedToMark, DC_FLOATING);
    dcNode_freeShell(&shell);
    unlock(_evaluator);
}

void dcNodeEvaluator_popMarks(dcNodeEvaluator *_evaluator, uint32_t _count)
{
    uint32_t i;
    dcError_assert(_evaluator->evaluatedToMark->size >= _count);

    for (i = 0; i < _count; i++)
    {
        dcNodeEvaluator_popMark(_evaluator);
    }
}

uint32_t dcNodeEvaluator_pushListToMark(dcNodeEvaluator *_evaluator,
                                        dcList *_arguments)
{
    dcListElement *that;
    uint32_t added = 0;

    for (that = _arguments->head; that != NULL; that = that->next)
    {
        added += dcNodeEvaluator_pushMark(_evaluator, that->object);
    }

    return added;
}

uint32_t dcNodeEvaluator_pushArrayToMark(dcNodeEvaluator *_evaluator,
                                         dcArray *_array)
{
    uint32_t i;
    uint32_t added = 0;

    for (i = 0; i < _array->size; i++)
    {
        added += dcNodeEvaluator_pushMark(_evaluator, _array->objects[i]);
    }

    return added;
}

dcNode *dcNodeEvaluator_evaluateProcedureGuts(dcNodeEvaluator *_evaluator,
                                              dcNode *_body,
                                              dcMethodHeader *_methodHeader,
                                              dcList *_arguments)
{
    // the return value //
    dcNode *result = NULL;
    bool failure = false;

    // create the scope shells //
    uint32_t marked = dcNodeEvaluator_pushMark(_evaluator, _body);
    dcList *methodHeaderArguments = dcMethodHeader_getArguments(_methodHeader);

    // methodHeaderArguments may be null //
    if (methodHeaderArguments != NULL)
    {
        // sanity //
        if (methodHeaderArguments->size != _arguments->size)
        {
            dcNodeEvaluator_pushCallStackPosition(_evaluator);
            dcInvalidNumberArgumentsExceptionClass_throwObject
                (methodHeaderArguments->size, _arguments->size);
            dcNodeEvaluator_popCallStack(_evaluator, DC_DEEP);
            failure = true;
        }
        else
        {
            dcScope *tailScope = dcObjectStackList_getTailScope
                (_evaluator->objectStackList);

            //
            // assign the arguments to the scope
            // methodHeaderArguments contains the variable names
            // _arguments contains the variable values
            //
            dcListElement *thisKey = NULL;
            dcListElement *thisValue = NULL;

            for (thisKey = methodHeaderArguments->head,
                     thisValue = _arguments->head;
                 thisKey != NULL;
                 thisKey = thisKey->next, thisValue = thisValue->next)
            {
                // evaluate preValue //
                dcNode *evaluatedValue = dcNodeEvaluator_evaluate
                    (_evaluator, thisValue->object);

                // check for exceptions //
                if (evaluatedValue == NULL)
                {
                    failure = true;
                    break;
                }

                dcError_assert(IS_CLASS(evaluatedValue));
                evaluatedValue = dcNode_copyIfTemplate(evaluatedValue);
                dcScope_setObject(tailScope,
                                  evaluatedValue,
                                  dcIdentifier_getName(thisKey->object),
                                  NO_FLAGS);
            }
        }
    }

    if (! failure)
    {
        //
        // evaluate the body
        //   eg
        //     (@) foo { <body> }
        //
        result = dcNodeEvaluator_evaluate(_evaluator, _body);

        if (result != NULL)
        {
            // set the return value, so it's not wrongly freed //
            dcNodeEvaluator_setReturnValue(_evaluator, result);
        }
    }

    // pop scope shell //
    dcNodeEvaluator_popMarks(_evaluator, marked);

    return result;
}

void dcNodeEvaluator_pushCallStackNode(dcNodeEvaluator *_evaluator,
                                       dcNode *_callStackNode)
{
    dcList_push(_evaluator->callStack, _callStackNode);
}

void dcNodeEvaluator_pushCallStackPosition(dcNodeEvaluator *_evaluator)
{
    dcNodeEvaluator_pushCallStack(_evaluator, NULL);
}

void dcNodeEvaluator_pushCallStack(dcNodeEvaluator *_evaluator,
                                   const char *_methodName)
{
    // update the call stack //
    dcNodeEvaluator_pushCallStackNode
        (_evaluator,
         dcCallStackData_createNode
         (_methodName,
          _evaluator->filenameId,
          _evaluator->lineNumber));
}

void dcNodeEvaluator_pushCallStackFromGraphDataNode(dcNodeEvaluator *_evaluator,
                                                    dcNode *_node,
                                                    const char *_what)
{
    dcGraphData *graphData = CAST_GRAPH_DATA(_node);
    dcNodeEvaluator_pushCallStackNode
        (_evaluator,
         dcCallStackData_createNode
         (_what,
          graphData->filenameId,
          graphData->lineNumber));
}

dcNode *dcNodeEvaluator_popCallStack(dcNodeEvaluator *_evaluator,
                                     dcDepth _depth)
{
    return dcList_pop(_evaluator->callStack, _depth);
}

dcList *dcNodeEvaluator_evaluateUninitializedList(dcNodeEvaluator *_evaluator,
                                                  const dcList *_list,
                                                  bool _copy)
{
    dcList *initializedObjects = dcList_create();
    dcList *result = initializedObjects;
    int markPush = 0;

    if (_list != NULL)
    {
        dcListElement *that = NULL;

        for (that = _list->head; that != NULL; that = that->next)
        {
            markPush += dcNodeEvaluator_pushMark(_evaluator, that->object);
        }

        for (that = _list->head; that != NULL; that = that->next)
        {
            dcNode *evaluatedObject =
                dcNodeEvaluator_evaluate(_evaluator, that->object);

            if (evaluatedObject != NULL)
            {
                markPush +=
                    dcNodeEvaluator_pushMark(_evaluator, evaluatedObject);

                if (_copy)
                {
                    evaluatedObject =
                        dcNode_copyIfTemplate(evaluatedObject);

                    // the copy above might not have produced a new object,
                    // but better safe than sorry
                    markPush +=
                        dcNodeEvaluator_pushMark(_evaluator, evaluatedObject);
                }

                dcList_push(initializedObjects, evaluatedObject);
            }
            else
            {
                // exception occurred and is already set //
                dcList_free(&initializedObjects, DC_SHALLOW);
                result = NULL;
                break;
            }
        }
    }

    dcNodeEvaluator_popMarks(_evaluator, markPush);
    return result;
}

dcArray *dcNodeEvaluator_evaluateUninitializedArray(dcNodeEvaluator *_evaluator,
                                                    const dcArray *_array)
{
    dcArray *initializedObjects = dcArray_createWithSize(_array->size + 3);
    dcArray *result = initializedObjects;
    uint32_t i = 0;
    uint32_t markCount = 0;

    for (i = 0; i < _array->size; i++)
    {
        dcNode *preObject = dcArray_get(_array, i);
        dcNode *evaluatedObject =
            dcNodeEvaluator_evaluate(_evaluator, preObject);
        markCount += dcNodeEvaluator_pushMark(_evaluator, evaluatedObject);

        if (evaluatedObject != NULL)
        {
            evaluatedObject = dcClass_copyIfTemplateOrAtomic(evaluatedObject);
            markCount += dcNodeEvaluator_pushMark(_evaluator, evaluatedObject);
            dcArray_add(initializedObjects, evaluatedObject);
        }
        else
        {
            // exception occurred //
            dcArray_free(&initializedObjects, DC_SHALLOW);
            result = NULL;
            break;
        }
    }

    dcNodeEvaluator_popMarks(_evaluator, markCount);
    return result;
}

void dcNodeEvaluator_setException(dcNodeEvaluator *_evaluator,
                                  dcNode *_exception,
                                  bool _setExceptionCallStack)
{
    if (_exception != NULL)
    {
        _evaluator->exception = dcNode_register(_exception);

        if (_setExceptionCallStack)
        {
            dcList_free(&_evaluator->exceptionCallStack, DC_DEEP);
            _evaluator->exceptionCallStack =
                dcList_copy(_evaluator->callStack, DC_DEEP);
        }

        if (dcClass_isKindOfTemplate(_evaluator->exception,
                                     dcSystem_getAbortExceptionClassTemplate()))
        {
            _evaluator->abortExecutionState = NODE_EVALUATOR_ABORT_PLEASE;
        }
    }
}

static void *clearException(void *_evaluator)
{
    dcNodeEvaluator *evaluator = (dcNodeEvaluator *)_evaluator;
    evaluator->exception = NULL;
    dcList_clear(evaluator->exceptionCallStack, DC_DEEP);
    return NULL;
}

void dcNodeEvaluator_clearException(dcNodeEvaluator *_evaluator,
                                    dcDepth _depth)
{
    dcNodeEvaluator_synchronizeFunctionCall(_evaluator,
                                            &clearException,
                                            _evaluator);
}

char *dcNodeEvaluator_displayCallStack(dcNodeEvaluator *_evaluator)
{
    dcListIterator *callStackIterator =
        dcList_createTailIterator(_evaluator->callStack);
    dcNode *callStackNode = NULL;
    uint32_t callStackSize = _evaluator->callStack->size;
    dcString callStackString;
    dcString_initialize(&callStackString, 50 * callStackSize);

    while ((callStackNode = dcListIterator_getPrevious(callStackIterator))
           != NULL)
    {
        dcString_append(&callStackString,
                        "%s\n",
                        dcNode_display(callStackNode));
    }

    return callStackString.string;
}

//
// TODO: Make me faster
//
dcResult dcNodeEvaluator_compareObjects(dcNodeEvaluator *_evaluator,
                                        dcNode *_left,
                                        dcNode *_right,
                                        dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_SUCCESS;
    dcTaffyOperator tests[] = {TAFFY_EQUALS,
                               TAFFY_LESS_THAN,
                               TAFFY_GREATER_THAN};
    uint32_t i;
    bringUp(_evaluator);
    *_compareResult = TAFFY_UNKNOWN_OPERATOR;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        if (dcArrayClass_isMe(_left)
            && dcArrayClass_isMe(_right)
            && ! dcArrayClass_isInitialized(_left)
            && ! dcArrayClass_isInitialized(_right))
        {
            result = dcArray_compare(dcArrayClass_getObjects(_left),
                                     dcArrayClass_getObjects(_right),
                                     _compareResult);
        }
        else
        {
            dcNode *evaluationResult = (dcNodeEvaluator_callMethodWithArgument
                                        (_evaluator,
                                         _left,
                                         dcSystem_getOperatorName(tests[i]),
                                         _right));
            if (evaluationResult == NULL)
            {
                result = TAFFY_FAILURE;
                // flee, but clear the exception first, we don't need it
                dcNodeEvaluator_clearException(_evaluator, DC_DEEP);
                break;
            }
            else if (dcYesClass_isMe(evaluationResult))
            {
                *_compareResult = tests[i];
                break;
            }
        }
    }

    takeDown(_evaluator);
    return result;
}

void dcNodeEvaluator_mark(dcNodeEvaluator *_evaluator)
{
    //dcSystem_resetMarkCount();
    dcNode_mark(_evaluator->exception);
    //dcSystem_logAndResetMarkCount("_evaluator->exception");
    dcNode_mark(_evaluator->returnValue);
    //dcSystem_logAndResetMarkCount("_evaluator->returnValue");
    dcList_mark(_evaluator->evaluatedToMark);
    //dcSystem_logAndResetMarkCount("_evaluator->evaluatedToMark");
    dcObjectStackList_mark(_evaluator->objectStackList);
    //dcSystem_logAndResetMarkCount("_evaluator->objectStackList");
}

dcResult dcNodeEvaluator_hashNode(dcNode *_evaluator, dcHashType *_hashResult)
{
    return dcNode_hash(CAST_NODE_EVALUATOR(_evaluator)->threadId, _hashResult);
}

dcResult dcNodeEvaluator_compareNode(dcNode *_left,
                                     dcNode *_right,
                                     dcTaffyOperator *_compareResult)
{
    return dcNode_compare(CAST_NODE_EVALUATOR(_left)->threadId,
                          CAST_NODE_EVALUATOR(_right)->threadId,
                          _compareResult);
}

void *dcNodeEvaluator_synchronizeFunctionCall(dcNodeEvaluator *_evaluator,
                                              dcGenericFunction _function,
                                              void *_token)
{
    bringUp(_evaluator);
    void *result = _function(_token);
    takeDown(_evaluator);
    return result;
}

dcNode *dcNodeEvaluator_setPrinting(dcNodeEvaluator *_evaluator,
                                    dcNode *_toPrint,
                                    bool _yesNo)
{
    dcNode *result = NULL;
    dcNode *value = NULL;

    assert(dcHash_getValueWithKeys(_evaluator->asStringObjects,
                                   NULL,
                                   (dcHashType)(size_t)_toPrint,
                                   &value)
           != TAFFY_EXCEPTION);

    if(_yesNo)
    {
        if (value == NULL)
        {
            value = dcUnsignedInt32_createNode(1);
            assert(dcHash_setValueWithHashValue(_evaluator->asStringObjects,
                                                NULL,
                                                (dcHashType)(size_t)_toPrint,
                                                value)
                   == TAFFY_SUCCESS);
        }
        else
        {
            CAST_INT(value) += 1;
            result = dcNode_register
                (dcStringClass_createObject("...", true));
        }
    }
    else
    {
        dcError_assert(value != NULL
                       && value->type == NODE_INT
                       && CAST_INT(value) > 0);

        CAST_INT(value) -= 1;

        if (CAST_INT(value) == 0)
        {
            assert(dcHash_removeValueWithHashValue(_evaluator->asStringObjects,
                                                   NULL,
                                                   (dcHashType)(size_t)_toPrint,
                                                   NULL,
                                                   DC_DEEP)
                   == TAFFY_SUCCESS);
        }
    }

    return result;
}

void dcNodeEvaluator_incrementAbortDelay(dcNodeEvaluator *_evaluator)
{
    lock(_evaluator);
    _evaluator->abortDelay++;
    unlock(_evaluator);
}

bool dcNodeEvaluator_decrementAbortDelay(dcNodeEvaluator *_evaluator)
{
    lock(_evaluator);
    assert(_evaluator->abortDelay > 0);
    _evaluator->abortDelay--;
    bool result = false;

    if (_evaluator->abortDelay == 0
        && _evaluator->exception == NULL
        && _evaluator->abortExecutionState == NODE_EVALUATOR_ABORT_RECEIVED)
    {
        processAbort(_evaluator);
        result = true;
    }

    unlock(_evaluator);
    return result;
}
