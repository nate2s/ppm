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
#include <stdarg.h>
#include <string.h>

#include "dcString.h"
#include "dcUnsignedInt64.h"
#include "dcLexer.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"

dcNode *dcString_createNodeWithString(const char *_string, bool _copy)
{
    return dcNode_createWithGuts(NODE_STRING,
                                 dcString_createWithString(_string, _copy));
}

dcNode *dcString_createNode(void)
{
    return dcNode_createWithGuts(NODE_STRING, dcString_create());
}

void dcString_initialize(dcString *_string, uint64_t _length)
{
    _string->string = (char *)dcMemory_allocateAndInitialize(_length);
    _string->length = _length;
    _string->index = 0;
}

static dcString *createWithString(const char  *_string,
                                  uint64_t _length,
                                  bool _copy)
{
    assert(_length > 0);
    dcString *string = (dcString *)dcMemory_allocate(sizeof(dcString));
    string->string = (_copy
                      ? dcMemory_strdup(_string)
                      : (char*)_string);
    string->length = _length;
    string->index = _length - 1;
    return string;
}

dcString *dcString_createBytes(uint64_t _length)
{
    dcString *result = createWithString
        ((char *)dcMemory_allocateAndInitialize(_length),
         _length,
         false);
    result->index = 0;
    return result;
}

dcString *dcString_createWithLength(uint64_t _length)
{
    dcString *result = createWithString
        ((char *)dcMemory_allocateAndInitialize(_length), _length, false);
    result->index = 0;
    return result;
}

dcString *dcString_createWithString(const char *_string, bool _copy)
{
    return createWithString(_string, strlen(_string) + 1, _copy);
}

dcString *dcString_createWithBytes(const uint8_t *_bytes, uint64_t _length)
{
    dcString *result = dcString_createWithLength(_length);
    memcpy(result->string, _bytes, _length);
    result->length = _length;
    return result;
}

dcNode *dcString_createNodeWithBytes(const uint8_t *_bytes, uint64_t _length)
{
    return dcNode_createWithGuts(NODE_STRING,
                                 dcString_createWithBytes(_bytes, _length));
}

dcString *dcString_create(void)
{
    return dcString_createWithString("", true);
}

dcNode *dcString_createShell(dcString *_string)
{
    dcNode *node = dcNode_create(NODE_STRING);
    CAST_STRING(node) = _string;
    return node;
}

void dcString_free(dcString **_string, dcDepth _depth)
{
    if (_string != NULL && *_string != NULL)
    {
        if (_depth == DC_DEEP)
        {
            dcMemory_free((*_string)->string);
        }

        dcMemory_free(*_string);
    }
}

void dcString_freeCharArray(char **_string, dcDepth _depth)
{
    dcMemory_free(*_string);
}

// create dcString_freeNode()
dcTaffy_createFreeNodeFunctionMacro(dcString, CAST_STRING);

// create dcString_unmarshallNode()
dcTaffy_createUnmarshallNodeFunctionMacro(dcString, CAST_STRING);

// create dcString_marshallNode()
dcTaffy_createMarshallNodeFunctionMacro(dcString, CAST_STRING);

dcString *dcString_copy(const dcString *_from, dcDepth _depth)
{
    uint64_t memLength = _from->length + 1;
    dcString *to = dcString_createWithLength(memLength);
    dcMemory_copy(to->string, _from->string, memLength - 1);
    return to;
}

dcResult dcString_printNode(const dcNode *_node, dcString *_string)
{
    dcString_appendString(_string, dcString_getString(_node));
    return TAFFY_SUCCESS;
}

char dcString_get(const dcString *_string, uint64_t _index)
{
    assert(_index < _string->length);
    return _string->string[_index];
}

char *dcString_getString(const dcNode *_string)
{
    return CAST_STRING(_string)->string;
}

uint64_t dcString_getLength(const dcNode *_string)
{
    return CAST_STRING(_string)->length;
}

uint64_t dcString_getStringLength(const dcString *_string)
{
    return strlen(_string->string);
}

uint8_t *dcString_getFinger(const dcString *_string)
{
    return (uint8_t*)(_string->string + _string->index);
}

long int dcString_getLengthLeft(const dcString *_string)
{
    return (_string->length - _string->index);
}

