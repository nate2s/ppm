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

#ifndef __DC_NODE_EVALUATOR_H__
#define __DC_NODE_EVALUATOR_H__

#include "dcDefines.h"
#include "dcThreadInclude.h"

enum AbortStates
{
    NODE_EVALUATOR_ABORT_NOT        = BITS(0),
    NODE_EVALUATOR_ABORT_RECEIVED   = BITS(1),
    NODE_EVALUATOR_ABORT_PROCESSING = BITS(2),
    NODE_EVALUATOR_ABORT_PLEASE     = BITS(3)
};

enum dcNodeEvaluatorState_e
{
    // return from the current function
    NODE_EVALUATOR_RETURN         = BITS(1),

    // break from the current loop
    NODE_EVALUATOR_BREAK          = BITS(2),

    // exit evaluation
    NODE_EVALUATOR_EXIT           = BITS(3),

    // the last flag :(
    NODE_EVALUATOR_LAST_FLAG      = NODE_EVALUATOR_EXIT
};

typedef uint8_t dcNodeEvaluatorState;

struct dcNodeEvaluator_t
{
    // contains dcCallStackData nodes //
    struct dcList_t *callStack;

    // the exception call stack //
    struct dcList_t *exceptionCallStack;

    // the object stack list, for selves and scopes
    struct dcObjectStackList_t *objectStackList;

    // for call stack //
    uint32_t lineNumber;

    // used for filename lookup via dcStringManager //
    uint16_t filenameId;

    dcNodeEvaluatorState flags;

    struct dcMutex_t *evaluatorMutex;

    //
    // data for execution abortion
    //
    uint16_t abortExecutionState;
    int abortExecutionSignal;

    // for evaluating imports //
    uint32_t onlyEvaluateClasses;

    uint32_t maxStackDepth;

    // id for threading //
    struct dcNode_t *threadId;
    dcTaffyThreadId simpleThreadId;

    // id for external applications //
    uint32_t simpleId;

    // used for determining if we're entering or leaving evaluation,
    // or somewhere in the middle still
    uint32_t evaluationCount;

    // oh dear
    struct dcNode_t *exception;

    // for marking purposes //
    struct dcList_t *evaluatedToMark;
    struct dcNode_t *returnValue;

    // for importing //
    uint32_t importDepth;
    struct dcList_t *deferredImports;
    // class templates to initialize after importing is done
    struct dcList_t *importReferences;
    struct dcHash_t *importReferencesCallStacks;

    struct dcHash_t *asStringObjects;
    uint32_t printingFunction;

    struct dcHash_t *readLockedObjects;
    struct dcHash_t *writeLockedObjects;

    // should we delay abort (> 0), like during flat arithmetic operations
    uint32_t abortDelay;

    bool exit;

    TAFFY_DEBUG(int32_t upCounter);
};

typedef struct dcNodeEvaluator_t dcNodeEvaluator;

// creating //
dcNodeEvaluator *dcNodeEvaluator_create(void);
dcNodeEvaluator *dcNodeEvaluator_createSystemEvaluator(void);
struct dcNode_t *dcNodeEvaluator_createShell(dcNodeEvaluator *_evaluator);

// getting //

// gets the current node evaluator's package contents list
struct dcList_t *dcNodeEvaluator_getPackageContents
    (dcNodeEvaluator *_evaluator);

struct dcScope_t *dcNodeEvaluator_getCurrentScope(dcNodeEvaluator *_evaluator,
                                                  uint16_t _scopeFlags);
struct dcNode_t *dcNodeEvaluator_getCurrentSelf(dcNodeEvaluator *_evaluator);
struct dcClassTemplate_t *dcNodeEvaluator_getCurrentSelfTemplate
    (dcNodeEvaluator *_evaluator);

const char *dcNodeEvaluator_getFilename(dcNodeEvaluator *_evaluator);
bool dcNodeEvaluator_isBreaking(dcNodeEvaluator *_evaluator);
bool dcNodeEvaluator_abortReceived(dcNodeEvaluator *_evaluator);
void dcNodeEvaluator_setReturnValue(dcNodeEvaluator *_evaluator,
                                    struct dcNode_t *_node);
struct dcNode_t *dcNodeEvaluator_getReturnValue(dcNodeEvaluator *_evaluator);

// state //
void dcNodeEvaluator_resetState(dcNodeEvaluator *_evaluator);
void dcNodeEvaluator_finishGlobalMode(dcNodeEvaluator *_evaluator);

// exiting //
void dcNodeEvaluator_setExiting(dcNodeEvaluator *_evaluator);

// suspending //

