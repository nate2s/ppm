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

#ifndef __DC_EXCEPTIONS_H__
#define __DC_EXCEPTIONS_H__

#include <stdio.h>

#include "dcDefines.h"

void dcExceptions_create(void);

void dcNeedSliceExceptionClass_throwObject(void);
void dcFinalClassUpdateExceptionClass_throwObject(void);
void dcInconsistentClassUpdateExceptionClass_throwObject(void);

void dcInvalidHashValueExceptionClass_throwObject(struct dcNode_t *_value);

struct dcNode_t *dcInvalidHashValueExceptionClass_createObject
    (struct dcNode_t *_value);

void dcInvalidLibraryExceptionClass_throwObject(const char *_libraryName);

struct dcNode_t *dcInvalidLibraryExceptionClass_createObject
    (const char *_libraryname);

struct dcNode_t *dcLocalToGlobalConversionExceptionClass_createObject
    (const char *_objectName);

void dcLocalToGlobalConversionExceptionClass_throwObject
    (const char *_objectName);

void dcInvalidNumberArgumentsExceptionClass_throwObject(size_t _expected,
                                                        size_t _given);

struct dcNode_t *dcInvalidNumberArgumentsExceptionClass_createObject
    (size_t _expected,
     size_t _given);

void dcOperationOnFunctionOfNoArgumentsExceptionClass_throwObject(void);

void dcUnidentifiedMethodExceptionClass_throwObject
    (const char *_className, const char *_methodName);

struct dcNode_t *dcUnidentifiedMethodExceptionClass_createObject
    (const char *_className,
     const char *_methodName);

struct dcNode_t *dcUnidentifiedObjectExceptionClass_createObject
    (const char *_objectName);

struct dcNode_t *dcUnidentifiedObjectExceptionClass_createObjectWithReason
    (const char *_objectName,
     const char *_reason);

void dcUnidentifiedObjectExceptionClass_throwObject(const char *_objectName);

void dcUnidentifiedObjectExceptionClass_throwObjectWithReason
    (const char *_objectName,
     const char *_reason);

void dcUnidentifiedClassExceptionClass_throwObject(const char *_objectName);

void dcIndexOutOfBoundsExceptionClass_throwObject(int64_t _index);
void dcIndexOutOfBoundsExceptionClass_throwObjectFromNode
    (struct dcNode_t *_node);
bool dcIndexOutOfBoundsExceptionClass_checkThrow(uint64_t _index,
                                                 uint64_t _limit);

struct dcNode_t *dcInvalidMatrixSizeExceptionClass_createObject
    (size_t _wantedRows,
     size_t _wantedColumns,
     size_t _haveRows,
     size_t _haveColumns);

void dcInvalidMatrixSizeExceptionClass_throwObject(size_t _wantedRows,
                                                   size_t _wantedColumns,
                                                   size_t _haveRows,
                                                   size_t _haveColumns);

void dcInvalidCastExceptionClass_throwObject(const char *_from,
                                             const char *_to);
struct dcNode_t *dcInvalidCastExceptionClass_createObject(const char *_from,
                                                          const char *_to);

struct dcNode_t *dcInvalidIndexesExceptionClass_createObject
    (struct dcNode_t *_indexes);
void dcInvalidIndexesExceptionClass_throwObject(struct dcNode_t *_indexes);

void dcInvalidSynchronizerExceptionClass_throwObject(struct dcNode_t *_value);

struct dcNode_t *dcInvalidSynchronizerExceptionClass_createObject
    (struct dcNode_t *_value);

void dcInvalidComparisonResultExceptionClass_throwObject
    (struct dcNode_t *_value);

struct dcNode_t *dcInvalidComparisonResultExceptionClass_createObject
    (struct dcNode_t *_value);

void dcFileOpenExceptionClass_throwObject(const char *_filename);

struct dcNode_t *dcFileOpenExceptionClass_createObject
    (const char *_filename);

void dcFileWriteExceptionClass_throwObject(const char *_filename,
                                           const char *_error);

struct dcNode_t *dcLibraryOpenExceptionClass_createObject
    (const char *_libraryName,
     const char *_error);

