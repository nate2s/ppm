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

#ifndef __DC_TREE_H__
#define __DC_TREE_H__

#include "dcDefines.h"

#define TREE_ELEMENT_VALUE 0
#define TREE_ELEMENT_VALUES 1

struct dcTreeElement_t
{
    union
    {
        struct dcNode_t *value;
        struct dcList_t *values;
    } types;

    uint8_t type;
    struct dcTreeElement_t *left;
    struct dcTreeElement_t *right;

    // may or may not be used..
    struct dcTreeElement_t *parent;
    uint32_t branchSizes[2];
};

typedef struct dcTreeElement_t dcTreeElement;

dcTreeElement *dcTreeElement_create(struct dcNode_t *_value);
struct dcNode_t *dcTreeElement_createShell(dcTreeElement *_element);

void dcTreeElement_setChildren(dcTreeElement *_element,
                               dcTreeElement *_left,
                               dcTreeElement *_right);

void dcTreeElement_setChild(dcTreeElement *_parent,
                            dcDirection _direction,
                            dcTreeElement *_child);

void dcTreeElement_updateChildParent(dcTreeElement *_parent);
void dcTreeElement_updateBabysDaddy(dcTreeElement *_parent);

void dcTreeElement_free(dcTreeElement **_element, dcDepth _depth);

void dcTreeElement_updateBranchSizes(dcTreeElement *_element);

void dcTreeElement_copyValues(dcTreeElement *_first,
                              const dcTreeElement *_second);

size_t dcTreeElement_getValueCount(const dcTreeElement *_element);
struct dcNode_t *dcTreeElement_getValue(const dcTreeElement *_element);

int dcTreeElement_getChildIndex(const dcTreeElement *_parent,
                                const dcTreeElement *_child);

struct dcNode_t *dcTreeElement_popValue(dcTreeElement *_element,
                                        dcDepth _depth);

struct dcNode_t *dcTreeElement_flatten(dcTreeElement *_element);

struct dcNode_t *dcTreeElement_tryFlatten(dcTreeElement *_element);
struct dcNode_t *dcTreeElement_flatten(dcTreeElement *_element);

bool dcTreeElement_removeValue(dcTreeElement *_element,
                               struct dcNode_t *_value,
                               dcDepth _depth);

void dcTreeElement_addValue(dcTreeElement *_element, struct dcNode_t *_value);

bool dcTreeElement_contains(const dcTreeElement *_element,
                            const struct dcNode_t *_value);

enum dcTreeType_e
{
    BINARY_SEARCH_TREE = 1,
    AVL_TREE = 2,
    LAST_TREE_TYPE = 3
};

typedef uint8_t dcTreeType;

struct dcTree_t
{
    dcTreeElement *head;
    size_t size;
    dcTreeType type;
};

typedef struct dcTree_t dcTree;

dcTree *dcTree_create(uint8_t _type);
void dcTree_free(dcTree **_tree, dcDepth _depth);
char *dcTree_display(const dcTree *_tree);
dcTreeElement *dcTree_insert(dcTree *_tree, struct dcNode_t *_value);

bool dcTree_delete(dcTree *_tree,
                   struct dcNode_t *_value,
                   bool _pointerMatch,
                   dcDepth _depth);

bool dcTree_pop(dcTree *_tree, dcDepth _depth);
bool dcTree_verifyBranchSizes(const dcTree *_tree);

bool dcTree_find(const dcTree *_tree,
                 struct dcNode_t *_value,
                 bool _pointerMatch);

dcTaffyOperator dcTree_compareSizes(const dcTree *_left, const dcTree *_right);

#endif
