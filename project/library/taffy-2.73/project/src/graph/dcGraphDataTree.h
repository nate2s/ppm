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

#ifndef __DC_GRAPH_DATA_TREE_H__
#define __DC_GRAPH_DATA_TREE_H__

#include "dcDefines.h"

struct dcGraphDataTree_t
{
    struct dcNode_t *head;
    uint32_t size;
};

typedef struct dcGraphDataTree_t dcGraphDataTree;

/*
 * @brief Creates a graph data tree node
 */
struct dcNode_t *dcGraphDataTree_createNode(struct dcNode_t *_head);

/*
 * @brief Creates a graph data tree
 */
dcGraphDataTree *dcGraphDataTree_create(struct dcNode_t *_head);

/*
 * @brief Get the contents of the tree
 */
struct dcNode_t *dcGraphDataTree_getContents(const struct dcNode_t *_tree);

/*
 * @brief Get the tree size
 */
uint32_t dcGraphDataTree_getSize(const struct dcNode_t *_node);

/*
 * @brief Returns true if _node is a graph data tree
 */
bool dcGraphDataTree_isMe(const struct dcNode_t *_node);

COMPARE_FUNCTION(dcGraphDataTree_compareNode);
COPY_FUNCTION(dcGraphDataTree_copyNode);
FREE_FUNCTION(dcGraphDataTree_freeNode);
PRETTY_PRINT_FUNCTION(dcGraphDataTree_prettyPrintNode);
PRINT_FUNCTION(dcGraphDataTree_printNode);
MARSHALL_FUNCTION(dcGraphDataTree_marshallNode);
UNMARSHALL_FUNCTION(dcGraphDataTree_unmarshallNode);

#endif
