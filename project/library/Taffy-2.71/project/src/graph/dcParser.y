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

%{
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

%}

// keywords //
// tokens that begin with #
%token kMETHOD_ATTRIBUTE_BREAKTHROUGH kMETHOD_ATTRIBUTE_CONST
%token kMETHOD_ATTRIBUTE_CONTAINER_LOOP
%token kEQUATION kMETHOD_ATTRIBUTE_MODIFIES_CONTAINER
%token kMETHOD_ATTRIBUTE_OPERATOR kMETHOD_ATTRIBUTE_PREFIX_OPERATOR
%token kMETHOD_ATTRIBUTE_SYNCHRONIZED
%token kMETHOD_ATTRIBUTE_SYNCHRONIZED_READ
%token kMETHOD_ATTRIBUTE_SYNCHRONIZED_WRITE

%token kAUTOMATIC_FUNCTION
%token kABSTRACT kAND kATOMIC kBREAK kCATCH kCLASS kCONST kELSE kFALSE kFINAL
%token kFOR kGLOBAL kI kIF kIMPORT kIN kLOCAL kNEW kNIL kNO kOR kPACKAGE
%token kPROTECTED kPUBLIC kRETURN kR kRW kSELF kSINGLETON kSLICE kSUPER
%token kSYNCHRONIZED kTHROW kTRUE kTRY kUP_SELF kW kWHILE kYES

// tokens //
%token tBIT_AND_EQUAL tBIT_OR_EQUAL tDIVIDE_EQUAL tDOT tEXPR_END tEQUAL_EQUAL
%token tGREATER_THAN_OR_EQUAL tINSTANCE_SCOPED_IDENTIFIER tBLOCK_START
%token tBLOCK_END tLEFT_BRACE_LESS_THAN tLEFT_SHIFT tLEFT_SHIFT_EQUAL
%token tLESS_THAN_OR_EQUAL tMATRIX_DELIMITER tMETA_SCOPED_IDENTIFIER
%token tMETHOD_INSTANCE tMETHOD_META tMETHOD_PARAMETER tMINUS_EQUAL
%token tMINUS_MINUS tMODULUS_EQUAL tMULTIPLY_EQUAL tNOT_EQUAL tNUMBER
%token tCOMPLEX_NUMBER tPLUS_EQUAL tPLUS_PLUS tPOWER_EQUAL
%token tRIGHT_ARROW tRIGHT_BRACKET_EQUAL tRIGHT_SHIFT tRIGHT_SHIFT_EQUAL
%token tRIGHT_PAREN_EQUAL
%token tSTRING tSTRING_EXPRESSION_START tSYMBOL
%token tTILDE_EQUAL tTILDE_EQUAL_LESS_THAN tVERBATIM_TEXT_START tWORD
%token tBIT_XOR tBIT_XOR_EQUAL

// precedences //
%left '=' tRIGHT_BRACKET_EQUAL ','
%left kOR kAND
%left tPLUS_EQUAL tMINUS_EQUAL tDIVIDE_EQUAL tMULTIPLY_EQUAL tEQUAL_EQUAL
%left '<' '>' tLESS_THAN_OR_EQUAL tGREATER_THAN_OR_EQUAL
%left tTILDE_EQUAL tTILDE_EQUAL_LESS_THAN tNOT_EQUAL tPOWER_EQUAL tMODULUS_EQUAL tBIT_AND_EQUAL tBIT_OR_EQUAL
%left tBIT_XOR tBIT_XOR_EQUAL
%left tLEFT_SHIFT tLEFT_SHIFT_EQUAL tRIGHT_SHIFT tRIGHT_SHIFT_EQUAL
%left '|'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%left '!' tPLUS_PLUS tMINUS_MINUS '~'
%right '^'

// types //
%union
{
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
}

%type <node> expression expressions expressionP equation object
%type <node> function function_call function_arithmetic
%type <node> statement statementP statement_element automatic_function
%type <node> class new array matrix object_identifier object_identifierP
%type <node> return assignment assignmentP throw method_call
%type <node> method_call_receiver method_call_receiverP
%type <node> tryBlock self upSelf super break block string
%type <node> identifier_chain identifier_chain_rest
%type <node> string_expression number
%type <node> keyword hash arithmetic real_arithmetic prefix_arithmetic
%type <node> assignment_arithmetic
%type <node> bracketed_method_call indexed_method_call nil boolean
%type <node> block_head indexed_method_call_receiver
%type <node> expression_rhs expression_rhsP expression_rhs_with_in
%type <node> symbol if ifPrime if_guts
%type <node> while whileP while_condition for forP for_end
%type <node> method_parameter else
%type <node> metaclass_object instance_object scope_data_class
%type <node> synchronized synchronized_guts
%type <node> method_head method_headP method class_method
%type <node> method_calls import package
%type <node> identifier identifierP question_colon in function_rhs
%type <node> function_rhs_guts
%type <classHeader> class_header

%type <list> class_data function_call_guts

%type <methodHeader> method_header

%type <string> method_parameter_keyword kAUTOMATIC_FUNCTION

%type <taffyString> identifier_dot_list real_identifier_dot_list

%type <constString> kABSTRACT kAND kATOMIC kBREAK kCATCH kCLASS
%type <constString> kCONST kELSE kFALSE kGLOBAL kLOCAL kIF
%type <constString> kNEW kNIL kOR kRETURN kSELF kSLICE kSUPER
%type <constString> kMETHOD_ATTRIBUTE_SYNCHRONIZED
%type <constString> kMETHOD_ATTRIBUTE_SYNCHRONIZED_READ
%type <constString> kMETHOD_ATTRIBUTE_SYNCHRONIZED_WRITE
%type <constString> kMETHOD_ATTRIBUTE_BREAKTHROUGH
%type <constString> kMETHOD_ATTRIBUTE_CONST
%type <constString> kMETHOD_ATTRIBUTE_MODIFIES_CONTAINER
%type <constString> kMETHOD_ATTRIBUTE_CONTAINER_LOOP
%type <constString> kTHROW kTRUE kTRY kUP_SELF kWHILE kMETHOD_ATTRIBUTE_OPERATOR
%type <constString> method_parameter_const_keyword

