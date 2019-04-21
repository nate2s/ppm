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

#include "dcCommandLineArguments.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcExceptions.h"
#include "dcLexer.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcSystem.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcMemory.h"

#define EXCEPTIONS_TAFFY_FILE_NAME "src/class/Exceptions.ty"

#define EVAL_STRING(_string)                                        \
    dcStringEvaluator_evalString(_string,                           \
                                 EXCEPTIONS_TAFFY_FILE_NAME,        \
                                 STRING_EVALUATOR_HANDLE_EXCEPTION)

#define EVAL_FORMAT(_format, ...)                                       \
    dcStringEvaluator_evalFormat(EXCEPTIONS_TAFFY_FILE_NAME,            \
                                 STRING_EVALUATOR_HANDLE_EXCEPTION,     \
                                 _format,                               \
                                 __VA_ARGS__)

void dcExceptions_throwObject(dcNode *_node)
{
    // sanity, don't clobber an exception
    TAFFY_DEBUG(dcError_assert(dcSystem_getCurrentNodeEvaluator()->exception
                               == NULL));

    dcNodeEvaluator_setException(dcSystem_getCurrentNodeEvaluator(),
                                 _node,
                                 true);
}

void dcFinalClassUpdateExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING("new org.taffy.core.exception.FinalClassUpdateException"));
}

void dcInconsistentClassUpdateExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING
         ("new org.taffy.core.exception.InconsistentClassUpdateException"));
}

void dcNeedSliceExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING("new org.taffy.core.exception.NeedSliceException"));
}

void dcNonConstantUseOfConstantExceptionClass_throwObject
    (const char *_className)
{
    dcExceptions_throwObject
        (EVAL_FORMAT
         ("[org.taffy.core.exception.NonConstantUseOfConstantException "
          "createObject: \"%s\"]",
          _className));
}

void dcInvalidHashValueExceptionClass_throwObject(dcNode *_value)
{
    dcExceptions_throwObject
        (dcInvalidHashValueExceptionClass_createObject(_value));
}

dcNode *dcInvalidHashValueExceptionClass_createObject(dcNode *_value)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.InvalidHashValueException",
         "newWithValue:",
         _value,
         NULL);
}

void dcInvalidLibraryExceptionClass_throwObject(const char *_libraryName)
{
    return dcExceptions_throwObject(dcInvalidLibraryExceptionClass_createObject
                                    (_libraryName));
}

dcNode *dcLocalToGlobalConversionExceptionClass_createObject
    (const char *_objectName)
{
    return EVAL_FORMAT
        ("org.taffy.core.exception.LocalToGlobalConversionException "
         "createObjectWithName: \"%s\"",
         _objectName);
}

void dcLocalToGlobalConversionExceptionClass_throwObject
    (const char *_objectName)
{
    return (dcExceptions_throwObject
            (dcLocalToGlobalConversionExceptionClass_createObject
             (_objectName)));
}

dcNode *dcInvalidLibraryExceptionClass_createObject(const char *_libraryName)
{
    return EVAL_FORMAT
        ("org.taffy.core.exception.InvalidLibraryException "
         "newWithLibraryName: \"%s\" ",
         _libraryName);
}

dcNode *dcInvalidConnectorExceptionClass_createObject(const char *_name)
{
    return EVAL_FORMAT
        ("org.taffy.core.exception.InvalidConnectorException "
         "new: \"%s\"",
         _name);
}

void dcInvalidConnectorExceptionClass_throwObject(const char *_name)
{
    dcExceptions_throwObject
        (dcInvalidConnectorExceptionClass_createObject(_name));
}

void dcNeedIntegerExceptionClass_throwObject(dcNode *_value)
{
    dcExceptions_throwObject(dcNeedIntegerExceptionClass_createObject(_value));
}

dcNode *dcNeedIntegerExceptionClass_createObject(dcNode *_value)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.NeedIntegerException",
         "newObject:",
         _value,
         NULL);
}

void dcNeedDoubleExceptionClass_throwObject(dcNode *_value)
{
    dcExceptions_throwObject(dcNeedDoubleExceptionClass_createObject(_value));
}

