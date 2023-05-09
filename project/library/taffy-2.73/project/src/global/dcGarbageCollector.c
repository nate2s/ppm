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
    #include <inttypes.h>
    #include <dlfcn.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

#include "dcGarbageCollector.h"
#include "dcArray.h"
#include "dcClass.h"
#include "dcCondition.h"
#include "dcError.h"
#include "dcGraphData.h"
#include "dcHash.h"
#include "dcIOClass.h"
#include "dcList.h"
#include "dcLog.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcRootMarkFunction.h"
#include "dcScope.h"
#include "dcString.h"
#include "dcSystem.h"
#include "dcThread.h"
#include "dcThreadId.h"

#ifdef TAFFY_DEBUG
  #include "dcNumberClass.h"
  #include "dcNumber.h"
#endif

static dcGarbageCollector *sGarbageCollector = NULL;
static void go(void);

dcGarbageCollector *dcGarbageCollector_create(void)
{
    return dcGarbageCollector_createWithMemoryTip(GARBAGE_COLLECTOR_OBJECT_TIP);
}

dcGarbageCollector *dcGarbageCollector_createWithMemoryTip
    (uint64_t _startingMemoryTip)
{
    // singleton
    if (sGarbageCollector == NULL)
    {
        sGarbageCollector =
            ((dcGarbageCollector *)dcMemory_allocateAndInitialize
             (sizeof(dcGarbageCollector)));

        // watcher is disabled by default //

        sGarbageCollector->registeredNodes = dcList_create();
        sGarbageCollector->preRegisteredNodes = dcList_create();

        sGarbageCollector->roots = dcList_create();
        sGarbageCollector->memoryTip = _startingMemoryTip;
        sGarbageCollector->trackRegistration = false;
        sGarbageCollector->running = false;
        sGarbageCollector->stop = false;
        sGarbageCollector->finishCondition = dcCondition_create();
        sGarbageCollector->evaluatorCondition = dcCondition_create();
        sGarbageCollector->registerCondition = dcCondition_create();
        sGarbageCollector->mutex = dcMutex_create(false);
        sGarbageCollector->registerMutex = dcMutex_create(false);

        TAFFY_DEBUG(sGarbageCollector->enabled = true;);
        TAFFY_DEBUG(sGarbageCollector->collectionCounts = dcList_create(););
    }

    return sGarbageCollector;
}

void dcGarbageCollector_free(void)
{
    dcGarbageCollector_freeWithStatistics(false);
}

void dcGarbageCollector_freeWithStatistics(bool _displayStatistics)
{
    if (sGarbageCollector != NULL)
    {
        dcList_free(&sGarbageCollector->roots, DC_DEEP);

        while (sGarbageCollector->preRegisteredNodes->size > 0
               || sGarbageCollector->registeredNodes->size > 0)
        {
            go();
        }

        if (_displayStatistics)
        {
            dcGarbageCollector_printStatistics();
        }

        dcError_assert(sGarbageCollector->registeredNodes->size == 0);
        dcError_assert(sGarbageCollector->preRegisteredNodes->size == 0);
        dcList_free(&sGarbageCollector->registeredNodes, DC_SHALLOW);
        dcList_free(&sGarbageCollector->preRegisteredNodes, DC_SHALLOW);
        dcThreadId_free(&sGarbageCollector->executeThread);
        dcCondition_free(&sGarbageCollector->evaluatorCondition);
        dcCondition_free(&sGarbageCollector->finishCondition);
        dcCondition_free(&sGarbageCollector->registerCondition);
        dcMutex_free(&sGarbageCollector->mutex);
        dcMutex_free(&sGarbageCollector->registerMutex);

        TAFFY_DEBUG(dcList_free(&sGarbageCollector->collectionCounts, DC_DEEP));

        dcMemory_free(sGarbageCollector);
    }
}

