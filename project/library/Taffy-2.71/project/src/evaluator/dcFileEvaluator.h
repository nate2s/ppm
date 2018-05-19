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

#ifndef __DC_FILE_EVALUATOR_H__
#define __DC_FILE_EVALUATOR_H__

#include <stdio.h>
#include <stdlib.h>

#include "dcDefines.h"

struct dcFileEvaluator_t
{
    const struct dcString_t *filename;
};

typedef struct dcFileEvaluator_t dcFileEvaluator;

dcFileEvaluator *dcFileEvaluator_create(void);

dcFileEvaluator *dcFileEvaluator_createWithArguments
    (struct dcCommandLineArguments_t *_arguments);

void dcFileEvaluator_free(dcFileEvaluator **_evaluator);

dcResult dcFileEvaluator_execute(dcFileEvaluator *_fileEvaluator);

// evaluating //
struct dcNode_t *dcFileEvaluator_evaluateFile(const char *_filename);

// handle an exception if it occurs
struct dcNode_t *dcFileEvaluator_evaluateFileWithExceptionCatch
    (const char *_fileName,
     bool _handleException);

#endif