dcNode *dcNeedDoubleExceptionClass_createObject(dcNode *_value)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.NeedDoubleException",
         "newObject:",
         _value,
         NULL);
}

void dcNeedPositiveIntegerExceptionClass_throwObject(dcNode *_value)
{
    dcExceptions_throwObject
        (dcNeedPositiveIntegerExceptionClass_createObject
         (_value));
}

dcNode *dcNeedPositiveIntegerExceptionClass_createObject(dcNode *_value)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.NeedPositiveIntegerException",
         "newObject:",
         _value,
         NULL);
}

void dcNoRegisteredConnectorsExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (dcNoRegisteredConnectorsExceptionClass_createObject());
}

dcNode *dcNoRegisteredConnectorsExceptionClass_createObject(void)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.NoRegisteredConnectorsException",
         "newObject",
         NULL);
}

void dcMoreThanOnePackageExceptionClass_throwObject(const char *_packageName)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    uint32_t classesSave = evaluator->onlyEvaluateClasses;
    evaluator->onlyEvaluateClasses = 0;

    dcExceptions_throwObject
        (EVAL_FORMAT
         ("org.taffy.core.exception.MoreThanOnePackageException "
          "newObject: \"%s\"",
          _packageName));

    evaluator->onlyEvaluateClasses = classesSave;
}

void dcInvalidMarshalledDataExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (dcInvalidMarshalledDataExceptionClass_createObject());
}

dcNode *dcInvalidMarshalledDataExceptionClass_createObject(void)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.InvalidMarshalledDataException",
         "newObject",
         NULL);
}

void dcNeedByteExceptionClass_throwObject(dcNode *_value)
{
    dcExceptions_throwObject(dcNeedByteExceptionClass_createObject(_value));
}

dcNode *dcNeedByteExceptionClass_createObject(dcNode *_value)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.NeedByteException",
         "newObject:",
         _value,
         NULL);
}

void dcUnmarshallFailureExceptionClass_throwObject(dcNode *_value)
{
    dcExceptions_throwObject
        (dcUnmarshallFailureExceptionClass_createObject(_value));
}

dcNode *dcUnmarshallFailureExceptionClass_createObject(dcNode *_value)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.UnmarshallFailureException",
         "newObject:",
         _value,
         NULL);
}

dcNode *dcPrecisionTooSmallExceptionClass_createObject(dcNode *_value)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.PrecisionTooSmallException",
         "newObject:",
         _value,
         NULL);
}

void dcPrecisionTooSmallExceptionClass_throwObject(dcNode *_value)
{
    dcExceptions_throwObject
        (dcPrecisionTooSmallExceptionClass_createObject(_value));
}

void dcOperationOnFunctionOfNoArgumentsExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING
         ("new org.taffy.core.exception."
          "OperationOnFunctionOfNoArgumentsException"));
}

void dcInvalidNumberArgumentsExceptionClass_throwObject(size_t _expected,
                                                        size_t _given)
{
    dcExceptions_throwObject(dcInvalidNumberArgumentsExceptionClass_createObject
                             (_expected, _given));
}

dcNode *dcInvalidNumberArgumentsExceptionClass_createObject
    (size_t _expected, size_t _given)
{
    return EVAL_FORMAT
        ("org.taffy.core.exception.InvalidNumberArgumentsException "
         "expected: " SIZE_T_PRINTF " given: " SIZE_T_PRINTF,
         _expected,
         _given);
}

void dcEmptyListExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING("new org.taffy.core.exception.EmptyListException"));
}

void dcConstantRedefinitionExceptionClass_throwObject
    (const char *_identifierName)
{
    dcExceptions_throwObject(dcConstantRedefinitionExceptionClass_createObject
                             (_identifierName));
}

dcNode *dcConstantRedefinitionExceptionClass_createObject
    (const char *_identifierName)
{
    return EVAL_FORMAT
        ("org.taffy.core.exception.ConstantRedefinitionException "
         "createObject: \"%s\"",
         _identifierName);
}

void dcAbstractClassInstantiationExceptionClass_throwObject
    (const char *_className)
{
    dcExceptions_throwObject
        (dcAbstractClassInstantiationExceptionClass_createObject(_className));
}

