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

#ifndef __DC_FILE_CLASS_H__
#define __DC_FILE_CLASS_H__

#include "dcDefines.h"
#include <stdio.h>

enum dcFileClassMode_e
{
    FILE_READ,
    FILE_READWRITE,
    FILE_WRITE
};

typedef uint8_t dcFileClassMode;

////////////////////
// dcFileClassAux //
////////////////////

struct dcFileClassAux_t
{
    char *filename;
    FILE *file;
    dcFileClassMode mode;
    int readLength;
};

typedef struct dcFileClassAux_t dcFileClassAux;

/////////////////
// dcFileClass //
/////////////////

// creating //
struct dcNode_t *dcFileClass_createNode(const char *_filename,
                                        const char *_mode,
                                        bool _object);
struct dcNode_t *dcFileClass_createObject(const char *_filename,
                                          const char *_mode);

char *dcFileClass_fileErrorToString(dcFileClassMode _error);

struct dcNode_t *dcFileClass_processFileError(struct dcNode_t *_caller,
                                              dcFileClassMode _error);

void dcFileClass_setFile(struct dcNode_t *_fileNode, FILE *_file);

// standard functions //
ALLOCATE_FUNCTION(dcFileClass_allocateNode);
COPY_FUNCTION(dcFileClass_copyNode);
FREE_FUNCTION(dcFileClass_freeNode);
GET_TEMPLATE_FUNCTION(dcFileClass_getTemplate);
INITIALIZE_FUNCTION(dcFileClass_initialize);
MARK_FUNCTION(dcFileClass_markNode);

// taffy methods //
TAFFY_C_METHOD(dcFileMetaClass_openForRead);
TAFFY_C_METHOD(dcFileMetaClass_openForReadWrite);
TAFFY_C_METHOD(dcFileMetaClass_openForWrite);

TAFFY_C_METHOD(dcFileClass_close);
TAFFY_C_METHOD(dcFileClass_eachLine);
TAFFY_C_METHOD(dcFileClass_filename);
TAFFY_C_METHOD(dcFileClass_get);
TAFFY_C_METHOD(dcFileClass_getAll);
TAFFY_C_METHOD(dcFileClass_getLine);
TAFFY_C_METHOD(dcFileClass_getRest);
TAFFY_C_METHOD(dcFileClass_getUntil);
TAFFY_C_METHOD(dcFileClass_goTo);
TAFFY_C_METHOD(dcFileClass_init);
TAFFY_C_METHOD(dcFileClass_isClosed);
TAFFY_C_METHOD(dcFileClass_isOpen);
TAFFY_C_METHOD(dcFileClass_length);
TAFFY_C_METHOD(dcFileClass_open);
TAFFY_C_METHOD(dcFileClass_openForRead);
TAFFY_C_METHOD(dcFileClass_openForWrite);
TAFFY_C_METHOD(dcFileClass_openForReadWrite);
TAFFY_C_METHOD(dcFileClass_put);
TAFFY_C_METHOD(dcFileClass_putLine);
TAFFY_C_METHOD(dcFileClass_reset);
TAFFY_C_METHOD(dcFileClass_setFilename);

// meta class methods //
TAFFY_C_METHOD(dcFileMetaClass_copyFromTo);
TAFFY_C_METHOD(dcFileMetaClass_deleteDirectory);
TAFFY_C_METHOD(dcFileMetaClass_deleteFile);
TAFFY_C_METHOD(dcFileMetaClass_forEachInListingDo);
TAFFY_C_METHOD(dcFileMetaClass_makeDirectory);
TAFFY_C_METHOD(dcFileMetaClass_open);
TAFFY_C_METHOD(dcFileMetaClass_openForRead);
TAFFY_C_METHOD(dcFileMetaClass_openForWrite);
TAFFY_C_METHOD(dcFileMetaClass_openForReadWrite);

#endif
