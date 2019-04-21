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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdarg.h>

#include "dcComplexNumber.h"
#include "dcDefines.h"
#include "dcLexer.h"
#include "dcError.h"
#include "dcFileManagement.h"
#include "dcGraphData.h"
#include "dcUnsignedInt32.h"
#include "dcList.h"
#include "dcLog.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNumber.h"
#include "dcParser.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringManager.h"
#include "dcSystem.h"

// defines //

// CC stands for "current character" //
#define CC _lexer->input[_lexer->lexPosition]
#define CCNEXT _lexer->input[_lexer->lexPosition + 1]
#define CCNEXT_NEXT _lexer->input[_lexer->lexPosition + 2]
#define CCAT(__position__) _lexer->input[__position__]

#define ATEND_AT(__position__) (__position__ >= _lexer->inputLength)
#define ATEND_AT_CCNEXT ATEND_AT(_lexer->lexPosition + 1)
#define ATEND (ATEND_AT(_lexer->lexPosition))

#define TOKEN_AHEAD(_lexer, _more, _token)              \
    (! ATEND_AT(_lexer->lexPosition + _more)            \
     && CCAT(_lexer->lexPosition + _more) == _token)

#define METHOD_ATTRIBUTE_CHARACTERS "( ) + - / * = < > ! [ ] . ^ %"
#define WHITESPACE_CHARACTERS "\n \t  "
#define EXPRESSION_END_CHARACTERS WHITESPACE_CHARACTERS " ; ) } ] ["

#define END_OF_TOKEN_AT(_position)                                      \
    (ATEND_AT(_position)                                                \
     || sReservedMap[(uint8_t)_lexer->input[_position]])

static bool sReservedMap[256] = {0};

static const char sReservedCharacters[] =
    METHOD_ATTRIBUTE_CHARACTERS " "             \
    WHITESPACE_CHARACTERS " "                   \
    "\\ # { } , | ;";

static bool sMethodAttributeMap[256] = {0};
static const char sMethodAttributeCharacters[] = METHOD_ATTRIBUTE_CHARACTERS;

static bool sPositiveDigitMap[256] = {0};
static const char sPositiveDigits[] = "1 2 3 4 5 6 7 8 9";

static bool sHexDigitMap[256] = {0};
static const char sHexDigits[] = "0 A B C D E F a b c d e f";

static void fillMap(bool *_map, const char *_characters, size_t _count)
{
    size_t i;

    for (i = 0; i < _count; i += 2)
    {
        _map[(int)_characters[i]] = true;
    }
}

static void initializeMaps(void)
{
    fillMap(sReservedMap, sReservedCharacters, sizeof(sReservedCharacters));
    fillMap(sPositiveDigitMap, sPositiveDigits, sizeof(sPositiveDigits));
    fillMap(sHexDigitMap, sHexDigits, sizeof(sHexDigits));
    fillMap(sMethodAttributeMap,
            sMethodAttributeCharacters,
            sizeof(sMethodAttributeCharacters));
}

#define POSITIVE_DIGIT_AT(at) (sPositiveDigitMap[(int)CCAT(at)] != 0)
#define HEX_DIGIT_AT(at)                                        \
    (sHexDigitMap[(int)CCAT(at)] != 0                           \
     || POSITIVE_DIGIT_AT(at))

#define INCREMENT_START_POSITION(__num__) \
    _lexer->lexStartPosition += __num__;

#define POSITIVE_DIGIT (POSITIVE_DIGIT_AT(_lexer->lexPosition))
#define DIGIT_AT(at) ((CCAT(at) == '0') || POSITIVE_DIGIT_AT(at))
#define DIGIT (DIGIT_AT(_lexer->lexPosition))
#define BINARY_DIGIT_AT(at) (CCAT(at) == '0' || CCAT(at) == '1')

#define EXPR_END(__c__)  \
    (__c__ == '\n'       \
     || __c__ == ';'     \
     || __c__ == ' '     \
     || __c__ == ')'     \
     || __c__ == '}'     \
     || __c__ == ']'     \
     || __c__ == '[')

#define WORD_CHARACTER(character)                               \
    (isalnum((unsigned char)character)                          \
     || character == '_')

#define START_IDENTIFIER_CHARACTER(character)   \
    (character == '#' || character == '@')

// these special end word characters can only appear at the end of a word //
#define SPECIAL_END_WORD_CHARACTER(character)   \
    (character == '?' || character == '!')

#define END_WORD_CHARACTER(character)                       \
    (character == ' '                                       \
     || character == '\t'                                   \
     || sReservedMap[(int)CC]                                \
     || END_LINE_CHARACTER(character))

#define END_LINE_CHARACTER(character)               \
    (character == '\n'                              \
     || character == '{'                            \
     || character == '}'                            \
     || character == tEXPR_END)

#define WHITESPACE(character)                                       \
    (character == '\n'                                              \
     || character == ' '                                            \
     || character == '\t')

#define WHITESPACE_AT(position)                 \
    WHITESPACE(_lexer->input[position])

#define LEXED(__num, __token, __string)                          \
    dcLexer_setErrorString(_lexer, __string);                    \
    _lexer->lexPosition += __num;                                \
    _lexer->lastToken = __token;                                 \
    _lexer->lastTokenLineNumber = _lexer->previousLineNumber;    \
    _lexer->lexStartPosition += __num;                           \
    retval = __token;

#define LEXED_WITHOUT_POSITION_INCREMENT(__token, __string)             \
    dcLexer_setErrorString(_lexer, __string);                           \
    _lexer->lastToken = __token;                                        \
    _lexer->lastTokenLineNumber = _lexer->previousLineNumber;           \
    _lexer->lexStartPosition +=                                         \
        (_lexer->lexPosition - _lexer->lexStartPosition);               \
    retval = __token;

static TOKEN parseError(dcLexer *_lexer, const char *_message)
{
    dcLexer_setErrorString(_lexer, _message);
    dcLexer_parseError(_lexer);
    return EOF;
}

static TOKEN parseErrorFormat(dcLexer *_lexer, const char *_format, ...)
{
    va_list arguments;
    va_start(arguments, _format);
    char *errorString = dcLexer_sprintfWithVaList(_format, arguments, NULL);
    parseError(_lexer, errorString);
    //dcMemory_free(errorString);
    return EOF;
}

static dcLexer_state getState(const dcLexer *_lexer)
{
    return (_lexer->states->size > 0
            ? (dcLexer_state)CAST_INT(dcList_getTail(_lexer->states))
            : LEXER_NORMAL_STATE);
}

static dcLexer_lexPointer *sLexPointers = NULL;
static uint16_t sLexPointersSize = 256;

static const char * const kR_STRING = "@r";
static const char * const kRW_STRING = "@rw";
static const char * const kPROTECTED_STRING = "@protected";
static const char * const kPUBLIC_STRING = "@public";
static const char * const kW_STRING = "@w";

// tokens //
static const char tBANG_STRING[] = "!";
static const char tBIT_AND_STRING[] = "&";
static const char tBIT_AND_EQUAL_STRING[] = "&=";
static const char tBIT_XOR_STRING[] = "^^";
static const char tBIT_XOR_EQUAL_STRING[] = "^^=";
static const char tBLOCK_START_STRING[] = "{{";
static const char tBLOCK_END_STRING[] = "}";
static const char tCOLON_STRING[] = ":";
static const char tCOMMA_STRING[] = ",";
static const char tEQUAL_EQUAL_STRING[] = "==";
static const char tDIVIDE_EQUAL_STRING[] = "/=";
static const char tDIVIDE_STRING[] = "/";
static const char tDOT_STRING[] = ".";
static const char tEOF_STRING[] = "EOF";
static const char tEXPR_END_STRING[] = "tEXPR_END";
static const char tEQUAL_STRING[] = "=";
static const char tGREATER_THAN_OR_EQUAL_STRING[] = ">=";
static const char tGREATER_THAN_STRING[] = ">";
static const char tVERBATIM_TEXT_START_STRING[] = "'{";
static const char tLEFT_BRACE_LESS_THAN_STRING[] = "{{<";
static const char tLEFT_BRACE_STRING[] = "{";
static const char tLEFT_BRACKET_STRING[] = "[";
static const char tLEFT_PAREN_STRING[] = "(";
static const char tLEFT_SHIFT_STRING[] = "<<";
static const char tLEFT_SHIFT_EQUAL_STRING[] = "<<=";
static const char tLESS_THAN_OR_EQUAL_STRING[] = "<=";
static const char tLESS_THAN_STRING[] = "<";
static const char tMATRIX_DELIMITER_STRING[] = "||";
static const char tMETHOD_META_STRING[] = "(@@)";
static const char tMETHOD_INSTANCE_STRING[] = "(@)";
static const char tMINUS_EQUAL_STRING[] = "-=";
static const char tMINUS_MINUS_STRING[] = "--";
static const char tMINUS_STRING[] = "-";
static const char tMODULUS_STRING[] = "%";
static const char tMODULUS_EQUAL_STRING[] = "%=";
static const char tMULTIPLY_EQUAL_STRING[] = "*=";
static const char tMULTIPLY_STRING[] = "*";
static const char tNOT_EQUAL_STRING[] = "!=";
static const char tBIT_OR_STRING[] = "|";
static const char tBIT_OR_EQUAL_STRING[] = "|=";
static const char tPOWER_EQUAL_STRING[] = "^=";
static const char tPOWER_STRING[] = "^";
static const char tPLUS_EQUAL_STRING[] = "+=";
static const char tPLUS_PLUS_STRING[] = "++";
static const char tPLUS_STRING[] = "+";
static const char tQUOTE_STRING[] = "\"";
static const char tRIGHT_ARROW_STRING[] = "=>";
static const char tRIGHT_BRACE_STRING[] = "}";
static const char tRIGHT_BRACKET_EQUAL_STRING[] = "]=";
static const char tRIGHT_BRACKET_STRING[] = "]";
static const char tRIGHT_PAREN_STRING[] = ")";
static const char tRIGHT_PAREN_EQUAL_STRING[] = ")=";
static const char tRIGHT_SHIFT_STRING[] = ">>";
static const char tRIGHT_SHIFT_EQUAL_STRING[] = ">>=";
static const char tSEMI_COLON_STRING[] = ";";
static const char tSTRING_EXPRESSION_START_STRING[] = "#[";
static const char tTILDE_EQUAL_LESS_THAN_STRING[] = "~=<";
static const char tTILDE_EQUAL_STRING[] = "~=";
static const char tTILDE_STRING[] = "~";
static const char tZERO_STRING[] = "0";

static int sHexLookup[256] = {0};

#define makeKeyword(description, keyword) {#keyword, description, keyword}