%type <string> tVERBATIM_TEXT_START tWORD tMETHOD_PARAMETER tSTRING tSYMBOL
%type <string> tMETA_SCOPED_IDENTIFIER tINSTANCE_SCOPED_IDENTIFIER
%type <string> tPLUS_EQUAL tMINUS_EQUAL kR kW kRW
%type <string> operator argument_operator prefix_operator

%type <iValue> class_object_accessor
%type <iValue> class_keywords real_class_keywords real_real_class_keywords
%type <iValue> const_global_flags
%type <iValue> method_attributes method_attribute

%type <list> array_rest hash_objects hash_guts function_arithmetic_list
%type <list> string_contents catches identifier_list
%type <list> method_parameter_list expression_rhs_list matrix_guts
%type <list> class_path class_path_star expression_list

%type <number> tNUMBER
%type <complexNumber> tCOMPLEX_NUMBER

%start start

%%

start: statement
{
    sParseHead = $1;

    if ($1 == NULL)
    {
        sParseHead = dcNil_createNode();
    }
}

statement: statementP

statementP: statement_element tEXPR_END
| statement_element
| statement_element tEXPR_END statement
{
    dcGraphData_setNext($1, $3);
}

statement_element: class
| expression
| import
| package
| tEXPR_END
{
    $$ = NULL;
}

expressions: expression
| expression tEXPR_END
| expression tEXPR_END expressions
{
    dcGraphData_setNext($1, $3);
}

expression: expressionP

expressionP:        function |
                  assignment |
              expression_rhs |
                          if |
                     keyword |
                 method_call |
                synchronized |
                       throw |
                    tryBlock |
                       while |
                         for |
                          in

in: object_identifier kIN array
{
    dcArray *array = dcArrayClass_getObjects($3);
    dcArrayClass_clearObjects($3);
    dcNode_free(&$3, DC_DEEP);
    $$ = dcIn_createNode($1, array);
}

expression_rhs: expression_rhsP

expression_rhsP: assignment_arithmetic |
                 bracketed_method_call |
                         function_call |
                     object_identifier |
                   indexed_method_call |
                            arithmetic |
                        question_colon |
                          function_rhs

expression_rhs_with_in: expression_rhs | in

question_colon: '(' expression_rhs '?' expression_rhs ':' expression_rhs ')'
{
    $$ = dcIf_createNode($2,
                         $4,
                         dcIf_createNode(dcTrue_createNode(), $6, NULL));
}

function_rhs_guts: arithmetic |
                   identifier |
                  method_call |
                       number |
                function_call

function_rhs: '{' function_rhs_guts '}'
{
    $$ = dcParser_createFunctionFromGuts($2);
}

object_identifier: object_identifierP
{
    if ($1 == NULL)
    {
        dcAbort("NULL graph data in parser");
    }
}

object_identifierP: identifier |
                        object |
                           new |
                          self |
                         super |
                        symbol |
                        upSelf

object:  array |
        matrix |
         block |
          hash |
        method |
           nil |
        number |
        string |
      equation

equation: kEQUATION '(' expression_rhs '=' expression_rhs ')'
{
    $$ = dcEquationClass_createObject($3, $5);
}

nil: kNIL
{
    $$ = dcNil_createNode();
}

identifier: identifierP
{
}

identifierP: identifier_dot_list
{
    $$ = dcIdentifier_createNode($1->string, NO_FLAGS);
    dcString_free(&$1, DC_DEEP);
}
| tINSTANCE_SCOPED_IDENTIFIER
{
    $$ = dcIdentifier_createNode($1, SCOPE_DATA_INSTANCE);
    dcMemory_trackMemory($1);
}
| tMETA_SCOPED_IDENTIFIER
{
    $$ = dcIdentifier_createNode($1, SCOPE_DATA_META);
    dcMemory_trackMemory($1);
}
| tWORD
{
    $$ = dcIdentifier_createNode($1, NO_FLAGS);
    dcMemory_trackMemory($1);
}

identifier_dot_list: real_identifier_dot_list tDOT tWORD
{
    dcString_append($1, ".%s", $3);
    dcMemory_trackMemory($3);
}

real_identifier_dot_list: tWORD
{
    $$ = dcString_createWithString($1, false);
}
| real_identifier_dot_list tDOT tWORD
{
    dcString_append($1, ".%s", $3);
    dcMemory_trackMemory($3);
}

identifier_chain: '[' identifier tRIGHT_ARROW identifier_chain_rest
{
    $$ = $2;
}

identifier_chain_rest: identifier ']'
{
    $$ = $1;
}
| identifier tRIGHT_ARROW identifier_chain_rest
{
    $$ = $1;
}

arithmetic: real_arithmetic
| prefix_arithmetic
| boolean
| expression_rhs '!'
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_FACTORIAL), NULL);
}

prefix_arithmetic: '+' expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($2, dcSystem_getOperatorName(TAFFY_PREFIX_PLUS), NULL);
}
| tMINUS_MINUS expression_rhs
{
    $$ = $2;
}
| '-' expression_rhs
{
    dcNode *right = $2;

    if (dcNumberClass_isMe(right))
    {
        // is like: -3, so perform the multiply right now
        $$ = dcNumberClass_inlineMultiply
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
        $$ = right;
    }
    else
    {
        // defer the multiply until evaluation time
        $$ = (dcFlatArithmetic_createNodeWithValues
              (TAFFY_MULTIPLY,
               dcNode_setTemplate
               (dcNumberClass_createObjectFromInt32s(-1), true),
               right,
               NULL));
    }
}
| '~' expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($2, dcSystem_getOperatorName(TAFFY_BIT_NOT), NULL);
}

real_arithmetic: expression_rhs '+' expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_ADD, $1, $3, NULL);
}
| expression_rhs '-' expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_SUBTRACT, $1, $3, NULL);
}
| expression_rhs '*' expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_MULTIPLY, $1, $3, NULL);
}
| expression_rhs '/' expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_DIVIDE, $1, $3, NULL);
}
| expression_rhs '^' expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_RAISE, $1, $3, NULL);
}
| expression_rhs tBIT_XOR expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_BIT_XOR, $1, $3, NULL);
}
| expression_rhs '%' expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_MODULUS, $1, $3, NULL);
}
| expression_rhs '|' expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_BIT_OR, $1, $3, NULL);
}
| expression_rhs '&' expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_BIT_AND, $1, $3, NULL);
}
| expression_rhs tLEFT_SHIFT expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_LEFT_SHIFT, $1, $3, NULL);
}
| expression_rhs tRIGHT_SHIFT expression_rhs
{
    $$ = dcFlatArithmetic_createNodeWithValues(TAFFY_RIGHT_SHIFT, $1, $3, NULL);
}

