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

#ifndef __DC_TEST_UTILITIES_H__
#define __DC_TEST_UTILITIES_H__

#include "dcDefines.h"

typedef void (*dcTestUtilities_testFunction)(void);

struct dcTestFunctionMap_t
{
    const char *name;
    dcTestUtilities_testFunction function;
};

typedef struct dcTestFunctionMap_t dcTestFunctionMap;

void dcTestUtilities_stopAt(size_t _testNumber);

//
// Start, run, and stop, all in one!
//
void dcTestUtilities_go(const char *_name,
                        int _argc,
                        char **_argv,
                        dcTestUtilities_testFunction _initializer,
                        const dcTestFunctionMap *_map,
                        bool _synchronized);

// starting and ending //
void dcTestUtilities_start(const char *_name, int _argc, char **_argv);
void dcTestUtilities_startAndCreateNodes(const char *_name,
                                         int _argc,
                                         char **_argv,
                                         bool _createNodes);
void dcTestUtilities_end(void);

// run system tests? //
bool dcTestUtilities_runSystem(void);

// marking //
void dcTestUtilities_markNodes(void);

void dcTestUtilities_checkMarks(bool _yesno);
void dcTestUtilities_doMark(bool _yesno);

// container sizes //
void dcTestUtilities_checkArraySize(const struct dcArray_t *_array,
                                    dcContainerSizeType _wanted);

void dcTestUtilities_checkListSize(const struct dcList_t *_list,
                                   dcContainerSizeType _wanted);

void dcTestUtilities_checkHashSize(const struct dcHash_t *_hash,
                                   dcContainerSizeType _wanted);

// equality checks //
void dcTestUtilities_checkIntEqual(int _first, int _second);
void dcTestUtilities_checkEqual(const struct dcNode_t *_first,
                                const struct dcNode_t *_second);

#define dcTestUtilities_assert(what)         \
    dcTestUtilities_checkBool(what, #what)

void dcTestUtilities_checkBool(bool _got, const char *_description);
void dcTestUtilities_checkNonNull(const void *_pointer);
void dcTestUtilities_checkMark(const struct dcNode_t *_node,
                               bool _marked);
extern struct dcNode_t **gTestNodes;
extern dcContainerSizeType gTestNodesSize;

void dcTestUtilities_runTests(dcTestUtilities_testFunction _initializer,
                              const dcTestFunctionMap *_map,
                              bool _synchronized);

void dcTestUtilities_assertException(struct dcNode_t *_result,
                                     struct dcNodeEvaluator_t *_evaluator,
                                     const char *_exceptionClassName,
                                     const char *_objectName,
                                     const char *_expectedValue,
                                     bool _clearException);

void dcTestUtilities_assertCallStack
    (struct dcNode_t *_result,
     struct dcNodeEvaluator_t *_evaluator,
     struct dcList_t *_expectedCallStack);

void dcTestUtilities_expectEqual(struct dcNode_t **_node,
                                 struct dcNode_t **_expected);

void dcTestUtilities_expectStringEqual(const char *_expected, const char *_got);
void dcTestUtilities_expectStringNodeEqual(const char *_expected,
                                           struct dcNode_t *_node);
void dcTestUtilities_expectNodeStringsEqual(struct dcNode_t *_expected,
                                            struct dcNode_t *_node);

#endif
