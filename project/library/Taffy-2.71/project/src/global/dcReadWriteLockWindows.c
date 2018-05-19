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

#include "dcMemory.h"
#include "dcReadWriteLock.h"

/**
 * @brief Create the lock.
 */
dcReadWriteLock *dcReadWriteLock_create(void)
{
    dcReadWriteLock *result =
        (dcReadWriteLock *)dcMemory_allocate(sizeof(dcReadWriteLock));
    InitializeSRWLock(&result->lock);
    return result;
}

void dcReadWriteLock_free(dcReadWriteLock **_lock)
{
    if (*_lock != NULL)
    {
        // need to free the lock?
        dcMemory_free(*_lock);
    }
}

bool dcReadWriteLock_lockForRead(dcReadWriteLock *_lock)
{
    AcquireSRWLockShared(&_lock->lock);
    _lock->lockedForShared = true;
    return true;
}

bool dcReadWriteLock_lockForWrite(dcReadWriteLock *_lock)
{
    AcquireSRWLockExclusive(&_lock->lock);
    _lock->lockedForShared = false;
    return true;
}

void dcReadWriteLock_unlock(dcReadWriteLock *_lock)
{
    if (_lock->lockedForShared)
    {
        ReleaseSRWLockShared(&_lock->lock);
    }
    else
    {
        ReleaseSRWLockExclusive(&_lock->lock);
    }
}