static void *executeThread(void *unused)
{
    dcThread_ignoreAllSignals();

    // create a debug node evaluator for printing
    dcNodeEvaluator *evaluator = dcNodeEvaluator_create();

    // tell the system that we're up
    dcSystem_garbageCollectorIsUp();

    while (true)
    {
        dcMutex_lock(sGarbageCollector->registerMutex);

        while ((dcMemory_getAllocatedMemorySize()
                < sGarbageCollector->memoryTip)
               && ! sGarbageCollector->stop)
        {
            dcCondition_wait(sGarbageCollector->registerCondition,
                             sGarbageCollector->registerMutex);

            if (sGarbageCollector->alwaysGarbageCollect)
            {
                break;
            }

#ifdef TAFFY_DEBUG
            if (sGarbageCollector->forceGarbageCollect)
            {
                sGarbageCollector->forceGarbageCollect = false;
                break;
            }
#endif
        }

        bool myStop = sGarbageCollector->stop;
        dcMutex_unlock(sGarbageCollector->registerMutex);
        go();

        if (myStop)
        {
            break;
        }
    }

    dcGarbageCollector_lock();
    dcGarbageCollector_unlock();
    dcNodeEvaluator_free(&evaluator);
    return NULL;
}

// the main execution loop
void dcGarbageCollector_execute(void)
{
    sGarbageCollector->executeThread = dcThread_create(&executeThread, NULL);
}

void dcGarbageCollector_stop(void)
{
    dcMutex_lock(sGarbageCollector->registerMutex);
    sGarbageCollector->stop = true;
    dcCondition_signal(sGarbageCollector->registerCondition);
    dcMutex_unlock(sGarbageCollector->registerMutex);
    dcThread_join(sGarbageCollector->executeThread);

#ifndef __APPLE__
    dcThread_detach(sGarbageCollector->executeThread);
#endif

    dcThreadId_free(&sGarbageCollector->executeThread);
}

void dcGarbageCollector_setWatcher(dcGarbageCollectorWatcher _watcher)
{
    sGarbageCollector->watcher = _watcher;
}

void dcGarbageCollector_clearWatcher(void)
{
    sGarbageCollector->watcher = NULL;
}

static void notifyWatcher(dcNode *_node,
                          dcGarbageCollectorDebugState _state)
{
    if (sGarbageCollector->watcher != NULL)
    {
        sGarbageCollector->watcher(_node, _state);
    }
}

static void sweep(void)
{
    dcListElement *that = sGarbageCollector->registeredNodes->head;

    // help for debugging
    size_t i = 0;
    dcList *condemned = dcList_create();

    while (that != NULL)
    {
        dcListElement *saveIterator = that->next;
        dcNode *node = that->object;

        if (dcNode_isMarked(node))
        {
            notifyWatcher(node, TAFFY_GARBAGE_COLLECTOR_MARK_TO_NO);
            dcNode_markYesNo(node, false);
        }
        else
        {
            if (IS_CLASS(node))
            {
                dcClass_deallocateNode(node);
            }

            notifyWatcher(node, TAFFY_GARBAGE_COLLECTOR_CONDEMN);
            dcList_push(condemned, node);
            dcList_removeElement(sGarbageCollector->registeredNodes,
                                 &that,
                                 DC_SHALLOW);
        }

        i++;
        that = saveIterator;
    }
    // /sweep

    while (condemned->size > 0)
    {
        dcNode *node = dcList_pop(condemned, DC_SHALLOW);

        notifyWatcher(node, TAFFY_GARBAGE_COLLECTOR_FREE);
        dcDepth freeDepth = DC_DEEP;

        if (dcNode_isGarbageCollectionTrapped(node))
        {
            fprintf(stderr,
                    "[%zu] garbage collection trap hit",
                    i);
            dcError_assert(false);
        }

        TAFFY_DEBUG(if (! sGarbageCollector->enabled)
                    {
                        freeDepth = DC_SHALLOW;
                    });

        // unregister the node //
        dcNode_setRegistered(node, false);
        sGarbageCollector->nodeFreeCount++;
        dcNode_free(&node, freeDepth);
    }

    dcList_free(&condemned, DC_DEEP);
}

void dcGarbageCollector_lock(void)
{
    dcMutex_lock(sGarbageCollector->mutex);
}

void dcGarbageCollector_unlock(void)
{
    dcMutex_unlock(sGarbageCollector->mutex);
}

