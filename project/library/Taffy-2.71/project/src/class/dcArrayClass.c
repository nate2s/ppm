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

#include "CompiledArray.h"
#include "CompiledInvalidArraySizeException.h"

#include "dcArrayClass.h"
#include "dcArray.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcContainerClass.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcFlatArithmetic.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcLineContainerClass.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNilClass.h"
#include "dcNumberClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
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
        "createWithSize:",
        SCOPE_DATA_PUBLIC,
        &dcArrayMetaClass_createWithSize,
        gCFunctionArgument_number
    },
    {
        NULL
    }
};

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_CONST),
        &dcArrayClass_asString,
        gCFunctionArgument_none
    },
    {
        "clear",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcArrayClass_clear,
        gCFunctionArgument_none
    },
    {
        "collect!:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_WRITE
         | SCOPE_DATA_CONTAINER_LOOP),
        &dcArrayClass_collectBang,
        gCFunctionArgument_procedure
    },
    {
        "collect:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_CONST),
        &dcArrayClass_collect,
        gCFunctionArgument_procedure
    },
    {
        "concat:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcArrayClass_concat,
        gCFunctionArgument_array
    },
    {
        "each:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_CONST),
        &dcArrayClass_each,
        gCFunctionArgument_procedure
    },
    {
        "eachIndex:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH),
        &dcArrayClass_eachIndex,
        gCFunctionArgument_procedure
    },
    {
        "insertObject:atIndex:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcArrayClass_insertObjectAtIndex,
        gCFunctionArgument_wildNumber
    },
    {
        "isEmpty?",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcArrayClass_isEmpty,
        gCFunctionArgument_none
    },
    {
        "#operator([]):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_GETTER
         | SCOPE_DATA_CONST),
        &dcArrayClass_objectAtIndex,
        gCFunctionArgument_number
    },
    {
        "objectAtIndex:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_GETTER
         | SCOPE_DATA_CONST),
        &dcArrayClass_objectAtIndex,
        gCFunctionArgument_number
    },
    {
        "#operator([...]):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_GETTER
         | SCOPE_DATA_CONST),
        &dcArrayClass_objectAtIndexes,
        gCFunctionArgument_array
    },
    {
        "objectAtIndexes:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_GETTER
         | SCOPE_DATA_CONST),
        &dcArrayClass_objectAtIndexes,
        gCFunctionArgument_array
    },
    {
        "#operator(==):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcArrayClass_operatorEquals,
        gCFunctionArgument_wild
    },
    {
        "reject:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcArrayClass_reject,
        gCFunctionArgument_procedure
    },
    {
        "reverse",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcArrayClass_reverse,
        gCFunctionArgument_none
    },
    {
        "reverse!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE
         | SCOPE_DATA_CONTAINER_LOOP),
        &dcArrayClass_reverseBang,
        gCFunctionArgument_none
    },
    {
        "select:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_CONST),
        &dcArrayClass_select,
        gCFunctionArgument_procedure
    },
    {
        "select!:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_WRITE
         | SCOPE_DATA_CONTAINER_LOOP),
        &dcArrayClass_selectBang,
        gCFunctionArgument_procedure
    },
    {
        "#operator([]=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcArrayClass_setObjectAtIndex,
        gCFunctionArgument_array
    },
    {
        "#operator([...]=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcArrayClass_setObjectAtIndexes,
        gCFunctionArgument_array
    },
    {
        "size",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcArrayClass_size,
        gCFunctionArgument_none
    },
    {
        "subArrayFrom:to:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_CONST),
        &dcArrayClass_subArrayFromTo,
        gCFunctionArgument_numberNumber
    },
    {
        0
    }
};

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcArrayClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (ARRAY_PACKAGE_NAME,                  // package name
          ARRAY_CLASS_NAME,                    // class name
          "LineContainer",                     // super type
          CLASS_HAS_READ_WRITE_LOCK,           // class flags
          NO_FLAGS,                            // scope data flags
          sMetaMethodWrappers,                 // meta methods
          sMethodWrappers,                     // methods
          &dcArrayClass_initialize,            // initialization function
          NULL,                                // deinitialization function
          &dcArrayClass_allocateNode,          // allocate
          &dcArrayClass_deallocateNode,        // disconnect
          NULL,                                // meta mark
          &dcArrayClass_markNode,              // mark
          &dcArrayClass_copyNode,              // copy
          &dcArrayClass_freeNode,              // free
          &dcArrayClass_registerNode,          // register
          &dcArrayClass_marshallNode,          // marshall
          &dcArrayClass_unmarshallNode,        // unmarshall
          NULL));                              // set template
}