// a blocking call, waits for the evaluator to be suspended
void dcNodeEvaluator_blockForSuspend(dcNodeEvaluator *_evaluator);

// call these from somewhere else, like a thread, to signal that the
// node evaluator is suspended from somewhere else
void dcNodeEvaluator_signalSuspended(dcNodeEvaluator *_evaluator);
void dcNodeEvaluator_setResumed(dcNodeEvaluator *_evaluator);

// a command to the evaluator to resume execution
void dcNodeEvaluator_resume(dcNodeEvaluator *_evaluator);

// setting //
void dcNodeEvaluator_initializeSelf(dcNodeEvaluator *_evaluator);

void dcNodeEvaluator_setFlag(dcNodeEvaluator *_evaluator,
                             bool _yesno,
                             int _value);

void dcNodeEvaluator_setReturning(dcNodeEvaluator *_evaluator,
                                  bool _yesno);

void dcNodeEvaluator_setBreaking(dcNodeEvaluator *_evaluator,
                                 bool _yesno);

void dcNodeEvaluator_abortExecutionFromSignal(dcNodeEvaluator *_evaluator,
                                              int _signal);

void dcNodeEvaluator_clearReturnValue(dcNodeEvaluator *_evaluator);

void dcNodeEvaluator_setPosition(dcNodeEvaluator *_evaluator,
                                 struct dcNode_t *_graphDataNode);

// clearing //
void dcNodeEvaluator_clearSelfStack(dcNodeEvaluator *_evaluator);

// freeing and stuff //
void dcNodeEvaluator_free(dcNodeEvaluator **_evaluator);
void dcNodeEvaluator_clearScopes(dcNodeEvaluator *_evaluator);

//
// call stack
//

void dcNodeEvaluator_clearCallstack(dcNodeEvaluator *_evaluator);
void dcNodeEvaluator_pushCallStackPosition(dcNodeEvaluator *_evaluator);
void dcNodeEvaluator_pushCallStack(dcNodeEvaluator *_evaluator,
                                   const char *_methodName);

void dcNodeEvaluator_pushCallStackNode(dcNodeEvaluator *_evaluator,
                                       struct dcNode_t *_callStackNode);

void dcNodeEvaluator_pushCallStackFromGraphDataNode(dcNodeEvaluator *_evaluator,
                                                    struct dcNode_t *_node,
                                                    const char *_what);

struct dcNode_t *dcNodeEvaluator_popCallStack(dcNodeEvaluator *_evaluator,
                                              dcDepth _depth);

char *dcNodeEvaluator_generateExceptionText(dcNodeEvaluator *_evaluator);
char *dcNodeEvaluator_displayExceptionCallStack(dcNodeEvaluator *_evaluator);

// marking //
void dcNodeEvaluator_mark(dcNodeEvaluator *_evaluator);

// evaluating //
struct dcNode_t *dcNodeEvaluator_evaluate(dcNodeEvaluator *_evaluator,
                                          struct dcNode_t *_node);

struct dcArray_t *dcNodeEvaluator_evaluateUninitializedArray
    (dcNodeEvaluator *_evaluator, const struct dcArray_t *_listObject);

struct dcList_t *dcNodeEvaluator_evaluateUninitializedList
    (dcNodeEvaluator *_evaluator,
     const struct dcList_t *_listObject,
     bool _copy);

typedef struct dcNode_t *(*dcNodeEvaluator_evaluateHelperPointer)
    (dcNodeEvaluator *_evaluator,
     struct dcNode_t *_node);

// querying //
bool dcNodeEvaluator_hasException(dcNodeEvaluator *_evaluator);
bool dcNodeEvaluator_iHaveAnException(void);

// for interfacing with the garbage collector and loops
bool dcNodeEvaluator_canContinueEvaluating(dcNodeEvaluator *_evaluator);

//
// find a fully qualified object from the regular scope stack
//
struct dcNode_t *dcNodeEvaluator_findObject
    (dcNodeEvaluator *_evaluator,
     const char *_fullyQualifiedIdentifierName,
     bool _enableExceptionThrow);

// testing //
uint32_t dcNodeEvaluator_getEvaluatedToMarkSize
    (const dcNodeEvaluator *_evaluator);

// evaluating helpers //

#define EVALUATE_HELPER(_node_)                             \
    struct dcNode_t *_node_(dcNodeEvaluator *_evaluator,    \
                            struct dcNode_t *_node);