const SymbolData sKeywordDatas[] =
{
    makeKeyword("#breakthrough",      kMETHOD_ATTRIBUTE_BREAKTHROUGH),
    makeKeyword("#const",             kMETHOD_ATTRIBUTE_CONST),
    makeKeyword("#containerLoop",     kMETHOD_ATTRIBUTE_CONTAINER_LOOP),
    makeKeyword("#equation",          kEQUATION),
    makeKeyword("#modifiesContainer", kMETHOD_ATTRIBUTE_MODIFIES_CONTAINER),
    makeKeyword("#operator",          kMETHOD_ATTRIBUTE_OPERATOR),
    makeKeyword("#prefixOperator",    kMETHOD_ATTRIBUTE_PREFIX_OPERATOR),
    makeKeyword("#synchronized",      kMETHOD_ATTRIBUTE_SYNCHRONIZED),
    makeKeyword("#synchronizedRead",  kMETHOD_ATTRIBUTE_SYNCHRONIZED_READ),
    makeKeyword("#synchronizedWrite", kMETHOD_ATTRIBUTE_SYNCHRONIZED_WRITE),
    makeKeyword("abstract",           kABSTRACT),
    makeKeyword("and",                kAND),
    makeKeyword("atomic",             kATOMIC),
    makeKeyword("break",              kBREAK),
    makeKeyword("catch",              kCATCH),
    makeKeyword("class",              kCLASS),
    makeKeyword("const",              kCONST),
    makeKeyword("else",               kELSE),
    makeKeyword("false",              kFALSE),
    makeKeyword("final",              kFINAL),
    makeKeyword("for",                kFOR),
    makeKeyword("global",             kGLOBAL),
    makeKeyword("i",                  kI),
    makeKeyword("if",                 kIF),
    makeKeyword("import",             kIMPORT),
    makeKeyword("in",                 kIN),
    makeKeyword("local",              kLOCAL),
    makeKeyword("new",                kNEW),
    makeKeyword("nil",                kNIL),
    makeKeyword("no",                 kNO),
    makeKeyword("or",                 kOR),
    makeKeyword("package",            kPACKAGE),
    makeKeyword("return",             kRETURN),
    makeKeyword("self",               kSELF),
    makeKeyword("slice",              kSLICE),
    makeKeyword("super",              kSUPER),
    makeKeyword("synchronized",       kSYNCHRONIZED),
    makeKeyword("throw",              kTHROW),
    makeKeyword("true",               kTRUE),
    makeKeyword("try",                kTRY),
    makeKeyword("upSelf",             kUP_SELF),
    makeKeyword("while",              kWHILE),
    makeKeyword("yes",                kYES),
    {NULL, NULL, 0}
};

#define makeToken(token) {NULL, #token, token}

const SymbolData sTokenDatas[] =
{
    makeToken(tDIVIDE_EQUAL),
    makeToken(tDOT),
    makeToken(tEXPR_END),
    makeToken(tEQUAL_EQUAL),
    makeToken(tGREATER_THAN_OR_EQUAL),
    makeToken(tINSTANCE_SCOPED_IDENTIFIER),
    makeToken(tLEFT_BRACE_LESS_THAN),
    makeToken(tLEFT_SHIFT),
    makeToken(tLESS_THAN_OR_EQUAL),
    makeToken(tMATRIX_DELIMITER),
    makeToken(tMETA_SCOPED_IDENTIFIER),
    makeToken(tMETHOD_INSTANCE),
    makeToken(tMETHOD_META),
    makeToken(tMETHOD_PARAMETER),
    makeToken(tMINUS_EQUAL),
    makeToken(tMINUS_MINUS),
    makeToken(tMODULUS_EQUAL),
    makeToken(tMULTIPLY_EQUAL),
    makeToken(tNOT_EQUAL),
    makeToken(tPLUS_EQUAL),
    makeToken(tPLUS_PLUS),
    makeToken(tPOWER_EQUAL),
    makeToken(tRIGHT_ARROW),
    makeToken(tRIGHT_BRACKET_EQUAL),
    makeToken(tRIGHT_SHIFT),
    makeToken(tRIGHT_PAREN_EQUAL),
    makeToken(tSTRING),
    makeToken(tSTRING_EXPRESSION_START),
    makeToken(tSYMBOL),
    makeToken(tTILDE_EQUAL),
    makeToken(tTILDE_EQUAL_LESS_THAN),
    makeToken(tVERBATIM_TEXT_START),
    makeToken(tWORD),
    {NULL, NULL, 0}
};

#define LEXER_LEXER(_function_)                                 \
    static TOKEN _function_(dcLexer *_lexer, bool *_again);

LEXER_LEXER(dcLexer_lexAt);
LEXER_LEXER(dcLexer_lexBackSlash);
LEXER_LEXER(dcLexer_lexBang);
LEXER_LEXER(dcLexer_lexBitAnd);
LEXER_LEXER(dcLexer_lexCaret);
LEXER_LEXER(dcLexer_lexColon);
LEXER_LEXER(dcLexer_lexComma);
LEXER_LEXER(dcLexer_lexDot);
LEXER_LEXER(dcLexer_lexEquals);
LEXER_LEXER(dcLexer_lexForwardSlash);
LEXER_LEXER(dcLexer_lexGreaterThan);
LEXER_LEXER(dcLexer_lexHash);
LEXER_LEXER(dcLexer_lexIdentifier);
LEXER_LEXER(dcLexer_lexInStringState);
LEXER_LEXER(dcLexer_lexLeftBrace);
LEXER_LEXER(dcLexer_lexLeftBracket);
LEXER_LEXER(dcLexer_lexLeftParen);
LEXER_LEXER(dcLexer_lexLessThan);
LEXER_LEXER(dcLexer_lexMinus);
LEXER_LEXER(dcLexer_lexModulus);
LEXER_LEXER(dcLexer_lexNumber);
LEXER_LEXER(dcLexer_lexPipe);
LEXER_LEXER(dcLexer_lexPlus);
LEXER_LEXER(dcLexer_lexQuote);
LEXER_LEXER(dcLexer_lexReturn);
LEXER_LEXER(dcLexer_lexRightBrace);
LEXER_LEXER(dcLexer_lexRightBracket);
LEXER_LEXER(dcLexer_lexRightParen);
LEXER_LEXER(dcLexer_lexSemicolon);
LEXER_LEXER(dcLexer_lexSingleQuote);
LEXER_LEXER(dcLexer_lexStar);
LEXER_LEXER(dcLexer_lexTilde);
LEXER_LEXER(dcLexer_lexInVerbatimTextState);
LEXER_LEXER(dcLexer_lexQuestionMark);
LEXER_LEXER(dcLexer_lexZero);

static dcTaffyOperator compareKeyword(const void *_left,
                                      const SymbolData *_right)
{
    dcLog_log(LEXER_SEARCH_LOG,
              "comparing keyword %s with %s\n",
              (const char *)_left,
              _right->description);
    return dcMemory_taffyStringCompare((const char *)_left,
                                       _right->description);
}

static const SymbolData *findSymbolData
    (const SymbolData *_symbolDatas,
     size_t _symbolDataCount,
     dcTaffyOperator (*_comparer)(const void *_left,
                                  const SymbolData *_right),
     const void *_token)
{
    size_t bound1 = 0;
    size_t bound2 = _symbolDataCount;
    const SymbolData *result = NULL;
    size_t position = bound1 + ((bound2 - bound1) / 2);

    while (true)
    {
        dcLog_log(LEXER_SEARCH_LOG,
                  "bound1: %u | bound2: %u | position: %u\n",
                  bound1,
                  bound2,
                  position);

        const SymbolData *lookup = &_symbolDatas[position];
        dcTaffyOperator comparison = _comparer(_token, lookup);

        if (comparison == TAFFY_LESS_THAN)
        {
            bound2 = position;
        }
        else if (comparison == TAFFY_GREATER_THAN)
        {
            bound1 = position;
        }
        else
        {
            result = lookup;
            break;
        }

        if (bound1 == bound2)
        {
            break;
        }

        size_t oldPosition = position;
        position = bound1 + ((bound2 - bound1) / 2);

        if (position == oldPosition)
        {
            // nothing was found
            break;
        }
    }

    return result;
}

static const SymbolData *findSymbolDataFromKeyword
    (const char *_keyword)
{
    return findSymbolData(sKeywordDatas,
                          dcTaffy_countOf(sKeywordDatas) - 1,
                          &compareKeyword,
                          _keyword);
}

#ifdef ENABLE_DEBUG
static dcTaffyOperator compareToken(const void *_left,
                                    const SymbolData *_right)
{
    const int *left = (const int*)_left;
    dcLog_log(LEXER_SEARCH_LOG,
              "comparing token %u with %u\n",
              *left,
              _right->symbol);

    return (*left < _right->symbol
            ? TAFFY_LESS_THAN
            : (*left > _right->symbol
               ? TAFFY_GREATER_THAN
               : TAFFY_EQUALS));
}

static const SymbolData *findSymbolDataFromToken(int _token)
{
    const SymbolData *result = (findSymbolData
                                (sKeywordDatas,
                                 dcTaffy_countOf(sKeywordDatas) - 1,
                                 &compareToken,
                                 &_token));
    return (result == NULL
            ? findSymbolData(sTokenDatas,
                             dcTaffy_countOf(sTokenDatas) - 1,
                             &compareToken,
                             &_token)
            : result);
}
#endif

TOKEN yylex(void)
{
    dcLexer *lexer = dcParser_getLexer();

    if (lexer == NULL)
    {
        dcError_internal("Cannot lex with a NULL lexer");
    }

    return dcLexer_lex(lexer);
}

void yyerror(char *_msg)
{
    //
    // the parser will do the actual ParseErrorException object creation
    //
    dcLexer *_lexer = dcParser_getLexer();
    dcError_assert(_lexer != NULL);
    _lexer->parseError = true;
}

dcLexResult *dcLexResult_create(const char *_text, TOKEN _token)
{
    dcLexResult *result = (dcLexResult *)dcMemory_allocate(sizeof(dcLexResult));
    result->text = dcMemory_strdup(_text);
    result->token = _token;
    return result;
}

dcNode *dcLexResult_createNode(const char *_text, TOKEN _token)
{
    return dcNode_createWithGuts(NODE_LEX_RESULT,
                                 dcLexResult_create(_text, _token));
}

void dcLexResult_free(dcLexResult **_result)
{
    dcMemory_free((*_result)->text);
    dcMemory_free((*_result));
}

void dcLexResult_freeNode(dcNode *_lexResultNode, dcDepth _depth)
{
    dcLexResult_free(&(CAST_LEXRESULT(_lexResultNode)));
}

