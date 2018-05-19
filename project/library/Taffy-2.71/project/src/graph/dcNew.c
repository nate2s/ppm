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

#include "dcNew.h"
#include "dcGraphData.h"
#include "dcLexer.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"
#include "dcVoid.h"

dcNode *dcNew_createNode(dcNode *_value)
{
    return dcGraphData_createNodeWithGuts(NODE_NEW, _value);
}

char *dcNew_display(const dcNode *_node)
{
    return (char *)(CAST_VOID
                    (dcNode_register
                     (dcVoid_createNode
                      (dcLexer_sprintf
                       ("new %s",
                        dcNode_display(CAST_NEW(_node)))))));
}

dcResult dcNew_printNode(const dcNode *_node, dcString *_string)
{
    dcString_appendString(_string, "new ");
    return dcNode_print(CAST_NEW(_node), _string);
}

// for copy, free, see dcGraphDataNode.c //
