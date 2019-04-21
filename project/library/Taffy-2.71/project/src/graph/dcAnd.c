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

#include "dcAnd.h"
#include "dcGraphData.h"
#include "dcNode.h"
#include "dcPair.h"
#include "dcString.h"

dcNode *dcAnd_createNode(dcNode *_left, dcNode *_right)
{
    dcNode *andNode = dcGraphData_createNode(NODE_AND);
    CAST_AND(andNode) = dcPair_create(_left, _right);
    return andNode;
}

dcNode *dcAnd_getLeft(const dcNode *_andNode)
{
    return CAST_GRAPH_DATA_PAIR(_andNode)->left;
}

dcNode *dcAnd_getRight(const dcNode *_andNode)
{
    return CAST_GRAPH_DATA_PAIR(_andNode)->right;
}
