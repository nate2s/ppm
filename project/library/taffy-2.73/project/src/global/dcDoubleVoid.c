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

#include "dcVoid.h"
#include "dcMemory.h"
#include "dcNode.h"

dcNode *dcDoubleVoid_createNode(void **_location)
{
    return dcNode_createWithGuts(NODE_DOUBLE_VOID, _location);
}

void dcDoubleVoid_freeNode(dcNode *_node, dcDepth _depth)
{
    if (_depth == DC_DEEP)
    {
        dcMemory_free(*CAST_DOUBLE_VOID(_node));
    }
}
