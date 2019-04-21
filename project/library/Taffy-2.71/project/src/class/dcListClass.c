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

#include "CompiledList.h"
#include "dcListClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcContainerClass.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcLineContainerClass.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMarshaller.h"
#include "dcMethodHeader.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcPairClass.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcYesClass.h"

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "fromArray:",
        SCOPE_DATA_PUBLIC,
        &dcListMetaClass_fromArray,
        gCFunctionArgument_array
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "concat:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcListClass_concat,
        gCFunctionArgument_list
    },
    {
        "collect:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONST),
        &dcListClass_collect,
        gCFunctionArgument_procedure
    },
    {
        "collect!:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE
         | SCOPE_DATA_BREAKTHROUGH),
        &dcListClass_collectBang,
        gCFunctionArgument_procedure
    },
    {
        "each:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONST),
        &dcListClass_each,
        gCFunctionArgument_procedure
    },
    {
        "#operator(==):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcListClass_equals,
        gCFunctionArgument_wild
    },
    {
        "head",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcListClass_head,
        gCFunctionArgument_none
    },
    {
        "#operator([]):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcListClass_objectAtIndex,
        gCFunctionArgument_number
    },
    {
        "objectAtIndex:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcListClass_objectAtIndex,
        gCFunctionArgument_number
    },
    {
        "pop",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcListClass_pop,
        gCFunctionArgument_none
    },
    {
        "push:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcListClass_push,
        gCFunctionArgument_wild
    },
    {
        "reject:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONST),
        &dcListClass_reject,
        gCFunctionArgument_procedure
    },
    {
        "reject!:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE
         | SCOPE_DATA_BREAKTHROUGH),
        &dcListClass_rejectBang,
        gCFunctionArgument_procedure
    },
    {
        "removeAll",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcListClass_removeAll,
        gCFunctionArgument_none
    },
    {
        "reverse!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcListClass_reverseBang,
        gCFunctionArgument_none
    },
    {
        "select:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONST),
        &dcListClass_select,
        gCFunctionArgument_procedure
    },
    {
        "select!:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE
         | SCOPE_DATA_BREAKTHROUGH),
        &dcListClass_selectBang,
        gCFunctionArgument_procedure
    },
    {
        "shift",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcListClass_shift,
        gCFunctionArgument_none
    },
    {
        "size",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcListClass_size,
        gCFunctionArgument_none
    },
    {
        "tail",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcListClass_tail,
        gCFunctionArgument_none
    },
    {
        "unshift:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcListClass_unshift,
        gCFunctionArgument_wild
    },
    {
        0
    }
};

#define CAST_LIST_AUX(_node_) ((dcListClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcListClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (LIST_PACKAGE_NAME,                      // package name
          LIST_CLASS_NAME,                        // class name
          "LineContainer",                        // super name
          CLASS_HAS_READ_WRITE_LOCK,              // class flags
          NO_FLAGS,                               // scope data flags
          sMetaMethodWrappers,                    // meta methods
          sMethodWrappers,                        // methods
          &dcListClass_initialize,                // initialization function
          NULL,                                   // deinitialization function
          &dcListClass_allocateNode,              // allocate
          &dcListClass_deallocateNode,            // deallocate
          NULL,                                   // meta mark
          &dcListClass_markNode,                  // mark
          &dcListClass_copyNode,                  // copy
          &dcListClass_freeNode,                  // free
          &dcListClass_registerNode,              // register
          &dcListClass_marshallNode,              // marshall
          &dcListClass_unmarshallNode,            // unmarshall
          NULL));                                 // set template
};

static dcListClassAux *createAux(dcList *_objects, bool _initialized)
{
    dcListClassAux *aux =
        (dcListClassAux *)(dcMemory_allocateAndInitialize
                           (sizeof(dcListClassAux)));
    aux->objects = (_objects != NULL
                    ? _objects
                    : dcList_create());
    aux->initialized = _initialized;
    return aux;
}

void dcListClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux(NULL, true);
}

void dcListClass_deallocateNode(dcNode *_node)
{
    dcList_clear(dcListClass_getObjects(_node), DC_SHALLOW);
}

dcNode *dcListClass_createNode(dcList *_objects,
                               bool _initialized,
                               bool _object)
{
    return (dcClass_createNode
            (sTemplate,
             dcLineContainerClass_createNode(_object),
             NULL, // scope
             _object,
             createAux(_objects, _initialized)));
}

dcNode *dcListClass_createObject(dcList *_objects, bool _initialized)
{
    return dcListClass_createNode(_objects, _initialized, true);
}

