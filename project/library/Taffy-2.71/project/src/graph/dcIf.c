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

#include "dcIf.h"
#include "dcGraphData.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcIf *dcIf_create(dcNode *_condition, dcNode *_statement, dcNode *_next)
{
    dcIf *ifData = (dcIf *)dcMemory_allocate(sizeof(dcIf));
    ifData->condition = _condition;
    ifData->statement = _statement;
    ifData->next = _next;
    return ifData;
}

dcNode *dcIf_createNode(dcNode *_condition, dcNode *_statement, dcNode *_next)
{
    return dcGraphData_createNodeWithGuts
        (NODE_IF, dcIf_create(_condition, _statement, _next));
}

void dcIf_markNode(dcNode *_node)
{
    dcIf *ifData = CAST_IF(_node);
    dcNode_mark(ifData->condition);
    dcNode_mark(ifData->statement);
    dcNode_mark(ifData->next);
}

void dcIf_freeNode(dcNode *_node, dcDepth _depth)
{
    dcIf *ifData = CAST_IF(_node);

    if (ifData != NULL)
    {
        dcIf_free(&ifData, _depth);
    }
}

void dcIf_free(dcIf **_if, dcDepth _depth)
{
    if (*_if != NULL)
    {
        dcIf *ifData = *_if;

        if (_depth == DC_DEEP)
        {
            dcNode_free(&(ifData->condition), _depth);
            dcNode_free(&(ifData->statement), _depth);
        }

        dcNode_free(&(ifData->next), _depth);
        dcMemory_free(*_if);
    }
}

void dcIf_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcIf *fromIf = CAST_IF(_from);
    // ->next points to the "else" //

    CAST_IF(_to) = dcIf_create(dcNode_copy(fromIf->condition, _depth),
                               dcGraphData_copyTree(fromIf->statement),
                               dcNode_copy(fromIf->next, _depth));
}

dcResult dcIf_printNode(const dcNode *_node, dcString *_string)
{
    dcString_appendString(_string, "if (");
    dcIf *ifData = CAST_IF(_node);

    dcResult result = dcNode_print(ifData->condition, _string);

    if (result == TAFFY_SUCCESS)
    {
        dcString_appendString(_string, ") {");
        result = dcNode_print(ifData->statement, _string);
        dcString_appendString(_string, "}");
    }

    return result;
}

dcNode *dcIf_getNext(const dcNode *_if)
{
    return CAST_IF(_if)->next;
}

dcNode *dcIf_getCondition(const dcNode *_if)
{
    return CAST_IF(_if)->condition;
}

dcNode *dcIf_getStatement(const dcNode *_if)
{
    return CAST_IF(_if)->statement;
}

bool dcIf_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcNode *condition = NULL;
    dcNode *statement = NULL;
    dcIf *first = NULL;
    int more = IF_HAS_NEXT;
    dcIf *previousIf = NULL;

    while (more == IF_HAS_NEXT
           && (result = dcMarshaller_unmarshallNoNull(_stream,
                                                      "nti",
                                                      &condition,
                                                      &statement,
                                                      &more)))
    {
        if (first == NULL)
        {
            first = dcIf_create(condition, statement, NULL);
            previousIf = first;
        }
        else
        {
            previousIf->next = dcIf_createNode(condition, statement, NULL);
            previousIf = CAST_IF(previousIf->next);
        }
    }

    if (! result)
    {
        dcIf_freeNode(_node, DC_DEEP);
        dcIf_free(&first, DC_DEEP);
    }
    else
    {
        CAST_IF(_node) = first;
    }

    return result;
}

dcString *dcIf_marshallNode(const dcNode *_ifNode, dcString *_stream)
{
    const dcNode *ifIterator = _ifNode;
    dcString *result = _stream;

    //
    // marshall the "else" parts incrementally
    //
    while (ifIterator != NULL)
    {
        result = dcMarshaller_marshall(_stream,
                                       "nt",
                                       dcIf_getCondition(ifIterator),
                                       dcIf_getStatement(ifIterator));
        ifIterator = CAST_IF(ifIterator)->next;

        if (ifIterator != NULL)
        {
            dcString_appendCharacter(_stream, IF_HAS_NEXT);
        }
    }

    //
    // we're done
    //
    dcString_appendCharacter(_stream, IF_HAS_NO_NEXT);
    return result;
}