dcNode *dcAbstractClassInstantiationExceptionClass_createObject
    (const char *_className)
{
    return EVAL_FORMAT
        ("org.taffy.core.exception.AbstractClassInstantiationException "
         "createObject: \"%s\"",
         _className);
}

void dcUnidentifiedMethodExceptionClass_throwObject(const char *_className,
                                                    const char *_methodName)
{
    dcExceptions_throwObject(dcUnidentifiedMethodExceptionClass_createObject
                             (_className, _methodName));
}

dcNode *dcUnidentifiedMethodExceptionClass_createObject(const char *_className,
                                                        const char *_methodName)
{
    // no infinite loops!
    //if (strcmp(_className, "UnidentifiedMethodException") == 0)
    //{
    //    fprintf(stderr,
    //            "Infinite loop detected %s, %s\n",
    //             _className,
    //            _methodName);
    //    assert(false);
    //}

    return EVAL_FORMAT
        ("org.taffy.core.exception.UnidentifiedMethodException "
         "methodName: \"%s\" className: \"%s\"",
         _methodName,
         _className);
}

dcNode *dcUnidentifiedObjectExceptionClass_createObject(const char *_objectName)
{
    return (dcUnidentifiedObjectExceptionClass_createObjectWithReason
            (_objectName, NULL));
}

dcNode *dcUnidentifiedObjectExceptionClass_createObjectWithReason
    (const char *_objectName,
     const char *_reason)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    uint32_t classesSave = evaluator->onlyEvaluateClasses;
    evaluator->onlyEvaluateClasses = 0;
    const char *className =
        "org.taffy.core.exception.UnidentifiedObjectException";

    // no infinite loops!
    if (strcmp(_objectName, className) == 0)
    {
        fprintf(stderr,
                "Fatal error: Infinite loop for unidentified object: %s\n",
                _reason);
        assert(false);
    }

    dcNode *result = NULL;

    if (_reason == NULL)
    {
        result = EVAL_FORMAT("%s createObject: \"%s\"",
                             className,
                             _objectName);
    }
    else
    {
        result = EVAL_FORMAT("%s createObject: \"%s\" reason: \"%s\"",
                             className,
                             _objectName,
                             _reason);
    }

    evaluator->onlyEvaluateClasses = classesSave;
    return result;
}

void dcUnidentifiedObjectExceptionClass_throwObject(const char *_objectName)
{
    dcExceptions_throwObject(dcUnidentifiedObjectExceptionClass_createObject
                             (_objectName));
}

void dcUnidentifiedObjectExceptionClass_throwObjectWithReason
    (const char *_objectName,
     const char *_reason)
{
    dcExceptions_throwObject
        (dcUnidentifiedObjectExceptionClass_createObjectWithReason
         (_objectName, _reason));
}

void dcIndexOutOfBoundsExceptionClass_throwObject(int64_t _index)
{
    dcExceptions_throwObject
        (EVAL_FORMAT
         ("org.taffy.core.exception.IndexOutOfBoundsException index: %lld",
          _index));
}

void dcIndexOutOfBoundsExceptionClass_throwObjectFromNode(dcNode *_node)
{
    dcExceptions_throwObject
        (dcNodeEvaluator_callEvaluatedMethod
         (dcSystem_getCurrentNodeEvaluator(),
          "org.taffy.core.exception.IndexOutOfBoundsException",
          "index:",
          _node,
          NULL));
}

bool dcIndexOutOfBoundsExceptionClass_checkThrow(uint64_t _index,
                                                 uint64_t _limit)
{
    bool result = false;

    if (_index >= _limit)
    {
        result = true;
        dcIndexOutOfBoundsExceptionClass_throwObject(_index);
    }

    return result;
}

dcNode *dcInvalidMatrixSizeExceptionClass_createObject(size_t _wantedRows,
                                                       size_t _wantedColumns,
                                                       size_t _haveRows,
                                                       size_t _haveColumns)
{
    return EVAL_FORMAT
        ("org.taffy.core.exception.InvalidMatrixSizeException wantedRows: " SIZE_T_PRINTF " "
         "wantedColumns: " SIZE_T_PRINTF " "
         "haveRows: " SIZE_T_PRINTF " "
         "haveColumns: " SIZE_T_PRINTF,
         _wantedRows,
         _wantedColumns,
         _haveRows,
         _haveColumns);
}