uint8_t dcString_getCharacter(dcString *_stream)
{
    uint8_t result = 0xFF;

    if (_stream->index <= _stream->length)
    {
        _stream->index++;
        result = (_stream->string[_stream->index - 1]);
    }

    return result;
}

uint8_t dcString_peek(const dcString *_stream)
{
    assert(_stream->index < _stream->length);
    return _stream->string[_stream->index];
}

dcString *dcString_resetIndex(dcString *_string)
{
    _string->index = 0;
    return _string;
}

void dcString_incrementIndex(dcString *_string, uint64_t _amount)
{
    _string->index += _amount;
}

void dcString_incrementIndexByFinger(dcString *_string, uint8_t *_finger)
{
    _string->index += ((char*)_finger - _string->string);
}

uint8_t *dcString_incrementGetFinger(dcString *_string, uint64_t _amount)
{
    dcString_incrementIndex(_string, _amount);
    return dcString_getFinger(_string);
}

void dcString_decrementIndex(dcString *_string, uint64_t _amount)
{
    _string->index -= _amount;
}

void dcString_enlargeString(dcString *_string)
{
    dcString_enlargeStringFromLength(_string, _string->length * 2);
}

bool dcString_shorten(dcString *_string, uint64_t _howMuch)
{
    bool result = false;

    if (_string->index >= _howMuch)
    {
        _string->string[_string->index - _howMuch] = 0;
        _string->index -= _howMuch;
        result = true;
    }

    return result;
}

void dcString_resize(dcString *_string, uint64_t _newLength)
{
    _string->string = (char *)dcMemory_realloc(_string->string, _newLength);

    if (_newLength > _string->length)
    {
        memset(_string->string + _string->length,
               0,
               _newLength - _string->length);
    }

    _string->length = _newLength;
}

void dcString_enlargeStringFromLength(dcString *_string, uint64_t _newLength)
{
    dcString_resize(_string, _newLength);
}

void dcString_needBytes(dcString *_string, uint64_t _numBytes)
{
    while (_string->index + _numBytes >= _string->length)
    {
        dcString_enlargeString(_string);
    }
}

void dcString_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_STRING(_to) = dcString_copy(CAST_STRING(_from), _depth);
}

void dcString_seekEnd(dcString *_string)
{
    _string->index = _string->length;
}

void dcString_seekBeginning(dcString *_string)
{
    _string->index = 0;
}

void dcString_appendCharacter(dcString *_string, char _character)
{
    // check if the string needs to be resized due to squeeeeeeeze //
    if ((int32_t)_string->index >= (int32_t)_string->length - 2)
    {
        dcString_enlargeStringFromLength(_string, (_string->length + 1) * 2);
    }

    // append that char //
    _string->string[_string->index] = _character;
    _string->index++;
}

void dcString_appendBytes(dcString *_string,
                          const char *_toAppend,
                          uint64_t _length)
{
    if (_length > 0)
    {
        // test resizing
        if (_string->index + _length >= _string->length)
        {
            dcString_enlargeStringFromLength
                (_string, (_string->length + _length) * 2);
        }

        // append the string
        memcpy(_string->string + _string->index,
               _toAppend,
               _length * sizeof(char));

        _string->index += _length;
    }
}

void dcString_appendString(dcString *_string, const char *_toAppend)
{
    dcString_appendBytes(_string, _toAppend, strlen(_toAppend));
}

void dcString_append(dcString *_string, const char *_format, ...)
{
    va_list arguments;
    va_start(arguments, _format);
    size_t appendLength = 0;
    char *output = dcLexer_sprintfWithVaList(_format, arguments, &appendLength);

    assert(appendLength > 0);
    // don't append the 0, aka appendLength - 1
    dcString_appendBytes(_string, output, appendLength - 1);

    dcMemory_free(output);
}

void dcString_end(dcString *_string)
{
    _string->string[_string->index] = '\0';
    _string->index++;
}

void dcString_clear(dcString *_string)
{
    memset(_string->string, 0, sizeof(char) * _string->length);
    _string->index = 0;
}

void dcString_setString(dcString *_string, dcString *_newString)
{
    dcMemory_free(_string->string);
    _string->string = _newString->string;
    _string->index = 0;
    _string->length = _newString->length;
}

