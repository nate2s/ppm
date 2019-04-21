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

#ifndef __DC_SYMBOLCLASS_H__
#define __DC_SYMBOLCLASS_H__

#include "dcDefines.h"

//////////////////////
// dcSymbolClassAux //
//////////////////////

// string is a pointer to a global store //
struct dcSymbolClassAux_t
{
    char *string;
};

typedef struct dcSymbolClassAux_t dcSymbolClassAux;

///////////////////
// dcSymbolClass //
///////////////////

// creating //
struct dcNode_t *dcSymbolClass_createNode(const char *_string, bool _object);
struct dcNode_t *dcSymbolClass_createObject(const char *_string);

// getting //
const char *dcSymbolClass_getString_helper(const struct dcNode_t *_symbol);

// comparing //
bool dcSymbolClass_compareToString(const struct dcNode_t *_symbol,
                                   const char *_string);

bool dcSymbolClass_isMe(const struct dcNode_t *_node);

// standard functions //
ALLOCATE_FUNCTION(dcSymbolClass_allocateNode);
FREE_FUNCTION(dcSymbolClass_freeNode);
GET_TEMPLATE_FUNCTION(dcSymbolClass_getTemplate);
COPY_FUNCTION(dcSymbolClass_copyNode);
MARSHALL_FUNCTION(dcSymbolClass_marshallNode);
UNMARSHALL_FUNCTION(dcSymbolClass_unmarshallNode);

// taffy meta methods //
TAFFY_C_METHOD(dcSymbolMetaClass_new);

// taffy methods //
TAFFY_C_METHOD(dcSymbolClass_asString);
TAFFY_C_METHOD(dcSymbolClass_getString);

#define SYMBOL_PACKAGE_NAME CORE_PACKAGE_NAME
#define SYMBOL_CLASS_NAME "Symbol"

#endif
