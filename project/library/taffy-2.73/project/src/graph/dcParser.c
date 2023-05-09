/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 19 "src/graph/dcParser.y" /* yacc.c:339  */

#define YYDEBUG 1

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "dcDefines.h"

#include "dcCallStackData.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcComplexNumber.h"
#include "dcContainers.h"
#include "dcClass.h"
#include "dcError.h"
#include "dcGarbageCollector.h"
#include "dcLexer.h"
#include "dcLog.h"
#include "dcMatrix.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcParser.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcStringManager.h"
#include "dcSystem.h"
#include "dcThread.h"

// classes //
#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcComplexNumberClass.h"
#include "dcEquationClass.h"
#include "dcExceptions.h"
#include "dcFunctionClass.h"
#include "dcFutureClass.h"
#include "dcHashClass.h"
#include "dcMainClass.h"
#include "dcMatrixClass.h"
#include "dcNilClass.h"
#include "dcNumberClass.h"
#include "dcObjectClass.h"
#include "dcPairClass.h"
#include "dcParseErrorExceptionClass.h"
#include "dcProcedureClass.h"
#include "dcStringClass.h"
#include "dcYesClass.h"

// graph data //
#include "dcGraphDatas.h"

// set yydebug 1 via the "--log parser" argument

#ifdef ENABLE_DEBUG
#  undef YYDEBUG
#  define YYDEBUG 1
#endif

extern int yylex(void);
extern void yyerror(char* mesg);

static dcNode *sParseHead = NULL;
static dcLexer *sLexer = NULL;
static char *sPackageName = NULL;
static dcList *sClassNames = NULL;
static dcMutex *sMutex = NULL;
static dcPieLineEvaluatorOutFlag *sOutFlags = NULL;
static bool sGotComment = false;

void dcParser_extractAndClearMethodParameterListData(dcList *_methodInfo,
                                                     dcString *_methodName,
                                                     dcList *_methodArguments);
static dcNode *createFunction(dcNode *_identifier,
                              dcScopeDataFlags _flags,
                              dcList *_arguments,
                              dcNode *_arithmetic);

static void pushClassName(const char *_name);
static dcNode *createFunctionCall(dcNode *_receiver, dcList *_arguments);

dcNode *dcParser_parse(dcLexer *_lexer,
                       bool _handleParseError,
                       dcPieLineEvaluatorOutFlag *_outFlags);

//
// MyClassHeader
//

struct MyClassHeader_t
{
    char *className;
    char *superName;
    dcScopeDataFlags scopeDataFlags;
};

typedef struct MyClassHeader_t MyClassHeader;

static MyClassHeader *createClassHeader(char *_className,
                                        char *_superName,
                                        dcScopeDataFlags _scopeDataFlags);


#line 175 "src/graph/dcParser.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "dcParser.h".  */
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
#line 175 "src/graph/dcParser.y" /* yacc.c:355  */

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

#line 422 "src/graph/dcParser.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_GRAPH_DCPARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 437 "src/graph/dcParser.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  219
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4484

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  119
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  117
/* YYNRULES -- Number of rules.  */
#define YYNRULES  368
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  601

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   350

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   107,   118,     2,     2,   106,   101,     2,
     110,   113,   104,   102,    97,   103,     2,   105,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   112,     2,
      98,    96,    99,   111,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   116,     2,   117,   109,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   114,   100,   115,   108,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   260,   260,   270,   272,   273,   274,   279,   280,   281,
     282,   283,   288,   289,   290,   295,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   310,   318,
     320,   321,   322,   323,   324,   325,   326,   327,   329,   329,
     331,   338,   339,   340,   341,   342,   344,   349,   357,   358,
     359,   360,   361,   362,   363,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   375,   380,   385,   389,   394,   399,
     404,   410,   416,   420,   426,   431,   435,   440,   441,   442,
     443,   449,   454,   458,   490,   496,   500,   504,   508,   512,
     516,   520,   524,   528,   532,   536,   541,   546,   551,   556,
     561,   566,   571,   576,   581,   586,   591,   596,   601,   607,
     612,   617,   622,   627,   636,   641,   660,   677,   683,   687,
     691,   695,   699,   703,   707,   711,   716,   718,   722,   726,
     731,   736,   741,   746,   751,   755,   759,   763,   767,   771,
     775,   783,   791,   807,   816,   830,   831,   832,   833,   834,
     835,   837,   838,   839,   841,   843,   847,   852,   856,   861,
     865,   870,   875,   879,   883,   887,   892,   897,   908,   920,
     931,   943,   947,   952,   954,   954,   956,   956,   958,   962,
     966,   970,   975,   984,   995,   999,  1004,  1008,  1013,  1017,
    1021,  1028,  1036,  1051,  1058,  1062,  1067,  1073,  1077,  1088,
    1088,  1090,  1096,  1101,  1106,  1111,  1116,  1121,  1127,  1147,
    1155,  1163,  1168,  1173,  1178,  1183,  1188,  1192,  1197,  1202,
    1209,  1214,  1219,  1224,  1229,  1234,  1238,  1243,  1243,  1245,
    1250,  1254,  1259,  1266,  1271,  1275,  1281,  1284,  1289,  1293,
    1297,  1301,  1306,  1310,  1315,  1321,  1327,  1333,  1349,  1351,
    1359,  1366,  1375,  1376,  1381,  1385,  1389,  1393,  1398,  1400,
    1401,  1402,  1403,  1404,  1406,  1410,  1415,  1415,  1415,  1415,
    1417,  1438,  1465,  1469,  1489,  1509,  1522,  1535,  1553,  1557,
    1561,  1566,  1570,  1575,  1579,  1583,  1587,  1591,  1595,  1599,
    1603,  1607,  1611,  1615,  1619,  1623,  1627,  1631,  1635,  1639,
    1643,  1647,  1651,  1655,  1659,  1663,  1667,  1671,  1675,  1679,
    1683,  1687,  1691,  1695,  1699,  1703,  1708,  1713,  1717,  1722,
    1727,  1735,  1736,  1736,  1738,  1751,  1829,  1833,  1837,  1842,
    1846,  1848,  1852,  1856,  1860,  1866,  1880,  1889,  1891,  1896,
    1901,  1906,  1911,  1915,  1920,  1929,  1938,  1949,  1958,  1969,
    1974,  1979,  1985,  1992,  2001,  2006,  2012,  2013,  2018,  2022,
    2026,  2030,  2034,  2038,  2042,  2047,  2051,  2053,  2062
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "kMETHOD_ATTRIBUTE_BREAKTHROUGH",
  "kMETHOD_ATTRIBUTE_CONST", "kMETHOD_ATTRIBUTE_CONTAINER_LOOP",
  "kEQUATION", "kMETHOD_ATTRIBUTE_MODIFIES_CONTAINER",
  "kMETHOD_ATTRIBUTE_OPERATOR", "kMETHOD_ATTRIBUTE_PREFIX_OPERATOR",
  "kMETHOD_ATTRIBUTE_SYNCHRONIZED", "kMETHOD_ATTRIBUTE_SYNCHRONIZED_READ",
  "kMETHOD_ATTRIBUTE_SYNCHRONIZED_WRITE", "kAUTOMATIC_FUNCTION",
  "kABSTRACT", "kAND", "kATOMIC", "kBREAK", "kCATCH", "kCLASS", "kCONST",
  "kELSE", "kFALSE", "kFINAL", "kFOR", "kGLOBAL", "kI", "kIF", "kIMPORT",
  "kIN", "kLOCAL", "kNEW", "kNIL", "kNO", "kOR", "kPACKAGE", "kPROTECTED",
  "kPUBLIC", "kRETURN", "kR", "kRW", "kSELF", "kSINGLETON", "kSLICE",
  "kSUPER", "kSYNCHRONIZED", "kTHROW", "kTRUE", "kTRY", "kUP_SELF", "kW",
  "kWHILE", "kYES", "tBIT_AND_EQUAL", "tBIT_OR_EQUAL", "tDIVIDE_EQUAL",
  "tDOT", "tEXPR_END", "tEQUAL_EQUAL", "tGREATER_THAN_OR_EQUAL",
  "tINSTANCE_SCOPED_IDENTIFIER", "tBLOCK_START", "tBLOCK_END",
  "tLEFT_BRACE_LESS_THAN", "tLEFT_SHIFT", "tLEFT_SHIFT_EQUAL",
  "tLESS_THAN_OR_EQUAL", "tMATRIX_DELIMITER", "tMETA_SCOPED_IDENTIFIER",
  "tMETHOD_INSTANCE", "tMETHOD_META", "tMETHOD_PARAMETER", "tMINUS_EQUAL",
  "tMINUS_MINUS", "tMODULUS_EQUAL", "tMULTIPLY_EQUAL", "tNOT_EQUAL",
  "tNUMBER", "tCOMPLEX_NUMBER", "tPLUS_EQUAL", "tPLUS_PLUS",
  "tPOWER_EQUAL", "tRIGHT_ARROW", "tRIGHT_BRACKET_EQUAL", "tRIGHT_SHIFT",
  "tRIGHT_SHIFT_EQUAL", "tRIGHT_PAREN_EQUAL", "tSTRING",
  "tSTRING_EXPRESSION_START", "tSYMBOL", "tTILDE_EQUAL",
  "tTILDE_EQUAL_LESS_THAN", "tVERBATIM_TEXT_START", "tWORD", "tBIT_XOR",
  "tBIT_XOR_EQUAL", "'='", "','", "'<'", "'>'", "'|'", "'&'", "'+'", "'-'",
  "'*'", "'/'", "'%'", "'!'", "'~'", "'^'", "'('", "'?'", "':'", "')'",
  "'{'", "'}'", "'['", "']'", "'\"'", "$accept", "start", "statement",
  "statementP", "statement_element", "expressions", "expression",
  "expressionP", "in", "expression_rhs", "expression_rhsP",
  "expression_rhs_with_in", "question_colon", "function_rhs_guts",
  "function_rhs", "object_identifier", "object_identifierP", "object",
  "equation", "nil", "identifier", "identifierP", "identifier_dot_list",
  "real_identifier_dot_list", "identifier_chain", "identifier_chain_rest",
  "arithmetic", "prefix_arithmetic", "real_arithmetic",
  "assignment_arithmetic", "boolean", "assignment", "assignmentP", "self",
  "upSelf", "super", "new", "const_global_flags", "function",
  "function_arithmetic", "method_calls", "if", "ifPrime", "if_guts",
  "else", "synchronized", "synchronized_guts", "for", "forP", "for_end",
  "while", "expr_end_left_brace", "while_condition", "whileP", "block",
  "block_head", "identifier_list", "number", "hash", "hash_guts",
  "hash_objects", "expr_ends", "symbol", "function_arithmetic_list",
  "expression_list", "matrix_guts", "matrix", "array", "array_rest",
  "string", "string_contents", "string_expression", "keyword", "break",
  "return", "package", "import", "class_path", "class_path_star",
  "tryBlock", "catches", "throw", "method_call", "automatic_function",
  "function_call_guts", "function_call", "method_call_receiver",
  "method_call_receiverP", "expression_rhs_list",
  "indexed_method_call_receiver", "indexed_method_call",
  "bracketed_method_call", "method_header", "operator", "prefix_operator",
  "argument_operator", "method_parameter_list", "method_parameter_keyword",
  "method_parameter_const_keyword", "method_parameter", "class",
  "class_keywords", "real_class_keywords", "real_real_class_keywords",
  "class_header", "class_header_end", "class_data", "scope_data_class",
  "instance_object", "metaclass_object", "class_object_accessor", "method",
  "class_method", "method_head", "method_attribute", "method_attributes",
  "method_headP", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,    61,    44,    60,    62,
     124,    38,    43,    45,    42,    47,    37,    33,   126,    94,
      40,    63,    58,    41,   123,   125,    91,    93,    34
};
# endif

#define YYPACT_NINF -404

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-404)))

