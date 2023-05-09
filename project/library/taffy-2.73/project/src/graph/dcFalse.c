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

#include "dcFalse.h"
#include "dcGraphData.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcNode *dcFalse_createNode(void)
{
    return dcGraphData_createNode(NODE_NO);
}

dcResult dcFalse_printNode(const dcNode *_node, dcString *_string)
{
    dcString_appendString(_string, "false");
    return TAFFY_SUCCESS;
}