dcNode *dcListClass_createEmptyObject(void)
{
    return dcListClass_createNode(dcList_create(), true, true);
}

void dcListClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcListClassAux *aux = CAST_LIST_AUX(_node);

    if (aux != NULL)
    {
        dcList_free(&aux->objects, DC_DEEP);
        dcMemory_free(aux);
    }
}

#define LIST_CLASS_TAFFY_FILE_NAME "src/class/List.ty"

void dcListClass_initialize(void)
{
    dcError_assert(dcStringEvaluator_evalString(__compiledList,
                                                LIST_CLASS_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

void dcListClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcListClassAux *aux = CAST_LIST_AUX(_from);
    CAST_CLASS_AUX(_to) = createAux
        (dcList_copy(aux->objects, _depth), CAST_LIST_AUX(_from)->initialized);
}

void dcListClass_markNode(dcNode *_classNode)
{
    dcList *objects = dcListClass_getObjects(_classNode);
    dcListElement *listIt = objects->head;

    while (listIt != NULL)
    {
        dcNode_mark(listIt->object);
        listIt = listIt->next;
    }
}

void dcListClass_registerNode(dcNode *_classNode)
{
    dcList *objects = dcListClass_getObjects(_classNode);
    dcListElement *listIt = objects->head;

    while (listIt != NULL)
    {
        dcNode_register(listIt->object);
        listIt = listIt->next;
    }
}

dcNode *dcListClass_buildEmptyObject(void)
{
    return dcListClass_createNode(NULL, false, true);
}

bool dcListClass_isInitialized(const dcNode *_listNode)
{
    return CAST_LIST_AUX(_listNode)->initialized;
}

void dcListClass_setInitialized(dcNode *_listNode)
{
    CAST_LIST_AUX(_listNode)->initialized = true;
}

dcList *dcListClass_getObjects(const dcNode *_listNode)
{
    return CAST_LIST_AUX(_listNode)->objects;
}

dcContainerSizeType dcListClass_getSize(const struct dcNode_t *_node)
{
    return dcListClass_getObjects(_node)->size;
}

void dcListClass_setObjects(dcNode *_listNode,
                            dcList *_objects,
                            dcDepth _depth)
{
    dcListClassAux *aux = CAST_LIST_AUX(_listNode);

    if (aux->objects != NULL)
    {
        dcList_free(&aux->objects, _depth);
    }

    aux->objects = _objects;
}

dcNode *dcListClass_isEmpty(dcNode *_receiver, dcArray *_arguments)
{
    return (dcListClass_getSize(_receiver) == 0
            ? dcYesClass_getInstance()
            : dcNoClass_getInstance());
}

dcNode *dcListClass_size(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *integer = dcNumberClass_createObjectFromInt32s
        ((int32_t)dcListClass_getSize(_receiver));
    dcNode_register(integer);
    return integer;
}

dcNode *dcListMetaClass_fromArray(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *arrayNode = dcArray_get(_arguments, 0);
    dcArray *arrayObjects = dcArrayClass_getObjects(arrayNode);
    dcList *list = dcList_createFromArray(arrayObjects);

    // list is already initialized //
    dcNode *listObject = dcListClass_createObject(list, true);
    dcNode_register(listObject);
    return listObject;
}

typedef dcNode *(*LoopAction)(dcNode *_blockResult,
                              dcNode *_argument,
                              bool *_stop,
                              void *_data);

static dcNode *doBreakthroughLoop(dcNode *_receiver,
                                  dcArray *_arguments,
                                  LoopAction _action,
                                  void *_data,
                                  dcNode *_defaultResult,
                                  bool _isTrueLoop)
{
    dcNode *procedureNode = dcArray_get(_arguments, 0);
    dcMethodHeader *methodHeader =
        dcProcedureClass_getMethodHeader(procedureNode);
    dcList *blockArguments = dcMethodHeader_getArguments(methodHeader);
    dcList *list = dcListClass_getObjects(_receiver);
    dcNode *result = _defaultResult;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    size_t argumentCount = blockArguments->size;

    if (argumentCount == 1)
    {
        dcList *callArguments = dcList_create();
        dcListElement *listIt = list->head;
        dcContainerClass_startLoop(_receiver);

        if (_isTrueLoop)
        {
            dcNodeEvaluator_startLoop(evaluator);
        }

        while (listIt != NULL)
        {
            if (dcContainerClass_checkModified(_receiver))
            {
                result = NULL;
                break;
            }

            dcList_setHead(callArguments, listIt->object);

            dcNode *blockResult =
                dcNodeEvaluator_evaluateProcedure(evaluator,
                                                  _receiver,
                                                  procedureNode,
                                                  (SCOPE_DATA_BREAKTHROUGH
                                                   | SCOPE_DATA_CONST),
                                                  callArguments);
            if (blockResult == NULL
                || dcContainerClass_checkModified(_receiver))
            {
                result = NULL;
                break;
            }
            else if (_action != NULL)
            {
                bool stop = false;
                result = _action(blockResult, listIt->object, &stop, _data);

                if (stop)
                {
                    break;
                }
            }
            else if (! dcNodeEvaluator_canContinueEvaluating(evaluator))
            {
                break;
            }

            listIt = listIt->next;
        }

        if (_isTrueLoop)
        {
            dcNodeEvaluator_stopLoop(evaluator);
        }

        dcContainerClass_stopLoop(_receiver);
        dcList_free(&callArguments, DC_SHALLOW);
    }
    else
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject(1, argumentCount);
    }

    return result;
}

static dcNode *selectAction(dcNode *_blockResult,
                            dcNode *_argument,
                            bool *_stop,
                            void *_data)
{
    dcList *selected = (dcList*)_data;

    if (_blockResult != dcNoClass_getInstance())
    {
        dcList_push(selected, _argument);
    }

    return dcYesClass_getInstance();
}

static dcList *selectHelper(dcNode *_receiver, dcArray *_arguments)
{
    dcList *selected = dcList_create();

    if (doBreakthroughLoop(_receiver,
                           _arguments,
                           &selectAction,
                           selected,
                           dcYesClass_getInstance(),
                           false)
        == NULL)
    {
        dcList_free(&selected, DC_SHALLOW);
        selected = NULL;
    }

    return selected;
}

dcNode *dcListClass_select(dcNode *_receiver, dcArray *_arguments)
{
    dcList *selected = selectHelper(_receiver, _arguments);
    return (selected == NULL
            ? NULL
            : dcNode_register(dcListClass_createObject(selected, true)));
}

dcNode *dcListClass_selectBang(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcList *selected = selectHelper(_receiver, _arguments);

    if (selected != NULL)
    {
        dcList_free(&(CAST_LIST_AUX(_receiver)->objects), DC_SHALLOW);
        CAST_LIST_AUX(_receiver)->objects = selected;
        result = _receiver;
    }

    return result;
}

dcNode *dcListClass_each(dcNode *_receiver, dcArray *_arguments)
{
    return doBreakthroughLoop(_receiver,
                              _arguments,
                              NULL,
                              NULL,
                              dcNilClass_getInstance(),
                              true);
}

static dcNode *collectAction(dcNode *_blockResult,
                             dcNode *_argument,
                             bool *_stop,
                             void *_data)
{
    dcList *collected = (dcList*)_data;
    dcList_push(collected, _blockResult);
    return dcYesClass_getInstance();
}

static dcList *collectHelper(dcNode *_receiver, dcArray *_arguments)
{
    dcList *collected = dcList_create();

    if (doBreakthroughLoop(_receiver,
                           _arguments,
                           &collectAction,
                           collected,
                           dcYesClass_getInstance(),
                           false)
        == NULL)
    {
        dcList_free(&collected, DC_SHALLOW);
    }

    return collected;
}

dcNode *dcListClass_collect(dcNode *_receiver, dcArray *_arguments)
{
    dcList *collected = collectHelper(_receiver, _arguments);
    return (collected == NULL
            ? NULL
            : dcNode_register(dcListClass_createObject(collected, true)));
}

dcNode *dcListClass_collectBang(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcList *collected = collectHelper(_receiver, _arguments);

    if (collected != NULL)
    {
        dcList_free(&(CAST_LIST_AUX(_receiver)->objects), DC_SHALLOW);
        CAST_LIST_AUX(_receiver)->objects = collected;
        result = _receiver;
    }

    return result;
}

dcNode *dcListClass_head(dcNode *_receiver, dcArray *_arguments)
{
    dcList *list = dcListClass_getObjects(_receiver);
    dcNode *result = dcList_getHead(list);

    if (result == NULL)
    {
        dcEmptyListExceptionClass_throwObject();
    }

    return result;
}

static dcNode *objectAtIndex_helper(dcNode *_receiver, dcNode *_index)
{
    dcList *list = dcListClass_getObjects(_receiver);
    dcNode *result = NULL;
    uint32_t listIndex = 0;

    if (dcNumberClass_extractIndex(_index, &listIndex, list->size))
    {
        uint32_t i = 0;
        dcListElement *that = list->head;

        for (i = 0; i < listIndex; i++, that = that->next)
        {
        }

        result = that->object;
    }

    return result;
}

dcNode *dcListClass_objectAtIndex(dcNode *_receiver, dcArray *_arguments)
{
    return objectAtIndex_helper(_receiver, dcArray_get(_arguments, 0));
}

dcNode *dcListClass_tail(dcNode *_receiver, dcArray *_arguments)
{
    dcList *list = dcListClass_getObjects(_receiver);
    dcNode *result = dcList_getTail(list);

    if (result == NULL)
    {
        dcEmptyListExceptionClass_throwObject();
    }

    return result;
}

dcNode *dcListClass_equals(dcNode *_receiver, dcArray *_arguments)
{
    dcList *list1 = dcListClass_getObjects(_receiver);
    dcNode *preList2 = dcArray_get(_arguments, 0);
    bool exception = false;
    bool matched = true;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    preList2 = dcClass_castNode(preList2, sTemplate, false);

    if (preList2 == NULL)
    {
        matched = false;
    }
    else
    {
        dcList *list2 = dcListClass_getObjects(preList2);

        if (list1->size != list2->size)
        {
            matched = false;
        }
        else
        {
            dcListElement *list1It = list1->head;
            dcListElement *list2It = list2->head;

            dcContainerClass_startLoop(_receiver);
            dcContainerClass_startLoop(preList2);

            while (list1It)
            {
                if (dcContainerClass_checkModified(_receiver)
                    || dcContainerClass_checkModified(preList2))
                {
                    exception = true;
                    break;
                }

                dcNode *node1 = list1It->object;
                dcNode *node2 = list2It->object;
                dcNode *evaluateResult = dcNodeEvaluator_callMethodWithArgument
                    (evaluator,
                     node1,
                     dcSystem_getOperatorName(TAFFY_EQUALS),
                     node2);

                if (evaluateResult == NULL)
                {
                    // d'oh //
                    exception = true;
                    break;
                }
                else if (evaluateResult != dcYesClass_getInstance())
                {
                    // failed to match //
                    matched = false;
                    break;
                }

                list1It = list1It->next;
                list2It = list2It->next;
            }

            dcNodeEvaluator_setBreaking(evaluator, false);
            dcContainerClass_stopLoop(preList2);
            dcContainerClass_stopLoop(_receiver);
        }
    }

    return (exception
            ? NULL
            : (matched
               ? dcYesClass_getInstance()
               : dcNoClass_getInstance()));
}

dcNode *dcListClass_shift(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result =
        dcList_shift(dcListClass_getObjects(_receiver), DC_SHALLOW);
    return (result == NULL
            ? dcNilClass_getInstance()
            : ((dcContainerClass_setModified(_receiver, true)
                == NULL)
               ? NULL
               : result));
}

typedef void (*AddFunctionPointer)(dcList *_list, dcNode *_object);

static dcNode *addObject(dcNode *_receiver,
                         dcArray *_arguments,
                         AddFunctionPointer _function)
{
    _function(dcListClass_getObjects(_receiver),
              dcNode_copyIfTemplate(dcArray_get(_arguments, 0)));
    return dcContainerClass_setModified(_receiver, true);
}

dcNode *dcListClass_push(dcNode *_receiver, dcArray *_arguments)
{
    return addObject(_receiver, _arguments, &dcList_push);
}

dcNode *dcListClass_unshift(dcNode *_receiver, dcArray *_arguments)
{
    return addObject(_receiver, _arguments, &dcList_unshift);
}

dcNode *dcListClass_concat(dcNode *_receiver, dcArray *_arguments)
{
    dcList_concat(dcListClass_getObjects(_receiver),
                  dcListClass_getObjects(dcArray_get(_arguments, 0)),
                  DC_SHALLOW);
    return dcContainerClass_setModified(_receiver, true);
}

dcNode *dcListClass_pop(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcList_pop(dcListClass_getObjects(_receiver), DC_SHALLOW);
    return (result == NULL
            ? dcNilClass_getInstance()
            : ((dcContainerClass_setModified(_receiver, true)
                == NULL)
               ? NULL
               : result));
}

static dcNode *rejectAction(dcNode *_blockResult,
                            dcNode *_argument,
                            bool *_stop,
                            void *_data)
{
    dcList *survivors = (dcList*)_data;

    if (_blockResult == dcNoClass_getInstance())
    {
        dcList_push(survivors, _argument);
    }

    return dcYesClass_getInstance();
}

static dcList *rejectHelper(dcNode *_receiver, dcArray *_arguments)
{
    dcList *survivors = dcList_create();

    if (doBreakthroughLoop(_receiver,
                           _arguments,
                           &rejectAction,
                           survivors,
                           dcYesClass_getInstance(),
                           false)
        == NULL)
    {
        dcList_free(&survivors, DC_SHALLOW);
        survivors = NULL;
    }

    return survivors;
}

dcNode *dcListClass_reject(dcNode *_receiver, dcArray *_arguments)
{
    dcList *survivors = rejectHelper(_receiver, _arguments);
    return (survivors == NULL
            ? NULL
            : dcNode_register(dcListClass_createObject(survivors, true)));
}

dcNode *dcListClass_rejectBang(dcNode *_receiver, dcArray *_arguments)
{
    dcList *survivors = rejectHelper(_receiver, _arguments);
    dcNode *result = NULL;

    if (survivors != NULL)
    {
        dcList_free(&(CAST_LIST_AUX(_receiver)->objects), DC_SHALLOW);
        CAST_LIST_AUX(_receiver)->objects = survivors;
        dcContainerClass_setModified(_receiver, true);
        result = _receiver;
    }

    return result;
}

dcNode *dcListClass_removeAll(dcNode *_receiver, dcArray *_arguments)
{
    dcList *toClear = dcListClass_getObjects(_receiver);
    dcList_clear(toClear, DC_SHALLOW);
    return _receiver;
}

static dcList *reverseHelper(dcNode *_receiver, dcArray *_arguments)
{
    dcList *list = dcListClass_getObjects(_receiver);
    dcList *reverseList = dcList_create();
    reverseList->size = list->size;
    dcContainerClass_startLoop(_receiver);

    FOR_EACH_IN_LIST(reverseList, that)
    {
        if (dcContainerClass_checkModified(_receiver))
        {
            dcList_free(&reverseList, DC_SHALLOW);
            reverseList = NULL;
            break;
        }

        dcList_unshift(reverseList, that->object);
    }

    dcNodeEvaluator_setBreaking(dcSystem_getCurrentNodeEvaluator(), false);
    dcContainerClass_stopLoop(_receiver);
    return reverseList;
}

dcNode *dcListClass_reverseBang(dcNode *_receiver, dcArray *_arguments)
{
    dcList *reversed = reverseHelper(_receiver, _arguments);
    dcNode *result = NULL;

    if (reversed != NULL)
    {
        dcList_free(&(CAST_LIST_AUX(_receiver)->objects), DC_SHALLOW);
        CAST_LIST_AUX(_receiver)->objects = reversed;
        dcContainerClass_setModified(_receiver, true);
        result = _receiver;
    }

    return result;
}

dcNode *dcListClass_hash(dcNode *_receiver, dcArray *_arguments)
{
    // 37!
    uint32_t hash = 37;
    dcNode *result = NULL;
    bool exception = false;
    int32_t callResultHash = 0;
    dcList *list = dcListClass_getObjects(_receiver);
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    dcContainerClass_startLoop(_receiver);

    FOR_EACH_IN_LIST(list, that)
    {
        if (dcContainerClass_checkModified(_receiver))
        {
            exception = true;
            break;
        }

        dcNode *callResult =
            dcNodeEvaluator_callMethod(evaluator, that->object, "hash");

        if (callResult == NULL
            || dcContainerClass_checkModified(_receiver))
        {
            exception = true;
            break;
        }

        if (! dcNumberClass_extractInt32s(callResult, &callResultHash))
        {
            // set the exception //
            dcInvalidHashValueExceptionClass_throwObject(callResult);
            exception = true;
            break;
        }

        hash += callResultHash;
    }

    dcNodeEvaluator_setBreaking(evaluator, false);
    dcContainerClass_stopLoop(_receiver);

    if (!exception)
    {
        result = dcNumberClass_createObjectFromInt32s(hash);
        dcNode_register(result);
    }

    return result;
}

bool dcListClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    bool isInitialized;
    dcList *objects;

    if (dcMarshaller_unmarshallNoNull(_stream, "bl", &isInitialized, &objects))
    {
        CAST_CLASS_AUX(_node) = createAux(objects, false);
        result = true;
    }

    return result;
}

dcString *dcListClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    return dcMarshaller_marshall(_stream,
                                 "bl",
                                 dcListClass_isInitialized(_node),
                                 dcListClass_getObjects(_node));
}