#define YYTABLE_NINF -270

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2622,   -77,  3794,  -404,  -404,  -404,  -404,     7,  -404,  -404,
     -65,    42,  -404,   -24,   -19,    57,   -20,  -404,  -404,   -19,
    3862,  -404,  -404,  -404,  -404,    23,  3862,  -404,   -34,  -404,
      26,  -404,  -404,  -404,  -404,   -20,  3522,  -404,   440,   440,
    3862,    88,  -404,  -404,    53,    96,  3862,  3862,  3862,  3862,
    3386,  3590,  3208,    20,   173,  -404,  -404,   121,  -404,  -404,
    -404,  4115,  -404,  -404,   437,   394,  -404,  -404,  -404,  -404,
     699,  -404,  -404,   125,   448,  -404,  -404,  -404,  -404,  -404,
    -404,    -4,    64,    92,  -404,     8,  -404,  -404,  -404,  -404,
    -404,  -404,  -404,  -404,  -404,  1460,  -404,  -404,  -404,  -404,
    -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,
    -404,   510,   543,  -404,    74,   578,  -404,  -404,    99,   176,
     149,  -404,    83,   778,  3862,  3386,  4323,  -404,  -404,   806,
     122,   131,  -404,  -404,  -404,  2722,  -404,  3862,  -404,   143,
    -404,  -404,  -404,   144,  -404,  4323,  -404,   187,   746,  -404,
    -404,   -20,  4323,   103,  -404,  1668,  3862,  -404,   -40,  3862,
    3208,   606,   185,  1035,  -404,    90,   124,  -404,   168,   159,
    -404,   272,   361,   251,   117,   120,  -404,  -404,  -404,  3862,
    -404,  -404,  -404,  -404,  -404,   543,  -404,   127,  -404,  -404,
     320,   320,   127,   127,  3658,  -404,   118,  1507,   128,   181,
     130,   839,    39,   134,  -404,   301,   181,  3726,  -404,  4056,
    3726,   123,   126,   543,    61,  2822,  -404,   140,    61,  -404,
    2622,  3862,  3862,  3862,  3862,  3862,  3862,  3862,  -404,  3862,
    -404,  3862,  3862,   186,  3862,  3862,  3862,  3862,  3862,  3862,
    3862,  3862,  3862,  3862,  -404,  3862,   151,  3862,  3862,  3862,
    3862,  3862,  3862,  3862,  3862,  3862,  3862,  3862,  3590,  3590,
     183,  3590,  3590,  3590,   155,   -55,  -404,   163,   224,  -404,
    3862,   -35,   588,  -404,  -404,  1774,  -404,  -404,  -404,  -404,
    -404,  -404,  -404,   191,  -404,  4167,  1507,   -44,  3590,   195,
    3140,   181,   170,   -11,   200,   188,  -404,     9,   180,   885,
     196,   197,   198,   -20,  -404,  1241,  3522,  3522,  -404,  1153,
     -43,  4323,  -404,  4219,  -404,  -404,  3862,  3862,  -404,  -404,
    -404,  -404,  -404,  -404,  3454,   199,   205,  -404,  -404,   206,
    -404,  -404,   207,  -404,  -404,  -404,  4375,  4375,   153,  1562,
    1348,   744,  1348,   953,   744,   953,   213,   614,  1348,  1348,
    1032,   654,   320,   320,   -10,   -10,   -10,   127,  3297,  -404,
     953,   953,  1562,   744,  1562,   953,  1562,  1562,   953,   744,
     614,  4323,  -404,  4323,  -404,   202,     1,   257,   212,   229,
     214,   220,  3862,  3862,  3862,  -404,  3140,   -36,   -19,  -404,
    -404,   588,   238,   239,  -404,  -404,  -404,   588,   588,   588,
     588,   226,  -404,   223,   778,  3862,  3522,  3862,  3140,   181,
    3140,   -34,  -404,  -404,   -26,   236,   329,  -404,    10,   -34,
     -34,  -404,   -20,  -404,  -404,  -404,  -404,  -404,  -404,  -404,
    -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,
    -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,
    -404,  -404,  -404,  -404,  -404,  -404,   237,   -39,   240,   241,
    -404,  -404,   242,  4271,  3997,  3726,  -404,  3726,  -404,  -404,
    -404,  3862,  -404,  3522,  -404,  -404,  -404,    80,  4323,    89,
    -404,  3862,  -404,   -17,  -404,   172,   172,  -404,  -404,  -404,
    -404,  1880,  -404,  -404,  3883,  -404,  4323,  -404,  2928,   181,
     328,  1986,  2092,  -404,   244,   -20,  -404,   329,  -404,  2198,
    2622,   -52,  -404,  -404,   295,  -404,  -404,  -404,   248,  -404,
    3658,  3862,  -404,  -404,  1348,  -404,  3522,  3522,  4323,   -34,
    -404,  -404,  -404,  -404,  -404,  -404,   247,  -404,   -34,   243,
    3034,    -1,  -404,  -404,   252,  -404,   256,  2304,   -20,  -404,
    -404,   262,   263,   -20,  -404,   296,   270,  -404,  3945,  -404,
    -404,  -404,  -404,  2410,   -34,   -34,   268,  -404,  -404,  -404,
    -404,  -404,   267,   275,  -404,  -404,  -404,   -45,  -404,  -404,
    -404,   276,  -404,  2410,  2410,   -34,  -404,   -34,  -404,  -404,
    -404,  -404,  -404,  2410,  2516,  -404,   329,   277,  -404,   329,
    -404
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,   331,   332,   229,   326,   134,   122,   333,
       0,   135,   190,     0,     0,   136,     0,    65,   123,     0,
     230,   130,   334,   328,   132,     0,     0,   120,     0,   131,
       0,   121,    11,    68,   185,     0,     0,    69,     0,     0,
       0,   188,   189,   201,     0,    70,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,     3,     5,     8,    15,
      27,    18,    29,    36,    37,    33,    47,    49,    63,    60,
      48,    66,    67,     0,    35,    78,    77,    30,    79,    17,
     126,    51,    54,    52,    50,     0,    16,    19,   154,    22,
      26,   166,    25,   173,    57,     0,    61,    58,    53,    56,
      55,    62,    20,   228,   227,    10,     9,    24,    23,    21,
     248,    32,     0,   258,     0,    34,    31,     7,     0,     0,
     330,    59,     0,   356,     0,     0,     0,    37,    33,    48,
      35,    32,    34,   137,   139,     0,   138,     0,   235,   236,
     233,   140,   133,   232,    39,    38,   231,    33,    48,    35,
      32,     0,   246,     0,   175,     0,     0,   187,     0,     0,
       0,    33,    49,    48,   147,    35,   203,   146,   207,     0,
     149,    32,    34,    31,     0,     0,   320,   323,   322,     0,
     321,   367,   274,   318,   319,   317,   368,    82,   191,   220,
      81,    83,   124,    84,     0,   193,     0,     0,     0,   194,
       0,    48,    35,    61,    43,    32,   200,     0,   211,   216,
       0,     0,     0,    31,   223,     0,   219,     0,   224,     1,
       4,     0,     0,     0,     0,     0,     0,     0,   108,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   183,     0,    12,   247,
       0,     0,     0,   327,   329,     0,   358,   362,   363,   364,
     359,   360,   361,   366,   357,     0,   265,     0,     0,   205,
       0,     0,     0,     0,     0,     0,   174,     0,     0,    48,
      79,     0,    31,     0,   184,    48,     0,     0,   208,     0,
       0,   324,   316,     0,   197,   125,     0,     0,   113,   192,
     195,    46,   199,   215,     0,   217,     0,   209,   272,     0,
     221,   226,     0,   218,   222,     6,   118,   119,    79,   114,
     112,    94,   110,   117,    95,   115,     0,    90,   109,   111,
      92,    93,    85,    86,    87,    88,    91,    89,     0,    28,
     106,   105,   101,    99,    97,   103,   100,    96,   102,    98,
     104,   127,   128,   265,   253,     0,   252,    71,     0,   252,
       0,     0,     0,     0,     0,   182,    13,     0,     0,   337,
     336,     0,   345,   347,   343,   344,   325,     0,     0,     0,
       0,     0,   353,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   234,   237,     0,     0,     0,   239,     0,     0,
       0,   186,     0,   202,   206,   305,   307,   312,   283,   291,
     297,   314,   289,   310,   280,   303,   311,   309,   279,   313,
     298,   315,   296,   300,   301,   288,   290,   306,   304,   292,
     293,   294,   295,   302,   278,   299,     0,     0,     0,     0,
     282,   281,     0,   198,     0,     0,   212,     0,   210,   273,
     225,     0,   254,     0,   255,   256,   257,     0,   129,     0,
      14,     0,   271,     0,   342,     0,     0,   341,   339,   338,
     340,     0,   352,   365,     0,   142,   264,   204,     0,     0,
     155,     0,     0,   161,     0,     0,   241,     0,   238,     0,
       0,     0,    74,   284,     0,   308,   285,   275,     0,   276,
       0,     0,   214,   213,   116,   141,     0,     0,   270,     0,
     349,   351,   350,   346,   348,   355,     0,    64,     0,     0,
       0,     0,   156,   158,     0,   164,     0,     0,     0,   240,
     179,     0,     0,     0,    75,     0,     0,   196,     0,   144,
     143,   335,   354,     0,     0,     0,     0,   160,   159,   157,
     162,   165,     0,     0,   178,   180,    76,     0,   277,    40,
     172,     0,   170,     0,     0,     0,   163,     0,   287,   286,
     171,   168,   169,     0,     0,   167,   244,     0,   245,   242,
     243
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -404,  -404,    12,  -404,  -404,    -3,   -89,  -404,     4,  1152,
    -404,   258,  -404,  -404,   577,    69,  -404,   -27,  -404,  -404,
       0,  -404,  -404,  -404,  -404,  -157,   182,  -404,  -404,  -404,
    -131,  -404,  -404,  -404,  -404,  -404,  -404,  -404,  -404,  -402,
    -404,  -404,  -140,  -138,  -404,  -404,  -404,  -404,  -404,  -370,
    -404,   -21,  -404,  -404,  -404,  -404,  -404,   360,  -404,  -404,
    -186,  -115,  -404,   106,  -398,   107,  -404,   169,  -196,  -404,
    -103,  -404,  -404,  -404,  -404,  -404,  -404,   -18,  -404,  -404,
    -403,  -404,    -2,   161,  -145,   480,  -404,  -404,  -120,  -404,
     777,   -33,   378,  -404,  -404,  -404,   -83,  -404,  -404,  -404,
    -254,  -404,   300,  -404,  -404,  -108,  -144,  -404,  -404,  -404,
     -56,  -404,  -404,  -229,  -404,    27,  -404
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    54,   581,    56,    57,   267,    58,    59,    60,    61,
      62,   146,    63,   200,   127,   128,    66,    67,    68,    69,
     148,    71,    72,    73,   164,   512,   149,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,   166,
     167,    87,    88,   500,   542,    89,   503,    90,    91,   582,
      92,   389,   301,    93,    94,    95,   158,    96,    97,   198,
     199,   210,    98,   168,   291,   169,    99,   100,   211,   101,
     217,   218,   102,   103,   104,   105,   106,   139,   140,   107,
     417,   108,   109,   110,   375,   150,   112,   113,   379,   114,
     132,   116,   181,   458,   462,   459,   182,   183,   184,   185,
     117,   118,   119,   120,   272,   390,   396,   397,   398,   399,
     533,   121,   400,   122,   283,   284,   123
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      70,   143,   129,   173,   495,   287,   268,   155,   314,   162,
     497,   323,    55,   506,   326,   508,   142,   514,   395,   213,
     290,   264,   153,   153,   144,   300,    13,   415,   415,   269,
     553,   206,   133,   124,   170,   157,   163,   134,   588,   294,
      33,   383,   406,   401,   515,   135,   289,   481,    37,   204,
     212,   201,  -261,   407,   196,   384,   153,   303,  -261,   304,
     460,   407,   136,   228,   461,   554,   416,   507,    33,    65,
     230,   525,   589,    45,   138,   388,    37,   141,   516,   154,
     154,   482,   412,  -261,   320,   265,   137,   473,   502,   147,
    -145,   322,   338,   413,   325,    70,   529,   244,   407,   245,
     539,    45,   312,  -261,   549,   161,   261,   214,   215,  -261,
    -261,   330,  -267,   154,   188,   334,   378,   380,   381,   147,
     161,   161,  -249,   302,   559,   560,   332,   213,   466,   196,
     329,  -251,  -261,   151,  -261,    70,   156,   395,   216,   376,
     189,   144,   566,   395,   395,   395,   395,  -145,   214,   215,
     387,   295,   -72,  -181,   -41,    70,   299,  -145,   212,   129,
     305,  -261,   401,     3,    65,     4,   526,   298,   401,   401,
     401,   401,     9,   219,   262,   527,   410,   407,   220,  -249,
    -268,   260,    74,  -261,   130,  -150,   407,  -145,  -251,  -249,
     270,    22,   271,   598,   147,   273,   600,   275,  -251,   293,
     294,   409,   263,  -145,    65,  -145,   147,  -145,  -269,   465,
    -181,   530,   531,   591,   592,    70,   246,   296,   165,  -249,
      70,   306,   532,   595,    65,   307,   308,   309,  -251,   161,
     310,   315,   335,   202,    74,  -249,   245,  -249,   206,  -249,
     327,   319,  -150,   328,  -251,   321,  -251,   484,  -251,   -44,
    -181,  -151,  -150,   487,   488,   489,   490,   374,   333,   374,
     374,   374,   477,   346,   479,   382,  -181,   358,  -181,   522,
    -181,   523,  -153,   173,   173,    70,   377,    74,   385,   162,
     162,   386,  -150,   411,    65,  -262,   374,   403,   404,    65,
      70,  -262,   408,   412,   498,   418,   467,   268,  -150,   504,
    -150,   414,  -150,   421,   170,   170,   163,   163,  -151,  -176,
     419,   420,   471,   -73,  -262,   472,  -262,    74,  -151,   289,
    -262,   499,   468,   469,   470,   474,   407,   475,   161,  -153,
     161,   161,   161,   476,   557,   485,   486,    74,   492,  -153,
     491,   130,    74,  -262,    65,  -262,   505,   415,  -151,   541,
     513,   555,   577,   517,   518,   519,   564,   161,   547,    65,
     556,  -152,   562,   578,  -151,  -262,  -151,   569,  -151,  -153,
     483,   570,  -262,   173,  -260,   161,   161,   574,   575,   162,
    -260,   585,   586,   480,   540,  -153,    70,  -153,   587,  -153,
     501,   590,   599,   228,  -262,   292,   576,    74,   509,   510,
     230,   567,    74,   568,   170,  -260,   163,  -259,    70,   289,
      70,   203,   423,  -259,   424,   359,   -45,   186,  -152,   372,
     274,   561,   511,   246,   241,   242,   243,   244,  -152,   245,
     534,   493,  -260,     0,     0,     0,     0,     0,  -259,     0,
     173,    74,     0,    74,    74,    74,   162,     0,   174,   175,
    -263,   289,     0,   176,  -260,    65,  -263,    74,  -152,   177,
       0,  -261,     0,     0,     0,  -259,     0,  -261,     0,     0,
      74,   170,    74,   163,  -152,   161,  -152,    65,  -152,    65,
     111,  -263,   131,     0,   178,     0,     0,  -259,   165,   165,
       0,    70,  -261,   173,   173,     0,     0,     0,    70,   162,
     162,    70,    70,   536,     0,   548,     0,     0,  -263,    70,
      70,   179,     0,   544,   546,     0,   171,   563,     0,  -261,
     501,   551,   552,  -262,   170,   170,   163,   163,     0,  -262,
    -263,   205,   111,   180,     0,     0,     0,     0,     0,     0,
      70,  -261,   161,   583,   584,     0,     0,    70,   573,     0,
       0,     0,     0,   511,  -262,     0,   176,     0,     0,   572,
      65,     0,   177,    70,   593,     0,   594,    65,    74,     0,
      65,    65,     0,     0,     0,   111,     0,    64,    65,    65,
       0,  -262,     0,    70,    70,     0,     0,   178,   165,     0,
      74,  -260,    74,    70,    70,   161,   161,  -260,     0,     0,
       0,     0,     3,  -262,     4,     0,   597,     6,     0,    65,
       0,     9,     0,    64,   179,   111,    65,     0,     0,  -259,
       0,     0,  -260,     0,     0,  -259,     0,     0,    64,    64,
      22,    23,    65,     0,     0,   111,   180,     0,     0,   131,
     111,     0,     0,     0,     0,   391,     0,     0,   392,  -260,
    -259,     0,    65,    65,     0,   165,   393,    38,    39,     0,
       0,     0,    65,    65,     0,     0,     0,     0,     0,     0,
       0,  -260,    64,    74,     0,     0,     0,  -259,   226,     0,
      74,     0,     0,    74,    74,     0,     0,   228,     0,     0,
       0,    74,    74,     0,   230,   111,     0,     0,   231,  -259,
     111,     0,     0,   394,     0,     0,     0,     0,   165,   165,
       0,     0,    64,     0,   237,   238,   239,   240,   241,   242,
     243,   244,    74,   245,     0,     0,     0,   228,     0,    74,
       0,     0,    64,     0,   230,     0,     0,    64,     0,   111,
       0,   111,   111,   111,     0,    74,     0,     0,     0,     0,
       0,     0,   247,   248,   249,   111,   239,   240,   241,   242,
     243,   244,     0,   245,   250,    74,    74,     0,   111,     0,
     111,   251,     0,   252,   253,    74,    74,   115,   254,     0,
     255,   276,   277,   278,   256,   279,   171,   171,   280,   281,
     282,     0,    64,     0,   257,   258,     0,    64,     0,   247,
     248,   249,     0,     0,     0,     0,  -250,     0,     0,   259,
       0,   250,     0,   172,     0,  -266,     0,   228,   251,     0,
     252,   253,     0,     0,   230,   254,     0,   255,   115,   115,
       0,   256,     0,     0,     0,     0,    64,     0,    64,    64,
      64,   257,     0,     0,   237,   238,   239,   240,   241,   242,
     243,   244,    64,   245,     0,     0,   288,     0,     0,   247,
     248,   249,  -266,  -250,     0,    64,   111,    64,     0,     0,
       0,   250,   115,  -250,     0,     0,     0,     0,   251,     0,
     252,   253,     0,    64,    64,   254,   171,   255,   111,     0,
     111,   256,   247,   248,   249,     0,     0,     0,     0,     0,
       0,   257,     0,  -250,   250,     0,     0,     0,     0,     0,
       0,   251,   115,   252,   253,     0,   288,     0,   254,  -250,
     255,  -250,  -266,  -250,   256,     0,     0,     0,     0,     0,
       0,     0,   115,     0,   257,     0,     0,   115,   247,   248,
     249,     0,     0,     0,     0,     0,     0,     0,     0,   288,
     250,     0,     0,   171,   -42,  -266,     0,   251,     0,   252,
     253,     0,     0,    64,   254,     0,   255,     0,     0,     0,
     256,   111,     0,     0,     0,     0,     0,     0,   111,     0,
     257,   111,   111,    64,     0,    64,     0,    64,     0,   111,
     111,     0,   115,     0,     0,   288,     0,   115,  -177,     0,
       0,  -266,     0,     0,     0,     0,   171,   171,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   226,     0,     0,
     111,     0,     0,     0,     0,     0,   228,   111,     0,     0,
       0,     0,     0,   230,     0,  -148,   115,   231,   115,   115,
     115,     0,     0,   111,     0,     0,     0,   234,     0,     0,
      64,     0,   115,   237,   238,   239,   240,   241,   242,   243,
     244,     0,   245,   111,   111,   115,     0,   115,    64,     0,
       0,     0,     0,   111,   111,    64,     0,     0,    64,    64,
       0,     0,     0,   172,   172,     0,    64,    64,   247,   248,
     249,     0,  -148,     0,     0,     0,     0,     0,     0,     0,
     250,     0,  -148,    64,    64,   228,     0,   251,     0,   252,
     253,     0,   230,     0,   254,     0,   255,    64,     0,     0,
     256,     0,     0,     0,    64,     0,     0,     0,     0,     0,
     257,     0,  -148,   238,   239,   240,   241,   242,   243,   244,
      64,   245,     0,     0,     0,   288,     0,     0,  -148,     0,
    -148,  -266,  -148,     0,   126,     0,     0,     0,     0,     0,
      64,    64,     0,   115,     0,     0,     0,     0,     0,     0,
      64,    64,   145,     0,     0,     0,     0,     0,   152,     0,
       0,     0,     0,   172,     0,   115,     0,   115,   126,     0,
       0,     0,   187,     0,     0,     0,     0,     0,   190,   191,
     192,   193,   197,   126,   209,     0,   425,   426,   427,     0,
       0,   428,   429,     0,     0,     0,     0,   430,   431,   432,
       0,     0,     0,     0,     0,   433,   434,   435,   436,     0,
       0,     0,   437,   438,   439,     0,     0,   440,   441,     0,
       0,     0,     0,   442,     0,     0,     0,   443,   444,     0,
     172,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,     0,   455,   456,     0,     0,     0,     0,   115,   457,
       0,     0,     0,     0,     0,   115,   285,   286,   115,   115,
       0,     0,     0,     0,     0,     0,   115,   115,     0,   145,
       0,     0,     0,     0,   247,   248,   249,     0,     0,     0,
       0,     0,     0,   172,   172,     0,   250,     0,   126,     0,
       0,   126,   209,   251,     0,   252,   253,   115,     0,     0,
     254,     0,   255,   422,   115,     0,   256,     0,     0,     0,
       0,   311,     0,     0,     0,     0,   257,     0,     0,     0,
     115,     0,     0,     0,     0,     0,   313,     0,     0,     0,
       0,   288,     0,     0,     0,     0,     0,  -266,     0,   209,
     115,   115,   209,     0,     0,     0,     0,     0,     0,     0,
     115,   115,     0,   336,   337,   126,   339,   340,   341,   342,
       0,   343,     0,   344,   345,     0,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,     0,   357,     0,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   373,   226,   373,   373,   373,     0,     0,     0,     0,
       0,   228,   373,     0,   229,     0,     0,     0,   230,     0,
       0,     0,   231,     0,     0,     0,     0,     0,   232,   233,
     373,     0,   234,     0,     0,     0,     0,     0,   237,   238,
     239,   240,   241,   242,   243,   244,     0,   245,   126,   126,
       0,     0,     0,     0,     0,     0,     1,     0,   463,   464,
       0,     0,     0,     2,     0,     0,   209,     5,     0,     0,
       7,     0,     8,     0,    10,    11,    12,    13,     0,     0,
      15,    16,    17,    18,     0,     0,     0,     0,    20,     0,
       0,    21,     0,     0,    24,    25,    26,    27,    28,    29,
     209,    30,    31,     0,     0,     0,     0,     0,     0,     0,
      33,    34,   221,    35,     0,     0,     0,    36,    37,    38,
      39,     0,     0,    40,   373,   478,   373,    41,    42,     0,
       0,   222,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,    44,    45,     0,     0,     0,   494,   126,   496,
       0,     0,    46,    47,     0,   224,   225,    48,    49,     0,
      50,   226,     0,   227,    51,   266,    52,     0,    53,     0,
     228,     0,     0,   229,     0,     0,     0,   230,     0,   316,
       0,   231,     0,     0,     0,     0,     0,   232,   233,     0,
       0,   234,     0,     0,     0,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,     0,   245,   209,   317,   209,
     318,   225,     0,   524,     0,   126,   226,     0,   227,     0,
       0,     0,     0,   528,     0,   228,     0,     0,   229,     0,
       0,     0,   230,     0,     0,     0,   231,     0,     0,     0,
       0,     0,   232,   233,     0,     0,   234,     0,     0,     0,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
       0,   245,   313,   558,     1,     0,     0,     0,   126,   126,
       0,     2,     3,     0,     4,     5,     0,     6,     7,     0,
       8,     9,    10,    11,    12,    13,    14,     0,    15,    16,
      17,    18,     0,    19,     0,     0,    20,     0,     0,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,    30,
      31,     0,     0,     0,     0,    32,     0,     0,    33,    34,
       0,    35,     0,     0,     0,    36,    37,    38,    39,     0,
       0,    40,     0,     0,     0,    41,    42,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
      44,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,    47,     0,     0,     0,    48,    49,     0,    50,     0,
       1,     0,    51,   297,    52,     0,    53,     2,     3,     0,
       4,     5,     0,     6,     7,     0,     8,     9,    10,    11,
      12,    13,    14,     0,    15,    16,    17,    18,     0,    19,
       0,     0,    20,     0,     0,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,    30,    31,     0,     0,     0,
       0,    32,     0,     0,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,     0,    40,     0,     0,
       0,    41,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,    44,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,    47,     0,     0,
       0,    48,    49,     0,    50,     0,     1,     0,    51,   402,
      52,     0,    53,     2,     3,     0,     4,     5,     0,     6,
       7,     0,     8,     9,    10,    11,    12,    13,    14,     0,
      15,    16,    17,    18,     0,    19,     0,     0,    20,     0,
       0,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,    30,    31,     0,     0,     0,     0,    32,     0,     0,
      33,    34,     0,    35,     0,     0,     0,    36,    37,    38,
      39,     0,     0,    40,     0,     0,     0,    41,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,    44,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,    47,     0,     0,     0,    48,    49,     0,
      50,     0,     1,     0,    51,   535,    52,     0,    53,     2,
       3,     0,     4,     5,     0,     6,     7,     0,     8,     9,
      10,    11,    12,    13,    14,     0,    15,    16,    17,    18,
       0,    19,     0,     0,    20,     0,     0,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,    30,    31,     0,
       0,     0,     0,    32,     0,     0,    33,    34,     0,    35,
       0,     0,     0,    36,    37,    38,    39,     0,     0,    40,
       0,     0,     0,    41,    42,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,    44,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,    47,
       0,     0,     0,    48,    49,     0,    50,     0,     1,     0,
      51,   543,    52,     0,    53,     2,     3,     0,     4,     5,
       0,     6,     7,     0,     8,     9,    10,    11,    12,    13,
      14,     0,    15,    16,    17,    18,     0,    19,     0,     0,
      20,     0,     0,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,    30,    31,     0,     0,     0,     0,    32,
       0,     0,    33,    34,     0,    35,     0,     0,     0,    36,
      37,    38,    39,     0,     0,    40,     0,     0,     0,    41,
      42,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,    44,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,    47,     0,     0,     0,    48,
      49,     0,    50,     0,     1,     0,    51,   545,    52,     0,
      53,     2,     3,     0,     4,     5,     0,     6,     7,     0,
       8,     9,    10,    11,    12,    13,    14,     0,    15,    16,
      17,    18,     0,    19,     0,     0,    20,     0,     0,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,    30,
      31,     0,     0,     0,     0,    32,     0,     0,    33,    34,
       0,    35,     0,     0,     0,    36,    37,    38,    39,     0,
       0,    40,     0,     0,     0,    41,    42,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
      44,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,    47,     0,     0,     0,    48,    49,     0,    50,     0,
       1,     0,    51,   550,    52,     0,    53,     2,     3,     0,
       4,     5,     0,     6,     7,     0,     8,     9,    10,    11,
      12,    13,    14,     0,    15,    16,    17,    18,     0,    19,
       0,     0,    20,     0,     0,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,    30,    31,     0,     0,     0,
       0,    32,     0,     0,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,     0,    40,     0,     0,
       0,    41,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,    44,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,    47,     0,     0,
       0,    48,    49,     0,    50,     0,     1,     0,    51,   571,
      52,     0,    53,     2,     3,     0,     4,     5,     0,     6,
       7,     0,     8,     9,    10,    11,    12,    13,    14,     0,
      15,    16,    17,    18,     0,    19,     0,     0,    20,     0,
       0,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,    30,    31,     0,     0,     0,     0,    32,     0,     0,
      33,    34,     0,    35,     0,     0,     0,    36,    37,    38,
      39,     0,     0,    40,     0,     0,     0,    41,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,    44,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,    47,     0,     0,     0,    48,    49,     0,
      50,     0,     1,     0,    51,   580,    52,     0,    53,     2,
       3,     0,     4,     5,     0,     6,     7,     0,     8,     9,
      10,    11,    12,    13,    14,     0,    15,    16,    17,    18,
       0,    19,     0,     0,    20,     0,     0,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,    30,    31,     0,
       0,     0,     0,    32,     0,     0,    33,    34,     0,    35,
       0,     0,     0,    36,    37,    38,    39,     0,     0,    40,
       0,     0,     0,    41,    42,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,    44,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,    47,
       0,     0,     0,    48,    49,     0,    50,     0,     1,     0,
      51,   596,    52,     0,    53,     2,     3,     0,     4,     5,
       0,     6,     7,     0,     8,     9,    10,    11,    12,    13,
      14,     0,    15,    16,    17,    18,     0,    19,     0,     0,
      20,     0,     0,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,    30,    31,     0,     0,     0,     0,    32,
       0,     0,    33,    34,     0,    35,     0,     0,     0,    36,
      37,    38,    39,     0,     0,    40,     0,     0,     0,    41,
      42,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,    44,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,    47,     0,     0,     1,    48,
      49,     0,    50,     0,     0,     2,    51,     0,    52,     5,
      53,     0,     7,     0,     8,     0,    10,    11,    12,    13,
       0,     0,    15,    16,    17,    18,     0,     0,     0,     0,
      20,     0,     0,    21,     0,     0,    24,    25,    26,    27,
      28,    29,     0,    30,    31,     0,     0,     0,     0,   206,
       0,     0,    33,    34,     0,    35,     0,     0,     0,    36,
      37,    38,    39,     0,     0,    40,     0,     0,     0,    41,
      42,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,    44,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,    47,     0,     0,     1,    48,
      49,     0,    50,     0,     0,     2,    51,     0,    52,     5,
      53,     0,     7,     0,     8,     0,    10,    11,    12,    13,
       0,     0,    15,    16,    17,    18,     0,     0,     0,     0,
      20,     0,     0,    21,     0,     0,    24,    25,    26,    27,
      28,    29,     0,    30,    31,     0,     0,     0,     0,     0,
       0,     0,    33,    34,     0,    35,     0,     0,     0,    36,
      37,    38,    39,     0,     0,    40,     0,     0,     0,    41,
      42,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,    44,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,    47,     0,     0,     0,    48,
      49,     0,    50,     0,     1,     0,    51,     0,    52,   331,
      53,     2,     0,     0,     0,     5,     0,     0,     7,     0,
       8,     0,    10,    11,    12,    13,     0,     0,    15,    16,
      17,    18,     0,     0,     0,     0,    20,     0,     0,    21,
       0,     0,    24,    25,    26,    27,    28,    29,     0,    30,
      31,     0,     0,     0,     0,     0,     0,     0,    33,    34,
       0,    35,     0,     0,     0,    36,    37,    38,    39,     0,
       0,    40,     0,     0,     0,    41,    42,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
      44,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,    47,     0,     0,     0,    48,    49,     0,    50,     0,
       1,   538,    51,     0,    52,     0,    53,     2,     0,     0,
       0,     5,     0,     0,     7,     0,     8,     0,    10,    11,
      12,    13,     0,     0,    15,    16,    17,    18,     0,     0,
       0,     0,    20,     0,     0,    21,     0,     0,    24,    25,
      26,    27,    28,    29,     0,    30,    31,     0,     0,     0,
       0,     0,     0,     0,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,     0,    40,     0,     0,
       0,    41,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,    44,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,    47,     0,     0,
       0,    48,    49,     0,    50,     0,     1,   565,    51,     0,
      52,     0,    53,     2,     0,     0,     0,     5,     0,     0,
       7,     0,     8,     0,    10,    11,    12,    13,     0,     0,
      15,    16,    17,    18,     0,     0,     0,     0,    20,     0,
       0,    21,     0,     0,    24,    25,    26,    27,    28,    29,
       0,    30,    31,     0,     0,     0,     0,     0,     0,     0,
      33,    34,     0,    35,     0,     0,     0,    36,    37,    38,
      39,     0,     0,    40,     1,     0,     0,    41,    42,     0,
       0,   159,     0,     0,     0,     0,     0,     0,     0,    43,
       8,     0,    44,    45,    12,     0,     0,     0,     0,    16,
      17,    18,    46,    47,     0,     0,     0,    48,    49,    21,
      50,     0,    24,     0,    51,    27,    52,    29,    53,     0,
      31,     0,     0,     0,     0,   206,     0,     0,    33,    34,
       0,    35,     0,     0,     0,    36,    37,    38,    39,     0,
       0,    40,     0,     0,     0,    41,    42,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
      44,    45,     0,     1,     0,   207,     0,     0,     0,     0,
      46,    47,     0,     0,     0,    48,    49,     0,    50,     8,
       0,     0,    51,    12,    52,   208,    53,     0,    16,    17,
      18,     0,     0,     0,     0,     0,     0,     0,    21,     0,
       0,    24,     0,     0,    27,     0,    29,     0,     0,    31,
       0,     0,     0,     0,   206,     0,     0,    33,    34,     0,
      35,     0,     0,     0,    36,    37,    38,    39,     0,     0,
      40,     0,     0,     0,    41,    42,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,    44,
      45,     0,     1,     0,   207,     0,     0,     0,     0,    46,
      47,     0,     0,     0,    48,    49,     0,    50,     8,     0,
       0,    51,    12,    52,   208,    53,     0,    16,    17,    18,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
      24,     0,     0,    27,     0,    29,     0,     0,    31,     0,
       0,     0,     0,   194,     0,     0,    33,    34,     0,    35,
       0,     0,     0,    36,    37,    38,    39,     0,     0,    40,
       1,     0,     0,    41,    42,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    43,     8,     0,    44,    45,
      12,     0,     0,     0,     0,    16,    17,    18,    46,    47,
       0,     0,     0,    48,    49,    21,    50,     0,    24,   195,
      51,    27,    52,    29,    53,     0,    31,     0,     0,     0,
       0,   206,     0,     0,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,     0,    40,     1,     0,
       0,    41,    42,     0,     0,   159,     0,     0,     0,     0,
       0,     0,     0,    43,     8,     0,    44,    45,    12,     0,
       0,   207,     0,    16,    17,    18,    46,    47,     0,     0,
       0,    48,    49,    21,    50,     0,    24,     0,    51,    27,
      52,    29,    53,     0,    31,     0,     0,     0,     0,     0,
       0,     0,    33,    34,     0,    35,     0,     0,     0,    36,
      37,    38,    39,     0,     0,    40,     1,     0,     0,    41,
      42,     0,     0,   159,     0,     0,     0,     0,     0,     0,
       0,    43,     8,     0,    44,    45,    12,     0,     0,     0,
       0,    16,    17,    18,    46,    47,     0,     0,     0,    48,
      49,    21,    50,     0,    24,     0,    51,    27,   160,    29,
      53,     0,    31,     0,     0,     0,     0,     0,     0,     0,
      33,    34,     0,    35,     0,     0,     0,    36,    37,    38,
      39,     0,     0,    40,     1,     0,     0,    41,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    43,
       8,     0,    44,    45,    12,     0,     0,     0,     0,    16,
      17,    18,    46,    47,     0,     0,     0,    48,    49,    21,
      50,     0,    24,     0,    51,    27,    52,    29,    53,     0,
      31,     0,     0,     0,     0,   194,     0,     0,    33,    34,
       0,    35,     0,     0,     0,    36,    37,    38,    39,     0,
       0,    40,     1,     0,     0,    41,    42,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    43,     8,     0,
      44,    45,    12,     0,     0,     0,     0,    16,    17,    18,
      46,    47,     0,     0,     0,    48,    49,    21,    50,     0,
      24,     0,    51,    27,    52,    29,    53,     0,    31,     0,
       0,     0,     0,     0,     0,     0,    33,    34,     0,    35,
       0,     0,     0,    36,    37,    38,    39,     0,     0,    40,
       1,     0,     0,    41,    42,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    43,     8,     0,    44,    45,
      12,     0,     0,   207,     0,    16,    17,    18,    46,    47,
       0,     0,     0,    48,    49,    21,    50,     0,    24,     0,
      51,    27,    52,    29,    53,     0,    31,     0,     0,     0,
       0,     0,     0,     0,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,     0,    40,     1,     0,
       0,    41,    42,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    43,     8,     0,    44,    45,    12,     0,
       0,     0,     0,    16,    17,    18,    46,    47,   221,     0,
       0,    48,    49,    21,   125,     0,    24,     0,    51,    27,
      52,    29,    53,     0,    31,     0,     0,   222,     0,     0,
       0,     0,    33,    34,     0,    35,     0,     0,     0,    36,
      37,    38,    39,     0,     0,    40,     0,     0,     0,    41,
      42,   224,   225,     0,     0,     0,     0,   226,     0,   227,
       0,    43,     0,     0,    44,    45,   228,     0,     0,   229,
     221,     0,     0,   230,    46,    47,     0,   231,     0,    48,
      49,     0,    50,   232,   233,     0,    51,   234,    52,   222,
      53,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,     0,   245,     0,     0,     0,   537,     0,     0,     0,
       0,     0,     0,   224,   225,     0,     0,     0,     0,   226,
       0,   227,   221,     0,     0,     0,     0,     0,   228,     0,
       0,   229,     0,     0,     0,   230,     0,     0,     0,   231,
       0,   222,     0,     0,     0,   232,   233,     0,     0,   234,
       0,     0,     0,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,     0,   245,   224,   225,     0,   579,     0,
       0,   226,     0,   227,     0,     0,     0,     0,     0,     0,
     228,   221,     0,   229,     0,     0,     0,   230,     0,     0,
       0,   231,     0,     0,     0,     0,     0,   232,   233,     0,
     222,   234,     0,     0,     0,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,     0,   245,     0,     0,   521,
       0,     0,     0,   206,   224,   225,     0,     0,     0,     0,
     226,     0,   227,     0,     0,     0,     0,     0,     0,   228,
     221,     0,   229,     0,     0,     0,   230,     0,     0,     0,
     231,     0,     0,     0,     0,     0,   232,   233,     0,   222,
     234,     0,     0,   324,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,     0,   245,   223,     0,     0,     0,
       0,     0,     0,   224,   225,     0,     0,     0,     0,   226,
       0,   227,   221,     0,     0,     0,     0,     0,   228,     0,
       0,   229,     0,     0,     0,   230,     0,     0,     0,   231,
       0,   222,     0,     0,     0,   232,   233,     0,     0,   234,
       0,     0,     0,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,     0,   245,   224,   225,     0,     0,     0,
       0,   226,     0,   227,   221,     0,     0,     0,     0,     0,
     228,     0,     0,   229,     0,     0,     0,   230,     0,     0,
       0,   231,     0,   222,     0,     0,     0,   232,   233,     0,
       0,   234,     0,   405,     0,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,     0,   245,   224,   225,     0,
       0,     0,     0,   226,     0,   227,   221,     0,     0,     0,
       0,     0,   228,     0,     0,   229,     0,     0,     0,   230,
       0,   316,     0,   231,     0,   222,     0,     0,     0,   232,
     233,     0,     0,   234,     0,     0,     0,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,     0,   245,   224,
     225,     0,     0,     0,     0,   226,     0,   227,   221,     0,
       0,     0,     0,     0,   228,     0,     0,   229,     0,     0,
       0,   230,     0,     0,     0,   231,     0,   222,     0,     0,
       0,   232,   233,     0,     0,   234,     0,     0,   520,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,     0,
     245,   224,   225,     0,     0,     0,     0,   226,     0,   227,
       0,     0,     0,     0,     0,     0,   228,     0,     0,   229,
       0,     0,     0,   230,     0,     0,     0,   231,     0,     0,
       0,     0,     0,   232,   233,     0,     0,   234,     0,     0,
       0,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,     0,   245,   224,   225,     0,     0,     0,     0,   226,
       0,   227,     0,     0,     0,     0,     0,     0,   228,     0,
       0,   229,     0,     0,     0,   230,     0,     0,     0,   231,
       0,     0,     0,     0,     0,   232,   233,     0,     0,   234,
       0,     0,     0,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,     0,   245
};

