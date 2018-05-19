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

#include "dcInt32.h"
#include "dcError.h"
#include "dcMarshaller.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcMemory.h"
#include "dcString.h"
#include "dcUnsignedInt32.h"

dcNode *dcInt32_createNode(int32_t _value)
{
    dcNode *integerNode = dcNode_create(NODE_INT);
    CAST_SIGNED_INT_32(integerNode) = _value;
    return integerNode;
}

void dcInt32_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_SIGNED_INT_32(_to) = CAST_SIGNED_INT_32(_from);
}

int32_t dcInt32_getInt(const dcNode *_intNode)
{
    return CAST_SIGNED_INT_32(_intNode);
}

dcResult dcInt32_compareNode(dcNode *_left,
                             dcNode *_right,
                             dcTaffyOperator *_compareResult)
{
    int32_t left = CAST_SIGNED_INT_32(_left);
    int32_t right = CAST_SIGNED_INT_32(_right);
    *_compareResult = (left < right
                       ? TAFFY_LESS_THAN
                       : (left > right
                          ? TAFFY_GREATER_THAN
                          : TAFFY_EQUALS));
    return TAFFY_SUCCESS;
}

dcResult dcInt32_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    *_hashResult = CAST_SIGNED_INT_32(_node);
    return TAFFY_SUCCESS;
}

bool dcInt32_unmarshall(dcString *_stream, int32_t *_number)
{
    uint32_t number = 0;
    bool result = dcUnsignedInt32_unmarshall(_stream, &number);

    if (result)
    {
        *_number = number;
    }

    return result;
}

dcString *dcInt32_marshall(int32_t _number, dcString *_stream)
{
    return dcUnsignedInt32_marshall(_number, _stream);
}

bool dcInt32_unmarshallNode(dcNode *_node, dcString *_stream)
{
    int32_t number = 0;
    bool result = dcInt32_unmarshall(_stream, &number);

    if (result)
    {
        CAST_SIGNED_INT_32(_node) = number;
    }

    return result;
}

dcTaffy_createMarshallNodeFunctionMacro(dcInt32, CAST_SIGNED_INT_32);

dcResult dcInt32_printNode(const dcNode *_node, dcString *_stream)
{
    dcString_append(_stream, "SIGNED_INT_32(%d)", dcInt32_getInt(_node));
    return TAFFY_SUCCESS;
}
