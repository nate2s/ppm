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

#include "dcAssignment.h"
#include "dcGraphData.h"
#include "dcUnsignedInt32.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcScopeData.h"
#include "dcString.h"

dcAssignment *dcAssignment_create(dcNode *_identifier,
                                  dcNode *_value,
                                  dcScopeDataFlags _flags)
{
    dcAssignment *assignment =
        (dcAssignment *)dcMemory_allocate(sizeof(dcAssignment));
    assignment->identifier = _identifier;
    assignment->value = _value;
    assignment->flags = _flags;
    return assignment;
}

dcNode *dcAssignment_createNode(dcNode *_identifier,
                                dcNode *_value,
                                dcScopeDataFlags _flags)
{
    return dcGraphData_createNodeWithGuts
        (NODE_ASSIGNMENT, dcAssignment_create(_identifier, _value, _flags));
}

void dcAssignment_freeNode(dcNode *_node, dcDepth _depth)
{
    dcAssignment *assignment = CAST_ASSIGNMENT(_node);

    if (_depth == DC_DEEP)
    {
        dcNode_free(&assignment->value, _depth);
        dcNode_free(&assignment->identifier, _depth);
    }

    dcMemory_free(assignment);
}

void dcAssignment_markNode(dcNode *_assignment)
{
    dcNode_mark(CAST_ASSIGNMENT(_assignment)->value);
}

void dcAssignment_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcAssignment *from = CAST_ASSIGNMENT(_from);
    CAST_ASSIGNMENT(_to) =
        dcAssignment_create
        (dcNode_copy(from->identifier, _depth),
         dcNode_copy(from->value, _depth),
         from->flags);
}

dcResult dcAssignment_printNode(const dcNode *_node, dcString *_string)
{
    dcResult result = dcNode_print(dcAssignment_getIdentifier(_node), _string);

    if (result == TAFFY_SUCCESS)
    {
        dcString_appendString(_string, " = ");
        result = dcNode_print(dcAssignment_getValue(_node), _string);
    }

    return result;
}

dcNode *dcAssignment_getIdentifier(const dcNode *_assignment)
{
    return CAST_ASSIGNMENT(_assignment)->identifier;
}

dcNode *dcAssignment_getValue(const dcNode *_assignment)
{
    return CAST_ASSIGNMENT(_assignment)->value;
}

dcScopeDataFlags dcAssignment_getFlags(const dcNode *_assignment)
{
    return CAST_ASSIGNMENT(_assignment)->flags;
}

bool dcAssignment_isGlobal(const dcAssignment *_assignment)
{
    return ((_assignment->flags & SCOPE_DATA_GLOBAL) != 0);
}

bool dcAssignment_isConstant(const dcAssignment *_assignment)
{
    return ((_assignment->flags & SCOPE_DATA_CONSTANT) != 0);
}

bool dcAssignment_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    uint16_t flags;
    dcNode *identifier;
    dcNode *value;

    if (dcMarshaller_unmarshallNoNull(_stream,
                                      "vnn",
                                      &flags,
                                      &identifier,
                                      &value))
    {
        result = true;
        CAST_ASSIGNMENT(_node) =
            dcAssignment_create(identifier, value, flags);
    }

    return result;
}

dcString *dcAssignment_marshallNode(const dcNode *_node, dcString *_stream)
{
    return dcMarshaller_marshall(_stream,
                                 "vnn",
                                 dcAssignment_getFlags(_node),
                                 dcAssignment_getIdentifier(_node),
                                 dcAssignment_getValue(_node));
}

dcAssignment *dcAssignment_castMe(dcNode *_graphDataNode)
{
    return CAST_ASSIGNMENT(_graphDataNode);
}