static const yytype_int16 yycheck[] =
{
       0,    19,     2,    36,   406,   125,    95,    28,   194,    36,
     408,   207,     0,   416,   210,   418,    16,    56,   272,    52,
     135,    13,    57,    57,    20,   156,    27,    18,    18,   112,
      82,    57,    25,   110,    36,    35,    36,    30,    83,    56,
      60,    96,    86,   272,    83,   110,   135,    83,    68,    51,
      52,    51,    13,    97,    50,   110,    57,    97,    19,    99,
     103,    97,    20,    73,   107,   117,    57,    57,    60,     0,
      80,   473,   117,    93,    93,   110,    68,    20,   117,   114,
     114,   117,    93,    44,   199,    85,   110,    86,   114,    20,
       0,   206,   223,   104,   209,    95,   113,   107,    97,   109,
     498,    93,   185,    13,   507,    36,   110,    87,    88,    19,
      71,   214,   116,   114,    26,   218,   261,   262,   263,    50,
      51,    52,     0,   156,   526,   527,   215,   160,   324,   125,
     213,     0,    93,   110,    44,   135,   110,   391,   118,   259,
      87,   137,   540,   397,   398,   399,   400,    57,    87,    88,
     270,   151,    56,     0,   115,   155,   156,    67,   160,   159,
     160,    71,   391,    14,    95,    16,    86,   155,   397,   398,
     399,   400,    23,     0,   110,    86,   291,    97,    57,    57,
     116,    56,     0,    93,     2,     0,    97,    97,    57,    67,
     116,    42,    93,   596,   125,    19,   599,   114,    67,    56,
      56,   290,   110,   113,   135,   115,   137,   117,   116,   324,
      57,    39,    40,   583,   584,   215,    29,   114,    36,    97,
     220,    97,    50,   593,   155,    57,    67,   110,    97,   160,
     110,   113,   220,    51,    52,   113,   109,   115,    57,   117,
     117,   113,    57,   117,   113,   115,   115,   391,   117,   115,
      97,     0,    67,   397,   398,   399,   400,   259,   118,   261,
     262,   263,   382,    77,   384,   110,   113,   116,   115,   465,
     117,   467,     0,   306,   307,   275,    93,    95,   115,   306,
     307,    57,    97,   113,   215,    13,   288,   275,    97,   220,
     290,    19,    97,    93,   409,   115,    97,   386,   113,   414,
     115,   113,   117,   303,   306,   307,   306,   307,    57,   113,
     113,   113,    99,    56,    13,   113,    44,   135,    67,   408,
      19,   410,   117,   117,   117,   113,    97,   113,   259,    57,
     261,   262,   263,   113,   520,    97,    97,   155,   115,    67,
     114,   159,   160,    71,   275,    44,   110,    18,    97,    21,
     113,    56,    56,   113,   113,   113,   113,   288,   114,   290,
     112,     0,   115,    93,   113,    93,   115,   115,   117,    97,
     388,   115,    71,   406,    13,   306,   307,   115,   115,   406,
      19,   113,   115,   386,   499,   113,   386,   115,   113,   117,
     411,   115,   115,    73,    93,   137,   553,   215,   419,   420,
      80,   541,   220,   541,   406,    44,   406,    13,   408,   498,
     410,    51,   306,    19,   307,   246,   115,    39,    57,   258,
     120,   529,   422,    29,   104,   105,   106,   107,    67,   109,
     486,   404,    71,    -1,    -1,    -1,    -1,    -1,    44,    -1,
     473,   259,    -1,   261,   262,   263,   473,    -1,     8,     9,
      13,   540,    -1,    13,    93,   386,    19,   275,    97,    19,
      -1,    13,    -1,    -1,    -1,    71,    -1,    19,    -1,    -1,
     288,   473,   290,   473,   113,   406,   115,   408,   117,   410,
       0,    44,     2,    -1,    44,    -1,    -1,    93,   306,   307,
      -1,   491,    44,   526,   527,    -1,    -1,    -1,   498,   526,
     527,   501,   502,   491,    -1,   505,    -1,    -1,    71,   509,
     510,    71,    -1,   501,   502,    -1,    36,   538,    -1,    71,
     541,   509,   510,    13,   526,   527,   526,   527,    -1,    19,
      93,    51,    52,    93,    -1,    -1,    -1,    -1,    -1,    -1,
     540,    93,   473,   564,   565,    -1,    -1,   547,   548,    -1,
      -1,    -1,    -1,   553,    44,    -1,    13,    -1,    -1,   547,
     491,    -1,    19,   563,   585,    -1,   587,   498,   386,    -1,
     501,   502,    -1,    -1,    -1,    95,    -1,     0,   509,   510,
      -1,    71,    -1,   583,   584,    -1,    -1,    44,   406,    -1,
     408,    13,   410,   593,   594,   526,   527,    19,    -1,    -1,
      -1,    -1,    14,    93,    16,    -1,   594,    19,    -1,   540,
      -1,    23,    -1,    36,    71,   135,   547,    -1,    -1,    13,
      -1,    -1,    44,    -1,    -1,    19,    -1,    -1,    51,    52,
      42,    43,   563,    -1,    -1,   155,    93,    -1,    -1,   159,
     160,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,    71,
      44,    -1,   583,   584,    -1,   473,    68,    69,    70,    -1,
      -1,    -1,   593,   594,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    95,   491,    -1,    -1,    -1,    71,    64,    -1,
     498,    -1,    -1,   501,   502,    -1,    -1,    73,    -1,    -1,
      -1,   509,   510,    -1,    80,   215,    -1,    -1,    84,    93,
     220,    -1,    -1,   115,    -1,    -1,    -1,    -1,   526,   527,
      -1,    -1,   135,    -1,   100,   101,   102,   103,   104,   105,
     106,   107,   540,   109,    -1,    -1,    -1,    73,    -1,   547,
      -1,    -1,   155,    -1,    80,    -1,    -1,   160,    -1,   259,
      -1,   261,   262,   263,    -1,   563,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    55,   275,   102,   103,   104,   105,
     106,   107,    -1,   109,    65,   583,   584,    -1,   288,    -1,
     290,    72,    -1,    74,    75,   593,   594,     0,    79,    -1,
      81,     3,     4,     5,    85,     7,   306,   307,    10,    11,
      12,    -1,   215,    -1,    95,    96,    -1,   220,    -1,    53,
      54,    55,    -1,    -1,    -1,    -1,     0,    -1,    -1,   110,
      -1,    65,    -1,    36,    -1,   116,    -1,    73,    72,    -1,
      74,    75,    -1,    -1,    80,    79,    -1,    81,    51,    52,
      -1,    85,    -1,    -1,    -1,    -1,   259,    -1,   261,   262,
     263,    95,    -1,    -1,   100,   101,   102,   103,   104,   105,
     106,   107,   275,   109,    -1,    -1,   110,    -1,    -1,    53,
      54,    55,   116,    57,    -1,   288,   386,   290,    -1,    -1,
      -1,    65,    95,    67,    -1,    -1,    -1,    -1,    72,    -1,
      74,    75,    -1,   306,   307,    79,   406,    81,   408,    -1,
     410,    85,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    95,    -1,    97,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    72,   135,    74,    75,    -1,   110,    -1,    79,   113,
      81,   115,   116,   117,    85,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   155,    -1,    95,    -1,    -1,   160,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,
      65,    -1,    -1,   473,   115,   116,    -1,    72,    -1,    74,
      75,    -1,    -1,   386,    79,    -1,    81,    -1,    -1,    -1,
      85,   491,    -1,    -1,    -1,    -1,    -1,    -1,   498,    -1,
      95,   501,   502,   406,    -1,   408,    -1,   410,    -1,   509,
     510,    -1,   215,    -1,    -1,   110,    -1,   220,   113,    -1,
      -1,   116,    -1,    -1,    -1,    -1,   526,   527,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,
     540,    -1,    -1,    -1,    -1,    -1,    73,   547,    -1,    -1,
      -1,    -1,    -1,    80,    -1,     0,   259,    84,   261,   262,
     263,    -1,    -1,   563,    -1,    -1,    -1,    94,    -1,    -1,
     473,    -1,   275,   100,   101,   102,   103,   104,   105,   106,
     107,    -1,   109,   583,   584,   288,    -1,   290,   491,    -1,
      -1,    -1,    -1,   593,   594,   498,    -1,    -1,   501,   502,
      -1,    -1,    -1,   306,   307,    -1,   509,   510,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    67,   526,   527,    73,    -1,    72,    -1,    74,
      75,    -1,    80,    -1,    79,    -1,    81,   540,    -1,    -1,
      85,    -1,    -1,    -1,   547,    -1,    -1,    -1,    -1,    -1,
      95,    -1,    97,   101,   102,   103,   104,   105,   106,   107,
     563,   109,    -1,    -1,    -1,   110,    -1,    -1,   113,    -1,
     115,   116,   117,    -1,     2,    -1,    -1,    -1,    -1,    -1,
     583,   584,    -1,   386,    -1,    -1,    -1,    -1,    -1,    -1,
     593,   594,    20,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      -1,    -1,    -1,   406,    -1,   408,    -1,   410,    36,    -1,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    50,    51,    52,    -1,    53,    54,    55,    -1,
      -1,    58,    59,    -1,    -1,    -1,    -1,    64,    65,    66,
      -1,    -1,    -1,    -1,    -1,    72,    73,    74,    75,    -1,
      -1,    -1,    79,    80,    81,    -1,    -1,    84,    85,    -1,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    94,    95,    -1,
     473,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,    -1,   109,   110,    -1,    -1,    -1,    -1,   491,   116,
      -1,    -1,    -1,    -1,    -1,   498,   124,   125,   501,   502,
      -1,    -1,    -1,    -1,    -1,    -1,   509,   510,    -1,   137,
      -1,    -1,    -1,    -1,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,   526,   527,    -1,    65,    -1,   156,    -1,
      -1,   159,   160,    72,    -1,    74,    75,   540,    -1,    -1,
      79,    -1,    81,    82,   547,    -1,    85,    -1,    -1,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    95,    -1,    -1,    -1,
     563,    -1,    -1,    -1,    -1,    -1,   194,    -1,    -1,    -1,
      -1,   110,    -1,    -1,    -1,    -1,    -1,   116,    -1,   207,
     583,   584,   210,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     593,   594,    -1,   221,   222,   223,   224,   225,   226,   227,
      -1,   229,    -1,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,    -1,   245,    -1,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,    64,   261,   262,   263,    -1,    -1,    -1,    -1,
      -1,    73,   270,    -1,    76,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,    90,    91,
     288,    -1,    94,    -1,    -1,    -1,    -1,    -1,   100,   101,
     102,   103,   104,   105,   106,   107,    -1,   109,   306,   307,
      -1,    -1,    -1,    -1,    -1,    -1,     6,    -1,   316,   317,
      -1,    -1,    -1,    13,    -1,    -1,   324,    17,    -1,    -1,
      20,    -1,    22,    -1,    24,    25,    26,    27,    -1,    -1,
      30,    31,    32,    33,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    41,    -1,    -1,    44,    45,    46,    47,    48,    49,
     358,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    15,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    -1,    -1,    73,   382,   383,   384,    77,    78,    -1,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      -1,    -1,    92,    93,    -1,    -1,    -1,   405,   406,   407,
      -1,    -1,   102,   103,    -1,    58,    59,   107,   108,    -1,
     110,    64,    -1,    66,   114,   115,   116,    -1,   118,    -1,
      73,    -1,    -1,    76,    -1,    -1,    -1,    80,    -1,    82,
      -1,    84,    -1,    -1,    -1,    -1,    -1,    90,    91,    -1,
      -1,    94,    -1,    -1,    -1,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    -1,   109,   465,   111,   467,
     113,    59,    -1,   471,    -1,   473,    64,    -1,    66,    -1,
      -1,    -1,    -1,   481,    -1,    73,    -1,    -1,    76,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    84,    -1,    -1,    -1,
      -1,    -1,    90,    91,    -1,    -1,    94,    -1,    -1,    -1,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
      -1,   109,   520,   521,     6,    -1,    -1,    -1,   526,   527,
      -1,    13,    14,    -1,    16,    17,    -1,    19,    20,    -1,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    -1,    35,    -1,    -1,    38,    -1,    -1,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      52,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,    61,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    -1,
      -1,    73,    -1,    -1,    -1,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     102,   103,    -1,    -1,    -1,   107,   108,    -1,   110,    -1,
       6,    -1,   114,   115,   116,    -1,   118,    13,    14,    -1,
      16,    17,    -1,    19,    20,    -1,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    31,    32,    33,    -1,    35,
      -1,    -1,    38,    -1,    -1,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    52,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    60,    61,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    -1,    -1,    73,    -1,    -1,
      -1,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    92,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,   103,    -1,    -1,
      -1,   107,   108,    -1,   110,    -1,     6,    -1,   114,   115,
     116,    -1,   118,    13,    14,    -1,    16,    17,    -1,    19,
      20,    -1,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    -1,    35,    -1,    -1,    38,    -1,
      -1,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      60,    61,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    -1,    -1,    73,    -1,    -1,    -1,    77,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   102,   103,    -1,    -1,    -1,   107,   108,    -1,
     110,    -1,     6,    -1,   114,   115,   116,    -1,   118,    13,
      14,    -1,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      -1,    35,    -1,    -1,    38,    -1,    -1,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    51,    52,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    60,    61,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    -1,    -1,    73,
      -1,    -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,    92,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,   103,
      -1,    -1,    -1,   107,   108,    -1,   110,    -1,     6,    -1,
     114,   115,   116,    -1,   118,    13,    14,    -1,    16,    17,
      -1,    19,    20,    -1,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    -1,    35,    -1,    -1,
      38,    -1,    -1,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    51,    52,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    60,    61,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    -1,    -1,    73,    -1,    -1,    -1,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   102,   103,    -1,    -1,    -1,   107,
     108,    -1,   110,    -1,     6,    -1,   114,   115,   116,    -1,
     118,    13,    14,    -1,    16,    17,    -1,    19,    20,    -1,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    -1,    35,    -1,    -1,    38,    -1,    -1,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      52,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,    61,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    -1,
      -1,    73,    -1,    -1,    -1,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     102,   103,    -1,    -1,    -1,   107,   108,    -1,   110,    -1,
       6,    -1,   114,   115,   116,    -1,   118,    13,    14,    -1,
      16,    17,    -1,    19,    20,    -1,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    31,    32,    33,    -1,    35,
      -1,    -1,    38,    -1,    -1,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    52,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    60,    61,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    -1,    -1,    73,    -1,    -1,
      -1,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    92,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,   103,    -1,    -1,
      -1,   107,   108,    -1,   110,    -1,     6,    -1,   114,   115,
     116,    -1,   118,    13,    14,    -1,    16,    17,    -1,    19,
      20,    -1,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    -1,    35,    -1,    -1,    38,    -1,
      -1,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    51,    52,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      60,    61,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    -1,    -1,    73,    -1,    -1,    -1,    77,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      -1,    -1,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   102,   103,    -1,    -1,    -1,   107,   108,    -1,
     110,    -1,     6,    -1,   114,   115,   116,    -1,   118,    13,
      14,    -1,    16,    17,    -1,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      -1,    35,    -1,    -1,    38,    -1,    -1,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    51,    52,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    60,    61,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    -1,    -1,    73,
      -1,    -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,    92,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,   103,
      -1,    -1,    -1,   107,   108,    -1,   110,    -1,     6,    -1,
     114,   115,   116,    -1,   118,    13,    14,    -1,    16,    17,
      -1,    19,    20,    -1,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    -1,    35,    -1,    -1,
      38,    -1,    -1,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    51,    52,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    60,    61,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    -1,    -1,    73,    -1,    -1,    -1,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   102,   103,    -1,    -1,     6,   107,
     108,    -1,   110,    -1,    -1,    13,   114,    -1,   116,    17,
     118,    -1,    20,    -1,    22,    -1,    24,    25,    26,    27,
      -1,    -1,    30,    31,    32,    33,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    41,    -1,    -1,    44,    45,    46,    47,
      48,    49,    -1,    51,    52,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    60,    61,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    -1,    -1,    73,    -1,    -1,    -1,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   102,   103,    -1,    -1,     6,   107,
     108,    -1,   110,    -1,    -1,    13,   114,    -1,   116,    17,
     118,    -1,    20,    -1,    22,    -1,    24,    25,    26,    27,
      -1,    -1,    30,    31,    32,    33,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    41,    -1,    -1,    44,    45,    46,    47,
      48,    49,    -1,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    -1,    -1,    73,    -1,    -1,    -1,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    -1,    -1,    92,    93,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   102,   103,    -1,    -1,    -1,   107,
     108,    -1,   110,    -1,     6,    -1,   114,    -1,   116,   117,
     118,    13,    -1,    -1,    -1,    17,    -1,    -1,    20,    -1,
      22,    -1,    24,    25,    26,    27,    -1,    -1,    30,    31,
      32,    33,    -1,    -1,    -1,    -1,    38,    -1,    -1,    41,
      -1,    -1,    44,    45,    46,    47,    48,    49,    -1,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    -1,
      -1,    73,    -1,    -1,    -1,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     102,   103,    -1,    -1,    -1,   107,   108,    -1,   110,    -1,
       6,   113,   114,    -1,   116,    -1,   118,    13,    -1,    -1,
      -1,    17,    -1,    -1,    20,    -1,    22,    -1,    24,    25,
      26,    27,    -1,    -1,    30,    31,    32,    33,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    41,    -1,    -1,    44,    45,
      46,    47,    48,    49,    -1,    51,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    61,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    -1,    -1,    73,    -1,    -1,
      -1,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    -1,    92,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,   103,    -1,    -1,
      -1,   107,   108,    -1,   110,    -1,     6,   113,   114,    -1,
     116,    -1,   118,    13,    -1,    -1,    -1,    17,    -1,    -1,
      20,    -1,    22,    -1,    24,    25,    26,    27,    -1,    -1,
      30,    31,    32,    33,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    41,    -1,    -1,    44,    45,    46,    47,    48,    49,
      -1,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    -1,    -1,    73,     6,    -1,    -1,    77,    78,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      22,    -1,    92,    93,    26,    -1,    -1,    -1,    -1,    31,
      32,    33,   102,   103,    -1,    -1,    -1,   107,   108,    41,
     110,    -1,    44,    -1,   114,    47,   116,    49,   118,    -1,
      52,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,    61,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    -1,
      -1,    73,    -1,    -1,    -1,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
      92,    93,    -1,     6,    -1,    97,    -1,    -1,    -1,    -1,
     102,   103,    -1,    -1,    -1,   107,   108,    -1,   110,    22,
      -1,    -1,   114,    26,   116,   117,   118,    -1,    31,    32,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    44,    -1,    -1,    47,    -1,    49,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    60,    61,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    -1,    -1,
      73,    -1,    -1,    -1,    77,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,    92,
      93,    -1,     6,    -1,    97,    -1,    -1,    -1,    -1,   102,
     103,    -1,    -1,    -1,   107,   108,    -1,   110,    22,    -1,
      -1,   114,    26,   116,   117,   118,    -1,    31,    32,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      44,    -1,    -1,    47,    -1,    49,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    60,    61,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    -1,    -1,    73,
       6,    -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    22,    -1,    92,    93,
      26,    -1,    -1,    -1,    -1,    31,    32,    33,   102,   103,
      -1,    -1,    -1,   107,   108,    41,   110,    -1,    44,   113,
     114,    47,   116,    49,   118,    -1,    52,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    60,    61,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    -1,    -1,    73,     6,    -1,
      -1,    77,    78,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    22,    -1,    92,    93,    26,    -1,
      -1,    97,    -1,    31,    32,    33,   102,   103,    -1,    -1,
      -1,   107,   108,    41,   110,    -1,    44,    -1,   114,    47,
     116,    49,   118,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    -1,    -1,    73,     6,    -1,    -1,    77,
      78,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    22,    -1,    92,    93,    26,    -1,    -1,    -1,
      -1,    31,    32,    33,   102,   103,    -1,    -1,    -1,   107,
     108,    41,   110,    -1,    44,    -1,   114,    47,   116,    49,
     118,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    -1,    -1,    73,     6,    -1,    -1,    77,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      22,    -1,    92,    93,    26,    -1,    -1,    -1,    -1,    31,
      32,    33,   102,   103,    -1,    -1,    -1,   107,   108,    41,
     110,    -1,    44,    -1,   114,    47,   116,    49,   118,    -1,
      52,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,    61,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    -1,
      -1,    73,     6,    -1,    -1,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    22,    -1,
      92,    93,    26,    -1,    -1,    -1,    -1,    31,    32,    33,
     102,   103,    -1,    -1,    -1,   107,   108,    41,   110,    -1,
      44,    -1,   114,    47,   116,    49,   118,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    -1,    -1,    73,
       6,    -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    22,    -1,    92,    93,
      26,    -1,    -1,    97,    -1,    31,    32,    33,   102,   103,
      -1,    -1,    -1,   107,   108,    41,   110,    -1,    44,    -1,
     114,    47,   116,    49,   118,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    61,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    -1,    -1,    73,     6,    -1,
      -1,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    22,    -1,    92,    93,    26,    -1,
      -1,    -1,    -1,    31,    32,    33,   102,   103,    15,    -1,
      -1,   107,   108,    41,   110,    -1,    44,    -1,   114,    47,
     116,    49,   118,    -1,    52,    -1,    -1,    34,    -1,    -1,
      -1,    -1,    60,    61,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    -1,    -1,    73,    -1,    -1,    -1,    77,
      78,    58,    59,    -1,    -1,    -1,    -1,    64,    -1,    66,
      -1,    89,    -1,    -1,    92,    93,    73,    -1,    -1,    76,
      15,    -1,    -1,    80,   102,   103,    -1,    84,    -1,   107,
     108,    -1,   110,    90,    91,    -1,   114,    94,   116,    34,
     118,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,    -1,   109,    -1,    -1,    -1,   113,    -1,    -1,    -1,
      -1,    -1,    -1,    58,    59,    -1,    -1,    -1,    -1,    64,
      -1,    66,    15,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    84,
      -1,    34,    -1,    -1,    -1,    90,    91,    -1,    -1,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,   109,    58,    59,    -1,   113,    -1,
      -1,    64,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    15,    -1,    76,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    84,    -1,    -1,    -1,    -1,    -1,    90,    91,    -1,
      34,    94,    -1,    -1,    -1,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    -1,   109,    -1,    -1,   112,
      -1,    -1,    -1,    57,    58,    59,    -1,    -1,    -1,    -1,
      64,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      15,    -1,    76,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      84,    -1,    -1,    -1,    -1,    -1,    90,    91,    -1,    34,
      94,    -1,    -1,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,    -1,   109,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    58,    59,    -1,    -1,    -1,    -1,    64,
      -1,    66,    15,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    84,
      -1,    34,    -1,    -1,    -1,    90,    91,    -1,    -1,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,   109,    58,    59,    -1,    -1,    -1,
      -1,    64,    -1,    66,    15,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    76,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    84,    -1,    34,    -1,    -1,    -1,    90,    91,    -1,
      -1,    94,    -1,    96,    -1,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,    -1,   109,    58,    59,    -1,
      -1,    -1,    -1,    64,    -1,    66,    15,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    76,    -1,    -1,    -1,    80,
      -1,    82,    -1,    84,    -1,    34,    -1,    -1,    -1,    90,
      91,    -1,    -1,    94,    -1,    -1,    -1,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,    -1,   109,    58,
      59,    -1,    -1,    -1,    -1,    64,    -1,    66,    15,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,    76,    -1,    -1,
      -1,    80,    -1,    -1,    -1,    84,    -1,    34,    -1,    -1,
      -1,    90,    91,    -1,    -1,    94,    -1,    -1,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,    -1,
     109,    58,    59,    -1,    -1,    -1,    -1,    64,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    76,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    84,    -1,    -1,
      -1,    -1,    -1,    90,    91,    -1,    -1,    94,    -1,    -1,
      -1,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,    -1,   109,    58,    59,    -1,    -1,    -1,    -1,    64,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    84,
      -1,    -1,    -1,    -1,    -1,    90,    91,    -1,    -1,    94,
      -1,    -1,    -1,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,    -1,   109
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     6,    13,    14,    16,    17,    19,    20,    22,    23,
      24,    25,    26,    27,    28,    30,    31,    32,    33,    35,
      38,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      51,    52,    57,    60,    61,    63,    67,    68,    69,    70,
      73,    77,    78,    89,    92,    93,   102,   103,   107,   108,
     110,   114,   116,   118,   120,   121,   122,   123,   125,   126,
     127,   128,   129,   131,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   160,   161,   164,
     166,   167,   169,   172,   173,   174,   176,   177,   181,   185,
     186,   188,   191,   192,   193,   194,   195,   198,   200,   201,
     202,   204,   205,   206,   208,   209,   210,   219,   220,   221,
     222,   230,   232,   235,   110,   110,   128,   133,   134,   139,
     145,   204,   209,    25,    30,   110,    20,   110,    93,   196,
     197,    20,   139,   196,   127,   128,   130,   134,   139,   145,
     204,   110,   128,    57,   114,   170,   110,   139,   175,    13,
     116,   134,   136,   139,   143,   145,   158,   159,   182,   184,
     201,   204,   209,   210,     8,     9,    13,    19,    44,    71,
      93,   211,   215,   216,   217,   218,   211,   128,    26,    87,
     128,   128,   128,   128,    57,   113,   127,   128,   178,   179,
     132,   139,   145,   176,   201,   204,    57,    97,   117,   128,
     180,   187,   201,   210,    87,    88,   118,   189,   190,     0,
      57,    15,    34,    51,    58,    59,    64,    66,    73,    76,
      80,    84,    90,    91,    94,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   109,    29,    53,    54,    55,
      65,    72,    74,    75,    79,    81,    85,    95,    96,   110,
      56,   110,   110,   110,    13,   139,   115,   124,   125,   215,
     116,    93,   223,    19,   221,   114,     3,     4,     5,     7,
      10,    11,    12,   233,   234,   128,   128,   207,   110,   125,
     180,   183,   130,    56,    56,   139,   114,   115,   121,   139,
     149,   171,   210,    97,    99,   139,    97,    57,    67,   110,
     110,   128,   215,   128,   179,   113,    82,   111,   113,   113,
     180,   115,   180,   187,    97,   180,   187,   117,   117,   215,
     189,   117,   125,   118,   189,   121,   128,   128,   149,   128,
     128,   128,   128,   128,   128,   128,    77,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   116,   186,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   202,   128,   201,   203,   207,    93,   203,   207,
     203,   203,   110,    96,   110,   115,    57,   207,   110,   170,
     224,    57,    60,    68,   115,   219,   225,   226,   227,   228,
     231,   232,   115,   121,    97,    96,    86,    97,    97,   125,
     180,   113,    93,   104,   113,    18,    57,   199,   115,   113,
     113,   139,    82,   182,   184,    53,    54,    55,    58,    59,
      64,    65,    66,    72,    73,    74,    75,    79,    80,    81,
      84,    85,    90,    94,    95,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   109,   110,   116,   212,   214,
     103,   107,   213,   128,   128,   180,   187,    97,   117,   117,
     117,    99,   113,    86,   113,   113,   113,   207,   128,   207,
     124,    83,   117,   196,   225,    97,    97,   225,   225,   225,
     225,   114,   115,   234,   128,   158,   128,   183,   180,   125,
     162,   170,   114,   165,   180,   110,   199,    57,   199,   170,
     170,   139,   144,   113,    56,    83,   117,   113,   113,   113,
      97,   112,   187,   187,   128,   158,    86,    86,   128,   113,
      39,    40,    50,   229,   229,   115,   121,   113,   113,   183,
     180,    21,   163,   115,   121,   115,   121,   114,   139,   199,
     115,   121,   121,    82,   117,    56,   112,   179,   128,   158,
     158,   224,   115,   170,   113,   113,   183,   161,   162,   115,
     115,   115,   121,   139,   115,   115,   144,    56,    93,   113,
     115,   121,   168,   170,   170,   113,   115,   113,    83,   117,
     115,   168,   168,   170,   170,   168,   115,   121,   199,   115,
     199
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   119,   120,   121,   122,   122,   122,   123,   123,   123,
     123,   123,   124,   124,   124,   125,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   127,   128,
     129,   129,   129,   129,   129,   129,   129,   129,   130,   130,
     131,   132,   132,   132,   132,   132,   133,   134,   135,   135,
     135,   135,   135,   135,   135,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   137,   138,   139,   140,   140,   140,
     140,   141,   142,   142,   143,   144,   144,   145,   145,   145,
     145,   146,   146,   146,   146,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   150,   151,   151,   151,
     152,   153,   154,   155,   156,   156,   156,   156,   156,   156,
     156,   157,   157,   157,   157,   158,   158,   158,   158,   158,
     158,   159,   159,   159,   160,   161,   161,   162,   162,   163,
     163,   164,   165,   165,   165,   165,   166,   167,   167,   167,
     167,   168,   168,   169,   170,   170,   171,   171,   172,   172,
     172,   172,   173,   173,   174,   174,   175,   175,   176,   176,
     176,   176,   177,   177,   178,   178,   179,   179,   179,   180,
     180,   181,   182,   182,   183,   183,   184,   184,   185,   186,
     186,   186,   187,   187,   187,   187,   187,   187,   188,   188,
     188,   189,   189,   189,   189,   190,   190,   191,   191,   192,
     193,   193,   194,   195,   196,   196,   197,   197,   198,   198,
     198,   198,   199,   199,   199,   199,   200,   201,   201,   202,
     202,   202,   203,   203,   204,   204,   204,   204,   205,   206,
     206,   206,   206,   206,   207,   207,   208,   208,   208,   208,
     209,   209,   210,   210,   211,   211,   211,   211,   212,   212,
     212,   213,   213,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   215,   215,   215,   215,
     215,   216,   217,   217,   218,   219,   220,   220,   220,   221,
     221,   222,   222,   222,   222,   223,   223,   224,   225,   225,
     225,   225,   225,   225,   226,   227,   227,   228,   228,   229,
     229,   229,   230,   230,   231,   231,   232,   232,   233,   233,
     233,   233,   233,   233,   233,   234,   234,   235,   235
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     3,     1,     1,     1,
       1,     1,     1,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       7,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     6,     1,     1,     1,     1,     1,
       1,     3,     1,     3,     4,     2,     3,     1,     1,     1,
       2,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     5,     3,     3,     3,
       1,     1,     1,     1,     2,     3,     1,     3,     3,     4,
       1,     1,     1,     2,     1,     1,     1,     2,     2,     2,
       2,     5,     5,     6,     6,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     5,     6,     3,     2,     2,
       2,     5,     3,     4,     2,     3,     1,    10,     9,     9,
       8,     2,     1,     1,     2,     1,     1,     1,     7,     6,
       7,     3,     3,     2,     3,     1,     3,     1,     1,     1,
       1,     2,     3,     2,     1,     2,     5,     2,     3,     2,
       1,     1,     3,     1,     3,     1,     3,     1,     3,     3,
       4,     2,     3,     4,     4,     2,     1,     2,     3,     2,
       2,     2,     2,     1,     1,     3,     2,     1,     1,     1,
       1,     2,     2,     2,     3,     1,     1,     3,     5,     4,
       6,     5,     8,     9,     7,     8,     2,     2,     1,     2,
       2,     2,     1,     1,     4,     4,     4,     4,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     1,     1,
       5,     4,     3,     4,     1,     4,     4,     6,     1,     1,
       1,     1,     1,     1,     2,     2,     5,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     1,     2,     1,     2,
       1,     1,     1,     1,     1,     5,     2,     1,     2,     2,
       2,     2,     2,     1,     1,     1,     3,     1,     3,     1,
       1,     1,     4,     3,     4,     3,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     2,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 261 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    sParseHead = (yyvsp[0].node);

    if ((yyvsp[0].node) == NULL)
    {
        sParseHead = dcNil_createNode();
    }
}
#line 2789 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 275 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcGraphData_setNext((yyvsp[-2].node), (yyvsp[0].node));
}
#line 2797 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 284 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = NULL;
}
#line 2805 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 291 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcGraphData_setNext((yyvsp[-2].node), (yyvsp[0].node));
}
#line 2813 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 311 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcArray *array = dcArrayClass_getObjects((yyvsp[0].node));
    dcArrayClass_clearObjects((yyvsp[0].node));
    dcNode_free(&(yyvsp[0].node), DC_DEEP);
    (yyval.node) = dcIn_createNode((yyvsp[-2].node), array);
}
#line 2824 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 332 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcIf_createNode((yyvsp[-5].node),
                         (yyvsp[-3].node),
                         dcIf_createNode(dcTrue_createNode(), (yyvsp[-1].node), NULL));
}
#line 2834 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 345 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcParser_createFunctionFromGuts((yyvsp[-1].node));
}
#line 2842 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 350 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    if ((yyvsp[0].node) == NULL)
    {
        dcAbort("NULL graph data in parser");
    }
}
#line 2853 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 376 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcEquationClass_createObject((yyvsp[-3].node), (yyvsp[-1].node));
}
#line 2861 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 381 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNil_createNode();
}
#line 2869 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 386 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
}
#line 2876 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 390 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcIdentifier_createNode((yyvsp[0].taffyString)->string, NO_FLAGS);
    dcString_free(&(yyvsp[0].taffyString), DC_DEEP);
}
#line 2885 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 395 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcIdentifier_createNode((yyvsp[0].string), SCOPE_DATA_INSTANCE);
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 2894 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 400 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcIdentifier_createNode((yyvsp[0].string), SCOPE_DATA_META);
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 2903 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 405 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcIdentifier_createNode((yyvsp[0].string), NO_FLAGS);
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 2912 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 411 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcString_append((yyvsp[-2].taffyString), ".%s", (yyvsp[0].string));
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 2921 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 417 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.taffyString) = dcString_createWithString((yyvsp[0].string), false);
}
#line 2929 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 421 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcString_append((yyvsp[-2].taffyString), ".%s", (yyvsp[0].string));
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 2938 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 427 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[-2].node);
}
#line 2946 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 432 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[-1].node);
}
#line 2954 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 436 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[-2].node);
}
#line 2962 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 444 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-1].node), dcSystem_getOperatorName(TAFFY_FACTORIAL), NULL);
}
#line 2971 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 450 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[0].node), dcSystem_getOperatorName(TAFFY_PREFIX_PLUS), NULL);
}
#line 2980 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 455 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[0].node);
}
#line 2988 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 459 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcNode *right = (yyvsp[0].node);

    if (dcNumberClass_isMe(right))
    {
        // is like: -3, so perform the multiply right now
        (yyval.node) = dcNumberClass_inlineMultiply
            (right, dcNumberClass_getNegativeOneNumberObject());
    }
    else if (IS_FLAT_ARITHMETIC(right)
             && CAST_FLAT_ARITHMETIC(right)->taffyOperator == TAFFY_MULTIPLY
             && (dcNumberClass_isMe
                 (dcList_getHead(dcFlatArithmetic_getValues(right)))))
    {
        // is like: -3x, so perform the multiply right now
        dcNumberClass_inlineMultiply
            (dcList_getHead(dcFlatArithmetic_getValues(right)),
             dcNumberClass_getNegativeOneNumberObject());
        (yyval.node) = right;
    }
    else
    {
        // defer the multiply until evaluation time
        (yyval.node) = (dcFlatArithmetic_createNodeWithValues
              (TAFFY_MULTIPLY,
               dcNode_setTemplate
               (dcNumberClass_createObjectFromInt32s(-1), true),
               right,
               NULL));
    }
}
#line 3024 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 491 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[0].node), dcSystem_getOperatorName(TAFFY_BIT_NOT), NULL);
}
#line 3033 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 497 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_ADD, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3041 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 501 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_SUBTRACT, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3049 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 505 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_MULTIPLY, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3057 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 509 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_DIVIDE, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3065 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 513 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_RAISE, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3073 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 517 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_BIT_XOR, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3081 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 521 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_MODULUS, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3089 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 525 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_BIT_OR, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3097 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 529 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_BIT_AND, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3105 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 533 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_LEFT_SHIFT, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3113 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 537 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFlatArithmetic_createNodeWithValues(TAFFY_RIGHT_SHIFT, (yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3121 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 542 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_PLUS_EQUAL), (yyvsp[0].node));
}
#line 3130 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 547 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_MINUS_EQUAL), (yyvsp[0].node));
}
#line 3139 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 552 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_RIGHT_SHIFT_EQUAL), (yyvsp[0].node));
}
#line 3148 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 557 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_LEFT_SHIFT_EQUAL), (yyvsp[0].node));
}
#line 3157 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 562 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_MULTIPLY_EQUAL), (yyvsp[0].node));
}
#line 3166 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 567 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_DIVIDE_EQUAL), (yyvsp[0].node));
}
#line 3175 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 572 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_RAISE_EQUAL), (yyvsp[0].node));
}
#line 3184 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 577 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_MODULUS_EQUAL), (yyvsp[0].node));
}
#line 3193 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 582 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_BIT_XOR_EQUAL), (yyvsp[0].node));
}
#line 3202 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 587 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_BIT_OR_EQUAL), (yyvsp[0].node));
}
#line 3211 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 592 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_BIT_AND_EQUAL), (yyvsp[0].node));
}
#line 3220 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 597 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-1].node), dcSystem_getOperatorName(TAFFY_PLUS_PLUS), NULL);
}
#line 3229 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 602 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-1].node), dcSystem_getOperatorName(TAFFY_MINUS_MINUS), NULL);
}
#line 3238 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 608 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_LESS_THAN), (yyvsp[0].node));
}
#line 3247 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 613 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_LESS_THAN_OR_EQUAL), (yyvsp[0].node));
}
#line 3256 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 618 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_GREATER_THAN), (yyvsp[0].node));
}
#line 3265 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 623 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_GREATER_THAN_OR_EQUAL), (yyvsp[0].node));
}
#line 3274 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 628 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    if (IS_FLAT_ARITHMETIC((yyvsp[-1].node)))
    {
        CAST_FLAT_ARITHMETIC((yyvsp[-1].node))->grouped = true;
    }

    (yyval.node) = (yyvsp[-1].node);
}
#line 3287 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 114:
#line 637 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument
        ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_EQUALS), (yyvsp[0].node));
}
#line 3296 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 115:
#line 642 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (dcMethodCall_createNode
          ((yyvsp[-2].node),
           dcSystem_getOperatorName(TAFFY_DELTA_EQUALS),
           dcList_createWithObjects
           (dcNode_setTemplate
            (dcArrayClass_createObject
             (dcArray_createWithObjects
              ((yyvsp[0].node),
               dcNode_setTemplate
               (dcNode_copy(dcNumberMetaClass_getDefaultDelta(),
                            DC_DEEP),
                true),
               NULL),
              false),
             true),
            NULL)));
}
#line 3319 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 116:
#line 661 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (dcMethodCall_createNode
          ((yyvsp[-4].node),
           dcSystem_getOperatorName(TAFFY_DELTA_EQUALS),
           dcList_createWithObjects
           (dcNode_setTemplate(dcArrayClass_createObject
                               (dcArray_createWithObjects
                                ((yyvsp[0].node),
                                 dcNode_setTemplate
                                 (dcNumberClass_createObject((yyvsp[-2].number)),
                                  true),
                                 NULL),
                                false),
                               true),
            NULL)));
}
#line 3340 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 117:
#line 678 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNotEqualCall_createNode
        (dcMethodCall_createNodeWithArgument
         ((yyvsp[-2].node), dcSystem_getOperatorName(TAFFY_EQUALS), (yyvsp[0].node)));
}
#line 3350 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 118:
#line 684 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcAnd_createNode((yyvsp[-2].node), (yyvsp[0].node));
}
#line 3358 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 119:
#line 688 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcOr_createNode((yyvsp[-2].node), (yyvsp[0].node));
}
#line 3366 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 120:
#line 692 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcTrue_createNode();
}
#line 3374 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 121:
#line 696 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcTrue_createNode();
}
#line 3382 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 122:
#line 700 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFalse_createNode();
}
#line 3390 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 123:
#line 704 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFalse_createNode();
}
#line 3398 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 124:
#line 708 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcMethodCall_createNodeWithArgument((yyvsp[0].node), "#prefixOperator(!)", NULL);
}
#line 3406 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 125:
#line 712 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[-1].node);
}
#line 3414 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 127:
#line 719 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcAssignment_createNode((yyvsp[-2].node), (yyvsp[0].node), NO_FLAGS);
}
#line 3422 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 128:
#line 723 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcAssignment_createNode((yyvsp[-2].node), (yyvsp[0].node), NO_FLAGS);
}
#line 3430 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 129:
#line 727 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcAssignment_createNode((yyvsp[-2].node), (yyvsp[0].node), (yyvsp[-3].iValue));
}
#line 3438 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 130:
#line 732 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcSelf_createNode();
}
#line 3446 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 131:
#line 737 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcUpSelf_createNode();
}
#line 3454 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 132:
#line 742 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcSuper_createNode();
}
#line 3462 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 133:
#line 747 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNew_createNode((yyvsp[0].node));
}
#line 3470 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 134:
#line 752 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_CONSTANT;
}
#line 3478 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 135:
#line 756 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_GLOBAL;
}
#line 3486 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 136:
#line 760 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_LOCAL;
}
#line 3494 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 137:
#line 764 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_CONSTANT | SCOPE_DATA_GLOBAL;
}
#line 3502 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 138:
#line 768 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_GLOBAL | SCOPE_DATA_CONSTANT;
}
#line 3510 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 139:
#line 772 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_CONSTANT | SCOPE_DATA_LOCAL;
}
#line 3518 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 140:
#line 776 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_LOCAL | SCOPE_DATA_CONSTANT;
}
#line 3526 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 141:
#line 788 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = createFunction((yyvsp[-4].node), NO_FLAGS, (yyvsp[-2].list), (yyvsp[0].node));
}
#line 3534 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 142:
#line 796 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = createFunction(dcIdentifier_createNode((yyvsp[-4].string), NO_SCOPE_DATA_FLAGS),
                        NO_FLAGS,
                        (yyvsp[-2].list),
                        (yyvsp[0].node));
    dcMemory_trackMemory((yyvsp[-4].string));
}
#line 3546 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 143:
#line 813 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = createFunction((yyvsp[-4].node), (yyvsp[-5].iValue), (yyvsp[-2].list), (yyvsp[0].node));
}
#line 3554 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 144:
#line 822 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = createFunction(dcIdentifier_createNode((yyvsp[-4].string), NO_SCOPE_DATA_FLAGS),
                        (yyvsp[-5].iValue),
                        (yyvsp[-2].list),
                        (yyvsp[0].node));
    dcMemory_trackMemory((yyvsp[-4].string));
}
#line 3566 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 155:
#line 844 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcIf_createNode((yyvsp[-2].node), (yyvsp[0].node), NULL);
}
#line 3574 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 156:
#line 848 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcIf_createNode((yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node));
}
#line 3582 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 157:
#line 853 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[-1].node);
}
#line 3590 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 158:
#line 857 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNil_createNode();
}
#line 3598 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 159:
#line 862 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcIf_createNode(dcTrue_createNode(), (yyvsp[0].node), NULL);
}
#line 3606 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 160:
#line 866 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[0].node);
}
#line 3614 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 161:
#line 871 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcSynchronized_createNode((yyvsp[-2].node), (yyvsp[0].node));
}
#line 3622 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 162:
#line 876 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[-1].node);
}
#line 3630 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 163:
#line 880 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[-1].node);
}
#line 3638 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 164:
#line 884 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNil_createNode();
}
#line 3646 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 165:
#line 888 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNil_createNode();
}
#line 3654 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 166:
#line 893 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[0].node);
}
#line 3662 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 167:
#line 902 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcFor_createNode(dcList_createShell((yyvsp[-7].list)),
                          (yyvsp[-5].node),
                          dcList_createShell((yyvsp[-3].list)),
                          (yyvsp[0].node));
}
#line 3673 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 168:
#line 913 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (dcFor_createNode
          (dcList_createNodeWithObjects(dcNil_createNode(), NULL),
           (yyvsp[-5].node),
           dcList_createShell((yyvsp[-3].list)),
           (yyvsp[0].node)));
}
#line 3685 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 169:
#line 924 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (dcFor_createNode
          (dcList_createShell((yyvsp[-6].list)),
           (yyvsp[-4].node),
           dcList_createNodeWithObjects(dcNil_createNode(), NULL),
           (yyvsp[0].node)));
}
#line 3697 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 170:
#line 935 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (dcFor_createNode
          (dcList_createNodeWithObjects(dcNil_createNode(), NULL),
           (yyvsp[-4].node),
           dcList_createNodeWithObjects(dcNil_createNode(), NULL),
           (yyvsp[0].node)));
}
#line 3709 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 171:
#line 944 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[-1].node);
}
#line 3717 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 172:
#line 948 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNil_createNode();
}
#line 3725 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 178:
#line 959 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcWhile_createNode((yyvsp[-4].node), (yyvsp[-1].node));
}
#line 3733 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 179:
#line 963 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcWhile_createNode((yyvsp[-3].node), dcNil_createNode());
}
#line 3741 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 180:
#line 967 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcWhile_createNode((yyvsp[-4].node), (yyvsp[-1].node));
}
#line 3749 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 181:
#line 971 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcWhile_createNode((yyvsp[0].node), (yyvsp[-2].node));
}
#line 3757 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 182:
#line 976 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcNode *procedure = (dcClass_castNodeWithAssert
                         ((yyvsp[-2].node),
                          dcProcedureClass_getTemplate(),
                          false,
                          true));
    dcProcedureClass_setBody(procedure, dcGraphDataTree_createNode((yyvsp[-1].node)));
}
#line 3770 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 183:
#line 985 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcNode *procedure = (dcClass_castNodeWithAssert
                         ((yyvsp[-1].node),
                          dcProcedureClass_getTemplate(),
                          false,
                          true));
    dcProcedureClass_setBody(procedure,
                             dcGraphDataTree_createNode(dcNil_createNode()));
}
#line 3784 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 184:
#line 996 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate(dcBlockClass_createObject(NULL, (yyvsp[-1].list)), true);
}
#line 3792 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1000 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate(dcBlockClass_createObject(NULL, NULL), true);
}
#line 3800 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1005 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_push((yyvsp[-2].list), (yyvsp[0].node));
}
#line 3808 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1009 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects((yyvsp[0].node), NULL);
}
#line 3816 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1014 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate(dcNumberClass_createObject((yyvsp[0].number)), true);
}
#line 3824 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 189:
#line 1018 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate(dcComplexNumberClass_createObject((yyvsp[0].complexNumber)), true);
}
#line 3832 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 190:
#line 1022 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (dcNode_setTemplate
          (dcComplexNumberClass_createObject
           (dcComplexNumber_create(NULL, dcNumber_createFromInt32u(1))),
           true));
}
#line 3843 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 191:
#line 1029 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (dcNode_setTemplate
          (dcComplexNumberClass_createObject
           (dcComplexNumber_create(NULL, (yyvsp[-1].number))),
           true));
}
#line 3854 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 192:
#line 1037 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcNode *keysNode = dcList_get((yyvsp[-1].list), 0);
    dcNode *valuesNode = dcList_get((yyvsp[-1].list), 1);

    (yyval.node) = dcNode_setTemplate
        (dcHashClass_createUninitializedObject(CAST_LIST(keysNode),
                                               CAST_LIST(valuesNode)),
         true);
    dcList_pop((yyvsp[-1].list), DC_FLOATING);
    dcList_pop((yyvsp[-1].list), DC_FLOATING);
    dcNode_freeShell(&keysNode);
    dcNode_freeShell(&valuesNode);
    dcList_free(&(yyvsp[-1].list), DC_DEEP);
}
#line 3873 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 193:
#line 1052 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate
        (dcHashClass_createInitializedObject(dcHash_create()),
         true);
}
#line 3883 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1059 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = (yyvsp[0].list);
}
#line 3891 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1063 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = (yyvsp[-1].list);
}
#line 3899 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 196:
#line 1068 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift(CAST_LIST(dcList_get((yyvsp[0].list), 0)), (yyvsp[-4].node));
    dcList_unshift(CAST_LIST(dcList_get((yyvsp[0].list), 1)), (yyvsp[-2].node));
    (yyval.list) = (yyvsp[0].list);
}
#line 3909 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 197:
#line 1074 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = (yyvsp[0].list);
}
#line 3917 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 198:
#line 1078 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects(dcNode_setTemplate
                                  (dcList_createNodeWithObjects((yyvsp[-2].node), NULL),
                                   true),
                                  dcNode_setTemplate
                                  (dcList_createNodeWithObjects((yyvsp[0].node), NULL),
                                   true),
                                  NULL);
}
#line 3931 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 201:
#line 1091 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcSymbol_createNode((yyvsp[0].string));
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 3940 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 202:
#line 1097 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), (yyvsp[-2].node));
    (yyval.list) = (yyvsp[0].list);
}
#line 3949 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 203:
#line 1102 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects((yyvsp[0].node), NULL);
}
#line 3957 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 204:
#line 1107 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), (yyvsp[-2].node));
    (yyval.list) = (yyvsp[0].list);
}
#line 3966 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 205:
#line 1112 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects((yyvsp[0].node), NULL);
}
#line 3974 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 206:
#line 1117 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), dcNode_setTemplate(dcList_createShell((yyvsp[-2].list)), true));
    (yyval.list) = (yyvsp[0].list);
}
#line 3983 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 207:
#line 1122 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects
        (dcNode_setTemplate(dcList_createShell((yyvsp[0].list)), true), NULL);
}
#line 3992 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 208:
#line 1128 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcMatrix *matrix = dcMatrix_createFromLists((yyvsp[-1].list));

    if (matrix == NULL)
    {
        yyerror(NULL);
        dcLexer_setErrorString(sLexer, "malformed matrix");
        YYABORT;
    }
    else
    {
        TAFFY_DEBUG(dcMatrix_assertIsTemplate(matrix));
        dcList_free(&(yyvsp[-1].list), DC_SHALLOW);
        (yyval.node) = dcNode_setTemplate(dcMatrixClass_createObject
                                (matrix, false),
                                true);
    }
}
#line 4015 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 209:
#line 1148 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate
        (dcArrayClass_createObject(dcArray_createFromList((yyvsp[-1].list), DC_SHALLOW),
                                   false),
         true);
    dcList_free(&(yyvsp[-1].list), DC_SHALLOW);
}
#line 4027 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 210:
#line 1156 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate
        (dcArrayClass_createObject(dcArray_createFromList((yyvsp[-1].list), DC_SHALLOW),
                                   false),
         true);
    dcList_free(&(yyvsp[-1].list), DC_SHALLOW);
}
#line 4039 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 211:
#line 1164 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate(dcArrayClass_createEmptyObject(), true);
}
#line 4047 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 212:
#line 1169 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), (yyvsp[-2].node));
    (yyval.list) = (yyvsp[0].list);
}
#line 4056 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 213:
#line 1174 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), (yyvsp[-3].node));
    (yyval.list) = (yyvsp[0].list);
}
#line 4065 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 214:
#line 1179 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), (yyvsp[-3].node));
    (yyval.list) = (yyvsp[0].list);
}
#line 4074 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 215:
#line 1184 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), dcNil_createNode());
    (yyval.list) = (yyvsp[0].list);
}
#line 4083 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 216:
#line 1189 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects((yyvsp[0].node), NULL);
}
#line 4091 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 217:
#line 1193 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects((yyvsp[-1].node), NULL);
}
#line 4099 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 218:
#line 1198 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate
        (dcStringClass_createObjectFromList((yyvsp[-1].list), false), true);
}
#line 4108 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 219:
#line 1203 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate
        (dcStringClass_createObjectFromList(dcList_createWithObjects(NULL),
                                            false),
         true);
}
#line 4119 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 220:
#line 1210 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate(dcStringClass_createObject((yyvsp[0].string), false), true);
}
#line 4127 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 221:
#line 1215 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), dcString_createNodeWithString((yyvsp[-1].string), false));
    (yyval.list) = (yyvsp[0].list);
}
#line 4136 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 222:
#line 1220 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), (yyvsp[-1].node));
    (yyval.list) = (yyvsp[0].list);
}
#line 4145 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 223:
#line 1225 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects
        (dcString_createNodeWithString((yyvsp[0].string), false), NULL);
}
#line 4154 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 224:
#line 1230 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects((yyvsp[0].node), NULL);
}
#line 4162 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 225:
#line 1235 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (yyvsp[-1].node);
}
#line 4170 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 226:
#line 1239 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNil_createNode();
}
#line 4178 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 229:
#line 1246 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcBreak_createNode();
}
#line 4186 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 230:
#line 1251 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcReturn_createNode(NULL);
}
#line 4194 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 231:
#line 1255 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcReturn_createNode((yyvsp[0].node));
}
#line 4202 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 232:
#line 1260 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcPackage_createNode((yyvsp[0].list));
    sPackageName = dcPackage_getPathString(CAST_PACKAGE((yyval.node)));
    dcMemory_trackMemory(sPackageName);
}
#line 4212 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 233:
#line 1267 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcImport_createNode((yyvsp[0].list));
}
#line 4220 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 234:
#line 1272 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_push((yyvsp[-2].list), dcString_createNodeWithString((yyvsp[0].string), false));
}
#line 4228 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 235:
#line 1276 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects
        (dcString_createNodeWithString((yyvsp[0].string), false), NULL);
}
#line 4237 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 236:
#line 1282 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
}
#line 4244 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 237:
#line 1285 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_push((yyvsp[-2].list), dcString_createNodeWithString("*", true));
}
#line 4252 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 238:
#line 1290 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcTryBlock_createNode((yyvsp[-2].node), (yyvsp[0].list));
}
#line 4260 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 239:
#line 1294 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcTryBlock_createNode(dcNil_createNode(), (yyvsp[0].list));
}
#line 4268 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 240:
#line 1298 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcTryBlock_createNode((yyvsp[-3].node), (yyvsp[0].list));
}
#line 4276 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 241:
#line 1302 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcTryBlock_createNode(dcNil_createNode(), (yyvsp[0].list));
}
#line 4284 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 242:
#line 1307 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects(dcCatchBlock_createNode((yyvsp[-4].node), (yyvsp[-5].node), (yyvsp[-1].node)), NULL);
}
#line 4292 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 243:
#line 1311 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), dcCatchBlock_createNode((yyvsp[-5].node), (yyvsp[-6].node), (yyvsp[-2].node)));
    (yyval.list) = (yyvsp[0].list);
}
#line 4301 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 244:
#line 1316 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects
        (dcCatchBlock_createNode((yyvsp[-3].node), (yyvsp[-4].node), dcNil_createNode()),
         NULL);
}
#line 4311 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1322 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), dcCatchBlock_createNode((yyvsp[-4].node), (yyvsp[-5].node), dcNil_createNode()));
    (yyval.list) = (yyvsp[0].list);
}
#line 4320 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 246:
#line 1328 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcThrow_createNode((yyvsp[0].node));
}
#line 4328 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1334 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcString *methodNameString = dcString_create();
    dcList *methodArgumentList = dcList_create();

    dcParser_extractAndClearMethodParameterListData((yyvsp[0].list),
                                                    methodNameString,
                                                    methodArgumentList);
    (yyval.node) = dcNode_setTemplate(dcMethodCall_createNode((yyvsp[-1].node),
                                                    methodNameString->string,
                                                    methodArgumentList),
                            true);
    dcGraphData_copyPosition((yyvsp[-1].node), (yyval.node));
    dcList_free(&(yyvsp[0].list), DC_SHALLOW);
    dcString_free(&methodNameString, DC_DEEP);
}
#line 4348 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 249:
#line 1352 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (dcParser_createParenthesesOperatorFunctionCall
          (dcIdentifier_createNode((yyvsp[-1].string), NO_SCOPE_DATA_FLAGS),
           dcArray_createWithObjects(dcParser_createFunctionFromGuts((yyvsp[0].node)),
                                     NULL)));
    dcMemory_trackMemory((yyvsp[-1].string));
}
#line 4360 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 250:
#line 1360 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (dcParser_createParenthesesOperatorFunctionCall
          (dcIdentifier_createNode((yyvsp[-1].string), NO_SCOPE_DATA_FLAGS),
           dcArray_createWithObjects((yyvsp[0].node), NULL)));
    dcMemory_trackMemory((yyvsp[-1].string));
}
#line 4371 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 251:
#line 1367 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = (dcParser_createParenthesesOperatorFunctionCall
          (dcIdentifier_createNode((yyvsp[-1].string), NO_SCOPE_DATA_FLAGS),
           dcArray_createWithObjects(dcParser_createFunctionFromGuts((yyvsp[0].node)),
                                     NULL)));
    dcMemory_trackMemory((yyvsp[-1].string));
}
#line 4383 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 253:
#line 1377 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects((yyvsp[0].node), NULL);
}
#line 4391 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 254:
#line 1382 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = createFunctionCall((yyvsp[-3].node), (yyvsp[-1].list));
}
#line 4399 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 255:
#line 1386 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = createFunctionCall((yyvsp[-3].node), (yyvsp[-1].list));
}
#line 4407 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 256:
#line 1390 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = createFunctionCall((yyvsp[-3].node), (yyvsp[-1].list));
}
#line 4415 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 257:
#line 1394 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = createFunctionCall((yyvsp[-3].node), (yyvsp[-1].list));
}
#line 4423 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 264:
#line 1407 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_push((yyvsp[-2].list), (yyvsp[0].node));
}
#line 4431 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 265:
#line 1411 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects((yyvsp[0].node), NULL);
}
#line 4439 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 270:
#line 1422 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_push((yyvsp[-2].list), (yyvsp[0].node));
    dcArray *array = dcArray_createFromList((yyvsp[-2].list), DC_SHALLOW);
    dcList_free(&(yyvsp[-2].list), DC_SHALLOW);

    (yyval.node) = dcMethodCall_createNode
        ((yyvsp[-4].node),
         dcSystem_getOperatorName(array->size == 2
                                  ? TAFFY_BRACKETS_EQUAL
                                  : TAFFY_BRACKETS_DOTS_EQUAL),
         dcList_createWithObjects
         (dcNode_setTemplate
          (dcArrayClass_createObject(array, false),
           true), // template
          NULL)); // no more objects
}
#line 4460 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 271:
#line 1439 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    //
    // operator [...]
    //
    dcList *list = (yyvsp[-1].list);

    if (list->size == 1)
    {
        (yyval.node) = dcMethodCall_createNode
            ((yyvsp[-3].node), dcSystem_getOperatorName(TAFFY_BRACKETS), list);
    }
    else
    {
        dcArray *array = dcArray_createFromList((yyvsp[-1].list), DC_SHALLOW);
        dcList_free(&(yyvsp[-1].list), DC_SHALLOW);
        (yyval.node) = dcMethodCall_createNode
            ((yyvsp[-3].node),
             dcSystem_getOperatorName(TAFFY_BRACKETS_DOTS),
             dcList_createWithObjects
             (dcNode_setTemplate
              (dcArrayClass_createObject(array, false),
               true),
              NULL)); // no more objects
    }
}
#line 4490 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 272:
#line 1466 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcList_createGraphDataNodeWithObjects((yyvsp[-1].node), NULL);
}
#line 4498 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 273:
#line 1470 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcString *methodNameString = dcString_create();
    dcList *methodArgumentList = dcList_create();

    dcParser_extractAndClearMethodParameterListData
        ((yyvsp[-1].list), methodNameString, methodArgumentList);

    // the receiver will be evaluated //
    dcNode *methodCall = dcMethodCall_createNode
        (NULL, methodNameString->string, methodArgumentList);

    dcGraphData_copyPosition((yyvsp[-2].node), methodCall);
    dcString_free(&methodNameString, DC_DEEP);
    dcList_push(CAST_GRAPH_DATA_LIST((yyvsp[-2].node)), methodCall);
    dcList_free(&(yyvsp[-1].list), DC_SHALLOW);

    (yyval.node) = (yyvsp[-2].node);
}
#line 4521 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 274:
#line 1490 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    //
    // a 'normal' method header, like:
    //
    // (@) objectAtIndex: index
    // {
    // }
    //

    dcString *methodNameString = dcString_create();
    dcList *methodArgumentList = dcList_create();

    dcParser_extractAndClearMethodParameterListData
        ((yyvsp[0].list), methodNameString, methodArgumentList);

    (yyval.methodHeader) = dcMethodHeader_create(methodNameString->string, methodArgumentList);
    dcString_free(&methodNameString, DC_DEEP);
    dcList_free(&(yyvsp[0].list), DC_SHALLOW);
}
#line 4545 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 275:
#line 1510 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    //
    // an operator method header, like:
    //
    // (@) #operator(++)
    // {
    // }
    //
    char *methodName = dcLexer_sprintf("#operator(%s)", (yyvsp[-1].string));
    (yyval.methodHeader) = dcMethodHeader_create(methodName, NULL);
    dcMemory_trackMemory(methodName);
}
#line 4562 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 276:
#line 1523 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    //
    // a prefix operator method header, like:
    //
    // (@) #prefixOperator(-)
    // {
    // }
    //
    char *methodName = dcLexer_sprintf("#prefixOperator(%s)", (yyvsp[-1].string));
    (yyval.methodHeader) = dcMethodHeader_create(methodName, NULL);
    dcMemory_trackMemory(methodName);
}
#line 4579 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 277:
#line 1536 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    //
    // a argument_operator method header, like (value is the argument):
    //
    // (@) #operator(+): value
    // {
    // }
    //
    char *methodName = dcLexer_sprintf("#operator(%s):", (yyvsp[-3].string));
    (yyval.methodHeader) = dcMethodHeader_create
        (methodName,
         dcList_createWithObjects
         (dcIdentifier_createNode((yyvsp[0].string), NO_FLAGS), NULL));
    dcMemory_trackMemory((yyvsp[0].string));
    dcMemory_trackMemory(methodName);
}
#line 4600 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 278:
#line 1554 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "!";
}
#line 4608 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 279:
#line 1558 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "++";
}
#line 4616 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 280:
#line 1562 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "--";
}
#line 4624 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 281:
#line 1567 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "!";
}
#line 4632 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 282:
#line 1571 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "-";
}
#line 4640 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 283:
#line 1576 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "==";
}
#line 4648 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 284:
#line 1580 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "()";
}
#line 4656 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 285:
#line 1584 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "[]";
}
#line 4664 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 286:
#line 1588 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "[...]";
}
#line 4672 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 287:
#line 1592 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "[...]=";
}
#line 4680 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 288:
#line 1596 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "<";
}
#line 4688 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 289:
#line 1600 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "<=";
}
#line 4696 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 290:
#line 1604 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = ">";
}
#line 4704 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 291:
#line 1608 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = ">=";
}
#line 4712 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 292:
#line 1612 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "+";
}
#line 4720 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 293:
#line 1616 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "-";
}
#line 4728 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 294:
#line 1620 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "*";
}
#line 4736 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 295:
#line 1624 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "/";
}
#line 4744 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 296:
#line 1628 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "~=";
}
#line 4752 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 297:
#line 1632 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "<<";
}
#line 4760 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 298:
#line 1636 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = ">>";
}
#line 4768 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 299:
#line 1640 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "^";
}
#line 4776 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 300:
#line 1644 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "^^";
}
#line 4784 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 301:
#line 1648 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "^^=";
}
#line 4792 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 302:
#line 1652 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "%";
}
#line 4800 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 303:
#line 1656 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "%=";
}
#line 4808 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 304:
#line 1660 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "&";
}
#line 4816 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 305:
#line 1664 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "&=";
}
#line 4824 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 306:
#line 1668 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "|";
}
#line 4832 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 307:
#line 1672 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "|=";
}
#line 4840 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 308:
#line 1676 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "[]=";
}
#line 4848 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 309:
#line 1680 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "+=";
}
#line 4856 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 310:
#line 1684 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "-=";
}
#line 4864 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 311:
#line 1688 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "*=";
}
#line 4872 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 312:
#line 1692 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "/=";
}
#line 4880 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 313:
#line 1696 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "^=";
}
#line 4888 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 314:
#line 1700 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = "<<=";
}
#line 4896 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 315:
#line 1704 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.string) = ">>=";
}
#line 4904 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 316:
#line 1709 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcList_unshift((yyvsp[0].list), (yyvsp[-1].node));
    (yyval.list) = (yyvsp[0].list);
}
#line 4913 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 317:
#line 1714 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects((yyvsp[0].node), NULL);
}
#line 4921 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 318:
#line 1718 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcNode *left = dcString_createNodeWithString((yyvsp[0].string), false);
    (yyval.list) = dcList_createWithObjects(dcPair_createNode(left, NULL), NULL);
}
#line 4930 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 319:
#line 1723 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcNode *left = dcString_createNodeWithString((yyvsp[0].constString), true);
    (yyval.list) = dcList_createWithObjects(dcPair_createNode(left, NULL), NULL);
}
#line 4939 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 320:
#line 1728 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_createWithObjects(dcPair_createNode
                                  (dcString_createNodeWithString((yyvsp[0].string), false),
                                   NULL),
                                  NULL);
}
#line 4950 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 324:
#line 1739 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcNode_setTemplate
        (dcPair_createNode
         (dcNode_setTemplate(dcString_createNodeWithString((yyvsp[-1].string), true), true),
          dcNode_setTemplate((yyvsp[0].node), true)),
         true);
    dcMemory_trackMemory((yyvsp[-1].string));
}
#line 4963 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 325:
#line 1752 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    MyClassHeader *header = (yyvsp[-1].classHeader);
    dcString *packageName = dcString_createWithString(sPackageName, true);

    //
    // <create> the embedded package, but don't include the current class
    //
    dcListElement *that = NULL;
    // if we don't have a current package, then don't start the string with
    // a dot
    bool doDot = (strcmp(packageName->string, "") == 0
                  ? false
                  : true);

    for (that = sClassNames->head;
         that != NULL && that->next != NULL;
         that = that->next)
    {
        if (doDot)
        {
            dcString_appendCharacter(packageName, '.');
        }

        dcString_append(packageName, "%s", dcString_getString(that->object));
        doDot = true;
    }
    // </create>

    // create the ClassTemplate template
    bool stateSave = dcMemory_pushStateToMalloc();
    dcClassTemplate *classTemplate = (dcClassTemplate_createSimple
                                      (packageName->string,
                                       header->className,
                                       header->superName,
                                       (yyvsp[-2].iValue),
                                       header->scopeDataFlags));
    // add the template to the class manager so it can be eventually freed
    dcClassManager_addClassTemplateTemplate(classTemplate);
    dcMemory_popState(stateSave);
    dcString_free(&packageName, DC_DEEP);

    // create the meta class
    dcNode *result = (dcNode_setTemplate
                      (dcNode_setTemplate
                       (dcClass_createBasicNode(classTemplate, false), true),
                       true));

    dcGraphData_setPosition(CAST_GRAPH_DATA(result),
                            sLexer->filenameId,
                            sLexer->classLineNumberSave);

    // add the class data
    for (that = (yyvsp[0].list)->head; that != NULL; that = that->next)
    {
        // that is a little messy -- we need to convert memory region memory to
        // malloc memory, because the class template lives in global space
        stateSave = dcMemory_pushStateToMalloc();
        dcScopeData *data = CAST_SCOPE_DATA(that->object);
        assert(dcClass_set(result,
                           data->name,
                           dcNode_tryCopy(data->object, DC_DEEP),
                           data->flags,
                           false));
        dcMemory_popState(stateSave);
    }

    // free the class header
    dcMemory_trackMemory(header->className);
    dcMemory_trackMemory(header->superName);
    dcMemory_trackMemory(header);

    dcList_free(&(yyvsp[0].list), DC_DEEP);
    dcList_pop(sClassNames, DC_DEEP);
    dcLexer_popScopeDataFlags(sLexer);
    (yyval.node) = result;
}
#line 5044 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 326:
#line 1830 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = NO_FLAGS;
}
#line 5052 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 327:
#line 1834 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = (yyvsp[-1].iValue);
}
#line 5060 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 328:
#line 1838 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = CLASS_SLICE;
}
#line 5068 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 329:
#line 1843 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = (yyvsp[-1].iValue) | (yyvsp[0].iValue);
}
#line 5076 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 331:
#line 1849 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = CLASS_ABSTRACT;
}
#line 5084 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 332:
#line 1853 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = CLASS_ATOMIC;
}
#line 5092 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 333:
#line 1857 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = CLASS_FINAL;
}
#line 5100 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 334:
#line 1861 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = CLASS_SINGLETON;
}
#line 5108 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 335:
#line 1867 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    // add class name to help with package name for composited classes
    pushClassName((yyvsp[-4].string));

    // TODO: keep class_path a dcList all the way through
    dcPackage *tempPackage = dcPackage_create((yyvsp[-2].list));
    (yyval.classHeader) = createClassHeader((yyvsp[-4].string),
                           dcPackage_getPathString(tempPackage),
                           dcLexer_getCurrentScopeDataFlags(sLexer));
    dcPackage_free(&tempPackage);
    dcLexer_pushScopeDataFlags(sLexer);
}
#line 5125 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 336:
#line 1881 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    pushClassName((yyvsp[-1].string));
    (yyval.classHeader) = createClassHeader((yyvsp[-1].string),
                           dcMemory_strdup(MAKE_FULLY_QUALIFIED(OBJECT)),
                           dcLexer_getCurrentScopeDataFlags(sLexer));
    dcLexer_pushScopeDataFlags(sLexer);
}
#line 5137 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 338:
#line 1892 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = (yyvsp[0].list);
    dcList_unshift((yyvsp[0].list), (yyvsp[-1].node));
}
#line 5146 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 339:
#line 1897 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = (yyvsp[0].list);
    dcList_unshift((yyvsp[0].list), (yyvsp[-1].node));
}
#line 5155 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 340:
#line 1902 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = (yyvsp[0].list);
    dcList_unshift((yyvsp[0].list), (yyvsp[-1].node));
}
#line 5164 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 341:
#line 1907 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = (yyvsp[0].list);
    dcList_unshift((yyvsp[0].list), (yyvsp[-1].node));
}
#line 5173 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 342:
#line 1912 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = (yyvsp[0].list);
}
#line 5181 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 343:
#line 1916 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.list) = dcList_create();
}
#line 5189 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 344:
#line 1921 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcScopeData_createNode(dcClass_getName((yyvsp[0].node)),
                                (yyvsp[0].node),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_OBJECT
                                 | SCOPE_DATA_META));
}
#line 5201 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 345:
#line 1930 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcScopeData_createNode((yyvsp[0].string),
                                dcNil_createNode(),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_INSTANCE
                                 | SCOPE_DATA_OBJECT));
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 5214 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 346:
#line 1939 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcScopeData_createNode((yyvsp[-2].string),
                                dcNil_createNode(),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_INSTANCE
                                 | SCOPE_DATA_OBJECT
                                 | (yyvsp[0].iValue)));
    dcMemory_trackMemory((yyvsp[-2].string));
}
#line 5228 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 347:
#line 1950 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcScopeData_createNode((yyvsp[0].string),
                                dcNil_createNode(),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_META
                                 | SCOPE_DATA_OBJECT));
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 5241 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 348:
#line 1959 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcScopeData_createNode((yyvsp[-2].string),
                                dcNil_createNode(),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_META
                                 | SCOPE_DATA_OBJECT
                                 | (yyvsp[0].iValue)));
    dcMemory_trackMemory((yyvsp[-2].string));
}
#line 5255 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 349:
#line 1970 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_READER;
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 5264 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 350:
#line 1975 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_WRITER;
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 5273 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 351:
#line 1980 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_READER | SCOPE_DATA_WRITER;
    dcMemory_trackMemory((yyvsp[0].string));
}
#line 5282 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 352:
#line 1986 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcNode *methodNode = dcScopeData_getObject((yyvsp[-3].node));
    dcProcedureClass_setBody(methodNode, dcGraphDataTree_createNode((yyvsp[-1].node)));
    dcNode_free(&(yyvsp[-3].node), DC_SHALLOW);
    (yyval.node) = methodNode;
}
#line 5293 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 353:
#line 1993 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcNode *methodNode = dcScopeData_getObject((yyvsp[-2].node));
    dcProcedureClass_setBody(methodNode,
                             dcGraphDataTree_createNode(dcNil_createNode()));
    dcNode_free(&(yyvsp[-2].node), DC_SHALLOW);
    (yyval.node) = methodNode;
}
#line 5305 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 354:
#line 2002 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcProcedureClass_setBody(dcScopeData_getObject((yyvsp[-3].node)),
                             dcGraphDataTree_createNode((yyvsp[-1].node)));
}
#line 5314 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 355:
#line 2007 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcProcedureClass_setBody(dcScopeData_getObject((yyvsp[-2].node)),
                             dcGraphDataTree_createNode(dcNil_createNode()));
}
#line 5323 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 357:
#line 2014 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    dcScopeData_updateFlags(CAST_SCOPE_DATA((yyvsp[-1].node)), (yyvsp[0].iValue));
}
#line 5331 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 358:
#line 2019 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_BREAKTHROUGH;
}
#line 5339 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 359:
#line 2023 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_SYNCHRONIZED;
}
#line 5347 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 360:
#line 2027 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_SYNCHRONIZED_READ;
}
#line 5355 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 361:
#line 2031 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_SYNCHRONIZED_WRITE;
}
#line 5363 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 362:
#line 2035 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_CONST;
}
#line 5371 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 363:
#line 2039 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_CONTAINER_LOOP;
}
#line 5379 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 364:
#line 2043 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = SCOPE_DATA_MODIFIES_CONTAINER;
}
#line 5387 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 365:
#line 2048 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.iValue) = (yyvsp[-2].iValue) | (yyvsp[0].iValue);
}
#line 5395 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 367:
#line 2054 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcScopeData_createNode((yyvsp[0].methodHeader)->name,
                                dcNode_setTemplate
                                (dcProcedureClass_createObject(NULL, (yyvsp[0].methodHeader)), true),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_INSTANCE
                                 | SCOPE_DATA_METHOD));
}
#line 5408 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;

  case 368:
