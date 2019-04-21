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

#include <string.h>

#include "dcCommandLineArgument.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcNode *dcCommandLineArgument_createNode(const char *_littleKey,
                                         const char *_bigKey)
{
    dcCommandLineArgument *argument =
        (dcCommandLineArgument *)
        dcMemory_allocateAndInitialize(sizeof(dcCommandLineArgument));

    if (_littleKey != NULL)
    {
        argument->littleKey = dcMemory_strdup(_littleKey);
    }

    argument->bigKey = dcMemory_strdup(_bigKey);
    argument->values = dcList_create();
    return dcNode_createWithGuts(NODE_COMMAND_LINE_ARGUMENT, argument);
}

void dcCommandLineArgument_freeNode(dcNode *_node, dcDepth _depth)
{
    dcCommandLineArgument *argument = CAST_COMMAND_LINE_ARGUMENT(_node);
    dcMemory_free(argument->littleKey);
    dcMemory_free(argument->bigKey);
    dcList_free(&argument->values, DC_DEEP);
    dcMemory_free(argument);
}

bool dcCommandLineArgument_equalsName(const dcCommandLineArgument *_argument,
                                      const char *_name)
{
    return (((_argument->littleKey == NULL
              && _name == NULL)
             || (_argument->littleKey != NULL
                 && _name != NULL
                 && strcmp(_argument->littleKey, _name) == 0))
            || (_name != NULL
                && strcmp(_argument->bigKey, _name) == 0));
}

dcResult dcCommandLineArgument_printNode(const dcNode *_node, dcString *_stream)
{
    dcCommandLineArgument *argument = CAST_COMMAND_LINE_ARGUMENT(_node);
    dcString_append(_stream,
                    "<<%s, %s, %lu>>",
                    (argument->littleKey == NULL
                     ? (const char *)"<none>"
                     : (const char *)argument->littleKey),
                    argument->bigKey,
                    argument->values->size);
    return TAFFY_SUCCESS;
}
