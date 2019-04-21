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
#include "dcString.h"

dcNode *dcVoid_createNode(void *_location)
{
    return dcNode_createWithGuts(NODE_VOID, _location);
}

void dcVoid_freeNode(dcNode *_node, dcDepth _depth)
{
    if (_depth == DC_DEEP)
    {
        dcMemory_free(CAST_VOID(_node));
    }
}

dcResult dcVoid_compareNode(dcNode *_left,
                            dcNode *_right,
                            dcTaffyOperator *_compareResult)
{
    if (CAST_VOID(_left) == CAST_VOID(_right))
    {
        *_compareResult = TAFFY_EQUALS;
        return TAFFY_SUCCESS;
    }

    return TAFFY_FAILURE;
}

void dcVoid_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_VOID(_to) = CAST_VOID(_from);
}

dcResult dcVoid_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    *_hashResult = (dcHashType)(size_t)CAST_VOID(_node);
    return TAFFY_SUCCESS;
}

dcResult dcVoid_printNode(const dcNode *_node, dcString *_stream)
{
    dcString_append(_stream, "VOID(%p)", CAST_VOID(_node));
    return TAFFY_SUCCESS;
}

void *dcVoid_castMe(const dcNode *_node)
{
    return CAST_VOID(_node);
}