void dcLibraryOpenExceptionClass_throwObject(const char *_libraryName,
                                             const char *_error);

struct dcNode_t *dcNeedIntegerExceptionClass_createObject
    (struct dcNode_t *_value);

void dcNeedIntegerExceptionClass_throwObject(struct dcNode_t *_value);

void dcNeedDoubleExceptionClass_throwObject(struct dcNode_t *_value);

struct dcNode_t *dcNeedDoubleExceptionClass_createObject
    (struct dcNode_t *_value);

struct dcNode_t *dcNeedPositiveIntegerExceptionClass_createObject
    (struct dcNode_t *_value);

void dcNeedPositiveIntegerExceptionClass_throwObject(struct dcNode_t *_value);

struct dcNode_t *dcNeedByteExceptionClass_createObject
    (struct dcNode_t *_value);

void dcNeedByteExceptionClass_throwObject(struct dcNode_t *_value);

struct dcNode_t *dcPrecisionTooSmallExceptionClass_createObject
    (struct dcNode_t *_value);

void dcPrecisionTooSmallExceptionClass_throwObject(struct dcNode_t *_value);

void dcUnmarshallFailureExceptionClass_throwObject(struct dcNode_t *_value);

struct dcNode_t *dcUnmarshallFailureExceptionClass_createObject
    (struct dcNode_t *_value);

void dcDivideByZeroExceptionClass_throwObject(void);
struct dcNode_t *dcDivideByZeroExceptionClass_createObject(void);

void dcNoRegisteredConnectorsExceptionClass_throwObject(void);
struct dcNode_t *dcNoRegisteredConnectorsExceptionClass_createObject(void);

void dcMoreThanOnePackageExceptionClass_throwObject(const char *_packageName);

void dcInvalidMarshalledDataExceptionClass_throwObject(void);
struct dcNode_t *dcInvalidMarshalledDataExceptionClass_createObject(void);

struct dcNode_t *dcInvalidConnectorExceptionClass_createObject
    (const char *_connectorName);
void dcInvalidConnectorExceptionClass_throwObject(const char *_connectorName);

void dcSocketSendFailureExceptionClass_throwObject(void);
struct dcNode_t *dcSocketSendFailureExceptionClass_createObject(void);

void dcAbstractClassInstantiationExceptionClass_throwObject
    (const char *_className);
struct dcNode_t *dcAbstractClassInstantiationExceptionClass_createObject
    (const char *_className);

void dcConstantRedefinitionExceptionClass_throwObject
    (const char *_identifierName);
struct dcNode_t *dcConstantRedefinitionExceptionClass_createObject
    (const char *_identifierName);

void dcEmptyListExceptionClass_throwObject(void);

void dcCInputExceptionClass_throwObject(void);

void dcStackOverflowExceptionClass_throwObject(void);

struct dcNode_t *dcAbortExceptionClass_createObject(const char *_why);
void dcAbortExceptionClass_throwObject(const char *_why);

struct dcNode_t *dcUserGeneratedAbortSignalExceptionClass_createObject
    (const char *_why);
void dcUserGeneratedAbortSignalExceptionClass_throwObject(const char *_why);

void dcDerivationExceptionClass_throwObject(void);

void dcImportFailedExceptionClass_throwObject(const char *_packageName);
void dcReturnWithNoCallStackExceptionClass_throwObject(void);
void dcBreakWithoutALoopExceptionClass_throwObject(void);

void dcNeedMetaClassExceptionClass_throwObject(void);

void dcPluginInstantiationExceptionClass_throwObject(const char *_reason);

void dcSingletonInstantiationExceptionClass_throwObject(const char *_className);
void dcNonConstantUseOfConstantExceptionClass_throwObject
    (const char *_className);

void dcUnsupportedMathOperationExceptionClass_throwObject(const char *_reason);
void dcNeedVectorException_throwObject(void);
void dcInvalidSuperClassExceptionClass_throwObject(const char *_superName);

void dcDeadlockExceptionClass_throwObject(void);
void dcBlockNotSingularExceptionClass_throwObject(void);
void dcAssertFailedExceptionClass_throwObject(void);

void dcExceptions_throwObject(struct dcNode_t *_node);

#endif
