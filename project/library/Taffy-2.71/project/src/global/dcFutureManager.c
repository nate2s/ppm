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
#include <string.h>

#include "dcArray.h"
#include "dcFutureClass.h"
#include "dcGarbageCollector.h"
#include "dcList.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcSystem.h"
#include "dcThread.h"
#include "dcThreadClass.h"

typedef struct
{
    size_t maxThreads;
    size_t threadCount;
    dcMutex *mutex;

    dcList *futures;
} FutureManager;

static FutureManager manager = {0};

static void futureMarker(void)
{
    dcList_mark(manager.futures);
}

void dcFutureManager_initialize(size_t _maxThreads)
{
    memset(&manager, 0, sizeof(FutureManager));
    manager.maxThreads = _maxThreads;
    manager.futures = dcList_create();
    manager.mutex = dcMutex_create(false);
    dcGarbageCollector_addRoot(futureMarker);
}

void dcFutureManager_free(void)
{
    dcMutex_free(&manager.mutex);
    dcList_free(&manager.futures, DC_SHALLOW);

    // TODO: wait on threads??
}

static void promiseCallback(dcNode *_result, dcNode *_future)
{
    dcMutex_lock(manager.mutex);
    assert(manager.threadCount > 0);
    manager.threadCount--;
    dcFutureClass_setValue(_future, _result);
    assert(dcList_remove(manager.futures, _future, DC_SHALLOW)
           == TAFFY_SUCCESS);
    dcMutex_unlock(manager.mutex);
}

void dcFutureManager_setMaxThreads(size_t _maxThreads)
{
    manager.maxThreads = _maxThreads;
}

dcNode *dcFutureManager_schedule(dcNode *_promise, dcNode *_arguments)
{
    dcMutex_lock(manager.mutex);
    dcNode *result = NULL;

    if (manager.threadCount < manager.maxThreads)
    {
        result = dcNode_register(dcFutureClass_createObject());
        manager.threadCount++;
        dcList_push(manager.futures, result);

        dcNode *thread = dcNode_register(dcThreadClass_createObject
                                         (dcNode_copy(_promise, DC_DEEP)));
        dcThreadClass_setCallback(thread, &promiseCallback, result);
        dcThreadClass_startHelper(thread, _arguments);
        dcMutex_unlock(manager.mutex);
    }
    else
    {
        dcMutex_unlock(manager.mutex);

        // TODO: test me
        result = (dcNodeEvaluator_evaluate
                  (dcSystem_getCurrentNodeEvaluator(), _promise));
    }

    return result;
}
