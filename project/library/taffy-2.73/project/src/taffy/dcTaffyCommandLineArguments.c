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

/////////////////////////////////////////////////////////
//
//  -i <include directories>
//  -f <extra files to include>
//  -s <interpret single line>
//
/////////////////////////////////////////////////////////

#include "dcTaffyCommandLineArguments.h"
#include "dcCommandLineArguments.h"
#include "dcContainers.h"
#include "dcGarbageCollector.h"
#include "dcNode.h"
#include "dcString.h"

dcCommandLineArguments *dcTaffyCommandLineArguments_create(void)
{
    char *arguments[] = {"taffy"};
    return dcTaffyCommandLineArguments_parseAndCreate(1, arguments);
}

dcCommandLineArguments *dcTaffyCommandLineArguments_parseAndCreate
    (int _argc,
     char **_argv)
{
    return dcTaffyCommandLineArguments_parseAndCreateWithFailure(_argc,
                                                                 _argv,
                                                                 true);
}

dcCommandLineArguments *dcTaffyCommandLineArguments_parseAndCreateWithFailure
    (int _argc,
     char **_argv,
     bool _failOnUnknownArgument)
{
    dcCommandLineArguments *arguments = dcCommandLineArguments_create();

    dcCommandLineArguments_registerArguments(arguments, "-a", "--arguments");
    dcCommandLineArguments_registerArguments(arguments, "-c", "--commandLine");
    dcCommandLineArguments_registerArguments(arguments, "-f", "--files");
    dcCommandLineArguments_registerArguments(arguments, "-h", "--help");
    dcCommandLineArguments_registerArguments(arguments, "-i", "--include");
    dcCommandLineArguments_registerArguments(arguments, "-p", "--plugins");
    dcCommandLineArguments_registerArguments(arguments, "-t", "--time");
    dcCommandLineArguments_registerArguments(arguments, "-v", "--version");
    dcCommandLineArguments_registerArguments(arguments, "-x", "--color");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--enable-auto-complete");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--set-max-future-threads");

    //
    // <debug> arguments
    //
    dcCommandLineArguments_registerArguments(arguments, "-d", "--debug");
    dcCommandLineArguments_registerArguments(arguments, "-l", "--log");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--always-garbage-collect");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--disable-free");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--disable-garbage-collection");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--garbage-collector-object-tip");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--garbage-collector-statistics");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--noSignalCatch");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--stop-bootstrap-after-class");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--track-creations");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--track-registrations");
    dcCommandLineArguments_registerArguments(arguments,
                                             NULL,
                                             "--silence-plugin-warnings");
    //
    // </debug>
    //

    // --files is the default argument
    dcCommandLineArguments_setDefaultArgument(arguments, "--files");

    if (! dcCommandLineArguments_parse(arguments,
                                       _argc,
                                       _argv,
                                       _failOnUnknownArgument))
    {
        dcCommandLineArguments_free(&arguments);
        arguments = NULL;
    }

    return arguments;
}

const dcString *dcTaffyCommandLineArguments_getFilename
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getString(_arguments, "--files");
}

dcList *dcTaffyCommandLineArguments_getIncludeDirectories
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getValues(_arguments, "--include");
}

dcList *dcTaffyCommandLineArguments_getPluginDirectories
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getValues(_arguments, "--plugins");
}

dcList *dcTaffyCommandLineArguments_getArguments
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getValues(_arguments, "--arguments");
}

int dcTaffyCommandLineArguments_getIndent
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getInt(_arguments, "--indent", 0);
}

int dcTaffyCommandLineArguments_getMaxFutureThreads
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getInt(_arguments,
                                         "--set-max-future-threads",
                                         -1);
}

bool dcTaffyCommandLineArguments_getDebug
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHits(_arguments, "--debug", "-d");
}

dcList *dcTaffyCommandLineArguments_getLogs
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getStringList(_arguments, "--log");
}

bool dcTaffyCommandLineArguments_getDisplayTime
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHits(_arguments, "--time", "-t");
}

const dcString *dcTaffyCommandLineArguments_getCommandLine
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getString(_arguments, "--commandLine");
}

const dcString *dcTaffyCommandLineArguments_getStopBootstrapAfterClass
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getString(_arguments,
                                            "--stop-bootstrap-after-class");
}

bool dcTaffyCommandLineArguments_getDisableGarbageCollection
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit(_arguments,
                                         "--disable-garbage-collection");
}

bool dcTaffyCommandLineArguments_getDisableFree
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit(_arguments, "--disable-free");
}

bool dcTaffyCommandLineArguments_getTrackCreations
    (const dcCommandLineArguments *_arguments)
{
    return (dcCommandLineArguments_getHit(_arguments, "--track-creations"));
}

bool dcTaffyCommandLineArguments_getTrackRegistration
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit(_arguments, "--track-registrations");
}

bool dcTaffyCommandLineArguments_getNoSignalCatch
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit(_arguments, "--noSignalCatch");
}

bool dcTaffyCommandLineArguments_displayHelp
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit(_arguments, "--help");
}

bool dcTaffyCommandLineArguments_displayVersion
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit(_arguments, "--version");
}

bool dcTaffyCommandLineArguments_useColor
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit(_arguments, "--color");
}

bool dcTaffyCommandLineArguments_enableAutoComplete
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit(_arguments, "--enable-auto-complete");
}

int dcTaffyCommandLineArguments_getGarbageCollectorObjectTip
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getInt(_arguments,
                                         "--garbage-collector-object-tip",
                                         GARBAGE_COLLECTOR_OBJECT_TIP);
}

bool dcTaffyCommandLineArguments_displayGarbageCollectorStatistics
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit
        (_arguments, "--garbage-collector-statistics");
}

bool dcTaffyCommandLineArguments_getAlwaysGarbageCollect
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit(_arguments,
                                         "--always-garbage-collect");
}

bool dcTaffyCommandLineArguments_silencePluginWarnings
    (const dcCommandLineArguments *_arguments)
{
    return dcCommandLineArguments_getHit(_arguments,
                                         "--silence-plugin-warnings");
}
