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

#ifndef __DC_LIST_H__
#define __DC_LIST_H__

#include "dcDefines.h"

#define FOR_EACH_IN_LIST(_list, _iterator)                              \
    dcListElement *_iterator = NULL;                                    \
    for (_iterator = _list->head;                                       \
         _iterator != NULL;                                             \
         _iterator = _iterator->next)

#define FOR_EACH_IN_LIST_REVERSE(_list, _iterator)                      \
    dcListElement *_iterator = NULL;                                    \
    for (_iterator = _list->tail;                                       \
         _iterator != NULL;                                             \
         _iterator = _iterator->previous)

///////////////////
// dcListElement //
///////////////////

/**
 * dcListS are composed of these
 */
struct dcListElement_t
{
    /**
     * The object
     */
    struct dcNode_t *object;

    /**
     * The next dcListElement
     */
    struct dcListElement_t *next;

    /**
     * The previous dcListElement
     */
    struct dcListElement_t *previous;
};

typedef struct dcListElement_t dcListElement;

//////////////
// creating //
//////////////

/**
 * Allocates a dcListElement
 * \return A newly allocated dcListElement
 */
dcListElement *dcListElement_create(struct dcNode_t *_what);

/////////////
// freeing //
/////////////

/**
 * Frees a dcListElement
 * \param _element The dcListElement to free
 * \param _list The dcList the dcListElement belongs to
 * \param _depth The depth of the free
 * \return The object/dcNode the dcListElement contained,
 * may be NULL, depending on _depth
 */
struct dcNode_t *dcListElement_free(dcListElement **_element,
                                    struct dcList_t *_list,
                                    dcDepth _depth);
/////////////
// copying //
/////////////

/**
 * Copies a dcListElement
 * \param _from The dcListElement to copy from
 * \param _depth The depth of the copy
 * \return A newly allocated dcListElement with elements copied from _from
 */
dcListElement *dcListElement_copy(const dcListElement *_from, dcDepth _depth);

/////////////
// setting //
/////////////

/**
 * Sets the object in a dcListElement
 * \param _element The dcListElement to set _object in
 * \param _object The dcNode object to set into _element
 */
void dcListElement_setObject(dcListElement *_element, struct dcNode_t *_object);

void dcListElement_replaceObject(dcListElement *_element,
                                 struct dcNode_t *_object,
                                 dcDepth _depth);

dcListElement *dcListElement_getNext(const dcListElement *_element);

////////////////////
// dcListIterator //
////////////////////

/**
 * A list iterator for dcList
 */
struct dcListIterator_t
{
    /**
     * The current dcListElement
     */
    dcListElement *element;
};

typedef struct dcListIterator_t dcListIterator;

//////////////
// creating //
//////////////

/**
 * Allocates a new dcListIterator
 * \param _element The current dcListElement
 * \return A newly allocated dcListIterator
 */
dcListIterator *dcListIterator_create(dcListElement *_element);

///////////////
// iterating //
///////////////

/**
 * Increments a dcListIterator, and returns its next object
 * \param _iterator A dcListIterator to increment
 * \return The next object, may be NULL
 */
struct dcNode_t *dcListIterator_getNext(dcListIterator *_iterator);

/**
 * Decrements a dcListIterator, and returns its previous object
 * \param _iterator A dcListIterator to decrement
 * \return The previous object, may be NULL
 */
struct dcNode_t *dcListIterator_getPrevious(dcListIterator *_iterator);

/**
 * Determines whether a dcListIterator can be incremented
 * \param _iterator The dcListIterator to test
 * \return Whether or not _iterator can be incremented
 */
bool dcListIterator_hasNext(dcListIterator *_iterator);

/**
 * Determines whether a dcListIterator can be decremented
 * \param _iterator The dcListIterator to test
 * \return Whether or not _iterator can be decremented
 */
bool dcListIterator_hasPrevious(dcListIterator *_iterator);

//////////////
// deleting //
//////////////

/**
 * Frees a dcListIterator
 * \param _iterator The dcListIterator to free
 */
void dcListIterator_free(dcListIterator **_iterator);

////////////
// dcList //
////////////

/**
 * A list container
 */
