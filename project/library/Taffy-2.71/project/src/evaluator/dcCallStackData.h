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

#ifndef __DC_CALL_STACK_DATA_H__
#define __DC_CALL_STACK_DATA_H__

#include "dcDefines.h"

struct dcCallStackData_t
{
    uint32_t lineNumber;
    dcStringId filenameId;
    char *methodName;
};

typedef struct dcCallStackData_t dcCallStackData;

// creating //
dcCallStackData *dcCallStackData_create(const char *_methodName,
                                        dcStringId _filenameId,
                                        uint32_t _line);

struct dcNode_t *dcCallStackData_createNode(const char *_methodName,
                                            dcStringId _filenameId,
                                            uint32_t _line);

// comparing //
bool dcCallStackData_equals(const dcCallStackData *_left,
                            const dcCallStackData *_right);

const char *dcCallStackData_display(const dcCallStackData *_callStackData);

// standard functions //
COPY_FUNCTION(dcCallStackData_copyNode);
FREE_FUNCTION(dcCallStackData_freeNode);
PRINT_FUNCTION(dcCallStackData_printNode);

#endif
