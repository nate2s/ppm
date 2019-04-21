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
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "CompiledHelpTopic.h"
#include "CompiledIcuCopyrightHelpTopic.h"

#include "dcPieLineEvaluator.h"
#include "dcCallStackData.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcIOClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcLog.h"
#include "dcMemory.h"
#include "dcNilClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcParser.h"
#include "dcScope.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcPieApplication.h"

dcPieLineEvaluator *dcPieLineEvaluator_create(void)
{
    dcPieLineEvaluator *result =
        (dcPieLineEvaluator *)dcMemory_allocate(sizeof(dcPieLineEvaluator));
    result->lexer = dcLexer_createForConsole();
    result->inString = false;
    result->bracketCount = 0;
    result->commentLevel = 0;
    result->multiLineCommand = false;
    result->input = dcString_create();
    result->pseudoLineNumber = 1;
    return result;
}

void dcPieLineEvaluator_free(dcPieLineEvaluator **_evaluator)
{
    if (_evaluator != NULL)
    {
        dcPieLineEvaluator *evaluator = *_evaluator;
        dcLexer_free(&evaluator->lexer);
        dcString_free(&evaluator->input, DC_DEEP);
        dcMemory_free(evaluator);
    }
}

void dcPieLineEvaluator_resetState(dcPieLineEvaluator *_evaluator)
{
    // reset the evaluator state //
    _evaluator->bracketCount = 0;
    _evaluator->inString = false;
    _evaluator->multiLineCommand = false;
    dcString_clear(_evaluator->input);
}

char *dcPieLineEvaluator_getPrompt(dcPieLineEvaluator *_evaluator,
                                   const char *_program,
                                   bool _displayLineNumber)
{
    dcString *result = dcString_create();

    if (_program != NULL)
    {
        dcString_appendString(result, _program);
    }

    if (_evaluator->bracketCount > 0
        || _evaluator->commentLevel > 0
        || _evaluator->inString
        || _evaluator->multiLineCommand)
    {
        if (_displayLineNumber)
        {
            dcString_append(result, ".%lu", _evaluator->pseudoLineNumber);
        }

        dcString_appendString(result, ">> ");
    }
    else
    {
        if (_displayLineNumber)
        {
            dcString_append(result, ".%li", _evaluator->lexer->lineNumber);
        }

        dcString_appendString(result, "> ");
    }

    return dcString_freeAndReturn(&result);
}

static bool lastTokenNot(const char *_line,
                         int _lineLength,
                         int _lastCharIndex,
                         char _token)
{
    return (_lineLength == 1
            || (_lineLength > 1
                && _lastCharIndex > 0
                && _line[_lastCharIndex - 1] != _token));
}

static bool finishedInput(dcPieLineEvaluator *_evaluator, const char *_line)
{
    //
    // this function performs no ordering on the brackets and parentheses --
    // ordering is ensured via the parser
    //

    size_t lineLength = strlen(_line);
    size_t lineIt = 0;
    dcError_assert(lineLength > 0);
    size_t lastCharIndex = lineLength - 1;
    _evaluator->multiLineCommand = false;

    //
    // shrink the end past spaces and tabs
    //
    while (lastCharIndex > 0
           && (_line[lastCharIndex] == ' '
               || _line[lastCharIndex] == '\t'))
    {
        lastCharIndex--;
    }

    switch (_line[lastCharIndex])
    {
    case '/':
    case '=':
    case '&':
    case '^':
    case '%':
    case '<':
    case '>':
        _evaluator->multiLineCommand = true;
        break;
    case '*':
        // an import directive may end in .*, account for this
        if (lastTokenNot(_line, lineLength, lastCharIndex, '.'))
        {
            _evaluator->multiLineCommand = true;
        }
        break;
    case '+':
        // end a line if ++ is hit
        if (lastTokenNot(_line, lineLength, lastCharIndex, '+'))
        {
            _evaluator->multiLineCommand = true;
        }
        break;
    case '-':
        // end a line if -- is hit
        if (lastTokenNot(_line, lineLength, lastCharIndex, '-'))
        {
            _evaluator->multiLineCommand = true;
        }
        break;
    }

    bool gotBackslash = false;

    for (lineIt = 0; lineIt < lineLength; lineIt++)
    {
        if (_evaluator->commentLevel > 0)
        {
            if (_line[lineIt] == ')'
                && lineIt < lineLength -1
                && _line[lineIt + 1] == '~')
            {
                _evaluator->commentLevel--;
                lineIt++;
                continue;
            }
        }

        if (_evaluator->commentLevel == 0)
        {
            switch (_line[lineIt])
            {
            case '~':
                // start of comment?
                if (lineIt < lineLength - 1
                    && _line[lineIt + 1] == '(')
                {
                    _evaluator->commentLevel++;
                    lineIt++;
                }
                break;

            case '\\':
                lineIt++;

                if (lineIt == lineLength)
                {
                    gotBackslash = true;
                    _evaluator->multiLineCommand = true;
                }
                break;

            case '"':
                _evaluator->inString = ! _evaluator->inString;
                break;

            case '[':
            case '(':
            case '{':
                // the special case #[] in string is handled via the parser //
                if (! _evaluator->inString)
                {
                    _evaluator->bracketCount++;
                }

                break;

            case ')':
            case ']':
            case '}':
                // the special case #[] in string is handled via the parser //
                if (! _evaluator->inString)
                {
                    _evaluator->bracketCount--;
                }

                break;
            }
        }
    }

    bool result = ! (_evaluator->multiLineCommand
                     || _evaluator->bracketCount > 0
                     || _evaluator->inString
                     || _evaluator->commentLevel > 0
                     || gotBackslash);

    if (gotBackslash)
    {
        // don't include the backslash in the output
        char *lineCopy = (char *)dcMemory_duplicate(_line, lineLength);
        TAFFY_DEBUG(assert(lineCopy[lineLength - 1] == '\\'));
        lineCopy[lineLength - 1] = 0;
        dcString_appendString(_evaluator->input, lineCopy);
        dcMemory_free(lineCopy);
    }
    else
    {
        dcString_appendString(_evaluator->input, _line);
    }

    if (! result)
    {
        // we're not done with input yet..
        _evaluator->pseudoLineNumber++;

        if (gotBackslash)
        {
            _evaluator->lexer->lineNumber++;
        }
        else
        {
            dcString_appendCharacter(_evaluator->input, '\n');
        }
    }

    return result;
}