#line 2063 "src/graph/dcParser.y" /* yacc.c:1646  */
    {
    (yyval.node) = dcScopeData_createNode((yyvsp[0].methodHeader)->name,
                                dcNode_setTemplate
                                (dcProcedureClass_createObject(NULL, (yyvsp[0].methodHeader)), true),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_META
                                 | SCOPE_DATA_METHOD));
}
#line 5421 "src/graph/dcParser.c" /* yacc.c:1646  */
    break;


#line 5425 "src/graph/dcParser.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 2072 "src/graph/dcParser.y" /* yacc.c:1906  */


////////////////////////////////////////////////////////////////////////////////

void dcParser_extractAndClearMethodParameterListData(dcList *_methodInfo,
                                                     dcString *_methodName,
                                                     dcList *_methodArguments)
{
    dcListElement *methodElement = NULL;

    for (methodElement = _methodInfo->head;
         methodElement != NULL;
         methodElement = methodElement->next)
    {
        dcPair *pair = CAST_PAIR(methodElement->object);
        char *appendString = dcString_getString(pair->left);

        dcString_appendString(_methodName, appendString);

        if (pair->right != NULL)
        {
            dcList_push(_methodArguments, pair->right);
        }

        dcPair_clearLeft(pair, DC_DEEP);
    }
}

