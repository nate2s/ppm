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

#ifndef __DC_MATH_H__
#define __DC_MATH_H__

#include "dcDefines.h"

struct dcFlatArithmetic_t
{
    dcTaffyOperator taffyOperator;
    struct dcList_t *values;
    bool grouped;
};

typedef struct dcFlatArithmetic_t dcFlatArithmetic;

//////////////////
// initializing //
//////////////////

void dcFlatArithmetic_initialize(void);
void dcFlatArithmetic_deinitialize(void);

//////////////
// creating //
//////////////

struct dcNode_t *dcFlatArithmetic_createNode(dcTaffyOperator _operator);
struct dcNode_t *dcFlatArithmetic_createNodeWithList(dcTaffyOperator _operator,
                                                     struct dcList_t *_values);
struct dcNode_t *dcFlatArithmetic_createNodeWithValues
    (dcTaffyOperator _operator,
     struct dcNode_t *_value,
     ...);

struct dcNode_t *dcFlatArithmetic_createNodeFromGraph(struct dcNode_t *_graph);

/////////////
// copying //
/////////////

dcFlatArithmetic *dcFlatArithmetic_copy(const dcFlatArithmetic *_arithmetic,
                                        dcDepth _depth);

struct dcList_t *dcFlatArithmetic_getValues(struct dcNode_t *_flatArithmetic);

/////////////////
// conversions //
/////////////////

typedef struct dcNode_t *(*dcFlatArithmeticOperation)(struct dcNode_t *_node,
                                                      bool *_modified);

struct dcNode_t *dcFlatArithmetic_factorWithSymbol(struct dcNode_t *_node,
                                                   const char *_symbol,
                                                   bool *_modified);

#define SIMPLIFY_FUNCTION(_function)                                    \
    struct dcNode_t *_function(struct dcNode_t *_node, bool *_modified);

SIMPLIFY_FUNCTION(dcFlatArithmetic_shrink);

SIMPLIFY_FUNCTION(dcFlatArithmetic_snip);
SIMPLIFY_FUNCTION(dcFlatArithmetic_combine);
SIMPLIFY_FUNCTION(dcFlatArithmetic_convert);
SIMPLIFY_FUNCTION(dcFlatArithmetic_multiplyByDenominator);
SIMPLIFY_FUNCTION(dcFlatArithmetic_collectPowers);
SIMPLIFY_FUNCTION(dcFlatArithmetic_convertSubtractToAdd);
SIMPLIFY_FUNCTION(dcFlatArithmetic_undoConvertSubtractToAdd);
SIMPLIFY_FUNCTION(dcFlatArithmetic_undoConvertSubtractToAdd);
SIMPLIFY_FUNCTION(dcFlatArithmetic_convertDivideToMultiply);
SIMPLIFY_FUNCTION(dcFlatArithmetic_expandRaise);
SIMPLIFY_FUNCTION(dcFlatArithmetic_expandDivide);
SIMPLIFY_FUNCTION(dcFlatArithmetic_orderPolynomial);
SIMPLIFY_FUNCTION(dcFlatArithmetic_shrunkenOrderPolynomial);
SIMPLIFY_FUNCTION(dcFlatArithmetic_orderSubtract);
SIMPLIFY_FUNCTION(dcFlatArithmetic_moveNumberToFront);
SIMPLIFY_FUNCTION(dcFlatArithmetic_merge);
SIMPLIFY_FUNCTION(dcFlatArithmetic_multiFactor);
SIMPLIFY_FUNCTION(dcFlatArithmetic_cancel);
SIMPLIFY_FUNCTION(dcFlatArithmetic_factor);
SIMPLIFY_FUNCTION(dcFlatArithmetic_factorDivideWithRationalRoots);
SIMPLIFY_FUNCTION(dcFlatArithmetic_factorDivideWithQuadratic);
SIMPLIFY_FUNCTION(dcFlatArithmetic_factorPolynomialByGcd);
SIMPLIFY_FUNCTION(dcFlatArithmetic_factorPolynomialByGrouping);
SIMPLIFY_FUNCTION(dcFlatArithmetic_distribute);
SIMPLIFY_FUNCTION(dcFlatArithmetic_distributeLikeAMadman);
SIMPLIFY_FUNCTION(dcFlatArithmetic_distributeDivide);
SIMPLIFY_FUNCTION(dcFlatArithmetic_expand);
SIMPLIFY_FUNCTION(dcFlatArithmetic_simplifyTrigonometry);
SIMPLIFY_FUNCTION(dcFlatArithmetic_simplifyMethodCall);
SIMPLIFY_FUNCTION(dcFlatArithmetic_factorQuadratic);
SIMPLIFY_FUNCTION(dcFlatArithmetic_factorQuadraticWhatever);

