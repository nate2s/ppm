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

#ifndef __DC_AVL_TREE_H__
#define __DC_AVL_TREE_H__

#include "dcDefines.h"

struct dcTreeElement_t *dcAVLTree_insert(struct dcTree_t *_tree,
                                         struct dcNode_t *_node);

bool dcAVLTree_delete(struct dcTree_t *_tree,
                        struct dcNode_t *_value,
                        bool _pointerMatch,
                        dcDepth _depth);

bool dcAVLTree_find(const struct dcTree_t *_tree,
                      struct dcNode_t *_value,
                      bool _pointerMatch);

#endif
