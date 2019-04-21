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
#include <string.h>

#include "CompiledMatrix.h"

#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcComplexNumberClass.h"
#include "dcContainerClass.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcMarshaller.h"
#include "dcMatrix.h"
#include "dcMatrixClass.h"
#include "dcMemory.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcNumber.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSymbolClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

struct dcMatrixMetaClassAux_t
{
    dcNode **eyes;
    uint32_t eyesSize;
};

typedef struct dcMatrixMetaClassAux_t dcMatrixMetaClassAux;

static dcMatrixMetaClassAux *sMetaAux = NULL;

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcMatrixClass_asString,
        gCFunctionArgument_none
    },
    // columnCount doesn't change so doesn't need to be synchronized
    {
        "columnCount",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcMatrixClass_columnCount,
        gCFunctionArgument_none
    },
    {
        "#operator(~=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcMatrixClass_deltaEqual,
        gCFunctionArgument_array
    },
    {
        "#operator(==):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcMatrixClass_equals,
        gCFunctionArgument_wild
    },
    {
        "mathOperation:object:",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcMatrixClass_mathOperationObject,
        gCFunctionArgument_stringWild
    },
    {
        "#operator(*):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcMatrixClass_multiply,
        gCFunctionArgument_wild
    },
    {
        "#operator([...]):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcMatrixClass_objectAtIndexes,
        gCFunctionArgument_array
    },
    {
        "#operator([...]=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE
         | SCOPE_DATA_CONST),
        &dcMatrixClass_setObjectAtIndexes,
        gCFunctionArgument_array
    },
    {
        "subMatrixFromRow:column:toRow:column:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcMatrixClass_subMatrixFromRowColumnToRowColumn,
        gCFunctionArgument_numberNumberNumberNumber
    },
    {
        "objectAtIndexes:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcMatrixClass_objectAtIndexes,
        gCFunctionArgument_array
    },
    // rowCount doesn't change so doesn't need to be synchronized
    {
        "rowCount",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcMatrixClass_rowCount,
        gCFunctionArgument_none
    },
    {
        "transpose",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcMatrixClass_transpose,
        gCFunctionArgument_none
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "createWithRows:columns:",
        SCOPE_DATA_PUBLIC,
        &dcMatrixMetaClass_createWithRowsColumns,
        gCFunctionArgument_numberNumber
    },
    {
        "eye:",
        SCOPE_DATA_PUBLIC,
        &dcMatrixMetaClass_eye,
        gCFunctionArgument_number
    },
    {
        0
    }
};

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcMatrixClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (MATRIX_PACKAGE_NAME,                    // package name
          MATRIX_CLASS_NAME,                      // class name
          MAKE_FULLY_QUALIFIED(CONTAINER),        // super name
          CLASS_HAS_READ_WRITE_LOCK,              // class flags
          NO_FLAGS,                               // scope data flags
          sMetaMethodWrappers,                    // meta methods
          sMethodWrappers,                        // methods
          &dcMatrixClass_initialize,              // initialization function
          &dcMatrixClass_deinitialize,            // deinitialization function
          &dcMatrixClass_allocateNode,            // allocate
          &dcMatrixClass_deallocateNode,          // deallocate
          &dcMatrixClass_markMetaNode,            // meta mark
          &dcMatrixClass_markNode,                // mark
          &dcMatrixClass_copyNode,                // copy
          &dcMatrixClass_freeNode,                // free
          &dcMatrixClass_registerNode,            // register
          &dcMatrixClass_marshallNode,            // marshall
          &dcMatrixClass_unmarshallNode,          // unmarshall
          &dcMatrixClass_setTemplate));           // set template
};

static dcMatrixClassAux *createAux(dcMatrix *_matrix, bool _initialized)
{
    dcMatrixClassAux *aux =
        (dcMatrixClassAux *)dcMemory_allocate(sizeof(dcMatrixClassAux));
    aux->matrix = _matrix;
    aux->initialized = _initialized;
    return aux;
}

void dcMatrixClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux(dcMatrix_create(dcArray_createWithObjects
                                                      (dcNilClass_getInstance(),
                                                       dcNilClass_getInstance(),
                                                       NULL),
                                                      1, 1),
                                      true);
}

