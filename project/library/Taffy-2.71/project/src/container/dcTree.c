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

#include "dcAVLTree.h"
#include "dcBinarySearchTree.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"
#include "dcSystem.h"
#include "dcTree.h"

// for memset
#include <string.h>

dcTreeElement *dcTreeElement_create(dcNode *_value)
{
    dcTreeElement *element =
        (dcTreeElement *)dcMemory_allocate(sizeof(dcTreeElement));
    memset(element, 0, sizeof(dcTreeElement));
    element->types.value = _value;
    return element;
}

dcNode *dcTreeElement_createShell(dcTreeElement *_element)
{
    dcNode *node = dcNode_create(NODE_TREE_ELEMENT);
    CAST_TREEELEMENT(node) = _element;
    return node;
}

void dcTreeElement_setChildren(dcTreeElement *_element,
                               dcTreeElement *_left,
                               dcTreeElement *_right)
{
    _element->left = _left;
    _left->parent = _element;
    _element->right = _right;
    _right->parent = _element;
}

void dcTreeElement_updateBranchSizes(dcTreeElement *_element)
{
    memset(_element->branchSizes, 0, sizeof(_element->branchSizes));

    if (_element->left != NULL)
    {
        _element->branchSizes[TAFFY_LEFT] =
            _element->left->branchSizes[TAFFY_LEFT] +
            _element->left->branchSizes[TAFFY_RIGHT] +
            1;
    }

    if (_element->right != NULL)
    {
        _element->branchSizes[TAFFY_RIGHT] =
            _element->right->branchSizes[TAFFY_LEFT] +
            _element->right->branchSizes[TAFFY_RIGHT] +
            1;
    }
}

void dcTreeElement_setChild(dcTreeElement *_parent,
                            dcDirection _direction,
                            dcTreeElement *_child)
{
    if (_direction == TAFFY_LEFT)
    {
        _parent->left = _child;
    }
    else if (_direction == TAFFY_RIGHT)
    {
        _parent->right = _child;
    }
    else
    {
        assert(false);
    }

    _child->parent = _parent;
    dcTreeElement_updateBranchSizes(_parent);
}

void dcTreeElement_updateBabysDaddy(dcTreeElement *_parent)
{
    dcTreeElement_updateChildParent(_parent);
}

void dcTreeElement_updateChildParent(dcTreeElement *_parent)
{
    if (_parent->left != NULL)
    {
        _parent->left->parent = _parent;
    }

    if (_parent->right != NULL)
    {
        _parent->right->parent = _parent;
    }
}

void dcTreeElement_free(dcTreeElement **_element, dcDepth _depth)
{
    dcTreeElement *element = *_element;

    if (element->type == TREE_ELEMENT_VALUE)
    {
        dcNode_tryFree(&(element->types.value), _depth);
    }
    else if (element->type == TREE_ELEMENT_VALUES)
    {
        if (_depth != DC_FLOATING)
        {
            dcList_free(&(element->types.values), _depth);
        }
    }

    dcMemory_free(element);
}

dcNode *dcTreeElement_popValue(dcTreeElement *_element, dcDepth _depth)
{
    return dcList_pop(_element->types.values, _depth);
}

dcNode *dcTreeElement_getValue(const dcTreeElement *_element)
{
    if (_element->type == TREE_ELEMENT_VALUE)
    {
        return _element->types.value;
    }
    else if (_element->type == TREE_ELEMENT_VALUES)
    {
        return dcList_getHead(_element->types.values);
    }
    else
    {
        assert(false);
    }

    // avoid compiler warning //
    return NULL;
}

void dcTreeElement_addValue(dcTreeElement *_element, dcNode *_value)
{
    if (_element->type == TREE_ELEMENT_VALUE)
    {
        dcList *list = dcList_create();
        dcList_push(list, _element->types.value);
        dcList_push(list, _value);
        _element->types.values = list;
        _element->type = TREE_ELEMENT_VALUES;
    }
    else if (_element->type == TREE_ELEMENT_VALUES)
    {
        dcList_push(_element->types.values, _value);
    }
    else
    {
        assert(false);
    }
}

bool dcTreeElement_contains(const dcTreeElement *_element, const dcNode *_value)
{
    if (_element->type == TREE_ELEMENT_VALUE &&
        _element->types.value == _value)
    {
        return true;
    }
    else if (_element->type == TREE_ELEMENT_VALUES)
    {
        return dcList_contains(_element->types.values, _value);
    }

    return false;
}

size_t dcTreeElement_getValueCount(const dcTreeElement *_element)
{
    if (_element->type == TREE_ELEMENT_VALUE)
    {
        return 1;
    }
    else
    {
        return _element->types.values->size;
    }
}

dcNode *dcTreeElement_tryFlatten(dcTreeElement *_element)
{
    if (_element->type == TREE_ELEMENT_VALUES &&
        _element->types.values->size == 1)
    {
        return dcTreeElement_flatten(_element);
    }

    return NULL;
}

dcNode *dcTreeElement_flatten(dcTreeElement *_element)
{
    assert(_element->type == TREE_ELEMENT_VALUES);
    assert(_element->types.values->size == 1);

    dcNode *node = dcList_getHead(_element->types.values);
    dcList_free(&(_element->types.values), DC_FLOATING);
    _element->types.value = node;
    _element->type = TREE_ELEMENT_VALUE;

    return node;
}

bool dcTreeElement_removeValue(dcTreeElement *_element,
                               dcNode *_value,
                               dcDepth _depth)
{
    assert(_element->type == TREE_ELEMENT_VALUES);
    bool retval = dcList_remove(_element->types.values, _value, _depth);
    assert(_element->types.values->size >= 1);

    if (_element->types.values->size == 1)
    {
        dcTreeElement_flatten(_element);
    }

    return retval;
}

