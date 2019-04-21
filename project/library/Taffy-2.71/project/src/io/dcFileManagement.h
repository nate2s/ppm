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

#ifndef __DC_FILE_MANAGEMENT_H__
#define __DC_FILE_MANAGEMENT_H__

#include <stdio.h>
#include <stdlib.h>

#include "dcDefines.h"
#include "dcString.h"

// extracting //
struct dcString_t *dcFileManagement_extractAllInputFromFile
    (FILE *_fileName);
struct dcString_t *dcFileManagement_extractInputFromFile
    (FILE *_fileName, size_t _size);
struct dcString_t *dcFileManagement_extractAllInputFromFileFromCurrentPosition
    (FILE *_file);
struct dcString_t *dcFileManagement_extractInputFromFileWithName
    (const char *file);

// opening //
FILE *dcFileManagement_openFile(const char *_fileName, const char *_mode);

// closing //
bool dcFileManagement_closeFile(FILE *_file);

// existence //
bool dcFileManagement_fileExists(const char *_fileName);

// return TAFFY_SUCCESS to continue iteration
// return TAFFY_FAILURE or TAFFY_EXCEPTION to stop iteration
typedef dcResult (*dcFilenameCallback)(const char *_filename, void *_token);

dcResult dcFileManagement_iterateOverFilesInDirectory
    (const char *_directoryName,
     const char *_wantedFileSuffix,
     dcFilenameCallback _callback,
     void *_token);

char dcFileManagement_getDirectorySeparator(void);

struct dcString_t *dcFileManagement_getLine(FILE *_file);

#endif
