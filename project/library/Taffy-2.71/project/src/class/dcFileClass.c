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
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "dcFileClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcClass.h"
#include "dcCFunctionArgument.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcExceptions.h"
#include "dcFileManagement.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcYesClass.h"

#define FILE_TAFFY_FILE_NAME "src/modules/class/File.ty"

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "openForRead:",
        SCOPE_DATA_PUBLIC,
        &dcFileMetaClass_openForRead,
        gCFunctionArgument_string
    },
    {
        "openForReadWrite:",
        SCOPE_DATA_PUBLIC,
        &dcFileMetaClass_openForReadWrite,
        gCFunctionArgument_string
    },
    {
        "openForWrite:",
        SCOPE_DATA_PUBLIC,
        &dcFileMetaClass_openForWrite,
        gCFunctionArgument_string
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "close",
        SCOPE_DATA_PUBLIC,
        &dcFileClass_close,
        gCFunctionArgument_none
    },
    {
        "eachLine:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_BREAKTHROUGH),
        &dcFileClass_eachLine,
        gCFunctionArgument_procedure
    },
    {
        "filename",
        SCOPE_DATA_PUBLIC,
        &dcFileClass_filename,
        gCFunctionArgument_none
    },
    {
        "getAll",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONTAINER_LOOP),
        &dcFileClass_getAll,
        gCFunctionArgument_none
    },
    {
        "getLine",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFileClass_getLine,
        gCFunctionArgument_none
    },
    {
        "getRest",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONTAINER_LOOP),
        &dcFileClass_getRest,
        gCFunctionArgument_none
    },
    {
        "goTo:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFileClass_goTo,
        gCFunctionArgument_string
    },
    {
        "length",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFileClass_length,
        gCFunctionArgument_none
    },
    {
        "isClosed?",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFileClass_isClosed,
        gCFunctionArgument_none
    },
    {
        "isOpen?",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFileClass_isOpen,
        gCFunctionArgument_none
    },
    {
        "put:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFileClass_put,
        gCFunctionArgument_string
    },
    {
        "putLine:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFileClass_putLine,
        gCFunctionArgument_string
    },
    {
        "reset",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED),
        &dcFileClass_reset,
        gCFunctionArgument_none
    },
    {
        0
    }
};

#define CAST_FILE_AUX(_node_) ((dcFileClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcFileClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         ("org.taffy.core.io",                    // package name
          "File",                                 // class name
          "org.taffy.core.Object",                // super name
          CLASS_ABSTRACT,                         // class flags
          NO_FLAGS,                               // scope data flags
          sMetaMethodWrappers,                    // meta methods
          sMethodWrappers,                        // methods
          &dcFileClass_initialize,                // initialization function
          NULL,                                   // deinitialization function
          NULL,                                   // allocate
          NULL,                                   // deallocate
          NULL,                                   // meta mark
          &dcFileClass_markNode,                  // mark
          &dcFileClass_copyNode,                  // copy
          &dcFileClass_freeNode,                  // free
          NULL,                                   // register
          NULL,                                   // marshall
          NULL,                                   // unmarshall
          NULL));                                 // set template
};

static FILE *getFile(const dcNode *_node)
{
    return CAST_FILE_AUX(_node)->file;
}

static void setFilename(dcFileClassAux *_aux, const char *_filename)
{
    dcMemory_free(_aux->filename);
    _aux->filename = dcMemory_strdup(_filename);
}

static dcNode *dcInvalidFileModeExceptionClass_createObject(void)
{
    return dcStringEvaluator_evalString
        ("new org.taffy.core.exception.InvalidFileModeException",
         FILE_TAFFY_FILE_NAME,
         STRING_EVALUATOR_HANDLE_EXCEPTION);
}

void dcInvalidFileModeExceptionClass_throwObject(void)
{
    dcExceptions_throwObject(dcInvalidFileModeExceptionClass_createObject());
}

void dcFileOperationFailedExceptionClass_throwObject(const char *_name,
                                                     const char *_reason)
{
    dcExceptions_throwObject
        (dcStringEvaluator_evalFormat
         (FILE_TAFFY_FILE_NAME,
          NO_STRING_EVALUATOR_FLAGS,
          "FileOperationFailedException newWithName: \"%s\" "
          "reason: \"%s\"",
          _name,
          _reason));
}

