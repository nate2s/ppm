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

#include "dcTaffyCMethodPointer.h"
#include "dcNode.h"
#include "dcArray.h"
#include "dcString.h"
#include "dcMemory.h"

dcNode *dcTaffyCMethodPointer_createNode(dcTaffyCMethodPointer _method)
{
    return dcNode_createWithGuts(NODE_TAFFY_C_METHOD_POINTER, (void *)_method);
}

dcNode *dcTaffyCMethodPointer_createShell(dcTaffyCMethodPointer _method)
{
    return dcTaffyCMethodPointer_createNode(_method);
}

void dcTaffyCMethodPointer_copyNode(dcNode *_to,
                                    const dcNode *_from,
                                    dcDepth _depth)
{
    CAST_TAFFY_C_METHOD_POINTER(_to) = CAST_TAFFY_C_METHOD_POINTER(_from);
}
