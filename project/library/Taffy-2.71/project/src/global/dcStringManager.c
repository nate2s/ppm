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

#include "dcArray.h"
#include "dcError.h"
#include "dcHash.h"
#include "dcMemory.h"
#include "dcUnsignedInt32.h"
#include "dcNode.h"
#include "dcString.h"

// a hash of (dcString => uint32_t), is a reverse map of stringIds below
typedef struct
{
    dcHash *reverseStringIds;
    dcStringId maxStringId;

    // maps (uint32_t => dcString)
    dcArray *stringIds;
} dcStringManager;

static dcStringManager *sStringManager = NULL;

static void createMapping(const char *_string, dcStringId _id)
{
    bool stateSave = dcMemory_pushStateToMalloc();

    // create an Int => String mapping
    dcArray_set(sStringManager->stringIds,
                dcString_createNodeWithString(_string, true),
                _id);

    // create a String => Int mapping
    dcHash_setValueWithStringKey(sStringManager->reverseStringIds,
                                 _string,
                                 dcUnsignedInt32_createNode(_id));
    dcMemory_popState(stateSave);
}

void dcStringManager_create(void)
{
    if (sStringManager == NULL)
    {
        sStringManager =
            (dcStringManager *)dcMemory_allocate(sizeof(dcStringManager));
        sStringManager->stringIds = dcArray_createWithSize(200);
        sStringManager->reverseStringIds = dcHash_createWithCapacity(200);
        sStringManager->maxStringId = 1;
        createMapping("INVALID_STRING_ID", 0);
    }
}

void dcStringManager_free(void)
{
    if (sStringManager != NULL)
    {
        dcArray_free(&sStringManager->stringIds, DC_DEEP);
        dcHash_free(&sStringManager->reverseStringIds, DC_DEEP);
        dcMemory_free(sStringManager);
    }
}

dcStringId dcStringManager_getStringId(const char *_string)
{
    dcError_assert(sStringManager != NULL);

    dcNode *already = NULL;
    dcError_assert(dcHash_getValueWithStringKey
                   (sStringManager->reverseStringIds,
                    (_string == NULL
                     ? ""
                     : _string),
                    &already)
                   != TAFFY_EXCEPTION);
    uint32_t result = 0;

    if (already != NULL)
    {
        // it already exists
        result = CAST_INT(already);
    }
    else
    {
        // it doesn't already exist, create it
        result = sStringManager->maxStringId;
        createMapping(_string, sStringManager->maxStringId);

        // increment the max string id
        sStringManager->maxStringId++;
    }

    return result;
}

const char *dcStringManager_getStringFromId(dcStringId _id)
{
    dcNode *node = dcArray_get(sStringManager->stringIds, _id);
    return (node == NULL
            ? NULL
            : CAST_STRING(node)->string);
}

const char *dcStringManager_getStringFromString(const char *_string)
{
    return (_string == NULL
            ? NULL
            : (dcStringManager_getStringFromId
               (dcStringManager_getStringId(_string))));
}
