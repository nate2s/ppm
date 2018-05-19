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

#include "dcUnsignedInt64.h"
#include "dcError.h"
#include "dcMarshaller.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcMemory.h"
#include "dcString.h"

dcNode *dcUnsignedInt64_createNode(uint64_t _value)
{
    dcNode *integerNode = dcNode_create(NODE_INT_64);
    CAST_INT_64(integerNode) = _value;
    return integerNode;
}

void dcUnsignedInt64_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_INT_64(_to) = CAST_INT_64(_from);
}

uint64_t dcUnsignedInt64_getValue(const dcNode *_intNode)
{
    return CAST_INT_64(_intNode);
}

dcResult dcUnsignedInt64_compareNode(dcNode *_left,
                                     dcNode *_right,
                                     dcTaffyOperator *_compareResult)
{
    uint64_t left = CAST_INT_64(_left);
    uint64_t right = CAST_INT_64(_right);

    *_compareResult = (left < right
                       ? TAFFY_LESS_THAN
                       : (left > right
                          ? TAFFY_GREATER_THAN
                          : TAFFY_EQUALS));
    return TAFFY_SUCCESS;
}

dcResult dcUnsignedInt64_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    *_hashResult = CAST_INT_64(_node);
    return TAFFY_SUCCESS;
}

bool dcUnsignedInt64_unmarshallInt64u(dcString *_stream, uint64_t *_number)
{
    uint64_t number;
    bool result = dcUnsignedInt64_unmarshall(_stream, &number);

    if (result)
    {
        *_number = number;
    }

    return result;
}

bool dcUnsignedInt64_unmarshall(dcString *_stream, uint64_t *_number)
{
    bool result = true;
    *_number = 0;
    uint8_t number;

    if (dcMarshaller_unmarshall(_stream, "u", &number))
    {
        *_number = (number & 0x7F);
        uint64_t i = 1;

        while ((number & 0x80) != 0)
        {
            if (dcMarshaller_unmarshall(_stream, "u", &number))
            {
                *_number += (number & 0x7F) << (7 * i);
                i++;
            }
            else
            {
                // FAILURE //
                result = false;
                break;
            }
        }
    }

    return result;
}

bool dcUnsignedInt64_unmarshallNode(dcNode *_node, dcString *_stream)
{
    uint64_t number;
    bool result = dcUnsignedInt64_unmarshall(_stream, &number);

    if (result)
    {
        CAST_INT_64(_node) = number;
    }

    return result;
}

dcTaffy_createMarshallNodeFunctionMacro(dcUnsignedInt64, CAST_INT);

dcString *dcUnsignedInt64_marshall(uint64_t _number, dcString *_stream)
{
    uint64_t number = (uint64_t)_number;
    bool doIt = true;
    uint64_t mask = 0x7F;
    uint64_t i = 1;
    dcString *result = _stream;

    while (doIt)
    {
        uint8_t toAppend = 0;

        if (number > mask)
        {
            toAppend |= 0x80;
        }
        else
        {
            doIt = false;
        }

        toAppend |= ((number & mask) >> (7 * (i - 1)));
        uint64_t oldMask = mask;
        mask = 0x7F << (7 * i);

        if (mask < oldMask)
        {
            doIt = false;
        }

        result = dcMarshaller_marshall(result, "u", toAppend);
        i++;
    }

    return result;
}

dcResult dcUnsignedInt64_printNode(const dcNode *_node, dcString *_stream)
{
    dcString_append(_stream, "INT_64(%lld)", dcUnsignedInt64_getValue(_node));
    return TAFFY_SUCCESS;
}

uint64_t dcUnsignedInt64_getInt(const dcNode *_intNode)
{
    return CAST_INT_64(_intNode);
}