assignment_arithmetic: identifier tPLUS_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_PLUS_EQUAL), $3);
}
| identifier tMINUS_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_MINUS_EQUAL), $3);
}
| identifier tRIGHT_SHIFT_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_RIGHT_SHIFT_EQUAL), $3);
}
| identifier tLEFT_SHIFT_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_LEFT_SHIFT_EQUAL), $3);
}
| identifier tMULTIPLY_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_MULTIPLY_EQUAL), $3);
}
| identifier tDIVIDE_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_DIVIDE_EQUAL), $3);
}
| identifier tPOWER_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_RAISE_EQUAL), $3);
}
| identifier tMODULUS_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_MODULUS_EQUAL), $3);
}
| identifier tBIT_XOR_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_BIT_XOR_EQUAL), $3);
}
| identifier tBIT_OR_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_BIT_OR_EQUAL), $3);
}
| identifier tBIT_AND_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_BIT_AND_EQUAL), $3);
}
| expression_rhs tPLUS_PLUS
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_PLUS_PLUS), NULL);
}
| expression_rhs tMINUS_MINUS
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_MINUS_MINUS), NULL);
}

boolean: expression_rhs '<' expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_LESS_THAN), $3);
}
| expression_rhs tLESS_THAN_OR_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_LESS_THAN_OR_EQUAL), $3);
}
| expression_rhs '>' expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_GREATER_THAN), $3);
}
| expression_rhs tGREATER_THAN_OR_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_GREATER_THAN_OR_EQUAL), $3);
}
| '(' expression_rhs ')'
{
    if (IS_FLAT_ARITHMETIC($2))
    {
        CAST_FLAT_ARITHMETIC($2)->grouped = true;
    }

    $$ = $2;
}
| expression_rhs tEQUAL_EQUAL expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument
        ($1, dcSystem_getOperatorName(TAFFY_EQUALS), $3);
}
| expression_rhs tTILDE_EQUAL expression_rhs
{
    $$ = (dcMethodCall_createNode
          ($1,
           dcSystem_getOperatorName(TAFFY_DELTA_EQUALS),
           dcList_createWithObjects
           (dcNode_setTemplate
            (dcArrayClass_createObject
             (dcArray_createWithObjects
              ($3,
               dcNode_setTemplate
               (dcNode_copy(dcNumberMetaClass_getDefaultDelta(),
                            DC_DEEP),
                true),
               NULL),
              false),
             true),
            NULL)));
}
| expression_rhs tTILDE_EQUAL_LESS_THAN tNUMBER '>' expression_rhs
{
    $$ = (dcMethodCall_createNode
          ($1,
           dcSystem_getOperatorName(TAFFY_DELTA_EQUALS),
           dcList_createWithObjects
           (dcNode_setTemplate(dcArrayClass_createObject
                               (dcArray_createWithObjects
                                ($5,
                                 dcNode_setTemplate
                                 (dcNumberClass_createObject($3),
                                  true),
                                 NULL),
                                false),
                               true),
            NULL)));
}
| expression_rhs tNOT_EQUAL expression_rhs
{
    $$ = dcNotEqualCall_createNode
        (dcMethodCall_createNodeWithArgument
         ($1, dcSystem_getOperatorName(TAFFY_EQUALS), $3));
}
| expression_rhs kAND expression_rhs
{
    $$ = dcAnd_createNode($1, $3);
}
| expression_rhs kOR expression_rhs
{
    $$ = dcOr_createNode($1, $3);
}
| kTRUE
{
    $$ = dcTrue_createNode();
}
| kYES
{
    $$ = dcTrue_createNode();
}
| kFALSE
{
    $$ = dcFalse_createNode();
}
| kNO
{
    $$ = dcFalse_createNode();
}
| '!' expression_rhs
{
    $$ = dcMethodCall_createNodeWithArgument($2, "#prefixOperator(!)", NULL);
}
| '(' in ')'
{
    $$ = $2;
}

assignment: assignmentP

assignmentP: identifier '=' expression_rhs
{
    $$ = dcAssignment_createNode($1, $3, NO_FLAGS);
}
| identifier '=' automatic_function
{
    $$ = dcAssignment_createNode($1, $3, NO_FLAGS);
}
| const_global_flags identifier '=' expression_rhs
{
    $$ = dcAssignment_createNode($2, $4, $1);
}

self: kSELF
{
    $$ = dcSelf_createNode();
}

upSelf: kUP_SELF
{
    $$ = dcUpSelf_createNode();
}

super: kSUPER
{
    $$ = dcSuper_createNode();
}

new: kNEW identifier
{
    $$ = dcNew_createNode($2);
}

const_global_flags: kCONST
{
    $$ = SCOPE_DATA_CONSTANT;
}
| kGLOBAL
{
    $$ = SCOPE_DATA_GLOBAL;
}
| kLOCAL
{
    $$ = SCOPE_DATA_LOCAL;
}
| kCONST kGLOBAL
{
    $$ = SCOPE_DATA_CONSTANT | SCOPE_DATA_GLOBAL;
}
| kGLOBAL kCONST
{
    $$ = SCOPE_DATA_GLOBAL | SCOPE_DATA_CONSTANT;
}
| kCONST kLOCAL
{
    $$ = SCOPE_DATA_CONSTANT | SCOPE_DATA_LOCAL;
}
| kLOCAL kCONST
{
    $$ = SCOPE_DATA_LOCAL | SCOPE_DATA_CONSTANT;
}

