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

#ifndef __DC_IO_CLASS_H__
#define __DC_IO_CLASS_H__

#include <stdlib.h> // for size_t
#include "dcDefines.h"

struct dcClassTemplate_t *dcIOClass_getTemplate(void);

// creating //
struct dcNode_t *dcIOClass_createNode(bool _object);
struct dcNode_t *dcIOClass_createObject(void);

struct dcNode_t *dcIOClass_getInstance(void);

void dcIOClass_resetInstance(void);

// output configuring //
typedef void (*OutputFunction)(const char *_output);
void dcIOClass_setOutputFunction(OutputFunction _function);
void dcIOClass_resetOutputFunction(void);

// printing //
size_t dcIOClass_printFormat(const char *_format, ...);

INITIALIZE_FUNCTION(dcIOClass_initialize);

// taffy methods //
TAFFY_C_METHOD(dcIOClass_flush);
TAFFY_C_METHOD(dcIOClass_getNonEmptyString);
TAFFY_C_METHOD(dcIOClass_put);
TAFFY_C_METHOD(dcIOClass_putWidth);
TAFFY_C_METHOD(dcIOClass_putLine);

// IO_PACKAGE_NAME defined in dcDefines.h
#define IO_CLASS_NAME "IO"

#endif