static void nodeEvaluatorUp(void)
{
    sGarbageCollector->runningEvaluatorCount++;
}

void dcGarbageCollector_nodeEvaluatorUp(void)
{
    dcGarbageCollector_lock();
    nodeEvaluatorUp();
    dcGarbageCollector_unlock();
}

void dcGarbageCollector_nodeEvaluatorBlockUp(void)
{
    dcGarbageCollector_nodeEvaluatorUp();
    dcGarbageCollector_blockNodeEvaluator();
}

static void nodeEvaluatorDown(void)
{
    dcError_assert(sGarbageCollector->runningEvaluatorCount > 0);
    sGarbageCollector->runningEvaluatorCount--;
    dcCondition_signal(sGarbageCollector->evaluatorCondition);
}

void dcGarbageCollector_nodeEvaluatorDown(void)
{
    dcGarbageCollector_lock();
    nodeEvaluatorDown();
    dcGarbageCollector_unlock();
}

static void blockUntilFinished(void)
{
    nodeEvaluatorDown();

    // block until the garbage collector is finished
    while (sGarbageCollector->running)
    {
        dcCondition_wait(sGarbageCollector->finishCondition,
                         sGarbageCollector->mutex);
    }

    nodeEvaluatorUp();
}

void dcGarbageCollector_blockNodeEvaluator(void)
{
    dcGarbageCollector_lock();

    if (sGarbageCollector->running)
    {
        blockUntilFinished();
    }

    dcGarbageCollector_unlock();
}

void dcGarbageCollector_blockOtherNodeEvaluators(void)
{
    dcGarbageCollector_lock();

    if (sGarbageCollector->running)
    {
        blockUntilFinished();
    }

    sGarbageCollector->running = true;

    // allow tests to not care about dcSystem
    assert(! dcSystem_isLive()
           || (sGarbageCollector->runningEvaluatorCount
               <= dcSystem_getNodeEvaluatorCount()));

    while (sGarbageCollector->runningEvaluatorCount > 1)
    {
        // wait for the evaluators to finish
        dcCondition_wait(sGarbageCollector->evaluatorCondition,
                         sGarbageCollector->mutex);
    }

    dcGarbageCollector_unlock();
}

void dcGarbageCollector_unblockOtherNodeEvaluators(void)
{
    dcGarbageCollector_lock();
    sGarbageCollector->running = false;
    dcCondition_broadcast(sGarbageCollector->finishCondition);
    dcGarbageCollector_unlock();
}

#define GARBAGE_COLLECTOR_BUMP 0.7

void dcGarbageCollector_go(void)
{
    go();
}

