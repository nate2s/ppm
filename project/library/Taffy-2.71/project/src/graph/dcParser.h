#ifndef __DC_PARSER_INCLUDES_H__
#define __DC_PARSER_INCLUDES_H__

#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include "dcDefines.h"

void dcParser_initialize(void);
void dcParser_deinitialize(void);

struct dcNode_t *dcParser_parseString(const char *_string,
                                      const char *_fileName,
                                      bool _handleParseError);

// the main parser entry point
struct dcNode_t *dcParser_parse
    (struct dcLexer_t *_lexer,
     bool _handleParseError,
     dcPieLineEvaluatorOutFlag *_outFlags); // nullable

// the lexer uses this to get itself
struct dcLexer_t *dcParser_getLexer(void);

struct dcNode_t *dcParser_createParenthesesOperatorFunctionCall
    (struct dcNode_t *_identifier,
     struct dcArray_t *_arguments);

struct dcNode_t *dcParser_synchronizedParse
    (struct dcLexer_t *_lexer,
     bool _handleParseError,
     dcPieLineEvaluatorOutFlag *_outFlags); // nullable

void dcParser_lock(void);
void dcParser_unlock(void);

// the lexer sets this when a comment is read
void dcParser_setGotComment();

struct dcNode_t *dcParser_createFunctionFromGuts(struct dcNode_t *_arithmetic);

#endif
/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_SRC_GRAPH_DCPARSER_H_INCLUDED
# define YY_YY_SRC_GRAPH_DCPARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    kMETHOD_ATTRIBUTE_BREAKTHROUGH = 258,
    kMETHOD_ATTRIBUTE_CONST = 259,
    kMETHOD_ATTRIBUTE_CONTAINER_LOOP = 260,
    kEQUATION = 261,
    kMETHOD_ATTRIBUTE_MODIFIES_CONTAINER = 262,
    kMETHOD_ATTRIBUTE_OPERATOR = 263,
    kMETHOD_ATTRIBUTE_PREFIX_OPERATOR = 264,
    kMETHOD_ATTRIBUTE_SYNCHRONIZED = 265,
    kMETHOD_ATTRIBUTE_SYNCHRONIZED_READ = 266,
    kMETHOD_ATTRIBUTE_SYNCHRONIZED_WRITE = 267,
    kAUTOMATIC_FUNCTION = 268,
    kABSTRACT = 269,
    kAND = 270,
    kATOMIC = 271,
    kBREAK = 272,
    kCATCH = 273,
    kCLASS = 274,
    kCONST = 275,
    kELSE = 276,
    kFALSE = 277,
    kFINAL = 278,
    kFOR = 279,
    kGLOBAL = 280,
    kI = 281,
    kIF = 282,
    kIMPORT = 283,
    kIN = 284,
    kLOCAL = 285,
    kNEW = 286,
    kNIL = 287,
    kNO = 288,
    kOR = 289,
    kPACKAGE = 290,
    kPROTECTED = 291,
    kPUBLIC = 292,
    kRETURN = 293,
    kR = 294,
    kRW = 295,
    kSELF = 296,
    kSINGLETON = 297,
    kSLICE = 298,
    kSUPER = 299,
    kSYNCHRONIZED = 300,
    kTHROW = 301,
    kTRUE = 302,
    kTRY = 303,
    kUP_SELF = 304,
    kW = 305,
    kWHILE = 306,
    kYES = 307,
    tBIT_AND_EQUAL = 308,
    tBIT_OR_EQUAL = 309,
    tDIVIDE_EQUAL = 310,
    tDOT = 311,
    tEXPR_END = 312,
    tEQUAL_EQUAL = 313,
    tGREATER_THAN_OR_EQUAL = 314,
    tINSTANCE_SCOPED_IDENTIFIER = 315,
    tBLOCK_START = 316,
    tBLOCK_END = 317,
    tLEFT_BRACE_LESS_THAN = 318,
    tLEFT_SHIFT = 319,
    tLEFT_SHIFT_EQUAL = 320,
    tLESS_THAN_OR_EQUAL = 321,
    tMATRIX_DELIMITER = 322,
    tMETA_SCOPED_IDENTIFIER = 323,
    tMETHOD_INSTANCE = 324,
    tMETHOD_META = 325,
    tMETHOD_PARAMETER = 326,
    tMINUS_EQUAL = 327,
    tMINUS_MINUS = 328,
    tMODULUS_EQUAL = 329,
    tMULTIPLY_EQUAL = 330,
    tNOT_EQUAL = 331,
    tNUMBER = 332,
    tCOMPLEX_NUMBER = 333,
    tPLUS_EQUAL = 334,
    tPLUS_PLUS = 335,
    tPOWER_EQUAL = 336,
    tRIGHT_ARROW = 337,
    tRIGHT_BRACKET_EQUAL = 338,
    tRIGHT_SHIFT = 339,
    tRIGHT_SHIFT_EQUAL = 340,
    tRIGHT_PAREN_EQUAL = 341,
    tSTRING = 342,
    tSTRING_EXPRESSION_START = 343,
    tSYMBOL = 344,
    tTILDE_EQUAL = 345,
    tTILDE_EQUAL_LESS_THAN = 346,
    tVERBATIM_TEXT_START = 347,
    tWORD = 348,
    tBIT_XOR = 349,
    tBIT_XOR_EQUAL = 350
  };