//
// a function like: f(x, y) = x + y
//
function: identifier
          '('
          expression_rhs_list
          tRIGHT_PAREN_EQUAL
          function_arithmetic
{
    $$ = createFunction($1, NO_FLAGS, $3, $5);
}
| kAUTOMATIC_FUNCTION
  '('
  expression_rhs_list
  tRIGHT_PAREN_EQUAL
  function_arithmetic
{
    $$ = createFunction(dcIdentifier_createNode($1, NO_SCOPE_DATA_FLAGS),
                        NO_FLAGS,
                        $3,
                        $5);
    dcMemory_trackMemory($1);
}
//
// a const or global function, can't attach global or const to self, super, etc
// so use identifier instead of object_identifier
//
| const_global_flags
  identifier
  '('
  expression_rhs_list
  tRIGHT_PAREN_EQUAL
  function_arithmetic
{
    $$ = createFunction($2, $1, $4, $6);
}
| const_global_flags
  kAUTOMATIC_FUNCTION
  '('
  expression_rhs_list
  tRIGHT_PAREN_EQUAL
  function_arithmetic
{
    $$ = createFunction(dcIdentifier_createNode($2, NO_SCOPE_DATA_FLAGS),
                        $1,
                        $4,
                        $6);
    dcMemory_trackMemory($2);
}

function_arithmetic:  arithmetic
| method_calls
| identifier_chain
| identifier
| method_call
| object

method_calls: bracketed_method_call |
                indexed_method_call |
                      function_call

if: ifPrime

ifPrime: kIF '(' expression_rhs_with_in ')' if_guts
{
    $$ = dcIf_createNode($3, $5, NULL);
}
| kIF '(' expression_rhs_with_in ')' if_guts else
{
    $$ = dcIf_createNode($3, $5, $6);
}

if_guts: expr_end_left_brace statement '}'
{
    $$ = $2;
}
| expr_end_left_brace '}'
{
    $$ = dcNil_createNode();
}

else: kELSE if_guts
{
    $$ = dcIf_createNode(dcTrue_createNode(), $2, NULL);
}
| kELSE ifPrime
{
    $$ = $2;
}

synchronized: kSYNCHRONIZED '(' identifier ')' synchronized_guts
{
    $$ = dcSynchronized_createNode($3, $5);
}

synchronized_guts: '{' statement '}'
{
    $$ = $2;
}
| expr_ends '{' statement '}'
{
    $$ = $3;
}
| '{' '}'
{
    $$ = dcNil_createNode();
}
| expr_ends '{' '}'
{
    $$ = dcNil_createNode();
}

for: forP
{
    $$ = $1;
}

forP: kFOR '(' expression_list expr_ends
               expression expr_ends
               expression_list
           ')'
      expr_end_left_brace for_end
{
    $$ = dcFor_createNode(dcList_createShell($3),
                          $5,
                          dcList_createShell($7),
                          $10);
}
| kFOR '(' expr_ends
           expression expr_ends
           expression_list
       ')'
  expr_end_left_brace for_end
{
    $$ = (dcFor_createNode
          (dcList_createNodeWithObjects(dcNil_createNode(), NULL),
           $4,
           dcList_createShell($6),
           $9));
}
| kFOR '(' expression_list expr_ends
           expression expr_ends
       ')'
  expr_end_left_brace for_end
{
    $$ = (dcFor_createNode
          (dcList_createShell($3),
           $5,
           dcList_createNodeWithObjects(dcNil_createNode(), NULL),
           $9));
}
| kFOR '(' expr_ends
           expression expr_ends
       ')'
  expr_end_left_brace for_end
{
    $$ = (dcFor_createNode
          (dcList_createNodeWithObjects(dcNil_createNode(), NULL),
           $4,
           dcList_createNodeWithObjects(dcNil_createNode(), NULL),
           $8));
}

for_end:  statement '}'
{
    $$ = $1;
}
| '}'
{
    $$ = dcNil_createNode();
}

while: whileP

expr_end_left_brace: tEXPR_END '{' | '{'

while_condition: boolean | identifier

whileP: kWHILE '(' while_condition ')' expr_end_left_brace statement '}'
{
    $$ = dcWhile_createNode($3, $6);
}
| kWHILE '(' while_condition ')' expr_end_left_brace '}'
{
    $$ = dcWhile_createNode($3, dcNil_createNode());
}
| kWHILE '(' bracketed_method_call ')' expr_end_left_brace statement '}'
{
    $$ = dcWhile_createNode($3, $6);
}
| expression_rhs kWHILE boolean
{
    $$ = dcWhile_createNode($3, $1);
}

block: block_head expressions '}'
{
    dcNode *procedure = (dcClass_castNodeWithAssert
                         ($1,
                          dcProcedureClass_getTemplate(),
                          false,
                          true));
    dcProcedureClass_setBody(procedure, dcGraphDataTree_createNode($2));
}
| block_head '}'
{
    dcNode *procedure = (dcClass_castNodeWithAssert
                         ($1,
                          dcProcedureClass_getTemplate(),
                          false,
                          true));
    dcProcedureClass_setBody(procedure,
                             dcGraphDataTree_createNode(dcNil_createNode()));
}

block_head: tLEFT_BRACE_LESS_THAN identifier_list '>'
{
    $$ = dcNode_setTemplate(dcBlockClass_createObject(NULL, $2), true);
}
| tBLOCK_START
{
    $$ = dcNode_setTemplate(dcBlockClass_createObject(NULL, NULL), true);
}

identifier_list: identifier_list ',' identifier
{
    dcList_push($1, $3);
}
| identifier
{
    $$ = dcList_createWithObjects($1, NULL);
}

number: tNUMBER
{
    $$ = dcNode_setTemplate(dcNumberClass_createObject($1), true);
}
| tCOMPLEX_NUMBER
{
    $$ = dcNode_setTemplate(dcComplexNumberClass_createObject($1), true);
}
| kI
{
    $$ = (dcNode_setTemplate
          (dcComplexNumberClass_createObject
           (dcComplexNumber_create(NULL, dcNumber_createFromInt32u(1))),
           true));
}
| tNUMBER kI
{
    $$ = (dcNode_setTemplate
          (dcComplexNumberClass_createObject
           (dcComplexNumber_create(NULL, $1)),
           true));
}

hash: '(' hash_guts ')'
{
    dcNode *keysNode = dcList_get($2, 0);
    dcNode *valuesNode = dcList_get($2, 1);

    $$ = dcNode_setTemplate
        (dcHashClass_createUninitializedObject(CAST_LIST(keysNode),
                                               CAST_LIST(valuesNode)),
         true);
    dcList_pop($2, DC_FLOATING);
    dcList_pop($2, DC_FLOATING);
    dcNode_freeShell(&keysNode);
    dcNode_freeShell(&valuesNode);
    dcList_free(&$2, DC_DEEP);
}
| '(' ')'
{
    $$ = dcNode_setTemplate
        (dcHashClass_createInitializedObject(dcHash_create()),
         true);
}

