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

#include "dcError.h"
#include "dcMemory.h"
#include "dcMutex.h"

static pthread_mutexattr_t sRecursiveAttributes;

TAFFY_DEBUG(bool sInitialized = false);

void dcMutex_initialize(void)
{
    if (pthread_mutexattr_init(&sRecursiveAttributes)
        == 0)
    {
        pthread_mutexattr_settype(&sRecursiveAttributes,
                                  PTHREAD_MUTEX_RECURSIVE);
    }

    TAFFY_DEBUG(sInitialized = true);
}

dcMutex *dcMutex_create(bool _recursive)
{
    TAFFY_DEBUG(if (_recursive)
                {
                    dcError_assert(sInitialized);
                });

    dcMutex *result = (dcMutex *)dcMemory_allocate(sizeof(dcMutex));
    pthread_mutex_init(&result->mutex,
                       _recursive ? &sRecursiveAttributes : NULL);
    return result;
}

void dcMutex_free(dcMutex **_mutex)
{
    if (*_mutex != NULL)
    {
        pthread_mutex_destroy(&(*_mutex)->mutex);
        dcMemory_free(*_mutex);
    }
}

bool dcMutex_lock(dcMutex *_mutex)
{
    return (pthread_mutex_lock(&_mutex->mutex) == 0);
}

bool dcMutex_unlock(dcMutex *_mutex)
{
    return (pthread_mutex_unlock(&_mutex->mutex) == 0);
}
