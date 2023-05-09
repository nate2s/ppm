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

#ifndef __DC_GARBAGE_COLLECTOR_H__
#define __DC_GARBAGE_COLLECTOR_H__

#include "dcDefines.h"

//
// API Functions
//

/*
 * @brief Indicate that the current thread is beginning a blocking operation.
 */
void dcGarbageCollector_nodeEvaluatorDown(void);

/*
 * @brief Indicate that the current thread has finished the blocking operation.
 */
void dcGarbageCollector_nodeEvaluatorBlockUp(void);

/*
 * @brief Add a marking root.
 */
void dcGarbageCollector_addRoot(dcTaffy_rootMarkPointer _pointer);

/*
 * @brief Remove a marking root.
 */
void dcGarbageCollector_removeRoot(dcTaffy_rootMarkPointer _pointer);

//
// </API> Functions
//

//
// for testing
//
typedef enum
{
    TAFFY_GARBAGE_COLLECTOR_NO_TEST_STATE = 0,
    TAFFY_GARBAGE_COLLECTOR_MARK_TO_NO    = 1,
    TAFFY_GARBAGE_COLLECTOR_CONDEMN       = 2,
    TAFFY_GARBAGE_COLLECTOR_FREE          = 3
} dcGarbageCollectorDebugState;

#define GARBAGE_COLLECTOR_OBJECT_TIP 6000000

typedef void (*dcGarbageCollectorWatcher)(struct dcNode_t *_node,
                                          dcGarbageCollectorDebugState _state);

void dcGarbageCollector_setWatcher(dcGarbageCollectorWatcher _watcher);
void dcGarbageCollector_clearWatcher(void);
void dcGarbageCollector_go(void);
//
// /for testing
//

struct dcGarbageCollector_t
{
    dcGarbageCollectorWatcher watcher;

    struct dcList_t *registeredNodes;

    // the pre-registered nodes, they are moved to registeredNodes
    // during garbage collection
    struct dcList_t *preRegisteredNodes;
    struct dcMutex_t *registerMutex;

    // initial value set via global configuration
    uint64_t memoryTip;

    // the roots, a collection of dcNode(dcRootMarkFunction)
    struct dcList_t *roots;

    // are we running?
    bool running;

    // should we stop?
    bool stop;

    // the main mutex, locked during running
    struct dcMutex_t *mutex;

    // how many evaluators are still running?
    size_t runningEvaluatorCount;

    // signaled when a node evaluator blocks for garbage collection
    struct dcCondition_t *evaluatorCondition;

    // let watchers know that we've finished garbage collection
    struct dcCondition_t *finishCondition;

    // let the garbage collector know something was registered
    struct dcCondition_t *registerCondition;

    // the execution thread
    struct dcThreadId_t *executeThread;

    // trackRegistration is false by default
    // set to true to track node registration
    bool trackRegistration;

    //
    // <debug> variables
    //

    // run the garbage collector after each evaluation?
    // this slows things the hella down but can catch bugs
    bool alwaysGarbageCollect;

    // how many times have we run the garbage collector
    uint32_t runCount;

    // how many nodes were freed
    uint32_t nodeFreeCount;

    // </debug>

#ifdef TAFFY_DEBUG
    // force garbage collection?
    bool forceGarbageCollect;

    struct dcList_t *collectionCounts;

    // enabled is true by default
    // set to false to avoid actual freeing of garbage collected objects
    bool enabled;

    bool assertIfRun;
#endif
};

typedef struct dcGarbageCollector_t dcGarbageCollector;

// creates the GarbageCollector singleton
dcGarbageCollector *dcGarbageCollector_create(void);
dcGarbageCollector *dcGarbageCollector_createWithMemoryTip
    (uint64_t _startingMemoryTip);

// deletes the singleton
void dcGarbageCollector_free(void);
void dcGarbageCollector_freeWithStatistics(bool _displayStatistics);

// register a node to be garbage collected //
void dcGarbageCollector_registerNode(struct dcNode_t *_node);
void dcGarbageCollector_registerNodeWithLevel
    (struct dcNode_t *_node, dcGarbageCollectorLevel _level);

//
// the main execution thread
//
void dcGarbageCollector_execute(void);

void dcGarbageCollector_stop(void);

void dcGarbageCollector_lock(void);
void dcGarbageCollector_unlock(void);

// Called by a node evaluator. This function blocks until the GC has finished
// collecting.
void dcGarbageCollector_blockNodeEvaluator(void);

void dcGarbageCollector_logState(void);
void dcGarbageCollector_setAlwaysGarbageCollect(bool _yesno);
void dcGarbageCollector_setTrackRegistration(bool _yesno);
bool dcGarbageCollector_trackRegistration(void);
void dcGarbageCollector_nodeEvaluatorUp(void);

void dcGarbageCollector_blockOtherNodeEvaluators(void);
void dcGarbageCollector_unblockOtherNodeEvaluators(void);

void dcGarbageCollector_printStatistics(void);

void dcGarbageCollector_forceStop(void);

#ifdef TAFFY_DEBUG
void dcGarbageCollector_setEnabled(bool _yesno);
void dcGarbageCollector_forceExecution(void);
#endif

void *dcGarbageCollector_synchronizeFunctionCall(dcGenericFunction _function,
                                                 void *_token);

#endif