hash_guts: hash_objects
{
    $$ = $1;
}
| hash_objects expr_ends
{
    $$ = $1;
}

hash_objects: expression_rhs tRIGHT_ARROW expression_rhs ',' hash_objects
{
    dcList_unshift(CAST_LIST(dcList_get($5, 0)), $1);
    dcList_unshift(CAST_LIST(dcList_get($5, 1)), $3);
    $$ = $5;
}
| tEXPR_END hash_objects
{
    $$ = $2;
}
| expression_rhs tRIGHT_ARROW expression_rhs
{
    $$ = dcList_createWithObjects(dcNode_setTemplate
                                  (dcList_createNodeWithObjects($1, NULL),
                                   true),
                                  dcNode_setTemplate
                                  (dcList_createNodeWithObjects($3, NULL),
                                   true),
                                  NULL);
}

expr_ends: tEXPR_END expr_ends | tEXPR_END

symbol: tSYMBOL
{
    $$ = dcSymbol_createNode($1);
    dcMemory_trackMemory($1);
}

function_arithmetic_list: function_arithmetic ',' function_arithmetic_list
{
    dcList_unshift($3, $1);
    $$ = $3;
}
| function_arithmetic
{
    $$ = dcList_createWithObjects($1, NULL);
}

expression_list: expression ',' expression_list
{
    dcList_unshift($3, $1);
    $$ = $3;
}
| expression
{
    $$ = dcList_createWithObjects($1, NULL);
}

matrix_guts: function_arithmetic_list tEXPR_END matrix_guts
{
    dcList_unshift($3, dcNode_setTemplate(dcList_createShell($1), true));
    $$ = $3;
}
| function_arithmetic_list
{
    $$ = dcList_createWithObjects
        (dcNode_setTemplate(dcList_createShell($1), true), NULL);
}

matrix: tMATRIX_DELIMITER matrix_guts tMATRIX_DELIMITER
{
    dcMatrix *matrix = dcMatrix_createFromLists($2);

    if (matrix == NULL)
    {
        yyerror(NULL);
        dcLexer_setErrorString(sLexer, "malformed matrix");
        YYABORT;
    }
    else
    {
        TAFFY_DEBUG(dcMatrix_assertIsTemplate(matrix));
        dcList_free(&$2, DC_SHALLOW);
        $$ = dcNode_setTemplate(dcMatrixClass_createObject
                                (matrix, false),
                                true);
    }
}

array: '[' array_rest ']'
{
    $$ = dcNode_setTemplate
        (dcArrayClass_createObject(dcArray_createFromList($2, DC_SHALLOW),
                                   false),
         true);
    dcList_free(&$2, DC_SHALLOW);
}
| '[' expr_ends array_rest ']'
{
    $$ = dcNode_setTemplate
        (dcArrayClass_createObject(dcArray_createFromList($3, DC_SHALLOW),
                                   false),
         true);
    dcList_free(&$3, DC_SHALLOW);
}
| '[' ']'
{
    $$ = dcNode_setTemplate(dcArrayClass_createEmptyObject(), true);
}

array_rest: expression_rhs ',' array_rest
{
    dcList_unshift($3, $1);
    $$ = $3;
}
| expression_rhs expr_ends ',' array_rest
{
    dcList_unshift($4, $1);
    $$ = $4;
}
| expression_rhs ',' expr_ends array_rest
{
    dcList_unshift($4, $1);
    $$ = $4;
}
| ',' array_rest
{
    dcList_unshift($2, dcNil_createNode());
    $$ = $2;
}
| expression_rhs
{
    $$ = dcList_createWithObjects($1, NULL);
}
| expression_rhs expr_ends
{
    $$ = dcList_createWithObjects($1, NULL);
}

string: '"' string_contents '"'
{
    $$ = dcNode_setTemplate
        (dcStringClass_createObjectFromList($2, false), true);
}
| '"' '"'
{
    $$ = dcNode_setTemplate
        (dcStringClass_createObjectFromList(dcList_createWithObjects(NULL),
                                            false),
         true);
}
| tVERBATIM_TEXT_START tSTRING
{
    $$ = dcNode_setTemplate(dcStringClass_createObject($2, false), true);
}

string_contents: tSTRING string_contents
{
    dcList_unshift($2, dcString_createNodeWithString($1, false));
    $$ = $2;
}
| string_expression string_contents
{
    dcList_unshift($2, $1);
    $$ = $2;
}
| tSTRING
{
    $$ = dcList_createWithObjects
        (dcString_createNodeWithString($1, false), NULL);
}
| string_expression
{
    $$ = dcList_createWithObjects($1, NULL);
}

string_expression: tSTRING_EXPRESSION_START expression ']'
{
    $$ = $2;
}
| tSTRING_EXPRESSION_START ']'
{
    $$ = dcNil_createNode();
}

keyword: return | break

break: kBREAK
{
    $$ = dcBreak_createNode();
}

return: kRETURN
{
    $$ = dcReturn_createNode(NULL);
}
| kRETURN expression_rhs_with_in
{
    $$ = dcReturn_createNode($2);
}

package: kPACKAGE class_path
{
    $$ = dcPackage_createNode($2);
    sPackageName = dcPackage_getPathString(CAST_PACKAGE($$));
    dcMemory_trackMemory(sPackageName);
}

import: kIMPORT class_path_star
{
    $$ = dcImport_createNode($2);
}

class_path: class_path tDOT tWORD
{
    dcList_push($1, dcString_createNodeWithString($3, false));
}
| tWORD
{
    $$ = dcList_createWithObjects
        (dcString_createNodeWithString($1, false), NULL);
}

class_path_star: class_path
{
}
| class_path tDOT '*'
{
    dcList_push($1, dcString_createNodeWithString("*", true));
}

