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

#include "dcGraphDataNode.h"
#include "dcError.h"
#include "dcGraphData.h"
#include "dcMarshaller.h"
#include "dcNode.h"
#include "dcString.h"

void dcGraphDataNode_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_GRAPH_DATA_NODE(_to) =
        dcNode_copy(CAST_GRAPH_DATA_NODE(_from), _depth);
}

void dcGraphDataNode_copyTree(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_GRAPH_DATA_NODE(_to) =
        dcGraphData_copyTree(CAST_GRAPH_DATA_NODE(_from));
}

void dcGraphDataNode_freeNode(dcNode *_node, dcDepth _depth)
{
    if (_depth == DC_DEEP)
    {
        dcNode_free(&(CAST_GRAPH_DATA_NODE(_node)), _depth);
    }
}

void dcGraphDataNode_freeTree(dcNode *_node, dcDepth _depth)
{
    if (_depth == DC_DEEP)
    {
        dcGraphData_freeTree(&(CAST_GRAPH_DATA_NODE(_node)), DC_DEEP);
    }
}

void dcGraphDataNode_markNode(dcNode *_node)
{
    dcNode_mark(CAST_GRAPH_DATA_NODE(_node));
}

dcString *dcGraphDataNode_marshallNode(const dcNode *_node, dcString *_stream)
{
    return dcMarshaller_marshall(_stream, "n", CAST_GRAPH_DATA_NODE(_node));
}

bool dcGraphDataNode_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcNode *node;

    if (dcMarshaller_unmarshall(_stream, "g", &node))
    {
        result = true;
        CAST_GRAPH_DATA_NODE(_node) = node;

        if (node != NULL)
        {
            dcNode_setTemplate(node, true);
        }
    }

    return result;
}