bool dcFlatArithmetic_isPolynomial(struct dcNode_t *_node);

bool dcFlatArithmetic_newSolve(const char *_x,
                               struct dcNode_t **_left,
                               struct dcNode_t **_right);

struct dcNode_t *dcFlatArithmetic_solve(const char *_x,
                                        struct dcNode_t *_leftHandSide,
                                        struct dcNode_t *_rightHandSide);

struct dcNode_t *dcFlatArithmetic_reallyShrink(struct dcNode_t *_arithmetic,
                                               bool _factor,
                                               bool *_modified);

void dcFlatArithmetic_sort(struct dcNode_t *_arithmetic);

dcResult dcFlatArithmetic_print(const dcFlatArithmetic *_arithmetic,
                                struct dcString_t *_stream);
const char *dcFlatArithmetic_display(const dcFlatArithmetic *_arithmetic);

typedef struct dcNode_t *(*dcFlatArithmetic_calculusOperationFunction)
    (struct dcNode_t *_arithmetic, const char *_symbol);

struct dcNode_t *dcFlatArithmetic_derive(struct dcNode_t *_arithmetic,
                                         const char *_symbol);

bool dcFlatArithmetic_dividePolynomials
    (struct dcNode_t *_dividend,
     struct dcNode_t *_divisor,
     struct dcList_t *_symbols,
     struct dcNode_t **_quotient,
     struct dcNode_t **_remainder);

struct dcNode_t *dcFlatArithmetic_newDerive(struct dcNode_t *_node,
                                            const char *_symbol);

typedef enum
{
    REMOVE_FOR_SUBSTITUTION,
    REMOVE_BY_PARTS
} RemoveType;

bool dcFlatArithmetic_remove(struct dcNode_t **_arithmetic, // in/out
                             struct dcNode_t *_toRemove,    // in
                             const char *_symbol);

bool dcFlatArithmetic_cancelTopAndBottom(struct dcNode_t **_top,
                                         struct dcNode_t **_bottom,
                                         bool _factor);

struct dcNode_t *dcFlatArithmetic_integrate(struct dcNode_t *_toDerive,
                                            const char *_symbol);
struct dcNode_t *dcFlatArithmetic_integrateByParts(struct dcNode_t *_node,
                                                   const char *_symbol);

struct dcNode_t *dcFlatArithmetic_compose(const struct dcNode_t *_arithmetic,
                                          const struct dcHash_t *_composements);

dcResult dcFlatArithmetic_prettyPrint(const dcFlatArithmetic *_arithmetic,
                                      struct dcCharacterGraph_t **_graph);

bool dcFlatArithmetic_findIdentifier(struct dcNode_t **_node,
                                     const char *_identifier,
                                     struct dcNode_t *_replacement);

bool dcFlatArithmetic_containsIdentifier(struct dcNode_t *_node,
                                         const char *_identifier);

const char *dcFlatArithmetic_prettyDisplay(const dcFlatArithmetic *_arithmetic);

bool dcFlatArithmetic_degree(struct dcNode_t *_node,
                             struct dcList_t *_symbols,
                             int32_t *_degree);

bool dcFlatArithmetic_shrunkenDegree(struct dcNode_t *_node,
                                     struct dcList_t *_symbols,
                                     int32_t *_degree);

// note: _node must be shrunk, sorted, have a single identifier,
// and must be converted to add
struct dcArray_t *dcFlatArithmetic_getOrderedPolynomialCoefficients
    (struct dcNode_t *_node, bool _needIntegers);

// note: _node must be shrunk, and must be converted to add
struct dcArray_t *dcFlatArithmetic_getPolynomialCoefficients
    (struct dcNode_t *_node, bool _needIntegers);

// note: _node must be shrunk, sorted, and must be converted to add
struct dcNode_t *dcFlatArithmetic_factorPolynomialByRationalRoots
    (struct dcNode_t *_node,
     bool *_modified);

// same as above
struct dcNode_t *dcFlatArithmetic_factorDifferenceOfSquares
    (struct dcNode_t *_node,
     bool *_modified);

// you guessed it...
struct dcNode_t *dcFlatArithmetic_factorDifferenceOfCubes
    (struct dcNode_t *_node,
     bool *_modified);

