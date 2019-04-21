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

#include "dcUnsignedInt32.h"
#include "dcError.h"
#include "dcMarshaller.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcMemory.h"
#include "dcString.h"

dcNode *dcUnsignedInt32_createNode(uint32_t _value)
{
    dcNode *integerNode = dcNode_create(NODE_INT);
    CAST_INT(integerNode) = _value;
    return integerNode;
}

void dcUnsignedInt32_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_INT(_to) = CAST_INT(_from);
}

uint32_t dcUnsignedInt32_getInt(const dcNode *_intNode)
{
    return CAST_INT(_intNode);
}

dcResult dcUnsignedInt32_compareNode(dcNode *_left,
                                     dcNode *_right,
                                     dcTaffyOperator *_compareResult)
{
    uint32_t left = CAST_INT(_left);
    uint32_t right = CAST_INT(_right);
    *_compareResult = (left < right
                       ? TAFFY_LESS_THAN
                       : (left > right
                          ? TAFFY_GREATER_THAN
                          : TAFFY_EQUALS));
    return TAFFY_SUCCESS;
}

dcResult dcUnsignedInt32_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    *_hashResult = CAST_INT(_node);
    return TAFFY_SUCCESS;
}

bool dcUnsignedInt32_unmarshallInt16u(dcString *_stream, uint16_t *_number)
{
    uint32_t number;
    bool result = dcUnsignedInt32_unmarshall(_stream, &number);

    if (result)
    {
        *_number = number;
    }

    return result;
}

bool dcUnsignedInt32_unmarshall(dcString *_stream, uint32_t *_number)
{
    bool result = true;
    *_number = 0;
    uint8_t number;

    if (dcMarshaller_unmarshallNoNull(_stream, "u", &number))
    {
        *_number = (number & 0x7F);
        uint32_t i = 1;

        while ((number & 0x80) != 0)
        {
            if (dcMarshaller_unmarshallNoNull(_stream, "u", &number))
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

bool dcUnsignedInt32_unmarshallNode(dcNode *_node, dcString *_stream)
{
    uint32_t number;
    bool result = dcUnsignedInt32_unmarshall(_stream, &number);

    if (result)
    {
        CAST_INT(_node) = number;
    }

    return result;
}

dcTaffy_createMarshallNodeFunctionMacro(dcUnsignedInt32, CAST_INT);

dcString *dcUnsignedInt32_marshallInt16u(uint16_t _number, dcString *_stream)
{
    return dcUnsignedInt32_marshall((int32_t)_number, _stream);
}

dcString *dcUnsignedInt32_marshall(uint32_t _number, dcString *_stream)
{
    uint32_t number = _number;
    bool doIt = true;
    uint32_t mask = 0x7F;
    uint32_t i = 1;
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
        uint32_t oldMask = mask;
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

dcResult dcUnsignedInt32_printNode(const dcNode *_node, dcString *_stream)
{
    dcString_append(_stream, "INT(%d)", dcUnsignedInt32_getInt(_node));
    return TAFFY_SUCCESS;
}
