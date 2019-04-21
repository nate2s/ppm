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
#include "dcImport.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcPackage.h"
#include "dcString.h"

dcNode *dcImport_createNode(dcList *_path)
{
    dcNode *importNode = dcGraphData_createNode(NODE_IMPORT);
    CAST_IMPORT(importNode) = dcPackage_create(_path);
    return importNode;
}

dcResult dcImport_printNode(const dcNode *_node, dcString *_string)
{
    dcString_appendString(_string, "[import ");
    dcPackage_print(CAST_IMPORT(_node), _string);
    dcString_appendCharacter(_string, ']');
    return TAFFY_SUCCESS;
}
