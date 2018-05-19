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

#ifndef __DC_NO_CLASS_H__
#define __DC_NO_CLASS_H__

#include "dcDefines.h"

// creating //
struct dcNode_t *dcNoClass_createNode(bool _object);
struct dcNode_t *dcNoClass_createObject(void);

struct dcNode_t *dcNoClass_getInstance(void);
void dcNoClass_resetInstance(void);
bool dcNoClass_isMe(const struct dcNode_t *_node);

GET_TEMPLATE_FUNCTION(dcNoClass_getTemplate);
INITIALIZE_FUNCTION(dcNoClass_initialize);

// taffy methods //
TAFFY_C_METHOD(dcNoClass_asString);
TAFFY_C_METHOD(dcNoClass_equals);
TAFFY_C_METHOD(dcNoClass_negate);

#define NO_PACKAGE_NAME CORE_PACKAGE_NAME
#define NO_CLASS_NAME "No"

#endif
