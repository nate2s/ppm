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

#include "dcBool.h"
#include "dcFlatArithmetic.h"
#include "dcGraphData.h"
#include "dcNode.h"
#include "dcPair.h"
#include "dcString.h"

dcResult dcBool_printNode(const dcNode *_node, dcString *_stream)
{
    dcNode *nodes[] =
        {CAST_GRAPH_DATA_PAIR(_node)->left,
         CAST_GRAPH_DATA_PAIR(_node)->right};
    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(nodes); i++)
    {
        bool parentheses = false;

        if (IS_FLAT_ARITHMETIC(nodes[i])
            || (IS_AND(_node) && IS_OR(nodes[i]))
            || (IS_OR(_node) && IS_AND(nodes[i])))
        {
            parentheses = true;
            dcString_appendString(_stream, "(");
        }

        dcNode_print(nodes[i], _stream);

        if (parentheses)
        {
            dcString_appendString(_stream, ")");
        }

        if (i < dcTaffy_countOf(nodes) - 1)
        {
            dcString_append(_stream,
                            " %s ",
                            (IS_AND(_node)
                             ? "and"
                             : (IS_OR(_node)
                                ? "or"
                                : "??????")));
        }
    }

    return TAFFY_SUCCESS;
}
