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

#ifndef __DC_ARRAY_H__
#define __DC_ARRAY_H__

#include "dcDefines.h"

#define FOR_EACH_IN_ARRAY(_array, _iterator, _object)                   \
    dcContainerSizeType _iterator;                                      \
    dcNode *_object;                                                    \
    for (_iterator = 0, _object = _array->objects[0];                   \
         _iterator < _array->size;                                      \
         _iterator++, _object = _array->objects[_iterator])

/**
 * An array container
 */
struct dcArray_t
{
    /**
     * An array containing dcNodeS
     */
    struct dcNode_t **objects;

    /**
     * The capacity of this array, or the size of objects
     */
    dcContainerSizeType capacity;

    /**
     * How many dcNodeS the objects array contains
     */
    dcContainerSizeType size;
};

typedef struct dcArray_t dcArray;

//////////////
// creating //
//////////////

/**
 * Creates a new instance of a dcArray
 * \param _capacity capacity of the array
 * \return The newly allocated dcArray
 */
dcArray *dcArray_createWithSize(dcContainerSizeType _capacity);

struct dcNode_t *dcArray_createNodeWithCapacity(dcContainerSizeType _capacity);

/**
 * Creates a new instance of a dcArray
 * \param _first first dcNode
 * \param ... second, and beyond, dcNodeS
 * \n Note: The argument list must be NULL terminated
 * \return The newly allocated dcArray
 */
dcArray *dcArray_createWithObjects(struct dcNode_t *_first, ...);

/**
 * Creates a new instance of a dcArray-node
 * \param _first first dcNode
 * \param ... second, and beyond, dcNodeS
 * \n Note: The argument list must be NULL terminated
 * \return The newly allocated dcNode array
 */
struct dcNode_t *dcArray_createNodeWithObjects(struct dcNode_t *_first, ...);

/**
 * Creates a new instance of a dcArray-node, whose elements are copied from
 * a list
 * \param _list The list whose elements are copied
 * \param _depth The depth of the copy
 * \return The newly allocated dcNode array
 */
struct dcNode_t *dcArray_createNodeFromList(const struct dcList_t *_list,
                                            dcDepth _depth);


/**
 * Creates a new instance of a dcArray, whose elements are copied from a list
 * \param _list The list whose elements are copied
 * \param _depth The depth of the copy
 * \return The newly allocated dcArray
 */
dcArray *dcArray_createFromList(const struct dcList_t *_list, dcDepth _depth);

struct dcNode_t *dcArray_createShell(dcArray *_array);

/////////////
// freeing //
/////////////

/**
 * Frees a dcArray, and potentially all its contents
 * \param _array The array to free
 * \param _depth The depth of the free
 */
void dcArray_free(dcArray **_array, dcDepth _depth);

/**
 * Removes an object from a dcArray
 * \param _array The array to remove the object from
 * \param _object The object to remove
 * \param _depth The depth of the removal
 * \return Whether the removal was successful
 */
bool dcArray_removeObject(dcArray *_array,
                          struct dcNode_t *_object,
                          dcDepth _depth);

/**
 * Pops a dcNode from the end of the dcArray
 * \param _array The array to pop the dcNode from
 * \param _depth The depth of the pop
 * \return The popped dcNode, may be NULL
 */
struct dcNode_t *dcArray_pop(dcArray *_array,  dcDepth _depth);

////////////////
// displaying //
////////////////

/**
 * Converts a dcArray to a char* array
 * \param _array The array to convert
 * \return The char* representation of the dcArray
 */
dcResult dcArray_print(const dcArray *_array, struct dcString_t *_string);
const char *dcArray_display(const dcArray *_array);

//////////////
// morphing //
//////////////

/**
 * Converts a dcArray-node to a char array
 * \param _array The array to convert
 */
void dcArray_autoResize(dcArray *_array);

/**
 * Removes all dcNodeS from a dcArray
 * \param _array The array to clear
 * \param _depth The depth of the clear
 */
void dcArray_clear(dcArray *_array, dcDepth _depth);

/**
 * Shifts, or removes, the first dcNode of the array
 * \param _array The array to shift
 * \param _index The index to shift at
 * \param _depth The depth of the shift
 * \return The node at index _index, if not freed
 */
struct dcNode_t *dcArray_shift(dcArray *_array,
                               dcContainerSizeType _index,
                               dcDepth _depth);

/**
 * Resizes a dcArray. This function preserves the dcArray's dcNodeS if
 * _newCapacity >= _array->capacity
 * \param _array The array to resize
 * \param _newCapacity The new capacity
 */
void dcArray_resize(dcArray *_array, dcContainerSizeType _newCapacity);

/////////////
// getting //
/////////////

