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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "dcCommandLineArguments.h"
#include "dcError.h"
#include "dcIOClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcLog.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"
#include "dcTaffyCommandLineArguments.h"

static dcLogType sLogTypes = NO_LOG_TYPE;

static const char * const sLogTypeNames[] =
{
    "garbage-collector",
    "garbage-collector-tick",
    "kernel",
    "lexer",
    "lexer-search",
    "node-evaluator",
    "parser",
    "class-manager",
    "flat-arithmetic",
    "flat-arithmetic-derivation",
    "flat-arithmetic-integration",
    "flat-arithmetic-parse",
    "flat-arithmetic-substitution",
    "flat-arithmetic-substitution-cache",
    "flat-arithmetic-merge",
    "flat-arithmetic-verbose",
    "flat-arithmetic-factor",
    "flat-arithmetic-remove",
    "flat-arithmetic-choose",
    "flat-arithmetic-tick",
    "flat-arithmetic-test",
    "flat-arithmetic-divide",
    "flat-arithmetic-cancel",
    "flat-arithmetic-trigonometry",
    "flat-arithmetic-shrink",
    "flat-arithmetic-solve-for-x",
    "flat-arithmetic-quadratic",
    "flat-arithmetic-derive-cache",
    "flat-arithmetic-shrink-iteration"
};

static const char *getLogTypeName(dcLogType _type)
{
    size_t i = (float)log(_type) / (float)log(2) + 1;
    dcError_assert(i < dcTaffy_countOf(sLogTypeNames));
    return sLogTypeNames[i];
}

bool dcLog_configureFromString(const char *_type, bool _yesno)
{
    size_t i;
    bool result = false;

    for (i = 0; i < dcTaffy_countOf(sLogTypeNames); i++)
    {
        if (strcmp(sLogTypeNames[i], _type) == 0)
        {
            dcLog_configure(1 << i, _yesno);
            result = true;
            break;
        }
    }

    if (! result)
    {
        dcIOClass_printFormat("Invalid log type: %s\n", _type);
    }

    return result;
}

bool dcLog_configureFromList(const dcList *_logs, bool _yesno)
{
    dcListIterator *it = dcList_createHeadIterator(_logs);
    dcNode *type;
    bool result = true;

    while (result
           && ((type = dcListIterator_getNext(it))
               != NULL))
    {
        result = dcLog_configureFromString(dcString_getString(type), _yesno);
    }

    dcListIterator_free(&it);
    return result;
}

void dcLog_configure(dcLogType _type, bool _yesno)
{
    if (_yesno)
    {
        sLogTypes |= _type;
    }
    else
    {
        sLogTypes &= ~_type;
    }
}

bool dcLog_configureFromCommandLineArguments(dcCommandLineArguments *_arguments)
{
    dcList *logs = dcTaffyCommandLineArguments_getLogs(_arguments);
    bool result = true;

    if (logs != NULL)
    {
        result = dcLog_configureFromList(logs, true);
        dcList_free(&logs, DC_DEEP);
    }

    return result;
}

bool dcLog_isEnabled(dcLogType _type)
{
    return ((sLogTypes & _type) != 0);
}

static void logFormat(dcLogType _type,
                      const char *_format,
                      va_list _list,
                      bool _printType)
{
    if (_printType)
    {
        dcIOClass_printFormat("[%s]: ", getLogTypeName(_type));
    }

    char *output = dcLexer_sprintfWithVaList(_format, _list, NULL);
    dcIOClass_printFormat("%s", output);
    dcMemory_free(output);
}

void dcLog_log(dcLogType _type, const char *_format, ...)
{
    if (dcLog_isEnabled(_type))
    {
        va_list list;
        va_start(list, _format);
        logFormat(_type, _format, list, true);
    }
}

void dcLog_logLine(dcLogType _type, const char *_format, ...)
{
    if (dcLog_isEnabled(_type))
    {
        va_list list;
        va_start(list, _format);
        logFormat(_type, _format, list, true);
        dcLog_append(_type, "\n");
    }
}

void dcLog_append(dcLogType _type, const char *_format, ...)
{
    if (dcLog_isEnabled(_type))
    {
        va_list list;
        va_start(list, _format);
        logFormat(_type, _format, list, false);
    }
}