#define CAST_ARRAY_AUX(_node_) ((dcArrayClassAux*)CAST_CLASS_AUX(_node_))

static dcArrayClassAux *createAux(dcArray *_objects, bool _initialized)
{
    dcArrayClassAux *aux =
        (dcArrayClassAux *)dcMemory_allocate(sizeof(dcArrayClassAux));
    aux->objects = (_objects == NULL
                    ? dcArray_createWithSize(0)
                    : _objects);
    aux->initialized = _initialized;
    return aux;
}

void dcArrayClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux(NULL, true);
}

void dcArrayClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcArrayClassAux *aux = CAST_ARRAY_AUX(_node);

    if (aux != NULL)
    {
        dcArray_free(&aux->objects, DC_DEEP);
        dcMemory_free(aux);
    }
}

#define ARRAY_TAFFY_FILE_NAME "src/class/Array.ty"
#define ARRAY_EXCEPTION_TAFFY_FILE_NAME "src/class/InvalidArraySizeException.ty"

void dcArrayClass_initialize(void)
{
    dcError_assert(dcStringEvaluator_evalString(__compiledArray,
                                                ARRAY_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
    dcError_assert(dcStringEvaluator_evalString
                   (__compiledInvalidArraySizeException,
                    ARRAY_EXCEPTION_TAFFY_FILE_NAME,
                    NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

void dcArrayClass_deallocateNode(dcNode *_node)
{
    dcArray_clear(dcArrayClass_getObjects(_node), DC_SHALLOW);
}

void dcArrayClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcArrayClassAux *aux = CAST_ARRAY_AUX(_from);
    CAST_CLASS_AUX(_to) = createAux(dcArray_copy(aux->objects, _depth),
                                    CAST_ARRAY_AUX(_from)->initialized);
}

dcNode *dcArrayClass_createNode(dcArray *_objects,
                                bool _initialized,
                                bool _object)
{
    return dcClass_createNode
        (sTemplate,
         dcLineContainerClass_createNode(_object),
         NULL,
         _object,
         createAux(_objects, _initialized));
}

dcNode *dcArrayClass_createObject(dcArray *_objects, bool _initialized)
{
    return dcArrayClass_createNode(_objects, _initialized, true);
}

dcNode *dcArrayClass_createEmptyObject(void)
{
    return dcArrayClass_createNode(dcArray_createWithSize(1), true, true);
}

void dcArrayClass_markNode(dcNode *_classNode)
{
    dcArray_mark(dcArrayClass_getObjects(_classNode));
}

void dcArrayClass_registerNode(dcNode *_classNode)
{
    dcArray *objects = dcArrayClass_getObjects(_classNode);
    dcContainerSizeType i = 0;

    for (i = 0; i < objects->size; i++)
    {
        dcNode_register(dcArray_get(objects, i));
    }
}

dcNode *dcArrayClass_buildEmptyObject(void)
{
    return dcArrayClass_createNode(NULL, false, true);
}

bool dcArrayClass_isInitialized(const dcNode *_arrayNode)
{
    return CAST_ARRAY_AUX(_arrayNode)->initialized;
}

void dcArrayClass_setInitialized(dcNode *_arrayNode)
{
    CAST_ARRAY_AUX(_arrayNode)->initialized = true;
}

dcArray *dcArrayClass_getObjects(const dcNode *_arrayNode)
{
    return CAST_ARRAY_AUX(_arrayNode)->objects;
}

void dcArrayClass_clearObjects(dcNode *_arrayNode)
{
    CAST_ARRAY_AUX(_arrayNode)->objects = NULL;
}

void dcArrayClass_setObjects(dcNode *_arrayNode,
                             dcArray *_objects,
                             dcDepth _depth)
{
    dcArrayClassAux *aux = CAST_ARRAY_AUX(_arrayNode);
    dcArray_free(&(aux->objects), _depth);
    aux->objects = _objects;
}

void dcArrayClass_fillWithNil(dcArray *_array)
{
    uint32_t i;

    for (i = 0; i < _array->capacity; i++)
    {
        _array->objects[i] = dcNilClass_getInstance();
    }
}

dcNode *dcArrayMetaClass_createWithSize(dcNode *_receiver,
                                        dcArray *_arguments)
{
    uint32_t capacity;
    dcNode *result = NULL;

    if (dcNumberClass_extractInt32u_withException(dcArray_get(_arguments, 0),
                                                  &capacity))
    {
        dcArray *array = dcArray_createWithSize(capacity);
        array->size = capacity;
        dcArrayClass_fillWithNil(array);
        result = dcNode_register(dcArrayClass_createNode(array, true, true));
    }

    return result;
}

static dcNode *objectAtIndex_helper(dcNode *_receiver, dcNode *_index)
{
    dcArray *array = dcArrayClass_getObjects(_receiver);
    dcNode *result = NULL;
    uint32_t arrayIndex = 0;

    if (dcNumberClass_extractIndex(_index, &arrayIndex, array->size))
    {
        result = array->objects[arrayIndex];
    }

    return result;
}

dcNode *dcArrayClass_objectAtIndex(dcNode *_receiver, dcArray *_arguments)
{
    return objectAtIndex_helper(_receiver, dcArray_get(_arguments, 0));
}

dcNode *dcArrayClass_setObjectAtIndex(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *array = CAST_ARRAY_AUX(_receiver)->objects;
    uint32_t arrayIndex = 0;
    dcNode *result = NULL;

    // the objects are stuffed into an array via #operator([]=)
    dcArray *arguments = dcArrayClass_getObjects(dcArray_get(_arguments, 0));
    dcNode *indexNode = dcArray_get(arguments, 0);
    dcNode *addObject =
        dcClass_copyIfTemplateOrAtomic(dcArray_get(arguments, 1));

    if (dcNumberClass_extractIndex(indexNode, &arrayIndex, array->size))
    {
        dcArray_set(array,
                    addObject,
                    dcArray_sanctifyIndex(array, arrayIndex));
        result = _receiver;
    }

    return result;
}

static dcNode *objectAtIndexesHelper(dcNode *_receiver,
                                     dcArray *_arguments,
                                     bool _set)
{
    dcNode *arrayObject = dcArray_get(_arguments, 0);
    dcArray *array = dcArrayClass_getObjects(arrayObject);
    uint32_t i = 0;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *result = objectAtIndex_helper(_receiver,
                                          dcArray_get(array, i));
    i++;
    uint32_t marksPushed = 0;
    uint32_t limit = (_set ? array->size - 2 : array->size);

    while (result != NULL && i < limit)
    {
        result = dcNodeEvaluator_callMethodWithArgument
            (evaluator,
             result,
             dcSystem_getOperatorName(TAFFY_BRACKETS),
             dcArray_get(array, i));
        marksPushed += dcNodeEvaluator_pushMark(evaluator, result);
        i++;
    }

    if (result != NULL && _set)
    {
        dcError_assert(i <= array->size - 2);

        dcList *arguments = dcList_createWithObjects
            (dcNode_register
             (dcArrayClass_createObject
              (dcArray_createWithObjects
               (dcArray_get(array, i),
                dcArray_get(array, i + 1),
                NULL), // dcArray_createWithObjects
               true)), // dcArrayClass_createObject + dcNode_register
             NULL); // dcList_createWithObjects

        result = dcNodeEvaluator_callMethodWithArguments
            (evaluator,
             result,
             dcSystem_getOperatorName(TAFFY_BRACKETS_EQUAL),
             arguments,
             false);

        dcList_free(&arguments, DC_SHALLOW);
    }

    dcNodeEvaluator_popMarks(evaluator, marksPushed);
    return result;
}

dcNode *dcArrayClass_setObjectAtIndexes(dcNode *_receiver, dcArray *_arguments)
{
    return objectAtIndexesHelper(_receiver, _arguments, true);
}

dcNode *dcArrayClass_objectAtIndexes(dcNode *_receiver, dcArray *_arguments)
{
    return objectAtIndexesHelper(_receiver, _arguments, false);
}

dcNode *dcArrayClass_isEmpty(dcNode *_receiver, dcArray *_arguments)
{
    return (dcArrayClass_getObjects(_receiver)->size == 0
            ? dcYesClass_getInstance()
            : dcNoClass_getInstance());
}

dcNode *dcArrayClass_insertObjectAtIndex(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *array = CAST_ARRAY_AUX(_receiver)->objects;
    uint32_t arrayIndex = 0;
    dcNode *result = NULL;
    dcNode *toAdd = dcArray_get(_arguments, 0);

    if (dcNumberClass_extractIndex(_arguments->objects[1],
                                   &arrayIndex,
                                   array->size))
    {
        result = dcClass_copyIfTemplateOrAtomic(toAdd);
        dcArray_unshiftAtIndex(array, result, arrayIndex);
    }

    return result;
}

dcNode *dcArrayClass_clear(dcNode *_receiver, dcArray *_arguments)
{
    dcArray_clear(CAST_ARRAY_AUX(_receiver)->objects, DC_SHALLOW);
    return _receiver;
}

dcNode *dcArrayClass_size(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *integer = dcNumberClass_createObjectFromInt32s
        (CAST_ARRAY_AUX(_receiver)->objects->size);
    dcNode_register(integer);
    return integer;
}

dcNode *dcArrayClass_subArrayFromTo(dcNode *_receiver, dcArray *_arguments)
{
    uint32_t from = 0;
    uint32_t to = 0;
    dcArray *array = dcArrayClass_getObjects(_receiver);
    dcNode *result = NULL;

    if (dcNumberClass_extractIndex(dcArray_get(_arguments, 0),
                                   &from,
                                   array->size)
        && dcNumberClass_extractIndex(dcArray_get(_arguments, 1),
                                      &to,
                                      array->size))
    {
        if (to < from)
        {
            dcIndexOutOfBoundsExceptionClass_throwObject(to);
        }
        else
        {
            dcArray *newArray = dcArray_createWithSize((to - from) + 1);
            uint32_t j = 0;
            uint32_t i;

            for (i = from; i <= to; i++, j++)
            {
                newArray->objects[j] = array->objects[i];
            }

            newArray->size = newArray->capacity;
            result = dcNode_register(dcArrayClass_createObject(newArray, true));
        }
    }

    return result;
}

typedef dcNode *(*LoopAction)(dcNode *_blockResult,
                              dcNode *_argument,
                              bool *_stop,
                              void *_data);

static dcNode *doBreakthroughLoop(dcNode *_receiver,
                                  dcArray *_arguments,
                                  LoopAction _action,
                                  void *_data,
                                  dcNode *_defaultResult)
{
    uint32_t argumentCount = 0;
    uint32_t i = 0;
    dcNode *procedureNode = dcArray_get(_arguments, 0);
    dcMethodHeader *methodHeader =
        dcProcedureClass_getMethodHeader(procedureNode);
    dcList *blockArguments = dcMethodHeader_getArguments(methodHeader);
    dcNode *result = _defaultResult;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcArray *array = CAST_ARRAY_AUX(_receiver)->objects;
    argumentCount = blockArguments->size;

    if (argumentCount == 1)
    {
        dcList *callArguments = dcList_create();
        dcContainerClass_startLoop(_receiver);
        dcNodeEvaluator_startLoop(evaluator);

        for (i = 0; i < array->size; i++)
        {
            if (dcContainerClass_checkModified(_receiver))
            {
                result = NULL;
                break;
            }

            dcNode *argument = dcArray_get(array, i);
            dcList_setHead(callArguments, argument);
            dcNode *blockResult = dcNodeEvaluator_evaluateProcedure
                (evaluator,
                 NULL,
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
                result = _action(blockResult, argument, &stop, _data);

                if (stop)
                {
                    break;
                }
            }
            else if (! dcNodeEvaluator_canContinueEvaluating(evaluator))
            {
                break;
            }
        }

        dcContainerClass_stopLoop(_receiver);
        dcNodeEvaluator_stopLoop(evaluator);
        dcNodeEvaluator_setBreaking(evaluator, false);
        dcList_free(&callArguments, DC_SHALLOW);
    }
    else
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject(1, argumentCount);
    }

    return result;
}

dcNode *dcArrayClass_each(dcNode *_receiver, dcArray *_arguments)
{
    return doBreakthroughLoop(_receiver,
                              _arguments,
                              NULL,
                              NULL,
                              dcNilClass_getInstance());
}

dcNode *dcArrayClass_operatorEquals(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();
    dcArray *left = dcArrayClass_getObjects(_receiver);
    dcNode *rightObject = dcArray_get(_arguments, 0);
    dcClass_lock(rightObject);
    dcNode *casted = dcClass_castNode(rightObject,
                                      sTemplate,
                                      false); // don't throw a fail exception

    if (casted == NULL)
    {
        result = dcNoClass_getInstance();
    }
    else
    {
        dcArray *right = dcArrayClass_getObjects(casted);

        if (left->size == right->size)
        {
            size_t i;
            dcNode *yiss = dcYesClass_getInstance();
            dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
            result = yiss;

            for (i = 0; i < left->size; i++)
            {
                if (dcNodeEvaluator_callMethodWithArgument
                    (evaluator,
                     left->objects[i],
                     dcSystem_getOperatorName(TAFFY_EQUALS),
                     right->objects[i])
                    != yiss)
                {
                    result = dcNoClass_getInstance();
                    break;
                }
            }
        }
    }

    dcClass_unlock(rightObject);
    return result;
}

dcNode *dcArrayClass_eachIndex(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcYesClass_getInstance();
    dcNode *procedureNode = dcArray_get(_arguments, 0);
    dcMethodHeader *methodHeader =
        dcProcedureClass_getMethodHeader(procedureNode);
    dcList *blockArguments = dcMethodHeader_getArguments(methodHeader);
    dcArray *array = CAST_ARRAY_AUX(_receiver)->objects;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    const uint32_t blockArgumentsSize = blockArguments->size;

    if (blockArgumentsSize == 1)
    {
        uint32_t marksPushed = 0;
        dcContainerSizeType i = 0;
        dcList *callArguments = dcList_create();
        bool breakout = false;
        dcContainerClass_startLoop(_receiver);
        dcNodeEvaluator_startLoop(evaluator);

        for (i = 0; i < array->size && ! breakout; i++)
        {
            dcNode *indexNode =
                dcNode_register(dcNumberClass_createObjectFromInt32s(i));
            marksPushed += dcNodeEvaluator_pushMark(evaluator, indexNode);

            dcNode_setTemplate(indexNode, false);
            dcList_setHead(callArguments, indexNode);
            dcNode *blockResult = (dcNodeEvaluator_evaluateProcedure
                                   (evaluator,
                                    NULL,
                                    procedureNode,
                                    SCOPE_DATA_BREAKTHROUGH,
                                    callArguments));

            if (blockResult == NULL
                || dcContainerClass_checkModified(_receiver))
            {
                // exception is already set, get out of here
                result = NULL;
                breakout = true;
            }
            else if (! dcNodeEvaluator_canContinueEvaluating(evaluator))
            {
                breakout = true;
            }
        }

        dcNodeEvaluator_popMarks(evaluator, marksPushed);
        dcNodeEvaluator_setBreaking(evaluator, false);
        dcContainerClass_stopLoop(_receiver);
        dcNodeEvaluator_stopLoop(evaluator);
        dcList_free(&callArguments, DC_SHALLOW);
    }
    else
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject
            (1, blockArgumentsSize);
    }

    return result;
}

static dcNode *selectAction(dcNode *_blockResult,
                            dcNode *_argument,
                            bool *_stop,
                            void *_data)
{
    dcArray *selected = (dcArray*)_data;

    if (_blockResult != dcNoClass_getInstance())
    {
        dcArray_add(selected, _argument);
    }

    return dcYesClass_getInstance();
}

static dcArray *selectHelper(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *selected =
        dcArray_createWithSize(dcArrayClass_getObjects(_receiver)->size);

    if (doBreakthroughLoop(_receiver,
                           _arguments,
                           &selectAction,
                           selected,
                           dcYesClass_getInstance())
        == NULL)
    {
        dcArray_free(&selected, DC_SHALLOW);
        selected = NULL;
    }

    return selected;
}

dcNode *dcArrayClass_select(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *selected = selectHelper(_receiver, _arguments);
    return (selected == NULL
            ? NULL
            : dcNode_register(dcArrayClass_createObject(selected, true)));
}

dcNode *dcArrayClass_selectBang(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcArray *collected = selectHelper(_receiver, _arguments);

    if (collected != NULL)
    {
        dcArray_free(&(CAST_ARRAY_AUX(_receiver)->objects), DC_SHALLOW);
        CAST_ARRAY_AUX(_receiver)->objects = collected;
        result = _receiver;
    }

    return result;
}

static dcNode *collectAction(dcNode *_blockResult,
                             dcNode *_argument,
                             bool *_stop,
                             void *_data)
{
    dcArray *collected = (dcArray*)_data;
    dcArray_add(collected, _blockResult);
    return dcYesClass_getInstance();
}

static dcArray *collectHelper(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *collected =
        dcArray_createWithSize(dcArrayClass_getObjects(_receiver)->size);

    if (doBreakthroughLoop(_receiver,
                           _arguments,
                           &collectAction,
                           collected,
                           dcYesClass_getInstance())
        == NULL)
    {
        dcArray_free(&collected, DC_SHALLOW);
    }

    return collected;
}

dcNode *dcArrayClass_collectBang(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcArray *collected = collectHelper(_receiver, _arguments);

    if (collected != NULL)
    {
        dcArray_free(&(CAST_ARRAY_AUX(_receiver)->objects), DC_SHALLOW);
        CAST_ARRAY_AUX(_receiver)->objects = collected;
        result = _receiver;
    }

    return result;
}

dcNode *dcArrayClass_collect(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *collected = collectHelper(_receiver, _arguments);

    return (collected == NULL
            ? NULL
            : dcNode_register(dcArrayClass_createObject(collected, true)));
}

dcNode *dcArrayClass_arrayOperation_helper(dcArray *_array1,
                                           dcArray *_array2,
                                           dcNode *_container1,
                                           dcNode *_container2,
                                           const char *_operation,
                                           dcNode *_extra)
{
    dcNode *result = dcNoClass_getInstance();
    uint32_t i = 0;
    bool exception = false;
    bool matched = true;

    if (_array1->size == _array2->size)
    {
        dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
        uint32_t marksPushed = 0;
        dcContainerClass_startLoop(_container1);
        dcContainerClass_startLoop(_container2);

        for (i = 0; i < _array1->size; i++)
        {
            dcNode *node1 = dcArray_get(_array1, i);
            dcNode *node2 = dcArray_get(_array2, i);
            dcList *arguments = NULL;

            if (_extra != NULL)
            {
                dcNode *arrayObject =
                    dcArrayClass_createObject
                    (dcArray_createWithObjects(node2, _extra, NULL), true);
                dcNode_register(arrayObject);
                marksPushed += dcNodeEvaluator_pushMark(evaluator, arrayObject);
                arguments = dcList_createWithObjects(arrayObject, NULL);
            }
            else
            {
                arguments = dcList_createWithObjects(node2, NULL);
            }

            dcNode *evaluateResult =
                dcNodeEvaluator_callMethodWithArguments
                (evaluator, node1, _operation, arguments, false);
            dcList_free(&arguments, DC_SHALLOW);
            marksPushed +=
                dcNodeEvaluator_pushMark(evaluator, evaluateResult);

            if (evaluateResult == NULL
                || dcContainerClass_checkModified(_container1)
                || dcContainerClass_checkModified(_container2))
            {
                exception = true;
                break;
            }
            else if (evaluateResult != dcYesClass_getInstance())
            {
                matched = false;
                break;
            }
        }

        dcNodeEvaluator_popMarks(evaluator, marksPushed);
        dcNodeEvaluator_setBreaking(evaluator, false);
        dcContainerClass_stopLoop(_container1);
        dcContainerClass_stopLoop(_container2);

        result = (exception
                  ? NULL
                  : (matched
                     ? dcYesClass_getInstance()
                     : dcNoClass_getInstance()));
    }

    return result;
}

dcNode *dcArrayClass_shift(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *array = dcArrayClass_getObjects(_receiver);
    dcArray_shift(array, 0, DC_SHALLOW);
    dcNode *result = dcContainerClass_setModified(_receiver, true);
    return result;
}

dcNode *dcArrayClass_push(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *array = dcArrayClass_getObjects(_receiver);
    dcNode *pushObject =
        dcClass_copyIfTemplateOrAtomic(dcArray_get(_arguments, 0));
    dcArray_add(array, pushObject);
    return dcContainerClass_setModified(_receiver, true);
}

dcNode *dcArrayClass_concat(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcArray *left = dcArrayClass_getObjects(_receiver);
    dcNode *rightObject =
        dcClass_copyIfTemplateOrAtomic(dcArray_get(_arguments, 0));

    dcArray *right = CAST_ARRAY_AUX(rightObject)->objects;
    dcClass_lock(rightObject);

    dcArray *newArray = dcArray_createWithSize
        (left->size + right->size + 5);

    dcArray_converge(newArray, left, 0);
    dcArray_converge(newArray, right, left->size);
    result = dcContainerClass_setModified(_receiver, true);

    if (result != NULL)
    {
        result = dcArrayClass_createObject(newArray, true);
        dcNode_register(result);
    }

    dcClass_unlock(rightObject);
    return result;
}

dcNode *dcArrayClass_pop(dcNode *_receiver, dcArray *_arguments)
{
    dcArray_pop(dcArrayClass_getObjects(_receiver), DC_SHALLOW);
    return _receiver;
}

static dcNode *rejectAction(dcNode *_blockResult,
                            dcNode *_argument,
                            bool *_stop,
                            void *_data)
{
    dcArray *survivors = (dcArray*)_data;

    if (_blockResult == dcNoClass_getInstance())
    {
        dcArray_add(survivors, _argument);
    }

    return dcYesClass_getInstance();
}

static dcArray *rejectHelper(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *survivors =
        dcArray_createWithSize(dcArrayClass_getObjects(_receiver)->size);

    if (doBreakthroughLoop(_receiver,
                           _arguments,
                           &rejectAction,
                           survivors,
                           dcYesClass_getInstance())
        == NULL)
    {
        dcArray_free(&survivors, DC_SHALLOW);
        survivors = NULL;
    }

    return survivors;
}

dcNode *dcArrayClass_reject(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *survivors = rejectHelper(_receiver, _arguments);
    return (survivors == NULL
            ? NULL
            : dcNode_register(dcArrayClass_createObject(survivors, true)));
}

dcNode *dcArrayClass_reverse(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *array = dcArrayClass_getObjects(_receiver);
    dcArray *reverseArray = dcArray_createWithSize(array->size);
    uint32_t i = 0;

    for (i = 0; i < array->size; i++)
    {
        dcArray_set(reverseArray,
                    array->objects[i],
                    array->size - i - 1);
    }

    return dcNode_register(dcArrayClass_createObject(reverseArray, true));
}

dcNode *dcArrayClass_reverseBang(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *array = dcArrayClass_getObjects(_receiver);
    size_t i = 0;

    for (i = 0; i < array->size / 2; i++)
    {
        dcNode *temp = array->objects[array->size - i - 1];
        array->objects[array->size - i - 1] = array->objects[i];
        array->objects[i] = temp;
    }

    return dcContainerClass_setModified(_receiver, true);
}

bool dcArrayClass_arrayContentsAsString(dcArray *_objects, dcString *_string)
{
    uint32_t i = 0;
    bool success = true;

    for (i = 0; i < _objects->size; i++)
    {
        const char *objectAsString =
            dcStringClass_asString_helper(dcArray_get(_objects, i));

        if (objectAsString == NULL)
        {
            // exception occurred, and is already set //
            success = false;
            break;
        }

        dcString_appendString(_string, objectAsString);

        if (i < _objects->size - 1)
        {
            dcString_appendCharacter(_string, ',');
        }
    }

    return success;
}

dcNode *dcArrayClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    uint32_t i = 0;
    dcString *displayString = dcString_create();
    dcArray *objects = dcArrayClass_getObjects(_receiver);
    dcString_appendString(displayString, "[");
    dcNode *result = NULL;
    dcResult printResult = TAFFY_SUCCESS;

    for (i = 0;
         i < objects->size && printResult == TAFFY_SUCCESS;
         i++)
    {
        printResult = dcNode_print(dcArray_get(objects, i), displayString);

        if (i < objects->size - 1)
        {
            dcString_appendString(displayString, ", ");
        }
    }

    if (printResult == TAFFY_SUCCESS)
    {
        dcString_appendString(displayString, "]");
        result = dcNode_register
            (dcStringClass_createObject(displayString->string, false));
        dcString_free(&displayString, DC_SHALLOW);
    }
    else
    {
        dcString_free(&displayString, DC_DEEP);
    }

    return result;
}

bool dcArrayClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    bool isInitialized;
    dcArray *objects;

    if (dcMarshaller_unmarshallNoNull(_stream, "ba", &isInitialized, &objects))
    {
        CAST_CLASS_AUX(_node) = createAux(objects, isInitialized);
        result = true;
    }

    return result;
}