/**
 * Gets an element from a dcArray
 * \param _array The array
 * \param _index The index
 * \return The dcNode at index _index, may be NULL
 */
struct dcNode_t *dcArray_get(const dcArray *_array, dcContainerSizeType _index);

/**
 * Gets the size of a dcArray
 * \param _array The array
 * \return The size of the array
 */
dcContainerSizeType dcArray_getSize(const struct dcNode_t *_array);

/**
 * Gets the objects of a dcArray
 * \param _array The array
 * \return The objects of the array
 */
struct dcNode_t **dcArray_getObjects(const struct dcNode_t *_array);

/**
 * Gets the capacity of a dcArray
 * \param _array The array
 * \return The capacity of the array
 */
dcContainerSizeType dcArray_getCapacity(const struct dcNode_t *_array);

////////////
// adding //
////////////

/**
 * Sets an element in a dcArray
 * \param _array The array
 * \param _node The dcNode to set
 * \param _index The index to set the dcNode at
 */
void dcArray_set(dcArray *_array,
                 struct dcNode_t *_node,
                 dcContainerSizeType _index);

/**
 * Copies the dcNodeS from one array to another
 * \param _to The array to copy to
 * \param _from The array to copy from
 * \param _startingIndex The index to start the copy from
 */
void dcArray_converge(dcArray *_to,
                      const dcArray *_from,
                      dcContainerSizeType _startingIndex);

/**
 * Unshifts a dcNode into a dcArray,
 * with no loss to the original contents of the array
 * \param _array The array
 * \param _node The dcNode to unshift
 * \param _index The index to unshift at
 */
void dcArray_unshiftAtIndex(dcArray *_array,
                            struct dcNode_t *_node,
                            dcContainerSizeType _index);

/**
 * Unshifts a dcNode into the beginning of a dcArray,
 * with no loss to the original contents of the array
 * \param _array The array
 * \param _node The dcNode to unshift
  */
void dcArray_unshift(dcArray *_array, struct dcNode_t *_node);

/**
 * Adds a dcNode to the end of a dcArray
 * \param _array The array
 * \param _node The dcNode
 */
void dcArray_add(dcArray *_array, struct dcNode_t *_node);

/**
 * Adds dcNodeS to the end of a dcArray
 * \n Note: This function is potentially unsafe (ABR/ABW)
 * and should be used with caution (see dcArray_addSafely)
 * \param _array The array
 * \param _first The first dcNode
 * \param ... The second, and beyond, dcNodeS
 */
void dcArray_addObjects(dcArray *_array, struct dcNode_t *_first, ...);

/////////////
// copying //
/////////////

/**
 * Creates a new dcArray whose elements are copies of a dcArray
 * \param _from The array to copy from
 * \param _depth The depth of the copy
 * \return The newly allocated copy of _from
 */
dcArray *dcArray_copy(const dcArray *_from, dcDepth _depth);

////////////////////
// sanctification //
////////////////////

/**
 * Sanctifies an index in reference to a dcArray \n
 * The index 'wraps around' the array
 * \n Example:
 * \n   if x == -1, index = length(array) - 1
 * \n   if x == length(array), index = 0
 * \n   in general, for some x, index = (length(array) - x) % length(array)
 * \param _array The array to sanctify against
 * \param _index The index to sanctify
 * \return The sanctified index
 */
dcContainerSizeType dcArray_sanctifyIndex(const dcArray *_array, int _index);

/////////////
// marking //
/////////////

/**
 * Marks a dcArray, and potentially all its contents,
 * for garbage collection
 * \param _array The array to mark
 */
void dcArray_mark(dcArray *_array);

/////////////
// setting //
/////////////

void dcArray_setTemplate(dcArray *_array, bool _yesNo);

///////////////
// comparing //
///////////////

dcResult dcArray_compare(dcArray *_left,
                         dcArray *_right,
                         dcTaffyOperator *_compareResult);

////////////////////////
// standard functions //
////////////////////////

FREE_FUNCTION(dcArray_freeNode);
COPY_FUNCTION(dcArray_copyNode);
MARK_FUNCTION(dcArray_markNode);
PRINT_FUNCTION(dcArray_printNode);
REGISTER_FUNCTION(dcArray_registerNode);
MARSHALL_FUNCTION(dcArray_marshallNode);
UNMARSHALL_FUNCTION(dcArray_unmarshallNode);

/////////////////
// registering //
/////////////////

/**
 * Registers a dcArray-node, and potentially all its contents,
 * for garbage collection
 */
void dcArray_register(dcArray *_array);

/////////////////
// marshalling //
/////////////////

dcArray *dcArray_unmarshall(struct dcString_t *_stream);
struct dcString_t *dcArray_marshall(const dcArray *_array,
                                    struct dcString_t *_stream);

#endif