dcLexResult *dcLexResult_copy(const dcLexResult *_from, dcDepth _depth)
{
    return dcLexResult_create(_from->text, _from->token);
}

void dcLexResult_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_LEXRESULT(_to) = dcLexResult_copy(CAST_LEXRESULT(_from), _depth);
}

/////////////
// dcLexer //
/////////////

void dcLexer_pushScopeDataFlags(dcLexer *_lexer)
{
    dcList_push(_lexer->scopeDataFlags,
                dcUnsignedInt32_createNode(SCOPE_DATA_PUBLIC));
}

void dcLexer_popScopeDataFlags(dcLexer *_lexer)
{
    dcList_pop(_lexer->scopeDataFlags, DC_DEEP);
}

void dcLexer_clearScopeDataFlags(dcLexer *_lexer)
{
    dcList_clear(_lexer->scopeDataFlags, DC_DEEP);
}

static void setCurrentScopeDataFlags(dcLexer *_lexer, dcScopeDataFlags _flags)
{
    CAST_INT(dcList_getTail(_lexer->scopeDataFlags)) = _flags;
}

dcScopeDataFlags dcLexer_getCurrentScopeDataFlags(dcLexer *_lexer)
{
    return (_lexer->scopeDataFlags->size > 0
            ? CAST_INT(dcList_getTail(_lexer->scopeDataFlags))
            : SCOPE_DATA_PUBLIC);
}

dcLexer *dcLexer_create(void)
{
    dcLexer *lexer = (dcLexer *)dcMemory_allocateAndInitialize(sizeof(dcLexer));

    // initialize the lexer variables //

    // the line number inits at 1 //
    lexer->lineNumber = 1;

    // the states //
    lexer->states = dcList_create();
    lexer->residuals = dcList_create();

    // the scope we're in //
    lexer->scopeDataFlags = dcList_create();

    lexer->errorString = NULL;
    yylval.string = NULL;

    lexer->stringString = dcString_create();
    lexer->verbatimTextString = dcString_create();
    lexer->keywordString = dcString_create();
    lexer->numberString = dcString_create();
    return lexer;
}

dcLexer *dcLexer_createForConsole(void)
{
    dcLexer *lexer = dcLexer_create();
    lexer->filenameId = dcStringManager_getStringId("<console>");
    return lexer;
}

dcLexer *dcLexer_createWithInput(const char *_filename,
                                 char *_input,
                                 bool _takeOwnership)
{
    dcLexer *lexer = dcLexer_create();
    dcLexer_setInput(lexer, _input, _takeOwnership);
    lexer->filenameId = dcStringManager_getStringId(_filename);
    return lexer;
}

dcLexer *dcLexer_createFromFile(const char *_filename, FILE *_file)
{
    dcString *input = dcFileManagement_extractAllInputFromFile(_file);

    if (input == NULL)
    {
        input = dcString_create();
    }

    dcString_seekEnd(input);
    dcString_appendCharacter(input, 0);
    dcString_seekBeginning(input);
    dcLexer *lexer = dcLexer_createWithInput(_filename, input->string, true);
    dcString_free(&input, DC_SHALLOW);
    return lexer;
}

void dcLexer_free(dcLexer **_lexer)
{
    dcLexer *lexer = *_lexer;

    if (lexer->input != NULL
        && lexer->inputOwned)
    {
        dcMemory_free(lexer->input);
    }

    // these are freed via memory regions
    dcMemory_free(lexer->states);
    dcMemory_free(lexer->residuals);

    dcList_free(&lexer->scopeDataFlags, DC_DEEP);
    dcString_free(&lexer->stringString, DC_DEEP);
    dcString_free(&lexer->verbatimTextString, DC_DEEP);
    dcString_free(&lexer->keywordString, DC_DEEP);
    dcString_free(&lexer->numberString, DC_DEEP);

    // error text is freed in memory regions
    dcMemory_free(*_lexer);
}

void dcLexer_freeNode(dcNode *_lexerNode, dcDepth _depth)
{
    dcLexer_free(&(CAST_LEXER(_lexerNode)));
}

void dcLexer_setInput(dcLexer *_lexer, char *_input, bool _takeOwnership)
{
    if (_lexer->input != NULL
        && _lexer->inputOwned)
    {
        dcMemory_free(_lexer->input);
    }

    _lexer->inputOwned = _takeOwnership;
    _lexer->input = _input;
    _lexer->inputLength = strlen(_lexer->input);
    _lexer->lexPosition = _lexer->lexStartPosition = 0;
    _lexer->atEnd = false;
}

const char *dcLexer_getFilename(const dcLexer *_lexer)
{
    return dcStringManager_getStringFromId(_lexer->filenameId);
}

void dcLexer_incrementLineNumber(dcLexer *_lexer)
{
    _lexer->lineNumber++;
}

void dcLexer_setErrorState(dcLexer *_lexer, bool _errorState)
{
    _lexer->errorState = _errorState;
}

void dcLexer_setErrorString(dcLexer *_lexer, const char *_errorString)
{
    _lexer->errorString = dcMemory_strdup(_errorString);
    dcMemory_trackMemory(_lexer->errorString);
}

static void resetTokens(dcLexer *_lexer)
{
    _lexer->lastToken = -1;
    _lexer->lexPosition = 0;
    _lexer->inMatrix = false;
    _lexer->bracketCount = 0;
    _lexer->stringBracketCount = 0;
}

void dcLexer_reset(dcLexer *_lexer)
{
    resetTokens(_lexer);
    _lexer->lexStartPosition = 0;
    _lexer->lexPosition = 0;
}

static void verifyOrder(const SymbolData *_symbolDatas, size_t _size)
{
    size_t i;
    int previousToken = -1;

    for (i = 0; i < _size; i++)
    {
        dcError_assert(previousToken == -1
                       || (_symbolDatas[i].symbol > previousToken));
        previousToken = _symbolDatas[i].symbol;
    }
}

void dcLexer_initialize(void)
{
    initializeMaps();

    // verify the order of sKeywordDatas and sTokenDatas
    verifyOrder(sKeywordDatas, dcTaffy_countOf(sKeywordDatas) - 1);
    verifyOrder(sTokenDatas, dcTaffy_countOf(sTokenDatas) - 1);

    if (sLexPointers == NULL)
    {
        sLexPointersSize = 256;

        sLexPointers = (dcLexer_lexPointer *)(dcMemory_allocate
                                              (sLexPointersSize
                                               * sizeof(&dcLexer_lexQuote)));
        memset(sLexPointers,
               0,
               sizeof(dcLexer_lexPointer) * sLexPointersSize);

        sLexPointers[(int)'"'] = &dcLexer_lexQuote;

        sLexPointers[(int)'1'] = &dcLexer_lexNumber;
        sLexPointers[(int)'2'] = &dcLexer_lexNumber;
        sLexPointers[(int)'3'] = &dcLexer_lexNumber;
        sLexPointers[(int)'4'] = &dcLexer_lexNumber;
        sLexPointers[(int)'5'] = &dcLexer_lexNumber;
        sLexPointers[(int)'6'] = &dcLexer_lexNumber;
        sLexPointers[(int)'7'] = &dcLexer_lexNumber;
        sLexPointers[(int)'8'] = &dcLexer_lexNumber;
        sLexPointers[(int)'9'] = &dcLexer_lexNumber;
        sLexPointers[(int)'0'] = &dcLexer_lexZero;

        sLexPointers[(int)'~']  = &dcLexer_lexTilde;
        sLexPointers[(int)'@']  = &dcLexer_lexAt;
        sLexPointers[(int)'\\'] = &dcLexer_lexBackSlash;
        sLexPointers[(int)'!']  = &dcLexer_lexBang;
        sLexPointers[(int)'&']  = &dcLexer_lexBitAnd;
        sLexPointers[(int)'^']  = &dcLexer_lexCaret;
        sLexPointers[(int)',']  = &dcLexer_lexComma;
        sLexPointers[(int)'.']  = &dcLexer_lexDot;
        sLexPointers[(int)'=']  = &dcLexer_lexEquals;
        sLexPointers[(int)'/']  = &dcLexer_lexForwardSlash;
        sLexPointers[(int)'>']  = &dcLexer_lexGreaterThan;
        sLexPointers[(int)'#']  = &dcLexer_lexHash;
        sLexPointers[(int)'{']  = &dcLexer_lexLeftBrace;
        sLexPointers[(int)'[']  = &dcLexer_lexLeftBracket;
        sLexPointers[(int)'(']  = &dcLexer_lexLeftParen;
        sLexPointers[(int)'<']  = &dcLexer_lexLessThan;
        sLexPointers[(int)'-']  = &dcLexer_lexMinus;
        sLexPointers[(int)'%']  = &dcLexer_lexModulus;
        sLexPointers[(int)'|']  = &dcLexer_lexPipe;
        sLexPointers[(int)'"']  = &dcLexer_lexQuote;
        sLexPointers[(int)'\n'] = &dcLexer_lexReturn;
        sLexPointers[(int)'}']  = &dcLexer_lexRightBrace;
        sLexPointers[(int)']']  = &dcLexer_lexRightBracket;
        sLexPointers[(int)')']  = &dcLexer_lexRightParen;
        sLexPointers[(int)';']  = &dcLexer_lexSemicolon;
        sLexPointers[(int)'\''] = &dcLexer_lexSingleQuote;
        sLexPointers[(int)'*']  = &dcLexer_lexStar;
        sLexPointers[(int)'+']  = &dcLexer_lexPlus;
        sLexPointers[(int)':']  = &dcLexer_lexColon;
        sLexPointers[(int)'?']  = &dcLexer_lexQuestionMark;

        sHexLookup[(int)'1'] = 1;
        sHexLookup[(int)'2'] = 2;
        sHexLookup[(int)'3'] = 3;
        sHexLookup[(int)'4'] = 4;
        sHexLookup[(int)'5'] = 5;
        sHexLookup[(int)'6'] = 6;
        sHexLookup[(int)'7'] = 7;
        sHexLookup[(int)'8'] = 8;
        sHexLookup[(int)'9'] = 9;
        sHexLookup[(int)'a'] = 10;
        sHexLookup[(int)'A'] = 10;
        sHexLookup[(int)'b'] = 11;
        sHexLookup[(int)'B'] = 11;
        sHexLookup[(int)'c'] = 12;
        sHexLookup[(int)'C'] = 12;
        sHexLookup[(int)'d'] = 13;
        sHexLookup[(int)'D'] = 13;
        sHexLookup[(int)'e'] = 14;
        sHexLookup[(int)'E'] = 14;
        sHexLookup[(int)'f'] = 15;
        sHexLookup[(int)'F'] = 15;
    }
}