static void go(void)
{
    dcGarbageCollector_lock();

    if (sGarbageCollector->assertIfRun)
    {
        dcGarbageCollector_unlock();
        abort();
    }

    if (sGarbageCollector->running)
    {
        dcGarbageCollector_unlock();
        return;
    }

    sGarbageCollector->running = true;
    sGarbageCollector->runCount++;

    // allow tests to disregard dcSystem
    assert(! dcSystem_isLive()
           || (sGarbageCollector->runningEvaluatorCount
               <= dcSystem_getNodeEvaluatorCount()));

    while (sGarbageCollector->runningEvaluatorCount > 0)
    {
        // wait for the evaluators to finish
        dcCondition_wait(sGarbageCollector->evaluatorCondition,
                         sGarbageCollector->mutex);
    }

    if (dcLog_isEnabled(GARBAGE_COLLECTOR_TICK_LOG))
    {
        printf("!");
        fflush(stdout);
    }

    dcMutex_lock(sGarbageCollector->registerMutex);

    // transfer the nodes from pre registered to registered
    while (sGarbageCollector->preRegisteredNodes->size > 0)
    {
        dcList_push(sGarbageCollector->registeredNodes,
                    dcList_getTail(sGarbageCollector->preRegisteredNodes));
        dcList_pop(sGarbageCollector->preRegisteredNodes, DC_SHALLOW);
    }

    dcMutex_unlock(sGarbageCollector->registerMutex);

    dcLog_log(GARBAGE_COLLECTOR_LOG,
              "start, %u nodes, allocated: %" PRId64 " tip: %" PRId64 "\n",
              sGarbageCollector->registeredNodes->size,
              dcMemory_getAllocatedMemorySize(),
              sGarbageCollector->memoryTip);

    // mark! the slow way
    dcList_mark(sGarbageCollector->roots);

    // TODO: 64-bit
    // don't let the collectionCounts grow arbitrarily
    TAFFY_DEBUG(if (sGarbageCollector->collectionCounts->size < 1000)
                {
                    dcList_push
                        (sGarbageCollector->collectionCounts,
                         dcNumber_createNodeFromInt32u
                         (sGarbageCollector->registeredNodes->size));
                });

    // sweep!
    sweep();

    // alert all the listeners in dcGarbageCollector_blockNodeEvaluator()
    sGarbageCollector->running = false;
    dcCondition_broadcast(sGarbageCollector->finishCondition);

    uint64_t allocatedMemorySize = dcMemory_getAllocatedMemorySize();
    sGarbageCollector->memoryTip = (allocatedMemorySize
                                    + (((double)allocatedMemorySize
                                        * GARBAGE_COLLECTOR_BUMP)));
    dcLog_log(GARBAGE_COLLECTOR_LOG,
              "end, %u nodes, allocated: %" PRId64 " tip: %" PRId64 "\n",
              sGarbageCollector->registeredNodes->size,
              allocatedMemorySize,
              sGarbageCollector->memoryTip);

    if (dcLog_isEnabled(GARBAGE_COLLECTOR_TICK_LOG))
    {
        printf("@");
        fflush(stdout);
    }

    dcGarbageCollector_unlock();
}

void dcGarbageCollector_logState(void)
{
    dcGarbageCollector_lock();
    dcLog_log(GARBAGE_COLLECTOR_LOG,
              "%u nodes %" PRId64 " allocated\n",
              sGarbageCollector->registeredNodes->size,
              dcMemory_getAllocatedMemorySize());
    dcGarbageCollector_unlock();
}

void dcGarbageCollector_registerNodeWithLevel
    (dcNode *_node, dcGarbageCollectorLevel _level)
{
#ifdef ENABLE_DEBUG
    assert(! dcNode_isContainer(_node)
           && ! dcNode_isTemplate(_node));

    if (! sGarbageCollector->enabled
        || dcGarbageCollector_trackRegistration())
    {
        dcMemory_createBacktrace(_node->registerBacktraceList);
    }

    assert(! dcNode_isRegisterTrapped(_node));
#endif
    //dcError_assert(IS_CLASS(_node)
    //               || _node->type == NODE_TAFFY_C_METHOD_POINTER
    //               || (_node->type == NODE_GRAPH_DATA
    //                   && (dcGraphData_getType(_node) == NODE_GRAPH_DATA_TREE
    //                       || dcGraphData_getType(_node) == NODE_NIL))
    //               || _node->type == NODE_STRING);

    dcMutex_lock(sGarbageCollector->registerMutex);

    if (! dcNode_isRegistered(_node))
    {
        dcList_push(sGarbageCollector->preRegisteredNodes, _node);
        dcNode_setRegistered(_node, true);
        dcCondition_signal(sGarbageCollector->registerCondition);
    }

    dcMutex_unlock(sGarbageCollector->registerMutex);
}

//
// The register function, this is either called directly (rare),
// or called via dcNode_register() (much less rare)
//
void dcGarbageCollector_registerNode(dcNode *_node)
{
    dcGarbageCollector_registerNodeWithLevel
        (_node, TAFFY_GARBAGE_COLLECTOR_LEVEL_1);
}

void dcGarbageCollector_addRoot(dcTaffy_rootMarkPointer _pointer)
{
    dcGarbageCollector_lock();
    dcList_push(sGarbageCollector->roots,
                dcRootMarkFunction_createNode(_pointer));
    dcGarbageCollector_unlock();
}

