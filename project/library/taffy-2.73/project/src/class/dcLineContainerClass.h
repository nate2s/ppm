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

#ifndef __DC_LINE_CONTAINER_CLASS_H__
#define __DC_LINE_CONTAINER_CLASS_H__

#include "dcDefines.h"

struct dcNode_t *dcLineContainerClass_createNode(bool _object);

// standard functions //
GET_TEMPLATE_FUNCTION(dcLineContainerClass_getTemplate);
INITIALIZE_FUNCTION(dcLineContainerClass_initialize);

#define LINE_CONTAINER_PACKAGE_NAME CONTAINER_PACKAGE_NAME
#define LINE_CONTAINER_CLASS_NAME "LineContainer"

#endif