void dcTreeElement_copyValues(dcTreeElement *_first,
                              const dcTreeElement *_second)
{
    if (_second->type == TREE_ELEMENT_VALUE)
    {
        _first->types.value = _second->types.value;
    }
    else if (_second->type == TREE_ELEMENT_VALUES)
    {
        _first->types.values = _second->types.values;
        _first->type = TREE_ELEMENT_VALUES;
    }
}

int dcTreeElement_getChildIndex(const dcTreeElement *_parent,
                                const dcTreeElement *_child)
{
    int retval = TAFFY_UNINITIALIZED_DIRECTION;

    if (_parent)
    {
        if (_parent->left == _child)
        {
            retval = TAFFY_LEFT;
        }
        else if (_parent->right == _child)
        {
            retval = TAFFY_RIGHT;
        }
    }

    return retval;
}

dcTree *dcTree_create(uint8_t _type)
{
    dcTree *tree = (dcTree *)dcMemory_allocateAndInitialize(sizeof(dcTree));
    assert(_type < LAST_TREE_TYPE);
    tree->type = _type;
    return tree;
}

void dcTree_free(dcTree **_tree, dcDepth _depth)
{
    while (dcTree_pop(*_tree, _depth))
    {
    }

    dcMemory_free((*_tree));
}

typedef dcTreeElement *(*dcTreeInsertFunction)(dcTree *_tree, dcNode *_value);

static const dcTreeInsertFunction __insertFunctions[] =
{
    &dcBinarySearchTree_insert,
    &dcAVLTree_insert,
    NULL
};

dcTreeElement *dcTree_insert(dcTree *_tree, dcNode *_value)
{
    if (_tree->head == NULL)
    {
        _tree->head = dcTreeElement_create(_value);
        _tree->size = 1;
        return _tree->head;
    }
    else
    {
        return (*__insertFunctions[_tree->type])(_tree, _value);
    }
}

bool dcTree_pop(dcTree *_tree, dcDepth _depth)
{
    if (_tree->head != NULL)
    {
        dcNode *value = dcTreeElement_getValue(_tree->head);
        dcTree_delete(_tree, value, false, _depth);
        return true;
    }

    return false;
}

typedef bool (*dcTreeDeleteFunction)(dcTree *_tree,
                                     dcNode *_value,
                                     bool _pointerMatch,
                                     dcDepth _depth);

static const dcTreeDeleteFunction __deleteFunctions[] =
{
    &dcBinarySearchTree_delete,
    &dcAVLTree_delete,
    NULL
};

bool dcTree_delete(dcTree *_tree,
                   dcNode *_value,
                   bool _pointerMatch,
                   dcDepth _depth)
{
    return (*__deleteFunctions[_tree->type])(_tree,
                                             _value,
                                             _pointerMatch,
                                             _depth);
}

typedef bool (*dcTreeFindFunction)(const dcTree *_tree,
                                   dcNode *_value,
                                   bool _pointerMatch);

static const dcTreeFindFunction __findFunctions[] =
{
    &dcBinarySearchTree_find,
    &dcAVLTree_find,
    NULL
};

bool dcTree_find(const dcTree *_tree,
                 dcNode *_value,
                 bool _pointerMatch)
{
    return (*__findFunctions[_tree->type])(_tree,
                                           _value,
                                           _pointerMatch);
}

static int dcTree_getRealBranchSize(const dcTreeElement *_element)
{
    if (_element == NULL)
    {
        return 0;
    }

    return (1 +
            dcTree_getRealBranchSize(_element->left) +
            dcTree_getRealBranchSize(_element->right));
}

static void dcTree_verifyBranchSizes_helper(const dcTreeElement *_element,
                                            bool *_retval)
{
    if (_element == NULL)
    {
        return;
    }

    if (_element->parent)
    {
        assert(_element->parent->left == _element ||
               _element->parent->right == _element);
    }

    if (_element->left)
    {
        assert(_element->left->parent == _element);
    }

    if (_element->right)
    {
        assert(_element->right->parent == _element);
    }

    size_t leftBranchSize = _element->branchSizes[TAFFY_LEFT];
    size_t rightBranchSize = _element->branchSizes[TAFFY_RIGHT];

    size_t realLeftBranchSize = dcTree_getRealBranchSize(_element->left);

    if (leftBranchSize != realLeftBranchSize)
    {
        *_retval = false;
        printf("LEFT VERIFY FAILURE FOR ELEMENT (%zu):\n",
               realLeftBranchSize);
    }

    size_t realRightBranchSize = dcTree_getRealBranchSize(_element->right);

    if (rightBranchSize != realRightBranchSize)
    {
        *_retval = false;
        printf("RIGHT VERIFY FAILURE FOR ELEMENT (%zu):\n",
               realRightBranchSize);
    }

    dcTree_verifyBranchSizes_helper(_element->left, _retval);
    dcTree_verifyBranchSizes_helper(_element->right, _retval);
}

bool dcTree_verifyBranchSizes(const dcTree *_tree)
{
    bool result = true;
    dcTree_verifyBranchSizes_helper(_tree->head, &result);

    if (!result)
    {
        printf("falseT VERIFIED\n");
    }

    return result;
}

dcTaffyOperator dcTree_compareSizes(const dcTree *_left, const dcTree *_right)
{
    return (_left->size < _right->size
            ? TAFFY_LESS_THAN
            : (_left->size > _right->size
               ? TAFFY_GREATER_THAN
               : TAFFY_EQUALS));
}