void dcPieLineEvaluator_loadHelp(void)
{
    const char *helpStrings[] =
        {
            __compiledHelpTopic,
            __compiledIcuCopyrightHelpTopic
        };

    assert(dcStringEvaluator_evalStringArray(helpStrings,
                                             dcTaffy_countOf(helpStrings))
           != NULL);
}

static char *undoQuotes(const char *_file, char **_original)
{
    char *file = dcMemory_strdup(_file);
    *_original = file;
    file++;
    file[strlen(file) - 1] = 0;
    return file;
}

typedef char *(*SaveOrLoadHistoryFunction)(const char *_filename);

static char *saveOrLoadHistory(const char *_value,
                               const char *_line,
                               const SaveOrLoadHistoryFunction _function)
{
    const char *file = _line + strlen(_value) + 1;
    char *result = NULL;

    if (strlen(file) > 1
        && file[0] == '"'
        && file[strlen(file) - 1] == '"')
    {
        char *original;
        char *newFile = undoQuotes(file, &original);
        result = _function(newFile);
        dcMemory_free(original);
    }
    else
    {
        result = dcMemory_strdup("Error: filename must be inside quotes");
    }

    return result;
}

static char *executeLine(dcPieLineEvaluator *_evaluator,
                         const char *_line,
                         dcPieLineEvaluatorOutFlag *_outFlags)
{
    char *result = NULL;
    dcNodeEvaluator *nodeEvaluator = dcSystem_getCurrentNodeEvaluator();

    const char save[] = "%save_history";
    const char load[] = "%load_history";
    const char help[] = "%help";
    const char logOn[] = "%log_on";
    const char logOff[] = "%log_off";
    const char importHelper[] = "%import_helper";

    if (strcmp(_line, "%print_classes") == 0)
    {
        result = dcClassManager_displayClasses();
    }
    else if (strncmp(_line, save, sizeof(save) - 1) == 0)
    {
        result = saveOrLoadHistory(save,
                                   _line,
                                   &dcPieApplication_saveHistory);
    }
    else if (strncmp(_line, load, sizeof(load) - 1) == 0)
    {
        result = saveOrLoadHistory(load,
                                   _line,
                                   &dcPieApplication_loadHistory);
    }
    else if (strncmp(_line, help, sizeof(help) - 1) == 0)
    {
        result = (char*)(dcNodeEvaluator_synchronizeFunctionCall
                         (nodeEvaluator,
                          &dcPieApplication_executeHelpString,
                          (void*)(_line + sizeof("%help"))));
    }
    else if (strncmp(_line, logOn, sizeof(logOn) - 1) == 0)
    {
        result = (dcLog_configureFromString(_line + sizeof(logOn), true)
                  ? dcMemory_strdup("true")
                  : dcMemory_strdup("false"));
    }
    else if (strncmp(_line, logOff, sizeof(logOff) - 1) == 0)
    {
        result = (dcLog_configureFromString((_line + sizeof(logOff)), false)
                  ? dcMemory_strdup("true")
                  : dcMemory_strdup("false"));
    }
    else if (strncmp(_line, importHelper, sizeof(importHelper) - 1) == 0)
    {
        assert(dcStringEvaluator_evalString
               (("import org.taffy.core.*;"
                 "import org.taffy.core.container.*;"
                 "import org.taffy.core.exception.*;"
                 "import org.taffy.core.io.*;"
                 "import org.taffy.core.maths.*;"
                 "import org.taffy.core.threading.*;"),
                "<console>",
                NO_STRING_EVALUATOR_FLAGS));
        result = dcMemory_strdup("true");
    }
    else
    {
        dcNode *evaluateResult = NULL;
        dcNodeEvaluator_resetState(nodeEvaluator);
        dcLexer_setInput(_evaluator->lexer,
                         dcMemory_strdup(_line),
                         true);
        dcNode *parseHead =
            dcParser_synchronizedParse(_evaluator->lexer, _line, _outFlags);

        if (parseHead != NULL)
        {
            nodeEvaluator->lineNumber = _evaluator->lexer->lineNumber;
            evaluateResult = dcNodeEvaluator_evaluate(nodeEvaluator, parseHead);

            if (evaluateResult != NULL
                && IS_CLASS(evaluateResult)
                && ! dcClass_isObject(evaluateResult))
            {
                // don't check the result here because we will handle any
                // exception below in dcNodeEvaluator_generateExceptionText
                dcClassTemplate_createRuntimeValues
                    (dcClass_getTemplate(evaluateResult), NULL);
            }
        }

        if (nodeEvaluator->exit)
        {
            result = dcMemory_strdup("true");
        }
        else
        {
            if (_outFlags != NULL)
            {
                *_outFlags &= ~PARSER_GOT_EXCEPTION;
            }

            result = dcNodeEvaluator_generateExceptionText(nodeEvaluator);

            if (result != NULL)
            {
                if (_outFlags != NULL)
                {
                    *_outFlags |= PARSER_GOT_EXCEPTION;
                }
            }
            else if (result == NULL && evaluateResult != NULL)
            {
                result = dcNode_synchronizedDisplay(evaluateResult);

                if (result == NULL)
                {
                    result = (dcNodeEvaluator_generateExceptionText
                              (nodeEvaluator));
                    assert(result != NULL);

                    if (_outFlags != NULL)
                    {
                        *_outFlags |= PARSER_GOT_EXCEPTION;
                    }
                }
            }
        }

        // we must not have an exception by this point
        assert(nodeEvaluator->exception == NULL);
        dcNode_register(parseHead);
    }

    return result;
}

