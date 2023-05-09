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

#include "dcCharacterGraph.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcCharacterGraph *dcCharacterGraph_create(uint32_t _height, uint32_t _width)
{
    dcCharacterGraph *result =
        (dcCharacterGraph *)dcMemory_allocate(sizeof(dcCharacterGraph));
    result->graph = (char **)dcMemory_allocate(sizeof(char *) * _height);

    uint32_t i;

    for (i = 0; i < _height; i++)
    {
        result->graph[i] = (char *)dcMemory_allocate(sizeof(char) * _width);
        memset(result->graph[i], ' ', _width);
    }

    result->height = _height;
    result->width = _width;
    result->index = 0;

    return result;
}

dcCharacterGraph *dcCharacterGraph_createFromString(const dcString *_string)
{
    return dcCharacterGraph_createFromCharString(_string->string);
}

dcCharacterGraph *dcCharacterGraph_createFromCharString(const char *_string)
{
    dcList *parts = dcLexer_splitString(_string, '\n');
    // find the max length
    uint64_t maxLength = 0;

    FOR_EACH_IN_LIST(parts, that)
    {
        maxLength = dcTaffy_max
            (dcString_getStringLength(CAST_STRING(that->object)), maxLength);
    }

    // TODO: account for overflow
    dcCharacterGraph *result = dcCharacterGraph_create(parts->size,
                                                       (int32_t)maxLength);
    uint32_t row;

    for (row = 0, that = parts->head; that != NULL; row++, that = that->next)
    {
        dcCharacterGraph_insertString(result,
                                      CAST_STRING(that->object),
                                      0,
                                      row);
    }

    dcList_free(&parts, DC_DEEP);
    return result;
}

dcString *dcCharacterGraph_convertToString(const dcCharacterGraph *_graph)
{
    uint32_t i;
    dcString *result = dcString_create();

    for (i = 0; i < _graph->height; i++)
    {
        uint32_t j;

        for (j = 0; j < _graph->width; j++)
        {
            dcString_appendCharacter(result, _graph->graph[i][j]);
        }

        if (i < _graph->height - 1)
        {
            dcString_appendCharacter(result, '\n');
        }
    }

    return result;
}

void dcCharacterGraph_appendString(dcCharacterGraph *_graph,
                                   const dcString *_string,
                                   int32_t _y)
{
    dcCharacterGraph_appendCharString(_graph, _string->string, _y);
}

void dcCharacterGraph_appendCharString(dcCharacterGraph *_graph,
                                       const char *_string,
                                       int32_t _y)
{
    dcCharacterGraph_insertCharacterString(_graph, _string, _graph->width, _y);
}

void dcCharacterGraph_insertString(dcCharacterGraph *_graph,
                                   const dcString *_string,
                                   int32_t _x,
                                   int32_t _y)
{
    dcCharacterGraph_insertCharacterString(_graph,
                                           _string->string,
                                           _x,
                                           _y);
}

void dcCharacterGraph_insertCharacterString(dcCharacterGraph *_graph,
                                            const char *_string,
                                            int32_t _x,
                                            int32_t _y)
{
    dcCharacterGraph_insertCharacterStringWithLength(_graph,
                                                     _string,
                                                     strlen(_string),
                                                     _x,
                                                     _y);
}

void dcCharacterGraph_insertCharacterStringWithLength(dcCharacterGraph *_graph,
                                                      const char *_string,
                                                      size_t _length,
                                                      int32_t _x,
                                                      int32_t _y)
{
    // TODO: account for overflow

    if (_x + _length >= _graph->width)
    {
        // don't store the null-terminator
        dcCharacterGraph_addColumns(_graph,
                                    ((uint32_t)(_x + _length)
                                     - _graph->width));
    }

    if (_y < 0)
    {
        dcCharacterGraph_prependRows(_graph, _y * -1);
        _y = 0;
    }
    else if (_y >= (int32_t)_graph->height)
    {
        dcCharacterGraph_addRows(_graph, (_y + 1) - _graph->height);
    }

    memcpy(_graph->graph[_y] + _x, _string, _length);
}

void dcCharacterGraph_insertRow(dcCharacterGraph *_graph,
                                int32_t _row)
{
    dcCharacterGraph_addRows(_graph,
                             (_row < 0
                              ? _row * -1
                              : (_row < (int32_t)_graph->height
                                 ? 1
                                 : (_row - (int32_t)_graph->height) + 1)));

    if (_row < 0)
    {
        _row = 0;
    }

    int32_t i;

    for (i = (int32_t)_graph->height - 2; i >= _row; i--)
    {
        memcpy(_graph->graph[i + 1], _graph->graph[i], _graph->width);
    }

    memset(_graph->graph[_row], ' ', _graph->width);
}

