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

#ifndef __DC_MUTEX_H__
#define __DC_MUTEX_H__

#include "dcDefines.h"

#ifdef TAFFY_WINDOWS
    #include <Windows.h>
#else
    #include <pthread.h>
#endif

struct dcMutex_t
{
#ifdef TAFFY_WINDOWS
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
};

typedef struct dcMutex_t dcMutex;

/**
 * @brief Initialize the mutex system. Call this first.
 */
void dcMutex_initialize(void);

/**
 * @brief Create the mutex.
 */
dcMutex *dcMutex_create(bool _recursive);

/**
 * @brief Free the mutex.
 */
void dcMutex_free(dcMutex **_mutex);

/**
 * @brief Lock the mutex.
 */
bool dcMutex_lock(dcMutex *_mutex);

/**
 * @brief Unlock the mutex.
 */
bool dcMutex_unlock(dcMutex *_mutex);

#endif
