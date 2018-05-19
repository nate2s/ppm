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
#include "dcUnsignedInt32.h"
#include "dcList.h"
#include "dcNode.h"
#include "dcPair.h"
#include "dcSystem.h"
#include "dcTree.h"

bool dcBinarySearchTree_delete(dcTree *_tree,
                               dcNode *_value,
                               bool _pointerMatch,
                               dcDepth _depth)
{
    dcTreeElement *parent = NULL;
    return dcBinarySearchTree_deleteWithParent(_tree,
                                               _value,
                                               _pointerMatch,
                                               &parent,
                                               _depth);
}

bool dcBinarySearchTree_deleteWithParent(dcTree *_tree,
                                         dcNode *_value,
                                         bool _pointerMatch,
                                         dcTreeElement **_parent,
                                         dcDepth _depth)
{
    dcTreeElement *iterator = _tree->head;
    dcTreeElement *previous = iterator;
    bool deleted = false;
    bool trulyDeleted = true;
    dcTreeElement **directionElement = NULL;
    dcList *leftPotentials = dcList_create();
    dcList *rightPotentials = dcList_create();

    *_parent = NULL;

    while (!deleted && iterator != NULL)
    {
        dcNode *value = dcTreeElement_getValue(iterator);
        dcTaffyOperator compareResult;
        dcResult result = dcNode_compare(_value, value, &compareResult);

        if (result == TAFFY_EXCEPTION
            || result == TAFFY_FAILURE)
        {
            break;
        }

        dcTreeElement_tryFlatten(iterator);

        if ((dcTreeElement_getValueCount(iterator) > 1
             && compareResult == TAFFY_EQUALS
             && _pointerMatch
             && dcTreeElement_removeValue(iterator, _value, _depth)))
        {
            deleted = true;
            trulyDeleted = false;
        }
        else if ((_pointerMatch && (_value == value))
                 || (!_pointerMatch && compareResult == TAFFY_EQUALS))
        {
            // if we've actually incremented down the tree,
            // set the parent accordingly
            if (previous != iterator)
            {
                *_parent = previous;
            }

            if (iterator->left == NULL && iterator->right == NULL)
            {
                // leaf //

                if (directionElement != NULL)
                {
                    *directionElement = NULL;
                }

                dcTreeElement_free(&iterator, _depth);
            }
            else if (iterator->left != NULL && iterator->right == NULL)
            {
                // one child //

                if (directionElement == NULL)
                {
                    // we're at the root //
                    _tree->head = iterator->left;
                    _tree->head->parent = NULL;
                }
                else
                {
                    *directionElement = iterator->left;
                    dcTreeElement_updateBabysDaddy(*_parent);
                }

                dcTreeElement_free(&iterator, _depth);
            }
            else if (iterator->left == NULL && iterator->right != NULL)
            {
                // one child //

                if (directionElement == NULL)
                {
                    // we're at the root //
                    _tree->head = iterator->right;
                    _tree->head->parent = NULL;
                }
                else
                {
                    *directionElement = iterator->right;
                    dcTreeElement_updateBabysDaddy(*_parent);
                }

                dcTreeElement_free(&iterator, _depth);
            }
            else
            {
                // two children
                // which way to go?
                int leftRight = rand() % 2;
                int depth = 0;

                dcTreeElement *successor = NULL;
                dcTreeElement *previousSuccessor = NULL;

                if (leftRight == TAFFY_LEFT)
                {
                    // left. go on boy, get there in-order successor of
                    // the left tree!
                    dcList_push(leftPotentials,
                                dcTreeElement_createShell(iterator));
                    successor = iterator->left;
                    previousSuccessor = iterator;

                    while (successor->right != NULL)
                    {
                        dcList_push(rightPotentials,
                                    dcTreeElement_createShell(successor));
                        previousSuccessor = successor;
                        successor = successor->right;
                        depth++;
                    }
                }
                else
                {
                    // right. get the in-order predecessor of the right tree //
                    dcList_push(rightPotentials,
                                dcTreeElement_createShell(iterator));
                    successor = iterator->right;
                    previousSuccessor = iterator;

                    while (successor->left != NULL)
                    {
                        dcList_push(leftPotentials,
                                    dcTreeElement_createShell(successor));
                        previousSuccessor = successor;
                        successor = successor->left;
                        depth++;
                    }
                }

                // flip successor and iterator //
                dcError_assert(iterator->type == TREE_ELEMENT_VALUE);
                dcNode_tryFree(&(iterator->types.value), _depth);
                dcTreeElement_copyValues(iterator, successor);

                if (leftRight == TAFFY_LEFT)
                {
                    if (depth == 0)
                    {
                        iterator->left = successor->left;
                        dcTreeElement_updateBabysDaddy(iterator);
                    }
                    else
                    {
                        previousSuccessor->right = successor->left;
                        dcTreeElement_updateBabysDaddy(previousSuccessor);
                    }
                }
                else
                {
                    if (depth == 0)
                    {
                        iterator->right = successor->right;
                        dcTreeElement_updateBabysDaddy(iterator);
                    }
                    else
                    {
                        previousSuccessor->left = successor->right;
                        dcTreeElement_updateBabysDaddy(previousSuccessor);
                    }
                }

                //successor->parent = NULL;
                dcTreeElement_free(&successor, DC_FLOATING);
            }

            deleted = true;
        }
        else if ((_pointerMatch
                  && compareResult == TAFFY_EQUALS)
                 || compareResult == TAFFY_LESS_THAN)
        {
            dcList_push(leftPotentials, dcTreeElement_createShell(iterator));
            previous = iterator;
            directionElement = &(iterator->left);
            iterator = iterator->left;
        }
        else if (compareResult == TAFFY_GREATER_THAN)
        {
            dcList_push(rightPotentials, dcTreeElement_createShell(iterator));
            previous = iterator;
            directionElement = &(iterator->right);
            iterator = iterator->right;
        }
        else
        {
            dcError_assert(false);
        }
    }

    if (deleted)
    {
        _tree->size--;

        if (_tree->size == 0)
        {
            _tree->head = NULL;
        }

        if (trulyDeleted)
        {
            dcNode *node = dcList_pop(leftPotentials, DC_FLOATING);

            while (node != NULL)
            {
                dcTreeElement *element = CAST_TREEELEMENT(node);
                dcError_assert(element->branchSizes[TAFFY_LEFT] > 0);
                element->branchSizes[TAFFY_LEFT]--;
                dcNode_freeShell(&node);
                node = dcList_pop(leftPotentials, DC_FLOATING);
            }

            node = dcList_pop(rightPotentials, DC_FLOATING);

            while (node != NULL)
            {
                dcTreeElement *element = CAST_TREEELEMENT(node);
                dcError_assert(element->branchSizes[TAFFY_RIGHT] > 0);
                element->branchSizes[TAFFY_RIGHT]--;
                dcNode_freeShell(&node);
                node = dcList_pop(rightPotentials, DC_FLOATING);
            }
        }
    }

    dcError_assert(dcTree_verifyBranchSizes(_tree));

    dcNode *node = dcList_pop(leftPotentials, DC_FLOATING);

    while (node != NULL)
    {
        dcNode_freeShell(&node);
        node = dcList_pop(leftPotentials, DC_FLOATING);
    }

    node = dcList_pop(rightPotentials, DC_FLOATING);

    while (node != NULL)
    {
        dcNode_freeShell(&node);
        node = dcList_pop(rightPotentials, DC_FLOATING);
    }

    dcList_free(&leftPotentials, DC_SHALLOW);
    dcList_free(&rightPotentials, DC_SHALLOW);

    return deleted;
}

