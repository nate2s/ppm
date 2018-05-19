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

#include "dcSymbol.h"
#include "dcGraphData.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

#include <string.h>

dcSymbol *dcSymbol_create(const char *_symbol)
{
    dcSymbol *symbol = (dcSymbol *)dcMemory_allocate(sizeof(dcSymbol));
    symbol->symbol = dcMemory_strdup(_symbol);
    return symbol;
}

dcNode *dcSymbol_createNode(const char *_symbol)
{
    return dcGraphData_createNodeWithGuts
        (NODE_SYMBOL, dcSymbol_create(_symbol));
}

void dcSymbol_free(dcSymbol **_symbol, dcDepth _depth)
{
    dcMemory_free((*_symbol)->symbol);
    dcMemory_free(*_symbol);
}

void dcSymbol_freeNode(dcNode *_node, dcDepth _depth)
{
    dcSymbol_free(&CAST_SYMBOL(_node), _depth);
}

dcResult dcSymbol_printNode(const dcNode *_node, dcString *_string)
{
    dcString_append(_string, "'%s", dcSymbol_getSymbol(_node));
    return TAFFY_SUCCESS;
}

dcSymbol *dcSymbol_copy(const dcSymbol *_from, dcDepth _depth)
{
    return dcSymbol_create(_from->symbol);
}

void dcSymbol_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_SYMBOL(_to) = dcSymbol_copy(CAST_SYMBOL(_from), _depth);
}

const char *dcSymbol_getSymbol(const dcNode *_symbol)
{
    return CAST_SYMBOL(_symbol)->symbol;
}

bool dcSymbol_equals(const dcSymbol *_left, const dcSymbol *_right)
{
    return (strcmp(_left->symbol, _right->symbol) == 0);
}
