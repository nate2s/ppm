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

#ifndef __DC_SEQUENCE_CLASS_H__
#define __DC_SEQUENCE_CLASS_H__

#include "dcDefines.h"

////////////////////////
// dcSequenceClassAux //
////////////////////////

// creating //
struct dcNode_t *dcSequenceClass_createNode(struct dcNode_t *_body,
                                            struct dcMethodHeader_t *_header,
                                            bool _object);

struct dcNode_t *dcSequenceClass_createObject(struct dcNode_t *_body,
                                              struct dcMethodHeader_t *_header);

// getting //
struct dcList_t *dcSequenceClass_getIndexFunctions
    (const struct dcNode_t *_node);

// standard functions //
COPY_FUNCTION(dcSequenceClass_copyNode);
FREE_FUNCTION(dcSequenceClass_freeNode);
GET_TEMPLATE_FUNCTION(dcSequenceClass_getTemplate);
INITIALIZE_FUNCTION(dcSequenceClass_initialize);
REGISTER_FUNCTION(dcSequenceClass_registerNode);
DO_GRAPH_OPERATION_FUNCTION(dcSequenceClass_doGraphOperation);

#define SEQUENCE_CLASS_MARSHALL_SIZE 3

// meta methods //
TAFFY_C_METHOD(dcSequenceMetaClass_createWithBlock);

// methods //
TAFFY_C_METHOD(dcSequenceClass_asString);
TAFFY_C_METHOD(dcSequenceClass_parenthesesDots);

#define SEQUENCE_PACKAGE_NAME MATHS_PACKAGE_NAME
#define SEQUENCE_CLASS_NAME "Sequence"

#endif
