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

#ifndef __DC_STRING_H__
#define __DC_STRING_H__

#include "dcDefines.h"

#define DEFAULT_STRING_LENGTH 20

struct dcString_t
{
    // the string
    char *string;

    // how many bytes are there in string
    uint64_t length;

    // used for streaming
    uint64_t index;
};

typedef struct dcString_t dcString;

  //////////////
 // creating //
///////////////

// create a dcNode
struct dcNode_t *dcString_createNode(void);
struct dcNode_t *dcString_createNodeWithString(const char *_string,
                                               bool _copy);
struct dcNode_t *dcString_createNodeWithBytes(const uint8_t *_bytes,
                                              uint64_t _length);

// create a string on the stack
void dcString_initialize(dcString *_string, uint64_t _length);

// create a dcString
dcString *dcString_create(void);
dcString *dcString_createWithLength(uint64_t _length);
dcString *dcString_createWithString(const char *_string, bool _copy);
dcString *dcString_createWithBytes(const uint8_t *_bytes, uint64_t _length);
struct dcNode_t *dcString_createShell(dcString *_string);

// freeing //
void dcString_free(dcString **_string, dcDepth _depth);
void dcString_freeCharArray(char **_string, dcDepth _depth);

// displaying //
char *dcString_displayBytes(const dcString *_string);
dcResult dcString_hashCharArray(const char *_string, dcHashType *_hashResult);

// getting //
char dcString_get(const dcString *_string, uint64_t _index);
char *dcString_getString(const struct dcNode_t *_string);
uint64_t dcString_getLength(const struct dcNode_t *_string);
uint64_t dcString_getStringLength(const struct dcString_t *_string);

// free the container, *_string, shallowly, and return its contents
char *dcString_freeAndReturn(dcString **_string);

// streaming //
uint8_t *dcString_getFinger(const dcString *_stream);
long int dcString_getLengthLeft(const dcString *_stream);
bool dcString_hasLengthLeft(const dcString *_stream, uint64_t _length);
uint8_t dcString_getCharacter(dcString *_stream);
uint8_t dcString_peek(const dcString *_stream);

// seeking //
void dcString_seekEnd(dcString *_string);
void dcString_seekBeginning(dcString *_string);

// concatentation, etc //
void dcString_appendCharacter(dcString *_string, char _character);
void dcString_appendString(dcString *_string, const char *_appendString);
void dcString_appendBytes(dcString *_string,
                          const char *_toAppend,
                          uint64_t _length);
void dcString_append(dcString *_string, const char *_format, ...);

// length modification //
void dcString_resize(dcString *_string, uint64_t _newLength);
void dcString_enlargeString(dcString *_string);
void dcString_enlargeStringFromLength(dcString *_string,
                                      uint64_t _newLength);
bool dcString_shorten(dcString *_string, uint64_t _howMuch);
void dcString_needBytes(dcString *_string, uint64_t _size);

// index modification //
dcString *dcString_resetIndex(dcString *_string);
void dcString_incrementIndex(dcString *_string, uint64_t _amount);
void dcString_incrementIndexByFinger(dcString *_string, uint8_t *_finger);
uint8_t *dcString_incrementGetFinger(dcString *_string, uint64_t _amount);
void dcString_decrementIndex(dcString *_string, uint64_t _amount);

// copying //
dcString *dcString_copy(const dcString *_from, dcDepth _depth);

// clearing //
void dcString_clear(dcString *_string);
void dcString_setString(dcString *_string, dcString *_newString);

// equals //
bool dcString_equals(const dcString *_left, const dcString *_right);
bool dcString_equalsCharArray(const dcString *_left, const char *_right);

/////////////////
// marshalling //
/////////////////

dcString *dcString_unmarshall(dcString *_stream);
dcString *dcString_marshall(const dcString *_string, dcString *_stream);

char *dcString_unmarshallCharArray(dcString *_stream);
dcString *dcString_marshallCharArray(const char *_string, dcString *_stream);

/////////////
// hashing //
/////////////

dcResult dcString_hashBytes(const uint8_t *_bytes,
                            uint64_t _length,
                            dcHashType *_hashResult);

// standard functions //
COPY_FUNCTION(dcString_copyNode);
COMPARE_FUNCTION(dcString_compareNode);
FREE_FUNCTION(dcString_freeNode);
PRINT_FUNCTION(dcString_printNode);
MARSHALL_FUNCTION(dcString_marshallNode);
UNMARSHALL_FUNCTION(dcString_unmarshallNode);
HASH_FUNCTION(dcString_hashNode);

#endif
