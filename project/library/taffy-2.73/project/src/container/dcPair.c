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

#include "dcPair.h"
#include "dcGraphData.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcNode *dcPair_createNode(dcNode *_left, dcNode *_right)
{
    return dcNode_createWithGuts(NODE_PAIR, dcPair_create(_left, _right));
}

dcPair *dcPair_create(dcNode *_left, dcNode *_right)
{
    dcPair *pair = (dcPair *)dcMemory_allocate(sizeof(dcPair));
    pair->left = _left;
    pair->right = _right;
    return pair;
}

dcNode *dcPair_createGraphDataNode(dcNode *_left, dcNode *_right)
{
    return dcGraphData_createNodeWithGuts
        (NODE_GRAPH_DATA_PAIR, dcPair_create(_left, _right));
}

void dcPair_free(dcPair **_pair, dcDepth _depth)
{
    if (*_pair != NULL)
    {
        dcPair_clear(*_pair, _depth);
    }

    dcMemory_free(*_pair);
}

void dcPair_clear(dcPair *_pair, dcDepth _depth)
{
    dcPair_clearLeft(_pair, _depth);
    dcPair_clearRight(_pair, _depth);
}

void dcPair_clearLeft(dcPair *_pair, dcDepth _depth)
{
    if (_depth != DC_FLOATING)
    {
        dcNode_tryFree(&_pair->left, _depth);
    }

    _pair->left = NULL;
}

void dcPair_clearRight(dcPair *_pair, dcDepth _depth)
{
    if (_depth != DC_FLOATING)
    {
        dcNode_tryFree(&_pair->right, _depth);
    }

    _pair->right = NULL;
}

// create dcPair_freeNode //
dcTaffy_createFreeNodeFunctionMacro(dcPair, CAST_PAIR);

// create dcPair_copyNode //
dcTaffy_createCopyNodeFunctionMacro(dcPair, CAST_PAIR);

// create dcPair_markNode //
dcTaffy_createMarkNodeFunctionMacro(dcPair, CAST_PAIR);

// create dcPair_unmarshallNode
dcTaffy_createUnmarshallNodeFunctionMacro(dcPair, CAST_PAIR);

// create dcPair_marshallNode
dcTaffy_createMarshallNodeFunctionMacro(dcPair, CAST_PAIR);

// create dcPair_printNode
dcTaffy_createPrintNodeFunctionMacro(dcPair, CAST_PAIR);

// create dcPair_display
dcTaffy_createDisplayFunctionMacro(dcPair);

void dcPair_freeGraphDataNode(dcNode *_node, dcDepth _depth)
{
    dcPair_free(&(CAST_GRAPH_DATA_PAIR(_node)), _depth);
}

dcPair *dcPair_copy(const dcPair *_from, dcDepth _depth)
{
    dcNode *leftCopy = dcNode_tryCopy(_from->left, _depth);
    dcNode *rightCopy = dcNode_tryCopy(_from->right, _depth);
    dcPair *pairCopy = dcPair_create(leftCopy, rightCopy);
    return pairCopy;
}

dcNode *dcPair_getLeft(const dcNode *_pair)
{
    return CAST_PAIR(_pair)->left;
}

dcNode *dcPair_getRight(const dcNode *_pair)
{
    return CAST_PAIR(_pair)->right;
}

void dcPair_setLeft(dcPair *_pair, dcNode *_left)
{
    _pair->left = _left;
}

void dcPair_setRight(dcPair *_pair, dcNode *_right)
{
    _pair->right = _right;
}

void dcPair_set(dcPair *_pair, dcNode *_left, dcNode *_right)
{
    _pair->left = _left;
    _pair->right = _right;
}

void dcPair_copyGraphDataNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_GRAPH_DATA_PAIR(_to) =
        dcPair_copy(CAST_GRAPH_DATA_PAIR(_from), _depth);
}