void dcGarbageCollector_removeRoot(dcTaffy_rootMarkPointer _pointer)
{
    dcNode *node = dcRootMarkFunction_createNode(_pointer);
    dcError_assert(dcList_remove(sGarbageCollector->roots, node, DC_DEEP)
                   == TAFFY_SUCCESS);
    dcNode_free(&node, DC_DEEP);
}

void *dcGarbageCollector_synchronizeFunctionCall(dcGenericFunction _function,
                                                 void *_token)
{
    dcGarbageCollector_lock();

    // block until the garbage collector is finished
    while (sGarbageCollector->running)
    {
        dcCondition_wait(sGarbageCollector->finishCondition,
                         sGarbageCollector->mutex);
    }

    sGarbageCollector->running = true;
    dcGarbageCollector_unlock();

    void *result = _function(_token);

    dcGarbageCollector_lock();
    sGarbageCollector->running = false;
    dcCondition_broadcast(sGarbageCollector->finishCondition);
    dcGarbageCollector_unlock();

    return result;
}

void dcGarbageCollector_setAlwaysGarbageCollect(bool _yesno)
{
    dcMutex_lock(sGarbageCollector->registerMutex);
    sGarbageCollector->alwaysGarbageCollect = _yesno;
    dcMutex_unlock(sGarbageCollector->registerMutex);
}

void dcGarbageCollector_setTrackRegistration(bool _yesno)
{
    sGarbageCollector->trackRegistration = _yesno;
}

bool dcGarbageCollector_trackRegistration(void)
{
    return (sGarbageCollector != NULL
            && sGarbageCollector->trackRegistration);
}

void dcGarbageCollector_forceStop(void)
{
    sGarbageCollector->running = false;
}

#ifdef TAFFY_DEBUG

void dcGarbageCollector_forceExecution(void)
{
    dcMutex_lock(sGarbageCollector->registerMutex);
    sGarbageCollector->forceGarbageCollect = true;
    dcCondition_signal(sGarbageCollector->registerCondition);
    dcMutex_unlock(sGarbageCollector->registerMutex);
}

void dcGarbageCollector_assertIfRun(bool _yesno)
{
    sGarbageCollector->assertIfRun = _yesno;
}

void dcGarbageCollector_setEnabled(bool _yesno)
{
    sGarbageCollector->enabled = _yesno;
}

void dcGarbageCollector_printStatistics(void)
{
    dcIOClass_printFormat("garbage collector ran %u times | "
                          "collected %u nodes | average: %f\n",
                          sGarbageCollector->runCount,
                          sGarbageCollector->nodeFreeCount,
                          (double)sGarbageCollector->nodeFreeCount
                          / (double)sGarbageCollector->runCount);

    // compute the standard deviation
    dcNumber *sum = dcNumber_createFromInt32u(0);
    dcNumber *average = dcNumber_createFromDouble
        ((double)sGarbageCollector->nodeFreeCount
         / (double)sGarbageCollector->runCount);
    dcNumber *one = dcNumber_createFromInt32u(1);
    dcNumber *two = dcNumber_createFromInt32u(2);

    FOR_EACH_IN_LIST(sGarbageCollector->collectionCounts, that)
    {
        dcNumber *term = dcNumber_copy(CAST_NUMBER(that->object), DC_DEEP);
        dcNumber_subtract(term, term, average);
        dcNumber_raise(term, term, two);
        dcNumber_add(sum, sum, term);
        dcNumber_free(&term, DC_DEEP);
    }

    dcNumber *oneOverAverage = dcNumber_copy(average, DC_DEEP);
    dcNumber_divide(oneOverAverage,
                    one,
                    oneOverAverage);
    dcNumber_multiply(sum, sum, oneOverAverage);
    dcNumber_squareRoot(sum, sum);

    char *deviation = dcNumber_display(sum);
    dcIOClass_printFormat("standard deviation: %s\n", deviation);
    dcMemory_free(deviation);

    dcNumber_free(&sum, DC_DEEP);
    dcNumber_free(&average, DC_DEEP);
    dcNumber_free(&one, DC_DEEP);
    dcNumber_free(&two, DC_DEEP);
    dcNumber_free(&oneOverAverage, DC_DEEP);
}

#endif