tryBlock: kTRY expr_end_left_brace statement '}' catches
{
    $$ = dcTryBlock_createNode($3, $5);
}
| kTRY expr_end_left_brace '}' catches
{
    $$ = dcTryBlock_createNode(dcNil_createNode(), $4);
}
| kTRY expr_end_left_brace statement '}' tEXPR_END catches
{
    $$ = dcTryBlock_createNode($3, $6);
}
| kTRY expr_end_left_brace '}' tEXPR_END catches
{
    $$ = dcTryBlock_createNode(dcNil_createNode(), $5);
}

catches: kCATCH '(' identifier identifier ')' expr_end_left_brace statement '}'
{
    $$ = dcList_createWithObjects(dcCatchBlock_createNode($4, $3, $7), NULL);
}
| kCATCH '(' identifier identifier ')' expr_end_left_brace statement '}' catches
{
    dcList_unshift($9, dcCatchBlock_createNode($4, $3, $7));
    $$ = $9;
}
| kCATCH '(' identifier identifier ')' expr_end_left_brace '}'
{
    $$ = dcList_createWithObjects
        (dcCatchBlock_createNode($4, $3, dcNil_createNode()),
         NULL);
}
| kCATCH '(' identifier identifier ')' expr_end_left_brace '}' catches
{
    dcList_unshift($8, dcCatchBlock_createNode($4, $3, dcNil_createNode()));
    $$ = $8;
}

throw: kTHROW expression_rhs
{
    $$ = dcThrow_createNode($2);
}

///////////////////// Method call rules ///////////////////////////////////////
method_call: method_call_receiver method_parameter_list
{
    dcString *methodNameString = dcString_create();
    dcList *methodArgumentList = dcList_create();

    dcParser_extractAndClearMethodParameterListData($2,
                                                    methodNameString,
                                                    methodArgumentList);
    $$ = dcNode_setTemplate(dcMethodCall_createNode($1,
                                                    methodNameString->string,
                                                    methodArgumentList),
                            true);
    dcGraphData_copyPosition($1, $$);
    dcList_free(&$2, DC_SHALLOW);
    dcString_free(&methodNameString, DC_DEEP);
}
| automatic_function

automatic_function: kAUTOMATIC_FUNCTION arithmetic
{
    $$ = (dcParser_createParenthesesOperatorFunctionCall
          (dcIdentifier_createNode($1, NO_SCOPE_DATA_FLAGS),
           dcArray_createWithObjects(dcParser_createFunctionFromGuts($2),
                                     NULL)));
    dcMemory_trackMemory($1);
}
| kAUTOMATIC_FUNCTION identifier
{
    $$ = (dcParser_createParenthesesOperatorFunctionCall
          (dcIdentifier_createNode($1, NO_SCOPE_DATA_FLAGS),
           dcArray_createWithObjects($2, NULL)));
    dcMemory_trackMemory($1);
}
| kAUTOMATIC_FUNCTION function_call
{
    $$ = (dcParser_createParenthesesOperatorFunctionCall
          (dcIdentifier_createNode($1, NO_SCOPE_DATA_FLAGS),
           dcArray_createWithObjects(dcParser_createFunctionFromGuts($2),
                                     NULL)));
    dcMemory_trackMemory($1);
}

function_call_guts: expression_rhs_list
| method_call
{
    $$ = dcList_createWithObjects($1, NULL);
}

function_call: identifier '(' function_call_guts ')'
{
    $$ = createFunctionCall($1, $3);
}
| self '(' function_call_guts ')'
{
    $$ = createFunctionCall($1, $3);
}
| upSelf '(' function_call_guts ')'
{
    $$ = createFunctionCall($1, $3);
}
| super '(' function_call_guts ')'
{
    $$ = createFunctionCall($1, $3);
}

method_call_receiver: method_call_receiverP

method_call_receiverP: object_identifier |
                     indexed_method_call |
                              arithmetic |
                           function_call |
                            function_rhs

expression_rhs_list: expression_rhs_list ',' expression_rhs
{
    dcList_push($1, $3);
}
| expression_rhs
{
    $$ = dcList_createWithObjects($1, NULL);
}

indexed_method_call_receiver: identifier | self | upSelf | super

indexed_method_call: indexed_method_call_receiver
                     '['
                     expression_rhs_list
                     tRIGHT_BRACKET_EQUAL
                     expression_rhs
{
    dcList_push($3, $5);
    dcArray *array = dcArray_createFromList($3, DC_SHALLOW);
    dcList_free(&$3, DC_SHALLOW);

    $$ = dcMethodCall_createNode
        ($1,
         dcSystem_getOperatorName(array->size == 2
                                  ? TAFFY_BRACKETS_EQUAL
                                  : TAFFY_BRACKETS_DOTS_EQUAL),
         dcList_createWithObjects
         (dcNode_setTemplate
          (dcArrayClass_createObject(array, false),
           true), // template
          NULL)); // no more objects
}
| indexed_method_call_receiver '[' expression_rhs_list ']'
{
    //
    // operator [...]
    //
    dcList *list = $3;

    if (list->size == 1)
    {
        $$ = dcMethodCall_createNode
            ($1, dcSystem_getOperatorName(TAFFY_BRACKETS), list);
    }
    else
    {
        dcArray *array = dcArray_createFromList($3, DC_SHALLOW);
        dcList_free(&$3, DC_SHALLOW);
        $$ = dcMethodCall_createNode
            ($1,
             dcSystem_getOperatorName(TAFFY_BRACKETS_DOTS),
             dcList_createWithObjects
             (dcNode_setTemplate
              (dcArrayClass_createObject(array, false),
               true),
              NULL)); // no more objects
    }
}

bracketed_method_call: '[' method_call ']'
{
    $$ = dcList_createGraphDataNodeWithObjects($2, NULL);
}
| '[' bracketed_method_call method_parameter_list ']'
{
    dcString *methodNameString = dcString_create();
    dcList *methodArgumentList = dcList_create();

    dcParser_extractAndClearMethodParameterListData
        ($3, methodNameString, methodArgumentList);

    // the receiver will be evaluated //
    dcNode *methodCall = dcMethodCall_createNode
        (NULL, methodNameString->string, methodArgumentList);

    dcGraphData_copyPosition($2, methodCall);
    dcString_free(&methodNameString, DC_DEEP);
    dcList_push(CAST_GRAPH_DATA_LIST($2), methodCall);
    dcList_free(&$3, DC_SHALLOW);

    $$ = $2;
}

