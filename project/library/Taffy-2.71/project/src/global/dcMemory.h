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

#ifndef __DC_MEMORY_H__
#define __DC_MEMORY_H__

// for size_t
#include <stdlib.h>

#include "dcDefines.h"

/**
 * Initialize the memory system. Call this before any call to
 * dcMemory_allocate().
 */
void dcMemory_initialize(void);

/**
 * Deinitialize the memory system. Call this after all calls to
 * dcMemory_allocate().
 */
void dcMemory_deinitialize(void);

/**
 * Allocate a memory region, like malloc().
 */
void *dcMemory_allocate(size_t _size);

/**
 * @brief Allocate and initialize a memory region to 0.
 */
void *dcMemory_allocateAndInitialize(size_t _size);

/**
 * @brief Reallocate a memory region to a new size, like realloc().
 */
void *dcMemory_realloc(void *_target, size_t _newLength);

void *dcMemory_duplicate(const void *_from, size_t _size);
char *dcMemory_strdup(const char *_from);

/**
 * @brief Compares two strings
 * Returns a value in: {TAFFY_LESS_THAN, TAFFY_EQUAL, TAFFY_GREATER_THAN}
 * which correspond to, respectively:
 * _left less than _right
 * _less equal to _right
 * _left greater than _right
 */
dcTaffyOperator dcMemory_taffyStringCompare(const char *_left,
                                            const char *_right);

/** @brief Compares two segments of memory
 * Returns a value in: {TAFFY_LESS_THAN, TAFFY_EQUAL, TAFFY_GREATER_THAN}
 * which correspond to _left less than _right, _less equal to _right, etc
 */
dcTaffyOperator dcMemory_taffyMemcompare(const void *_left,
                                         const void *_right,
                                         size_t _size);

size_t dcMemory_copy(void *_to, const void *_from, size_t _size);
void dcMemory_freeDoubleVoid(void **_memory);
#define dcMemory_free(value) dcMemory_freeDoubleVoid((void**)&value);

/**
 * @brief Get the allocated size of a memory region
 */
size_t dcMemory_getSize(void *_area);
uint64_t dcMemory_getAllocatedMemorySize(void);

/////////////
// testing //
/////////////

/**
 * @brief Test function. Query if memory management is enabled
 */
bool dcMemory_isEnabled(void);

/**
 * @brief Test function. Enable or disable memory management
 */
void dcMemory_setEnabled(bool _yesno);

/**
 * @brief Test function. Create a backtrace
 */
void dcMemory_createBacktrace(struct dcList_t *_list);

/**
 * @brief Test function. Print a backtrace
 */
void dcMemory_printBacktrace(const struct dcList_t *_list);

/**
 * @brief Test function. Enable or disable node creation backtraces
 */
void dcMemory_setTrackCreations(bool _yesno);

/**
 * @brief Test function. Query if track creations is enabled
 */
bool dcMemory_trackCreations(void);

void dcMemory_useMemoryRegions(void);
void dcMemory_freeMemoryRegions(dcDepth _depth);
void dcMemory_useMalloc(void);
void *dcMemory_trackMemory(void *_region);

bool dcMemory_pushStateToMalloc(void);
void dcMemory_popState(bool _stateSave);

void dcMemory_assertMemoryRegionsState(void);

#endif
