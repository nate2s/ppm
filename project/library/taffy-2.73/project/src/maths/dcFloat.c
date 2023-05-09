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

#include "dcFloat.h"
#include "dcError.h"
#include "dcMarshaller.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcMemory.h"
#include "dcString.h"

dcNode *dcFloat_createNode(float _value)
{
    dcNode *integerNode = dcNode_create(NODE_FLOAT);
    CAST_FLOAT(integerNode) = _value;
    return integerNode;
}

void dcFloat_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_FLOAT(_to) = CAST_FLOAT(_from);
}

float dcFloat_getFloat(const dcNode *_floatNode)
{
    return CAST_FLOAT(_floatNode);
}

dcResult dcFloat_compareNode(dcNode *_left,
                             dcNode *_right,
                             dcTaffyOperator *_compareResult)
{
    float left = CAST_FLOAT(_left);
    float right = CAST_FLOAT(_right);

    *_compareResult = (left < right
                       ? TAFFY_LESS_THAN
                       : (left > right
                          ? TAFFY_GREATER_THAN
                          : TAFFY_EQUALS));
    return TAFFY_SUCCESS;
}

dcResult dcFloat_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    return dcString_hashBytes((const uint8_t*)&(CAST_FLOAT(_node)),
                              sizeof(CAST_FLOAT(_node)),
                              _hashResult);
}

bool dcFloat_unmarshall(dcString *_stream, float *_number)
{
    dcNumber *number = dcNumber_unmarshall(_stream);
    bool result = false;

    if (number != NULL)
    {
        double value;

        // no conversion loss is possible
        if (dcNumber_extractDouble(number, &value))
        {
            *_number = (float)value;
            result = true;
        }
    }

    dcNumber_free(&number, DC_DEEP);
    return result;
}

bool dcFloat_unmarshallNode(dcNode *_node, dcString *_stream)
{
    float number;
    bool result = dcFloat_unmarshall(_stream, &number);

    if (result)
    {
        CAST_FLOAT(_node) = number;
    }

    return result;
}

dcTaffy_createMarshallNodeFunctionMacro(dcFloat, CAST_FLOAT);

dcString *dcFloat_marshall(float _number, dcString *_stream)
{
    dcNumber *number = dcNumber_createFromDouble(_number);
    dcString *result = dcNumber_marshall(number, _stream);
    dcNumber_free(&number, DC_DEEP);
    return result;
}

dcResult dcFloat_printNode(const dcNode *_node, dcString *_stream)
{
    dcString_append(_stream, "FLOAT(%f)", dcFloat_getFloat(_node));
    return TAFFY_SUCCESS;
}
