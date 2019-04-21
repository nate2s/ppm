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
#include <string.h>

#include "dcError.h"
#include "dcNode.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcString.h"
#include "dcThread.h"
#include "dcVoid.h"

#include "dcSystem.h" // for dcSystem_outOfMemory()

#if !(defined TAFFY_CYGWIN) && !(defined TAFFY_WINDOWS)
    #include <execinfo.h>
#endif

#ifdef TAFFY_APPLE
    #include <malloc/malloc.h>
#else
    #include <malloc.h>
#endif

static bool sEnabled = true;
static bool sTrackCreations = false;
static bool sUseMalloc = true;
static dcList *sMemoryRegions = NULL;
static dcList *sTrackedMemoryRegions = NULL;
static uint64_t sAllocatedMemory = 0;
static dcMutex *sMutex = NULL;
static bool sInitialized = false;

void dcMemory_initialize(void)
{
    if (! sInitialized)
    {
        sMutex = dcMutex_create(false);
        sInitialized = true;
    }
}

void dcMemory_deinitialize(void)
{
    assert(sInitialized);
    sInitialized = false;
    dcMutex_free(&sMutex);
}

//#define ENABLE_DOUBLE_TRACK_CHECK

// debug function
void dcMemory_assertMemoryRegionsState(void)
{
    dcError_assert(! sUseMalloc);
}

static void lock(void)
{
    if (sInitialized)
    {
        dcMutex_lock(sMutex);
    }
}

static void unlock(void)
{
    if (sInitialized)
    {
        dcMutex_unlock(sMutex);
    }
}

bool dcMemory_pushStateToMalloc(void)
{
    lock();
    bool result = sUseMalloc;
    sUseMalloc = true;
    unlock();
    return result;
}

void dcMemory_popState(bool _stateSave)
{
    lock();
    sUseMalloc = _stateSave;
    unlock();
}

void dcMemory_useMemoryRegions(void)
{
    sMemoryRegions = dcList_create();
    sTrackedMemoryRegions = dcList_create();
    lock();
    sUseMalloc = false;
    unlock();
}

void dcMemory_freeMemoryRegions(dcDepth _depth)
{
    if (_depth == DC_SHALLOW)
    {
        while (sMemoryRegions->size > 0)
        {
            TAFFY_DEBUG(uint32_t size = sMemoryRegions->size);
            dcNode *value = dcList_pop(sMemoryRegions, DC_SHALLOW);
            dcNode_free(&value, DC_SHALLOW);
            TAFFY_DEBUG(dcError_assert(sMemoryRegions->size == size - 1));
        }

        dcList_free(&sTrackedMemoryRegions, DC_DEEP);
    }
    else
    {
        lock();
        sAllocatedMemory -= dcMemory_getSize(sTrackedMemoryRegions);
        unlock();
        free(sTrackedMemoryRegions);
    }

    dcList_free(&sMemoryRegions, DC_DEEP);
}

void dcMemory_useMalloc(void)
{
    lock();
    sUseMalloc = true;
    unlock();
}

TAFFY_DEBUG(extern pthread_t parserSelf);

static void *allocateMemoryRegion(size_t _size)
{
    void *result = malloc(_size);

    if (result != NULL)
    {
        lock();
        // memory regions are single threaded so this is thread safe
        TAFFY_DEBUG(dcError_assert(! sUseMalloc));
        sUseMalloc = true;
        unlock();

#if !(defined TAFFY_APPLE) && (defined ENABLE_DEBUG)
        // for some reason this always fails in OS X
        if (pthread_equal(pthread_self(), parserSelf) == 0)
        {
            // make call trace
            abort();
        }
#endif

        dcList_push(sMemoryRegions, dcVoid_createNode(result));

        lock();
        sUseMalloc = false;
        unlock();
    }

    return result;
}

static void *reallocateMemoryRegion(void *_target, size_t _newLength)
{
    void *result = NULL;

    // slow :( use hash?
    FOR_EACH_IN_LIST(sMemoryRegions, that)
    {
        if (CAST_VOID(that->object) == _target)
        {
            result = realloc(_target, _newLength);
            CAST_VOID(that->object) = result;
            break;
        }
    }

    if (result == NULL)
    {
        result = realloc(_target, _newLength);
    }

    return result;
}

void *dcMemory_trackMemory(void *_region)
{
    if (_region != NULL)
    {
        // enable this code to catch double-tracks
#ifdef ENABLE_DOUBLE_TRACK_CHECK
        FOR_EACH_IN_LIST(sTrackedMemoryRegions, that)
        {
            dcError_assert(CAST_VOID(that->object) != _region);
        }
#endif

        dcListElement *toPush =
            dcListElement_create(dcVoid_createNode(_region));

        lock();
        dcList_pushElement(sTrackedMemoryRegions, toPush);
        unlock();
    }

    return _region;
}

void *dcMemory_allocate(size_t _size)
{
    lock();
    void *result = NULL;

    if (sUseMalloc)
    {
        result = (_size > 0
                  ? malloc(_size)
                  : NULL);
    }
    else
    {
        unlock();
        result = (_size > 0
                  ? allocateMemoryRegion(_size)
                  : NULL);
        lock();
    }

    if (result != NULL)
    {
        size_t allocatedSize = dcMemory_getSize(result);
        TAFFY_DEBUG(dcError_assert(sAllocatedMemory + allocatedSize
                                   > sAllocatedMemory));
        sAllocatedMemory += allocatedSize;
    }
    else if (_size > 0)
    {
        dcSystem_outOfMemory();
    }

    unlock();
    return result;
}

