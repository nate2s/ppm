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

#include "dcDefines.h"

#ifndef TAFFY_WINDOWS
    #include <dirent.h>
#endif

#include <string.h>

#include "dcFileManagement.h"
#include "dcCommandLineArguments.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcSystem.h"
#include "dcVoid.h"

dcString *dcFileManagement_extractInputFromFileWithName(const char *_fileName)
{
    FILE *file = dcFileManagement_openFile(_fileName, "r+");
    dcString *result = NULL;

    if (file != NULL)
    {
        result = dcFileManagement_extractAllInputFromFile(file);
    }

    dcFileManagement_closeFile(file);
    return result;
}

dcString *dcFileManagement_extractAllInputFromFile(FILE *_file)
{
    dcString *result = NULL;
    long size = 0;

    if (_file != NULL && ! feof(_file))
    {
        fseek(_file, 0, SEEK_END);
        size = ftell(_file);
        rewind(_file);
        result = dcFileManagement_extractInputFromFile(_file, size);
        rewind(_file);
    }
    else
    {
        result = dcString_create();
    }

    return result;
}

dcString *dcFileManagement_extractAllInputFromFileFromCurrentPosition
    (FILE *_file)
{
    dcString *result = NULL;
    size_t size1 = 0;

    if (_file != NULL && ! feof(_file))
    {
        size1 = ftell(_file);
        fseek(_file, 0, SEEK_END);
        size_t size2 = -1 * (ftell(_file) - size1);
        fseek(_file, size2, SEEK_CUR);
        result = dcFileManagement_extractInputFromFile(_file, size2);
    }

    return result;
}

dcString *dcFileManagement_extractInputFromFile(FILE *_file, size_t _size)
{
    dcString *result = NULL;

    // size_t is bounded by max 32-bit integer
    if (_size > (uint32_t)-1)
    {
        return NULL;
    }

    if (_file != NULL && ! feof(_file) && _size > 0)
    {
        // +1 for EOF
        result = dcString_createWithLength((uint32_t)_size + 1);

        if (fread(result->string, _size, 1, _file) == 0)
        {
            dcString_free(&result, DC_DEEP);
            result = NULL;
        }
    }
    else
    {
        result = dcString_create();
    }

    return result;
}

typedef struct
{
    const char *mode;
    FILE *fileResult;
    const char *fileName;
} EachFileData;

static char *joinPath(const char *_left, const char *_right)
{
	return (strcmp(_left, "") == 0
			? dcMemory_strdup(_right)
			: dcLexer_sprintf("%s%c%s",
							  _left,
			                  dcFileManagement_getDirectorySeparator(),
			                  _right));
}

static dcResult eachIncludeDirectoryForFile(dcNode *_includeDirectory,
                                            dcNode *_token)
{
    EachFileData *fileData = (EachFileData*)CAST_VOID(_token);
    char *qualifiedFileName = joinPath(dcString_getString(_includeDirectory),
                                       fileData->fileName);
    fileData->fileResult = fopen(qualifiedFileName, fileData->mode);
    dcMemory_free(qualifiedFileName);
    return (fileData->fileResult == NULL
            ? TAFFY_SUCCESS   // try again
            : TAFFY_FAILURE); // we're done
}

FILE *dcFileManagement_openFile(const char *_fileName, const char *_mode)
{
    const dcList *includeDirectories = dcSystem_getIncludeDirectories();
    EachFileData data = {0};
    data.mode = _mode;
    data.fileResult = NULL;
    data.fileName = _fileName;
    dcNode *token = dcVoid_createNode(&data);
    dcList_each(includeDirectories, &eachIncludeDirectoryForFile, token);
    FILE *result = data.fileResult;
    dcNode_free(&token, DC_SHALLOW);
    return result;
}

typedef struct
{
    const char *directoryName;

#ifdef TAFFY_WINDOWS
    HANDLE directoryResult;
    WIN32_FIND_DATA fileFindData;
#else
    DIR *directoryResult;
#endif
} EachDirectoryData;