void dcInvalidMatrixSizeExceptionClass_throwObject(size_t _wantedRows,
                                                   size_t _wantedColumns,
                                                   size_t _haveRows,
                                                   size_t _haveColumns)
{
    dcExceptions_throwObject
        (dcInvalidMatrixSizeExceptionClass_createObject(_wantedRows,
                                                        _wantedColumns,
                                                        _haveRows,
                                                        _haveColumns));
}

void dcInvalidCastExceptionClass_throwObject(const char *_from,
                                             const char *_to)
{
    dcExceptions_throwObject
        (dcInvalidCastExceptionClass_createObject(_from, _to));
}

dcNode *dcInvalidCastExceptionClass_createObject(const char *_from,
                                                 const char *_to)
{
    return EVAL_FORMAT
        ("org.taffy.core.exception.InvalidCastException "
         "newFrom: \"%s\" to: \"%s\"",
         _from,
         _to,
         NULL);
}

void dcDivideByZeroExceptionClass_throwObject(void)
{
    dcExceptions_throwObject(dcDivideByZeroExceptionClass_createObject());
}

dcNode *dcDivideByZeroExceptionClass_createObject(void)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.DivideByZeroException",
         "create",
         NULL);
}

dcNode *dcInvalidIndexesExceptionClass_createObject(dcNode *_indexes)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.InvalidIndexesException",
         "new:",
         _indexes,
         NULL);
}

void dcInvalidIndexesExceptionClass_throwObject(dcNode *_indexes)
{
    dcExceptions_throwObject
        (dcInvalidIndexesExceptionClass_createObject(_indexes));
}

void dcInvalidSynchronizerExceptionClass_throwObject(dcNode *_value)
{
    dcExceptions_throwObject
        (dcInvalidSynchronizerExceptionClass_createObject(_value));
}

dcNode *dcInvalidSynchronizerExceptionClass_createObject(dcNode *_value)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.InvalidSynchronizerException",
         "new:",
         _value,
         NULL);
}

void dcInvalidComparisonResultExceptionClass_throwObject(dcNode *_value)
{
    dcExceptions_throwObject
        (dcInvalidComparisonResultExceptionClass_createObject(_value));
}

dcNode *dcInvalidComparisonResultExceptionClass_createObject(dcNode *_value)
{
    return dcNodeEvaluator_callEvaluatedMethod
        (dcSystem_getCurrentNodeEvaluator(),
         "org.taffy.core.exception.InvalidComparisonResultException",
         "new:",
         _value,
         NULL);
}

void dcFileOpenExceptionClass_throwObject(const char *_filename)
{
    return dcExceptions_throwObject
        (dcFileOpenExceptionClass_createObject(_filename));
}

dcNode *dcFileOpenExceptionClass_createObject(const char *_filename)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    uint32_t classesSave = evaluator->onlyEvaluateClasses;
    evaluator->onlyEvaluateClasses = 0;
    dcNode *result = EVAL_FORMAT
        ("org.taffy.core.exception.FileOpenException new: \"%s\"", _filename);
    evaluator->onlyEvaluateClasses = classesSave;
    return result;
}

void dcFileWriteExceptionClass_throwObject(const char *_filename,
                                           const char *_error)
{
    return (dcExceptions_throwObject
            (EVAL_FORMAT
             ("org.taffy.core.exception.FileWriteException "
              "new: \"%s\" error: \"%s\"",
              _filename,
              _error)));
}

void dcSocketSendFailureExceptionClass_throwObject(void)
{
    dcExceptions_throwObject(dcSocketSendFailureExceptionClass_createObject());
}

dcNode *dcSocketSendFailureExceptionClass_createObject(void)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    return dcNodeEvaluator_callEvaluatedMethod(evaluator,
                                               "SocketSendFailureException",
                                               "create",
                                               NULL);
}