method_header: method_parameter_list
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
        ($1, methodNameString, methodArgumentList);

    $$ = dcMethodHeader_create(methodNameString->string, methodArgumentList);
    dcString_free(&methodNameString, DC_DEEP);
    dcList_free(&$1, DC_SHALLOW);
}
| kMETHOD_ATTRIBUTE_OPERATOR '(' operator ')'
{
    //
    // an operator method header, like:
    //
    // (@) #operator(++)
    // {
    // }
    //
    char *methodName = dcLexer_sprintf("#operator(%s)", $3);
    $$ = dcMethodHeader_create(methodName, NULL);
    dcMemory_trackMemory(methodName);
}
| kMETHOD_ATTRIBUTE_PREFIX_OPERATOR '(' prefix_operator ')'
{
    //
    // a prefix operator method header, like:
    //
    // (@) #prefixOperator(-)
    // {
    // }
    //
    char *methodName = dcLexer_sprintf("#prefixOperator(%s)", $3);
    $$ = dcMethodHeader_create(methodName, NULL);
    dcMemory_trackMemory(methodName);
}
| kMETHOD_ATTRIBUTE_OPERATOR '(' argument_operator ')' ':' tWORD
{
    //
    // a argument_operator method header, like (value is the argument):
    //
    // (@) #operator(+): value
    // {
    // }
    //
    char *methodName = dcLexer_sprintf("#operator(%s):", $3);
    $$ = dcMethodHeader_create
        (methodName,
         dcList_createWithObjects
         (dcIdentifier_createNode($6, NO_FLAGS), NULL));
    dcMemory_trackMemory($6);
    dcMemory_trackMemory(methodName);
}

operator: '!'
{
    $$ = "!";
}
| tPLUS_PLUS
{
    $$ = "++";
}
| tMINUS_MINUS
{
    $$ = "--";
}

prefix_operator: '!'
{
    $$ = "!";
}
| '-'
{
    $$ = "-";
}

argument_operator: tEQUAL_EQUAL
{
    $$ = "==";
}
| '(' ')'
{
    $$ = "()";
}
| '[' ']'
{
    $$ = "[]";
}
| '[' tDOT tDOT tDOT ']'
{
    $$ = "[...]";
}
| '[' tDOT tDOT tDOT tRIGHT_BRACKET_EQUAL
{
    $$ = "[...]=";
}
| '<'
{
    $$ = "<";
}
| tLESS_THAN_OR_EQUAL
{
    $$ = "<=";
}
| '>'
{
    $$ = ">";
}
| tGREATER_THAN_OR_EQUAL
{
    $$ = ">=";
}
| '+'
{
    $$ = "+";
}
| '-'
{
    $$ = "-";
}
| '*'
{
    $$ = "*";
}
| '/'
{
    $$ = "/";
}
| tTILDE_EQUAL
{
    $$ = "~=";
}
| tLEFT_SHIFT
{
    $$ = "<<";
}
| tRIGHT_SHIFT
{
    $$ = ">>";
}
| '^'
{
    $$ = "^";
}
| tBIT_XOR
{
    $$ = "^^";
}
| tBIT_XOR_EQUAL
{
    $$ = "^^=";
}
| '%'
{
    $$ = "%";
}
| tMODULUS_EQUAL
{
    $$ = "%=";
}
| '&'
{
    $$ = "&";
}
| tBIT_AND_EQUAL
{
    $$ = "&=";
}
| '|'
{
    $$ = "|";
}
| tBIT_OR_EQUAL
{
    $$ = "|=";
}
| '[' tRIGHT_BRACKET_EQUAL
{
    $$ = "[]=";
}
| tPLUS_EQUAL
{
    $$ = "+=";
}
| tMINUS_EQUAL
{
    $$ = "-=";
}
| tMULTIPLY_EQUAL
{
    $$ = "*=";
}
| tDIVIDE_EQUAL
{
    $$ = "/=";
}
| tPOWER_EQUAL
{
    $$ = "^=";
}
| tLEFT_SHIFT_EQUAL
{
    $$ = "<<=";
}
| tRIGHT_SHIFT_EQUAL
{
    $$ = ">>=";
}

method_parameter_list: method_parameter method_parameter_list
{
    dcList_unshift($2, $1);
    $$ = $2;
}
| method_parameter
{
    $$ = dcList_createWithObjects($1, NULL);
}
| method_parameter_keyword
{
    dcNode *left = dcString_createNodeWithString($1, false);
    $$ = dcList_createWithObjects(dcPair_createNode(left, NULL), NULL);
}
| method_parameter_const_keyword
{
    dcNode *left = dcString_createNodeWithString($1, true);
    $$ = dcList_createWithObjects(dcPair_createNode(left, NULL), NULL);
}
| kAUTOMATIC_FUNCTION
{
    $$ = dcList_createWithObjects(dcPair_createNode
                                  (dcString_createNodeWithString($1, false),
                                   NULL),
                                  NULL);
}

method_parameter_keyword: tWORD
method_parameter_const_keyword: kSUPER | kCLASS

method_parameter: tMETHOD_PARAMETER expression_rhs
{
    $$ = dcNode_setTemplate
        (dcPair_createNode
         (dcNode_setTemplate(dcString_createNodeWithString($1, true), true),
          dcNode_setTemplate($2, true)),
         true);
    dcMemory_trackMemory($1);
}

//////////// end method call rules ///////////////////

///////////////////////// class rules ///////////////////////////////////////
class: class_keywords class_header class_data
{
    MyClassHeader *header = $2;
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
                                       $1,
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
    for (that = $3->head; that != NULL; that = that->next)
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

    dcList_free(&$3, DC_DEEP);
    dcList_pop(sClassNames, DC_DEEP);
    dcLexer_popScopeDataFlags(sLexer);
    $$ = result;
}

class_keywords: kCLASS
{
    $$ = NO_FLAGS;
}
| real_class_keywords kCLASS
{
    $$ = $1;
}
| kSLICE
{
    $$ = CLASS_SLICE;
}

real_class_keywords: real_real_class_keywords real_class_keywords
{
    $$ = $1 | $2;
}
| real_real_class_keywords

