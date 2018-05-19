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

#include "dcArray.h"
#include "dcDoubleVoid.h"
#include "dcError.h"
#include "dcGraphData.h"
#include "dcHash.h"
#include "dcInt32.h"
#include "dcUnsignedInt32.h"
#include "dcList.h"
#include "dcMatrix.h"
#include "dcMethodHeader.h"
#include "dcNode.h"
#include "dcNumber.h"
#include "dcPair.h"
#include "dcScope.h"
#include "dcString.h"
#include "dcVoidContainer.h"

#include <stdarg.h>
#include <string.h>

typedef dcString *(*MarshallFunction)(void *_arguments, dcString *_stream);
typedef void *(*UnmarshallFunction)(dcString *_stream);
typedef void (*FreeFunction)(void **_object, dcDepth _depth);

struct dcMarshallType_t
{
    char type;
    MarshallFunction marshallFunction;
    UnmarshallFunction unmarshallFunction;
    FreeFunction freeFunction;
};

typedef struct dcMarshallType_t dcMarshallType;

static const dcMarshallType sMarshallTypes[] =
{
    {
        'a',
        (MarshallFunction)&dcArray_marshall,
        (UnmarshallFunction)&dcArray_unmarshall,
        (FreeFunction)&dcArray_free
    },
    {
        'h',
        (MarshallFunction)&dcHash_marshall,
        (UnmarshallFunction)&dcHash_unmarshall,
        (FreeFunction)&dcHash_free
    },
    {
        'H',
        (MarshallFunction)&dcMethodHeader_marshall,
        (UnmarshallFunction)&dcMethodHeader_unmarshall,
        (FreeFunction)&dcMethodHeader_free
    },
    {
        'l',
        (MarshallFunction)&dcList_marshall,
        (UnmarshallFunction)&dcList_unmarshall,
        (FreeFunction)&dcList_free
    },
    {
        'm',
        (MarshallFunction)&dcMatrix_marshall,
        (UnmarshallFunction)&dcMatrix_unmarshall,
        (FreeFunction)&dcMatrix_free
    },
    {
        'n',
        (MarshallFunction)&dcNode_marshall,
        (UnmarshallFunction)&dcNode_unmarshall,
        (FreeFunction)&dcNode_free
    },
    {
        'N',
        (MarshallFunction)&dcNumber_marshall,
        (UnmarshallFunction)&dcNumber_unmarshall,
        (FreeFunction)&dcNumber_free
    },
    {
        's',
        (MarshallFunction)&dcString_marshallCharArray,
        (UnmarshallFunction)&dcString_unmarshallCharArray,
        (FreeFunction)&dcString_freeCharArray
    },
    {
        'X',
        (MarshallFunction)&dcString_marshall,
        (UnmarshallFunction)&dcString_unmarshall,
        (FreeFunction)&dcString_free
    },
    {
        'S',
        (MarshallFunction)&dcScope_marshall,
        (UnmarshallFunction)&dcScope_unmarshall,
        (FreeFunction)&dcScope_free
    },
    {
        'p',
        (MarshallFunction)&dcPair_marshall,
        (UnmarshallFunction)&dcPair_unmarshall,
        (FreeFunction)&dcPair_free
    },
    {
        't',
        (MarshallFunction)&dcGraphData_marshallTree,
        (UnmarshallFunction)&dcGraphData_unmarshallTree,
        (FreeFunction)&dcGraphData_freeTree
    },
    {
        0
    }
};

static const dcMarshallType *getMarshallType(char _type)
{
    const dcMarshallType *finger = &sMarshallTypes[0];
    const dcMarshallType *result = NULL;

    while (finger->marshallFunction != NULL)
    {
        if (finger->type == _type)
        {
            result = finger;
            break;
        }

        finger++;
    }

    return result;
}

dcString *dcMarshaller_marshall(dcString *_stream, const char *_format, ...)
{
    size_t i;
    size_t formatLength = strlen(_format);
    va_list list;
    va_start(list, _format);
    dcString *result = _stream;

    if (result == NULL)
    {
        result = dcString_create();
    }

    for (i = 0; i < formatLength; i++)
    {
        char type = _format[i];
        const dcMarshallType *marshallType = getMarshallType(type);

        if (marshallType != NULL)
        {
            void *object = va_arg(list, void*);

            if (object == NULL)
            {
                dcString_appendCharacter(result, 0xFF);
            }
            else
            {
                marshallType->marshallFunction(object, result);
            }
        }
        else if (_format[i] == 'b')
        {
            // b == bool
            dcString_appendCharacter(result, va_arg(list, int));
        }
        else if (_format[i] == 'u')
        {
            // u == uint8_t
            dcString_appendCharacter(result, va_arg(list, unsigned int));
        }
        else if (_format[i] == 'v')
        {
            // v == uint16_t
            // uint16_t is promoted to int
            dcUnsignedInt32_marshallInt16u(va_arg(list, int), result);
        }
        else if (_format[i] == 'c')
        {
            // c == char / int8_t
            dcString_appendCharacter(result, va_arg(list, int));
        }
        else if (_format[i] == 'w')
        {
            // w == uint32_t
            dcUnsignedInt32_marshall(va_arg(list, uint32_t), result);
        }
        else if (_format[i] == 'x'
                 || _format[i] == 'o')
        {
            // x == int32_t
            // o == dcTaffyOperator
            dcUnsignedInt32_marshall(va_arg(list, int32_t), result);
        }
        else if (_format[i] == 'i')
        {
            // i == int32_t
            dcInt32_marshall(va_arg(list, uint32_t), result);
        }
        else
        {
            dcError("unknown marshall type: %c", type);
        }
    }

    return result;
}