bool dcBinarySearchTree_find(const dcTree *_tree,
                             dcNode *_value,
                             bool _pointerMatch)
{
    dcTreeElement *iterator = _tree->head;
    bool found = false;

    while (!found && iterator != NULL)
    {
        dcNode *node = dcTreeElement_getValue(iterator);
        dcTaffyOperator compareResult;
        dcResult result = dcNode_compare(_value, node, &compareResult);

        if (result == TAFFY_EXCEPTION
            || result == TAFFY_FAILURE)
        {
            break;
        }

        bool contains = false;

        if (_pointerMatch)
        {
            contains = dcTreeElement_contains(iterator, _value);
        }

        if (_pointerMatch
            && compareResult == TAFFY_EQUALS
            && !contains)
        {
            break;
        }
        else if ((_pointerMatch
                  && contains)
                 || (!_pointerMatch
                     && compareResult == TAFFY_EQUALS))
        {
            found = true;
        }
        else if (compareResult == TAFFY_LESS_THAN)
        {
            iterator = iterator->left;
        }
        else if (compareResult == TAFFY_GREATER_THAN)
        {
            iterator = iterator->right;
        }
        else
        {
            dcError_assert(false);
        }
    }

    return found;
}

dcTreeElement *dcBinarySearchTree_insert(dcTree *_tree, dcNode *_value)
{
    dcTreeElement *iterator = _tree->head;
    bool inserted = false;
    dcList *potentials = dcList_create();
    bool incrementBranchSizes = true;

    while (!inserted)
    {
        dcNode *node = dcTreeElement_getValue(iterator);
        dcTaffyOperator compareResult;
        dcResult result = dcNode_compare(_value, node, &compareResult);

        if (result == TAFFY_EXCEPTION
            || result == TAFFY_FAILURE)
        {
            break;
        }
        else if (compareResult == TAFFY_EQUALS)
        {
            dcTreeElement_addValue(iterator, _value);
            inserted = true;

            if (dcTreeElement_getValueCount(iterator) > 1)
            {
                incrementBranchSizes = false;
            }
        }
        else if (compareResult == TAFFY_LESS_THAN)
        {
            dcList_push(potentials,
                        dcPair_createNode(dcTreeElement_createShell(iterator),
                                          dcUnsignedInt32_createNode(TAFFY_LEFT)));

            if (iterator->left == NULL)
            {
                // leaf //
                iterator->left = dcTreeElement_create(_value);
                iterator->left->parent = iterator;
                inserted = true;
            }

            iterator = iterator->left;
        }
        else
        {
            dcList_push(potentials,
                        dcPair_createNode
                        (dcTreeElement_createShell(iterator),
                         dcUnsignedInt32_createNode(TAFFY_RIGHT)));

            if (iterator->right == NULL)
            {
                // leaf //
                iterator->right = dcTreeElement_create(_value);
                iterator->right->parent = iterator;
                inserted = true;
            }

            iterator = iterator->right;
        }
    }

    _tree->size++;

    dcNode *pairNode = NULL;

    while ((pairNode = dcList_pop(potentials, DC_FLOATING)))
    {
        dcPair *pair = CAST_PAIR(pairNode);
        dcNode *elementNode = pair->left;
        dcNode *intNode = pair->right;

        if (incrementBranchSizes)
        {
            dcTreeElement *element = CAST_TREEELEMENT(elementNode);
            element->branchSizes[CAST_INT(intNode)]++;
        }

        dcNode_free(&pairNode, DC_FLOATING);
        dcNode_freeShell(&elementNode);
        dcNode_free(&intNode, DC_DEEP);
    }

    dcList_free(&potentials, DC_DEEP);

    return iterator;
}
