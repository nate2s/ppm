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

#include <signal.h>

#include "dcThreadClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcCondition.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcIOClass.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcSystem.h"
#include "dcMainClass.h"
#include "dcMethodHeader.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNilClass.h"
#include "dcNode.h"
#include "dcNoClass.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcNumberClass.h"
#include "dcObjectClass.h"
#include "dcParser.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcThread.h"
#include "dcThreadId.h"
#include "dcYesClass.h"

/////////////////////////////
// dcThreadClassStartData  //
/////////////////////////////

struct StartData_t
{
    struct dcNode_t *thread;
    struct dcNode_t *arguments;
};

typedef struct StartData_t StartData;

static StartData *createStartData(dcNode *_thread, dcNode *_arguments)
{
    StartData *data = (StartData *)dcMemory_allocate(sizeof(StartData));
    data->thread = _thread;
    data->arguments = _arguments;
    return data;
}

static void freeStartData(StartData **_startData)
{
    dcMemory_free(*_startData);
}

static int currentId = 0;

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "new:",
        SCOPE_DATA_PUBLIC,
        &dcThreadMetaClass_new,
        gCFunctionArgument_procedure
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "addMessage:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcThreadClass_addMessage,
        gCFunctionArgument_wild
    },
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcThreadClass_asString,
        gCFunctionArgument_none
    },
    {
        "hasMessage?",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcThreadClass_hasMessage,
        gCFunctionArgument_none
    },
    {
        "id",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcThreadClass_id,
        gCFunctionArgument_none
    },
    {
        "wait",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcThreadClass_wait,
        gCFunctionArgument_none
    },
    {
        "kill",
        SCOPE_DATA_PUBLIC,
        &dcThreadClass_kill,
        gCFunctionArgument_none
    },
    {
        "result",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcThreadClass_result,
        gCFunctionArgument_none
    },
    {
        "start",
        SCOPE_DATA_PUBLIC,
        &dcThreadClass_start,
        gCFunctionArgument_none
    },
    {
        "startWith:",
        SCOPE_DATA_PUBLIC,
        &dcThreadClass_startWith,
        gCFunctionArgument_array
    },
    {
        0
    }
};

#define CAST_THREAD_AUX(_node_) ((dcThreadClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcThreadClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (THREAD_PACKAGE_NAME,                      // package name
          THREAD_CLASS_NAME,                        // class name
          MAKE_FULLY_QUALIFIED(OBJECT),             // super name
          CLASS_ABSTRACT,                           // class flags
          NO_FLAGS,                                 // scope data flags
          sMetaMethodWrappers,                      // meta methods
          sMethodWrappers,                          // methods
          NULL,                                     // initialization function
          NULL,                                     // deinitialization function
          &dcThreadClass_allocateNode,              // allocate
          &dcThreadClass_deallocateNode,            // deallocate
          NULL,                                     // meta mark
          &dcThreadClass_markNode,                  // mark
          &dcThreadClass_copyNode,                  // copy
          &dcThreadClass_freeNode,                  // free
          &dcThreadClass_registerNode,              // register
          NULL,                                     // marshall
          NULL,                                     // unmarshall
          NULL));                                   // set template
}

static dcThreadClassAux *createAux(dcNode *_body)
{
    dcThreadClassAux *aux =
        (dcThreadClassAux *)(dcMemory_allocateAndInitialize
                             (sizeof(dcThreadClassAux)));

    aux->body = _body;
    aux->state = THREAD_IDLE;
    aux->messages = dcList_create();
    aux->id = currentId++;
    aux->arguments = NULL;
    aux->stateMutex = dcMutex_create(false);
    aux->suspendCondition = dcCondition_create();

    // thread hasn't been executed yet //
    aux->result = NULL;
    return aux;
}

static void lock(dcNode *_thread)
{
    dcMutex_lock(CAST_THREAD_AUX(_thread)->stateMutex);
}

static void unlock(dcNode *_thread)
{
    dcMutex_unlock(CAST_THREAD_AUX(_thread)->stateMutex);
}

static void setState(dcThreadClassAux *_aux, dcThreadClassAuxState _state)
{
    dcMutex_lock(_aux->stateMutex);
    _aux->state = _state;
    dcMutex_unlock(_aux->stateMutex);
}

void dcThreadClass_allocateNode(dcNode *_node)
{
    dcError_assert(CAST_CLASS_AUX(_node) == NULL);
    CAST_CLASS_AUX(_node) = createAux(dcNilClass_getInstance());
}

void dcThreadClassAux_setResult(dcThreadClassAux *_aux, dcNode *_result)
{
    _aux->result = _result;
}