void dcCharacterGraph_free(dcCharacterGraph **_graph)
{
    if (_graph == NULL || *_graph == NULL)
    {
        return;
    }

    dcCharacterGraph *graph = *_graph;
    uint32_t i;

    for (i = 0; i < graph->height; i++)
    {
        dcMemory_free(graph->graph[i]);
    }

    dcMemory_free(graph->graph);
    dcMemory_free(graph);
}

void dcCharacterGraph_addRows(dcCharacterGraph *_graph, uint32_t _value)
{
    uint32_t newHeight = _graph->height + _value;
    uint32_t i;

    _graph->graph = (char **)dcMemory_realloc(_graph->graph,
                                              sizeof(char *) * newHeight);

    for (i = _graph->height; i < newHeight; i++)
    {
        _graph->graph[i] = (char *)(dcMemory_allocateAndInitialize
                                    (sizeof(char) * _graph->width));
        memset(_graph->graph[i], ' ', _graph->width);
    }

    _graph->height = newHeight;
}

void dcCharacterGraph_prependRows(dcCharacterGraph *_graph, uint32_t _value)
{
    uint32_t newHeight = _graph->height + _value;
    uint32_t i;

    _graph->graph = (char **)dcMemory_realloc(_graph->graph,
                                              sizeof(char *) * newHeight);

    for (i = _graph->height; i < newHeight; i++)
    {
        _graph->graph[i] = (char *)dcMemory_allocate(_graph->width);
    }

    _graph->height = newHeight;

    for (i = _graph->height - 1;
         ((int32_t)i - (int32_t)_value) >= 0;
         i--)
    {
        memcpy(_graph->graph[i], _graph->graph[i - _value], _graph->width);
    }

    for (i = 0; i < _value; i++)
    {
        memset(_graph->graph[i], ' ', _graph->width);
    }
}

void dcCharacterGraph_prependColumns(dcCharacterGraph *_graph, uint32_t _value)
{
    uint32_t newWidth = _graph->width + _value;
    uint32_t i;

    for (i = 0; i < _graph->height; i++)
    {
        _graph->graph[i] = (char *)dcMemory_realloc(_graph->graph[i], newWidth);
        memmove(_graph->graph[i] + _value, _graph->graph[i], _graph->width);
        memset(_graph->graph[i], ' ', _value);
    }

    _graph->width = newWidth;
}

void dcCharacterGraph_addColumns(dcCharacterGraph *_graph, uint32_t _value)
{
    uint32_t newWidth = _graph->width + _value;
    uint32_t i;

    for (i = 0; i < _graph->height; i++)
    {
        _graph->graph[i] = (char *)dcMemory_realloc(_graph->graph[i], newWidth);
        memset(_graph->graph[i] + _graph->width, ' ', newWidth - _graph->width);
    }

    _graph->width = newWidth;
}

void dcCharacterGraph_unshiftRows(dcCharacterGraph *_graph, uint32_t _value)
{
    uint32_t oldHeight = _graph->height;
    dcCharacterGraph_addRows(_graph, _value);
    uint32_t i;

    for (i = _graph->height - 1; i >= oldHeight; i--)
    {
        memcpy(_graph->graph[i], _graph->graph[i - _value], _graph->width);
    }

    for (i = 0; i < _value; i++)
    {
        uint32_t j;

        for (j = 0; j < _graph->width; j++)
        {
            _graph->graph[i][j] = ' ';
        }
    }
}

// _x, _y is the bottom left of _toInsert
// so it is pasted up and to the right
void dcCharacterGraph_insertCharacterGraphUp(dcCharacterGraph *_graph,
                                             int32_t _x,
                                             int32_t _y,
                                             const dcCharacterGraph *_toInsert)
{
    int32_t top = _y - ((int32_t)_toInsert->height - 1);

    if (top < 0)
    {
        dcCharacterGraph_prependRows(_graph, -top);

        if (_y < 0)
        {
            _y += _toInsert->height;
        }
        else
        {
            _y += -top;
        }
    }

    int32_t right = _x + _toInsert->width;

    if (right >= (int32_t)_graph->width)
    {
        dcCharacterGraph_addColumns(_graph, right - _graph->width);
    }

    int32_t i;

    for (i = 0; i < (int32_t)_toInsert->height; i++)
    {
        dcCharacterGraph_insertCharacterStringWithLength
            (_graph,
             _toInsert->graph[_toInsert->height - i - 1],
             _toInsert->width,
             _x,
             _y - i);
    }
}