void dcLexer_cleanup(void)
{
    dcMemory_free(sLexPointers);
}

#ifdef ENABLE_DEBUG
static const char * const lexerStateStrings[] = {
    "LEXER_NORMAL_STATE",
    "LEXER_STRING_STATE",
    "LEXER_IGNORE_RETURNS_STATE",
    "LEXER_IGNORE_RETURNS_FOR_METHOD_STATE",
    "LEXER_STRING_EXPRESSION_STATE",
    "LEXER_VERBATIM_TEXT_STATE",
    "LEXER_IDENTIFIER_STATE",
    "LEXER_NUMBER_STATE",
    "LEXER_PUBLIC_CLASS_STATE",
    "LEXER_PRIVATE_CLASS_STATE"
};
#endif

void dcLexer_pushState(dcLexer *_lexer, dcLexer_state _state)
{
    dcLog_log(LEXER_LOG,
              "pushing state: %s\n",
              lexerStateStrings[_state]);
    dcNode *state = dcUnsignedInt32_createNode(_state);
    dcList_push(_lexer->states, state);
    dcMemory_trackMemory(state);
    dcMemory_trackMemory(_lexer->states->tail);
}

void dcLexer_popState(dcLexer *_lexer)
{
    if (_lexer->states->size > 0)
    {
        dcListElement *tail = _lexer->states->tail;
        // hehe
        dcListElement_free(&tail, _lexer->states, DC_SPACE);

        dcLog_log(LEXER_LOG,
                  "popped state, new head: %s\n",
                  lexerStateStrings[getState(_lexer)]);
    }
}

void dcLexer_handleParseError(dcLexer *_lexer)
{
    dcLexer_setErrorState(_lexer, true);
    dcList_memoryRegionsFree(_lexer->states);
    dcList_memoryRegionsFree(_lexer->residuals);
}

static TOKEN lex(dcLexer *_lexer)
{
    TOKEN retval = EOF;
    bool again = false;

    yylval.string = NULL;
    dcLexer_setErrorState(_lexer, false);

    // reset some variables //
    _lexer->isFloat = false;
    _lexer->previousLineNumber = _lexer->lineNumber;

    if (_lexer->residuals->size > 0)
    {
        dcNode *residual = dcList_shift(_lexer->residuals, DC_SHALLOW);
        dcLexResult *result = CAST_LEXRESULT(residual);
        LEXED_WITHOUT_POSITION_INCREMENT(result->token, result->text);
        dcNode_free(&residual, DC_DEEP);
    }
    else
    {
        do
        {
            dcLexer_state state = getState(_lexer);
            again = false;

            if (_lexer->atEnd)
            {
                LEXED(1, EOF, tEOF_STRING);
                _lexer->atEnd = false;
            }
            else if (state == LEXER_VERBATIM_TEXT_STATE)
            {
                retval = dcLexer_lexInVerbatimTextState(_lexer, &again);
            }
            else if (state == LEXER_STRING_STATE)
            {
                retval = dcLexer_lexInStringState(_lexer, &again);
            }
            else if (state == LEXER_IDENTIFIER_STATE)
            {
                dcLexer_eatSpacesAndTabs(_lexer);
                retval = dcLexer_lexIdentifier(_lexer, &again);
                dcLexer_popState(_lexer);
                _lexer->gotSymbol = false;
            }
            else if (state == LEXER_NUMBER_STATE)
            {
                dcLexer_eatSpacesAndTabs(_lexer);
                retval = dcLexer_lexNumber(_lexer, &again);
            }
            else if (! ATEND)
            {
                dcLexer_eatSpacesAndTabs(_lexer);

                if ((ssize_t)CC >= sLexPointersSize
                    || (ssize_t)CC < 0)
                {
                    retval = parseErrorFormat(_lexer,
                                              "invalid character: %c",
                                              CC);
                }
                else
                {
                    dcLexer_lexPointer lexPointer = sLexPointers[(size_t)CC];

                    if (lexPointer != NULL)
                    {
                        retval = lexPointer(_lexer, &again);
                    }
                    else if (WORD_CHARACTER(CC)
                             || START_IDENTIFIER_CHARACTER(CC))
                    {
                        dcLexer_pushState(_lexer, LEXER_IDENTIFIER_STATE);
                        again = true;
                    }
                    else
                    {
                        retval = parseErrorFormat(_lexer,
                                                  "invalid character: %c",
                                                  CC);
                    }
                }
            }
            else
            {
                _lexer->atEnd = true;
                LEXED(1, tEXPR_END, tEXPR_END_STRING);
            }
        }
        while (again);
    }

    return retval;
}

TOKEN dcLexer_lex(dcLexer *_lexer)
{
    TOKEN result = EOF;

    // refuse to lex if there's a parse error
    if (! dcLexer_hasParseError(_lexer))
    {
        result = lex(_lexer);
    }

#ifdef ENABLE_DEBUG
    if (result != -1)
    {
        const SymbolData *symbolData = findSymbolDataFromToken(result);

        if (symbolData != NULL)
        {
            dcLog_log(LEXER_LOG, "lexed: \"%s\" ", _lexer->errorString);

            if (symbolData->name != NULL)
            {
                dcLog_append(LEXER_LOG, "| %s ", symbolData->name);
            }

            dcLog_append(LEXER_LOG, "| %s\n", symbolData->description);
        }
        else
        {
            dcLog_log(LEXER_LOG, "lexed: %c | %u\n", result, result);
        }
    }
#endif

    return result;
}

TOKEN dcLexer_lexQuestionMark(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    LEXED(1, '?', "?");
    return retval;
}

TOKEN dcLexer_lexTilde(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    size_t commentLevel = 1;
    _lexer->lexPosition++;

    if (! ATEND && CC == '=')
    {
        if (! ATEND_AT_CCNEXT && CCNEXT == '<')
        {
            // got ~=< //
            LEXED(2, tTILDE_EQUAL_LESS_THAN, tTILDE_EQUAL_LESS_THAN_STRING);
        }
        else
        {
            // got ~= //
            LEXED(1, tTILDE_EQUAL, tTILDE_EQUAL_STRING);
        }
    }
    else if (! ATEND && CC == '(')
    {
        // start of a comment //

        do
        {
            while (! ATEND && CC != ')' && CC != '~')
            {
                if (CC == '\n')
                {
                    dcLexer_incrementLineNumber(_lexer);
                }

                _lexer->lexPosition++;
            }

            if (ATEND)
            {
                retval = parseError(_lexer, "Unterminated comment");
                break;
            }

            if (!(ATEND_AT_CCNEXT))
            {
                if (CC == '~' && CCNEXT == '(')
                {
                    commentLevel++;
                    _lexer->lexPosition += 2;
                }
                else if (CC == ')' && CCNEXT == '~')
                {
                    _lexer->lexPosition += 2;
                    commentLevel--;
                }
                else
                {
                    _lexer->lexPosition++;
                }
            }
            else
            {
                _lexer->lexPosition++;
            }
        }
        while (commentLevel > 0);

        *_again = true;

        dcLexer_eatWhitespace(_lexer);
        dcLexer_eatWhitespace(_lexer);
    }
    else
    {
        LEXED_WITHOUT_POSITION_INCREMENT('~', tTILDE_STRING);
    }

    return retval;
}