#endif
/* Tokens.  */
#define kMETHOD_ATTRIBUTE_BREAKTHROUGH 258
#define kMETHOD_ATTRIBUTE_CONST 259
#define kMETHOD_ATTRIBUTE_CONTAINER_LOOP 260
#define kEQUATION 261
#define kMETHOD_ATTRIBUTE_MODIFIES_CONTAINER 262
#define kMETHOD_ATTRIBUTE_OPERATOR 263
#define kMETHOD_ATTRIBUTE_PREFIX_OPERATOR 264
#define kMETHOD_ATTRIBUTE_SYNCHRONIZED 265
#define kMETHOD_ATTRIBUTE_SYNCHRONIZED_READ 266
#define kMETHOD_ATTRIBUTE_SYNCHRONIZED_WRITE 267
#define kAUTOMATIC_FUNCTION 268
#define kABSTRACT 269
#define kAND 270
#define kATOMIC 271
#define kBREAK 272
#define kCATCH 273
#define kCLASS 274
#define kCONST 275
#define kELSE 276
#define kFALSE 277
#define kFINAL 278
#define kFOR 279
#define kGLOBAL 280
#define kI 281
#define kIF 282
#define kIMPORT 283
#define kIN 284
#define kLOCAL 285
#define kNEW 286
#define kNIL 287
#define kNO 288
#define kOR 289
#define kPACKAGE 290
#define kPROTECTED 291
#define kPUBLIC 292
#define kRETURN 293
#define kR 294
#define kRW 295
#define kSELF 296
#define kSINGLETON 297
#define kSLICE 298
#define kSUPER 299
#define kSYNCHRONIZED 300
#define kTHROW 301
#define kTRUE 302
#define kTRY 303
#define kUP_SELF 304
#define kW 305
#define kWHILE 306
#define kYES 307
#define tBIT_AND_EQUAL 308
#define tBIT_OR_EQUAL 309
#define tDIVIDE_EQUAL 310
#define tDOT 311
#define tEXPR_END 312
#define tEQUAL_EQUAL 313
#define tGREATER_THAN_OR_EQUAL 314
#define tINSTANCE_SCOPED_IDENTIFIER 315
#define tBLOCK_START 316
#define tBLOCK_END 317
#define tLEFT_BRACE_LESS_THAN 318
#define tLEFT_SHIFT 319
#define tLEFT_SHIFT_EQUAL 320
#define tLESS_THAN_OR_EQUAL 321
#define tMATRIX_DELIMITER 322
#define tMETA_SCOPED_IDENTIFIER 323
#define tMETHOD_INSTANCE 324
#define tMETHOD_META 325
#define tMETHOD_PARAMETER 326
#define tMINUS_EQUAL 327
#define tMINUS_MINUS 328
#define tMODULUS_EQUAL 329
#define tMULTIPLY_EQUAL 330
#define tNOT_EQUAL 331
#define tNUMBER 332
#define tCOMPLEX_NUMBER 333
#define tPLUS_EQUAL 334
#define tPLUS_PLUS 335
#define tPOWER_EQUAL 336
#define tRIGHT_ARROW 337
#define tRIGHT_BRACKET_EQUAL 338
#define tRIGHT_SHIFT 339
#define tRIGHT_SHIFT_EQUAL 340
#define tRIGHT_PAREN_EQUAL 341
#define tSTRING 342
#define tSTRING_EXPRESSION_START 343
#define tSYMBOL 344
#define tTILDE_EQUAL 345
#define tTILDE_EQUAL_LESS_THAN 346
#define tVERBATIM_TEXT_START 347
#define tWORD 348
#define tBIT_XOR 349
#define tBIT_XOR_EQUAL 350

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 175 "src/graph/dcParser.y" /* yacc.c:1909  */

    int iValue;
    char *string;
    char *heapString;
    const char *constString;
    struct dcNode_t *node;
    struct dcList_t *list;
    struct dcPair_t *pair;
    struct dcScopeData_t *scopeData;
    struct dcMethodHeader_t *methodHeader;
    struct dcNumber_t *number;
    struct dcComplexNumber_t *complexNumber;
    struct MyClassHeader_t *classHeader;
    struct dcClassTemplate_t *classTemplate;
    struct dcString_t *taffyString;

#line 261 "src/graph/dcParser.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_GRAPH_DCPARSER_H_INCLUDED  */
