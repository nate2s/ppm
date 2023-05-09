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

#ifndef __DC_OBJECT_CLASS_H__
#define __DC_OBJECT_CLASS_H__

#include "dcDefines.h"

// creating //
struct dcNode_t *dcObjectClass_createNode(void);

// taffy c methods //
TAFFY_C_METHOD(dcObjectClass_address);
TAFFY_C_METHOD(dcObjectClass_allMethods);
TAFFY_C_METHOD(dcObjectClass_allObjects);
TAFFY_C_METHOD(dcObjectClass_asString);
TAFFY_C_METHOD(dcObjectClass_attach);
TAFFY_C_METHOD(dcObjectClass_castAs);
TAFFY_C_METHOD(dcObjectClass_class);
TAFFY_C_METHOD(dcObjectClass_className);
TAFFY_C_METHOD(dcObjectClass_copy);
TAFFY_C_METHOD(dcObjectClass_equals);
TAFFY_C_METHOD(dcObjectClass_getVariable);
TAFFY_C_METHOD(dcObjectClass_hash);
TAFFY_C_METHOD(dcObjectClass_hasMethod);
TAFFY_C_METHOD(dcObjectClass_init);
TAFFY_C_METHOD(dcObjectClass_initWithFields);
TAFFY_C_METHOD(dcObjectClass_kindOf);
TAFFY_C_METHOD(dcObjectClass_objects);
TAFFY_C_METHOD(dcObjectClass_methods);
TAFFY_C_METHOD(dcObjectClass_perform);
TAFFY_C_METHOD(dcObjectClass_performWith);
TAFFY_C_METHOD(dcObjectClass_prettyPrint);
TAFFY_C_METHOD(dcObjectClass_printObjects);
TAFFY_C_METHOD(dcObjectClass_setValueForObject);
TAFFY_C_METHOD(dcObjectClass_setVariable);
TAFFY_C_METHOD(dcObjectClass_super);
TAFFY_C_METHOD(dcObjectClass_variableExists);

GET_TEMPLATE_FUNCTION(dcObjectClass_getTemplate);

#define OBJECT_PACKAGE_NAME CORE_PACKAGE_NAME
#define OBJECT_CLASS_NAME "Object"

#endif
