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

#ifndef __DC_COMMAND_LINE_ARGUMENT_H__
#define __DC_COMMAND_LINE_ARGUMENT_H__

#include "dcDefines.h"

// A command line arguments element
struct dcCommandLineArgument_t
{
    // the little key, like: -i
    char *littleKey;

    // the big key, like: --includes
    char *bigKey;

    // the values on the right hand side of the argument
    struct dcList_t *values;

    // was the argument at least present?
    bool hit;
};

typedef struct dcCommandLineArgument_t dcCommandLineArgument;

struct dcNode_t *dcCommandLineArgument_createNode(const char *_littleKey,
                                                  const char *_bigKey);

bool dcCommandLineArgument_equalsName(const dcCommandLineArgument *_argument,
                                      const char *_name);

FREE_FUNCTION(dcCommandLineArgument_freeNode);
PRINT_FUNCTION(dcCommandLineArgument_printNode);

#endif