void dcFileClass_initialize(void)
{
}

static dcFileClassAux *createAux(const char *_filename,
                                 const char *_mode,
                                 int _numericalMode)
{
    dcFileClassAux *aux =
        (dcFileClassAux *)dcMemory_allocate(sizeof(dcFileClassAux));
    aux->filename = NULL;
    setFilename(aux, _filename);
    aux->readLength = 20;
    aux->file = NULL;

    if (_mode != NULL)
    {
        if (strcmp(_mode, "r") == 0)
        {
            aux->mode = FILE_READ;
        }
        else if (strcmp(_mode, "w+") == 0)
        {
            aux->mode = FILE_READWRITE;
        }
        else if (strcmp(_mode, "w") == 0)
        {
            aux->mode = FILE_WRITE;
        }
        else
        {
            dcError_internal("invalid mode for file open");
        }
    }
    else
    {
        aux->mode = _numericalMode;
    }

    return aux;
}

void dcFileClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcFileClassAux *fromAux = CAST_FILE_AUX(_from);
    dcFileClassAux *toAux = createAux(fromAux->filename, NULL, fromAux->mode);
    CAST_CLASS_AUX(_to) = toAux;
}

void dcFileClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcFileClassAux *aux = CAST_FILE_AUX(_node);

    if (aux != NULL)
    {
        if (aux->file != NULL)
        {
            fclose(aux->file);
        }

        dcMemory_free(aux->filename);
        dcMemory_free(aux);
    }
}

dcNode *dcFileClass_createNode(const char *_filename,
                               const char *_mode,
                               bool _object)
{
    dcNode *fileClass = dcClass_createBasicNode(sTemplate, _object);
    CAST_CLASS_AUX(fileClass) = createAux(_filename, _mode, 0);
    return fileClass;
}

dcNode *dcFileClass_createObject(const char *_filename, const char *_mode)
{
    return dcFileClass_createNode(_filename, _mode, true);
}

void dcFileClass_markNode(dcNode *_node)
{
    // do nothing
}

dcNode *dcFileClass_init(dcNode *_receiver, dcArray *_arguments)
{
    FILE *file = CAST_FILE_AUX(_receiver)->file;

    if (file != NULL)
    {
        fclose(file);
    }

    setFilename(CAST_FILE_AUX(_receiver), "");
    return _receiver;
}

dcNode *dcFileClass_filename(dcNode *_receiver,
                             dcArray *_arguments)
{
    dcNode *filenameNode =
        dcStringClass_createObject(CAST_FILE_AUX(_receiver)->filename, true);
    dcNode_register(filenameNode);
    return filenameNode;
}

static dcNode *openAndAssign(dcArray *_arguments, const char *_mode)
{
    const char *filename = dcStringClass_getString(dcArray_get(_arguments, 0));
    dcNode *result = dcNode_register(dcFileClass_createObject(filename, _mode));
    FILE *file = fopen(filename, _mode);

    if (file != NULL)
    {
        CAST_FILE_AUX(result)->file = file;
        setFilename(CAST_FILE_AUX(result), filename);
    }
    else
    {
        result = NULL;
        dcFileOpenExceptionClass_throwObject(filename);
    }

    return result;
}

#define FILE_OPERATION(_name, _result)                          \
    errno = 0, fileOperation(_name, _result);

static dcNode *fileOperation(const char *_name, int _result)
{
    dcNode *result = NULL;

    if (_result == 0 && errno == 0)
    {
        result = dcYesClass_getInstance();
    }
    else
    {
        dcFileOperationFailedExceptionClass_throwObject(_name, strerror(errno));
    }

    errno = 0;
    return result;
}

static dcNode *makeDirectory(char *_directory)
{
    mode_t mode =
        (S_IRWXU     // read, write, and execute for the owner
         | S_IRWXG   // read, write, and execute for the group
         | S_IROTH   // read for other users
         | S_IXOTH); // execute for other users)
    return FILE_OPERATION(_directory, mkdir(_directory, mode));
}

