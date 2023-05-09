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

#ifndef __DC_LEXER_H__
#define __DC_LEXER_H__

#include <stdio.h>
#include <stdarg.h>

#include "dcDefines.h"

#define TOKEN int

typedef enum
{
    LEXER_NORMAL_STATE                    = 0,
    LEXER_STRING_STATE                    = 1,
    LEXER_IGNORE_RETURNS_STATE            = 2,
    LEXER_IGNORE_RETURNS_FOR_METHOD_STATE = 3,
    LEXER_STRING_EXPRESSION_STATE         = 4,
    LEXER_VERBATIM_TEXT_STATE             = 5,
    LEXER_IDENTIFIER_STATE                = 6,
    LEXER_NUMBER_STATE                    = 7,
    LEXER_PUBLIC_CLASS_STATE              = 8,
    LEXER_PRIVATE_CLASS_STATE             = 9
} dcLexer_state;

/////////////////
// dcLexResult //
/////////////////

struct dcLexResult_t
{
    char *text;
    TOKEN token;
};

typedef struct dcLexResult_t dcLexResult;

// creating //
dcLexResult *dcLexResult_create(const char *_text, TOKEN _token);
struct dcNode_t *dcLexResult_createNode(const char *_text, TOKEN _token);

// freeing //
void dcLexResult_free(dcLexResult **_result);
void dcLexResult_freeNode(struct dcNode_t *_lexResultNode, dcDepth _depth);

// copying //
dcLexResult *dcLexResult_copy(const dcLexResult *_from, dcDepth _depth);

void dcLexResult_copyNode(struct dcNode_t *_to,
                          const struct dcNode_t *_from,
                          dcDepth _depth);

/////////////
// dcLexer //
/////////////

struct dcLexer_t
{
    uint32_t lineNumber;
    uint32_t lastTokenLineNumber;
    uint32_t previousLineNumber;
    uint32_t classLineNumberSave;
    uint16_t filenameId;
    char *input;

    struct dcList_t *scopeDataFlags;

    size_t lexStartPosition;
    size_t lexPosition;
    size_t lastToken;
    size_t bracketCount;
    size_t stringBracketCount;
    size_t inputLength;

    bool atEnd;
    bool inStringExpression;
    bool gotSymbol;
    bool isFloat;
    bool errorState;
    bool parseError;
    bool inputOwned;
    bool inImport;
    bool inMatrix;

    char *errorString;

    int leftBracket;

    struct dcList_t *states;
    struct dcList_t *residuals;

    struct dcString_t *stringString;
    struct dcString_t *verbatimTextString;
    struct dcString_t *keywordString;
    struct dcString_t *numberString;
};

typedef struct dcLexer_t dcLexer;

// initialization //
void dcLexer_initialize();
void dcLexer_cleanup();

// creating //
dcLexer *dcLexer_create(void);
dcLexer *dcLexer_createForConsole(void);

dcLexer *dcLexer_createWithInput(const char *_filename,
                                 char *_input,
                                 bool _takeOwnership);

dcLexer *dcLexer_createFromFile(const char *_filename, FILE *_file);

// freeing //
void dcLexer_free(dcLexer **_lexer);
void dcLexer_freeNode(struct dcNode_t *_lexer, dcDepth _depth);

void dcLexer_setInput(dcLexer *_lexer,
                             char *_input,
                             bool _takeOwnership);

// lexing //
typedef TOKEN (*dcLexer_lexPointer)(dcLexer *_lexer, bool *_again);

// getting //
uint32_t dcLexer_getLineNumber(const dcLexer *_lexer);
uint32_t dcLexer_getPreviousLineNumber(const dcLexer *_lexer);
const char *dcLexer_getFilename(const dcLexer *_lexer);
uint16_t dcLexer_getFilenameId(const dcLexer *_lexer);
uint16_t dcLexer_getScopeFlags(const dcLexer *_lexer);
bool dcLexer_hasParseError(dcLexer *_lexer);
dcScopeDataFlags dcLexer_getCurrentScopeDataFlags(dcLexer *_lexer);

// setting //
void dcLexer_setLineNumber(dcLexer *_lexer, uint32_t _lineNumber);
void dcLexer_incrementLineNumber(dcLexer *_lexer);
void dcLexer_setErrorState(dcLexer *_lexer, bool _errorState);
void dcLexer_setErrorString(dcLexer *_lexer, const char *_errorString);

// whitespace //
bool dcLexer_isWhitespace(char _character);

// lexing //
TOKEN dcLexer_lex(dcLexer *_lexer);
void dcLexer_eatLine(dcLexer *_lexer);
size_t dcLexer_eatWhitespace(dcLexer *_lexer);
size_t dcLexer_eatSpacesAndTabs(dcLexer *_lexer);
struct dcString_t *dcLexer_escapeString(const char *_string);

char *dcLexer_lexString(const char *_input);

// resetting //
void dcLexer_reset(dcLexer *_lexer);

// error //
void dcLexer_parseError(dcLexer *_lexer);
void dcLexer_parseErrorWithString(dcLexer *_lexer, const char *_errorString);
void dcLexer_parseErrorWithCharacter(dcLexer *_lexer, char _errorString);

// querying //
bool dcLexer_isNextText(const dcLexer *_lexer, const char *_text);

// splitting //
struct dcList_t *dcLexer_splitString(const char *_string, char _token);

// printing //
char *dcLexer_sprintf(const char *_format, ...);
char *dcLexer_sprintfWithVaList(const char *_format,
                                va_list _arguments,
                                size_t *_length);

// exceptions //
void dcLexer_throwExceptionWithString(dcLexer *_lexer, const char *_error);
void dcLexer_yyerror(dcLexer *_lexer, const char *_format, ...);
void dcLexer_handleParseError(dcLexer *_lexer);

// states /
void dcLexer_pushScopeDataFlags(dcLexer *_lexer);
void dcLexer_popScopeDataFlags(dcLexer *_lexer);
void dcLexer_clearScopeDataFlags(dcLexer *_lexer);
void dcLexer_popState(dcLexer *_lexer);
void dcLexer_pushState(dcLexer *_lexer, dcLexer_state _state);
bool dcLexer_hasParseError(dcLexer *_lexer);

typedef struct
{
    const char * const name;
    const char * const description;
    const int symbol;
} SymbolData;

extern const SymbolData sKeywordDatas[];
extern const SymbolData sTokenDatas[];

#endif