static dcNode *createFunction(dcNode *_identifier,
                              dcScopeDataFlags _flags,
                              dcList *_arguments,
                              dcNode *_arithmetic)
{
    dcNode *result = NULL;
    bool isUpdate = false;

    if (_arguments != NULL)
    {
        dcListElement *that;

        // determine whether that definition is an update //
        for (that = _arguments->head; that != NULL; that = that->next)
        {
            if (! IS_IDENTIFIER(that->object))
            {
                isUpdate = true;
            }
        }
    }

    if (isUpdate)
    {
        // this definition is an update //
        result = dcFunctionUpdate_createNode
            (_identifier, _arguments, _arithmetic);
    }
    else
    {
        // this definition is a real definition //
        dcNode *functionNode =
            dcNode_setTemplate
            (dcFunctionClass_createNode
             (dcGraphDataTree_createNode(_arithmetic),
              dcMethodHeader_create("", _arguments),
              true),
             true);
        result = dcAssignment_createNode(_identifier, functionNode, _flags);
    }

    return result;
}

void dcParser_handleParseError(dcLexer *_lexer)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNodeEvaluator_pushCallStackNode
        (evaluator,
         dcCallStackData_createNode
         (_lexer->errorString,
          _lexer->filenameId,
          _lexer->previousLineNumber));

    // if this parse error occurred during an import, then we need to
    // reset (start oveerrrr, agaaaaiiin) evaluator's onlyEvaluateJoins variable
    uint32_t classesSave = evaluator->onlyEvaluateClasses;
    evaluator->onlyEvaluateClasses = 0;
    dcParseErrorExceptionClass_throwObject(_lexer->errorString);
    evaluator->onlyEvaluateClasses = classesSave;

    dcNodeEvaluator_popCallStack(evaluator, DC_DEEP);

    // tell lexer we had a parse error //
    dcLexer_handleParseError(_lexer);
}