struct dcList_t
{
    /**
     * The head of the list
     */
    dcListElement *head;

    /**
     * The tail of the list
     */
    dcListElement *tail;

    /**
     * The list's size
     */
    dcContainerSizeType size;
};

typedef struct dcList_t dcList;

//////////////
// creating //
//////////////

/**
 * Allocates and returns a new dcList
 * \return A newly allocated dcList
 */
dcList *dcList_create(void);

/**
 * Allocates and returns a new dcList with given objects
 * \param _first The first dcNode to be inserted
 * \param ... The second, and beyond, dcNodeS to be inserted.
 *        ... must be NULL terminated
 * \return A newly allocated dcList stuffed with _first and ...
 */
dcList *dcList_createWithObjects(struct dcNode_t *_first, ...);

/**
 * Allocates a new dcList-node
 * \return A new dcList-node
 */
struct dcNode_t *dcList_createNode(void);

/**
 * Allocates a new dcNode(dcGraphData(dcList))
 * \return A new dcNode(dcGraphData(dcList))
 */
struct dcNode_t *dcList_createGraphDataNode(void);

/**
 * Allocates a new dcList-node with the given objects
 * \param _first The first dcNode to be inserted
 * \param ... The second, and beyond, dcNodeS to be inserted.
 *        ... must be NULL terminated
 * \return A new dcList-node
 */
struct dcNode_t *dcList_createNodeWithObjects(struct dcNode_t *_first, ...);

/**
 * Allocates a new dcNode(dcGraphData(dcList)) with the given objects
 * \param _first The first dcNode to be inserted
 * \param ... The second, and beyond, dcNodeS to be inserted. must
 * be NULL terminated
 * \return A new dcNode(dcGraphData(dcList))
 */
struct dcNode_t *dcList_createGraphDataNodeWithObjects(struct dcNode_t *_first,
                                                       ...);
/**
 * Creates a dcNode shell for a dcList
 * \param _list The dcList to create a shell for
 */
struct dcNode_t *dcList_createShell(dcList *_list);

/////////////////////////
// deleting & removing //
/////////////////////////

/**
 * Frees a dcList, and potentially its contents as well
 * \param _list The dcList to free
 * \param _depth The depth of the free
 */
void dcList_free(dcList **_list, dcDepth _depth);

void dcList_memoryRegionsFree(dcList *_list);

/**
 * Clears a dcList, and potentially frees its contents
 * \param _list The dcList to clear
 * \param _depth The depth of the clear
 */
void dcList_clear(dcList *_list,  dcDepth _depth);

////////////////
// displaying //
////////////////

/**
 * Returns the contents of a dcList
 * \param _list The dcList to display
 */
dcResult dcList_print(const dcList *_list, struct dcString_t *_stream);
dcResult dcList_printWithDelimiter(const dcList *_list,
                                   struct dcString_t *_stream,
                                   const char *_delimiter);
const char *dcList_display(const dcList *_list);

////////////
// adding //
////////////

/**
 * Inserts an element into the position before the given _element
 * If _element is NULL, then the element is inserted onto the tail of the list
 */
dcListElement *dcList_insertBeforeListElement(dcList *_list,
                                              dcListElement *_element,
                                              struct dcNode_t *_node);

/**
 * Pushes a dcNode onto the end of a dcList
 * \param _list The dcList which _node is pushed onto
 * \param _node The dcNode to push onto _list
 */
void dcList_push(dcList *_list, struct dcNode_t *_node);

void dcList_pushElement(dcList *_list, dcListElement *_element);


/**
 * Unshifts a dcNode onto a dcList
 * (inserts a dcNode onto the head of the dcList)
 * \param _list The dcList which _node is unshifted onto
 * \param _node The dcNode to unshift onto _list
 */
void dcList_unshift(dcList *_list, struct dcNode_t *_node);

/////////////
// getting //
/////////////

/**
 * Gets an object from a dcList
 * \param _list The dcList to query
 * \param _index The index of the object to get
 * \return The dcNode that is in the _index'th place in _list
 */
struct dcNode_t *dcList_get(const dcList *_list, dcContainerSizeType _index);

/**
 * Gets the tail of a dcList
 * \param _list The dcList to query
 * \return The tail of _list
 */