//
// some keyword lines can end in tokens that otherwise would wrap a line
// for example, an "import" line can end in .*
// normally, Taffy would treat "*\n" as "*", but if an import is being parsed,
// then the statement must end at the newline
//
TOKEN dcLexer_lexReturn(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    dcLexer_incrementLineNumber(_lexer);
    _lexer->lexPosition++;
    dcLexer_state currentState = getState(_lexer);

    // TODO: put these values into an array for fast lookup
    if (currentState == LEXER_IGNORE_RETURNS_STATE
        || currentState == LEXER_IGNORE_RETURNS_FOR_METHOD_STATE
        || _lexer->lastToken == 0
        || _lexer->lastToken == tEXPR_END
        || _lexer->lastToken == '+'
        || _lexer->lastToken == '-'
        || (_lexer->lastToken == '*'
            && ! _lexer->inImport)
        || _lexer->lastToken == '/'
        || _lexer->lastToken == '='
        || _lexer->lastToken == '<'
        || _lexer->lastToken == '>'
        || _lexer->lastToken == '^'
        || _lexer->lastToken == '%'
        || _lexer->lastToken == '&'
        || _lexer->lastToken == '|'
        || _lexer->lastToken == ','
        || _lexer->lastToken == tLEFT_SHIFT
        || _lexer->lastToken == tRIGHT_SHIFT
        || _lexer->lastToken == tEQUAL_EQUAL
        || _lexer->lastToken == tLESS_THAN_OR_EQUAL
        || _lexer->lastToken == tGREATER_THAN_OR_EQUAL
        || _lexer->lastToken == tPLUS_EQUAL
        || _lexer->lastToken == tMINUS_EQUAL
        || _lexer->lastToken == tMULTIPLY_EQUAL
        || _lexer->lastToken == tDIVIDE_EQUAL
        || _lexer->lastToken == tPOWER_EQUAL
        || _lexer->lastToken == tTILDE_EQUAL
        || _lexer->lastToken == kAND
        || _lexer->lastToken == kOR
        || (_lexer->bracketCount > 0 && !_lexer->inStringExpression))
    {
        _lexer->leftBracket = 0;
        dcLexer_eatWhitespace(_lexer);
        *_again = true;
    }
    else
    {
        _lexer->inImport = false;
        dcLexer_eatWhitespace(_lexer);
        LEXED_WITHOUT_POSITION_INCREMENT(tEXPR_END, tEXPR_END_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexBackSlash(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == 'n')
        {
            _lexer->lexPosition += 2;

            // eat any remaining returns or semicolons
            if (_lexer->lexStartPosition != 0)
            {
                dcLexer_eatWhitespace(_lexer);
                LEXED_WITHOUT_POSITION_INCREMENT(tEXPR_END, tEXPR_END_STRING);
            }
            else
            {
                dcLexer_eatWhitespace(_lexer);
                *_again = true;
            }
        }
        else if (CCNEXT == 't')
        {
            _lexer->lexPosition += 2;
            *_again = true;
        }
        else
        {
            retval = parseError(_lexer, "unknown escape sequence");
        }
    }
    else
    {
        retval = parseError(_lexer, "blank escape sequence: \\");
    }

    return retval;
}

TOKEN dcLexer_lexSemicolon(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    // eat any remaining returns or semicolons
    if (_lexer->lexStartPosition != 0)
    {
        dcLexer_eatWhitespace(_lexer);
        LEXED_WITHOUT_POSITION_INCREMENT(tEXPR_END, tSEMI_COLON_STRING);
    }
    else
    {
        _lexer->lexPosition++;
        dcLexer_eatWhitespace(_lexer);
        *_again = true;
    }

    return retval;
}

TOKEN dcLexer_lexModulus(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '=')
        {
            // got %=, POWER_EQUAL //
            LEXED(2, tMODULUS_EQUAL, tMODULUS_EQUAL_STRING);
        }
        else
        {
            LEXED(1, '%', tMODULUS_STRING);
        }
    }
    else
    {
        LEXED(1, '%', tMODULUS_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexLeftBrace(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    _lexer->bracketCount = 0;

    if (getState(_lexer) == LEXER_IGNORE_RETURNS_FOR_METHOD_STATE)
    {
        dcLexer_popState(_lexer);
    }

    dcLexer_pushState(_lexer, LEXER_NORMAL_STATE);
    _lexer->lexPosition++;
    dcLexer_eatWhitespace(_lexer);

    // is this reverse really needed?
    _lexer->lexPosition--;
    LEXED(1, '{', tLEFT_BRACE_STRING);
    return retval;
}

TOKEN dcLexer_lexRightBrace(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    // pop LEXER_NORMAL_STATE
    dcLexer_popState(_lexer);

    // check for an else coming up //
    if (dcLexer_isNextText(_lexer, "else")
        || dcLexer_isNextText(_lexer, "catch"))
    {
        // discard any whitespace between '}' and
        // ('else' or 'catch')
        _lexer->lexPosition++;
        dcLexer_eatWhitespace(_lexer);
        LEXED_WITHOUT_POSITION_INCREMENT('}', tRIGHT_BRACE_STRING);
    }
    else
    {
        _lexer->lexPosition++;

        if (! ATEND && CC == '}')
        {
            dcLexer_eatSpacesAndTabs(_lexer);
            LEXED(1, tBLOCK_END, tBLOCK_END_STRING);
        }
        else
        {
            dcLexer_eatSpacesAndTabs(_lexer);
            LEXED_WITHOUT_POSITION_INCREMENT('}', tRIGHT_BRACE_STRING);
        }
    }

    return retval;
}

TOKEN dcLexer_lexQuote(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (ATEND_AT_CCNEXT)
    {
        retval = parseError(_lexer, "Unterminated string");
    }
    else
    {
        dcLexer_pushState(_lexer, LEXER_STRING_STATE);
        LEXED(1, '"', tQUOTE_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexSingleQuote(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! _lexer->gotSymbol)
    {
        if (ATEND_AT_CCNEXT)
        {
            retval = parseError(_lexer, "Unterminated symbol");
        }
        else if (CCNEXT == '{')
        {
            // got the start of a verbatim text: '{
            _lexer->lexPosition += 2;
            dcLexer_pushState(_lexer, LEXER_VERBATIM_TEXT_STATE);
            LEXED_WITHOUT_POSITION_INCREMENT(tVERBATIM_TEXT_START,
                                             tVERBATIM_TEXT_START_STRING);
        }
        else
        {
            _lexer->lexPosition++;
            dcLexer_pushState(_lexer, LEXER_IDENTIFIER_STATE);
            _lexer->gotSymbol = true;
            *_again = true;
        }
    }

    return retval;
}

TOKEN dcLexer_lexColon(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    LEXED(1, ':', tCOLON_STRING);
    return retval;
}

TOKEN dcLexer_lexComma(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    LEXED(1, ',', tCOMMA_STRING);
    return retval;
}

TOKEN dcLexer_lexLeftBracket(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    dcLexer_pushState(_lexer, LEXER_IGNORE_RETURNS_STATE);

    _lexer->bracketCount++;
    _lexer->leftBracket++;

    if (_lexer->inStringExpression)
    {
        _lexer->stringBracketCount++;
    }

    LEXED(1, '[', tLEFT_BRACKET_STRING);
    return retval;
}

TOKEN dcLexer_lexRightBracket(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    size_t saveLexPosition = 0;

    // LEXER_IGNORE_RETURNS_STATE or LEXER_NORMAL_STATE
    dcLexer_popState(_lexer);

    if (_lexer->bracketCount > 0)
    {
        _lexer->bracketCount--;
    }

    if (_lexer->inStringExpression)
    {
        _lexer->stringBracketCount--;

        if (_lexer->stringBracketCount == 0)
        {
            _lexer->inStringExpression = false;
            LEXED(1, ']', (char*)tRIGHT_BRACKET_STRING);
        }
        else
        {
            LEXED(1, ']', (char*)tRIGHT_BRACKET_STRING);
        }
    }
    else
    {
        if (_lexer->leftBracket > 0)
        {
            _lexer->lexPosition++;
            saveLexPosition = _lexer->lexPosition;

            size_t returnsEaten = dcLexer_eatWhitespace(_lexer);

            if (! ATEND
                && CC == '='
                && (ATEND_AT(_lexer->lexPosition + 1)
                    || (! ATEND_AT(_lexer->lexPosition + 1)
                        && CCAT(_lexer->lexPosition + 1) != '=')))
            {
                _lexer->lexPosition++;

                LEXED_WITHOUT_POSITION_INCREMENT(tRIGHT_BRACKET_EQUAL,
                                                 tRIGHT_BRACKET_EQUAL_STRING);
            }
            else
            {
                // a litty bit of haxy but we all need a little bit sometimes //
                _lexer->lexPosition = saveLexPosition - 1;
                _lexer->lineNumber -= returnsEaten;

                LEXED(1, ']', tRIGHT_BRACKET_STRING);
            }

            _lexer->leftBracket--;
        }
        else
        {
            LEXED(1, ']', tRIGHT_BRACKET_STRING);
        }
    }

    return retval;
}

TOKEN dcLexer_lexAt(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    bool metaVariable = false;

    if (ATEND)
    {
        retval = parseError(_lexer, "@ must be followed by a string");
    }

    dcString_clear(_lexer->keywordString);
    dcString_appendCharacter(_lexer->keywordString, '@');
    _lexer->lexPosition++;

    if (CC == '@')
    {
        metaVariable = true;
        dcString_appendCharacter(_lexer->keywordString, '@');
        _lexer->lexPosition++;
    }

    while (! ATEND && WORD_CHARACTER(CC))
    {
        dcString_appendCharacter(_lexer->keywordString, CC);
        _lexer->lexPosition++;
    }

    char *stringResult = dcMemory_strdup(_lexer->keywordString->string);
    yylval.string = stringResult;

    if (EXPR_END(CC)
        || ATEND
        || sReservedMap[(int)CC])
    {
        //
        // check for @r, @w, and @rw
        //
        if (strcmp(stringResult, kRW_STRING) == 0)
        {
            LEXED_WITHOUT_POSITION_INCREMENT(kRW, kRW_STRING);
        }
        else if (strcmp(stringResult, kR_STRING) == 0)
        {
            LEXED_WITHOUT_POSITION_INCREMENT(kR, kR_STRING);
        }
        else if (strcmp(stringResult, kW_STRING) == 0)
        {
            LEXED_WITHOUT_POSITION_INCREMENT(kW, kW_STRING);
        }
        else if (strcmp(stringResult, kPUBLIC_STRING) == 0)
        {
            dcMemory_free(stringResult);
            setCurrentScopeDataFlags(_lexer, SCOPE_DATA_PUBLIC);
            *_again = true;
        }
        else if (strcmp(stringResult, kPROTECTED_STRING) == 0)
        {
            dcMemory_free(stringResult);
            setCurrentScopeDataFlags(_lexer, SCOPE_DATA_PROTECTED);
            *_again = true;
        }
        else
        {
            //
            // is not @r, @w, or @rw, but is an instance or meta variable
            //
            if (metaVariable)
            {
                LEXED_WITHOUT_POSITION_INCREMENT(tMETA_SCOPED_IDENTIFIER,
                                                 stringResult);
            }
            else
            {
                LEXED_WITHOUT_POSITION_INCREMENT(tINSTANCE_SCOPED_IDENTIFIER,
                                                 stringResult);
            }
        }

        dcString_clear(_lexer->keywordString);
    }
    else
    {
        char *errorString = dcLexer_sprintf("invalid class variable: %s%c",
                                            stringResult,
                                            CC);
        retval = parseError(_lexer, errorString);
        dcMemory_free(errorString);
        dcMemory_free(stringResult);
    }

    return retval;
}

static bool haveString(dcLexer *_lexer,
                       const char *_string,
                       size_t _length)
{
    bool result = false;

    if (! ATEND_AT((_lexer->lexPosition + _length) - 1))
    {
        size_t i;
        result = true;

        for (i = 0; i < _length; i++)
        {
            if (CCAT(_lexer->lexPosition + i) != _string[i])
            {
                result = false;
                break;
            }
        }
    }

    return result;
}

#define HAVE_STRING(_lexer, _string)            \
    haveString(_lexer, _string, sizeof(_string) - 1)

TOKEN dcLexer_lexLeftParen(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    TOKEN preRetval = EOF;
    bool isMethod = false;
    _lexer->lexPosition++;

    if (HAVE_STRING(_lexer, "@)"))
    {
        // (@)
        yylval.string = (char*)tMETHOD_INSTANCE_STRING;
        preRetval = tMETHOD_INSTANCE;
        isMethod = true;
    }
    else if (HAVE_STRING(_lexer, "@@)"))
    {
        // (@@)
        yylval.string = (char*)tMETHOD_META_STRING;
        preRetval = tMETHOD_META;
        isMethod = true;
    }

    if (isMethod)
    {
        dcLexer_pushState(_lexer, LEXER_IGNORE_RETURNS_FOR_METHOD_STATE);
        LEXED(strlen(yylval.string) - 1, preRetval, yylval.string);
    }
    else
    {
        dcLexer_pushState(_lexer, LEXER_IGNORE_RETURNS_STATE);
        LEXED_WITHOUT_POSITION_INCREMENT('(', tLEFT_PAREN_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexRightParen(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    _lexer->lexPosition++;
    dcLexer_eatSpacesAndTabs(_lexer);

    //
    // Sometimes this function isn't called even when a right paren
    // is given in the input to the lexer.
    //
    // Such as for instance methods: (@) foo
    // or for class methods: (@@) foo
    //
    // In this case, the dcLexer_lexLeftParen() function takes
    // care of lexing the right paren (see above)
    //

    dcLexer_popState(_lexer);

    if (! ATEND
        && CC == '='
        && ! ATEND_AT_CCNEXT
        && CCNEXT != '=')
    {
        _lexer->lexPosition++;

        LEXED_WITHOUT_POSITION_INCREMENT(tRIGHT_PAREN_EQUAL,
                                         tRIGHT_PAREN_EQUAL_STRING);
    }
    else
    {
        LEXED_WITHOUT_POSITION_INCREMENT(')', tRIGHT_PAREN_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexBitAnd(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '=')
        {
            LEXED(2, tBIT_AND_EQUAL, tBIT_AND_EQUAL_STRING);
        }
        else
        {
            LEXED(1, '&', tBIT_AND_STRING);
        }
    }
    else
    {
        LEXED(1, '&', tBIT_AND_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexPipe(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '|')
        {
            if (_lexer->inMatrix)
            {
                // pop LEXER_NORMAL_STATE
                dcLexer_popState(_lexer);
            }
            else
            {
                dcLexer_pushState(_lexer, LEXER_NORMAL_STATE);
            }

            _lexer->inMatrix = ! _lexer->inMatrix;

            // got ||
            LEXED(2, tMATRIX_DELIMITER, tMATRIX_DELIMITER_STRING);
        }
        else if (CCNEXT == '=')
        {
            // |=
            LEXED(2, tBIT_OR_EQUAL, tBIT_OR_EQUAL_STRING);
        }
        else
        {
            LEXED(1, '|', tBIT_OR_STRING);
        }
    }
    else
    {
        LEXED(1, '|', tBIT_OR_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexEquals(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '=')
        {
            LEXED(2, tEQUAL_EQUAL, tEQUAL_EQUAL_STRING);
        }
        else if (CCNEXT == '>')
        {
            LEXED(2, tRIGHT_ARROW, tRIGHT_ARROW_STRING);
        }
        else
        {
            LEXED(1, '=', tEQUAL_STRING);
        }
    }
    else
    {
        LEXED(1, '=', tEQUAL_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexStar(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT && CCNEXT == '=')
    {
        LEXED(2, tMULTIPLY_EQUAL, tMULTIPLY_EQUAL_STRING);
    }
    else
    {
        LEXED(1, '*', tMULTIPLY_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexForwardSlash(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '/')
        {
            // got a comment, eat the line
            _lexer->lexPosition++;
            bool returnExprEnd =
                (_lexer->lineNumber == _lexer->lastTokenLineNumber);
            dcLexer_eatLine(_lexer);

            dcParser_setGotComment();

            // if we haven't incremented our line number, then this comment
            // exists on the same line as some non-comments
            // so we want specify that an EXPR_END was hit
            if (returnExprEnd
                && _lexer->bracketCount == 0
                && _lexer->stringBracketCount == 0
                && getState(_lexer) != LEXER_IGNORE_RETURNS_STATE)
            {
                LEXED_WITHOUT_POSITION_INCREMENT(tEXPR_END, tEXPR_END_STRING);
            }
            else
            {
                // keep going, comments are like whitespace
                *_again = true;
            }
        }
        else if (CCNEXT == '=')
        {
            LEXED(2, tDIVIDE_EQUAL, tDIVIDE_EQUAL_STRING);
        }
        else
        {
            LEXED(1, '/', tDIVIDE_STRING);
        }
    }
    else
    {
        // got /, DIVIDE //
        LEXED(1, '/', tDIVIDE_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexCaret(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '=')
        {
            // got ^=, POWER_EQUAL //
            LEXED(2, tPOWER_EQUAL, tPOWER_EQUAL_STRING);
        }
        else if (CCNEXT == '^')
        {
            if (TOKEN_AHEAD(_lexer, 2, '='))
            {
                // got ^^=, XOR_EQUAL
                LEXED(3, tBIT_XOR_EQUAL, tBIT_XOR_EQUAL_STRING);
            }
            else
            {
                // got ^^, BIT_XOR
                LEXED(2, tBIT_XOR, tBIT_XOR_STRING);
            }
        }
        else if (CCNEXT == '{')
        {
            // we have a block start, ^{, now check whether we have ^{ or ^{<
            // ^{< is a block start, with arguments, but it's prettier if
            // written like ^{ <

            // march to '{'
            _lexer->lexPosition++;

            if (ATEND_AT_CCNEXT)
            {
                retval = parseError(_lexer, "unterminated block");
            }
            else
            {
                if (getState(_lexer) == LEXER_IGNORE_RETURNS_FOR_METHOD_STATE)
                {
                    dcLexer_popState(_lexer);
                }

                _lexer->bracketCount = 0;
                dcLexer_pushState(_lexer, LEXER_NORMAL_STATE);
                _lexer->lexPosition++;
                dcLexer_eatWhitespace(_lexer);

                if (! ATEND && CC == '<')
                {
                    // we have ^{< (a block start with arguments)
                    LEXED(1, tLEFT_BRACE_LESS_THAN,
                          tLEFT_BRACE_LESS_THAN_STRING);
                }
                else
                {
                    LEXED_WITHOUT_POSITION_INCREMENT(tBLOCK_START,
                                                     tBLOCK_START_STRING);
                }
            }
        }
        else
        {
            LEXED(1, '^', tPOWER_STRING);
        }
    }
    else
    {
        // got ^, POWER //
        LEXED(1, '^', tPOWER_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexPlus(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '=')
        {
            // got +=, PLUS_EQUAL
            LEXED(2, tPLUS_EQUAL, tPLUS_EQUAL_STRING);
        }
        else if (CCNEXT == '+')
        {
            // got ++, PLUS_PLUS
            LEXED(2, tPLUS_PLUS, tPLUS_PLUS_STRING);
        }
        else
        {
            // got +, PLUS
            LEXED(1, '+', tPLUS_STRING);
        }
    }
    else
    {
        // got +, PLUS
        LEXED(1, '+', tPLUS_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexMinus(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '=')
        {
            // got -=, MINUS_EQUAL
            LEXED(2, tMINUS_EQUAL, tMINUS_EQUAL_STRING);
        }
        else if (CCNEXT == '-')
        {
            // got --, MINUS_MINUS
            LEXED(2, tMINUS_MINUS, tMINUS_MINUS_STRING);
        }
        else
        {
            // got -, MINUS
            LEXED(1, '-', tMINUS_STRING);
        }
    }
    else
    {
        // got -, MINUS
        LEXED(1, '-', tMINUS_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexLessThan(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '=')
        {
            // got <=, LESS_THAN_OR_EQUAL
            LEXED(2,
                  tLESS_THAN_OR_EQUAL,
                  tLESS_THAN_OR_EQUAL_STRING);
        }
        else if (CCNEXT == '<')
        {
            if (TOKEN_AHEAD(_lexer, 2, '='))
            {
                // got <<=, left shift equal
                LEXED(3, tLEFT_SHIFT_EQUAL, tLEFT_SHIFT_EQUAL_STRING);
            }
            else
            {
                // got <<, left shift
                LEXED(2, tLEFT_SHIFT, tLEFT_SHIFT_STRING);
            }
        }
        else
        {
            // got <, LESS_THAN
            LEXED(1, '<', tLESS_THAN_STRING);
        }
    }
    else
    {
        // got <, LESS_THAN
        LEXED(1, '<', tLESS_THAN_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexHash(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    TOKEN token = EOF;
    dcString *keywordString = _lexer->keywordString;
    dcString_appendCharacter(keywordString, '#');

    for (_lexer->lexPosition += 1;
         ! ATEND && WORD_CHARACTER(CC);
         _lexer->lexPosition += 1)
    {
        dcString_appendCharacter(keywordString, CC);
    }

    if (! ATEND
        && ! WHITESPACE(CC)
        && ! sMethodAttributeMap[(int)CC]
        && CC != ',')
    {
        retval = parseError(_lexer, "# followed by invalid character");
    }
    else
    {
        const SymbolData *const lookup =
            findSymbolDataFromKeyword(_lexer->keywordString->string);

        if (lookup == NULL)
        {
            parseErrorFormat(_lexer,
                             "invalid keyword: %s",
                             keywordString->string);
        }
        else
        {
            yylval.constString = lookup->name;
            token = lookup->symbol;
            LEXED_WITHOUT_POSITION_INCREMENT(token, yylval.constString);
        }
    }

    dcString_clear(keywordString);
    return retval;
}

TOKEN dcLexer_lexGreaterThan(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '=')
        {
            // got >=, GREATERE_THAN_OR_EQUAL
            LEXED(2,
                  tGREATER_THAN_OR_EQUAL,
                  tGREATER_THAN_OR_EQUAL_STRING);
        }
        else if (CCNEXT == '>')
        {
            if (TOKEN_AHEAD(_lexer, 2, '='))
            {
                // got >>=, right shift equal
                LEXED(3, tRIGHT_SHIFT_EQUAL, tRIGHT_SHIFT_EQUAL_STRING);
            }
            else
            {
                // got >>, right shift
                LEXED(2, tRIGHT_SHIFT, tRIGHT_SHIFT_STRING);
            }
        }
        else
        {
            // got >, GREATER_THAN
            LEXED(1, '>', tGREATER_THAN_STRING);
        }
    }
    else
    {
        // got >, GREATER_THAN
        LEXED(1, '>', tGREATER_THAN_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexBang(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    _lexer->lexPosition++;

    if (! ATEND && CC == '=')
    {
        // got !=, falseT_EQUAL
        _lexer->lexPosition++;
        LEXED_WITHOUT_POSITION_INCREMENT(tNOT_EQUAL,
                                         tNOT_EQUAL_STRING);
    }
    else
    {
        // got !, BANG
        LEXED_WITHOUT_POSITION_INCREMENT('!', tBANG_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexDot(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    _lexer->lexPosition++;

    if (ATEND)
    {
        retval = parseError(_lexer, ".<EOF>");
    }
    else
    {
        LEXED_WITHOUT_POSITION_INCREMENT(tDOT, tDOT_STRING);
    }

    return retval;
}

// need some upper limit, what should it be?
#define MAX_NUMBER_CHARACTER_LIMIT 0xFFFF

TOKEN dcLexer_lexBinaryNumber(dcLexer *_lexer)
{
    TOKEN retval = EOF;
    dcString *numberString = _lexer->numberString;
    dcString_clear(numberString);

    while (! ATEND && BINARY_DIGIT_AT(_lexer->lexPosition))
    {
        dcString_appendCharacter(numberString, CCAT(_lexer->lexPosition));
        _lexer->lexPosition++;
    }

    if (numberString->index >= MAX_NUMBER_CHARACTER_LIMIT)
    {
        retval = parseErrorFormat(_lexer,
                                  "binary number is too long, max %u",
                                  MAX_NUMBER_CHARACTER_LIMIT);
    }
    else
    {
        // or / 3?
        uint32_t lsuSize = (uint32_t)numberString->index;
        dcNumber *finalNumber =
            dcNumber_createFromInt32uWithLsuSize(0, lsuSize);
        dcNumber *power = dcNumber_createFromInt32uWithLsuSize(1, lsuSize);
        dcNumber *two = dcNumber_createFromInt32uWithLsuSize(2, lsuSize);
        uint64_t i;

        for (i = numberString->index - 1; ; i--)
        {
            int number = sHexLookup[(int)numberString->string[i]];

            if (number == 1)
            {
                dcNumber_add(finalNumber, finalNumber, power);
            }

            dcNumber_multiply(power, power, two);

            if (i == 0)
            {
                break;
            }
        }

        dcNumber_free(&power, DC_DEEP);
        dcNumber_free(&two, DC_DEEP);
        yylval.number = finalNumber;
        LEXED_WITHOUT_POSITION_INCREMENT(tNUMBER, numberString->string);
    }

    dcString_clear(numberString);
    return retval;
}

TOKEN dcLexer_lexHexNumber(dcLexer *_lexer)
{
    TOKEN retval = EOF;
    dcString *numberString = _lexer->numberString;
    dcString_appendString(numberString, "0x");
    bool atLeastOne = false;
    dcString_clear(numberString);

    while (! ATEND
           && HEX_DIGIT_AT(_lexer->lexPosition))
    {
        dcString_appendCharacter(numberString, CCAT(_lexer->lexPosition));
        _lexer->lexPosition++;
        atLeastOne = true;
    }

    if (! atLeastOne
        || (! ATEND
            && ! WHITESPACE(_lexer->lexPosition)
            && ! sReservedMap[(int)CC]))
    {
        retval = parseErrorFormat(_lexer,
                                  "invalid hexadecimal number: %s",
                                  _lexer->numberString->string);
    }
    else
    {
        // need some upper limit, what should it be?
        if (numberString->index >= MAX_NUMBER_CHARACTER_LIMIT)
        {
            retval = parseErrorFormat(_lexer,
                                      "hex number is too long, max %u",
                                      MAX_NUMBER_CHARACTER_LIMIT);
        }
        else
        {
            uint64_t i;
            // there's up to 2 base-10 digits for every base-16 digit
            uint32_t lsuSize = (uint32_t)numberString->index * 2;
            dcNumber *finalNumber =
                dcNumber_createFromInt32uWithLsuSize(0, lsuSize);
            dcNumber *power = dcNumber_createFromInt32uWithLsuSize(1, lsuSize);
            dcNumber *sixteen = dcNumber_createFromInt32u(16);

            for (i = numberString->index - 1; ; i--)
            {
                int number = sHexLookup[(int)numberString->string[i]];

                if (number != 0)
                {
                    dcNumber *component =
                        dcNumber_createFromInt32uWithLsuSize(number, lsuSize);
                    dcNumber_multiply(component, component, power);
                    dcNumber_add(finalNumber, finalNumber, component);
                    dcNumber_free(&component, DC_DEEP);
                }

                dcNumber_multiply(power, power, sixteen);

                if (i == 0)
                {
                    break;
                }
            }

            dcNumber_free(&power, DC_DEEP);
            dcNumber_free(&sixteen, DC_DEEP);
            yylval.number = finalNumber;
            LEXED_WITHOUT_POSITION_INCREMENT(tNUMBER, numberString->string);
            dcString_clear(numberString);
        }
    }

    return retval;
}

TOKEN dcLexer_lexZero(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    bool justZero = true;

    if (! ATEND_AT_CCNEXT)
    {
        if (CCNEXT == '.'
            && DIGIT_AT(_lexer->lexPosition + 2))
        {
            _lexer->isFloat = true;
            dcString_appendCharacter(_lexer->numberString, '0');
            _lexer->lexPosition++;
            *_again = true;
            dcLexer_pushState(_lexer, LEXER_NUMBER_STATE);
            justZero = false;
        }
        else if (CCNEXT == 'x'
                 && HEX_DIGIT_AT(_lexer->lexPosition + 2))
        {
            _lexer->lexPosition += 2;
            retval = dcLexer_lexHexNumber(_lexer);
            justZero = false;
        }
        else if (CCNEXT == 'b'
                 && BINARY_DIGIT_AT(_lexer->lexPosition + 2))
        {
            _lexer->lexPosition += 2;
            retval = dcLexer_lexBinaryNumber(_lexer);
            justZero = false;
        }
        else if (CCNEXT == 'i'
                 && END_OF_TOKEN_AT(_lexer->lexPosition + 2))
        {
            // imaginary number! but it's really 0
            yylval.number = dcNumber_createFromInt32u(0);
            LEXED(2, tNUMBER, tZERO_STRING);
            justZero = false;
        }
        else if (POSITIVE_DIGIT_AT(_lexer->lexPosition + 1))
        {
            retval = parseError(_lexer, "positive digit found");
        }
    }

    if (justZero)
    {
        yylval.number = dcNumber_createFromInt32u(0);
        LEXED(1, tNUMBER, tZERO_STRING);
    }

    return retval;
}

TOKEN dcLexer_lexNumber(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    bool firstLex = true;
    bool gotDot = false;
    dcString *numberString = _lexer->numberString;

    while (! ATEND
           && ((! firstLex && CC == '0')
               || (_lexer->isFloat && CC == '0')
               || POSITIVE_DIGIT
               || CC == '.'))
    {
        firstLex = false;

        if (CC == '.' && ! gotDot)
        {
            if (TOKEN_AHEAD(_lexer, 1, '.'))
            {
                // got two ..'s
                break;
            }

            gotDot = true;
            _lexer->isFloat = true;
        }
        else if (CC == '.' && gotDot)
        {
            retval = parseError(_lexer, "invalid dot sequence");
            break;
        }

        dcString_appendCharacter(numberString, CC);
        _lexer->lexPosition++;
    }

    bool imaginary = false;

    if (_lexer->input[_lexer->lexPosition] == 'i'
        && END_OF_TOKEN_AT(_lexer->lexPosition + 1))
    {
        // imaginary number!
        yylval.complexNumber =
            dcComplexNumber_createFromString(numberString->string);
        LEXED(1, tCOMPLEX_NUMBER, numberString->string);
        imaginary = true;
    }
    else if (! ATEND_AT(_lexer->lexPosition + 2)
             && CC == 'E'
             && (CCNEXT == '+'
                 || CCNEXT == '-'))
    {
        dcString *exponentString = dcString_create();
        dcString_appendCharacter(exponentString, CCNEXT);

        _lexer->lexPosition += 2;

        // exponential number!
        while (! END_OF_TOKEN_AT(_lexer->lexPosition))
        {
            dcString_appendCharacter(exponentString, CC);
            _lexer->lexPosition++;
        }

        dcNumber *base = dcNumber_createFromString(numberString->string);
        dcNumber *exponent = dcNumber_createFromString(exponentString->string);
        dcNumber_raise(exponent, dcNumber_getConstant(10), exponent);
        dcNumber_multiply(base, base, exponent);
        dcString_free(&exponentString, DC_DEEP);
        dcNumber_free(&exponent, DC_DEEP);

        yylval.number = base;
        LEXED_WITHOUT_POSITION_INCREMENT(tNUMBER, numberString->string);
    }
    else
    {
        yylval.number = dcNumber_createFromString(numberString->string);
        LEXED_WITHOUT_POSITION_INCREMENT(tNUMBER, numberString->string);
    }

    // clear the number strings //
    dcString_clear(numberString);

    // reset the float flag //
    _lexer->isFloat = false;

    if (getState(_lexer) == LEXER_NUMBER_STATE)
    {
        dcLexer_popState(_lexer);
    }

    // 3x => 3 * x
    if (! imaginary
        && ! ATEND
        && (isalpha((unsigned char)CC)
            || CC == '('))
    {
        dcList_push(_lexer->residuals,
                    dcLexResult_createNode(tMULTIPLY_STRING, '*'));
    }

    return retval;
}

TOKEN dcLexer_lexIdentifier(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    TOKEN token = EOF;
    bool colon = false;
    dcString *keywordString = _lexer->keywordString;

    // check for keywords
    if (WORD_CHARACTER(CC) || START_IDENTIFIER_CHARACTER(CC))
    {
        dcString_appendCharacter(keywordString, CC);
        _lexer->lexPosition++;

        while (! ATEND
               && ((_lexer->gotSymbol
                    && (! sReservedMap[(int)CC]
                        || CC == ':'))
                   || (!_lexer->gotSymbol
                       && WORD_CHARACTER(CC))
                   || SPECIAL_END_WORD_CHARACTER(CC)))
        {
            dcString_appendCharacter(keywordString, CC);
            _lexer->lexPosition++;

            if (SPECIAL_END_WORD_CHARACTER(CCAT(_lexer->lexPosition - 1)))
            {
                break;
            }
        }

        if (! ATEND && (CC == ':'))
        {
            colon = true;
            dcString_appendCharacter(keywordString, ':');
            _lexer->lexPosition++;
        }

        token = EOF;
        dcLexer_setErrorString(_lexer, keywordString->string);

        if (_lexer->gotSymbol)
        {
            yylval.string = dcMemory_strdup(keywordString->string);
            token = tSYMBOL;
        }
        else
        {
            const SymbolData *data =
                findSymbolDataFromKeyword(keywordString->string);

            if (data != NULL)
            {
                yylval.constString = data->description;
                token = data->symbol;

                if (data->symbol == kIMPORT)
                {
                    _lexer->inImport = true;
                }

                // save the class line for error reporting
                if (data->symbol == kCLASS)
                {
                    _lexer->classLineNumberSave = _lexer->lineNumber;
                }
            }
        }

        if (token == EOF)
        {
            if (keywordString->index > 1 && colon)
            {
                yylval.string = dcMemory_strdup(keywordString->string);
                token = tMETHOD_PARAMETER;
            }
            else
            {
                yylval.string = dcMemory_strdup(keywordString->string);
                dcLexer_eatSpacesAndTabs(_lexer);
                token = tWORD;
            }
        }

        if (token == tWORD
            && dcSystem_isAutomaticFunction(yylval.string))
        {
            LEXED_WITHOUT_POSITION_INCREMENT(kAUTOMATIC_FUNCTION,
                                             yylval.string);
        }
        else
        {
            LEXED_WITHOUT_POSITION_INCREMENT(token, yylval.string);
        }
    }
    else
    {
        retval = parseError(_lexer, "invalid identifier");
    }

    dcString_clear(keywordString);

    return retval;
}

TOKEN dcLexer_lexInVerbatimTextState(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;
    dcLexer_popState(_lexer);

    while (! ATEND)
    {
        if (HAVE_STRING(_lexer, "'}"))
        {
            // we're done!
            _lexer->lexPosition += 2;
            yylval.string = dcMemory_strdup(_lexer->verbatimTextString->string);
            dcString_clear(_lexer->verbatimTextString);
            LEXED_WITHOUT_POSITION_INCREMENT(tSTRING, yylval.string);
            break;
        }
        else if (HAVE_STRING(_lexer, "\\'}"))
        {
            // it's escaped! increased again below to a total of 3
            _lexer->lexPosition += 2;
            dcString_appendString(_lexer->verbatimTextString, "'}");
        }
        else
        {
            dcString_appendCharacter(_lexer->verbatimTextString, CC);
        }

        _lexer->lexPosition++;
    }

    return retval;
}

TOKEN dcLexer_lexInStringState(dcLexer *_lexer, bool *_again)
{
    TOKEN retval = EOF;

    if (ATEND)
    {
        dcLexer_popState(_lexer);
        retval = parseError(_lexer, "Unterminated string");
    }

    if (CC == '"')
    {
        dcLexer_popState(_lexer);
        LEXED(1, '"', tQUOTE_STRING);
    }
    else if (CC == '#' && ! ATEND_AT_CCNEXT && CCNEXT == '[')
    {
        // got a #[ //
        _lexer->bracketCount++;
        _lexer->stringBracketCount = 1;
        _lexer->inStringExpression = true;

        dcLexer_pushState(_lexer, LEXER_NORMAL_STATE);

        LEXED(2,
              tSTRING_EXPRESSION_START,
              tSTRING_EXPRESSION_START_STRING);
    }
    else
    {
        while (! (ATEND
                  || CC == '"'
                  || (CC == '#'
                      && ! ATEND_AT_CCNEXT
                      && CCNEXT == '['))
               && ! dcLexer_hasParseError(_lexer))
        {
            if (CC == '\\')
            {
                if (! ATEND_AT_CCNEXT)
                {
                    _lexer->lexPosition++;

                    if (CC == '"')
                    {
                        dcString_appendCharacter(_lexer->stringString, '"');
                    }
                    else if (CC == 'n')
                    {
                        dcString_appendCharacter(_lexer->stringString, '\n');
                    }
                    else if (CC == 't')
                    {
                        dcString_appendCharacter(_lexer->stringString, '\t');
                    }
                    else if (CC == '\\')
                    {
                        dcString_appendCharacter(_lexer->stringString, '\\');
                    }
                    else if (CC == '#')
                    {
                        dcString_appendCharacter(_lexer->stringString, '#');
                    }
                    else
                    {
                        retval = (parseErrorFormat
                                  (_lexer,
                                   "unknown escape sequence: \\%c",
                                   CC));
                    }
                }
                else
                {
                    retval = parseError(_lexer,
                                        "\\ cannot appear at end of line");
                }
            }
            else
            {
                if (CC == '\n')
                {
                    dcLexer_incrementLineNumber(_lexer);
                }

                dcString_appendCharacter(_lexer->stringString, CC);
            }

            if (! dcLexer_hasParseError(_lexer))
            {
                _lexer->lexPosition++;
            }
        }

        if (! dcLexer_hasParseError(_lexer))
        {
            if (ATEND)
            {
                dcLexer_popState(_lexer);
                retval = parseError(_lexer, "Unterminated string");
            }
            else
            {
                bool stringExpression = false;

                if (CC == '"'
                    || CC == '#'
                    || (stringExpression =
                        (CC == '#'
                         && ! ATEND_AT_CCNEXT
                         && CCNEXT == '[')))
                {
                    yylval.string
                        = dcMemory_strdup(_lexer->stringString->string);
                    dcString_clear(_lexer->stringString);

                    if (stringExpression)
                    {
                        _lexer->inStringExpression = true;
                    }

                    LEXED_WITHOUT_POSITION_INCREMENT(tSTRING, yylval.string);
                }
                else if (CC == '#')
                {
                    dcString_appendCharacter(_lexer->stringString, '#');
                    _lexer->lexPosition++;
                    *_again = true;
                }
            }
        }
    }

    return retval;
}

void dcLexer_parseError(dcLexer *_lexer)
{
    dcLexer_throwExceptionWithString(_lexer, "");
}

void dcLexer_parseErrorWithCharacter(dcLexer *_lexer, char _error)
{
    const char error[] = {_error, 0};
    dcLexer_parseErrorWithString(_lexer, error);
}

void dcLexer_parseErrorWithString(dcLexer *_lexer, const char *_error)
{
    dcLexer_setErrorString(_lexer, _error);
    dcLexer_setErrorState(_lexer, true);
    resetTokens(_lexer);
}

void dcLexer_throwExceptionWithString(dcLexer *_lexer, const char *_error)
{
    resetTokens(_lexer);
    dcLexer_eatLine(_lexer);
    // :(
    yyerror((char *)_error);
}

char *dcLexer_lexString(const char *_input)
{
    size_t i = 0;
    size_t length = strlen(_input);
    char *output = (char *)dcMemory_allocateAndInitialize(length + 1);
    size_t j = 0;
    bool alreadyDidIt = false;

    for (i = 0; i < length; i++)
    {
        if (_input[i] != '\\' || alreadyDidIt)
        {
            output[j] = _input[i];
            j++;
            alreadyDidIt = false;
        }
        else
        {
            alreadyDidIt = true;
        }
    }

    return output;
}

size_t dcLexer_eatWhitespace(dcLexer *_lexer)
{
    size_t lineIt = 0;
    size_t returnsEaten = 0;

    while (! ATEND
           && (CC == ';'
               || CC == '\n'
               || CC == ' '
               || CC == '\t'))
    {
        lineIt++;

        if (CC == '\n')
        {
            dcLexer_incrementLineNumber(_lexer);
            returnsEaten++;
        }

        _lexer->lexPosition++;
    }

    _lexer->lexStartPosition += lineIt;
    return returnsEaten;
}

void dcLexer_eatLine(dcLexer *_lexer)
{
    size_t lineIt = 0;

    while (! ATEND && CC != '\n')
    {
        lineIt++;
        _lexer->lexPosition++;
    }

    _lexer->lexStartPosition += lineIt;
    dcLexer_eatWhitespace(_lexer);
}

size_t dcLexer_eatSpacesAndTabs(dcLexer *_lexer)
{
    size_t spacesEaten = 0;

    while (! ATEND && (CC == ' ' || CC == '\t'))
    {
        spacesEaten++;
        _lexer->lexPosition++;
    }

    _lexer->lexStartPosition += spacesEaten;
    return spacesEaten;
}

bool dcLexer_isNextText(const dcLexer *_lexer, const char *_text)
{
    size_t textIt = 0;
    size_t tempLexPosition = _lexer->lexPosition + 1;
    bool retval = true;
    const size_t textLength = strlen(_text);

    while (tempLexPosition < _lexer->inputLength &&
           WHITESPACE(_lexer->input[tempLexPosition]))
    {
        tempLexPosition++;
    }

    if (tempLexPosition >= _lexer->inputLength)
    {
        retval = false;
    }
    else
    {
        while (tempLexPosition < _lexer->inputLength &&
               textIt < textLength)
        {
            if (_lexer->input[tempLexPosition] != _text[textIt])
            {
                retval = false;
                break;
            }

            tempLexPosition++;
            textIt++;
        }
    }

    return retval;
}

static void addPart(const char *_string,
                    size_t _partStart,
                    size_t _partEnd,
                    dcList *_result)
{
    if (_partEnd > _partStart)
    {
        size_t partLength = (_partEnd - _partStart) + 1;
        char *part = (char *)dcMemory_allocate(partLength);
        memcpy(part, _string + _partStart, partLength - 1);
        // null terminate
        part[partLength - 1] = 0;
        dcList_push(_result,
                    dcString_createNodeWithString(part, false));
    }
}

dcList *dcLexer_splitString(const char *_string, char _token)
{
    size_t stringLength = strlen(_string);
    dcList *result = dcList_create();

    if (stringLength == 0)
    {
        return result;
    }

    size_t i = 0;
    size_t partStart = 0;
    size_t partEnd = stringLength;
    bool gotPart = false;

    for (i = 0; i < stringLength; i++)
    {
        if (_string[i] == _token)
        {
            if (! gotPart)
            {
                partEnd = i;
            }

            gotPart = true;
            continue;
        }

        if (gotPart)
        {
            if (partEnd > partStart)
            {
                addPart(_string, partStart, partEnd, result);
            }

            partStart = i;
            gotPart = false;
        }

        if (i == stringLength - 1)
        {
            partEnd = stringLength;
        }
    }

    addPart(_string, partStart, partEnd, result);
    return result;
}

dcString *dcLexer_escapeString(const char *_string)
{
    size_t length = strlen(_string);
    dcString *retval = dcString_createWithLength(length + 1);
    size_t i;

    for (i = 0; i < length; i++)
    {
        if (_string[i] == '#' || _string[i] == '"')
        {
            dcString_appendCharacter(retval, '\\');
        }

        dcString_appendCharacter(retval, _string[i]);
    }

    return retval;
}

bool dcLexer_hasParseError(dcLexer *_lexer)
{
    return _lexer->parseError;
}

char *dcLexer_sprintf(const char *_format, ...)
{
    va_list arguments;
    va_start(arguments, _format);
    return dcLexer_sprintfWithVaList(_format, arguments, NULL);
}

//
// utility functions
//
char *dcLexer_sprintfWithVaList(const char *_format,
                                va_list _arguments,
                                size_t *_length)
{
    //
    // run vsnprintf twice:
    // the first time it determines the length of the string
    // the second time it prints the output to the string
    //
    va_list argumentsCopy;

#ifdef TAFFY_WINDOWS
    // this makes me sad
    argumentsCopy = _arguments;
#else
    va_copy(argumentsCopy, _arguments);
#endif

    size_t length = vsnprintf(NULL, 0, _format, _arguments) + 1;

    if (_length != NULL)
    {
        *_length = length;
    }

    char *buffer = (char *)dcMemory_allocateAndInitialize(length);

    vsnprintf(buffer,
              length,
              _format,
              argumentsCopy);
    va_end(_arguments);
    va_end(argumentsCopy);

    return buffer;
}

void dcLexer_yyerror(dcLexer *_lexer, const char *_format, ...)
{
    va_list argumentPointer;
    va_start(argumentPointer, _format);
    dcLexer_setErrorString
        (_lexer, dcLexer_sprintfWithVaList(_format, argumentPointer, NULL));
    yyerror(NULL);
}

bool dcLexer_isWhitespace(char _character)
{
    return (WHITESPACE(_character));
}

char ccHelper(dcLexer *_lexer)
{
    return CC;
}

char ccNextHelper(dcLexer *_lexer)
{
    return CCNEXT;
}