void dcMatrixClass_deallocateNode(dcNode *_node)
{
    dcMatrix_clear(dcMatrixClass_getMatrix(_node), DC_SHALLOW);
}

#define MATRIX_CLASS_TAFFY_FILE_NAME "src/class/Matrix.ty"

void dcMatrixClass_initialize(void)
{
    assert(sMetaAux == NULL);
    sMetaAux = (dcMatrixMetaClassAux *)(dcMemory_allocate
                                        (sizeof(dcMatrixMetaClassAux)));
    sMetaAux->eyesSize = 10;
    sMetaAux->eyes =
        (dcNode **)(dcMemory_allocateAndInitialize
                    (sizeof(dcNode*) * sMetaAux->eyesSize));
    dcError_assert(dcStringEvaluator_evalString(__compiledMatrix,
                                                MATRIX_CLASS_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

void dcMatrixClass_deinitialize(void)
{
    dcMatrixMetaClass_dealloc();
}

void dcMatrixMetaClass_dealloc(void)
{
    if (sMetaAux != NULL)
    {
        dcMemory_free(sMetaAux->eyes);
        dcMemory_free(sMetaAux);
    }
}

void dcMatrixClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcMatrixClassAux *aux = CAST_MATRIX_AUX(_node);
    TAFFY_DEBUG(dcError_assert((dcNode_isRegistered(_node)
                                && aux->matrix->objects->size == 0)
                               || ! dcNode_isRegistered(_node)));
    dcMatrix_free(&aux->matrix, DC_DEEP);
    dcMemory_free(aux);
}

void dcMatrixClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_CLASS_AUX(_to) = createAux
        (dcMatrix_copy(CAST_MATRIX_AUX(_from)->matrix, _depth),
         CAST_MATRIX_AUX(_from)->initialized);
}

dcNode *dcMatrixClass_createNode(dcMatrix *_matrix,
                                 bool _initialized,
                                 bool _object)
{
    return (dcClass_createNode
            (sTemplate,                            // template
             dcContainerClass_createNode(_object), // super
             NULL,                                 // scope
             _object,                              // object ?
             createAux(_matrix, _initialized)));   // the aux
}

dcNode *dcMatrixClass_createObject(dcMatrix *_matrix, bool _initialized)
{
    return dcMatrixClass_createNode(_matrix, _initialized, true);
}

bool dcMatrixClass_isInitialized(const dcNode *_node)
{
    return CAST_MATRIX_AUX(_node)->initialized;
}

dcMatrix *dcMatrixClass_getMatrix(const dcNode *_node)
{
    return CAST_MATRIX_AUX(_node)->matrix;
}

void dcMatrixClass_markNode(dcNode *_node)
{
    dcMatrix_mark(dcMatrixClass_getMatrix(_node));
}

void dcMatrixClass_registerNode(dcNode *_node)
{
    dcArray_register(dcMatrixClass_getMatrix(_node)->objects);
}

static dcNode *matrixOperation(dcMatrix *_leftMatrix,
                               dcMatrix *_rightMatrix,
                               const char *_operation)
{
    dcNode *result = NULL;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    if (_leftMatrix->columnCount == _rightMatrix->columnCount
        && _leftMatrix->rowCount == _rightMatrix->rowCount)
    {
        dcArray *leftObjects = _leftMatrix->objects;
        dcArray *rightObjects = _rightMatrix->objects;
        uint32_t it = 0;
        dcArray *objects = dcArray_createWithSize(leftObjects->size);
        uint32_t markCount = 0;

        for (it = 0; it < leftObjects->size; it++)
        {
            dcNode *operationResult =
                dcNodeEvaluator_callMethodWithArgument
                (evaluator,
                 dcArray_get(leftObjects, it),
                 _operation,
                 dcArray_get(rightObjects, it));

            if (operationResult != NULL)
            {
                markCount +=
                    dcNodeEvaluator_pushMark(evaluator, operationResult);
                dcArray_add(objects, operationResult);
            }
            else
            {
                // exception //
                dcArray_free(&objects, DC_SHALLOW);
                objects = NULL;
                break;
            }
        }

        dcNodeEvaluator_popMarks(evaluator, markCount);

        if (objects != NULL)
        {
            result = dcNode_register
                (dcMatrixClass_createObject
                 (dcMatrix_create
                  (objects,
                   _leftMatrix->rowCount,
                   _leftMatrix->columnCount),
                  true));
        }
    }
    else
    {
        dcInvalidMatrixSizeExceptionClass_throwObject
            (_leftMatrix->rowCount,
             _leftMatrix->columnCount,
             _rightMatrix->rowCount,
             _rightMatrix->columnCount);
    }

    return result;
}

static dcNode *matrixAndNumberOperation(dcNode *_leftMatrixNode,
                                        dcNode *_rightNumber,
                                        const char *_operation,
                                        bool _left)
{
    uint32_t i = 0;
    uint32_t markCount = 0;
    dcNode *result = NULL;
    dcMatrix *leftMatrix = dcMatrixClass_getMatrix(_leftMatrixNode);
    dcArray *leftObjects = leftMatrix->objects;
    dcArray *addedObjects = dcArray_createWithSize(leftObjects->size);
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    for (i = 0; i < leftObjects->size; i++)
    {
        dcNode *leftObject = dcArray_get(leftObjects, i);
        dcNode *realLeft = (_left
                            ? leftObject
                            : _rightNumber);
        dcNode *realRight = (_left
                             ? _rightNumber
                             : leftObject);
        dcNode *operationResult =
            dcNodeEvaluator_callMethodWithArgument
            (evaluator, realLeft, _operation, realRight);

        if (operationResult != NULL)
        {
            markCount += dcNodeEvaluator_pushMark(evaluator, operationResult);
            dcArray_add(addedObjects, operationResult);
        }
        else
        {
            // exception //
            dcArray_free(&addedObjects, DC_SHALLOW);
            addedObjects = NULL;
            break;
        }
    }

    if (addedObjects != NULL)
    {
        result = dcNode_register(dcMatrixClass_createObject
                                 (dcMatrix_create(addedObjects,
                                                  leftMatrix->rowCount,
                                                  leftMatrix->columnCount),
                                  true));
    }

    dcNodeEvaluator_popMarks(evaluator, markCount);
    return result;
}

dcNode *dcMatrixClass_matrixAndNumberOperation(dcNode *_leftMatrixNode,
                                               dcNode *_rightNumber,
                                               dcTaffyOperator _operation,
                                               bool _left)
{
    return matrixAndNumberOperation(_leftMatrixNode,
                                    _rightNumber,
                                    dcSystem_getOperatorName(_operation),
                                    _left);
}

//
// called via Taffy code
//
dcNode *dcMatrixClass_mathOperationObject(dcNode *_receiver,
                                          dcArray *_arguments)
{
    dcNode *result = NULL;
    const char *operation =
        dcStringClass_getString(dcArray_get(_arguments, 0));
    dcNode *candidate = dcArray_get(_arguments, 1);
    dcNode *right = dcClass_castNode(candidate, sTemplate, false);

    if (right != NULL)
    {
        result = matrixOperation
            (dcMatrixClass_getMatrix(_receiver),
             dcMatrixClass_getMatrix(right),
             operation);
    }
    else
    {
        right = dcClass_castNode(candidate,
                                 dcNumberClass_getTemplate(),
                                 false);

        if (right != NULL)
        {
            result = matrixAndNumberOperation
                (_receiver, right, operation, true);
        }
        else
        {
            dcInvalidCastExceptionClass_throwObject(dcClass_getName(candidate),
                                                    "(Matrix or Number)");
        }
    }

    return result;
}

dcNode *dcMatrixClass_deltaEqual(dcNode *_receiver, dcArray *_arguments)
{
    dcMatrix *matrix1 = dcMatrixClass_getMatrix(_receiver);
    dcArray *array1 = matrix1->objects;

    // unstuff the delta-equal objects
    dcArray *realArguments =
        dcArrayClass_getObjects(dcArray_get(_arguments, 0));
    dcNode *matrixCandidate = dcArray_get(realArguments, 0);
    dcNode *delta = dcArray_get(realArguments, 1);

    dcNode *result = dcNoClass_getInstance();
    matrixCandidate = dcClass_castNode
        (matrixCandidate, sTemplate, false);

    if (matrixCandidate != NULL)
    {
        dcMatrix *matrix2 = dcMatrixClass_getMatrix(matrixCandidate);

        if (matrix1->columnCount == matrix2->columnCount
            && matrix1->rowCount == matrix2->rowCount)
        {
            dcArray *array2 = matrix2->objects;

            result = dcArrayClass_arrayOperation_helper
                (array1,
                 array2,
                 _receiver,
                 matrixCandidate,
                 dcSystem_getOperatorName(TAFFY_DELTA_EQUALS),
                 delta);
        }
    }

    return result;
}

dcNode *dcMatrixClass_equals(dcNode *_receiver, dcArray *_arguments)
{
    dcMatrix *matrix1 = dcMatrixClass_getMatrix(_receiver);
    dcArray *array1 = matrix1->objects;
    dcNode *matrixCandidate = dcArray_get(_arguments, 0);
    dcNode *result = dcNoClass_getInstance();

    matrixCandidate =
        dcClass_castNode(matrixCandidate, sTemplate, false);

    if (matrixCandidate != NULL)
    {
        dcMatrix *matrix2 = dcMatrixClass_getMatrix(matrixCandidate);

        if (matrix1->columnCount == matrix2->columnCount
            && matrix1->rowCount == matrix2->rowCount)
        {
            result = dcArrayClass_arrayOperation_helper
                (array1,
                 matrix2->objects,
                 _receiver,
                 matrixCandidate,
                 dcSystem_getOperatorName(TAFFY_EQUALS),
                 NULL);
        }
    }

    return result;
}

dcResult dcMatrixClass_printNode(const dcNode *_node, dcString *_string)
{
    uint32_t i = 0;
    dcMatrix *matrix = dcMatrixClass_getMatrix(_node);
    const dcArray *objects = matrix->objects;
    dcString_appendString(_string, "#Matrix||");
    dcResult result = TAFFY_SUCCESS;

    for (i = 0; i < objects->size && result == TAFFY_SUCCESS; i++)
    {
        result = dcNode_print(dcArray_get(objects, i), _string);

        if (result == TAFFY_SUCCESS)
        {
            if (i < objects->size - 1)
            {
                if ((i + 1) % matrix->columnCount == 0)
                {
                    dcString_appendString(_string, " ; ");
                }
                else
                {
                    dcString_appendCharacter(_string, ',');
                }
            }
        }
    }

    dcString_appendString(_string, "||");
    return result;
}

dcNode *dcMatrixClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    dcMatrixClassAux *aux = CAST_MATRIX_AUX(_receiver);
    dcMatrix *matrix = aux->matrix;
    dcArray *objects = matrix->objects;
    dcString *displayString = dcString_create();
    dcString_appendString(displayString, "||");
    dcNode *result = NULL;
    bool exception = false;
    uint32_t i = 0;

    for (i = 0; i < objects->size; i++)
    {
        if (dcNode_print(objects->objects[i], displayString) == TAFFY_EXCEPTION)
        {
            exception = true;
            break;
        }

        if (i < objects->size - 1)
        {
            if (((i + 1) % matrix->columnCount) == 0)
            {
                dcString_appendString(displayString, " ; ");
            }
            else
            {
                dcString_appendString(displayString, ", ");
            }
        }
    }

    if (! exception)
    {
        dcString_appendString(displayString, "||");
        result = (dcNode_register
                  (dcStringClass_createObject(displayString->string, false)));
        dcString_free(&displayString, DC_SHALLOW);
    }

    return result;
}