// _x, _y is the upper-right of _toInsert
// so it is pasted down and to the right
void dcCharacterGraph_insertCharacterGraphDown
    (dcCharacterGraph *_graph,
     int32_t _x,
     int32_t _y,
     const dcCharacterGraph *_toInsert)
{
    if (_y < 0)
    {
        dcCharacterGraph_prependRows(_graph, -_y);
        _y = 0;
    }

    if (_x < 0)
    {
        dcCharacterGraph_prependColumns(_graph, -_x);
        _x = 0;
    }

    int32_t top = _y + (int32_t)_toInsert->height;

    if (top > (int32_t)_graph->height)
    {
        dcCharacterGraph_addRows(_graph, top - _graph->height);
    }

    int32_t right = _x + _toInsert->width;

    if (right >= (int32_t)_graph->width)
    {
        dcCharacterGraph_addColumns(_graph, right - _graph->width);
    }

    uint32_t i;

    for (i = 0; i < _toInsert->height; i++)
    {
        dcCharacterGraph_insertCharacterStringWithLength
            (_graph,
             _toInsert->graph[i],
             _toInsert->width,
             _x,
             _y + i);
    }
}

// a graph with height 1 becomes [graph], but with height > 1 becomes
// something much more
void dcCharacterGraph_addBraces(dcCharacterGraph *_graph)
{
    if (_graph->height == 1)
    {
        _graph->graph[0] = (char *)(dcMemory_realloc
                                    (_graph->graph[0],
                                     sizeof(char) * _graph->width + 2));
        memmove(_graph->graph[0] + 1, _graph->graph[0], _graph->width);
        _graph->graph[0][0] = '[';
        _graph->graph[0][_graph->width + 1] = ']';
        _graph->width += 2;
    }
    else
    {
        uint32_t i;

        for (i = 0; i < _graph->height; i++)
        {
            _graph->graph[i] =
                (char *)dcMemory_realloc(_graph->graph[i],
                                         sizeof(char)
                                         * _graph->width + 4);
        }

        _graph->width += 4;

        for (i = 0; i < _graph->height; i++)
        {
            memmove(_graph->graph[i] + 2, _graph->graph[i], _graph->width - 4);
            _graph->graph[i][0] = '|';
            _graph->graph[i][1] = ' ';
            _graph->graph[i][_graph->width - 2] = ' ';
            _graph->graph[i][_graph->width - 1] = '|';
        }

        // upper left
        _graph->graph[0][0] = '+';
        _graph->graph[0][1] = '-';

        // uppper right
        _graph->graph[0][_graph->width - 2] = '-';
        _graph->graph[0][_graph->width - 1] = '+';

        // bottom left
        _graph->graph[_graph->height - 1][0] = '+';
        _graph->graph[_graph->height - 1][1] = '-';

        // bottom right
        _graph->graph[_graph->height - 1][_graph->width - 2] = '-';
        _graph->graph[_graph->height - 1][_graph->width - 1] = '+';
    }
}

// a graph with height 1 becomes (graph), but with height > 1 becomes
// something much more
void dcCharacterGraph_addParens(dcCharacterGraph *_graph)
{
    if (_graph->height == 1)
    {
        _graph->graph[0] = (char *)(dcMemory_realloc
                                    (_graph->graph[0],
                                     sizeof(char) * _graph->width + 2));
        memmove(_graph->graph[0] + 1, _graph->graph[0], _graph->width);
        _graph->graph[0][0] = '(';
        _graph->graph[0][_graph->width + 1] = ')';
        _graph->width += 2;
    }
    else
    {
        uint32_t i;

        for (i = 0; i < _graph->height; i++)
        {
            _graph->graph[i] =
                (char *)dcMemory_realloc(_graph->graph[i],
                                         sizeof(char)
                                         * _graph->width + 4);
        }

        _graph->width += 4;

        for (i = 0; i < _graph->height; i++)
        {
            memmove(_graph->graph[i] + 2, _graph->graph[i], _graph->width - 4);
            _graph->graph[i][0] = '|';
            _graph->graph[i][1] = ' ';
            _graph->graph[i][_graph->width - 2] = ' ';
            _graph->graph[i][_graph->width - 1] = '|';
        }

        // upper left
        _graph->graph[0][0] = '/';

        // uppper right
        _graph->graph[0][_graph->width - 1] = '\\';

        // bottom left
        _graph->graph[_graph->height - 1][0] = '\\';

        // bottom right
        _graph->graph[_graph->height - 1][_graph->width - 1] = '/';
    }
}

