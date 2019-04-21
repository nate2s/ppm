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
#include <string.h>

#include "dcIdentifier.h"
#include "dcClass.h"
#include "dcFunctionClass.h"
#include "dcGraphData.h"
#include "dcLexer.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcString.h"
#include "dcSymbolClass.h"
#include "dcSystem.h"

dcIdentifier *dcIdentifier_create(const char *_name,
                                  dcScopeDataFlags _scopeDataFlags)
{
    dcIdentifier *identifier =
        (dcIdentifier *)dcMemory_allocate(sizeof(dcIdentifier));
    identifier->name = dcMemory_strdup(_name);
    identifier->scopeDataFlags = _scopeDataFlags;
    return identifier;
}

dcNode *dcIdentifier_createNode(const char *_name,
                                dcScopeDataFlags _scopeDataFlags)
{
    return dcGraphData_createNodeWithGuts
        (NODE_IDENTIFIER, dcIdentifier_create(_name, _scopeDataFlags));
}

void dcIdentifier_free(dcIdentifier **_identifier, dcDepth _depth)
{
    if ((*_identifier)->name != NULL)
    {
        // name may be NULL due to a failed unmarshall //
        dcMemory_free((*_identifier)->name);
    }

    dcMemory_free((*_identifier));
}

void dcIdentifier_freeNode(dcNode *_node, dcDepth _depth)
{
    dcIdentifier_free(&CAST_IDENTIFIER(_node), _depth);
}

dcResult dcIdentifier_printNode(const dcNode *_node, dcString *_string)
{
    dcIdentifier *identifier = CAST_IDENTIFIER(_node);
    dcString_appendString(_string, identifier->name);
    return TAFFY_SUCCESS;
}

dcIdentifier *dcIdentifier_copy(const dcIdentifier *_from, dcDepth _depth)
{
    return dcIdentifier_create(_from->name, _from->scopeDataFlags);
}

// create dcIdentifier_marshallNode //
dcTaffy_createMarshallNodeFunctionMacro(dcIdentifier, CAST_IDENTIFIER);

// create dcIdentifier_unmarshallNode //
dcTaffy_createUnmarshallNodeFunctionMacro(dcIdentifier, CAST_IDENTIFIER);

// create dcIdentifier_copyNode //
dcTaffy_createCopyNodeFunctionMacro(dcIdentifier, CAST_IDENTIFIER);

const char *dcIdentifier_getName(const dcNode *_identifier)
{
    assert(IS_IDENTIFIER(_identifier));
    return CAST_IDENTIFIER(_identifier)->name;
}

dcScopeDataFlags dcIdentifier_getScopeDataFlags(const dcNode *_identifier)
{
    return CAST_IDENTIFIER(_identifier)->scopeDataFlags;
}

dcResult dcIdentifier_compareNode(dcNode *_left,
                                  dcNode *_right,
                                  dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_SUCCESS;

    if (dcGraphData_getType(_right) == NODE_IDENTIFIER)
    {
        int strcmpResult = strcmp(CAST_IDENTIFIER(_left)->name,
                            CAST_IDENTIFIER(_right)->name);
        *_compareResult = (strcmpResult == 0
                           ? TAFFY_EQUALS
                           : (strcmpResult < 0
                              ? TAFFY_LESS_THAN
                              : TAFFY_GREATER_THAN));
    }
    else if (dcNumberClass_isMe(_right))
    {
        *_compareResult = TAFFY_GREATER_THAN;
    }
    else
    {
        result = TAFFY_FAILURE;
    }

    return result;
}

bool dcIdentifier_equals(const dcIdentifier *_left,
                         const dcIdentifier *_right)
{
    return (strcmp(_left->name, _right->name) == 0);
}

bool dcIdentifier_equalsString(const dcNode *_left, const char *_name)
{
    return (strcmp(CAST_IDENTIFIER(_left)->name, _name) == 0);
}

dcIdentifier *dcIdentifier_unmarshall(dcString *_stream)
{
    dcIdentifier *result = NULL;
    uint16_t scopeDataFlags;
    char *name;

    if (dcMarshaller_unmarshallNoNull(_stream,
                                      "vs",
                                      &scopeDataFlags,
                                      &name))
    {
        result = dcIdentifier_create(name, scopeDataFlags);
        dcMemory_free(name);
    }

    return result;
}

dcString *dcIdentifier_marshall(dcIdentifier *_identifier, dcString *_stream)
{
    return dcMarshaller_marshall(_stream,
                                 "vs",
                                 _identifier->scopeDataFlags,
                                 _identifier->name);
}

dcResult dcIdentifier_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    dcHashType nameResult = 0;
    assert(dcString_hashCharArray(CAST_IDENTIFIER(_node)->name, &nameResult));
    *_hashResult = NODE_IDENTIFIER + nameResult;
    return TAFFY_SUCCESS;
}

bool dcIdentifier_isMe(const dcNode *_node)
{
    return (IS_GRAPH_DATA(_node) && IS_IDENTIFIER(_node));
}
