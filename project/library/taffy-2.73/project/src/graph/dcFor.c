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

#include <assert.h>

#include "dcFor.h"
#include "dcGraphData.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcPair.h"
#include "dcString.h"

dcFor *dcFor_create(dcNode *_initial,
                    dcNode *_condition,
                    dcNode *_increment,
                    dcNode *_statement)
{
    dcFor *result = (dcFor *)dcMemory_allocate(sizeof(dcFor));
    result->initial = _initial;
    result->condition = _condition;
    result->increment = _increment;
    result->statement = _statement;
    return result;
}

dcNode *dcFor_createNode(dcNode *_initial,
                         dcNode *_condition,
                         dcNode *_increment,
                         dcNode *_statement)
{
    dcNode *forNode = dcGraphData_createNode(NODE_FOR);
    CAST_FOR(forNode) =
        dcFor_create(_initial, _condition, _increment, _statement);
    return forNode;
}

void dcFor_freeNode(dcNode *_node, dcDepth _depth)
{
    dcFor *theFor = CAST_FOR(_node);

    if (_depth == DC_DEEP)
    {
        dcNode_free(&theFor->initial, _depth);
        dcNode_free(&theFor->condition, _depth);
        dcNode_free(&theFor->increment, _depth);
        dcNode_free(&theFor->statement, _depth);
    }

    dcMemory_free(theFor);
}

void dcFor_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcFor *fromFor = CAST_FOR(_from);
    dcFor *toFor = (dcFor *)dcMemory_allocate(sizeof(dcFor));
    toFor->initial = dcNode_copy(fromFor->initial, _depth);
    toFor->condition = dcNode_copy(fromFor->condition, _depth);
    toFor->increment = dcNode_copy(fromFor->increment, _depth);
    toFor->statement = dcGraphData_copyTree(fromFor->statement);
    CAST_FOR(_to) = toFor;
}

void dcFor_markNode(dcNode *_node)
{
    dcFor *theFor = CAST_FOR(_node);
    dcNode_mark(theFor->initial);
    dcNode_mark(theFor->condition);
    dcNode_mark(theFor->increment);
    dcNode_mark(theFor->statement);
}

dcResult dcFor_printNode(const dcNode *_node, dcString *_string)
{
    const dcFor *theFor = CAST_FOR(_node);
    dcString_append(_string, "for (");
    dcResult result = dcNode_print(theFor->initial, _string);

    if (result == TAFFY_SUCCESS)
    {
        dcString_append(_string, "; ");
        result = dcNode_print(theFor->condition, _string);

        if (result == TAFFY_SUCCESS)
        {
            dcString_append(_string, "; ");
            result = dcNode_print(theFor->increment, _string);

            if (result == TAFFY_SUCCESS)
            {
                dcString_append(_string, ") {");
                result = dcNode_print(theFor->statement, _string);
                dcString_append(_string, "}");
            }
        }
    }

    return result;
}

dcNode *dcFor_getInitial(const dcNode *_forNode)
{
    return CAST_FOR(_forNode)->initial;
}

dcNode *dcFor_getCondition(const dcNode *_forNode)
{
    return CAST_FOR(_forNode)->condition;
}

dcNode *dcFor_getIncrement(const dcNode *_forNode)
{
    return CAST_FOR(_forNode)->increment;
}

dcNode *dcFor_getStatement(const dcNode *_forNode)
{
    return CAST_FOR(_forNode)->statement;
}

dcString *dcFor_marshallNode(const dcNode *_node, dcString *_stream)
{
    // finish me
    return _stream;
}

bool dcFor_unmarshallNode(dcNode *_node, dcString *_stream)
{
    // finish me
    return false;
}