// note: this mucks with _arithmetic
// _value is not mucked with
struct dcNode_t *dcFlatArithmetic_factorValue(struct dcNode_t *_arithmetic,
                                              struct dcNode_t *_value,
                                              bool *_modified);

void dcFlatArithmetic_choose(struct dcNode_t *_arithmetic,  // input
                             const char *_symbol,           // input
                             uint32_t _count,               // input
                             RemoveType _chooseType,        // input
                             struct dcList_t *_results,     // output
                             struct dcHash_t *_parents);    // output

bool dcFlatArithmetic_unfindableSubstitution(struct dcNode_t *_program, // in
                                             const char *_symbol);      // in

struct dcNode_t *dcFlatArithmetic_substitute
    (struct dcNode_t *_program,              // input
     const char *_symbol,                    // input
     const char *_substituteSymbol,          // input
     struct dcNode_t **_substitution);       // output

uint32_t dcFlatArithmetic_symbolCount(struct dcNode_t *_node,
                                      const char *_symbol);

bool dcFlatArithmetic_orderForIlate(struct dcNode_t *_left,
                                    struct dcNode_t *_right,
                                    const char *_symbol);

#define QUERY_FUNCTION(_name)                                           \
    bool dcFlatArithmetic_##_name(struct dcNode_t *_node,               \
                                  const char *_symbol);

QUERY_FUNCTION(isInverseTrigonometric);
QUERY_FUNCTION(isAlgebraic);
QUERY_FUNCTION(isLogarithmic);
QUERY_FUNCTION(isTrigonometric);
QUERY_FUNCTION(isExponential);

bool dcFlatArithmetic_hasSingleIdentifier(struct dcNode_t *_node,
                                          struct dcNode_t **_identifier);

TAFFY_DEBUG(struct dcHash_t *dcFlatArithmetic_getArithmeticCounts(void);
            const char *dcFlatArithmetic_getArithmeticLanguageName
                (struct dcNode_t *_node);
            void dcFlatArithmetic_printCounts(void));

bool dcFlatArithmetic_equals(struct dcNode_t *_left, struct dcNode_t *_right);

bool dcFlatArithmetic_deltaEquals(struct dcNode_t *_left,
                                  struct dcNode_t *_right,
                                  struct dcNode_t *_delta);

bool dcFlatArithmetic_find(struct dcNode_t *_arithmetic,
                           struct dcNode_t *_node);

char *dcFlatArithmetic_displayReal(struct dcNode_t *_node);

bool dcFlatArithmetic_maxPower(struct dcNode_t *_node, uint32_t *_power);

bool dcFlatArithmetic_unfindableIntegral(struct dcNode_t *_node,
                                         const char *_symbol,
                                         bool _search);

dcResult dcFlatArithmetic_compile(struct dcNode_t **_node,
                                  struct dcList_t *_symbols,
                                  bool *_changed);

bool dcFlatArithmetic_isSingleRoot(struct dcNode_t *_node,
                                   struct dcNode_t *_value,
                                   const char *_symbol);

void dcFlatArithmetic_moveLeftAndRight(struct dcNode_t *_arithmetic,
                                       const char *_symbol,
                                       struct dcNode_t **_left,
                                       struct dcNode_t **_right);

void dcFlatArithmetic_populateIdentifiers(struct dcNode_t *_arithmetic,
                                          struct dcList_t *_identifiers);

// merge divides into each other
void dcFlatArithmetic_mergeDivide(dcFlatArithmetic *_arithmetic);

bool dcFlatArithmetic_isMe(const struct dcNode_t *_node);

// true if grouped
bool dcFlatArithmetic_isGrouped(const struct dcNode_t *_node);

#ifdef COMPILE_TESTS
// how many times was each shrink operation called?
void dcFlatArithmetic_printShrinkCounts(void);
#endif

////////////////////////
// standard functions //
////////////////////////

COMPARE_FUNCTION(dcFlatArithmetic_compareNode);
COPY_FUNCTION(dcFlatArithmetic_copyNode);
FREE_FUNCTION(dcFlatArithmetic_freeNode);
HASH_FUNCTION(dcFlatArithmetic_hashNode);
MARSHALL_FUNCTION(dcFlatArithmetic_marshallNode);
PRETTY_PRINT_FUNCTION(dcFlatArithmetic_prettyPrintNode);
PRINT_FUNCTION(dcFlatArithmetic_printNode);
UNMARSHALL_FUNCTION(dcFlatArithmetic_unmarshallNode);

#endif
