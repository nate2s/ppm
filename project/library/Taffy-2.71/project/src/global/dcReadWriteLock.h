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

#ifndef __DC_READ_WRITE_LOCK_H__
#define __DC_READ_WRITE_LOCK_H__

#include "dcDefines.h"

#ifdef TAFFY_WINDOWS
    #include <Windows.h>
#else
    #include <pthread.h>
#endif

struct dcReadWriteLock_t
{
#ifdef TAFFY_WINDOWS
    bool lockedForShared;
    SRWLOCK lock;
#else
    pthread_rwlock_t lock;
#endif
};

typedef struct dcReadWriteLock_t dcReadWriteLock;

/**
 * @brief Create the lock.
 */
dcReadWriteLock *dcReadWriteLock_create(void);

/**
 * @brief Free the lock.
 */
void dcReadWriteLock_free(dcReadWriteLock **_lock);

/**
 * @brief Lock for read.
 */
bool dcReadWriteLock_lockForRead(dcReadWriteLock *_lock);

/**
 * @brief Lock for write.
 */
bool dcReadWriteLock_lockForWrite(dcReadWriteLock *_lock);

/**
 * @brief Unlock.
 */
void dcReadWriteLock_unlock(dcReadWriteLock *_lock);

#endif
