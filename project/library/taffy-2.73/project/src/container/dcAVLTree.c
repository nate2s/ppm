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

#include "dcBinarySearchTree.h"
#include "dcError.h"
#include "dcList.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcTree.h"

static int32_t getBalanceFactor(dcTreeElement *_element)
{
    return (_element->branchSizes[TAFFY_LEFT]
            - _element->branchSizes[TAFFY_RIGHT]);
}

static void dcAVLTree_updateHeadForRotation(dcTree *_tree,
                                            dcTreeElement *_first,
                                            dcTreeElement *_second)
{
    if (_tree->head == _first)
    {
        _tree->head = _second;
        _tree->head->parent = NULL;
    }
    else
    {
        if (_tree->head->right == _first)
        {
            dcTreeElement_setChild(_tree->head, TAFFY_RIGHT, _second);
        }
        else if (_tree->head->left == _first)
        {
            dcTreeElement_setChild(_tree->head, TAFFY_LEFT, _second);
        }
    }
}

static void dcAVLTree_updateRelationshipsForRotation(dcTreeElement *_first,
                                                     dcTreeElement *_second)
{
    int childDirection = dcTreeElement_getChildIndex(_first->parent, _first);

    if (childDirection != TAFFY_UNINITIALIZED_DIRECTION)
    {
        dcTreeElement_setChild(_first->parent,
                               (dcDirection)childDirection,
                               _second);
    }

    _first->parent = _second;

    dcTreeElement_updateBabysDaddy(_first);
    dcTreeElement_updateBabysDaddy(_second);
}

static void dcAVLTree_rotationWrapup(dcTree *_tree,
                                     dcTreeElement *_first,
                                     dcTreeElement *_second)
{
    dcAVLTree_updateRelationshipsForRotation(_first, _second);
    dcTreeElement_updateBranchSizes(_first);
    dcTreeElement_updateBranchSizes(_second);
}

static void dcAVLTree_rotateLeft(dcTree *_tree,
                                 dcTreeElement *_first,
                                 dcTreeElement *_second)
{
    dcAVLTree_updateHeadForRotation(_tree, _first, _second);

    _first->right = _second->left;
    _second->left = _first;

    dcAVLTree_rotationWrapup(_tree, _first, _second);
}

static void dcAVLTree_rotateRight(dcTree *_tree,
                                  dcTreeElement *_first,
                                  dcTreeElement *_second)
{
    dcAVLTree_updateHeadForRotation(_tree, _first, _second);

    _first->left = _second->right;
    _second->right = _first;

    dcAVLTree_rotationWrapup(_tree, _first, _second);
}

static void dcAVLTree_balance(dcTree *_tree, dcTreeElement *_element)
{
    int balanceFactor = getBalanceFactor(_element);

    if (balanceFactor <= -2)
    {
        dcTreeElement *right = _element->right;
        int32_t rightBalanceFactor = getBalanceFactor(right);

        if (rightBalanceFactor <= 0)
        {
            dcAVLTree_rotateLeft(_tree, _element, right);
        }
        else
        {
            dcTreeElement *left = right->left;
            dcError_assert(left != NULL);
            dcAVLTree_rotateRight(_tree, right, left);
            dcAVLTree_rotateLeft(_tree, _element, left);
        }
    }
    else if (balanceFactor >= 2)
    {
        dcTreeElement *left = _element->left;
        int32_t leftBalanceFactor = getBalanceFactor(left);

        if (leftBalanceFactor <= 0)
        {
            dcTreeElement *right = left->right;
            dcError_assert(right != NULL);
            dcAVLTree_rotateLeft(_tree, left, right);
            dcAVLTree_rotateRight(_tree, _element, right);
        }
        else
        {
            dcAVLTree_rotateRight(_tree, _element, left);
        }
    }

    dcTreeElement_updateBranchSizes(_element);
}

static void dcAVLTree_balanceParents(dcTree *_tree, dcTreeElement *_element)
{
    while (_element != NULL)
    {
        dcTreeElement *parent = _element->parent;
        dcAVLTree_balance(_tree, _element);
        _element = parent;
    }

    if (_tree->head != NULL)
    {
        dcAVLTree_balance(_tree, _tree->head);
    }
}

dcTreeElement *dcAVLTree_insert(dcTree *_tree, dcNode *_node)
{
    dcTreeElement *element = dcBinarySearchTree_insert(_tree, _node);
    dcAVLTree_balanceParents(_tree, element);
    return element;
}

bool dcAVLTree_delete(dcTree *_tree,
                      dcNode *_value,
                      bool _pointerMatch,
                      dcDepth _depth)
{
    dcTreeElement *parent = NULL;
    bool retval = dcBinarySearchTree_deleteWithParent(_tree,
                                                      _value,
                                                      _pointerMatch,
                                                      &parent,
                                                      _depth);
    if (retval)
    {
        dcAVLTree_balanceParents(_tree, parent);
    }

    return retval;
}

bool dcAVLTree_find(const dcTree *_tree,
                    dcNode *_value,
                    bool _pointerMatch)
{
    return dcBinarySearchTree_find(_tree, _value, _pointerMatch);
}