void dcMatrixClass_markMetaNode(dcNode *_node)
{
    if (sMetaAux != NULL)
    {
        uint32_t i;

        for (i = 0; i < sMetaAux->eyesSize; i++)
        {
            dcNode_mark(sMetaAux->eyes[i]);
        }
    }
}

dcNode *dcMatrixMetaClass_createWithRowsColumns(dcNode *_receiver,
                                                dcArray *_arguments)
{
    dcNode *result = NULL;
    uint32_t rows = 0;
    uint32_t columns = 0;

    if (dcNumberClass_extractInt32u_withException(dcArray_get(_arguments, 0),
                                                  &rows)
        && dcNumberClass_extractInt32u_withException(dcArray_get(_arguments, 1),
                                                     &columns))
    {
        dcMatrix *matrix = dcMatrix_createBlank(rows, columns);
        uint32_t i;

        for (i = 0; i < matrix->rowCount * matrix->columnCount; i++)
        {
            matrix->objects->objects[i] = dcNilClass_getInstance();
        }

        matrix->objects->size = matrix->objects->capacity;
        result = dcNode_register(dcMatrixClass_createObject(matrix, true));
    }

    return result;
}

dcNode *dcMatrixMetaClass_eye(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *numberObject = dcArray_get(_arguments, 0);
    int32_t number;
    dcNode *result = NULL;

    if (dcNumberClass_extractInt32s_withException(numberObject, &number))
    {
        if ((uint32_t)number < sMetaAux->eyesSize)
        {
            if (sMetaAux->eyes[number] != NULL)
            {
                result = sMetaAux->eyes[number];
            }
            else
            {
                sMetaAux->eyes[number] =
                    dcNode_register(dcMatrixClass_createObject
                                    (dcMatrix_createEye(number, number),
                                     true));

                result = sMetaAux->eyes[number];
            }
        }
        else
        {
            result =
                dcNode_register
                (dcMatrixClass_createObject
                 (dcMatrix_createEye(number, number),
                  true));
        }
    }

    return result;
}

