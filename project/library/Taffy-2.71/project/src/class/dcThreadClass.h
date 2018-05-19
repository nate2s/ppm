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

#ifndef __DC_THREAD_CLASS_H__
#define __DC_THREAD_CLASS_H__

#include "dcDefines.h"

typedef enum
{
    THREAD_IDLE      = 0,
    THREAD_RUNNING   = 1,
    THREAD_SUSPENDED = 2
}  dcThreadClassAuxState;

//////////////////////
// dcThreadClassAux //
//////////////////////

typedef void (*ThreadCallback)(struct dcNode_t *_result,
                               struct dcNode_t *_token);

struct dcThreadClassAux_t
{
    // body is of type procedure //
    struct dcNode_t *body;
    struct dcNode_t *arguments;

    struct dcMutex_t *stateMutex;
    struct dcCondition_t *suspendCondition;
    struct dcThreadId_t *threadId;
    uint32_t id;

    dcThreadClassAuxState state;

    struct dcNode_t *result;
    struct dcList_t *messages;

    ThreadCallback callback;
    struct dcNode_t *callbackToken;

    struct dcNodeEvaluator_t *evaluator;
};

typedef struct dcThreadClassAux_t dcThreadClassAux;

///////////////////
// dcThreadClass //
///////////////////

// creating //
struct dcNode_t *dcThreadClass_createNode(struct dcNode_t *_body,
                                          bool _object);
struct dcNode_t *dcThreadClass_createObject(struct dcNode_t *_body);

// getting //
struct dcNode_t *dcThreadClass_getBody(const struct dcNode_t *_thread);

// setting //
void dcThreadClass_setBody(struct dcNode_t *_thread,
                           struct dcNode_t *_body);

struct dcNode_t *dcThreadClass_wait_local(struct dcNode_t *_thread);

void dcThreadClass_setCallback(struct dcNode_t *_thread,
                               ThreadCallback _callback,
                               struct dcNode_t *_token);

struct dcNode_t *dcThreadClass_startHelper(struct dcNode_t *_receiver,
                                           struct dcNode_t *_arguments);

// standard functions //
ALLOCATE_FUNCTION(dcThreadClass_allocateNode);
COPY_FUNCTION(dcThreadClass_copyNode);
DEALLOCATE_FUNCTION(dcThreadClass_deallocateNode);
FREE_FUNCTION(dcThreadClass_freeNode);
GET_TEMPLATE_FUNCTION(dcThreadClass_getTemplate);
MARK_FUNCTION(dcThreadClass_markNode);
REGISTER_FUNCTION(dcThreadClass_registerNode);

TAFFY_C_METHOD(dcThreadMetaClass_new);

TAFFY_C_METHOD(dcThreadClass_addMessage);
TAFFY_C_METHOD(dcThreadClass_asString);
TAFFY_C_METHOD(dcThreadClass_getMessage);
TAFFY_C_METHOD(dcThreadClass_hasMessage);
TAFFY_C_METHOD(dcThreadClass_id);
TAFFY_C_METHOD(dcThreadClass_wait);
TAFFY_C_METHOD(dcThreadClass_kill);
TAFFY_C_METHOD(dcThreadClass_result);
TAFFY_C_METHOD(dcThreadClass_start);
TAFFY_C_METHOD(dcThreadClass_startWith);

#define THREAD_PACKAGE_NAME THREADING_PACKAGE_NAME
#define THREAD_CLASS_NAME "Thread"

#endif
