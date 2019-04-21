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

#include <assert.h>
#include <string.h>

#include "dcCommandLineArgument.h"
#include "dcCommandLineArguments.h"
#include "dcError.h"
#include "dcFileManagement.h"
#include "dcHash.h"
#include "dcUnsignedInt32.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"
#include "dcSystem.h"

dcCommandLineArguments *dcCommandLineArguments_create(void)
{
    dcCommandLineArguments *arguments =
        (dcCommandLineArguments *)
        dcMemory_allocate(sizeof(dcCommandLineArguments));
    arguments->tinyArguments = dcHash_create();
    arguments->bigArguments = dcHash_create();
    arguments->defaultArgument = NULL;
    return arguments;
}

void dcCommandLineArguments_free(dcCommandLineArguments **_arguments)
{
    dcCommandLineArguments *arguments = *_arguments;
    dcHash_free(&arguments->tinyArguments, DC_SHALLOW);
    dcHash_free(&arguments->bigArguments, DC_DEEP);
    dcMemory_free(*_arguments);
}

void dcCommandLineArguments_setDefaultArgument
    (dcCommandLineArguments *_arguments,
     const char *_argument)
{
    dcCommandLineArgument *argument =
        dcCommandLineArguments_get(_arguments, _argument);
    assert(argument != NULL);
    _arguments->defaultArgument = argument;
}

void dcCommandLineArguments_registerArguments
    (dcCommandLineArguments *_arguments,
     const char *_tinyArgument,
     const char *_bigArgument)
{
    dcNode *argument =
        dcCommandLineArgument_createNode(_tinyArgument, _bigArgument);

    if (_tinyArgument != NULL)
    {
        assert(dcCommandLineArguments_get(_arguments, _tinyArgument)
               == NULL);
        dcHash_setValueWithStringKey(_arguments->tinyArguments,
                                     _tinyArgument,
                                     argument);
    }

    if (_bigArgument != NULL)
    {
        assert(dcCommandLineArguments_get(_arguments, _bigArgument)
               == NULL);
        assert(dcHash_setValueWithStringKey(_arguments->bigArguments,
                                            _bigArgument,
                                            argument)
               == TAFFY_SUCCESS);
    }
}

static dcCommandLineArgument *get
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName,
     bool _verifyExistence)
{
    dcNode *result = NULL;
    size_t length = strlen(_argumentName);

    if (length == 2)
    {
        // something like '-a'
        assert(dcHash_getValueWithStringKey(_arguments->tinyArguments,
                                            _argumentName,
                                            &result)
               != TAFFY_EXCEPTION);
    }
    else
    {
        assert(dcHash_getValueWithStringKey(_arguments->bigArguments,
                                            _argumentName,
                                            &result)
               != TAFFY_EXCEPTION);
    }

    assert(! _verifyExistence || result != NULL);
    return (result == NULL
            ? NULL
            : CAST_COMMAND_LINE_ARGUMENT(result));
}

dcCommandLineArgument *dcCommandLineArguments_get
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName)
{
    return get(_arguments, _argumentName, false);
}

dcList *dcCommandLineArguments_getValues
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName)
{
    return get(_arguments, _argumentName, true)->values;
}

bool dcCommandLineArguments_getHit(const dcCommandLineArguments *_arguments,
                                   const char *_argumentName)
{
    return get(_arguments, _argumentName, true)->hit;
}

bool dcCommandLineArguments_getHits(const dcCommandLineArguments *_arguments,
                                    const char *_argumentName1,
                                    const char *_argumentName2)
{
    return (get(_arguments, _argumentName1, true)->hit
            || get(_arguments, _argumentName2, true)->hit);
}

int dcCommandLineArguments_getInt(const dcCommandLineArguments *_arguments,
                                  const char *_argumentName,
                                  int _default)
{
    dcCommandLineArgument *argument = get(_arguments, _argumentName, true);
    return (argument->values->size > 0
            ? atoi(dcString_getString(dcList_getHead(argument->values)))
            : _default);
}

static dcNode *createIntNode(const dcNode *_value)
{
    return dcUnsignedInt32_createNode(atoi(dcString_getString(_value)));
}

static dcNode *createStringNode(const dcNode *_value)
{
    return dcNode_copy(_value, DC_DEEP);
}

static dcList *createList(const dcCommandLineArguments *_arguments,
                          const char *_argumentName,
                          dcNode *(*_nodeCreator)(const dcNode *_value))
{
    dcCommandLineArgument *argument = get(_arguments, _argumentName, true);
    dcList *result = NULL;

    if (argument != NULL)
    {
        result = dcList_create();

        FOR_EACH_IN_LIST(argument->values, that)
        {
            dcList_push(result, _nodeCreator(that->object));
        }
    }

    return result;
}

dcList *dcCommandLineArguments_getIntList
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName)
{
    return createList(_arguments, _argumentName, &createIntNode);
}

dcList *dcCommandLineArguments_getStringList
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName)
{
    return createList(_arguments, _argumentName, &createStringNode);
}

const dcString *dcCommandLineArguments_getString
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName)
{
    dcCommandLineArgument *argument = get(_arguments, _argumentName, true);
    return (argument->values->size > 0
            ? CAST_STRING(dcList_getHead(argument->values))
            : NULL);
}

bool dcCommandLineArguments_parse(dcCommandLineArguments *_arguments,
                                  int _argc,
                                  char **_argv,
                                  bool _failOnUnknownArgument)
{
    dcCommandLineArgument *current = _arguments->defaultArgument;
    bool result = true;
    dcString *argument = dcString_create();
    bool inString = false;
    int i;

    for (i = 1; i < _argc; i++)
    {
        if (_argv[i][0] == '-')
        {
            //
            // make sure it's a valid argument
            //
            dcCommandLineArgument *candidate =
                dcCommandLineArguments_get(_arguments, _argv[i]);

            if (candidate != NULL)
            {
                current = candidate;
                current->hit = true;
            }
            else if (_failOnUnknownArgument)
            {
                //
                // failure
                //
                result = false;
                break;
            }
        }
        else
        {
            if (current == NULL)
            {
                //
                // there's no default argument, and no argument was given,
                // confused, abort!
                //
                result = false;
                break;
            }

            size_t length = strlen(_argv[i]);
            current->hit = true;

            if (inString)
            {
                dcString_appendCharacter(argument, ' ');
            }

            size_t j;

            for (j = 0; j < length; j++)
            {
                if (_argv[i][j] == '"')
                {
                    inString = ! inString;
                }
            }

            dcString_appendString(argument, _argv[i]);

            if (! inString)
            {
                //
                // not in a string so we can stuff the argument
                //
                dcList_push(current->values,
                            dcString_createNodeWithString
                            (dcLexer_lexString(argument->string),
                             false));
                dcString_clear(argument);
            }
        }
    }

    dcString_free(&argument, DC_DEEP);
    return result;
}