static bool unmarshall(dcString *_stream,
                       const char *_format,
                       va_list _list,
                       bool _allowNull)
{
    size_t successCount = 0;
    size_t i;
    size_t formatLength = strlen(_format);
    bool result = true;
    dcList *unmarshalled = dcList_create();

    for (i = 0; result && i < formatLength; i++)
    {
        char type = _format[i];
        const dcMarshallType *marshallType = getMarshallType(type);

        if (dcString_getLengthLeft(_stream) == 0)
        {
            result = false;
        }
        else if (marshallType != NULL
                 || type == 'g') // g is for GraphData
        {
            void **object = va_arg(_list, void **);

            if (dcString_peek(_stream) == 0xFF)
            {
                if (! _allowNull)
                {
                    result = false;
                }
                else
                {
                    *object = NULL;
                    dcString_incrementIndex(_stream, 1);
                    // result = true by default
                }
            }
            else
            {
                if (type == 'g')
                {
                    *object = dcNode_unmarshall(_stream);
                }
                else
                {
                    *object = marshallType->unmarshallFunction(_stream);
                }

                if (type == 'g'
                    && *object != NULL
                    && ! IS_GRAPH_DATA(((dcNode *)*object)))
                {
                    result = false;
                    dcNode_free((dcNode **)object, DC_DEEP);
                }
                else
                {
                    result = (*object != NULL);

                    if (*object != NULL)
                    {
                        dcList_push(unmarshalled,
                                    dcPair_createNode
                                    (dcDoubleVoid_createNode(object),
                                     dcVoidContainer_createNode
                                     (type == 'g'
                                      ? (void *)dcNode_free
                                      : (void *)marshallType->freeFunction)));
                    }
                }
            }
        }
        else if (type == 'b')
        {
            // bool
            assert(sizeof(bool) == sizeof(char));
            *(va_arg(_list, char *)) = dcString_getCharacter(_stream);
        }
        else if (type == 'c')
        {
            // result = true by default
            *(va_arg(_list, int *)) = dcString_getCharacter(_stream);
        }
        else if (type == 'C')
        {
            // result = true by default
            *(va_arg(_list, char *)) = dcString_getCharacter(_stream);
        }
        else if (type == 'u')
        {
            // result = true by default
            *(va_arg(_list, uint8_t *)) = dcString_getCharacter(_stream);
        }
        else if (type == 'v')
        {
            result = dcUnsignedInt32_unmarshallInt16u
                (_stream, va_arg(_list, uint16_t *));
        }
        else if (type == 'w')
        {
            result = dcUnsignedInt32_unmarshall
                (_stream, va_arg(_list, uint32_t *));
        }
        else if (type == 'x'
                 || type == 'o')
        {
            // x == int32_t
            // o == dcTaffyOperator
            result = dcInt32_unmarshall(_stream, va_arg(_list, int32_t *));
        }
        else if (type == 'i')
        {
            // i == int32_t
            result = dcInt32_unmarshall
                (_stream, va_arg(_list, int32_t *));
        }
        else
        {
            fprintf(stderr, "don't know how to unmarshall type '%c'\n", type);
            assert(false);
        }

        if (result)
        {
            successCount++;
        }
    }

    if (! result)
    {
        while (unmarshalled->size > 0)
        {
            dcNode *pairNode = dcList_pop(unmarshalled, DC_FLOATING);
            dcPair *pair = CAST_PAIR(pairNode);
            ((FreeFunction)CAST_VOID(pair->right))
                (CAST_DOUBLE_VOID(pair->left), DC_DEEP);
            CAST_DOUBLE_VOID(pair->left) = NULL;
            dcNode_free(&pairNode, DC_DEEP);
        }
    }

    while (unmarshalled->size > 0)
    {
        dcNode *pair = dcList_pop(unmarshalled, DC_FLOATING);
        CAST_DOUBLE_VOID(CAST_PAIR(pair)->left) = NULL;
        dcNode_free(&pair, DC_DEEP);
    }

    dcList_free(&unmarshalled, DC_DEEP);
    va_end(_list);
    return result;
}

bool dcMarshaller_unmarshall(dcString *_stream, const char *_format, ...)
{
    va_list list;
    va_start(list, _format);
    return unmarshall(_stream, _format, list, true);
}

bool dcMarshaller_unmarshallNoNull(dcString *_stream, const char *_format, ...)
{
    va_list list;
    va_start(list, _format);
    return unmarshall(_stream, _format, list, false);
}