EVALUATE_HELPER(dcNodeEvaluator_evaluateAnd);
EVALUATE_HELPER(dcNodeEvaluator_evaluateAssignment);
EVALUATE_HELPER(dcNodeEvaluator_evaluateBreak);
EVALUATE_HELPER(dcNodeEvaluator_evaluateClass);
EVALUATE_HELPER(dcNodeEvaluator_evaluateExit);
EVALUATE_HELPER(dcNodeEvaluator_evaluateFactorial);
EVALUATE_HELPER(dcNodeEvaluator_evaluateFalse);
EVALUATE_HELPER(dcNodeEvaluator_evaluateFlatArithmetic);
EVALUATE_HELPER(dcNodeEvaluator_evaluateFor);
EVALUATE_HELPER(dcNodeEvaluator_evaluateFunctionUpdate);
EVALUATE_HELPER(dcNodeEvaluator_evaluateIdentifier);
EVALUATE_HELPER(dcNodeEvaluator_evaluateIf);
EVALUATE_HELPER(dcNodeEvaluator_evaluateImport);
EVALUATE_HELPER(dcNodeEvaluator_evaluateIn);
EVALUATE_HELPER(dcNodeEvaluator_evaluateJoin);
EVALUATE_HELPER(dcNodeEvaluator_evaluateJoinLib);
EVALUATE_HELPER(dcNodeEvaluator_evaluateGraphDataList);
EVALUATE_HELPER(dcNodeEvaluator_evaluateGraphDataTree);
EVALUATE_HELPER(dcNodeEvaluator_evaluateMethodCall);
EVALUATE_HELPER(dcNodeEvaluator_evaluateNew);
EVALUATE_HELPER(dcNodeEvaluator_evaluateNil);
EVALUATE_HELPER(dcNodeEvaluator_evaluateNotEqual);
EVALUATE_HELPER(dcNodeEvaluator_evaluateNumberTemplate);
EVALUATE_HELPER(dcNodeEvaluator_evaluateOr);
EVALUATE_HELPER(dcNodeEvaluator_evaluatePackage);
EVALUATE_HELPER(dcNodeEvaluator_evaluateRemoteExecution);
EVALUATE_HELPER(dcNodeEvaluator_evaluateReturn);
EVALUATE_HELPER(dcNodeEvaluator_evaluateSelf);
EVALUATE_HELPER(dcNodeEvaluator_evaluateSuper);
EVALUATE_HELPER(dcNodeEvaluator_evaluateSymbol);
EVALUATE_HELPER(dcNodeEvaluator_evaluateSynchronized);
EVALUATE_HELPER(dcNodeEvaluator_evaluateThrow);
EVALUATE_HELPER(dcNodeEvaluator_evaluateTrue);
EVALUATE_HELPER(dcNodeEvaluator_evaluateTryBlock);
EVALUATE_HELPER(dcNodeEvaluator_evaluateUninitializedArrayClass);
EVALUATE_HELPER(dcNodeEvaluator_evaluateUninitializedHashClass);
EVALUATE_HELPER(dcNodeEvaluator_evaluateUninitializedMatrixClass);
EVALUATE_HELPER(dcNodeEvaluator_evaluateUninitializedPairClass);
EVALUATE_HELPER(dcNodeEvaluator_evaluateUninitializedStringClass);
EVALUATE_HELPER(dcNodeEvaluator_evaluateUpSelf);
EVALUATE_HELPER(dcNodeEvaluator_evaluateWhile);

// misc helpers //
struct dcNode_t *dcNodeEvaluator_evaluateCatches(dcNodeEvaluator *_evaluator,
                                                 struct dcList_t *_catches,
                                                 struct dcNode_t *_exception,
                                                 bool *_foundBlock);

struct dcNode_t *dcNodeEvaluator_evaluateUnidentifiedJoin
    (dcNodeEvaluator *_evaluator,
     const char *_filename);

struct dcNode_t *dcNodeEvaluator_evaluateTaffyJoin(dcNodeEvaluator *_evaluator,
                                                   const char *_filename);

struct dcNode_t *dcNodeEvaluator_evaluateCJoin(dcNodeEvaluator *_evaluator,
                                               const char *_libraryName);

struct dcNode_t *dcNodeEvaluator_evaluateHandle(dcNodeEvaluator *_evaluator,
                                                void *_handle,
                                                const char *_libraryName);

struct dcNode_t *dcNodeEvaluator_evaluateJoinLib_helper
    (dcNodeEvaluator *_evaluator,
     const char *_libraryName);

// exception stuff //
void dcNodeEvaluator_setException(dcNodeEvaluator *_evaluator,
                                  struct dcNode_t *_exception,
                                  bool _resetExceptionCallStack);

void dcNodeEvaluator_clearException(dcNodeEvaluator *_evaluator,
                                    dcDepth _depth);

void dcNodeEvaluator_displayException(dcNodeEvaluator *_evaluator);