dcString *dcArrayClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcClass_lock((dcNode*)_node);
    dcString *result = dcMarshaller_marshall(_stream,
                                             "ba",
                                             dcArrayClass_isInitialized(_node),
                                             dcArrayClass_getObjects(_node));
    dcClass_unlock((dcNode*)_node);
    return result;
}

bool dcArrayClass_verifySizeWithThrow(dcNode *_arrayObject,
                                      dcContainerSizeType _size)
{
    uint32_t mySize = dcArrayClass_getObjects(_arrayObject)->size;
    bool result = true;

    if (mySize != _size)
    {
        result = false;
        dcInvalidArraySizeExceptionClass_throwObject(_size, mySize);
    }

    return result;
}

void dcInvalidArraySizeExceptionClass_throwObject(dcContainerSizeType _expected,
                                                  int32_t _given)
{
    dcExceptions_throwObject
        (dcStringEvaluator_evalFormat
         (ARRAY_TAFFY_FILE_NAME,
          STRING_EVALUATOR_HANDLE_EXCEPTION,
          "org.taffy.core.exception.InvalidArraySizeException "
          "expected: %u given: %d",
          _expected,
          _given));
}

bool dcArrayClass_isMe(const dcNode *_node)
{
    return dcClass_hasTemplate(_node, sTemplate, true);
}

dcResult dcArrayClass_compileHelper(dcNode *_node,
                                    dcList *_symbols,
                                    bool *_changed)
{
    dcArray *objects = dcArrayClass_getObjects(_node);
    dcContainerSizeType i;

    for (i = 0; i < objects->size; i++)
    {
        if (dcFlatArithmetic_compile(&objects->objects[i],
                                     _symbols,
                                     _changed)
            == TAFFY_EXCEPTION)
        {
            return TAFFY_EXCEPTION;
        }
    }

    return TAFFY_SUCCESS;
}