static dcResult eachIncludeDirectoryForDirectory(dcNode *_includeDirectory,
                                                 dcNode *_token)
{
    EachDirectoryData *directoryData = (EachDirectoryData*)CAST_VOID(_token);
    char *directoryName = joinPath(dcString_getString(_includeDirectory),
                                   directoryData->directoryName);

#ifdef TAFFY_WINDOWS
	char *newDirectoryName = dcLexer_sprintf("%s\\*", directoryName);
    directoryData->directoryResult =
		    FindFirstFile(newDirectoryName,
                          &directoryData->fileFindData);
    dcMemory_free(newDirectoryName);
#else
    directoryData->directoryResult = opendir(directoryName);
#endif

    dcMemory_free(directoryName);
    return (directoryData->directoryResult == NULL
            ? TAFFY_SUCCESS   // try again
            : TAFFY_FAILURE); // we're done
}

dcResult dcFileManagement_iterateOverFilesInDirectory
    (const char *_directoryName,
     const char *_wantedFileSuffix,
     dcFilenameCallback _callback,
     void *_token)
{
    EachDirectoryData directoryData = {0};
    directoryData.directoryName = _directoryName;
    directoryData.directoryResult = NULL;
    dcNode *dataWrapper = dcVoid_createNode(&directoryData);
    dcList_each(dcSystem_getIncludeDirectories(),
                &eachIncludeDirectoryForDirectory,
                dataWrapper);
    dcNode_free(&dataWrapper, DC_SHALLOW);

    dcResult result = TAFFY_SUCCESS;

    if (directoryData.directoryResult != NULL)
    {
#ifdef TAFFY_WINDOWS
        while (FindNextFile(directoryData.directoryResult,
                            &directoryData.fileFindData))
#else
        struct dirent *directoryEntry;

        while ((directoryEntry = readdir(directoryData.directoryResult))
               != NULL)
#endif
        {
            bool passed = true;

#ifdef TAFFY_WINDOWS
            const char *fileName = directoryData.fileFindData.cFileName;
#else
            const char *fileName = directoryEntry->d_name;
#endif

            if (strcmp(fileName, ".") == 0
                || strcmp(fileName, "..") == 0)
            {
                passed = false;
            }
            else if (_wantedFileSuffix != NULL)
            {
                dcList *parts =
                    dcLexer_splitString(fileName, '.');

                if (! (parts->size == 1
                       && strcmp(_wantedFileSuffix, "") == 0)
                    && (parts == NULL
                        || (strcmp(dcString_getString(dcList_getTail(parts)),
                                   _wantedFileSuffix)
                            != 0)))
                {
                    passed = false;
                }

                dcList_free(&parts, DC_DEEP);
            }

            if (passed)
            {
                char *path = joinPath(_directoryName, fileName);
                result = _callback(path, _token);
                dcMemory_free(path);
            }

            if (result == TAFFY_EXCEPTION || result == TAFFY_FAILURE)
            {
                break;
            }
        }

#ifdef TAFFY_WINDOWS
        FindClose(directoryData.directoryResult);
#else
        closedir(directoryData.directoryResult);
#endif
    }

    return result;
}

bool dcFileManagement_closeFile(FILE *_file)
{
    return fclose(_file);
}

bool dcFileManagement_fileExists(const char *_fileName)
{
    bool result = false;
    FILE *file = dcFileManagement_openFile(_fileName, "r");

    if (file != NULL)
    {
        result = true;
        fclose(file);
    }

    return result;
}

char dcFileManagement_getDirectorySeparator(void)
{
#ifdef TAFFY_WINDOWS
    return '\\';
#else
    return '/';
#endif
}

dcString *dcFileManagement_getLine(FILE *_file)
{
    dcString *line = dcString_create();
    char character = (char)fgetc(_file);
    bool gotReturn = false;

    // TODO: account for \r\n in windows
    while (character != EOF)
    {
        if (character == '\n')
        {
            gotReturn = true;
            break;
        }

        dcString_appendCharacter(line, character);
        character = (char)fgetc(_file);
    }

    if (gotReturn || strlen(line->string) > 0)
    {
        // shrink!
        size_t length = strlen(line->string);

        if (length > (uint32_t)-1)
        {
            dcString_free(&line, DC_DEEP);
        }
        else
        {
            line->length = (uint32_t)length;
        }
    }
    else
    {
        dcString_free(&line, DC_DEEP);
        line = NULL;
    }

    return line;
}