char *dcString_displayBytes(const dcString *_string)
{
    dcString *result = dcString_create();
    dcString_appendCharacter(result, '[');
    uint64_t i = 0;

    for (i = 0; i < _string->length; i++)
    {
        dcString_append(result,
                        "%d%s",
                        _string->string[i],
                        (i < _string->length - 1
                         ? " "
                         : ""));
    }

    dcString_appendCharacter(result, ']');
    return dcString_freeAndReturn(&result);
}

bool dcString_equals(const dcString *_left, const dcString *_right)
{
    return dcString_equalsCharArray(_left, _right->string);
}

bool dcString_equalsCharArray(const dcString *_left, const char *_right)
{
    return ((strlen(_left->string) == strlen(_right))
            && strncmp(_left->string, _right, strlen(_left->string)) == 0);
}

dcString *dcString_marshall(const dcString *_string, dcString *_stream)
{
    _stream = dcMarshaller_marshall(_stream, "u", NODE_STRING);
    return dcString_marshallCharArray(_string->string, _stream);
}

dcString *dcString_unmarshall(dcString *_stream)
{
    uint8_t type;
    dcString *result = NULL;

    if (dcMarshaller_unmarshall(_stream, "u", &type)
        && type == NODE_STRING)
    {
        char *unmarshalled = dcString_unmarshallCharArray(_stream);

        if (unmarshalled != NULL)
        {
            result = dcString_createWithString(unmarshalled, false);
        }
    }

    return result;
}

char *dcString_unmarshallCharArray(dcString *_stream)
{
    char *result = NULL;
    uint64_t length = 0;

    if (dcUnsignedInt64_unmarshall(_stream, &length)
        && dcString_hasLengthLeft(_stream, length))
    {
        result = (char *)dcMemory_allocateAndInitialize(length + 1);
        dcString_incrementIndex
            (_stream,
             dcMemory_copy(result, dcString_getFinger(_stream), length));
        result[length] = 0;
    }

    return result;
}

dcString *dcString_marshallCharArray(const char *_string, dcString *_stream)
{
    uint64_t length = (uint64_t)strlen(_string);
    _stream = dcUnsignedInt64_marshall(length, _stream);

    if (length > 0)
    {
        dcString_appendString(_stream, _string);
    }

    return _stream;
}

dcResult dcString_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    dcString *string = CAST_STRING(_node);
    return dcString_hashBytes((uint8_t*)string->string,
                              string->length,
                              _hashResult);
}

#define BODY (uint64_t)(sizeof(uint64_t) * 8)

static const uint64_t FIRST = (uint64_t)((BODY * 3) / 4);
static const uint64_t SECOND = (uint64_t)(BODY / 8);
static const uint64_t THIRD =
    ((uint64_t)(0xFFFFFFFF)                                         \
     << ((uint64_t)(sizeof(uint64_t) * 8) - (uint64_t)(BODY / 8)));

dcResult dcString_hashCharArray(const char *_string, dcHashType *_hashResult)
{
    return dcString_hashBytes((uint8_t*)_string,
                              strlen(_string),
                              _hashResult);
}

//
// The Peter J Weinberger Hash Algorithm
//
dcResult dcString_hashBytes(const uint8_t *_bytes,
                            uint64_t _length,
                            dcHashType *_hashResult)
{
    uint64_t i;
    uint64_t hash = 0;

    for (i = 0; i < _length; i++)
    {
        hash = (hash << SECOND) + _bytes[i];
        uint64_t test = hash & THIRD;

        if (test != 0)
        {
            hash = (( hash ^ (test >> FIRST)) & (~THIRD));
        }
    }

    *_hashResult = hash;
    return TAFFY_SUCCESS;
}

dcResult dcString_compareNode(dcNode *_first,
                              dcNode *_second,
                              dcTaffyOperator *_compareResult)
{
    int myCompareResult = strcmp(dcString_getString(_first),
                                 dcString_getString(_second));
    *_compareResult = (myCompareResult == 0
                       ? TAFFY_EQUALS
                       : (myCompareResult < 0
                          ? TAFFY_LESS_THAN
                          : TAFFY_GREATER_THAN));
    return TAFFY_SUCCESS;
}

char *dcString_freeAndReturn(dcString **_string)
{
    char *result = (*_string)->string;
    dcString_free(_string, DC_SHALLOW);
    return result;
}

bool dcString_hasLengthLeft(const dcString *_string, uint64_t _length)
{
    return (_string->index <= _string->length
            && ((_string->length - _string->index) >= _length));
}
