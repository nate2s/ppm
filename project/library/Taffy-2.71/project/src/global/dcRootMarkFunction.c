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

#include "dcRootMarkFunction.h"
#include "dcNode.h"

dcNode *dcRootMarkFunction_createNode(dcTaffy_rootMarkPointer markerFunction)
{
    return dcNode_createWithGuts(NODE_ROOT_MARK_FUNCTION,
                                 (void *)markerFunction);
}

dcTaffy_rootMarkPointer dcRootMarkFunction_getMarkPointer(const dcNode *_node)
{
    return CAST_ROOT_MARK_FUNCTION(_node);
}

void dcRootMarkFunction_markNode(dcNode *_node)
{
    dcTaffy_rootMarkPointer marker = CAST_ROOT_MARK_FUNCTION(_node);
    marker();
}

dcResult dcRootMarkFunction_compareNode(dcNode *_left,
                                        dcNode *_right,
                                        dcTaffyOperator *_compareResult)
{
    dcTaffy_rootMarkPointer leftMarker = CAST_ROOT_MARK_FUNCTION(_left);
    dcTaffy_rootMarkPointer rightMarker = CAST_ROOT_MARK_FUNCTION(_right);

    *_compareResult = (leftMarker == rightMarker
                       ? TAFFY_EQUALS
                       : TAFFY_LESS_THAN);

    return TAFFY_SUCCESS;
}
