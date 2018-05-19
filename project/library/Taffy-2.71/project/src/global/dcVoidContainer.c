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

//
// A void container is just like a void, but it doesn't free its memory
// when it is freed DC_DEEPly
//

#include "dcVoid.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcNode *dcVoidContainer_createNode(void *_location)
{
    return dcNode_createWithGuts(NODE_VOID_CONTAINER, _location);
}

dcResult dcVoidContainer_printNode(const dcNode *_node, dcString *_stream)
{
    dcString_append(_stream, "VOID_CONTAINER(%p)", CAST_VOID(_node));
    return TAFFY_SUCCESS;
}

// everything else is found in dcVoid.c