static MyClassHeader *createClassHeader(char *_className,
                                        char *_superName,
                                        dcScopeDataFlags _scopeDataFlags)
{
    MyClassHeader *result =
        (MyClassHeader *)dcMemory_allocate(sizeof(MyClassHeader));
    result->className = _className;
    result->superName = _superName;
    result->scopeDataFlags = _scopeDataFlags;
    return result;
}

dcTaffyThreadId parserSelf;

dcNode *dcParser_parseString(const char *_string,
                             const char *_fileName,
                             bool _handleParseError)
{
    // create a new lexer for the input, keep ownership of _inputString //
    dcLexer *lexer = dcLexer_createWithInput(_fileName, (char*)_string, false);
    dcNode *result = dcParser_synchronizedParse(lexer, _handleParseError, NULL);
    dcLexer_free(&lexer);
    return result;
}

typedef struct
{
    dcLexer *lexer;
    bool handleParseError;
    dcPieLineEvaluatorOutFlag *outFlags;
} ParseData;

static void *synchronizedParse(void *_argument)
{
    ParseData *data = (ParseData *)_argument;
    return dcParser_parse(data->lexer, data->handleParseError, data->outFlags);
}

dcNode *dcParser_synchronizedParse(dcLexer *_lexer,
                                   bool _handleParseError,
                                   dcPieLineEvaluatorOutFlag *_outFlags)
{
    ParseData data = {0};
    data.lexer = _lexer;
    data.handleParseError = _handleParseError;
    data.outFlags = _outFlags;

    return (dcNode *)(dcNodeEvaluator_synchronizeFunctionCall
                      (dcSystem_getCurrentNodeEvaluator(),
                       &synchronizedParse,
                       &data));
}

