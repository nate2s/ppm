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

#include "dcExit.h"
#include "dcNode.h"
#include "dcGraphData.h"
#include "dcMemory.h"
#include "dcString.h"

dcNode *dcExit_createNode(dcNode *_exitValue)
{
    // graph data node
    return dcGraphData_createNodeWithGuts(NODE_EXIT, _exitValue);
}

dcResult dcExit_printNode(const dcNode *_node, dcString *_string)
{
    dcString_appendString(_string, "exit");
    dcNode *exitNode = CAST_EXIT(_node);
    dcResult result = TAFFY_SUCCESS;

    if (exitNode != NULL)
    {
        dcString_appendCharacter(_string, '(');
        result = dcNode_print(exitNode, _string);
        dcString_appendCharacter(_string, ')');
    }

    return result;
}

dcNode *dcExit_getValueFromNode(const dcNode *_node)
{
    return CAST_EXIT(_node);
}
