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

#include "dcWhile.h"
#include "dcNode.h"
#include "dcPair.h"
#include "dcGraphData.h"
#include "dcString.h"

dcNode *dcWhile_createNode(dcNode *_condition, dcNode *_statement)
{
    return dcGraphData_createNodeWithGuts
        (NODE_WHILE, dcPair_create(_condition, _statement));
}

dcNode *dcWhile_getCondition(const dcNode *_whileNode)
{
    return CAST_GRAPH_DATA_PAIR(_whileNode)->left;
}

dcNode *dcWhile_getStatement(const dcNode *_whileNode)
{
    return CAST_GRAPH_DATA_PAIR(_whileNode)->right;
}

dcResult dcWhile_printNode(const dcNode *_whileNode, dcString *_stream)
{
    dcResult result = TAFFY_SUCCESS;
    const char *conditionDisplay =
        dcNode_display(dcWhile_getCondition(_whileNode));

    if (conditionDisplay != NULL)
    {
        const char *statementDisplay =
            dcNode_display(dcWhile_getStatement(_whileNode));

        if (statementDisplay != NULL)
        {
            dcString_append(_stream,
                            "while (%s) { %s }",
                            conditionDisplay,
                            statementDisplay);
        }
        else
        {
            result = TAFFY_EXCEPTION;
        }
    }
    else
    {
        result = TAFFY_EXCEPTION;
    }

    return result;
}