struct dcNode_t *dcList_getTail(const dcList *_list);

/**
 * Gets the head of a dcList
 * \param _list The dcList to query
 * \return The head of _list
 */
struct dcNode_t *dcList_getHead(const dcList *_list);

/**
 * Gets the neck of a dcList
 * \param _list The dcList to query
 * \return The neck of _list
 */
struct dcNode_t *dcList_getNeck(const dcList *_list);

dcListElement *dcList_getHeadElement(const dcList *_list);
dcListElement *dcList_getNeckElement(const dcList *_list);

/**
 * Gets the size of a dcList
 * \param _list The dcList to query
 * \return The size of _list
 */
dcContainerSizeType dcList_getSize(const struct dcNode_t *_list);

/////////////
// setting //
/////////////

/**
 * Sets the head of a dcList
 * \param _list The dcList to update
 * \param _node The dcNode to set as the head of _list
 */
void dcList_setHead(dcList *_list, struct dcNode_t *_node);

/**
 * Replaces a range of list elements with a node replacement
 */
dcListElement *dcList_replaceRange(dcList *_list,
                                   dcListElement *_start,
                                   dcListElement *_end,
                                   struct dcNode_t *_replacement,
                                   dcDepth _depth);

//////////////
// removing //
//////////////

/**
 * Pops the tail off a dcList
 * \param _list The dcList to update
 * \param _depth The depth of the pop
 * \return The dcNode that was popped, may be NULL,
 * depending on _depth, and the size of _list
 */
struct dcNode_t *dcList_pop(dcList *_list, dcDepth _depth);

/**
 * Performs a pop N times on a dcList
 * \param _list The dcList to pop
 * \param _times The number of pops to perform
 * \param _depth The depth of the pop
 */
void dcList_popNTimes(dcList *_list,
                      dcContainerSizeType _times,
                      dcDepth _depth);

/**
 * Shifts a list, or removes the head
 * \param _list The dcList to shift
 * \param _depth The depth of the shift
 * \return The shifted dcNode, may be NULL,
 * depending on _depth, and the size of _list
 */
struct dcNode_t *dcList_shift(dcList *_list, dcDepth _depth);

/**
 * Removes a dcNode from a dcList
 * \param _list The dcList to update
 * \param _node The dcNode to remove from _list
 * \param _depth The depth of the removal
 * \return Whether _node was removed successfully or not
 */
dcResult dcList_remove(dcList *_list, struct dcNode_t *_node, dcDepth _depth);

dcResult dcList_removeWithComparisonFunction
    (dcList *_list,
     struct dcNode_t *_node,
     dcDepth _depth,
     dcNode_comparePointer _comparisonFunction);

/**
 * Removes a dcListElement from a dcList
 * \param _list The dcList to update
 * \param _element The dcListElement to remove from _list
 * \param _depth The depth of the removal
 * \return The dcNode removed, may be NULL, depending on _depth
 */
struct dcNode_t *dcList_removeElement(dcList *_list,
                                      dcListElement **_element,
                                      dcDepth _depth);

///////////////
// concating //
///////////////

/**
 * Concats one dcList onto another
 * _list will share no list elements with _other
 * \param _list The dcList to concat onto
 * \param _other The dcList to concat from
 * \param _depth The depth of the concat. DC_DEEP means copy deeply,
 *               while anything else means copy shallowly
 */
void dcList_concat(dcList *_list,
                   const dcList *_other,
                   dcDepth _depth);

/**
 * Append one dcList onto another
 * This sets tail->next to (*_other)->head
 * _other is freed after the append is complete
 *
 * \param _list The list to append to
 * \param _other The list to append from
 */
void dcList_append(dcList *_list, dcList **_other);

//////////////
// morphing //
//////////////

/**
 * Creates and stuffs a dcArray with a dcList's contents
 * \param _list The template dcList
 * \return A newly allocated dcArray that contains the contents of _list
 */
struct dcArray_t *dcList_asArray(const dcList *_list);

/**
 * Creates and stuffs a dcList with the contents of a dcArray
 * \param _array The template dcArray
 * \return A newly allocated dcList that contains the contents of _array
 */
