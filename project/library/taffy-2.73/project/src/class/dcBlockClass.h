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

#ifndef __DC_BLOCK_CLASS_H__
#define __DC_BLOCK_CLASS_H__

#include "dcDefines.h"

// creating //
struct dcNode_t *dcBlockClass_createNode(struct dcNode_t *_body,
                                         struct dcList_t *_arguments,
                                         bool _object);

#define dcBlockClass_createObject(body, arguments) \
    dcBlockClass_createNode(body, arguments, true)

// getting //
struct dcNode_t *dcBlockClass_getBody(struct dcNode_t *_blockNode);

// standard functions //
GET_TEMPLATE_FUNCTION(dcBlockClass_getTemplate);
DO_GRAPH_OPERATION_FUNCTION(dcBlockClass_doGraphOperation);
IS_ME_FUNCTION(dcBlockClass_isMe);

// taffy c methods //
TAFFY_C_METHOD(dcBlockClass_asString);
TAFFY_C_METHOD(dcBlockClass_call);
TAFFY_C_METHOD(dcBlockClass_callWith);

#define BLOCK_PACKAGE_NAME CORE_PACKAGE_NAME
#define BLOCK_CLASS_NAME "Block"

#endif
