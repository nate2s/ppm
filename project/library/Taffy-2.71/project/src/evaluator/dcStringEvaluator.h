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

#ifndef __DC_STRING_EVALUATOR_H__
#define __DC_STRING_EVALUATOR_H__

#include "dcDefines.h"

typedef enum
{
    NO_STRING_EVALUATOR_FLAGS            = 0,
    STRING_EVALUATOR_HANDLE_EXCEPTION    = BITS(0),
    STRING_EVALUATOR_SYNCHRONIZE         = BITS(1),
    STRING_EVALUATOR_ASSERT_NO_EXCEPTION = BITS(2)
} dcStringEvaluator_evalFlag;

struct dcNode_t *dcStringEvaluator_evalFormat(const char *_fileName,
                                              dcStringEvaluator_evalFlag _flags,
                                              const char *_format,
                                              ...);

struct dcNode_t *dcStringEvaluator_evalString
    (const char *_inputString,
     const char *_fileName,
     dcStringEvaluator_evalFlag _flags);

struct dcNode_t *dcStringEvaluator_evalStringArray(const char *_array[],
                                                   size_t _count);

#endif
