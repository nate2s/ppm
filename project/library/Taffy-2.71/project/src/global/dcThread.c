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
#include <signal.h>
#include <stdarg.h>

#include "dcError.h"
#include "dcLexer.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcThread.h"
#include "dcThreadId.h"
#include "dcUnsignedInt64.h"
#include "dcVoid.h"

dcThreadId *dcThread_create(ThreadFunction _function, void *_argument)
{
    dcThreadId *threadId = dcThreadId_create();
    pthread_create(&threadId->threadId,
                   NULL,
                   _function,
                   _argument);
    return threadId;
}

void dcThread_join(dcThreadId *_threadId)
{
    pthread_join(_threadId->threadId, NULL);
}

void dcThread_detach(dcThreadId *_threadId)
{
    pthread_detach(_threadId->threadId);
}

void dcThread_kill(dcThreadId *_threadId, int signal)
{
    pthread_kill(_threadId->threadId, signal);
}

dcNode *dcThread_getSelf(void)
{
    pthread_t selfId = pthread_self();
    assert(sizeof(size_t) <= sizeof(uint64_t));
    return dcUnsignedInt64_createNode((uint64_t)(size_t)(void *)selfId);
}

dcTaffyThreadId dcThread_getSelfId(void)
{
#ifdef TAFFY_WINDOWS
    return GetCurrentThreadId(void);
#else
    return pthread_self();
#endif
}

void dcThread_ignoreAllSignals(void)
{
    //
    // ignore all signals
    //
    sigset_t mask;
    sigfillset(&mask);

    if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0)
    {
        perror("sigmask");
    }
}