void dcStackOverflowExceptionClass_throwObject(void)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    uint32_t maxStackDepthSave = evaluator->maxStackDepth;
    evaluator->maxStackDepth = 0xFFFFFFFF;
    dcExceptions_throwObject(EVAL_STRING("new StackOverflowException"));
    evaluator->maxStackDepth = maxStackDepthSave;
}

void dcCInputExceptionClass_throwObject(void)
{
    dcExceptions_throwObject(EVAL_STRING("new CInputException"));
}

dcNode *dcAbortExceptionClass_createObject(const char *_why)
{
    return EVAL_FORMAT
        ("org.taffy.core.exception.AbortException newWithText: \"%s\"", _why);
}

void dcAbortExceptionClass_throwObject(const char *_why)
{
    dcExceptions_throwObject(dcAbortExceptionClass_createObject(_why));
}

dcNode *dcUserGeneratedAbortSignalExceptionClass_createObject(const char *_why)
{
    return EVAL_FORMAT
        ("org.taffy.core.exception.UserGeneratedAbortSignalException "
         "newWithText: \"%s\"", _why);
}

void dcUserGeneratedAbortSignalExceptionClass_throwObject(const char *_why)
{
    dcExceptions_throwObject
        (dcUserGeneratedAbortSignalExceptionClass_createObject(_why));
}

void dcDerivationExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING("new org.taffy.core.exception.DerivationException"));
}

void dcImportFailedExceptionClass_throwObject(const char *_packageName)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    uint32_t classesSave = evaluator->onlyEvaluateClasses;
    evaluator->onlyEvaluateClasses = 0;

    dcExceptions_throwObject
        (EVAL_FORMAT
         ("[org.taffy.core.exception.ImportFailedException "
          "newWithPackageName: \"%s\"]",
          _packageName));

    evaluator->onlyEvaluateClasses = classesSave;
}

void dcReturnWithNoCallStackExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING
         ("new org.taffy.core.exception.ReturnWithNoCallStackException"));
}

void dcBreakWithoutALoopExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING
         ("new org.taffy.core.exception.BreakWithoutALoopException"));
}

void dcNeedMetaClassExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING
         ("new org.taffy.core.exception.NeedMetaClassException"));
}

void dcUnidentifiedClassExceptionClass_throwObject(const char *_objectName)
{
    dcExceptions_throwObject
        (EVAL_FORMAT
         ("org.taffy.core.exception.UnidentifiedClassException "
          "newWithName: \"%s\"", _objectName));
 }

void dcPluginInstantiationExceptionClass_throwObject(const char *_reason)
{
    dcExceptions_throwObject
        (EVAL_FORMAT
         ("org.taffy.core.exception.PluginInstantiationException "
          "newWithReason: \"%s\"", _reason));
}

void dcSingletonInstantiationExceptionClass_throwObject(const char *_className)
{
    dcExceptions_throwObject
        (EVAL_FORMAT
         ("org.taffy.core.exception.SingletonInstantiationException "
          "newWithClassName: \"%s\"", _className));
}

void dcInvalidSuperClassExceptionClass_throwObject(const char *_superName)
{
    dcExceptions_throwObject
        (EVAL_FORMAT
         ("org.taffy.core.exception.InvalidSuperClassException "
          "newWithSuperName: \"%s\"", _superName));
}

void dcUnsupportedMathOperationExceptionClass_throwObject(const char *_reason)
{
    dcExceptions_throwObject
        (EVAL_FORMAT
         ("org.taffy.core.exception.UnsupportedMathOperationException "
          "newWithReason: \"%s\"",
          _reason));
}

void dcNeedVectorException_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING
         ("new org.taffy.core.exception.NeedVectorException"));
}

void dcDeadlockExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING
         ("new org.taffy.core.exception.DeadlockException"));
}

void dcBlockNotSingularExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING
         ("new org.taffy.core.exception.BlockNotSingularException"));
}

void dcAssertFailedExceptionClass_throwObject(void)
{
    dcExceptions_throwObject
        (EVAL_STRING
         ("new org.taffy.core.exception.AssertFailedException"));
}

// dcCompiledExceptions.h gives const char *__compiledExceptions
#include "CompiledExceptions.h"

void dcExceptions_create(void)
{
    assert(EVAL_STRING(__compiledExceptions) != NULL);
}
