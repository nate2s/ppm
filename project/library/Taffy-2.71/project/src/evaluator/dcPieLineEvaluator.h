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

#ifndef __DC_LINE_EVALUATOR_H__
#define __DC_LINE_EVALUATOR_H__

#include "dcDefines.h"

struct dcPieLineEvaluator_t
{
    struct dcLexer_t *lexer;
    int bracketCount;
    int commentLevel;
    bool inString;
    bool multiLineCommand;
    uint32_t pseudoLineNumber;
    struct dcString_t *input;
};

typedef struct dcPieLineEvaluator_t dcPieLineEvaluator;

// dcPieLineEvaluatorOutFlag is in dcDefines.h

// creating //
dcPieLineEvaluator *dcPieLineEvaluator_create(void);

// freeing //
void dcPieLineEvaluator_free(dcPieLineEvaluator **_evaluator);

// executing //
void dcPieLineEvaluator_execute(dcPieLineEvaluator *_evaluator);

typedef void (*EvaluateLineCallback)(int _token);
char *dcPieLineEvaluator_evaluateLine(dcPieLineEvaluator *_evaluator,
                                      const char *_line,
                                      EvaluateLineCallback _callback,
                                      int _token,
                                      dcPieLineEvaluatorOutFlag *_outFlags);

char *dcPieLineEvaluator_getPrompt(dcPieLineEvaluator *_evaluator,
                                   const char *_program,
                                   bool _displayLineNumber);

// setting //
void dcPieLineEvaluator_setLineNumber(dcPieLineEvaluator *_evaluator,
                                      uint32_t _lineNumber);

// reset the current state
void dcPieLineEvaluator_resetState(dcPieLineEvaluator *_evaluator);

void dcPieLineEvaluator_loadHelp();

#endif
