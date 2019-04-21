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

#include "dcGraphData.h"
#include "dcList.h"
#include "dcMethodCall.h"
#include "dcNode.h"
#include "dcNotEqualCall.h"
#include "dcString.h"

dcNode *dcNotEqualCall_createNode(dcNode *_toNotEqual)
{
    // graph data node
    return dcGraphData_createNodeWithGuts(NODE_NOT_EQUAL_CALL, _toNotEqual);
}

dcResult dcNotEqualCall_printNode(const dcNode *_node, dcString *_string)
{
    const dcMethodCall *methodCall = CAST_METHOD_CALL(CAST_NOTEQUALCALL(_node));
    dcResult result = dcNode_print(methodCall->receiver, _string);

    if (result == TAFFY_SUCCESS)
    {
        dcString_appendString(_string, " != ");
        result = dcNode_print(dcList_get(methodCall->arguments, 0), _string);
    }

    return result;
}
