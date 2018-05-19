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

#include "dcDateClass.h"
#include "dcArray.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcKernelClass.h"
#include "dcMemory.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcYesClass.h"

const char * const sDateArgument[] = {"org.taffy.core.time.Date", NULL};

const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "day",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_day,
        gCFunctionArgument_none
    },
    {
        "#operator(==):",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_equalEqual,
        gCFunctionArgument_wild
    },
    {
        "fullDay",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_fullDay,
        gCFunctionArgument_none
    },
    {
        "fullMonth",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_fullMonth,
        gCFunctionArgument_none
    },
    {
        "#operator(>):",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_greaterThan,
        sDateArgument
    },
    {
        "#operator(>=):",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_greaterThanOrEqual,
        sDateArgument
    },
    {
        "hour",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_hour,
        gCFunctionArgument_none
    },
    {
        "init",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_init,
        gCFunctionArgument_none
    },
    {
        "#operator(<):",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_lessThan,
        sDateArgument
    },
    {
        "#operator(<=):",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_lessThanOrEqual,
        sDateArgument
    },
    {
        "minute",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_minute,
        gCFunctionArgument_none
    },
    {
        "month",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_month,
        gCFunctionArgument_none
    },
    {
        "second",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_second,
        gCFunctionArgument_none
    },
    {
        "year",
        SCOPE_DATA_PUBLIC,
        &dcDateClass_year,
        gCFunctionArgument_none
    },
    {
        0
    }
};

#define CAST_DATE_AUX(node) ((dcDateClassAux*)CAST_CLASS_AUX(node))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcDateClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         ("org.taffy.core.time",              // package name
          "Date",                             // class name
          "org.taffy.core.Object",            // super name
          CLASS_ATOMIC,                       // class flags
          NO_FLAGS,                           // scope data flags
          NULL,                               // meta methods
          sMethodWrappers,                    // methods
          NULL,                               // initialization function
          NULL,                               // deinitialization function
          &dcDateClass_allocateNode,          // allocate
          NULL,                               // deallocate
          NULL,                               // meta mark
          NULL,                               // mark
          &dcDateClass_copyNode,              // copy
          &dcDateClass_freeNode,              // free
          NULL,                               // register
          NULL,                               // marshall
          NULL,                               // unmarshall
          NULL));                             // set template
};

static dcDateClassAux *createAux(void)
{
    dcDateClassAux *aux =
        (dcDateClassAux *)dcMemory_allocate(sizeof(dcDateClassAux));
    aux->time = time(NULL);
    return aux;
}

void dcDateClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux();
}

dcNode *dcDateClass_createNode(bool _object)
{
    dcNode *classNode = dcClass_createBasicNode(sTemplate, _object);
    CAST_CLASS_AUX(classNode) = createAux();
    return classNode;
}

dcNode *dcDateClass_createObject(void)
{
    return dcDateClass_createNode(true);
}

//
// implement me
//
char *dcDateClass_displayNode(const dcNode *_date)
{
#ifdef TAFFY_WINDOWS
    return (char *)dcMemory_strdup("Unsupported");
#else
    char *buffer = (char *)dcMemory_allocateAndInitialize(80);
    time_t dateTime = CAST_DATE_AUX(_date)->time;
    struct tm *localTime = localtime(&dateTime);
    asctime_r(localTime, buffer);
    size_t bufferLength = strlen(buffer);

    // get rid of the newline //
    if (buffer[bufferLength - 1] == '\n')
    {
        buffer[bufferLength - 1] = 0;
    }

    return buffer;
#endif
}

void dcDateClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_CLASS_AUX(_to) = createAux();
    CAST_DATE_AUX(_to)->time = CAST_DATE_AUX(_from)->time;
}

void dcDateClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcDateClassAux *aux = CAST_DATE_AUX(_node);
    dcMemory_free(aux);
}

dcNode *dcDateClass_init(dcNode *_receiver, dcArray *_arguments)
{
    CAST_DATE_AUX(_receiver)->time = time(NULL);
    return _receiver;
}

dcNode *dcDateClass_lessThan(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();

    if (CAST_DATE_AUX(_receiver)->time
        < CAST_DATE_AUX(dcArray_get(_arguments, 0))->time)
    {
        result = dcYesClass_getInstance();
    }

    return result;
}

dcNode *dcDateClass_lessThanOrEqual(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();

    if (CAST_DATE_AUX(_receiver)->time
        <= CAST_DATE_AUX(dcArray_get(_arguments, 0))->time)
    {
        result = dcYesClass_getInstance();
    }

    return result;
}

dcNode *dcDateClass_greaterThan(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();

    if (CAST_DATE_AUX(_receiver)->time
        > CAST_DATE_AUX(dcArray_get(_arguments, 0))->time)
    {
        result = dcYesClass_getInstance();
    }

    return result;
}

dcNode *dcDateClass_greaterThanOrEqual(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();

    if (CAST_DATE_AUX(_receiver)->time
        >= CAST_DATE_AUX(dcArray_get(_arguments, 0))->time)
    {
        result = dcYesClass_getInstance();
    }

    return result;
}

dcNode *dcDateClass_equalEqual(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *candidate = dcClass_castNode(dcArray_get(_arguments, 0),
                                         sTemplate,
                                         false);
    return (candidate == NULL
            ? dcNoClass_getInstance()
            : (CAST_DATE_AUX(_receiver)->time == CAST_DATE_AUX(candidate)->time
               ? dcYesClass_getInstance()
               : dcNoClass_getInstance()));
}

static dcNode *dcDateClass_createTimeString(dcNode *_receiver,
                                            const char *_timeString)
{
    size_t daySize = 15;
    char *day = (char *)dcMemory_allocate(daySize);
    strftime(day,
             daySize,
             _timeString,
             localtime(&(CAST_DATE_AUX(_receiver)->time)));
    return dcNode_register(dcStringClass_createObject(day, false));
}

dcNode *dcDateClass_day(dcNode *_receiver, dcArray *_arguments)
{
    return dcDateClass_createTimeString(_receiver, "%a");
}

dcNode *dcDateClass_fullDay(dcNode *_receiver, dcArray *_arguments)
{
    return dcDateClass_createTimeString(_receiver, "%A");
}

dcNode *dcDateClass_month(dcNode *_receiver, dcArray *_arguments)
{
    return dcDateClass_createTimeString(_receiver, "%b");
}

dcNode *dcDateClass_fullMonth(dcNode *_receiver, dcArray *_arguments)
{
    return dcDateClass_createTimeString(_receiver, "%B");
}

dcNode *dcDateClass_year(dcNode *_receiver, dcArray *_arguments)
{
    return dcDateClass_createTimeString(_receiver, "%y");
}

dcNode *dcDateClass_hour(dcNode *_receiver, dcArray *_arguments)
{
    return dcDateClass_createTimeString(_receiver, "%H");
}

dcNode *dcDateClass_minute(dcNode *_receiver, dcArray *_arguments)
{
    return dcDateClass_createTimeString(_receiver, "%M");
}

dcNode *dcDateClass_second(dcNode *_receiver, dcArray *_arguments)
{
    return dcDateClass_createTimeString(_receiver, "%S");
}