dcNode *dcThreadClass_createNode(dcNode *_body, bool _object)
{
    dcNode *result = dcClass_createNode(sTemplate,
                                        NULL, // super
                                        NULL, // scope
                                        _object,
                                        NULL); // aux
    // help helgrind
    dcClass_lock(result);
    CAST_THREAD_AUX(result)->body = _body;
    dcClass_unlock(result);
    return result;
}

dcNode *dcThreadClass_createObject(dcNode *_body)
{
    return dcThreadClass_createNode(_body, true);
}

void dcThreadClass_deallocateNode(dcNode *_thread)
{
    // help helgrind
    dcClass_lock(_thread);
    dcThreadClassAux *aux = CAST_THREAD_AUX(_thread);
    aux->body = NULL;
    dcList_clear(aux->messages, DC_SHALLOW);
    aux->result = NULL;
    dcClass_unlock(_thread);
}

void dcThreadClass_freeNode(dcNode *_threadNode, dcDepth _depth)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_threadNode);
    dcMutex_free(&aux->stateMutex);
    dcCondition_free(&aux->suspendCondition);
    dcThreadId_free(&aux->threadId);
    dcList_free(&aux->messages, DC_SHALLOW);
    dcMemory_free(aux);
}

void dcThreadClass_copyNode(dcNode *_to,
                            const dcNode *_from,
                            dcDepth _depth)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_from);
    dcNode *bodyCopy = (aux->body == NULL
                        ? dcBlockClass_createObject(NULL, NULL)
                        : dcGraphData_copyTree(aux->body));
    CAST_CLASS_AUX(_to) = createAux(bodyCopy);
}

void dcThreadClass_registerNode(dcNode *_thread)
{
    dcNode_register(CAST_THREAD_AUX(_thread)->body);
}

void dcThreadClass_markNode(dcNode *_node)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_node);
    dcNode_mark(aux->body);
    dcNode_mark(aux->arguments);
    dcNode_mark(aux->result);
}

dcNode *dcThreadClass_getBody(const dcNode *_threadNode)
{
    return CAST_THREAD_AUX(_threadNode)->body;
}

void dcThreadClass_setBody(dcNode *_threadNode, dcNode *_body)
{
    TAFFY_DEBUG(dcError_assert(dcProcedureClass_isMe(_body)
                               || dcNilClass_isMe(_body)));

    CAST_THREAD_AUX(_threadNode)->body = _body;
}

/////////////////////////
// Thread meta methods //
/////////////////////////
dcNode *dcThreadMetaClass_new(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *toThread = dcNode_copyIfTemplate(dcArray_get(_arguments, 0));
    return dcNode_register(dcThreadClass_createObject(toThread));
}

static void *callBody(void *_argument)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    StartData *data = (StartData *)_argument;
    dcThreadClassAux *aux = CAST_THREAD_AUX(data->thread);

    dcNodeEvaluator_pushMark(evaluator, data->arguments);
    dcThread_ignoreAllSignals();

    // the body can be Nil (uninitialized) or a Procedure
    if (! dcNilClass_isMe(aux->body))
    {
        dcNode *body = aux->body;

        if (dcGraphData_isType(body, NODE_GRAPH_DATA_TREE))
        {
            body = dcGraphDataTree_getContents(body);
        }

        // XXXXXXXXXXXXXXXX revisit this conversion
        dcList *arguments =
            (data->arguments == NULL
             ? dcList_create()
             : dcList_createFromArray(dcArrayClass_getObjects
                                      (data->arguments)));

        dcNodeEvaluator_setPosition(evaluator, body);

        aux->result = (dcNodeEvaluator_evaluateProcedure
                       (evaluator,
                        data->thread,                             // self
                        dcClass_castNode(body,                    // body
                                         dcProcedureClass_getTemplate(),
                                         false),
                        NO_FLAGS,                                 // flags
                        arguments));                              // arguments
        dcList_free(&arguments, DC_SHALLOW);
    }

    // arguments
    dcNodeEvaluator_popMark(evaluator);

    aux->result = (aux->result != NULL
                   ? aux->result
                   : evaluator->exception);
    return aux->result;
}

static void *startHelper(void *_data)
{
    StartData *data = (StartData*)_data;
    dcParser_lock();
    dcNodeEvaluator *evaluator = dcNodeEvaluator_create();
    dcParser_unlock();
    dcThreadClassAux *aux = CAST_THREAD_AUX(data->thread);
    aux->evaluator = evaluator;
    setState(aux, THREAD_RUNNING);

    // execute the body
    dcNodeEvaluator_synchronizeFunctionCall(evaluator, &callBody, data);

    // do a final check for any uncaught <exceptions>
    char *exception = dcNodeEvaluator_generateExceptionText(evaluator);

    if (exception != NULL)
    {
        dcIOClass_printFormat("%s\n", exception);
        dcMemory_free(exception);
    }
    // </exceptions>

    lock(data->thread);
    dcNode *result = aux->result;
    unlock(data->thread);

    if (aux->callback != NULL)
    {
        aux->callback(result, aux->callbackToken);
    }

    dcNodeEvaluator_free(&evaluator);

    setState(aux, THREAD_IDLE);
    dcThread_detach(aux->threadId);
    dcCondition_signal(aux->suspendCondition);

    dcSystem_unregisterThread(data->thread);
    freeStartData(&data);
    return result;
}