static const char *getLineStart(const char *_line)
{
    // find the start of the input
    size_t i;
    size_t length = strlen(_line);

    for (i = 0; i < length && dcLexer_isWhitespace(_line[i]); i++)
    {
    }

    // _line[i] is either at the terminator (0),
    // or the first non-whitespace character
    return &_line[i];
}

char *dcPieLineEvaluator_evaluateLine(dcPieLineEvaluator *_evaluator,
                                      const char *_line,
                                      EvaluateLineCallback _callback,
                                      int _token,
                                      dcPieLineEvaluatorOutFlag *_outFlags)
{
    dcNodeEvaluator *nodeEvaluator = dcSystem_getCurrentNodeEvaluator();
    char *result = NULL;
    const char *lineStart = getLineStart(_line);

    if (strlen(lineStart) > 0 && finishedInput(_evaluator, lineStart))
    {
        // execute the line //
        result = executeLine(_evaluator,
                             _evaluator->input->string,
                             _outFlags);
        dcString_clear(_evaluator->input);
        dcLexer_incrementLineNumber(_evaluator->lexer);
        _evaluator->pseudoLineNumber = _evaluator->lexer->lineNumber;
        // restore the node evaluator's state
        dcNodeEvaluator_resetState(nodeEvaluator);

        // reset lexer and node evaluator error state
        dcLexer_setErrorState(_evaluator->lexer, false);
        _evaluator->multiLineCommand = false;
        _evaluator->bracketCount = 0;
        _evaluator->commentLevel = 0;
    }

    if (_callback != NULL)
    {
        _callback(_token);
    }

    return result;
}

void dcPieLineEvaluator_setLineNumber(dcPieLineEvaluator *_evaluator,
                                      uint32_t _lineNumber)
{
    _evaluator->lexer->lineNumber = _lineNumber;
}
