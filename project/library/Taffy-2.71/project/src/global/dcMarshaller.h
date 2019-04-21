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

#ifndef __DC_MARSHALLER_H__
#define __DC_MARSHALLER_H__

//
// dcMarshaller.h
//
// The marshaller! :D
//
// Supported types in the format string:
//
// a -- array
// h -- hash
// H -- method header
// l -- list
// n -- node
// s -- char* string
// p -- pair
// t -- a node, but its entire tree
// i -- dcInt struct pointer
// c -- char / int8_t
// u -- uint8_t
// v -- uint16_t
// w -- uint32_t
// x -- int32_t
//

#include "dcDefines.h"

//
// Pass values in the extra arguments to dcMarshaller_marshall(), like:
//
// dcNode *myNode = dcHash_createNode();
// dcList *myList = dcList_create();
// char myChar = 'a';
//
// dcString *marshalledBytes =
//     dcMarshaller_marshall(_stream, "nlC", myNode, myList, myChar);
//
//
struct dcString_t *dcMarshaller_marshall(struct dcString_t *_stream,
                                         const char *_format,
                                         ...);
//
// dcMarshaller_unmarshall()
//
// Pass addresses of values in the extra arguments to
// dcMarshaller_unmarshall(), like:
//
// if (dcMarshaller_unmarshall(_stream, "nlC", &myNode, &myList, &myChar))
// {
//     // success!
// }
//
// where myNode, myList and myChar are defined as above in the description for
// dcMarshaller_marshall()
//
// Note: verify that _stream's index == 0 before calling
// dcMarshaller_unmarshall(). This is accomplished by
// dcString_resetIndex(_stream)
//
bool dcMarshaller_unmarshall(struct dcString_t *_stream,
                               const char *_format,
                               ...);

/**
 * @brief Same as unmarshall, but allow for no NULL objects
 */
bool dcMarshaller_unmarshallNoNull(struct dcString_t *_stream,
                                   const char *_format,
                                   ...);

#endif
