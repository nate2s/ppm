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

#ifndef __DC_THREAD_H__
#define __DC_THREAD_H__

#include "dcDefines.h"

#include "dcThreadInclude.h"

typedef void *(*ThreadFunction)(void *_argument);

/**
 * @brief Create a thread
 **/
struct dcThreadId_t *dcThread_create(ThreadFunction _function,
                                     void *_argument);

/**
 * @brief Join a thread
 **/
void dcThread_join(struct dcThreadId_t *_threadId);

/**
 * @brief Detach a thread
 **/
void dcThread_detach(struct dcThreadId_t *_threadId);

/**
 * @brief Ignore all signals
 **/
void dcThread_ignoreAllSignals(void);

/**
 * @brief Get the thread self
 **/
struct dcNode_t *dcThread_getSelf(void);

/**
 * @brief Get the thread self
 **/
dcTaffyThreadId dcThread_getSelfId(void);

#endif
