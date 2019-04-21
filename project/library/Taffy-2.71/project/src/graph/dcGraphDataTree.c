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
#include <stdio.h>
#include <stdlib.h>

#include "dcCharacterGraph.h"
#include "dcError.h"
#include "dcDefines.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcGraphDataTree *dcGraphDataTree_create(dcNode *_head)
{
    dcGraphDataTree *tree =
        (dcGraphDataTree *)dcMemory_allocate(sizeof(dcGraphDataTree));
    tree->head = _head;

    // determine the size
    dcNode *iterator = NULL;

    for (tree->size = 0, iterator = _head; iterator != NULL; tree->size++)
    {
        TAFFY_DEBUG(dcError_assert(dcNode_isTemplate(iterator)););
        iterator = (iterator->type == NODE_GRAPH_DATA
                    ? dcGraphData_getNext(iterator)
                    : NULL);
    }

    return tree;
}

dcNode *dcGraphDataTree_createNode(dcNode *_head)
{
    return dcGraphData_createNodeWithGuts
        (NODE_GRAPH_DATA_TREE, dcGraphDataTree_create(_head));
}

void dcGraphDataTree_copyNode(dcNode *_to,
                              const dcNode *_from,
                              dcDepth _ignored)
{
    CAST_GRAPH_DATA_TREE(_to) = dcGraphDataTree_create
        (dcGraphData_copyTree(CAST_GRAPH_DATA_TREE(_from)->head));
}

void dcGraphDataTree_freeNode(dcNode *_node, dcDepth _depth)
{
    if (_depth == DC_DEEP)
    {
        dcGraphData_freeTree(&CAST_GRAPH_DATA_TREE(_node)->head, DC_DEEP);
    }

    dcMemory_free(CAST_GRAPH_DATA_TREE(_node));
}

dcResult dcGraphDataTree_printNode(const dcNode *_node, dcString *_output)
{
    dcGraphDataTree *tree = CAST_GRAPH_DATA_TREE(_node);
    dcNode *iterator;
    dcResult result = TAFFY_SUCCESS;

    for (iterator = tree->head;
         iterator != NULL && result == TAFFY_SUCCESS;
         (iterator->type == NODE_GRAPH_DATA
          ? iterator = dcGraphData_getNext(iterator)
          : NULL))
    {
        if (iterator != tree->head)
        {
            dcString_append(_output, "; ");
        }

        result = dcNode_print(iterator, _output);
    }

    return result;
}

dcNode *dcGraphDataTree_getContents(const dcNode *_node)
{
    return CAST_GRAPH_DATA_TREE(_node)->head;
}

uint32_t dcGraphDataTree_getSize(const dcNode *_node)
{
    return CAST_GRAPH_DATA_TREE(_node)->size;
}

dcResult dcGraphDataTree_prettyPrintNode(const dcNode *_node,
                                         dcCharacterGraph **_graph)
{
    // der
    dcGraphDataTree *tree = CAST_GRAPH_DATA_TREE(_node);
    dcResult result = TAFFY_SUCCESS;

    if (tree->size == 1)
    {
        result = dcNode_prettyPrint(tree->head, _graph);
    }
    else
    {
        result = dcNode_printToCharacterGraph(_node, _graph);
    }

    return result;
}

dcResult dcGraphDataTree_compareNode(dcNode *_left,
                                     dcNode *_right,
                                     dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_FAILURE;

    if (_right->type == NODE_GRAPH_DATA
        && dcGraphData_getType(_right) == NODE_GRAPH_DATA_TREE)
    {
        dcGraphDataTree *leftTree = CAST_GRAPH_DATA_TREE(_left);
        dcGraphDataTree *rightTree = CAST_GRAPH_DATA_TREE(_right);
        dcNode *left = leftTree->head;
        dcNode *right = rightTree->head;

        result = TAFFY_SUCCESS;

        if (leftTree->size < rightTree->size)
        {
            *_compareResult = TAFFY_LESS_THAN;
        }
        else if (leftTree->size > rightTree->size)
        {
            *_compareResult = TAFFY_GREATER_THAN;
        }
        else
        {
            *_compareResult = TAFFY_EQUALS;

            while (left != NULL
                   && right != NULL
                   && result == TAFFY_SUCCESS
                   && *_compareResult == TAFFY_EQUALS)
            {
                result = dcNode_compare(left, right, _compareResult);
                left = dcGraphData_getNext(left);
                right = dcGraphData_getNext(right);
            }
        }
    }

    return result;
}

bool dcGraphDataTree_unmarshallNode(dcNode *_node, dcString *_stream)
{
    uint32_t size;
    bool result = false;
    dcNode *head = NULL;

    if (dcMarshaller_unmarshall(_stream, "w", &size))
    {
        result = true;
        uint32_t i;
        dcNode *that;

        if (! dcMarshaller_unmarshallNoNull(_stream, "g", &that))
        {
            // FAILURE //
            result = false;
        }
        else
        {
            dcNode_setTemplate(that, true);
            head = that;

            if (that != NULL)
            {
                for (i = 1; i < size; i++)
                {
                    dcNode *next;

                    if (! dcMarshaller_unmarshall(_stream, "g", &next))
                    {
                        // FAILURE //
                        result = false;
                        break;
                    }
                    else
                    {
                        dcNode_setTemplate(next, true);
                        dcGraphData_setNext(that, next);
                        that = next;
                    }
                }
            }
            else
            {
                // FAILURE //
                result = false;
            }
        }
    }

    if (result)
    {
        CAST_GRAPH_DATA_TREE(_node) = dcGraphDataTree_create(head);
    }
    else
    {
        dcGraphData_freeTree(&head, DC_DEEP);
    }

    return result;
}

dcString *dcGraphDataTree_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcGraphDataTree *tree = CAST_GRAPH_DATA_TREE(_node);
    uint32_t i;
    dcNode *that = tree->head;

    _stream = dcMarshaller_marshall(_stream, "w", tree->size);

    for (i = 0; i < tree->size; i++)
    {
        dcNode_marshall(that, _stream);
        that = dcGraphData_getNext(that);
    }

    return _stream;
}

bool dcGraphDataTree_isMe(const dcNode *_node)
{
    return (IS_GRAPH_DATA(_node)
            && dcGraphData_isType(_node, NODE_GRAPH_DATA_TREE));
}