dcNode *dcMatrixClass_rowCount(dcNode *_receiver, dcArray *_arguments)
{
    dcMatrix *matrix = dcMatrixClass_getMatrix(_receiver);
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32s(matrix->rowCount)));
}

dcNode *dcMatrixClass_columnCount(dcNode *_receiver, dcArray *_arguments)
{
    dcMatrix *matrix = dcMatrixClass_getMatrix(_receiver);
    return (dcNode_register
            (dcNumberClass_createObjectFromInt32s(matrix->columnCount)));
}

TAFFY_HIDDEN dcNode *multiplyMatrices(dcMatrix *_leftMatrix,
                                      dcMatrix *_rightMatrix)
{
    dcNode *retval = NULL;
    bool exception = false;
    dcMatrix *matrix = NULL;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    if (_leftMatrix->columnCount == _rightMatrix->rowCount)
    {
        matrix = dcMatrix_createBlank(_leftMatrix->rowCount,
                                      _rightMatrix->columnCount);
        uint32_t rightColumnIt = 0;
        uint32_t markCount = 0;

        for (rightColumnIt = 0;
             rightColumnIt < _rightMatrix->columnCount;
             rightColumnIt++)
        {
            uint32_t leftRowIt = 0;
            uint32_t leftColumnIt = 0;

            for (leftRowIt = 0; leftRowIt < _leftMatrix->rowCount; leftRowIt++)
            {
                dcNode *result = NULL;

                for (leftColumnIt = 0;
                     leftColumnIt < _leftMatrix->columnCount;
                     leftColumnIt++)
                {
                    dcNode *multiplyResult =
                        dcNodeEvaluator_callMethodWithArgument
                        (evaluator,
                         dcMatrix_get(_leftMatrix, leftRowIt, leftColumnIt),
                         dcSystem_getOperatorName(TAFFY_MULTIPLY),
                         dcMatrix_get(_rightMatrix,
                                      leftColumnIt,
                                      rightColumnIt));

                    if (multiplyResult == NULL)
                    {
                        exception = true;
                        break;
                    }

                    markCount +=
                        dcNodeEvaluator_pushMark(evaluator, multiplyResult);

                    if (result == NULL)
                    {
                        result = multiplyResult;
                    }
                    else
                    {
                        result = dcNodeEvaluator_callMethodWithArgument
                            (evaluator,
                             result,
                             dcSystem_getOperatorName(TAFFY_ADD),
                             multiplyResult);
                    }

                    if (result == NULL)
                    {
                        exception = true;
                        break;
                    }

                    markCount += dcNodeEvaluator_pushMark(evaluator, result);
                }

                if (exception)
                {
                    break;
                }

                dcMatrix_set(matrix, result, leftRowIt, rightColumnIt);
            }
        }

        dcNodeEvaluator_popMarks(evaluator, markCount);
    }

    if (exception || matrix == NULL)
    {
        if (matrix != NULL)
        {
            dcMatrix_free(&matrix, DC_SHALLOW);
        }

        dcInvalidMatrixSizeExceptionClass_throwObject
            (_leftMatrix->rowCount,
             0,
             _rightMatrix->rowCount,
             _rightMatrix->columnCount);
    }
    else
    {
        retval = dcNode_register(dcMatrixClass_createObject(matrix, true));
    }

    return retval;
}

