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

#ifndef __DC_TAFFY_COMMAND_LINE_ARGUMENTS_H__
#define __DC_TAFFY_COMMAND_LINE_ARGUMENTS_H__

#include "dcDefines.h"

struct dcCommandLineArguments_t;

struct dcCommandLineArguments_t *dcTaffyCommandLineArguments_create(void);

struct dcCommandLineArguments_t *dcTaffyCommandLineArguments_parseAndCreate
    (int _argc,
     char **_argv);

struct dcCommandLineArguments_t *
dcTaffyCommandLineArguments_parseAndCreateWithFailure
    (int _argc,
     char **_argv,
     bool _failOnUnknownArgument);

bool dcTaffyCommandLineArguments_silencePluginWarnings
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_displayHelp
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_displayVersion
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_useColor
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_enableAutoComplete
    (const struct dcCommandLineArguments_t *_arguments);

const struct dcString_t *dcTaffyCommandLineArguments_getFilename
    (const struct dcCommandLineArguments_t *_arguments);

struct dcList_t *dcTaffyCommandLineArguments_getIncludeDirectories
    (const struct dcCommandLineArguments_t *_arguments);

struct dcList_t *dcTaffyCommandLineArguments_getPluginDirectories
    (const struct dcCommandLineArguments_t *_arguments);

struct dcList_t *dcTaffyCommandLineArguments_getArguments
    (const struct dcCommandLineArguments_t *_arguments);

int dcTaffyCommandLineArguments_getIndent
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_getDebug
    (const struct dcCommandLineArguments_t *_arguments);

struct dcList_t *dcTaffyCommandLineArguments_getLogs
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_getDisplayTime
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_getDisableGarbageCollection
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_getDisableFree
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_getNoSignalCatch
    (const struct dcCommandLineArguments_t *_arguments);

const struct dcString_t *dcTaffyCommandLineArguments_getCommandLine
    (const struct dcCommandLineArguments_t *_arguments);

const struct dcString_t *dcTaffyCommandLineArguments_getStopBootstrapAfterClass
    (const struct dcCommandLineArguments_t *_arguments);

int dcTaffyCommandLineArguments_getGarbageCollectorObjectTip
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_getTrackCreations
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_getTrackRegistration
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_getAlwaysGarbageCollect
    (const struct dcCommandLineArguments_t *_arguments);

bool dcTaffyCommandLineArguments_displayGarbageCollectorStatistics
    (const struct dcCommandLineArguments_t *_arguments);

int dcTaffyCommandLineArguments_getMaxFutureThreads
    (const struct dcCommandLineArguments_t *_arguments);

#endif