real_real_class_keywords: kABSTRACT
{
    $$ = CLASS_ABSTRACT;
}
| kATOMIC
{
    $$ = CLASS_ATOMIC;
}
| kFINAL
{
    $$ = CLASS_FINAL;
}
| kSINGLETON
{
    $$ = CLASS_SINGLETON;
}

// class with $3 as its superclass //
class_header: tWORD '(' class_path ')' class_header_end
{
    // add class name to help with package name for composited classes
    pushClassName($1);

    // TODO: keep class_path a dcList all the way through
    dcPackage *tempPackage = dcPackage_create($3);
    $$ = createClassHeader($1,
                           dcPackage_getPathString(tempPackage),
                           dcLexer_getCurrentScopeDataFlags(sLexer));
    dcPackage_free(&tempPackage);
    dcLexer_pushScopeDataFlags(sLexer);
}
// class with Object as its superclass //
| tWORD class_header_end
{
    pushClassName($1);
    $$ = createClassHeader($1,
                           dcMemory_strdup(MAKE_FULLY_QUALIFIED(OBJECT)),
                           dcLexer_getCurrentScopeDataFlags(sLexer));
    dcLexer_pushScopeDataFlags(sLexer);
}

class_header_end: expr_end_left_brace

class_data: metaclass_object class_data
{
    $$ = $2;
    dcList_unshift($2, $1);
}
| instance_object class_data
{
    $$ = $2;
    dcList_unshift($2, $1);
}
| class_method class_data
{
    $$ = $2;
    dcList_unshift($2, $1);
}
| scope_data_class class_data
{
    $$ = $2;
    dcList_unshift($2, $1);
}
| tEXPR_END class_data
{
    $$ = $2;
}
| '}'
{
    $$ = dcList_create();
}

scope_data_class: class
{
    $$ = dcScopeData_createNode(dcClass_getName($1),
                                $1,
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_OBJECT
                                 | SCOPE_DATA_META));
}

instance_object: tINSTANCE_SCOPED_IDENTIFIER
{
    $$ = dcScopeData_createNode($1,
                                dcNil_createNode(),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_INSTANCE
                                 | SCOPE_DATA_OBJECT));
    dcMemory_trackMemory($1);
}
| tINSTANCE_SCOPED_IDENTIFIER ',' class_object_accessor
{
    $$ = dcScopeData_createNode($1,
                                dcNil_createNode(),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_INSTANCE
                                 | SCOPE_DATA_OBJECT
                                 | $3));
    dcMemory_trackMemory($1);
}

metaclass_object: tMETA_SCOPED_IDENTIFIER
{
    $$ = dcScopeData_createNode($1,
                                dcNil_createNode(),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_META
                                 | SCOPE_DATA_OBJECT));
    dcMemory_trackMemory($1);
}
| tMETA_SCOPED_IDENTIFIER ',' class_object_accessor
{
    $$ = dcScopeData_createNode($1,
                                dcNil_createNode(),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_META
                                 | SCOPE_DATA_OBJECT
                                 | $3));
    dcMemory_trackMemory($1);
}

class_object_accessor: kR
{
    $$ = SCOPE_DATA_READER;
    dcMemory_trackMemory($1);
}
| kW
{
    $$ = SCOPE_DATA_WRITER;
    dcMemory_trackMemory($1);
}
| kRW
{
    $$ = SCOPE_DATA_READER | SCOPE_DATA_WRITER;
    dcMemory_trackMemory($1);
}

method: method_head '{' statement '}'
{
    dcNode *methodNode = dcScopeData_getObject($1);
    dcProcedureClass_setBody(methodNode, dcGraphDataTree_createNode($3));
    dcNode_free(&$1, DC_SHALLOW);
    $$ = methodNode;
}
| method_head '{' '}'
{
    dcNode *methodNode = dcScopeData_getObject($1);
    dcProcedureClass_setBody(methodNode,
                             dcGraphDataTree_createNode(dcNil_createNode()));
    dcNode_free(&$1, DC_SHALLOW);
    $$ = methodNode;
}

class_method: method_head '{' statement '}'
{
    dcProcedureClass_setBody(dcScopeData_getObject($1),
                             dcGraphDataTree_createNode($3));
}
| method_head '{' '}'
{
    dcProcedureClass_setBody(dcScopeData_getObject($1),
                             dcGraphDataTree_createNode(dcNil_createNode()));
}

method_head: method_headP
| method_headP method_attributes
{
    dcScopeData_updateFlags(CAST_SCOPE_DATA($1), $2);
}

method_attribute: kMETHOD_ATTRIBUTE_BREAKTHROUGH
{
    $$ = SCOPE_DATA_BREAKTHROUGH;
}
| kMETHOD_ATTRIBUTE_SYNCHRONIZED
{
    $$ = SCOPE_DATA_SYNCHRONIZED;
}
| kMETHOD_ATTRIBUTE_SYNCHRONIZED_READ
{
    $$ = SCOPE_DATA_SYNCHRONIZED_READ;
}
| kMETHOD_ATTRIBUTE_SYNCHRONIZED_WRITE
{
    $$ = SCOPE_DATA_SYNCHRONIZED_WRITE;
}
| kMETHOD_ATTRIBUTE_CONST
{
    $$ = SCOPE_DATA_CONST;
}
| kMETHOD_ATTRIBUTE_CONTAINER_LOOP
{
    $$ = SCOPE_DATA_CONTAINER_LOOP;
}
| kMETHOD_ATTRIBUTE_MODIFIES_CONTAINER
{
    $$ = SCOPE_DATA_MODIFIES_CONTAINER;
}

method_attributes: method_attribute ',' method_attributes
{
    $$ = $1 | $3;
}
| method_attribute

method_headP: tMETHOD_INSTANCE method_header
{
    $$ = dcScopeData_createNode($2->name,
                                dcNode_setTemplate
                                (dcProcedureClass_createObject(NULL, $2), true),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_INSTANCE
                                 | SCOPE_DATA_METHOD));
}
| tMETHOD_META method_header
{
    $$ = dcScopeData_createNode($2->name,
                                dcNode_setTemplate
                                (dcProcedureClass_createObject(NULL, $2), true),
                                (dcLexer_getCurrentScopeDataFlags(sLexer)
                                 | SCOPE_DATA_META
                                 | SCOPE_DATA_METHOD));
}

%%

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