dcNode *dcMatrixClass_multiply(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcNode *candidate = dcArray_get(_arguments, 0);
    dcMatrix *leftMatrix = dcMatrixClass_getMatrix(_receiver);
    dcNode *right = dcClass_castNode(candidate, sTemplate, false);

    if (right != NULL)
    {
        result = multiplyMatrices(leftMatrix, dcMatrixClass_getMatrix(right));
    }
    else
    {
        if (((right = dcClass_castNode(candidate,
                                      dcNumberClass_getTemplate(),
                                      false))
             != NULL)
            || ((right = dcClass_castNode(candidate,
                                          dcComplexNumberClass_getTemplate(),
                                          false))
                != NULL))
        {
            result = dcMatrixClass_matrixAndNumberOperation
                (_receiver, right, TAFFY_MULTIPLY, false);
        }
        else
        {
            dcInvalidCastExceptionClass_throwObject(dcClass_getName(candidate),
                                                    "(Matrix or Number)");
        }
    }

    return result;
}

dcNode *dcMatrixClass_objectAtIndexes(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *arrayObject = dcArray_get(_arguments, 0);
    dcArray *objects = dcArrayClass_getObjects(arrayObject);
    uint32_t size = objects->size;
    dcNode *result = NULL;

    if (size == 2)
    {
        dcMatrix *matrix = dcMatrixClass_getMatrix(_receiver);
        dcNode *rowNode = dcArray_get(objects, 0);
        dcNode *columnNode = dcArray_get(objects, 1);
        uint32_t row;
        uint32_t column;

        if (dcNumberClass_extractIndex(rowNode,
                                       &row,
                                       matrix->rowCount)
            && dcNumberClass_extractIndex(columnNode,
                                          &column,
                                          matrix->columnCount))
        {
            result = dcMatrix_get(matrix, row, column);
            dcError_assert(result != NULL);
        }
    }
    else
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject(2, size);
    }

    return result;
}

