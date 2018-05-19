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

#ifndef __DC_SYSTEM_H__
#define __DC_SYSTEM_H__

#include <setjmp.h>

#include "dcDefines.h"

struct dcSystem_t
{
    struct dcCommandLineArguments_t *commandLineArguments;
    struct dcNodeEvaluator_t *nodeEvaluator;

    struct dcList_t *includeDirectories;
    struct dcList_t *pluginDirectories;
    struct dcArray_t *arguments;
    struct dcHash_t *environmentVariables;
    struct dcNode_t *globalScope;

    struct dcList_t *automaticFunctions;
    struct dcList_t *registeredLibraryNames;
    struct dcList_t *handles;
    struct dcList_t *threads;

    struct dcMutex_t *threadMutex;
    struct dcCondition_t *threadCondition;
    bool stopping;

    struct dcMutex_t *upMutex;
    struct dcCondition_t *upCondition;
    bool up;

    struct dcReadWriteLock_t *evaluatorsLock;
    struct dcHash_t *evaluators;

    // true during bootstrapping, false otherwise
    bool bootstrap;

    struct dcClassTemplate_t *abortExceptionClassTemplate;

    // this hash maps dcNode(dcUnsignedInt) => dcNode(dcFilePackageData)
    // where the LHS is a filename ID
    struct dcHash_t *filePackageDatas;
    struct dcMutex_t *filePackageDatasMutex;

    char *taffyHome;

    struct dcHash_t *asStringObjects;

    struct dcList_t *outOfMemoryCallStack;
    jmp_buf outOfMemoryBuffer;

    // debugging variable, how many nodes were marked?
    uint32_t markCount;
};

typedef struct dcSystem_t dcSystem;

dcSystem *dcSystem_create(void);
dcSystem *dcSystem_createWithArguments
    (struct dcCommandLineArguments_t *_arguments);

void dcSystem_free(void);
void dcSystem_mark(void);

void dcSystem_garbageCollectorIsUp(void);

// get the current node evaluator //
bool dcSystem_currentThreadHasNodeEvaluator(void);
struct dcNodeEvaluator_t *dcSystem_getCurrentNodeEvaluator(void);

// how many evaluators exist //
uint32_t dcSystem_getNodeEvaluatorCount(void);

void dcSystem_addToGlobalScope(struct dcNode_t *_node, const char *_name);

struct dcNodeEvaluator_t *dcSystem_getNodeEvaluator(void);
const struct dcList_t *dcSystem_getIncludeDirectories(void);
const struct dcList_t *dcSystem_getPluginDirectories(void);
const struct dcList_t *dcSystem_getArguments(void);
struct dcNode_t *dcSystem_getGlobalScope(void);
bool dcSystem_isInBootstrap(void);

void dcSystem_addAutomaticFunction(const char *_name);
bool dcSystem_isAutomaticFunction(const char *_name);

// include directories //
void dcSystem_addIncludeDirectory(const char *_includeDirectory);

// scope //
void dcSystem_createGlobalObjects();
void dcSystem_fillGlobalScope(void);

// printing //
char *dcSystem_displayGlobalScope(void);
void dcSystem_printGlobalScope(void);

// environment //
struct dcHash_t *dcSystem_getEnvironmentVariables(void);
void dcSystem_setEnvironmentVariable(const char *_name, const char *_value);

char *dcSystem_getEnvironmentVariable(const char *_name);
dcResult dcSystem_refreshEnvironmentVariables(void);

// meta scopes //
void dcSystem_registerTemplateScope(struct dcScope_t *_scope);
void dcSystem_addIncludeDirectory(const char *_includeDirectory);
bool dcSystem_libraryRegistered(const char *_libraryName);

// threads //
void dcSystem_incrementThreadCount(void);
void dcSystem_decrementThreadCount(void);

void dcSystem_testGarbageCollector(void);
void dcSystem_forceGarbageCollection(void);

//
// Taffy Operation Functions and helpers
//
const char *dcSystem_getOperatorName(dcTaffyOperator _type);
const char *dcSystem_getOperatorSymbol(dcTaffyOperator _type);

//
// Abort the execution of all node evaluators
//
void dcSystem_propagateFlagToEvaluators(bool _yesno, uint16_t _flag);
void dcSystem_abortExecution(int _signal);

//
// Check live status
//
bool dcSystem_isLive(void);

//
// Thread registering and unregistering
//
void dcSystem_registerThread(struct dcNode_t *_thread);
void dcSystem_unregisterThread(struct dcNode_t *_thread);

void dcSystem_addNodeEvaluator(struct dcNodeEvaluator_t *_evaluator);
void dcSystem_removeNodeEvaluator(struct dcNodeEvaluator_t *_evaluator);

void dcSystem_suspend(void);
void dcSystem_resume(void);

struct dcNode_t *dcSystem_convertBoolToNode(bool _value);

struct dcClassTemplate_t *dcSystem_getAbortExceptionClassTemplate(void);

struct dcNode_t *dcSystem_getFilePackageData(dcStringId _filenameId);
void dcSystem_removeFilePackageData(dcStringId _filenameId);
bool dcSystem_filePackageDataExists(const char *_filename);

// exit during operation, propagate exit to all evaluators
void dcSystem_exit(void);

// TODO: put me somewhere better
bool dcSystem_silencePluginWarnings(void);

// convenience function
struct dcList_t *dcSystem_getPackageContentsFromGraphDataNode
    (struct dcNode_t *_node);

// handles for plugins
void dcSystem_addHandle(void *_handle);

// we're out of memory
void dcSystem_outOfMemory(void);

void dcSystem_displayVersion(const char *_programName);

void dcSystem_setFinal(const char *_className, bool _yesno);

// debugging
bool dcSystem_isDebugEnabled(void);
void dcSystem_resetMarkCount(void);
void dcSystem_nodeWasMarked(void);
void dcSystem_logAndResetMarkCount(const char *_name);

#endif