dcNode *dcParser_parse(dcLexer *_lexer,
                       bool _handleParseError,
                       dcPieLineEvaluatorOutFlag *_outFlags)
{
    // sanity
    TAFFY_DEBUG(dcError_assert(! dcSystem_isLive()
                               || (dcSystem_getCurrentNodeEvaluator()->exception
                                   == NULL)));

    // lock
    dcGarbageCollector_nodeEvaluatorDown();
    dcParser_lock();
    dcGarbageCollector_nodeEvaluatorBlockUp();

    dcGarbageCollector_blockOtherNodeEvaluators();

    TAFFY_DEBUG(parserSelf = dcThread_getSelfId());

    dcMemory_useMemoryRegions();

    if (dcLog_isEnabled(PARSER_LOG))
    {
        yydebug = 1;
    }

    assert(sLexer == NULL);
    sLexer = _lexer;
    sGotComment = false;
    sParseHead = NULL;
    sPackageName = (char *)dcMemory_trackMemory((void *)dcMemory_strdup(""));
    sClassNames = (dcList *)dcMemory_trackMemory((void *)dcList_create());
    sOutFlags = _outFlags;

    // parse!
    yyparse();

    dcLexer_clearScopeDataFlags(sLexer);
    sLexer = NULL;
    sOutFlags = NULL;

    dcMemory_useMalloc();

    if (dcLexer_hasParseError(_lexer) || sParseHead == NULL)
    {
        sParseHead = NULL;

        if (_handleParseError)
        {
            dcParser_handleParseError(_lexer);
            _lexer->parseError = false;
        }

        dcMemory_freeMemoryRegions(DC_DEEP);
    }
    else
    {
        dcMemory_freeMemoryRegions(DC_SHALLOW);
    }

    dcNode *parseHead = sParseHead;
    // unlock
    dcGarbageCollector_unblockOtherNodeEvaluators();
    dcParser_unlock();

    if (_outFlags != NULL
        && parseHead != NULL)
    {
        if (IS_GRAPH_DATA(parseHead))
        {
            if (IS_ASSIGNMENT(parseHead))
            {
                *_outFlags |= PARSER_IS_ASSIGNMENT;
            }
            else if (IS_FUNCTION_UPDATE(parseHead))
            {
                *_outFlags |= PARSER_IS_FUNCTION_UPDATE;
            }
            else if (sGotComment && IS_NIL(parseHead))
            {
                *_outFlags |= PARSER_IS_COMMENT;
            }
        }
    }

    return (parseHead == NULL
            ? NULL
            : dcGraphDataTree_createNode(parseHead));
}