dcNode *dcMatrixClass_transpose(dcNode *_receiver, dcArray *_arguments)
{
    dcMatrix *matrix = (dcMatrix_transpose
                        (dcMatrix_copy(dcMatrixClass_getMatrix(_receiver),
                                       DC_DEEP)));
    return dcNode_register(dcMatrixClass_createObject(matrix, true));
}

bool dcMatrixClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    bool initialized;
    dcMatrix *matrix;

    if (dcMarshaller_unmarshallNoNull(_stream, "bm", &initialized, &matrix))
    {
        result = true;
        CAST_CLASS_AUX(_node) = createAux(matrix, initialized);
    }

    return result;
}

dcString *dcMatrixClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcMatrixClassAux *aux = CAST_MATRIX_AUX(_node);
    return dcMarshaller_marshall(_stream, "bm", aux->initialized, aux->matrix);
}

void dcMatrixClass_setTemplate(dcNode *_node, bool _yesNo)
{
    if (CAST_MATRIX_AUX(_node) != NULL)
    {
        dcMatrix_setTemplate(dcMatrixClass_getMatrix(_node), _yesNo);
    }
}

bool dcMatrixClass_verifyMatrixDimensions_withException
    (dcNode *_matrix,
     uint32_t _wantedRowCount,
     uint32_t _wantedColumnCount)
{
    dcMatrix *matrix = dcMatrixClass_getMatrix(_matrix);
    bool result = false;

    if (matrix->rowCount == _wantedRowCount
        && matrix->columnCount == _wantedColumnCount)
    {
        result = true;
    }
    else
    {
        result = false;
        dcInvalidMatrixSizeExceptionClass_throwObject(_wantedRowCount,
                                                      _wantedColumnCount,
                                                      matrix->rowCount,
                                                      matrix->columnCount);
    }

    return result;
}