dcNode *dcFileMetaClass_makeDirectory(dcNode *_receiver,
                                      dcArray *_arguments)
{
    char *directory = dcMemory_strdup
        (dcStringClass_getString(dcArray_get(_arguments, 0)));
    dcNode *result = dcYesClass_getInstance();
    char *finger = directory;
    size_t directoryLength = strlen(directory);

    for (finger = strchr(finger + 1, DIRECTORY_SEPARATOR);
         result != NULL
             && (finger == NULL
                 || ((size_t)(finger - directory) < directoryLength));
         finger = strchr(finger + 1, DIRECTORY_SEPARATOR))
    {
        if (finger == NULL)
        {
            result = makeDirectory(directory);
            break;
        }
        else
        {
            *finger = 0;
            result = makeDirectory(directory);
            *finger = DIRECTORY_SEPARATOR;
        }
    }

    dcMemory_free(directory);
    return result;
}

dcNode *dcFileMetaClass_deleteFile(dcNode *_receiver, dcArray *_arguments)
{
    const char *fileName = dcStringClass_getString(dcArray_get(_arguments, 0));
    return FILE_OPERATION(fileName, unlink(fileName));
}

dcNode *dcFileMetaClass_copyFromTo(dcNode *_receiver, dcArray *_arguments)
{
    //
    // from stack overflow
    // stackoverflow.com/questions/2180079/how-can-i-copy-a-file-on-unix-using-c
    //

    pid_t pid = fork();
    dcNode *result = dcYesClass_getInstance();
    //printf("trying to copy from: '%s' to: '%s'\n", from, to);

    if (pid == 0)
    {
        execl("/bin/cp",
              "/bin/cp",
              dcStringClass_getString(dcArray_get(_arguments, 0)), // from
              dcStringClass_getString(dcArray_get(_arguments, 1)), // to
              (char *)0);
    }
    else if (pid < 0)
    {
        result = dcNoClass_getInstance();
    }
    else
    {
        wait(NULL);
    }

    return result;
}

dcNode *dcFileMetaClass_deleteDirectory(dcNode *_receiver, dcArray *_arguments)
{
    const char *directory = dcStringClass_getString(dcArray_get(_arguments, 0));
    return FILE_OPERATION(directory, rmdir(directory));
}

dcNode *dcFileMetaClass_openForRead(dcNode *_receiver, dcArray *_arguments)
{
    return openAndAssign(_arguments, "r");
}

dcNode *dcFileMetaClass_openForWrite(dcNode *_receiver, dcArray *_arguments)
{
    return openAndAssign(_arguments, "w");
}

dcNode *dcFileMetaClass_openForReadWrite(dcNode *_receiver, dcArray *_arguments)
{
    return openAndAssign(_arguments, "w+");
}

dcNode *dcFileClass_eachLine(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *procedureNode = dcArray_get(_arguments, 0);
    dcMethodHeader *methodHeader =
        dcProcedureClass_getMethodHeader(procedureNode);
    dcList *blockArguments = dcMethodHeader_getArguments(methodHeader);
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    const size_t blockArgumentsSize = blockArguments->size;
    dcNode *result = _receiver;

    if (blockArgumentsSize == 1)
    {
        dcList *callArguments = dcList_create();
        dcNode *line = dcFileClass_getLine(_receiver, _arguments);
        dcNode *nilObject = dcNilClass_getInstance();

        while (line != nilObject)
        {
            dcList_setHead(callArguments, line);
            dcNode *blockResult =
                dcNodeEvaluator_evaluateProcedure(evaluator,
                                                  NULL,
                                                  procedureNode,
                                                  SCOPE_DATA_BREAKTHROUGH,
                                                  callArguments);

            if (blockResult == NULL)
            {
                result = NULL;
                break;
            }
            else if (!dcNodeEvaluator_canContinueEvaluating(evaluator))
            {
                break;
            }

            line = dcFileClass_getLine(_receiver, _arguments);
        }

        dcList_free(&callArguments, DC_SHALLOW);
    }
    else
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject
            (1, blockArgumentsSize);
        result = NULL;
    }

    return result;
}

dcNode *dcFileClass_isOpen(dcNode *_receiver, dcArray *_arguments)
{
    return (CAST_FILE_AUX(_receiver)->file == NULL
            ? dcNoClass_getInstance()
            : dcYesClass_getInstance());
}

dcNode *dcFileClass_close(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();

    if ((CAST_FILE_AUX(_receiver)->file))
    {
        fclose(CAST_FILE_AUX(_receiver)->file);
        CAST_FILE_AUX(_receiver)->file = NULL;
        result = dcYesClass_getInstance();
    }

    return result;
}