dcLexer *dcParser_getLexer(void)
{
    return sLexer;
}

static void pushClassName(const char *_name)
{
    dcList_push(sClassNames, dcString_createNodeWithString(_name, true));
}

dcNode *dcParser_createParenthesesOperatorFunctionCall(dcNode *_identifier,
                                                       dcArray *_arguments)
{
    return (dcMethodCall_createNode
            (_identifier,
             dcSystem_getOperatorName(TAFFY_PARENTHESES),
             dcList_createWithObjects
             (dcNode_setTemplate
              (dcArrayClass_createObject(_arguments, false), true), // template
              NULL))); // no more objects
}

static dcNode *createFunctionCall(dcNode *_receiver, dcList *_arguments)
{
    //
    // first stuff the given arguments into an Array object
    // then stuff the Array object into a List for the method call, whew
    //

    dcNode *result = dcParser_createParenthesesOperatorFunctionCall
        (_receiver, dcArray_createFromList(_arguments, DC_SHALLOW));
    dcList_free(&_arguments, DC_SHALLOW);
    return result;
}

dcNode *dcParser_createFunctionFromGuts(dcNode *_arithmetic)
{
    dcList *identifiers = dcList_create();
    dcFlatArithmetic_populateIdentifiers(_arithmetic, identifiers);

    if (identifiers->size == 0)
    {
        dcList_push(identifiers,
                    dcIdentifier_createNode("x", NO_SCOPE_DATA_FLAGS));
    }

    return (dcNode_setTemplate
            (dcFunctionClass_createObjectWithArguments(identifiers,
                                                       _arithmetic),
             true));
}

void dcParser_lock(void)
{
    dcMutex_lock(sMutex);
}

void dcParser_unlock(void)
{
    dcMutex_unlock(sMutex);
}

void dcParser_initialize(void)
{
    sMutex = dcMutex_create(true);
}

void dcParser_deinitialize(void)
{
    dcMutex_free(&sMutex);
}

void dcParser_setGotComment(void)
{
    sGotComment = true;
}