bool dcMatrixClass_extractDoubles_withException(dcNode *_matrix,
                                                double *_output,
                                                uint32_t _rows,
                                                uint32_t _columns)
{
    bool result = true;

    if (dcMatrixClass_verifyMatrixDimensions_withException(_matrix,
                                                           _rows,
                                                           _columns))
    {
        dcMatrix *matrix = dcMatrixClass_getMatrix(_matrix);
        uint32_t i;
        uint32_t size = _rows * _columns;

        for (i = 0; i < size; i++)
        {
            if (! dcNumberClass_extractDouble(dcArray_get(matrix->objects,
                                                          i),
                                              &_output[i]))
            {
                result = false;
                break;
            }
        }
    }
    else
    {
        result = false;
    }

    return result;
}

//
// #operator([...]=):
//
dcNode *dcMatrixClass_setObjectAtIndexes(dcNode *_receiver, dcArray *_arguments)
{
    dcMatrix *matrix = dcMatrixClass_getMatrix(_receiver);
    uint32_t row = 0;
    uint32_t column = 0;
    dcNode *result = NULL;
    dcArray *realArguments = dcArrayClass_getObjects
        (dcArray_get(_arguments, 0));

    if (realArguments->size != 3)
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject(3, _arguments->size);
        goto kickout;
    }

    if (! (dcNumberClass_extractIndex(dcArray_get(realArguments, 0),
                                      &row,
                                      matrix->rowCount)
           && dcNumberClass_extractIndex(dcArray_get(realArguments, 1),
                                         &column,
                                         matrix->columnCount)))
    {
        goto kickout;
    }

    dcMatrix_set(matrix, dcArray_get(realArguments, 2), row, column);
    result = _receiver;

kickout:
    return result;
}

dcNode *dcMatrixClass_subMatrixFromRowColumnToRowColumn(dcNode *_receiver,
                                                        dcArray *_arguments)
{
    uint32_t fromRow;
    uint32_t fromColumn;
    uint32_t toRow;
    uint32_t toColumn;
    dcMatrix *matrix = dcMatrixClass_getMatrix(_receiver);
    dcMatrix *resultMatrix = NULL;

    if (dcNumberClass_extractIndex(dcArray_get(_arguments, 0),
                                   &fromRow,
                                   matrix->rowCount)
        && dcNumberClass_extractIndex(dcArray_get(_arguments, 1),
                                      &fromColumn,
                                      matrix->columnCount)
        && dcNumberClass_extractIndex(dcArray_get(_arguments, 2),
                                      &toRow,
                                      matrix->rowCount)
        && dcNumberClass_extractIndex(dcArray_get(_arguments, 3),
                                      &toColumn,
                                      matrix->columnCount))
    {
        if (toRow < fromRow)
        {
            dcIndexOutOfBoundsExceptionClass_throwObject(toRow);
        }
        else if (toColumn < fromColumn)
        {
            dcIndexOutOfBoundsExceptionClass_throwObject(toColumn);
        }
        else
        {
            resultMatrix = dcMatrix_createBlank(toRow - fromRow + 1,
                                                toColumn - fromColumn + 1);

            uint32_t r;
            uint32_t c;

            for (r = fromRow; r <= toRow; r++)
            {
                for (c = fromColumn; c <= toColumn; c++)
                {
                    dcMatrix_set(resultMatrix,
                                 dcMatrix_get(matrix, r, c),
                                 r - fromRow,
                                 c - fromColumn);
                }
            }
        }
    }

    dcNode *result = NULL;

    if (resultMatrix != NULL)
    {
        result = (dcNode_register
                  (dcMatrixClass_createObject
                   (resultMatrix,
                    true)));
    }

    return result;
}