static void startWithData(dcNode *_thread, dcNode *_arguments)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_thread);
    aux->state = THREAD_RUNNING;
    aux->arguments = _arguments;

    dcSystem_registerThread(_thread);
    aux->threadId =
        dcThread_create(&startHelper,
                        (void *)createStartData(_thread, _arguments));
}

dcNode *dcThreadClass_startHelper(dcNode *_receiver, dcNode *_arguments)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_receiver);
    dcNode *result = NULL;
    lock (_receiver);

    //
    // <critical> section
    //
    if (aux->state == THREAD_IDLE)
    {
        startWithData(_receiver, _arguments);
        result = _receiver;
    }
    else
    {
        result = dcNoClass_getInstance();
    }

    unlock(_receiver);
    //
    // </critical> section
    //

    return result;
}

// taffy methods //
dcNode *dcThreadClass_start(dcNode *_receiver, dcArray *_arguments)
{
    return dcThreadClass_startHelper(_receiver, NULL);
}

dcNode *dcThreadClass_startWith(dcNode *_receiver, dcArray *_arguments)
{
    return dcThreadClass_startHelper(_receiver,
                                     dcNode_copyIfTemplate
                                     (dcArray_get(_arguments, 0)));
}

dcNode *dcThreadClass_addMessage(dcNode *_receiver, dcArray *_arguments)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_receiver);
    lock(_receiver);
    dcList_push(aux->messages, dcArray_get(_arguments, 0));
    unlock(_receiver);
    return dcYesClass_getInstance();
}

dcNode *dcThreadClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcStringClass_createObject
             (dcLexer_sprintf("#Thread(%u)", CAST_THREAD_AUX(_receiver)->id),
              false)));
}

dcNode *dcThreadClass_id(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32u
             (CAST_THREAD_AUX(_receiver)->id)));
}

static void waitForSuspend(dcNode *_receiver)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_receiver);

    while (aux->state == THREAD_RUNNING)
    {
        dcGarbageCollector_nodeEvaluatorDown();
        dcCondition_wait(aux->suspendCondition, aux->stateMutex);
        dcGarbageCollector_nodeEvaluatorBlockUp();
    }
}

dcNode *dcThreadClass_kill(dcNode *_receiver, dcArray *_arguments)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_receiver);
    dcNode *result = dcNoClass_getInstance();
    lock(_receiver);

    if (aux->state == THREAD_RUNNING)
    {
        dcNodeEvaluator_setExiting(aux->evaluator);
        waitForSuspend(_receiver);
        result = dcYesClass_getInstance();
    }

    unlock(_receiver);
    return result;
}

dcNode *dcThreadClass_wait(dcNode *_receiver, dcArray *_arguments)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_receiver);
    lock(_receiver);

    if (aux->state == THREAD_RUNNING)
    {
        waitForSuspend(_receiver);
    }

    unlock(_receiver);
    return _receiver;
}

dcNode *dcThreadClass_result(dcNode *_receiver, dcArray *_arguments)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_receiver);
    return (aux->result != NULL
            ? aux->result
            : dcNilClass_getInstance());
}

dcNode *dcThreadClass_getMessage(dcNode *_receiver, dcArray *_arguments)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_receiver);
    dcNode *result = dcList_shift(aux->messages, DC_SHALLOW);
    return (result == NULL
            ? dcNilClass_getInstance()
            : result);
}

dcNode *dcThreadClass_hasMessage(dcNode *_receiver, dcArray *_arguments)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_receiver);
    lock(_receiver);
    dcNode *result = (aux->messages->size > 0
                      ? dcYesClass_getInstance()
                      : dcNoClass_getInstance());
    unlock(_receiver);
    return result;
}

dcNode *dcThreadClass_body(dcNode *_receiver, dcArray *_arguments)
{
    return CAST_THREAD_AUX(_receiver)->body;
}

void dcThreadClass_setCallback(dcNode *_thread,
                               ThreadCallback _callback,
                               dcNode *_token)
{
    dcThreadClassAux *aux = CAST_THREAD_AUX(_thread);
    aux->callback = _callback;
    aux->callbackToken = _token;
}
