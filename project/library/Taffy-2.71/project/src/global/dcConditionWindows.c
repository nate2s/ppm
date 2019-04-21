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
#include <stdio.h>

#include "dcMemory.h"
#include "dcCondition.h"
#include "dcMutex.h"

dcCondition *dcCondition_create(void)
{
    dcCondition *result = (dcCondition *)dcMemory_allocate(sizeof(dcCondition));
    InitializeConditionVariable(&result->condition);
    return result;
}

void dcCondition_free(dcCondition **_condition)
{
    dcMemory_free(*_condition);
}

void dcCondition_signal(dcCondition *_condition)
{
    WakeConditionVariable(&_condition->condition);
}

void dcCondition_broadcast(dcCondition *_condition)
{
    WakeAllConditionVariable(&_condition->condition);
}

void dcCondition_wait(dcCondition *_condition, dcMutex *_mutex)
{
    SleepConditionVariableCS(&_condition->condition, &_mutex->mutex, INFINITE);
}