// manual method calling //
struct dcNode_t *dcNodeEvaluator_callMethod(dcNodeEvaluator *_evaluator,
                                            struct dcNode_t *_receiver,
                                            const char *_methodName);

struct dcNode_t *dcNodeEvaluator_forceCallMethod(dcNodeEvaluator *_evaluator,
                                                 struct dcNode_t *_receiver,
                                                 const char *_methodName);

struct dcNode_t *dcNodeEvaluator_callMethodWithArgument
    (dcNodeEvaluator *_evaluator,
     struct dcNode_t *_receiver,
     const char *_methodName,
     struct dcNode_t *_argument);

struct dcNode_t *dcNodeEvaluator_callMethodWithArguments
    (dcNodeEvaluator *_evaluator,
     struct dcNode_t *_receiver,
     const char *_methodName,
     struct dcList_t *_arguments,
     bool _force);

struct dcNode_t *dcNodeEvaluator_callEvaluatedMethod
    (dcNodeEvaluator *_evaluator,
     const char *_receiverName,
     const char *_functionName,
     struct dcNode_t *_argument,
     ...);

struct dcNode_t *dcNodeEvaluator_evaluateProcedure(dcNodeEvaluator *_evaluator,
                                                   struct dcNode_t *_receiver,
                                                   struct dcNode_t *_procedure,
                                                   dcScopeDataFlags _flags,
                                                   struct dcList_t *_arguments);

struct dcNode_t *dcNodeEvaluator_evaluateProcedureGuts
    (dcNodeEvaluator *_evaluator,
     struct dcNode_t *_body,
     struct dcMethodHeader_t *_methodHeader,
     struct dcList_t *_arguments);

struct dcNode_t *dcNodeEvaluator_evaluateTaffyCMethodPointer
    (dcNodeEvaluator *_evaluator,
     struct dcNode_t *_receiver,
     dcTaffyCMethodPointer _method,
     struct dcList_t *_arguments);

struct dcNode_t *dcNodeEvaluator_setPrinting(dcNodeEvaluator *_evaluator,
                                             struct dcNode_t *_toPrint,
                                             bool _yesNo);

// comparing //
dcResult dcNodeEvaluator_compareObjects(dcNodeEvaluator *_evaluator,
                                        struct dcNode_t *_left,
                                        struct dcNode_t *_right,
                                        dcTaffyOperator *_operator);

uint32_t dcNodeEvaluator_pushMark(dcNodeEvaluator *_evaluator,
                                struct dcNode_t *_nodeToMark);
void dcNodeEvaluator_popMarks(dcNodeEvaluator *_evaluator,
                              uint32_t _count);
uint32_t dcNodeEvaluator_pushListToMark(dcNodeEvaluator *_evaluator,
                                           struct dcList_t *_arguments);
uint32_t dcNodeEvaluator_pushArrayToMark(dcNodeEvaluator *_evaluator,
                                       struct dcArray_t *_array);
void dcNodeEvaluator_popMark(dcNodeEvaluator *_evaluator);
void dcNodeEvaluator_popMarkShell(dcNodeEvaluator *_evaluator);

// sanity //
bool dcNodeEvaluator_isSuspended(dcNodeEvaluator *_evaluator);

COMPARE_FUNCTION(dcNodeEvaluator_compareNode);
FREE_FUNCTION(dcNodeEvaluator_freeNode);
HASH_FUNCTION(dcNodeEvaluator_hashNode);
MARK_FUNCTION(dcNodeEvaluator_markNode);

void *dcNodeEvaluator_synchronizeFunctionCall
    (dcNodeEvaluator *_evaluator,
     dcGenericFunction _function,
     void *_argument);

const char *dcNodeEvaluator_getCurrentFileName(dcNodeEvaluator *_evaluator);
struct dcFilePackageData_t *dcNodeEvaluator_getCurrentFilePackageData
    (dcNodeEvaluator *_evaluator);

void dcNodeEvaluator_incrementAbortDelay(dcNodeEvaluator *_evaluator);
bool dcNodeEvaluator_decrementAbortDelay(dcNodeEvaluator *_evaluator);

// verify that we can resolve imports and the super!
dcResult dcNodeEvaluator_checkDeferredImports(dcNodeEvaluator *_evaluator,
                                              struct dcNode_t *_node);

/**
 * @brief Call this before your loop starts
 */
void dcNodeEvaluator_startLoop(dcNodeEvaluator *_evaluator);

/**
 * @brief Call this after your loop ends
 */
void dcNodeEvaluator_stopLoop(dcNodeEvaluator *_evaluator);

#endif
