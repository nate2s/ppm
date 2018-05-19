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

#include <string.h>
#include "dcMemory.h"


#include "dcFunctionRepeater.h"
#include "dcArrayClass.h"
#include "dcClass.h"
#include "dcContainers.h"
#include "dcExceptions.h"
#include "dcGraphData.h"
#include "dcMethodHeader.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcNumberClass.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcSystem.h"

bool dcFunctionRepeater_executeHelper
    (dcNode *_receiver,
     dcNode *_from,
     dcNode *_to,
     dcHash *_memory,
     dcFunctionRepeaterExecuteFunction *_function,
     dcNode **_token)
{
    dcNode *iterator =
        dcNode_register(dcNode_copy(_from, DC_DEEP));
    dcNode *procedure = (dcClass_castNodeWithAssert
                         (_receiver,
                          dcProcedureClass_getTemplate(),
                          false,
                          true));
    dcList *seriesArguments = dcList_create();
    bool result = true;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNodeEvaluator_pushMark(evaluator, iterator);

    if (dcNumber_lessThan(dcNumberClass_getNumber(_to),
                          dcNumberClass_getNumber(iterator)))
    {
        dcNode *arguments =
            dcNode_register
            (dcArrayClass_createObject
             (dcArray_createWithObjects(iterator, _to, NULL), true));
        dcNodeEvaluator_pushMark(evaluator, arguments);
        dcInvalidIndexesExceptionClass_throwObject(arguments);
        dcNodeEvaluator_popMark(evaluator);
        result = false;
    }

    while (result
           && dcNumber_lessThanOrEqual(dcNumberClass_getNumber(iterator),
                                       dcNumberClass_getNumber(_to)))
    {
        dcNode *value = dcNode_register(dcNode_copy(iterator, DC_DEEP));
        dcNodeEvaluator_pushMark(evaluator, value);
        dcList_clear(seriesArguments, DC_SHALLOW);
        dcList_push(seriesArguments, value);
        dcNode *callResult = NULL;

        if (_memory != NULL
            && (dcHash_getValue(_memory, value, &callResult)
                == TAFFY_EXCEPTION))
        {
            result = false;
            break;
        }

        if (callResult == NULL)
        {
            callResult = (dcNodeEvaluator_evaluateProcedure
                          (evaluator,
                           NULL,
                           procedure,
                           (SCOPE_DATA_BREAKTHROUGH
                            | SCOPE_DATA_CONST),
                           seriesArguments));
        }

        if (callResult == NULL)
        {
            // exception occurred
            result = false;
            break;
        }
        else
        {
            if (_memory != NULL
                && (dcHash_getValue(_memory, value, NULL) == TAFFY_FAILURE))
            {
                dcHash_setValue
                    (_memory,
                     dcNode_register(dcNode_copy(value, DC_DEEP)),
                     callResult);
            }

            dcNodeEvaluator_pushMark(evaluator, callResult);

            if (! _function(_receiver, callResult, _token))
            {
                result = false;
                break;
            }

            // /callResult
            dcNodeEvaluator_popMark(evaluator);
        }

        dcNumberClass_increment(iterator);
        // /value
        dcNodeEvaluator_popMark(evaluator);
    }

    dcList_free(&seriesArguments, DC_SHALLOW);
    // /iterator
    dcNodeEvaluator_popMark(evaluator);
    return result;
}

bool dcFunctionRepeater_execute(dcNode *_receiver,
                                dcArray *_arguments,
                                dcHash *_memory,
                                dcFunctionRepeaterExecuteFunction *_function,
                                dcNode **_token)
{
    bool result = false;
    dcNode *arguments = dcArray_get(_arguments, 0);

    if (dcArrayClass_verifySizeWithThrow(arguments, 2))
    {
        dcArray *array = dcArrayClass_getObjects(arguments);
        dcNode *first = dcClass_castNode(dcArray_get(array, 0),
                                         dcNumberClass_getTemplate(),
                                         true);
        if (first != NULL
            && dcNumberClass_verifyInteger(first))
        {
            dcNode *second = dcClass_castNode(dcArray_get(array, 1),
                                              dcNumberClass_getTemplate(),
                                              true);
            if (second != NULL
                && dcNumberClass_verifyInteger(second))
            {
                result = dcFunctionRepeater_executeHelper(_receiver,
                                                          first,
                                                          second,
                                                          _memory,
                                                          _function,
                                                          _token);
            }
            // else exception is already output
        }
        // else exception is already output
    }

    return result;
}