void *dcMemory_realloc(void *_target, size_t _newLength)
{
    lock();
    size_t targetSize = dcMemory_getSize(_target);
    void *result = (_newLength > 0
                    ? (sUseMalloc
                       ? realloc(_target, _newLength)
                       : reallocateMemoryRegion(_target, _newLength))
                    : _target);
    sAllocatedMemory -= targetSize;
    sAllocatedMemory += dcMemory_getSize(result);
    unlock();
    return result;
}

void *dcMemory_allocateAndInitialize(size_t _size)
{
    void *result = dcMemory_allocate(_size);
    memset(result, 0, _size);
    return result;
}

void *dcMemory_duplicate(const void *_from, size_t _size)
{
    void *result = dcMemory_allocate(_size);

    if (result != NULL)
    {
        dcMemory_copy(result, _from, _size);
    }

    return result;
}

void dcMemory_freeDoubleVoid(void **_area)
{
    if (*_area == NULL)
    {
        return;
    }

    lock();

    if (sUseMalloc)
    {
        size_t areaSize = dcMemory_getSize(*_area);
        TAFFY_DEBUG(dcError_assert(sAllocatedMemory >= areaSize));
        sAllocatedMemory -= areaSize;

        unlock();
        free(*_area);
        *_area = NULL;
    }
    else
    {
        unlock();
        dcMemory_trackMemory(*_area);
        *_area = NULL;
    }
}

size_t dcMemory_copy(void *_to, const void *_from, size_t _size)
{
    memcpy(_to, _from, _size);
    return _size;
}

char *dcMemory_strdup(const char *_from)
{
    char *result = NULL;
    lock();

    if (sUseMalloc)
    {
        unlock();

#ifdef TAFFY_WINDOWS
        result = _strdup(_from);
#else
        result = strdup(_from);
#endif
    }
    else
    {
        unlock();
        size_t size = strlen(_from) + 1;
        result = (char *)allocateMemoryRegion(size);

        if (result != NULL)
        {
            memcpy(result, _from, size);
        }
    }

    if (result != NULL)
    {
        lock();
        sAllocatedMemory += dcMemory_getSize(result);
        unlock();
    }

    return result;
}

dcTaffyOperator dcMemory_taffyStringCompare(const char *_left,
                                            const char *_right)
{
    int result = strcmp(_left, _right);
    return (result == 0
            ? TAFFY_EQUALS
            : (result < 0
               ? TAFFY_LESS_THAN
               : TAFFY_GREATER_THAN));
}

dcTaffyOperator dcMemory_taffyMemcompare(const void *_left,
                                         const void *_right,
                                         size_t _size)
{
    int result = memcmp(_left, _right, _size);
    return (result == 0
            ? TAFFY_EQUALS
            : (result < 0
               ? TAFFY_LESS_THAN
               : TAFFY_GREATER_THAN));
}

bool dcMemory_isEnabled(void)
{
    return sEnabled;
}

void dcMemory_setEnabled(bool _yesno)
{
    sEnabled = _yesno;
}

void dcMemory_setTrackCreations(bool _yesno)
{
    sTrackCreations = _yesno;
}

bool dcMemory_trackCreations(void)
{
    return sTrackCreations;
}

#if !(defined TAFFY_CYGWIN) && !(defined TAFFY_WINDOWS)
void dcMemory_createBacktrace(dcList *_list)
{
    if (_list != NULL)
    {
        void *buffer[100];
        int count = backtrace(buffer, 100);
        char **strings = backtrace_symbols(buffer, count);
        dcError_assert(strings != NULL);
        int j;

        for (j = 0; j < count; j++)
        {
            dcList_push(_list, dcString_createNodeWithString(strings[j], true));
        }

        free(strings);
    }
}

void dcMemory_printBacktrace(const dcList *_list)
{
    if (_list == NULL)
    {
        fprintf(stderr, "NULL??\n");
    }
    else
    {
        // don't use dcNode_display(), as it might collide with the GC
        FOR_EACH_IN_LIST(_list, that)
        {
            fprintf(stderr, "%s\n", dcString_getString(that->object));
        }
    }
}
#else // TAFFY_CYGWIN
void dcMemory_createBacktrace(dcList *_list) {}
void dcMemory_printBacktrace(const dcList *_list) {}
#endif

size_t dcMemory_getSize(void *_area)
{
#ifdef TAFFY_APPLE
    return malloc_size(_area);
#elif (defined TAFFY_LINUX) || (defined TAFFY_CYGWIN)
    return malloc_usable_size(_area);
#elif (defined TAFFY_WINDOWS)
    return _msize(_area);
#else
    #error "Unknown platform in dcMemory_getSize"
#endif
}

uint64_t dcMemory_getAllocatedMemorySize(void)
{
    uint64_t result = 0;
    lock();
    result = sAllocatedMemory;
    unlock();
    return result;
}
