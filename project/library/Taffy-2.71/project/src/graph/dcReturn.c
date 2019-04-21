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

#include "dcError.h"
#include "dcReturn.h"
#include "dcGraphData.h"
#include "dcNode.h"
#include "dcString.h"

dcNode *dcReturn_createNode(dcNode *_returnValue)
{
    // graph data node
    TAFFY_DEBUG(dcError_assert(_returnValue == NULL
                               || dcNode_isTemplate(_returnValue)));
    return dcGraphData_createNodeWithGuts(NODE_RETURN, _returnValue);
}

dcResult dcReturn_printNode(const dcNode *_node, dcString *_string)
{
    dcString_appendString(_string, "return");
    dcNode *returnValue = CAST_RETURN(_node);
    dcResult result = TAFFY_SUCCESS;

    if (returnValue != NULL)
    {
        dcString_appendString(_string, " (");
        result = dcNode_print(returnValue, _string);
        dcString_appendString(_string, ")");
    }

    return result;
}

dcNode *dcReturn_getValueFromNode(dcNode *_returnNode)
{
    return CAST_RETURN(_returnNode);
}
