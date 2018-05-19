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
#include "dcIn.h"
#include "dcGraphData.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcIn *dcIn_create(dcNode *_left, dcArray *_array)
{
    dcIn *in = (dcIn *)dcMemory_allocate(sizeof(dcIn));
    in->left = _left;
    in->array = _array;
    return in;
}

dcNode *dcIn_createNode(dcNode *_left, dcArray *_array)
{
    return dcGraphData_createNodeWithGuts
        (NODE_IN, dcIn_create(_left, _array));
}

void dcIn_markNode(dcNode *_node)
{
    // this should not happen
    assert(false);
}

void dcIn_freeNode(dcNode *_node, dcDepth _depth)
{
    dcIn *in = CAST_IN(_node);

    if (in != NULL)
    {
        dcIn_free(&in, _depth);
    }
}

void dcIn_free(dcIn **_if, dcDepth _depth)
{
    if (*_if != NULL)
    {
        dcIn *in = *_if;

        if (_depth == DC_DEEP)
        {
            dcNode_free(&(in->left), _depth);
            dcArray_free(&(in->array), _depth);
        }

        dcMemory_free(*_if);
    }
}

void dcIn_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcIn *from = CAST_IN(_from);
    CAST_IN(_to) = dcIn_create(dcNode_copy(from->left, _depth),
                               dcArray_copy(from->array, _depth));
}

dcResult dcIn_printNode(const dcNode *_node, dcString *_string)
{
    dcIn *in = CAST_IN(_node);
    dcResult result = dcNode_print(in->left, _string);

    if (result == TAFFY_SUCCESS)
    {
        dcString_appendString(_string, " in ");
        result = dcArray_print(in->array, _string);
    }

    return result;
}

bool dcIn_unmarshallNode(dcNode *_node, dcString *_stream)
{
    // finish me
    return false;
}

dcString *dcIn_marshallNode(const dcNode *_ifNode, dcString *_stream)
{
    // finish me
    return _stream;
}
