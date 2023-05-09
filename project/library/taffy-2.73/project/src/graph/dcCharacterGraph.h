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

#ifndef __DC_CHARACTER_GRAPH_H__
#define __DC_CHARACTER_GRAPH_H__

#include "dcDefines.h"

struct dcCharacterGraph_t
{
    char **graph;
    uint32_t height;
    uint32_t width;

    // for appending only
    uint32_t index;
};

typedef struct dcCharacterGraph_t dcCharacterGraph;

dcCharacterGraph *dcCharacterGraph_create(uint32_t _height, uint32_t _width);
dcCharacterGraph *dcCharacterGraph_createFromString
    (const struct dcString_t *_string);

dcCharacterGraph *dcCharacterGraph_createFromCharString(const char *_string);

void dcCharacterGraph_free(dcCharacterGraph **_graph);

struct dcString_t *dcCharacterGraph_convertToString
    (const dcCharacterGraph *_graph);

void dcCharacterGraph_insertString(dcCharacterGraph *_graph,
                                   const struct dcString_t *_string,
                                   int32_t _x,
                                   int32_t _y);

void dcCharacterGraph_insertCharacterString(dcCharacterGraph *_graph,
                                            const char *_string,
                                            int32_t _x,
                                            int32_t _y);

void dcCharacterGraph_insertCharacterStringWithLength(dcCharacterGraph *_graph,
                                                      const char *_string,
                                                      size_t _length,
                                                      int32_t _x,
                                                      int32_t _y);

void dcCharacterGraph_insertRow(dcCharacterGraph *_graph,
                                int32_t _rightBefore);
void dcCharacterGraph_prependRows(dcCharacterGraph *_graph, uint32_t _value);
void dcCharacterGraph_prependColumns(dcCharacterGraph *_graph, uint32_t _value);
void dcCharacterGraph_addRows(dcCharacterGraph *_graph, uint32_t _value);
void dcCharacterGraph_addColumns(dcCharacterGraph *_graph, uint32_t _value);

void dcCharacterGraph_unshiftRows(dcCharacterGraph *_graph, uint32_t _value);

void dcCharacterGraph_insertCharacterGraphUp(dcCharacterGraph *_graph,
                                             int32_t _x,
                                             int32_t _y,
                                             const dcCharacterGraph *_toInsert);

void dcCharacterGraph_insertCharacterGraphDown
    (dcCharacterGraph *_graph,
     int32_t _x,
     int32_t _y,
     const dcCharacterGraph *_toInsert);

// append to the bottom
void dcCharacterGraph_appendCharacterGraph(dcCharacterGraph *_graph,
                                           const dcCharacterGraph *_toInsert);

void dcCharacterGraph_addParens(dcCharacterGraph *_graph);
void dcCharacterGraph_addBraces(dcCharacterGraph *_graph);

char *dcCharacterGraph_display(const dcCharacterGraph *_graph);

void dcCharacterGraph_appendString(dcCharacterGraph *_graph,
                                   const struct dcString_t *_string,
                                   int32_t _y);

void dcCharacterGraph_appendCharString(dcCharacterGraph *_graph,
                                       const char *_string,
                                       int32_t _y);

void dcCharacterGraph_prependMiddle(dcCharacterGraph *_graph,
                                    const dcCharacterGraph *_toAppend);

void dcCharacterGraph_appendMiddle(dcCharacterGraph *_graph,
                                   const dcCharacterGraph *_toAppend);

void dcCharacterGraph_appendMiddleAt(dcCharacterGraph *_graph,
                                     int32_t _x,
                                     const dcCharacterGraph *_toAppend);

void dcCharacterGraph_appendMiddleUpAt(dcCharacterGraph *_graph,
                                       int32_t _x,
                                       const dcCharacterGraph *_toAppend);

void dcCharacterGraph_appendMiddleUp(dcCharacterGraph *_graph,
                                     const dcCharacterGraph *_toAppend);

void dcCharacterGraph_appendStringMiddle(dcCharacterGraph *_graph,
                                         const struct dcString_t *_string);

void dcCharacterGraph_appendCharMiddle(dcCharacterGraph *_graph,
                                       char _character);

void dcCharacterGraph_appendCharStringMiddle(dcCharacterGraph *_graph,
                                             const char *_string);

dcResult dcCharacterGraph_appendNodeMiddle(dcCharacterGraph *_graph,
                                           const struct dcNode_t *_node);
dcResult dcCharacterGraph_appendNodeMiddleParens(dcCharacterGraph *_graph,
                                                 const struct dcNode_t *_node);

void dcCharacterGraph_shiftRow(dcCharacterGraph *_graph);

#endif
