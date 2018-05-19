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

void dcMutex_initialize(void)
{
}

dcMutex *dcMutex_create(bool _recursive)
{
    dcMutex *result = (dcMutex *)dcMemory_allocate(sizeof(dcMutex));

    if (!InitializeCriticalSectionAndSpinCount(&result->mutex, 0x00000400))
    {
        dcMutex_free(&result);
        return NULL;
    }

    return result;
}

void dcMutex_free(dcMutex **_mutex)
{
    if (*_mutex != NULL)
    {
        DeleteCriticalSection(&(*_mutex)->mutex);
        dcMemory_free(*_mutex);
    }
}

bool dcMutex_lock(dcMutex *_mutex)
{
    EnterCriticalSection(&_mutex->mutex);
    return true;
}

bool dcMutex_unlock(dcMutex *_mutex)
{
    LeaveCriticalSection(&_mutex->mutex);
    return true;
}