dcNode *dcFileClass_isClosed(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcFileClass_isOpen(_receiver, _arguments);

    if (result != NULL)
    {
        if (result == dcYesClass_getInstance())
        {
            result = dcNoClass_getInstance();
        }
        else
        {
            result = dcYesClass_getInstance();
        }
    }

    return result;
}

dcNode *dcFileClass_getLine(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcFileClassAux *aux = CAST_FILE_AUX(_receiver);

    if (aux->mode == FILE_WRITE)
    {
        dcInvalidFileModeExceptionClass_throwObject();
    }
    else
    {
        result = dcNilClass_getInstance();
        dcString *string =
            dcFileManagement_getLine(CAST_FILE_AUX(_receiver)->file);

        if (string != NULL)
        {
            result = dcNode_register
                (dcStringClass_createObject(string->string, false));
            dcString_free(&string, DC_SHALLOW);
        }
    }

    return result;
}

dcNode *dcFileClass_getAll(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *fileObject = _receiver;
    FILE *file = CAST_FILE_AUX(fileObject)->file;
    dcNode *result = dcNilClass_getInstance();

    if (file != NULL)
    {
        dcString *all = dcFileManagement_extractAllInputFromFile(file);
        result = (dcNode_register
                  (dcStringClass_createObject(all->string, false)));
        dcString_free(&all, DC_SHALLOW);
    }

    return result;
}

dcNode *dcFileClass_getRest(dcNode *_receiver, dcArray *_arguments)
{
    FILE *file = getFile(_receiver);
    dcString *fileContents = NULL;
    dcNode *result = dcNilClass_getInstance();

    if (file != NULL)
    {
        fileContents =
            dcFileManagement_extractAllInputFromFileFromCurrentPosition(file);

        if (fileContents != NULL)
        {
            result = dcStringClass_createObject(fileContents->string, false);
            dcNode_register(result);
        }
    }

    return result;
}

dcNode *dcFileClass_put(dcNode *_receiver, dcArray *_arguments)
{
    dcFileClassAux *aux = CAST_FILE_AUX(_receiver);
    dcNode *result = dcNoClass_getInstance();

    if (aux->mode == FILE_READ)
    {
        dcInvalidFileModeExceptionClass_throwObject();
        result = NULL;
    }
    else
    {
        if (aux->file != NULL)
        {
            fputs(dcStringClass_getString(dcArray_get(_arguments, 0)),
                  aux->file);
            result = dcYesClass_getInstance();
        }
    }

    return result;
}

dcNode *dcFileClass_putLine(dcNode *_receiver, dcArray *_arguments)
{
    dcFileClassAux *aux = CAST_FILE_AUX(_receiver);
    dcNode *result = dcNoClass_getInstance();

    if (aux->mode == FILE_READ)
    {
        dcInvalidFileModeExceptionClass_throwObject();
        result = NULL;
    }
    else
    {
        if (aux->file != NULL)
        {
            fputs(dcStringClass_getString(dcArray_get(_arguments, 0)),
                  aux->file);
            fputs("\n", aux->file);
            result = dcYesClass_getInstance();
        }
    }

    return result;
}

dcNode *dcFileClass_reset(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();
    FILE *file = CAST_FILE_AUX(_receiver)->file;

    if (file != NULL)
    {
        rewind(file);
        result = dcYesClass_getInstance();
    }

    return result;
}

dcNode *dcFileClass_goTo(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();
    FILE *file = CAST_FILE_AUX(_receiver)->file;

    if (file != NULL)
    {
        int32_t newPosition = -1;
        dcNode *number = dcArray_get(_arguments, 0);

        if (dcNumberClass_extractInt32s(number, &newPosition)
            && fseek(file, newPosition, SEEK_SET) == 0)
        {
            result = dcYesClass_getInstance();
        }
    }

    return result;
}

dcNode *dcFileClass_length(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    FILE *file = CAST_FILE_AUX(_receiver)->file;
    size_t currentPos = ftell(file);
    size_t size = 0;

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, currentPos, SEEK_SET);
    result = dcNumberClass_createObjectFromSizet(size);
    dcNode_register(result);
    return result;
}

char *dcFileClass_fileErrorToString(dcFileClassMode _error)
{
    char *result = NULL;
    return result;
}