void dcCharacterGraph_maybeAddParens(dcCharacterGraph *_graph)
{
    if (_graph->height > 3)
    {
        dcCharacterGraph_addParens(_graph);
    }
}

void dcCharacterGraph_appendCharacterGraph(dcCharacterGraph *_graph,
                                           const dcCharacterGraph *_toInsert)
{
    uint32_t i;

    if (_toInsert->width > _graph->width)
    {
        dcCharacterGraph_addColumns(_graph, _toInsert->width - _graph->width);
    }

    uint32_t oldHeight = _graph->height;
    dcCharacterGraph_addRows(_graph, _toInsert->height);

    for (i = 0; i < _toInsert->height; i++)
    {
        memcpy(_graph->graph[oldHeight + i],
               _toInsert->graph[i],
               _toInsert->width);
    }
}

char *dcCharacterGraph_display(const dcCharacterGraph *_graph)
{
    dcString *result = dcString_create();
    uint32_t i;

    for (i = 0; i < _graph->height; i++)
    {
        dcString_appendBytes(result, _graph->graph[i], _graph->width);
        dcString_appendCharacter(result, '\n');
    }

    return dcString_freeAndReturn(&result);
}

void dcCharacterGraph_appendMiddleAt(dcCharacterGraph *_graph,
                                     int32_t _x,
                                     const dcCharacterGraph *_toAppend)
{
    dcCharacterGraph_insertCharacterGraphDown
        (_graph,
         _x,
         (_graph->height / 2) - (_toAppend->height / 2),
         _toAppend);
}

void dcCharacterGraph_prependMiddle(dcCharacterGraph *_graph,
                                    const dcCharacterGraph *_toAppend)
{
    dcCharacterGraph_prependColumns(_graph, _toAppend->width);
    dcCharacterGraph_appendMiddleAt(_graph, 0, _toAppend);
    _graph->index = _graph->width;
}

void dcCharacterGraph_appendMiddle(dcCharacterGraph *_graph,
                                   const dcCharacterGraph *_toAppend)
{
    dcCharacterGraph_appendMiddleAt(_graph, _graph->index, _toAppend);
    _graph->index = _graph->width;
}

void dcCharacterGraph_appendMiddleUpAt(dcCharacterGraph *_graph,
                                       int32_t _x,
                                       const dcCharacterGraph *_toAppend)
{
    dcCharacterGraph_insertCharacterGraphUp
        (_graph,
         _x,
         (_graph->height / 2),
         _toAppend);
}

void dcCharacterGraph_appendMiddleUp(dcCharacterGraph *_graph,
                                   const dcCharacterGraph *_toAppend)
{
    dcCharacterGraph_appendMiddleUpAt(_graph, _graph->width, _toAppend);
}

void dcCharacterGraph_appendStringMiddle(dcCharacterGraph *_graph,
                                         const dcString *_string)
{
    dcCharacterGraph *graph = dcCharacterGraph_createFromString(_string);
    dcCharacterGraph_appendMiddle(_graph, graph);
    dcCharacterGraph_free(&graph);
}

void dcCharacterGraph_appendCharStringMiddle(dcCharacterGraph *_graph,
                                             const char *_string)
{
    dcCharacterGraph *graph = dcCharacterGraph_createFromCharString(_string);
    dcCharacterGraph_appendMiddle(_graph, graph);
    dcCharacterGraph_free(&graph);
}

void dcCharacterGraph_appendCharMiddle(dcCharacterGraph *_graph,
                                       char _character)
{
    dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
    graph->graph[0][0] = _character;
    dcCharacterGraph_appendMiddle(_graph, graph);
    dcCharacterGraph_free(&graph);
}

dcResult dcCharacterGraph_appendNodeMiddle(dcCharacterGraph *_graph,
                                           const dcNode *_node)
{
    dcCharacterGraph *graph = NULL;
    dcResult result = dcNode_prettyPrint(_node, &graph);

    if (result == TAFFY_SUCCESS)
    {
        dcCharacterGraph_appendMiddle(_graph, graph);
    }

    dcCharacterGraph_free(&graph);
    return result;
}

dcResult dcCharacterGraph_appendNodeMiddleParens(dcCharacterGraph *_graph,
                                                 const struct dcNode_t *_node)
{
    dcCharacterGraph *graph = NULL;
    dcResult result = dcNode_prettyPrint(_node, &graph);

    if (result == TAFFY_SUCCESS)
    {
        dcCharacterGraph_maybeAddParens(graph);
        dcCharacterGraph_appendMiddle(_graph, graph);
    }

    dcCharacterGraph_free(&graph);
    return result;
}