void dcPair_copyGraphDataTree(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_GRAPH_DATA_PAIR(_to) =
        dcPair_create
        (dcGraphData_copyTree(CAST_GRAPH_DATA_PAIR(_from)->left),
         dcGraphData_copyTree(CAST_GRAPH_DATA_PAIR(_from)->right));
}

void dcPair_mark(dcPair *_pair)
{
    dcNode_mark(_pair->left);
    dcNode_mark(_pair->right);
}

void dcPair_markGraphDataNode(dcNode *_pairNode)
{
    dcPair_mark(CAST_GRAPH_DATA_PAIR(_pairNode));
}

void dcPair_registerNode(dcNode *_pair)
{
    dcNode_register(dcPair_getLeft(_pair));
    dcNode_register(dcPair_getRight(_pair));
}

dcPair *dcPair_unmarshall(dcString *_stream)
{
    dcPair *result = NULL;
    dcNode *left;
    dcNode *right;
    uint8_t type;

    if (dcString_getLengthLeft(_stream) > 0
        && dcString_peek(_stream) != 0xFF
        && dcMarshaller_unmarshallNoNull(_stream, "u", &type)
        && type == NODE_PAIR
        && dcMarshaller_unmarshallNoNull(_stream, "tt", &left, &right))
    {
        result = dcPair_create(left, right);
    }

    return result;
}

bool dcPair_unmarshallGraphDataNode(dcNode *_node, dcString *_stream)
{
    CAST_GRAPH_DATA_PAIR(_node) = dcPair_unmarshall(_stream);
    return (CAST_GRAPH_DATA_PAIR(_node) != NULL);
}

dcString *dcPair_marshall(const dcPair *_pair, dcString *_stream)
{
    if (_pair == NULL)
    {
        return dcMarshaller_marshall(_stream, "N");
    }
    else
    {
        return dcMarshaller_marshall(_stream,
                                     "ctt",
                                     NODE_PAIR,
                                     _pair->left,
                                     _pair->right);
    }
}

dcString *dcPair_marshallGraphDataNode(const dcNode *_node, dcString *_stream)
{
    return dcPair_marshall(CAST_GRAPH_DATA_PAIR(_node), _stream);
}

bool dcPair_unmarshallGraphDataTree(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcNode *left;
    dcNode *right;

    if (dcMarshaller_unmarshallNoNull(_stream, "tt", &left, &right))
    {
        CAST_GRAPH_DATA_PAIR(_node) = dcPair_create(left, right);
        result = true;
    }

    return result;
}

dcString *dcPair_marshallGraphDataTree(const dcNode *_pair, dcString *_stream)
{
    dcPair *pair = CAST_GRAPH_DATA_PAIR(_pair);
    return dcMarshaller_marshall(_stream, "tt", pair->left, pair->right);
}

dcResult dcPair_print(const dcPair *_pair, dcString *_stream)
{
    dcString_appendString(_stream, "#Pair(");
    dcResult result = dcNode_print(_pair->left, _stream);

    if (result == TAFFY_SUCCESS)
    {
        dcString_appendString(_stream, ", ");
        result = dcNode_print(_pair->right, _stream);
    }

    return result;
}

dcResult dcPair_compareNode(dcNode *_left,
                            dcNode *_right,
                            dcTaffyOperator *_compareResult)
{
    dcPair *left = CAST_PAIR(_left);
    dcPair *right = CAST_PAIR(_right);
    dcResult result = TAFFY_FAILURE;

    if ((dcNode_compare(left->left, right->left, _compareResult)
         == TAFFY_SUCCESS)
        && *_compareResult == TAFFY_EQUALS
        && (dcNode_compare(left->right, right->right, _compareResult)
            == TAFFY_SUCCESS)
        && *_compareResult == TAFFY_EQUALS)
    {
        result = TAFFY_SUCCESS;
        *_compareResult = TAFFY_EQUALS;
    }

    return result;
}
