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
#include "dcNode.h"
#include "dcOr.h"
#include "dcPair.h"
#include "dcString.h"

dcNode *dcOr_createNode(dcNode *_left, dcNode *_right)
{
    dcNode *orNode = dcGraphData_createNode(NODE_OR);
    CAST_GRAPH_DATA_PAIR(orNode) = dcPair_create(_left, _right);
    return orNode;
}

dcNode *dcOr_getLeft(const dcNode *_andNode)
{
    return CAST_GRAPH_DATA_PAIR(_andNode)->left;
}

dcNode *dcOr_getRight(const dcNode *_andNode)
{
    return CAST_GRAPH_DATA_PAIR(_andNode)->right;
}