struct dcList_t *dcList_createFromArray(const struct dcArray_t *_array);

///////////////
// iterating //
///////////////

/**
 * Creates a dcListIterator that points to a dcList's head
 * \param _list The dcList to create the dcListIterator from
 * \return A newly allocated dcListIterator that points to a dcList's head
 */
dcListIterator *dcList_createHeadIterator(const dcList *_list);

void dcList_each(const dcList *_list,
                 dcContainerEachFunction _function,
                 void *_token);

typedef void (*CombinationFunction)(dcList *_left,
                                    dcList *_right,
                                    uint32_t _token);

void dcList_iterateCombinations(dcList *_list,
                                CombinationFunction _function,
                                uint32_t _token);

/**
 * Creates a dcListIterator that points to a dcList's tail
 * \param _list The dcList to create the dcListIterator from
 * \return A newly allocated dcListIterator that points to a dcList's tail
 */
dcListIterator *dcList_createTailIterator(const dcList *_list);

//////////////
// querying //
//////////////

/**
 * Determines whether a dcNode is included in a dcList
 * \param _list The dcList to query
 * \param _node The _node used to query _list
 * \return Whether _list contains _node
 */
bool dcList_contains(const dcList *_list, const struct dcNode_t *_node);

dcResult dcList_containsEqual(const dcList *_list,
                              struct dcNode_t *_node);

/////////////
// copying //
/////////////

/**
 * Copies a dcList
 * \param _from The dcList to copy
 * \param _depth The depth of the copy
 * \return A newly allocated dcList that contains copies of _from's objects
 */
dcList *dcList_copy(const dcList *_from, dcDepth _depth);

/////////////
// marking //
/////////////

/**
 * Marks a dcList, and potentially its contents, for garbage collection
 * \param _list The dcList to mark
 */
void dcList_mark(dcList *_list);

/////////////////
// registering //
/////////////////

/**
 * Potentially registers a dcList's contents for garbage collection
 * \param _list The dcList whose contents will be potentially registered
 */
void dcList_register(dcList *_list);

////////////////////////
// standard functions //
////////////////////////

COMPARE_FUNCTION(dcList_compareNode);
COPY_FUNCTION(dcList_copyNode);
COPY_FUNCTION(dcList_copyGraphDataNode);
FREE_FUNCTION(dcList_freeGraphDataNode);
FREE_FUNCTION(dcList_freeNode);
MARK_FUNCTION(dcList_markNode);
MARK_FUNCTION(dcList_markGraphDataNode);
PRINT_FUNCTION(dcList_printGraphDataNode);
PRINT_FUNCTION(dcList_printNode);
REGISTER_FUNCTION(dcList_registerNode);
MARSHALL_FUNCTION(dcList_marshallNode);
UNMARSHALL_FUNCTION(dcList_unmarshallNode);

/////////////////
// marshalling //
/////////////////

bool dcList_unmarshallGraphDataNode(struct dcNode_t *_node,
                                    struct dcString_t *_stream);

struct dcString_t *dcList_marshallGraphDataNode(const struct dcNode_t *_node,
                                                struct dcString_t *_stream);

struct dcString_t *dcList_marshall(const dcList *_list,
                                   struct dcString_t *_stream);
dcList *dcList_unmarshall(struct dcString_t *_stream);

dcResult dcList_compare(dcList *_left,
                        dcList *_right,
                        dcTaffyOperator *_compareResult);

/////////////
// testing //
/////////////

void dcList_verify(const dcList *_list);
void dcList_verifyTemplate(const dcList *_list);

/////////////
// sorting //
/////////////

void dcList_doSelectionSort(dcList *_list);

/////////////
// finding //
/////////////

dcResult dcList_find(dcList *_list,
                     struct dcNode_t *_node,
                     dcListElement **_found);

typedef dcResult (ListFindFunction)(struct dcNode_t *_left,
                                    struct dcNode_t *_right,
                                    dcTaffyOperator *_operatorResult);

dcResult dcList_findWithComparisonFunction(dcList *_list,
                                           struct dcNode_t *_node,
                                           ListFindFunction *_findFunction,
                                           dcListElement **_found);

#endif
