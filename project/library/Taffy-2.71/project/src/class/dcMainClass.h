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

#ifndef __DC_MAIN_CLASS_H__
#define __DC_MAIN_CLASS_H__

#include "dcDefines.h"

// building //
struct dcNode_t *dcMainClass_createNode(bool _object);
#define dcMainClass_createObject() dcMainClass_createNode(true)

struct dcNode_t *dcMainClass_getInstance(void);
void dcMainClass_resetInstance(void);

GET_TEMPLATE_FUNCTION(dcMainClass_getTemplate);
INITIALIZE_FUNCTION(dcMainClass_initialize);

#define MAIN_PACKAGE_NAME CORE_PACKAGE_NAME
#define MAIN_CLASS_NAME "Main"

#endif
