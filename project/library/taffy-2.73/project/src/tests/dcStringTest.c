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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcGarbageCollector.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"
#include "dcTestUtilities.h"

// determine why org.taffy.foo gehts or.taffy.fo during import

// test that it's blank
static void testBlank(dcString *_string)
{
    size_t i;

    for (i = 0; i < _string->length; i++)
    {
        dcTestUtilities_assert(_string->string[i] == 0);
    }
}

static void testCreateBlank(void)
{
    {
        dcString *string = dcString_create();
        testBlank(string);
        dcString_free(&string, DC_DEEP);
    }

    {
        dcNode *string = dcString_createNode();
        dcNode_assertType(string, NODE_STRING);
        testBlank(CAST_STRING(string));
        dcNode_free(&string, DC_DEEP);
    }
}

static void assertStringEqual(dcString *_string, const char *_other)
{
    dcTestUtilities_assert(strcmp(_string->string, _other) == 0);
}

static const char sSentence[] =
    "the brown taffy got done chewered by the fox, "
    "he really done did, yup yup he did! "
    "I swear he done chewered it gone";

static char *copySentence(void)
{
    return (char *)dcMemory_duplicate(sSentence, sizeof(sSentence));
}

static void testCreateWithString(void)
{
    {
        dcString *string = dcString_createWithString(sSentence, true);
        assertStringEqual(string, sSentence);
        dcString_free(&string, DC_DEEP);
    }

    {
        dcString *string = dcString_createWithString(copySentence(), false);
        assertStringEqual(string, sSentence);
        dcString_free(&string, DC_DEEP);
    }
}

static void testCreateNodeWithString(void)
{
    {
        dcNode *string = dcString_createNodeWithString(sSentence, true);
        assertStringEqual(CAST_STRING(string), sSentence);
        dcNode_free(&string, DC_DEEP);
    }

    {
        dcNode *string = dcString_createNodeWithString(copySentence(), false);
        assertStringEqual(CAST_STRING(string), sSentence);
        dcNode_free(&string, DC_DEEP);
    }
}

static void testCreateWithBytes(void)
{
    // node
    {
        const uint8_t bytes[] = {1, 2, 3, 4, 5, 6};
        dcNode *string = dcString_createNodeWithBytes(bytes, sizeof(bytes));
        dcTestUtilities_assert
            (memcmp((const uint8_t *)dcString_getString(string),
                    bytes,
                    sizeof(bytes))
             == 0);
        dcTestUtilities_assert(CAST_STRING(string)->length == sizeof(bytes));
        dcNode_free(&string, DC_DEEP);
    }

    // string
    {
        const uint8_t bytes[] = {1, 2, 3, 4, 5, 6};
        dcString *string = dcString_createWithBytes(bytes, sizeof(bytes));
        dcTestUtilities_assert
            (memcmp((const uint8_t *)string->string,
                    bytes,
                    sizeof(bytes))
             == 0);
        dcTestUtilities_assert(string->length == sizeof(bytes));
        dcString_free(&string, DC_DEEP);
    }
}

static void assertStringLoopedContents(dcString *_string)
{

    size_t j = 0;

    for (j = 0;
         j < dcString_getStringLength(_string);
         j += sizeof(sSentence) - 1)
    {
        dcTestUtilities_assert(memcmp(_string->string + j,
                                      sSentence,
                                      (j < (dcString_getStringLength(_string)
                                            - sizeof(sSentence))
                                       ? sizeof(sSentence) - 1
                                       : sizeof(sSentence)))
                               == 0);
    }
}

static void testAppendCharacter(void)
{
    dcString *string = dcString_create();
    size_t i = 0;

    for (i = 0; i < 50; i++)
    {
        size_t j = 0;

        for (j = 0; j < sizeof(sSentence) - 1; j++)
        {
            dcString_appendCharacter(string, sSentence[j]);
        }

        assertStringLoopedContents(string);
    }

    dcString_free(&string, DC_DEEP);
}

static void testAppendString(void)
{
    dcString *string = dcString_create();
    size_t i = 0;

    for (i = 0; i < 50; i++)
    {
        dcString_appendString(string, sSentence);
        assertStringLoopedContents(string);
    }

    dcString_free(&string, DC_DEEP);
}

static void testAppendFormat(void)
{
    // append a bunch of sentences
    {
        dcString *string = dcString_create();

        size_t i = 0;

        for (i = 0; i < 10; i++)
        {
            dcString_append(string, "%s", sSentence);
            assertStringLoopedContents(string);
        }

        dcString_free(&string, DC_DEEP);
    }

    // create a package
    {
        dcString *string = dcString_create();
        dcString_append(string, "%s.", "org");
        dcString_append(string, "%s.", "taffy");
        dcString_append(string, "%s", "core");
        assertStringEqual(string, "org.taffy.core");
        dcString_free(&string, DC_DEEP);
    }

    // append nothing
    {
        dcString *string = dcString_create();
        dcString_append(string, "");
        assertStringEqual(string, "");
        dcString_free(&string, DC_DEEP);
    }

    // append a bunch of characters
    {
        dcString *string = dcString_create();
        size_t i = 0;

        for (i = 0; i < 50; i++)
        {
            size_t j = 0;

            for (j = 0; j < sizeof(sSentence) - 1; j++)
            {
                dcString_append(string, "%c", sSentence[j]);
            }

            assertStringLoopedContents(string);
        }

        dcString_free(&string, DC_DEEP);
    }
}

static const dcTestFunctionMap map[] =
{
    {"Create Blank",            &testCreateBlank},
    {"Create With String",      &testCreateWithString},
    {"Create Node With String", &testCreateNodeWithString},
    {"Create With Bytes",       &testCreateWithBytes},
    {"Append Character",        &testAppendCharacter},
    {"Append String",           &testAppendString},
    {"Append Format",           &testAppendFormat},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcGarbageCollector_create();
    dcTestUtilities_go("String Test", _argc, _argv, NULL, map, false);
    dcGarbageCollector_free();
    dcMemory_deinitialize();
    return 0;
}
