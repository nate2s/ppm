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

#ifndef __DC_KERNEL_CLASS_H__
#define __DC_KERNEL_CLASS_H__

#include "dcDefines.h"

struct dcKernelClassAux_t
{
    struct dcHash_t *symbols;

    // pointers to symbols for speed
    struct dcNode_t *lessThanSymbol;     // 'lessThan
    struct dcNode_t *equalSymbol;        // 'equals
    struct dcNode_t *greaterThanSymbol;  // 'greaterThan
    struct dcNode_t *uncomparableSymbol; // 'uncomparable
};

typedef struct dcKernelClassAux_t dcKernelClassAux;

// creating //
struct dcNode_t *dcKernelClass_createNode(bool _object);
struct dcNode_t *dcKernelClass_createObject(void);

// get the singleton //
struct dcNode_t *dcKernelClass_getInstance(void);

// symbol //
struct dcNode_t *dcKernelClass_getOrCreateSymbol(const char *_symbolName);

struct dcNode_t *dcKernelClass_getLessThanSymbol(void);
struct dcNode_t *dcKernelClass_getEqualSymbol(void);
struct dcNode_t *dcKernelClass_getGreaterThanSymbol(void);
struct dcNode_t *dcKernelClass_getUncomparableSymbol(void);

bool dcKernelClass_joinedFile(struct dcNode_t *_kernel,
                              const char *_filename);
void dcKernelClass_cancelThreads(struct dcNode_t *_kernel);

// standard functions //
ALLOCATE_FUNCTION(dcKernelClass_allocateNode);
COPY_FUNCTION(dcKernelClass_copyNode);
DEALLOCATE_FUNCTION(dcKernelClass_deallocateNode);
DEINITIALIZE_FUNCTION(dcKernelClass_deinitialize);
FREE_FUNCTION(dcKernelClass_freeNode);
GET_TEMPLATE_FUNCTION(dcKernelClass_getTemplate);
INITIALIZE_FUNCTION(dcKernelClass_initialize);
MARK_FUNCTION(dcKernelClass_markNode);
MARSHALL_FUNCTION(dcKernelClass_marshallNode);
REGISTER_FUNCTION(dcKernelClass_registerNode);
UNMARSHALL_FUNCTION(dcKernelClass_unmarshallNode);

// taffy c methods //
TAFFY_C_METHOD(dcKernelClass_addIncludeDirectory);
TAFFY_C_METHOD(dcKernelClass_allocatedMemory);
TAFFY_C_METHOD(dcKernelClass_arguments);
TAFFY_C_METHOD(dcKernelClass_assert);
TAFFY_C_METHOD(dcKernelClass_constify);
TAFFY_C_METHOD(dcKernelClass_constructObjectWithFields);
TAFFY_C_METHOD(dcKernelClass_eval);
#ifndef DISABLE_SYTEM_EXECUTION
  TAFFY_C_METHOD(dcKernelClass_executeOnSystem);
#endif
TAFFY_C_METHOD(dcKernelClass_exit);
TAFFY_C_METHOD(dcKernelClass_executeRemotelyOn);
TAFFY_C_METHOD(dcKernelClass_getSecondTime);
TAFFY_C_METHOD(dcKernelClass_includeDirectories);
TAFFY_C_METHOD(dcKernelClass_joined);
TAFFY_C_METHOD(dcKernelClass_loadState);
TAFFY_C_METHOD(dcKernelClass_marshall);
TAFFY_C_METHOD(dcKernelClass_registerConnectorName);
TAFFY_C_METHOD(dcKernelClass_registerConnectorPortName);
TAFFY_C_METHOD(dcKernelClass_saveState);
TAFFY_C_METHOD(dcKernelClass_sleepSeconds);
TAFFY_DEBUG(TAFFY_C_METHOD(dcKernelClass_symbols));
TAFFY_C_METHOD(dcKernelClass_systemSupportsPlugins);
TAFFY_DEBUG(TAFFY_C_METHOD(dcKernelClass_testOutOfMemoryException));
TAFFY_C_METHOD(dcKernelClass_unmarshall);

// marshalling //
// _string may be NULL
struct dcString_t *dcKernelClass_marshallState
    (struct dcString_t *_string,
     struct dcNodeEvaluator_t *_evaluator);
// _input may not be NULL
bool dcKernelClass_unmarshallState(struct dcString_t *_input,
                                   struct dcNodeEvaluator_t *_evaluator);

// helpers //
struct dcString_t *dcKernelClass_getState(void);

bool dcKernelClass_equals_helper(struct dcNode_t *_left,
                                 struct dcNode_t *_right);

dcResult dcKernelClass_loadPlugin(const char *_directory);

#define KERNEL_PACKAGE_NAME CORE_PACKAGE_NAME
#define KERNEL_CLASS_NAME "Kernel"

#endif
