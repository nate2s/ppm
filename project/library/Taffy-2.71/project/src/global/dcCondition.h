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

#ifndef __DC_CONDITION_H__
#define __DC_CONDITION_H__

#include "dcDefines.h"

#ifdef TAFFY_WINDOWS
#include <windows.h>

struct dcCondition_t
{
    CONDITION_VARIABLE condition;
};

#else // TAFFY_WINDOWS
#include <pthread.h>

struct dcCondition_t
{
    pthread_cond_t condition;
};
#endif // TAFFY_WINDOWS

typedef struct dcCondition_t dcCondition;

/**
 * @brief Create the condition.
 */
dcCondition *dcCondition_create(void);

/**
 * @brief Free the condition.
 */
void dcCondition_free(dcCondition **_condition);

/**
 * @brief Signal the condition.
 */
void dcCondition_signal(dcCondition *_condition);

/**
 * @brief Broadcast the condition.
 */
void dcCondition_broadcast(dcCondition *_condition);

/**
 * @brief Wait for condition signal.
 */
void dcCondition_wait(dcCondition *_condition, struct dcMutex_t *_mutex);

#endif
