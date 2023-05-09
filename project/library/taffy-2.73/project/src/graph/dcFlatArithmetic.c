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
#include <stdarg.h>
#include <string.h>

#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcCharacterGraph.h"
#include "dcClass.h"
#include "dcComplexNumberClass.h"
#include "dcDoubleVoid.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcGraphData.h"
#include "dcGraphDataTree.h"
#include "dcHash.h"
#include "dcIdentifier.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcLog.h"
#include "dcFlatArithmetic.h"
#include "dcFunctionClass.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcMethodCall.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcNumberClass.h"
#include "dcPair.h"
#include "dcParser.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringCache.h"
#include "dcSymbolClass.h"
#include "dcSystem.h"
#include "dcThread.h"
#include "dcUnsignedInt32.h"
#include "dcUnsignedInt64.h"
#include "dcVoid.h"
#include "dcVoidContainer.h"

#define IS_ADD(_object)                                             \
    (IS_FLAT_ARITHMETIC(_object)                                    \
     && CAST_FLAT_ARITHMETIC(_object)->taffyOperator == TAFFY_ADD)

#define IS_SUBTRACT(_object)                                        \
    (IS_FLAT_ARITHMETIC(_object)                                    \
     && CAST_FLAT_ARITHMETIC(_object)->taffyOperator == TAFFY_SUBTRACT)

#define IS_DIVIDE(_object)                                          \
    (IS_FLAT_ARITHMETIC(_object)                                    \
     && CAST_FLAT_ARITHMETIC(_object)->taffyOperator == TAFFY_DIVIDE)

#define IS_MULTIPLY(_object)                                        \
    (IS_FLAT_ARITHMETIC(_object)                                    \
     && CAST_FLAT_ARITHMETIC(_object)->taffyOperator == TAFFY_MULTIPLY)

#define IS_RAISE(_object)                                           \
    (IS_FLAT_ARITHMETIC(_object)                                    \
     && CAST_FLAT_ARITHMETIC(_object)->taffyOperator == TAFFY_RAISE)

#define FLAT_ARITHMETIC_HEAD(_object)                   \
    CAST_FLAT_ARITHMETIC(_object)->values->head->object

#define FLAT_ARITHMETIC_TAIL(_object)                   \
    CAST_FLAT_ARITHMETIC(_object)->values->tail->object

#define FLAT_ARITHMETIC_OPERATOR(_object)       \
    CAST_FLAT_ARITHMETIC(_object)->taffyOperator

dcNode *getTail(dcNode *_node)
{
    return FLAT_ARITHMETIC_TAIL(_node);
}

#define FLAT_ARITHMETIC_SIZE(_object)           \
    CAST_FLAT_ARITHMETIC(_object)->values->size

#define FOR_EACH(_arithmetic, _iterator)                                \
    dcListElement *_iterator;                                           \
    for (_iterator = _arithmetic->values->head;                         \
         _iterator != NULL;                                             \
         _iterator = _iterator->next)

#define FOR_EACH_IN_NODE(_nodeArithmetic, _iterator)                    \
    dcListElement *_iterator;                                           \
    FOR_EACH_IN_NODE_AGAIN(_nodeArithmetic, _iterator)

#define FOR_EACH_IN_NODE_AGAIN(_nodeArithmetic, _iterator)                   \
    for (_iterator = CAST_FLAT_ARITHMETIC(_nodeArithmetic)->values->head; \
         _iterator != NULL;                                             \
         _iterator = _iterator->next)

#define QUOTE(x) #x

#define SHRINK_OPERATION(_name, _result, _changed)              \
    _result = shrinkOperation(QUOTE(_name),                     \
                              _result,                          \
                              dcFlatArithmetic_##_name,         \
                              _changed)

// forward declarations
typedef dcNode *(*IntegrateOperation)(dcNode *_node, const char *_symbol);
static const char *indent(void);
static dcNode *integrate(dcNode *_shrunk,
                         const char *_symbol,
                         IntegrateOperation _exceptThis);
static dcNode *getCoefficient(dcNode *_arithmetic);

#define MAX_INTEGRATE_DEPTH 45
#define MAX_INTEGRATE_BY_PARTS_DEPTH 2

#define CREATE_SUBTRACT(...)                                            \
    dcFlatArithmetic_createNodeWithValues(TAFFY_SUBTRACT, __VA_ARGS__)

#define CREATE_RAISE(...)                                           \
    dcFlatArithmetic_createNodeWithValues(TAFFY_RAISE, __VA_ARGS__)

#define CREATE_ADD(...)                                             \
    dcFlatArithmetic_createNodeWithValues(TAFFY_ADD, __VA_ARGS__)

#define CREATE_MULTIPLY(...)                                            \
    dcFlatArithmetic_createNodeWithValues(TAFFY_MULTIPLY, __VA_ARGS__)

#define CREATE_DIVIDE(...)                                              \
    dcFlatArithmetic_createNodeWithValues(TAFFY_DIVIDE, __VA_ARGS__)

TAFFY_HIDDEN dcNode *createNumber(int32_t _value)
{
    return dcNode_setTemplate(dcNumberClass_createObjectFromInt32s(_value),
                              true);
}

TAFFY_HIDDEN dcNode *createNumberFromDouble(double _value)
{
    return dcNode_setTemplate(dcNumberClass_createObjectFromDouble(_value),
                              true);
}

typedef struct
{
    uint32_t depth;
    uint32_t integrateByPartsDepth;
    dcNode *originalNode;
    dcHashType originalNodeHashValue;
    dcString *originalNodeDisplay;
    dcStringCache *history;
    dcList *description;
    size_t calculations;
} IntegrationData;

static dcMutex *sIntegrateMutex = NULL;
static dcHash *sIntegrateHash = NULL;

#ifdef ENABLE_DEBUG
static dcHash *sCounts = NULL;

static void addCount(const char *_name)
{
    dcNode *already = NULL;

    if (dcHash_getValueWithStringKey(sCounts, _name, &already)
        == TAFFY_SUCCESS)
    {
        assert(already->type == NODE_INT);
        CAST_INT(already)++;
    }
    else
    {
        dcHash_setValueWithStringKey(sCounts,
                                     _name,
                                     dcUnsignedInt32_createNode(1));
    }
}

void dcFlatArithmetic_printShrinkCounts(void)
{
    dcHashIterator *i = dcHash_createIterator(sCounts);
    dcNode *that = NULL;

    fprintf(stderr, "Flat Arithmetic Counts:\n");

    while ((that = dcHashIterator_getNext(i))
           != NULL)
    {
        dcHashElement *element = CAST_HASH_ELEMENT(that);
        fprintf(stderr,
                "%s: %u\n",
                element->key.keyUnion.stringKey,
                CAST_INT(element->value));
    }
}
#else // ENABLE_DEBUG
#    define addCount(x)
#endif // ENABLE_DEBUG

#define MAX_SHRINK_CACHE_SIZE 50

static dcStringCache *sIntegrateCache = NULL;
static dcStringCache *sShrinkCache = NULL;
static dcStringCache *sMultiFactorCache = NULL;
static dcStringCache *sFactorCache = NULL;
static dcStringCache *sCancelCache = NULL;
static dcStringCache *sSubstituteCache = NULL;
static dcStringCache *sDeriveCache = NULL;

// an identifier
static dcNode *sLeftHandSide;

#ifdef TAFFY_DEBUG
static char sIndents[MAX_INTEGRATE_DEPTH + 2][MAX_INTEGRATE_DEPTH + 2];

dcHash *sArithmeticCounts = NULL;

dcHash *dcFlatArithmetic_getArithmeticCounts(void)
{
    return sArithmeticCounts;
}
#endif // TAFFY_DEBUG

//
// The Flat Arithmetic Language
//
// Symbol Description
// $1     A node, matches a $1 on the other side
// ?      Anything
// N      A Number or ComplexNumber
// %X     A specific number X, like %1, %2, etc
// I      Identifier
// +      Plus Arithmetic
// -      Subtract Arithmetic
// *      Multiply Arithmetic
// /      Divide Arithmetic
// ^      Raise Arithmetic
// M      Method call
// S      Matches the symbol
// C      If first occurence, it matches. If second+ occurence, matches if it
//        matches the first occurence.
// e      e
// ln     ln
// sec    sec(S)
//
// Example for the expression: x + x * 2:
//
// Type: TAFFY_ADD
// Left:  $1
// Right: * == 2 $1 N
//
// Matches a node ($1) with a multiply arithmetic of size 2,
// containing two elements: the same node ($1) and some Number
//
// Example for the expression: x^a * x^b, for a and b as NumberS:
//
// Type: TAFFY_MULTIPLY
// Left:  ^ == 2 $1 N
// Right: ^ == 2 $1 N
//
// Matches two multiply arithmetics of size two, containing two elements:
// the same node ($1), and some Number (each Number may differ)
//

typedef struct
{
    dcGraphDataType type;
    dcTaffyOperator taffyOperator;
} TypeOperatorMap;

dcList *dcFlatArithmetic_getValues(dcNode *_flatArithmetic)
{
    return CAST_FLAT_ARITHMETIC(_flatArithmetic)->values;
}

TAFFY_HIDDEN dcFlatArithmetic *createFlatArithmetic(dcTaffyOperator _operator)
{
    dcFlatArithmetic *flatArithmetic =
        (dcFlatArithmetic *)(dcMemory_allocateAndInitialize
                             (sizeof(dcFlatArithmetic)));
    flatArithmetic->taffyOperator = _operator;
    return flatArithmetic;
}

dcNode *dcFlatArithmetic_createNode(dcTaffyOperator _operator)
{
    return dcFlatArithmetic_createNodeWithList(_operator, dcList_create());
}

dcNode *dcFlatArithmetic_createNodeWithList(dcTaffyOperator _operator,
                                            dcList *_values)
{
    dcFlatArithmetic *arithmetic = createFlatArithmetic(_operator);
    arithmetic->values = _values;
    arithmetic->grouped = false;
    dcNode *result =
        dcGraphData_createNodeWithGuts(NODE_FLAT_ARITHMETIC, arithmetic);
    dcNode_trackCreation(result);
    return result;
}

static void setModified(bool *_modified)
{
    if (_modified != NULL)
    {
        *_modified = true;
    }
}

TAFFY_HIDDEN dcNode *popIfNotFactorial(dcNode *_node, bool *_modified)
{
    dcNode *result = _node;

    if (_node != NULL)
    {
        if (IS_FLAT_ARITHMETIC(result))
        {
            FOR_EACH_IN_NODE(result, that)
            {
                that->object = popIfNotFactorial(that->object, _modified);
            }
        }

        while (IS_FLAT_ARITHMETIC(result)
               && CAST_FLAT_ARITHMETIC(result)->values->size == 1
               && (CAST_FLAT_ARITHMETIC(result)->taffyOperator
                   != TAFFY_FACTORIAL))
        {
            setModified(_modified);
            dcNode *save = dcList_pop(CAST_FLAT_ARITHMETIC(result)->values,
                                      DC_SHALLOW);
            dcNode_free(&result, DC_DEEP);
            result = save;
        }
    }

    return result;
}

static dcNode *mergeSubtract(dcNode *_node, bool *_modified)
{
    // change (a - b) - c to a - b - c
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);

    while (IS_SUBTRACT(arithmetic->values->head->object))
    {
        setModified(_modified);

        dcNode *heady = dcList_shift(arithmetic->values, DC_SHALLOW);
        dcFlatArithmetic *arithy = CAST_FLAT_ARITHMETIC(heady);
        dcListElement *that;

        for (that = arithy->values->tail; that != NULL; that = that->previous)
        {
            dcList_unshift(arithmetic->values, that->object);
        }

        dcNode_free(&heady, DC_SHALLOW);
    }

    return _node;
}

static dcNode *mergeAssociative(dcNode *_node, bool *_modified)
{
    dcListElement *that;
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);

    for (that = arithmetic->values->head; that != NULL; )
    {
        dcListElement *next = that->next;

        if (IS_FLAT_ARITHMETIC(that->object)
            && (CAST_FLAT_ARITHMETIC(that->object)->taffyOperator
                == arithmetic->taffyOperator))
        {
            dcFlatArithmetic *other = CAST_FLAT_ARITHMETIC(that->object);
            setModified(_modified);

            while (other->values->size > 0)
            {
                dcList_insertBeforeListElement
                    (arithmetic->values,
                     that,
                     dcList_shift(other->values, DC_SHALLOW));
            }

            dcList_removeElement(arithmetic->values, &that, DC_DEEP);
        }

        that = next;
    }

    return popIfNotFactorial(_node, _modified);
}

typedef dcNode *(*ShrinkOperation)(dcNode *_arithmetic, bool *_changed);

static dcNode *shrinkOperation(const char *_name,
                               dcNode *_result,
                               ShrinkOperation _operation,
                               bool *_changed)
{
    char *previousDisplay = NULL;
    char *realDisplay = NULL;

    if (dcLog_isEnabled(FLAT_ARITHMETIC_VERBOSE_LOG))
    {
        previousDisplay = dcNode_synchronizedDisplay(_result);
        realDisplay = dcFlatArithmetic_displayReal(_result);
    }

    bool myChanged = false;
    _result = _operation(_result, &myChanged);

    if (myChanged)
    {
        setModified(_changed);

        if (dcLog_isEnabled(FLAT_ARITHMETIC_VERBOSE_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_VERBOSE_LOG,
                      "[%s]: '%s' after %s: '%s'\n",
                      (_changed == NULL
                       ? "disabled"
                       : "enabled"),
                      previousDisplay,
                      _name,
                      dcNode_display(_result));
            dcMemory_free(realDisplay);
            dcMemory_free(previousDisplay);
        }
    }

    // return cleanup
    return popIfNotFactorial(_result, _changed);
}

// merge add and multiply up, like:
// 3 + (1 + 2) => 3 + 1 + 2
// a * b * (c * d) => a * b * c *d
dcNode *dcFlatArithmetic_merge(dcNode *_node, bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    FOR_EACH_IN_NODE(_node, that)
    {
        that->object = dcFlatArithmetic_merge(that->object, _modified);
    }

    if (IS_ADD(_node) || IS_MULTIPLY(_node))
    {
        return mergeAssociative(_node, _modified);
    }
    else if (IS_SUBTRACT(_node))
    {
        return mergeSubtract(_node, _modified);
    }

    return _node;
}

dcNode *dcFlatArithmetic_createNodeWithValues(dcTaffyOperator _operator,
                                              dcNode *_first,
                                              ...)
{
    va_list vaList;
    va_start(vaList, _first);
    dcNode *result = dcFlatArithmetic_createNode(_operator);
    dcNode *iterator = va_arg(vaList, dcNode*);

    if (_first != NULL)
    {
        assert(dcNode_isTemplate(_first));
        dcList_push(CAST_FLAT_ARITHMETIC(result)->values, _first);

        for ( ; iterator != NULL; iterator = va_arg(vaList, dcNode *))
        {
            assert(dcNode_isTemplate(iterator));
            dcList_push(CAST_FLAT_ARITHMETIC(result)->values, iterator);
        }
    }

    va_end(vaList);
    return dcFlatArithmetic_merge(result, NULL);
}

void dcFlatArithmetic_free(dcFlatArithmetic **_arithmetic, dcDepth _depth)
{
    if (*_arithmetic != NULL)
    {
        dcList_free(&(*_arithmetic)->values, _depth);
        dcMemory_free(*_arithmetic);
    }
}

void dcFlatArithmetic_freeNode(dcNode *_node, dcDepth _depth)
{
    dcFlatArithmetic *flats = CAST_FLAT_ARITHMETIC(_node);
    dcList_free(&flats->values, _depth);
    dcMemory_free(flats);
}

dcFlatArithmetic *dcFlatArithmetic_copy(const dcFlatArithmetic *_from,
                                        dcDepth _depth)
{
    dcFlatArithmetic *to = createFlatArithmetic(_from->taffyOperator);
    to->values = dcList_copy(_from->values, _depth);
    to->grouped = _from->grouped;
    return to;
}

// create dcFlatArithmetic_copyNode
dcTaffy_createCopyNodeFunctionMacro(dcFlatArithmetic, CAST_FLAT_ARITHMETIC);

// create dcFlatArithmetic_printNode
dcTaffy_createPrintNodeFunctionMacro(dcFlatArithmetic, CAST_FLAT_ARITHMETIC);

// create dcFlatArithmetic_prettyPrintNode
dcTaffy_createPrettyPrintNodeFunctionMacro(dcFlatArithmetic,
                                           CAST_FLAT_ARITHMETIC);

// create dcFlatArithmetic_display
dcTaffy_createDisplayFunctionMacro(dcFlatArithmetic);

dcResult dcFlatArithmetic_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    dcHashType value = FLAT_ARITHMETIC_OPERATOR(_node) * 2398471;
    dcResult result = TAFFY_SUCCESS;

    FOR_EACH_IN_NODE(_node, that)
    {
        dcHashType thisValue = 0;
        result = dcNode_hash(that->object, &thisValue);

        if (result == TAFFY_SUCCESS)
        {
            value += thisValue;
        }
    }

    *_hashResult = value;
    return result;
}

static dcResult prettyPrintDivide(const dcFlatArithmetic *_arithmetic,
                                  dcCharacterGraph **_result);
static dcResult prettyPrintFlatArithmetic(const dcFlatArithmetic *_arithmetic,
                                          dcCharacterGraph **_result);
static dcResult prettyPrintRaise(const dcFlatArithmetic *_arithmetic,
                                 dcCharacterGraph **_result);

dcResult dcFlatArithmetic_prettyPrint(const dcFlatArithmetic *_arithmetic,
                                      dcCharacterGraph **_result)
{
    dcResult result = TAFFY_SUCCESS;

    if (_arithmetic->taffyOperator == TAFFY_DIVIDE)
    {
        result = prettyPrintDivide(_arithmetic, _result);
    }
    else if (_arithmetic->taffyOperator == TAFFY_RAISE)
    {
        result = prettyPrintRaise(_arithmetic, _result);
    }
    else
    {
        result = prettyPrintFlatArithmetic(_arithmetic, _result);
    }

    return result;
}

static dcResult prettyPrintDivide(const dcFlatArithmetic *_arithmetic,
                                  dcCharacterGraph **_result)
{
    if (_arithmetic->values->size <= 1)
    {
        return TAFFY_FAILURE;
    }

    dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
    dcListElement *that;
    dcResult result = TAFFY_SUCCESS;
    dcList *graphs = dcList_create();

    for (that = _arithmetic->values->head; that != NULL; that = that->next)
    {
        dcCharacterGraph *thatGraph;
        dcResult thatResult = dcNode_prettyPrint(that->object, &thatGraph);
        if (thatResult != TAFFY_SUCCESS)
        {
            dcCharacterGraph_free(&thatGraph);
            result = thatResult;
            break;
        }

        dcList_push(graphs, dcVoid_createNode(thatGraph));
    }

    if (result == TAFFY_SUCCESS)
    {
        for (that = graphs->head->next; that != NULL; that = that->next)
        {
            dcCharacterGraph *bottom =
                (dcCharacterGraph *)CAST_VOID(that->object);
            dcCharacterGraph *top =
                (dcCharacterGraph *)CAST_VOID(that->previous->object);

            // add 2 for prettiness
            uint32_t linesWidth = dcTaffy_max(top->width, bottom->width) + 2;
            if (linesWidth % 2 == 0)
            {
                linesWidth++;
            }

            // +1 for terminator
            char *lines = (char *)(dcMemory_allocate
                                   (sizeof(char) * linesWidth + 1));
            uint32_t i;

            for (i = 0; i < linesWidth; i++)
            {
                lines[i] = '-';
            }

            lines[i] = 0;

            dcCharacterGraph_insertCharacterGraphDown
                (graph,
                 (linesWidth / 2) - (top->width / 2),
                 (that == graphs->head->next
                  ? graph->height - 1
                  : graph->height),
                 top);

            dcCharacterGraph *linesGraph =
                dcCharacterGraph_createFromCharString(lines);
            dcCharacterGraph_appendCharacterGraph(graph, linesGraph);

            dcCharacterGraph_free(&linesGraph);
            dcMemory_free(lines);

            dcCharacterGraph_insertCharacterGraphDown
                (graph,
                 (linesWidth / 2) - (bottom->width / 2),
                 graph->height,
                 bottom);
        }
    }

    while (graphs->size > 0)
    {
        dcNode *voidNode = dcList_pop(graphs, DC_SHALLOW);
        dcCharacterGraph *thisGraph = (dcCharacterGraph *)CAST_VOID(voidNode);
        dcCharacterGraph_free(&thisGraph);
        dcNode_free(&voidNode, DC_SHALLOW);
    }

    dcList_free(&graphs, DC_DEEP);

    if (result == TAFFY_SUCCESS)
    {
        *_result = graph;
    }
    else
    {
        dcCharacterGraph_free(&graph);
    }

    return result;
}

static dcResult prettyPrintRaise(const dcFlatArithmetic *_arithmetic,
                                 dcCharacterGraph **_result)
{
    dcResult result = TAFFY_SUCCESS;
    dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);

    FOR_EACH_IN_LIST(_arithmetic->values, that)
    {
        dcCharacterGraph *graphResult;
        dcResult thatResult = dcNode_prettyPrint(that->object, &graphResult);

        if (thatResult != TAFFY_SUCCESS)
        {
            result = thatResult;
            dcCharacterGraph_free(&graphResult);
            break;
        }

        if ((graphResult->width > 1 || graphResult->height > 1)
            && ! (IS_FLAT_ARITHMETIC(that->object)
                  && FLAT_ARITHMETIC_OPERATOR(that->object) == TAFFY_RAISE))
        {
            dcCharacterGraph_addParens(graphResult);
        }
        else if (IS_FLAT_ARITHMETIC(that->object))
        {
            if (IS_DIVIDE(that->object)
                && CAST_FLAT_ARITHMETIC(that->object)->values->size > 1)
            {
                dcCharacterGraph_addParens(graphResult);
            }
        }

        // insert it at the top-right, going up
        // don't indent the first element
        dcCharacterGraph_insertCharacterGraphUp
            (graph,
             (that == _arithmetic->values->head
              ? 0
              : graph->width),
             (that == _arithmetic->values->head
              ? 0
              : -1),
             graphResult);

        dcCharacterGraph_free(&graphResult);
    }

    if (result == TAFFY_SUCCESS)
    {
        *_result = graph;
    }
    else
    {
        *_result = NULL;
        dcCharacterGraph_free(&graph);
    }

    return result;
}

static dcResult prettyPrintFlatArithmetic(const dcFlatArithmetic *_arithmetic,
                                          dcCharacterGraph **_result)
{
    dcListElement *that;
    dcResult result = TAFFY_SUCCESS;
    dcString *separator =
        dcString_createWithString
        (dcLexer_sprintf
         (" %s ",
          dcSystem_getOperatorSymbol(_arithmetic->taffyOperator)),
         false);
    *_result = dcCharacterGraph_create(1, 1);
    dcCharacterGraph *graph = *_result;

    for (that = _arithmetic->values->head;
         that != NULL && result == TAFFY_SUCCESS;
         that = that->next)
    {
        dcCharacterGraph *graphResult;
        bool needParens = false;

        dcResult thatResult = dcNode_prettyPrint(that->object, &graphResult);

        if (IS_FLAT_ARITHMETIC(that->object))
        {
            dcTaffyOperator thatOperator =
                CAST_FLAT_ARITHMETIC(that->object)->taffyOperator;
            dcTaffyOperator myOperator = _arithmetic->taffyOperator;

            if (graphResult->height > 3
                || (myOperator == TAFFY_MULTIPLY
                    && (thatOperator != TAFFY_MULTIPLY
                        && thatOperator != TAFFY_DIVIDE
                        && thatOperator != TAFFY_RAISE))
                || (myOperator == TAFFY_ADD
                    && thatOperator == TAFFY_SUBTRACT))
            {
                needParens = true;
            }
        }
        else if (graphResult->height > 3)
        {
            needParens = true;
        }

        if (thatResult == TAFFY_SUCCESS)
        {
            if (that != _arithmetic->values->head)
            {
                dcCharacterGraph_appendStringMiddle(graph, separator);
            }

            if (needParens)
            {
                dcCharacterGraph_addParens(graphResult);
            }

            dcCharacterGraph_appendMiddle(graph, graphResult);
        }
        else
        {
            dcCharacterGraph_free(&graphResult);
            result = thatResult;
            break;
        }

        dcCharacterGraph_free(&graphResult);
    }

    dcString_free(&separator, DC_DEEP);
    return result;
}

uint32_t dcFlatArithmetic_count(dcNode *_node)
{
    uint32_t result = 1;

    if (IS_FLAT_ARITHMETIC(_node))
    {
        FOR_EACH_IN_NODE(_node, i)
        {
            result += dcFlatArithmetic_count(i->object);
        }
    }

    return result;
}

dcResult dcFlatArithmetic_print(const dcFlatArithmetic *_arithmetic,
                                dcString *_stream)
{
    dcListElement *that;
    const char *operatorSymbol =
        dcSystem_getOperatorSymbol(_arithmetic->taffyOperator);
    const char *separator = (_arithmetic->taffyOperator == TAFFY_RAISE
                             ? ""
                             : " ");
    dcResult result = TAFFY_SUCCESS;

    for (that = _arithmetic->values->head;
         that != NULL && result == TAFFY_SUCCESS;
         that = that->next)
    {
        bool printParens = false;
        bool printSeparator = true;

        // print 3x as 3x instead of 3 * x
        if (_arithmetic->taffyOperator == TAFFY_MULTIPLY
            && _arithmetic->values->size == 2
            && dcNumberClass_isMe(_arithmetic->values->head->object)
            && dcNumberClass_isWholeHelper(_arithmetic->values->head->object)
            && (IS_IDENTIFIER(_arithmetic->values->tail->object)
                || (IS_RAISE(_arithmetic->values->tail->object)
                    && (IS_IDENTIFIER
                        (FLAT_ARITHMETIC_HEAD
                         (_arithmetic->values->tail->object))))))
        {
            printSeparator = false;
        }

        if (IS_FLAT_ARITHMETIC(that->object))
        {
            dcFlatArithmetic *other = CAST_FLAT_ARITHMETIC(that->object);

            // print x^2 as x^2 instead of (x^2)
            // but print x^(x^2) as x^(x^2) instead of x^x^2
            // print 3x as 3x instead of (3x)
            // but print e^(6x) as e^(6x) and not e^6x
            // print 1 / (-(1 + x)) as 1 / -(1 + x)
            if ((other->taffyOperator == TAFFY_RAISE
                 && _arithmetic->taffyOperator != TAFFY_RAISE)
                || (_arithmetic->taffyOperator == TAFFY_DIVIDE
                    && other->taffyOperator == TAFFY_MULTIPLY
                    && other->values->size == 2
                    && dcNumberClass_isMe(other->values->head->object)
                    && (dcNumberClass_equalsNumber
                        (other->values->head->object,
                         dcNumberClass_getNegativeOneNumberObject())))
                || ((_arithmetic->taffyOperator == TAFFY_ADD
                     || _arithmetic->taffyOperator == TAFFY_SUBTRACT)
                    && (other->taffyOperator == TAFFY_MULTIPLY
                        || other->taffyOperator == TAFFY_DIVIDE))
                || (_arithmetic->taffyOperator == TAFFY_ADD
                    && other->taffyOperator == TAFFY_SUBTRACT)
                || (_arithmetic->taffyOperator == TAFFY_SUBTRACT
                    && other->taffyOperator == TAFFY_ADD
                    && that == _arithmetic->values->head))
            {
                printParens = false;
            }
            else
            {
                printParens = true;
            }
        }
        // print x^(-3) as x^(-3) and not x^-3
        else if (_arithmetic->taffyOperator == TAFFY_RAISE
                 && dcNumberClass_isMe(that->object)
                 && dcNumberClass_isNegative(that->object))
        {
            printParens = true;
        }

        if (printParens)
        {
            dcString_appendCharacter(_stream, '(');
        }

        // print -x instead of -1 * x
        if (that->next != NULL
            && _arithmetic->taffyOperator == TAFFY_MULTIPLY
            && dcNumberClass_isMe(that->object)
            && dcNumberClass_equalsInt32s(that->object, -1))
        {
            dcString_append(_stream, "-");
            printSeparator = false;
        }
        else
        {
            result = dcNode_print(that->object, _stream);
        }

        if (printParens)
        {
            dcString_appendCharacter(_stream, ')');
        }

        if (printSeparator && that->next != NULL)
        {
            dcString_append(_stream,
                            "%s%s%s",
                            separator,
                            operatorSymbol,
                            separator);
        }
    }

    if (_arithmetic->taffyOperator == TAFFY_FACTORIAL)
    {
        dcString_append(_stream, "%s", operatorSymbol);
    }

    return result;
}

// debug hook
dcFlatArithmetic *dcFlatArithmetic_castMe(dcNode *_node)
{
    return CAST_FLAT_ARITHMETIC(_node);
}

TAFFY_HIDDEN bool identifierEquals(dcNode *_candidate, const char *_string)
{
    return (dcGraphData_isType(_candidate, NODE_IDENTIFIER)
            && dcIdentifier_equalsString(_candidate, _string));
}

typedef void (CombineFunction)(dcFlatArithmetic *_leftArithmetic,
                               dcListElement **_left,
                               dcFlatArithmetic *_rightArithmetic,
                               dcListElement **_right,
                               bool *_modified);

typedef struct
{
    const char *left;
    dcList *leftTokens;
    const char *right;
    dcList *rightTokens;
    CombineFunction *combineFunction;
} ComparisonLanguage;

typedef struct
{
    const char *left;
    dcList *leftTokens;
    const char *right;
    dcList *rightTokens;
    const char *result;
    dcList *resultTokens;
} TextResultLanguage;

TAFFY_HIDDEN bool checkFlatArithmetic(dcNode *_node, dcTaffyOperator _operator)
{
    return (IS_FLAT_ARITHMETIC(_node)
            && CAST_FLAT_ARITHMETIC(_node)->taffyOperator == _operator);
}

typedef bool (MatchFunction)(const dcListElement **_i,
                             dcNode *_node,
                             dcNode **_match,
                             const char *_symbol);
typedef struct
{
    const char *token;
    MatchFunction *matchFunction;
} MatchFunctionMap;

TAFFY_HIDDEN bool matchNumber(const dcListElement **_i,
                              dcNode *_node,
                              dcNode **_match,
                              const char *_symbol)
{
    return (dcNumberClass_isMe(_node) || dcComplexNumberClass_isMe(_node));
}

TAFFY_HIDDEN bool matchZeroNumber(const dcListElement **_i,
                                  dcNode *_node,
                                  dcNode **_match,
                                  const char *_symbol)
{
    return dcNumberClass_equalsInt32u(_node, 0);
}

TAFFY_HIDDEN bool matchOneNumber(const dcListElement **_i,
                                 dcNode *_node,
                                 dcNode **_match,
                                 const char *_symbol)
{
    return dcNumberClass_equalsInt32u(_node, 1);
}

TAFFY_HIDDEN bool matchNotOneNumber(const dcListElement **_i,
                                    dcNode *_node,
                                    dcNode **_match,
                                    const char *_symbol)
{
    return (dcNumberClass_isMe(_node)
            && ! dcNumberClass_isOne(_node)
            && ! dcNumberClass_isNegativeOne(_node));
}

TAFFY_HIDDEN bool matchNegativeOneNumber(const dcListElement **_i,
                                         dcNode *_node,
                                         dcNode **_match,
                                         const char *_symbol)
{
    return dcNumberClass_equalsInt32s(_node, -1);
}

TAFFY_HIDDEN bool matchNegativeNumber(const dcListElement **_i,
                                      dcNode *_node,
                                      dcNode **_match,
                                      const char *_symbol)
{
    return (dcNumberClass_isMe(_node) && dcNumberClass_isNegative(_node));
}

TAFFY_HIDDEN bool matchTwoNumber(const dcListElement **_i,
                                 dcNode *_node,
                                 dcNode **_match,
                                 const char *_symbol)
{
    return dcNumberClass_equalsInt32u(_node, 2);
}

TAFFY_HIDDEN bool matchIdentifier(const dcListElement **_i,
                                  dcNode *_node,
                                  dcNode **_match,
                                  const char *_symbol)
{
    return IS_IDENTIFIER(_node);
}

TAFFY_HIDDEN bool matchNode(const dcListElement **_i,
                            dcNode *_node,
                            dcNode **_match,
                            const char *_symbol)
{
    bool result = false;

    if (*_match == NULL)
    {
        *_match = _node;
        result = true;
    }
    else
    {
        dcTaffyOperator compareResult;

        if ((dcNode_compareEqual(_node, *_match, &compareResult)
             == TAFFY_SUCCESS)
            && compareResult == TAFFY_EQUALS)
        {
            result = true;
        }
    }

    return result;
}

TAFFY_HIDDEN bool matchHash(const dcListElement **_i,
                            dcNode *_node,
                            dcNode **_match,
                            const char *_symbol)
{
    // we're done
    *_match = _node;
    return true;
}

TAFFY_HIDDEN bool matchSymbol(const dcListElement **_i,
                              dcNode *_node,
                              dcNode **_match,
                              const char *_symbol)
{
    return identifierEquals(_node, _symbol);
}

TAFFY_HIDDEN bool matchEndNumber(const dcListElement **_i,
                                 dcNode *_node,
                                 dcNode **_match,
                                 const char *_symbol)
{
    return dcNumberClass_isMe
        (dcList_getTail(CAST_FLAT_ARITHMETIC(_node)->values));
}

TAFFY_HIDDEN bool matchFlatArithmetic(const dcListElement **_i,
                                      dcNode *_node,
                                      dcNode **_match, const char *_symbol);

TAFFY_HIDDEN bool matchAddArithmetic(const dcListElement **_i,
                                     dcNode *_node,
                                     dcNode **_match,
                                     const char *_symbol)
{
    return (checkFlatArithmetic(_node, TAFFY_ADD)
            && matchFlatArithmetic(_i, _node, _match, _symbol));
}

TAFFY_HIDDEN bool matchSubtractArithmetic(const dcListElement **_i,
                                          dcNode *_node,
                                          dcNode **_match,
                                          const char *_symbol)
{
    return (checkFlatArithmetic(_node, TAFFY_SUBTRACT)
            && matchFlatArithmetic(_i, _node, _match, _symbol));
}

TAFFY_HIDDEN bool matchMultiplyArithmetic(const dcListElement **_i,
                                          dcNode *_node,
                                          dcNode **_match,
                                          const char *_symbol)
{
    return (checkFlatArithmetic(_node, TAFFY_MULTIPLY)
            && matchFlatArithmetic(_i, _node, _match, _symbol));
}

TAFFY_HIDDEN bool matchDivideArithmetic(const dcListElement **_i,
                                        dcNode *_node,
                                        dcNode **_match,
                                        const char *_symbol)
{
    return (checkFlatArithmetic(_node, TAFFY_DIVIDE)
            && matchFlatArithmetic(_i, _node, _match, _symbol));
}

TAFFY_HIDDEN bool matchRaiseArithmetic(const dcListElement **_i,
                                       dcNode *_node,
                                       dcNode **_match,
                                       const char *_symbol)
{
    return (checkFlatArithmetic(_node, TAFFY_RAISE)
            && matchFlatArithmetic(_i, _node, _match, _symbol));
}

TAFFY_HIDDEN bool matchAnything(const dcListElement **_i,
                                dcNode *_node,
                                dcNode **_match,
                                const char *_symbol)
{
    return true;
}

TAFFY_HIDDEN const MatchFunctionMap sMatchMap[] =
{
    {"N",      &matchNumber},
    {"%0",     &matchZeroNumber},
    {"%1",     &matchOneNumber},
    {"%-1",    &matchNegativeOneNumber},
    {"%NN",    &matchNegativeNumber},
    {"%N1",    &matchNotOneNumber},
    {"%2",     &matchTwoNumber},
    {"?",      &matchAnything},
    {"I",      &matchIdentifier},
    {"$1",     &matchNode},
    {"+",      &matchAddArithmetic},
    {"-",      &matchSubtractArithmetic},
    {"*",      &matchMultiplyArithmetic},
    {"/",      &matchDivideArithmetic},
    {"^",      &matchRaiseArithmetic},
    {"#",      &matchHash},
    {"S",      &matchSymbol},
    {"^N",     &matchEndNumber},
    {NULL}
};

TAFFY_HIDDEN MatchFunction *getMatchFunction(const char *_token)
{
    size_t i;
    MatchFunction *result = NULL;

    for (i = 0; i < dcTaffy_countOf(sMatchMap) - 1; i++)
    {
        if (strcmp(sMatchMap[i].token, _token) == 0)
        {
            result = sMatchMap[i].matchFunction;
            break;
        }
    }

    assert(result != NULL);
    return result;
}

static const char *shiftElementToken(const dcListElement **_element)
{
    if (*_element == NULL)
    {
        return NULL;
    }

    const char *result = dcString_getString((*_element)->object);
    *_element = (*_element)->next;
    return result;
}

TAFFY_HIDDEN bool matchFlatArithmetic(const dcListElement **_i,
                                      dcNode *_node,
                                      dcNode **_match,
                                      const char *_symbol)
{
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    bool greaterThan = false;
    bool result = true;
    const char *token = shiftElementToken(_i);

    if (strcmp(token, ">=") == 0)
    {
        greaterThan = true;
    }

    // get the size
    token = shiftElementToken(_i);
    size_t size = atoi(token);

    if ((greaterThan && arithmetic->values->size >= size)
        || (! greaterThan && arithmetic->values->size == size))
    {
        dcListElement *that;
        token = shiftElementToken(_i);

        for (that = arithmetic->values->head;
             token != NULL && that != NULL && result;
             that = that->next)
        {
            MatchFunction *matchFunction = getMatchFunction(token);

            if (matchFunction != NULL)
            {
                result = matchFunction(_i, that->object, _match, _symbol);
            }

            token = shiftElementToken(_i);
        }
    }
    else
    {
        result = false;
    }

    return result;
}

TAFFY_HIDDEN bool comparisonFunction(const dcList *_tokens,
                                     dcNode *_node,
                                     dcNode **_match,
                                     const char *_symbol)
{
    const dcListElement *i = _tokens->head;
    const char *token = shiftElementToken(&i);

    // help the debugger
    MatchFunction *matchFunction = getMatchFunction(token);

    return (matchFunction == NULL
            ? false
            : matchFunction(&i, _node, _match, _symbol));
}

#ifdef ENABLE_DEBUG

void dcFlatArithmetic_printCounts(void)
{
    dcHash *arithmeticCounts =
        dcFlatArithmetic_getArithmeticCounts();
    dcHashIterator *it = dcHashIterator_create(arithmeticCounts);
    dcNode *elementNode = NULL;

    assert(arithmeticCounts->size > 0);

    while ((elementNode = dcHashIterator_getNext(it))
           != NULL)
    {
        dcHashElement *element = CAST_HASH_ELEMENT(elementNode);
        assert(element->key.isNodeKey);

        const char *arithmeticName =
            dcFlatArithmetic_getArithmeticLanguageName
            (element->key.keyUnion.nodeKey);

        fprintf(stderr,
                "%s: %s\n",
                arithmeticName,
                dcNode_display(element->value));
    }

    dcHashIterator_free(&it);
}

void combineHelper(void)
{
}

void calculusHelper(void)
{
}

TAFFY_HIDDEN void updateCounts(dcNode *_languageVoid,
                               uint32_t _index)
{
    // update counts -- we need to verify that
    // all languages were tested
    dcNode *languageNode = NULL;
    dcResult languageResult = dcHash_getValue(sArithmeticCounts,
                                              _languageVoid,
                                              &languageNode);
    assert(languageResult != TAFFY_EXCEPTION);

    if (languageNode == NULL)
    {
        languageNode = dcList_createNode();
        dcHash_setValue(sArithmeticCounts,
                        _languageVoid,
                        languageNode);
    }
    else
    {
        dcNode_free(&_languageVoid, DC_DEEP);
    }

    dcNode *number = createNumber(_index);

    if (! dcList_containsEqual(CAST_LIST(languageNode), number))
    {
        // insert it in order
        dcListElement *that = NULL;

        if (CAST_LIST(languageNode)->size == 0)
        {
            dcList_push(CAST_LIST(languageNode), number);
        }
        else
        {
            bool inserted = false;

            for (that = CAST_LIST(languageNode)->head;
                 that != NULL;
                 that = that->next)
            {
                dcTaffyOperator compared;

                if ((dcNode_compareEqual(number, that->object, &compared)
                     == TAFFY_SUCCESS)
                    && compared == TAFFY_LESS_THAN)
                {
                    inserted = true;
                    dcList_insertBeforeListElement
                        (CAST_LIST(languageNode),
                         that,
                         number);
                    break;
                }
            }

            if (! inserted)
            {
                dcList_push(CAST_LIST(languageNode), number);
            }
        }
    }
    else
    {
        dcNode_free(&number, DC_DEEP);
    }
}
#endif // ENABLE_DEBUG

TAFFY_HIDDEN bool combineElements(const ComparisonLanguage *_languages,
                                  dcFlatArithmetic *_leftArithmetic,
                                  dcListElement **_left,
                                  dcFlatArithmetic *_rightArithmetic,
                                  dcListElement **_right,
                                  bool *_modified)
{
    uint32_t i;
    bool result = false;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    for (i = 0;
         (_languages[i].left != NULL
          && ! dcNodeEvaluator_abortReceived(evaluator));
         i++)
    {
        dcNode *match = NULL;

        if (comparisonFunction
            (_languages[i].leftTokens, (*_left)->object, &match, NULL)
            && (comparisonFunction
                (_languages[i].rightTokens, (*_right)->object, &match, NULL)))
        {
            if (dcLog_isEnabled(FLAT_ARITHMETIC_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_LOG,
                          "\nmatched left: %s for: %s\n"
                          "left arithmetic: %s\n"
                          "right: %s for: %s\n"
                          "right arithmetic: %s\n",
                          _languages[i].left,
                          dcNode_display((*_left)->object),
                          dcFlatArithmetic_display(_leftArithmetic),
                          _languages[i].right,
                          dcNode_display((*_right)->object),
                          dcFlatArithmetic_display(_rightArithmetic));
            }

            // set a breakpoint in combineHelper for help
            TAFFY_DEBUG(combineHelper();
                        updateCounts(dcVoidContainer_createNode
                                     ((void *)_languages), i));

            _languages[i].combineFunction(_leftArithmetic,
                                          _left,
                                          _rightArithmetic,
                                          _right,
                                          _modified);
            result = true;
            break;
        }
    }

    return result;
}

// forward declaration
TAFFY_HIDDEN dcNode *combineTextResultElements
    (const TextResultLanguage *_languages,
     dcFlatArithmetic *_leftArithmetic,
     dcListElement **_left,
     dcFlatArithmetic *_rightArithmetic,
     dcListElement **_right,
     bool *_modified);

TAFFY_HIDDEN dcNode *createRealNumber(double _value)
{
    return dcNode_setTemplate(dcNumberClass_createObjectFromDouble(_value),
                              true);
}

TAFFY_HIDDEN dcList *getValues(dcListElement **_element)
{
    return CAST_FLAT_ARITHMETIC((*_element)->object)->values;
}

TAFFY_HIDDEN void removeElement(dcFlatArithmetic *_arithmetic,
                                dcListElement **_element)
{
    dcList_removeElement(_arithmetic->values, _element, DC_DEEP);
}

//
// like:
// left: x
// right: 2x
//
TAFFY_HIDDEN void addMultiply(dcFlatArithmetic *_leftArithmetic,
                              dcListElement **_left,
                              dcFlatArithmetic *_rightArithmetic,
                              dcListElement **_right,
                              bool *_modified)
{
    setModified(_modified);
    dcNumberClass_increment(dcList_getTail(getValues(_right)));
    removeElement(_leftArithmetic, _left);
}

TAFFY_HIDDEN void addMultiplyB(dcFlatArithmetic *_leftArithmetic,
                               dcListElement **_left,
                               dcFlatArithmetic *_rightArithmetic,
                               dcListElement **_right,
                               bool *_modified)
{
    addMultiply(_rightArithmetic, _right, _leftArithmetic, _left, _modified);
}

TAFFY_HIDDEN void addMultiplyReverse(dcFlatArithmetic *_leftArithmetic,
                                     dcListElement **_left,
                                     dcFlatArithmetic *_rightArithmetic,
                                     dcListElement **_right,
                                     bool *_modified)
{
    setModified(_modified);
    dcNumberClass_increment(dcList_getHead(getValues(_right)));
    removeElement(_leftArithmetic, _left);
}

TAFFY_HIDDEN void addMultiplyBReverse(dcFlatArithmetic *_leftArithmetic,
                                      dcListElement **_left,
                                      dcFlatArithmetic *_rightArithmetic,
                                      dcListElement **_right,
                                      bool *_modified)
{
    addMultiplyReverse(_rightArithmetic,
                       _right,
                       _leftArithmetic,
                       _left,
                       _modified);
}

typedef dcNode *(InlineOperation)(dcNode *_left, dcNode *_right);
typedef dcNode *(OrderedInlineOperation)(dcNode *_left,
                                         dcNode *_right,
                                         bool _onLeft);

// check if the complex number can be converted to a real/Number
TAFFY_HIDDEN void checkComplexNumberConversion(dcFlatArithmetic *_arithmetic,
                                               dcListElement **_element)
{
    dcNode *object = (*_element)->object;
    TAFFY_DEBUG(assert(dcComplexNumberClass_isMe(object)));
    dcNode *realMaybe = dcComplexNumberClass_maybeConvertToReal(object);

    if (realMaybe != object)
    {
        (*_element)->object = realMaybe;
        dcNode_free(&object, DC_DEEP);
    }
}

TAFFY_HIDDEN void inlineOperation
    (dcFlatArithmetic *_leftArithmetic,
     dcListElement **_left,
     dcNode *_leftObject,
     dcFlatArithmetic *_rightArithmetic,
     dcListElement **_right,
     dcNode *_rightObject,
     dcTaffyOperator _operation,
     InlineOperation *_numberOperation,
     InlineOperation *_complexNumberOperation,
     OrderedInlineOperation *_complexNumberRealOperation,
     bool *_modified)
{
    setModified(_modified);

    if (dcNumberClass_isMe(_leftObject)
        && dcNumberClass_isMe(_rightObject))
    {
        _numberOperation(_leftObject, _rightObject);
        removeElement(_rightArithmetic, _right);
    }
    else if (dcComplexNumberClass_isMe(_leftObject)
             && dcComplexNumberClass_isMe(_rightObject))
    {
        _complexNumberOperation(_leftObject, _rightObject);
        removeElement(_rightArithmetic, _right);
        checkComplexNumberConversion(_leftArithmetic, _left);
    }
    else if (dcComplexNumberClass_isMe(_leftObject)
             && dcNumberClass_isMe(_rightObject))
    {
        _complexNumberRealOperation(_leftObject, _rightObject, true);
        removeElement(_rightArithmetic, _right);
        checkComplexNumberConversion(_leftArithmetic, _left);
    }
    else if (dcNumberClass_isMe(_leftObject)
             && dcComplexNumberClass_isMe(_rightObject))
    {
        // the result is stored into the complex number!
        _complexNumberRealOperation(_leftObject, _rightObject, false);
        removeElement(_leftArithmetic, _left);
        checkComplexNumberConversion(_rightArithmetic, _right);
    }
    else
    {
        assert(false);
    }
}

TAFFY_HIDDEN void inlineAdd(dcFlatArithmetic *_leftArithmetic,
                            dcListElement **_left,
                            dcNode *_leftObject,
                            dcFlatArithmetic *_rightArithmetic,
                            dcListElement **_right,
                            dcNode *_rightObject,
                            bool *_modified)
{
    inlineOperation(_leftArithmetic,
                    _left,
                    _leftObject,
                    _rightArithmetic,
                    _right,
                    _rightObject,
                    TAFFY_ADD,
                    &dcNumberClass_inlineAdd,
                    &dcComplexNumberClass_inlineAdd,
                    &dcComplexNumberClass_inlineAddReal,
                    _modified);
}

TAFFY_HIDDEN void inlineMultiply(dcFlatArithmetic *_leftArithmetic,
                                 dcListElement **_left,
                                 dcNode *_leftObject,
                                 dcFlatArithmetic *_rightArithmetic,
                                 dcListElement **_right,
                                 dcNode *_rightObject,
                                 bool *_modified)
{
    inlineOperation(_leftArithmetic,
                    _left,
                    _leftObject,
                    _rightArithmetic,
                    _right,
                    _rightObject,
                    TAFFY_MULTIPLY,
                    &dcNumberClass_inlineMultiply,
                    &dcComplexNumberClass_inlineMultiply,
                    &dcComplexNumberClass_inlineMultiplyReal,
                    _modified);
}

TAFFY_HIDDEN void addNumbers(dcFlatArithmetic *_leftArithmetic,
                             dcListElement **_left,
                             dcFlatArithmetic *_rightArithmetic,
                             dcListElement **_right,
                             bool *_modified)
{
    inlineAdd(_leftArithmetic,
              _left,
              (*_left)->object,
              _rightArithmetic,
              _right,
              (*_right)->object,
              _modified);
}

TAFFY_HIDDEN void multiplyNumbers(dcFlatArithmetic *_leftArithmetic,
                                  dcListElement **_left,
                                  dcFlatArithmetic *_rightArithmetic,
                                  dcListElement **_right,
                                  bool *_modified)
{
    inlineMultiply(_leftArithmetic,
                   _left,
                   (*_left)->object,
                   _rightArithmetic,
                   _right,
                   (*_right)->object,
                   _modified);
}

TAFFY_HIDDEN void addSame(dcFlatArithmetic *_leftArithmetic,
                          dcListElement **_left,
                          dcFlatArithmetic *_rightArithmetic,
                          dcListElement **_right,
                          bool *_modified)
{
    dcNode *leftObject = (*_left)->object;

    if (IS_MULTIPLY(leftObject))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(leftObject);
        dcList_unshift(arithmetic->values, createNumber(2));
    }
    else
    {
        // two non-flat-arithmetic terms added together
        dcNode *save = leftObject;
        (*_left)->object = CREATE_MULTIPLY(createNumber(2), save, NULL);
    }

    setModified(_modified);
    removeElement(_rightArithmetic, _right);
}

TAFFY_HIDDEN void addMultiplies(dcFlatArithmetic *_leftArithmetic,
                                dcListElement **_left,
                                dcFlatArithmetic *_rightArithmetic,
                                dcListElement **_right,
                                bool *_modified)
{
    dcFlatArithmetic *left = CAST_FLAT_ARITHMETIC((*_left)->object);
    dcFlatArithmetic *right = CAST_FLAT_ARITHMETIC((*_right)->object);
    dcListElement *that;

    dcFlatArithmetic *leftCopy = dcFlatArithmetic_copy(left, DC_DEEP);
    dcFlatArithmetic *rightCopy = dcFlatArithmetic_copy(right, DC_DEEP);

    // first see if non-numbers match
    for (that = leftCopy->values->head; that != NULL; )
    {
        dcListElement *next = that->next;

        if (! dcNumberClass_isMe(that->object))
        {
            dcListElement *found;
            dcResult result =
                dcList_find(rightCopy->values, that->object, &found);

            if (result == TAFFY_SUCCESS)
            {
                dcList_removeElement(rightCopy->values, &found, DC_DEEP);
                dcList_removeElement(leftCopy->values, &that, DC_DEEP);
            }
        }

        that = next;
    }

    dcListElement *leftNumberLocation = NULL;
    bool leftPass = (leftCopy->values->size == 0
                     || (leftCopy->values->size == 1
                         && (dcNumberClass_isMe
                             (leftCopy->values->head->object))));
    dcNode *rightNumber = NULL;

    if (leftPass && leftCopy->values->size == 1)
    {
        dcNode *leftNumber = dcList_pop(leftCopy->values, DC_SHALLOW);

        if (dcList_find(left->values, leftNumber, &leftNumberLocation)
            == TAFFY_EXCEPTION)
        {
            dcNode_free(&leftNumber, DC_DEEP);
            return;
        }

        dcNode_free(&leftNumber, DC_DEEP);
    }

    bool rightPass = (rightCopy->values->size == 0
                      || (rightCopy->values->size == 1
                          && (dcNumberClass_isMe
                              (rightCopy->values->head->object))));

    if (rightPass && rightCopy->values->size == 1)
    {
        rightNumber = dcList_pop(rightCopy->values, DC_SHALLOW);
    }

    if (leftPass && rightPass)
    {
        setModified(_modified);

        if (leftNumberLocation != NULL && rightNumber == NULL)
        {
            dcNumberClass_increment(leftNumberLocation->object);
        }
        else if (leftNumberLocation != NULL && rightNumber != NULL)
        {
            leftNumberLocation->object = CREATE_ADD(leftNumberLocation->object,
                                                    rightNumber,
                                                    NULL);
            rightNumber = NULL;
        }
        else if (leftNumberLocation == NULL && rightNumber == NULL)
        {
            dcList_unshift(left->values, createNumber(2));
        }
        else if (leftNumberLocation == NULL && rightNumber != NULL)
        {
            dcNumberClass_increment(rightNumber);
            dcList_unshift(left->values, rightNumber);
            rightNumber = NULL;
        }
        else
        {
            // derr?
            assert(false);
        }

        removeElement(_rightArithmetic, _right);
    }

    dcNode_free(&rightNumber, DC_DEEP);
    dcFlatArithmetic_free(&leftCopy, DC_DEEP);
    dcFlatArithmetic_free(&rightCopy, DC_DEEP);
}

#define COMPARISON_ELEMENT(_left, _right, _function)    \
    {_left, NULL, _right, NULL, _function}

TAFFY_HIDDEN ComparisonLanguage sAddLanguages[] =
{
    COMPARISON_ELEMENT("* == 2 N $1", "$1",          &addMultiplyBReverse),
    COMPARISON_ELEMENT("$1",          "* == 2 $1 N", &addMultiply),
    COMPARISON_ELEMENT("* == 2 $1 N", "$1",          &addMultiplyB),
    COMPARISON_ELEMENT("$1",          "* == 2 N $1", &addMultiplyReverse),
    COMPARISON_ELEMENT("* >= 2",      "* >= 2",      &addMultiplies),
    COMPARISON_ELEMENT("N",           "N",           &addNumbers),
    COMPARISON_ELEMENT("$1",          "$1",          &addSame),
    {0}
};

#define TEXT_RESULT_ELEMENT(_left, _right, _result) \
    {_left, NULL, _right, NULL, _result, NULL}

TAFFY_HIDDEN TextResultLanguage sAddTextResultLanguages[] =
{
    //////////////////////////////
    //
    // sin(x)^2 + cos(x)^2 = 1
    //
    //////////////////////////////

    // sin(x)^2 + cos(x)^2 = 1
    TEXT_RESULT_ELEMENT("^ == 2 FM sin ( C ) %2",
                        "^ == 2 FM cos ( C ) %2",
                        "1"),

    // -sin(x)^2 + -cos(x)^2 = -1 // TEST
    TEXT_RESULT_ELEMENT("* == 2 %-1 ( ^ == 2 FM sin ( C ) %2 )",
                        "* == 2 %-1 ( ^ == 2 FM cos ( C ) %2 )",
                        "-1"),

    // 1 + -sin(x)^2 = cos(x)^2
    TEXT_RESULT_ELEMENT("%1",
                        "* == 2 %-1 ( ^ == 2 FM sin ( C ) %2 )",
                        "^ F cos ( C ) 2"),

    // 1 + -cos(x)^2 = sin(x)^2
    TEXT_RESULT_ELEMENT("%1",
                        "* == 2 %-1 ( ^ == 2 FM cos ( C ) %2 )",
                        "^ F sin ( C ) 2"),

    // sin(x)^2 + -1 = -cos(x)^2 // TEST
    TEXT_RESULT_ELEMENT("^ == 2 FM sin ( C ) %2",
                        "%-1",
                        "* -1 ( ^ F cos ( C ) 2 )"),

    // cos(x)^2 + -1 = -sin(x)^2 // TEST
    TEXT_RESULT_ELEMENT("^ == 2 FM cos ( C ) %2",
                        "%-1",
                        "* -1 ( ^ F sin ( C ) 2 )"),

    //////////////////////////////
    //
    // 1 + cot(x)^2 = csc(x)^2
    //
    //////////////////////////////

    // 1 + cot(x)^2 = csc(x)^2
    TEXT_RESULT_ELEMENT("%1",
                        "^ == 2 FM cot ( C ) %2",
                        "^ F csc ( C ) 2"),

    // 1 - csc(x)^2 = -cot(x)^2 // TEST
    TEXT_RESULT_ELEMENT("%1",
                        "* == 2 %-1 ( ^ == 2 FM csc ( C ) %2 )",
                        "* -1 ( ^ F cot ( C ) 2 )"),

    // cot(x)^2 - csc(x)^2 = -1 // TEST
    TEXT_RESULT_ELEMENT("^ == 2 FM cot ( C ) %2",
                        "* == 2 %-1 ( ^ == 2 FM csc ( C ) %2 )",
                        "-1"),

    // -1 - cot(x)^2 = -csc(x)^2 // TEST
    TEXT_RESULT_ELEMENT("%-1",
     "* == 2 %-1 ( ^ == 2 FM cot ( C ) %2 )",
     "* -1 ( ^ F csc ( C ) 2 )"),

    // csc(x)^2 + -1 = cot(x)^2
    TEXT_RESULT_ELEMENT("^ == 2 FM csc ( C ) %2", "%-1", "^ F cot ( C ) 2"),

    // csc(x)^2 + -cot(x)^2 = 1 // TEST
    TEXT_RESULT_ELEMENT("^ == 2 FM csc ( C ) %2",
                        "* == 2 %-1 ( ^ == 2 FM cot ( C ) %2 )",
                        "1"),

    //////////////////////////////
    //
    // tan(x)^2 + 1 = sec(x)^2
    //
    //////////////////////////////

    // tan(x)^2 + 1 = sec(x)^2
    TEXT_RESULT_ELEMENT("^ == 2 FM tan ( C ) %2", "%1", "^ F sec ( C ) 2"),

    // -tan(x)^2 - 1 = -sec(x)^2 // TEST
    TEXT_RESULT_ELEMENT("* == 2 %-1 ( ^ == 2 FM tan ( C ) %2 )",
     "%-1",
     "* -1 ( ^ F sec ( C ) 2 )"),

    // 1 - sec(x)^2 = -tan(x)^2 // TEST
    TEXT_RESULT_ELEMENT("%1",
                        "* == 2 %-1 ( ^ == 2 FM sec ( C ) %2 )",
                        "* -1 ( ^ F tan ( C ) 2 )"),

    // sec(x)^2 + -1 = tan(x)^2
    TEXT_RESULT_ELEMENT("^ == 2 FM sec ( C ) %2", "%-1", "^ F tan ( C ) 2"),

    // 1 = sec(x)^2 + -tan(x)^2 // TEST
    TEXT_RESULT_ELEMENT("^ == 2 FM sec ( C ) %2",
                        "* == 2 %-1 ( ^ == 2 FM tan ( C ) %2 )",
                        "1"),

    // tan(x)^2 + -sec(x)^2 = -1 // TEST
    TEXT_RESULT_ELEMENT("^ == 2 FM tan ( C ) %2",
                        "* == 2 %-1 ( ^ == 2 FM sec ( C ) %2 )",
                        "-1"),

    //////////////////////////////
    //
    // cosh(x)^2 - sinh(x)^2 = 1
    //
    //////////////////////////////

    // cosh^2(x) + -sinh^2(x) = 1
    TEXT_RESULT_ELEMENT("^ == 2 FM cosh ( C ) %2",
                        "* == 2 %-1 ( ^ == 2 FM sinh ( C ) %2 )",
                        "1"),

    // cosh(x)^2 - 1 = sinh(x)^2 // TEST
    TEXT_RESULT_ELEMENT("^ == 2 FM cosh ( C ) %2",
                        "%-1",
                        "* -1 ( ^ F sinh ( C ) 2 )"),

    // -cosh(x)^2 + sinh(x)^2 = -1 // TEST
    TEXT_RESULT_ELEMENT("* == 2 %-1 ( ^ == 2 FM cosh ( C ) %2 )",
     "^ == 2 FM sinh ( C ) %2",
     "-1"),

    // -sinh(x)^2 - 1 = -cosh(x)^2 // TEST
    TEXT_RESULT_ELEMENT("* == 2 %-1 ( ^ == 2 FM sinh ( C ) %2 )",
     "%-1",
     "* -1 ( ^ F cosh ( C ) 2 )"),

    // 1 - cosh(x)^2 = -sinh(x)^2 // TEST
    TEXT_RESULT_ELEMENT("%1",
     "* == 2 %-1 ( ^ == 2 FM cosh ( C ) %2 )",
     "* -1 ( ^ F sinh ( C ) 2 )"),

    // 1 + sinh(x)^2 = cosh(x)^2 // TEST
    TEXT_RESULT_ELEMENT("%1", "^ == 2 FM sinh ( C ) %2", "^ F cosh ( C ) 2"),

    //////////////////////////////
    //
    // tanh(x)^2 + sech(x)^2 = 1
    //
    //////////////////////////////

    // tanh^2(x) + sech^2(x) = 1
    TEXT_RESULT_ELEMENT("^ == 2 FM tanh ( C ) %2",
                        "^ == 2 FM sech ( C ) %2",
                        "1"),

    // -tanh(x)^2 - sech(x)^2 = -1
    TEXT_RESULT_ELEMENT("* == 2 %-1 ( ^ == 2 FM tanh ( C ) %2 )",
     "* == 2 %-1 ( ^ == 2 FM sech ( C ) %2 )",
     "-1"),

    // tanh(x)^2 - 1 = -sech(x)^2 // TEST
    TEXT_RESULT_ELEMENT("^ == 2 FM tanh ( C ) %2",
                        "%-1",
                        "* -1 ( ^ F sech ( C ) 2 )"),

    // sech(x)^2 - 1 = -tanh(x)^2 // TEST
    TEXT_RESULT_ELEMENT("^ == 2 FM sech ( C ) %2",
                        "%-1",
                        "* -1 ( ^ F tanh ( C ) 2 )"),

    // -tanh(x)^2 + 1 = sech(x)^2 // TEST
    TEXT_RESULT_ELEMENT("* == 2 %-1 ( ^ == 2 FM tanh ( C ) %2 )",
     "%1",
     "^ F sech ( C ) 2"),

    // 1 - sech(x)^2 = tanh(x)^2 // TEST
    TEXT_RESULT_ELEMENT("%1",
                        "* == 2 %-1 ( ^ == 2 FM sech ( C ) %2 )",
                        "^ F tanh ( C ) 2"),

    //////////////////////////////
    //
    // coth(x)^2 - csch(x)^2 = 1
    //
    //////////////////////////////

    // coth^2(x) + -csch^2(x) = 1
    TEXT_RESULT_ELEMENT("^ == 2 FM coth ( C ) %2",
                        "* == 2 %-1 ( ^ == 2 FM csch ( C ) %2 )",
                        "1"),

    // coth(x)^2 - 1 = csch(x)^2 // TEST
    TEXT_RESULT_ELEMENT("^ == 2 FM coth ( C ) %2", "%-1", "^ F csch ( C ) 2"),

    // -coth(x)^2 + csch(x)^2 = -1 // TEST
    TEXT_RESULT_ELEMENT("* == 2 %-1 ( ^ == 2 FM coth ( C ) %2 )",
                        "^ == 2 FM csch ( C ) %2",
                        "-1"),

    // -csch(x)^2 - 1 = -coth(x)^2 // TEST
    TEXT_RESULT_ELEMENT("* == 2 %-1 ( ^ == 2 FM csch ( C ) %2 )",
     "%-1",
     "* -1 ( ^ F coth ( C ) 2 )"),

    // 1 - coth(x)^2 = -csch(x)^2 // TEST
    TEXT_RESULT_ELEMENT("%1",
     "* == 2 %-1 ( ^ == 2 FM coth ( C ) %2 )",
     "* -1 ( ^ F csch ( C ) 2 )"),

    // 1 + csch(x)^2 = coth(x)^2 // TEST
    TEXT_RESULT_ELEMENT("%1",
     "^ == 2 FM csch ( C ) %2",
     "^ F coth ( C ) 2 )"),

    {0}
};

// x * x^y
TAFFY_HIDDEN void multiplyRaise(dcFlatArithmetic *_leftArithmetic,
                                dcListElement **_left,
                                dcFlatArithmetic *_rightArithmetic,
                                dcListElement **_right,
                                bool *_modified)
{
    setModified(_modified);
    dcFlatArithmetic *right = CAST_FLAT_ARITHMETIC((*_right)->object);
    dcNode *head = dcList_shift(right->values, DC_SHALLOW);
    dcNode *exponent = dcFlatArithmetic_shrink(CREATE_ADD((*_right)->object,
                                                          createNumber(1),
                                                          NULL),
                                               NULL);
    (*_right)->object = CREATE_RAISE(head, exponent, NULL);
    removeElement(_leftArithmetic, _left);
}

TAFFY_HIDDEN void multiplyRaiseB(dcFlatArithmetic *_leftArithmetic,
                                 dcListElement **_left,
                                 dcFlatArithmetic *_rightArithmetic,
                                 dcListElement **_right,
                                 bool *_modified)
{
    multiplyRaise(_rightArithmetic,
                  _right,
                  _leftArithmetic,
                  _left,
                  _modified);
}

// combine: x^a * x^b
TAFFY_HIDDEN void multiplyTwoRaises(dcFlatArithmetic *_leftArithmetic,
                                    dcListElement **_left,
                                    dcFlatArithmetic *_rightArithmetic,
                                    dcListElement **_right,
                                    bool *_modified)
{
    setModified(_modified);
    dcFlatArithmetic *right = CAST_FLAT_ARITHMETIC((*_right)->object);
    dcFlatArithmetic *left = CAST_FLAT_ARITHMETIC((*_left)->object);
    dcList_shift(left->values, DC_DEEP);
    dcNode *head = dcList_shift(right->values, DC_SHALLOW);
    dcNode *exponent = (dcFlatArithmetic_shrink
                        (CREATE_ADD((*_right)->object,
                                    dcNode_copy((*_left)->object, DC_DEEP),
                                    NULL),
                         NULL));
    (*_right)->object = CREATE_RAISE(head, exponent, NULL);
    removeElement(_leftArithmetic, _left);
}

TAFFY_HIDDEN dcNode *createSimpleRaise(dcNode *_base, uint32_t _exponent)
{
    dcNode *result = dcFlatArithmetic_createNode(TAFFY_RAISE);
    dcList_push(CAST_FLAT_ARITHMETIC(result)->values, _base);
    dcList_push(CAST_FLAT_ARITHMETIC(result)->values,
                createNumber(_exponent));
    return result;
}

TAFFY_HIDDEN void multiplySame(dcFlatArithmetic *_leftArithmetic,
                               dcListElement **_left,
                               dcFlatArithmetic *_rightArithmetic,
                               dcListElement **_right,
                               bool *_modified)
{
    setModified(_modified);
    dcNode *save = (*_left)->object;

    if (IS_FLAT_ARITHMETIC(save))
    {
        CAST_FLAT_ARITHMETIC(save)->grouped = true;
    }

    (*_left)->object = createSimpleRaise(save, 2);
    removeElement(_rightArithmetic, _right);
}

static void multiplyDivides(dcFlatArithmetic *_left,
                            const dcFlatArithmetic *_right)
{
    dcListElement *that;
    dcListElement *uh;

    for (that = _left->values->head, uh = _right->values->head;
         that != NULL && uh != NULL;
         that = that->next, uh = uh->next)
    {
        that->object = CREATE_MULTIPLY(that->object,
                                       dcNode_copy(uh->object, DC_DEEP),
                                       NULL);
    }
}

// something like: x * (1/x^x)
TAFFY_HIDDEN void multiplyWhatAndDivide
    (dcFlatArithmetic *_leftArithmetic,
     dcListElement **_left,
     dcFlatArithmetic *_rightArithmetic,
     dcListElement **_right,
     bool *_modified)
{
    dcFlatArithmetic *divide = CAST_FLAT_ARITHMETIC((*_right)->object);

    if (IS_DIVIDE((*_left)->object))
    {
        multiplyDivides(divide, CAST_FLAT_ARITHMETIC((*_left)->object));
    }
    else
    {
        divide->values->head->object =
            CREATE_MULTIPLY(divide->values->head->object,
                            dcNode_copy((*_left)->object, DC_DEEP),
                            NULL);
    }

    removeElement(_leftArithmetic, _left);
}

// something like: (1/x^x) * x
TAFFY_HIDDEN void multiplyDivideAndWhat
    (dcFlatArithmetic *_leftArithmetic,
     dcListElement **_left,
     dcFlatArithmetic *_rightArithmetic,
     dcListElement **_right,
     bool *_modified)
{
    dcFlatArithmetic *divide = CAST_FLAT_ARITHMETIC((*_left)->object);

    if (IS_DIVIDE((*_right)->object))
    {
        multiplyDivides(divide, CAST_FLAT_ARITHMETIC((*_right)->object));
    }
    else
    {
        // test me
        divide->values->head->object =
            CREATE_MULTIPLY(divide->values->head->object,
                            dcNode_copy((*_right)->object, DC_DEEP),
                            NULL);
    }

    removeElement(_rightArithmetic, _right);
}

TAFFY_HIDDEN ComparisonLanguage sMultiplyLanguages[] =
{
    COMPARISON_ELEMENT("$1",          "^ >= 2 $1",   &multiplyRaise),
    COMPARISON_ELEMENT("^ >= 2 $1",   "$1",          &multiplyRaiseB),
    COMPARISON_ELEMENT("^ == 2 $1",   "^ == 2 $1",   &multiplyTwoRaises),
    COMPARISON_ELEMENT("N",           "N",           &multiplyNumbers),
    COMPARISON_ELEMENT("$1",          "$1",          &multiplySame),
    COMPARISON_ELEMENT("/ >= 2",      "?",           &multiplyDivideAndWhat),
    COMPARISON_ELEMENT("?",           "/ >= 2",      &multiplyWhatAndDivide),
    {0}
};

TAFFY_HIDDEN TextResultLanguage sMultiplyTextResultLanguages[] =
{
    // to 1
    TEXT_RESULT_ELEMENT("F sin ( C )", "F csc ( C )", "1"),
    TEXT_RESULT_ELEMENT("F cos ( C )", "F sec ( C )", "1"),
    TEXT_RESULT_ELEMENT("F cot ( C )", "F tan ( C )", "1"),

    // product identities
    TEXT_RESULT_ELEMENT("F tan ( C )", "F cos ( C )", "F sin ( C )"),
    TEXT_RESULT_ELEMENT("F sin ( C )", "F cot ( C )", "F cos ( C )"),
    TEXT_RESULT_ELEMENT("F cos ( C )", "F csc ( C )", "F cot ( C )"),
    TEXT_RESULT_ELEMENT("F cot ( C )", "F sec ( C )", "F csc ( C )"),
    TEXT_RESULT_ELEMENT("F csc ( C )", "F tan ( C )", "F sec ( C )"),
    TEXT_RESULT_ELEMENT("F sec ( C )", "F sin ( C )", "F tan ( C )"),

    TEXT_RESULT_ELEMENT("F sinh ( C )", "F sech ( C )", "F tanh ( C )"),

    // clockwise identities
    TEXT_RESULT_ELEMENT("F sin ( C )", "/ == 2 %1 F cos ( C )", "F tan ( C )"),
    TEXT_RESULT_ELEMENT("F cos ( C )", "/ == 2 %1 F cot ( C )", "F sin ( C )"),
    TEXT_RESULT_ELEMENT("F cot ( C )", "/ == 2 %1 F csc ( C )", "F cos ( C )"),
    TEXT_RESULT_ELEMENT("F csc ( C )", "/ == 2 %1 F sec ( C )", "F cot ( C )"),
    TEXT_RESULT_ELEMENT("F sec ( C )", "/ == 2 %1 F tan ( C )", "F csc ( C )"),
    TEXT_RESULT_ELEMENT("F tan ( C )", "/ == 2 %1 F sin ( C )", "F sec ( C )"),

    // counterclockwise identities
    TEXT_RESULT_ELEMENT("F sin ( C )", "/ == 2 %1 F tan ( C )", "F cos ( C )"),
    TEXT_RESULT_ELEMENT("F tan ( C )", "/ == 2 %1 F sec ( C )", "F sin ( C )"),
    TEXT_RESULT_ELEMENT("F sec ( C )", "/ == 2 %1 F csc ( C )", "F tan ( C )"),
    TEXT_RESULT_ELEMENT("F csc ( C )", "/ == 2 %1 F cot ( C )", "F sec ( C )"),
    TEXT_RESULT_ELEMENT("F cot ( C )", "/ == 2 %1 F cos ( C )", "F csc ( C )"),
    TEXT_RESULT_ELEMENT("F cos ( C )", "/ == 2 %1 F sin ( C )", "F cot ( C )"),

    {0}
};

TAFFY_HIDDEN void combineRaiseAndAny(dcFlatArithmetic *_leftArithmetic,
                                     dcListElement **_left,
                                     dcFlatArithmetic *_rightArithmetic,
                                     dcListElement **_right,
                                     bool *_modified)
{
    dcFlatArithmetic *left = CAST_FLAT_ARITHMETIC((*_left)->object);

    if (dcNumberClass_isMe(left->values->tail->object)
        && dcNumberClass_isMe((*_right)->object)
        && (dcNumberClass_isEven(left->values->tail->object)
            || ! dcNumberClass_isOneFractional((*_right)->object)))
    {
        setModified(_modified);
        dcNumberClass_inlineMultiply(left->values->tail->object,
                                     (*_right)->object);
        removeElement(_rightArithmetic, _right);
    }
    else
    {
        dcNode *tail = FLAT_ARITHMETIC_TAIL((*_left)->object);

        if (FLAT_ARITHMETIC_SIZE((*_left)->object) == 2
            && (! dcNumberClass_isMe(tail)
                || (dcNumberClass_isMe(tail)
                    && dcNumberClass_isEven(tail))))
        {
            left->values->tail->object =
                (CREATE_MULTIPLY
                 (left->values->tail->object,
                  dcNode_copy((*_right)->object, DC_DEEP),
                  NULL));
            removeElement(_rightArithmetic, _right);
        }
    }
}

TAFFY_HIDDEN void raiseNumbers(dcFlatArithmetic *_leftArithmetic,
                               dcListElement **_left,
                               dcFlatArithmetic *_rightArithmetic,
                               dcListElement **_right,
                               bool *_modified)
{
    dcNode *lefty = (*_left)->object;
    dcNode *righty = (*_right)->object;

    if (dcNumberClass_isMe(lefty) && dcNumberClass_isMe(righty))
    {
        setModified(_modified);

        if (dcNumberClass_isNegative(lefty)
            && (dcNumberClass_isOneFractional(righty)
                || ! dcNumberClass_isWholeHelper(righty)))
        {
            // negative raised to a power than is < 1, and > -1,
            // so the result is a complex number
            dcNode *result = dcComplexNumberClass_inlineRaise(lefty, righty);

            if (result != lefty)
            {
                dcNode_free(&lefty, DC_DEEP);
                (*_left)->object =
                    dcNode_setTemplate(dcNode_copy(result, DC_DEEP), true);;
            }
        }
        else
        {
            dcNumberClass_inlineRaise(lefty, righty);
        }

        removeElement(_rightArithmetic, _right);
    }
    else if ((dcComplexNumberClass_isMe(lefty)
              && dcNumberClass_isMe(righty))
             || (dcComplexNumberClass_isMe(righty)
                 && dcNumberClass_isMe(lefty)))
    {
        setModified(_modified);
        dcNode *result = (dcComplexNumberClass_inlineRaise
                          ((*_left)->object,
                           (*_right)->object));

        if (result != (*_left)->object)
        {
            dcNode_free(&(*_left)->object, DC_DEEP);
            (*_left)->object =
                dcNode_setTemplate(dcNode_copy(result, DC_DEEP), true);
        }

        removeElement(_rightArithmetic, _right);
    }
}

TAFFY_HIDDEN void raiseNumberAndDivide(dcFlatArithmetic *_leftArithmetic,
                                       dcListElement **_left,
                                       dcFlatArithmetic *_rightArithmetic,
                                       dcListElement **_right,
                                       bool *_modified)
{
    dcNode *left = FLAT_ARITHMETIC_HEAD((*_right)->object);
    dcNode *right = FLAT_ARITHMETIC_TAIL((*_right)->object);

    if (dcNumberClass_isMe(left)
        && dcNumberClass_isMe(right))
    {
        dcNode *copy = dcNode_copy(left, DC_DEEP);
        dcNumberClass_inlineDivide(copy, right);
        dcNode *leftCopy = dcNode_copy((*_left)->object, DC_DEEP);

        if (dcNumberClass_isOneFractional(copy)
            && dcNumberClass_isNegative(leftCopy))
        {
            dcNode *result = dcComplexNumberClass_inlineRaise(leftCopy, copy);
            if (result != leftCopy)
            {
                dcNode_free(&leftCopy, DC_DEEP);
                leftCopy = dcNode_setTemplate(dcNode_copy(result,
                                                          DC_DEEP),
                                              true);
            }

            if (dcComplexNumberClass_isMe(leftCopy)
                && dcComplexNumberClass_isWhole(leftCopy))
            {
                dcComplexNumberClass_chomp(leftCopy);
            }
        }
        else
        {
            dcNumberClass_inlineRaise(leftCopy, copy);
        }

        if (dcComplexNumberClass_isMe(leftCopy)
            || (dcNumberClass_isMe(leftCopy)
                && dcNumberClass_isWholeHelper(leftCopy)))
        {
            setModified(_modified);
            removeElement(_rightArithmetic, _right);
            dcNode_free(&(*_left)->object, DC_DEEP);

            if (dcNumberClass_isMe(leftCopy))
            {
                (*_left)->object = dcNumberClass_chompHelper(leftCopy);
            }
            else
            {
                (*_left)->object = leftCopy;
            }
        }
        else
        {
            dcNode_free(&leftCopy, DC_DEEP);
        }

        dcNode_free(&copy, DC_DEEP);
    }
}

TAFFY_HIDDEN ComparisonLanguage sRaiseLanguages[] =
{
    COMPARISON_ELEMENT("^ >= 2", "$1",     &combineRaiseAndAny),
    COMPARISON_ELEMENT("N",      "N",      &raiseNumbers),
    COMPARISON_ELEMENT("N",      "/ == 2", &raiseNumberAndDivide),
    {0}
};

// x / -a => -x / a
TAFFY_HIDDEN void divideSomethingByNegativeSomething
    (dcFlatArithmetic *_leftArithmetic,
     dcListElement **_left,
     dcFlatArithmetic *_rightArithmetic,
     dcListElement **_right,
     bool *_modified)
{
    setModified(_modified);
    (*_left)->object = CREATE_MULTIPLY(createNumber(-1),
                                       (*_left)->object,
                                       NULL);
    dcFlatArithmetic *right = CAST_FLAT_ARITHMETIC((*_right)->object);
    assert(dcNumberClass_isMe(right->values->head->object));
    dcNumberClass_inlineMultiply(right->values->head->object,
                                 dcNumberClass_getNegativeOneNumberObject());
}

// x & x ==> x
TAFFY_HIDDEN void bitAndSame(dcFlatArithmetic *_leftArithmetic,
                             dcListElement **_left,
                             dcFlatArithmetic *_rightArithmetic,
                             dcListElement **_right,
                             bool *_modified)
{
    setModified(_modified);
    removeElement(_rightArithmetic, _right);
}

TAFFY_HIDDEN ComparisonLanguage sBitAndLanguages[] =
{
    COMPARISON_ELEMENT("$1", "$1", &bitAndSame),
    {0}
};

// x | x ==> x
TAFFY_HIDDEN void bitOrSame(dcFlatArithmetic *_leftArithmetic,
                            dcListElement **_left,
                            dcFlatArithmetic *_rightArithmetic,
                            dcListElement **_right,
                            bool *_modified)
{
    setModified(_modified);
    removeElement(_rightArithmetic, _right);
}

TAFFY_HIDDEN ComparisonLanguage sBitOrLanguages[] =
{
    COMPARISON_ELEMENT("$1", "$1", &bitOrSame),
    {0}
};

TAFFY_HIDDEN ComparisonLanguage sDivideLanguages[] =
{
    COMPARISON_ELEMENT("?", "* >= 2 %NN",  &divideSomethingByNegativeSomething),
    {0}
};

TAFFY_HIDDEN TextResultLanguage sDivideTextResultLangauges[] =
{
    // z / (x / y) = (z * y) / x
    TEXT_RESULT_ELEMENT("A", "/ == 2 B C", "/ ( * A C ) B"),
    {0}
};

TAFFY_HIDDEN ComparisonLanguage sSubtractLanguages[] =
{
    {0}
};

dcString *dcFlatArithmetic_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    return dcMarshaller_marshall(_stream,
                                 "ol",
                                 arithmetic->taffyOperator,
                                 arithmetic->values);
}

bool dcFlatArithmetic_unmarshallNode(dcNode *_node, dcString *_stream)
{
    dcTaffyOperator taffyOperator;
    dcList *values = NULL;
    bool result = false;

    if (dcMarshaller_unmarshallNoNull(_stream,
                                      "ol",
                                      &taffyOperator,
                                      &values))
    {
        result = true;
        dcFlatArithmetic *arithmetic = createFlatArithmetic(taffyOperator);
        // TODO: grouped
        arithmetic->values = values;
        CAST_FLAT_ARITHMETIC(_node) = arithmetic;
    }

    return result;
}

TAFFY_HIDDEN void snip(dcFlatArithmetic *_arithmetic,
                       uint32_t _value,
                       bool _notFirst,
                       bool *_modified)
{
    dcListElement *that;

    for (that = _arithmetic->values->head;
         that != NULL;
        )
    {
        dcListElement *next = that->next;
        bool checkSnip = true;

        if (IS_FLAT_ARITHMETIC(that->object))
        {
            that->object = dcFlatArithmetic_snip(that->object, _modified);
        }

        if (checkSnip
            && _arithmetic->values->size > 1
            && IS_CLASS(that->object)
            && dcNumberClass_equalsInt32u(that->object, _value)
            && (! _notFirst
                || that != _arithmetic->values->head))
        {
            setModified(_modified);
            removeElement(_arithmetic, &that);
        }

        that = next;
    }
}

TAFFY_HIDDEN bool snipZero(dcFlatArithmetic *_arithmetic,
                           bool onlyFirst,
                           bool *_modified)
{
    FOR_EACH(_arithmetic, that)
    {
        if (onlyFirst && that != _arithmetic->values->head)
        {
            break;
        }

        if (dcNumberClass_equalsInt32u(that->object, 0))
        {
            setModified(_modified);
            dcList_clear(_arithmetic->values, DC_DEEP);
            dcList_push(_arithmetic->values, createNumber(0));
            return true;
        }
    }

    return false;
}

TAFFY_HIDDEN void snipMultiply(dcFlatArithmetic *_arithmetic, bool *_modified)
{
    if (! snipZero(_arithmetic, false, _modified))
    {
        // remove 1s!
        snip(_arithmetic, 1, false, _modified);
    }
}

TAFFY_HIDDEN void snipRaise(dcFlatArithmetic *_arithmetic, bool *_modified)
{
    snip(_arithmetic, 1, true, _modified);

    // x^0 == 1 !
    if (_arithmetic->values->size == 2
        && dcNumberClass_equalsInt32u(dcList_getTail(_arithmetic->values), 0))
    {
        setModified(_modified);
        dcList_clear(_arithmetic->values, DC_DEEP);
        dcList_push(_arithmetic->values, createNumber(1));
    }
}

//
// Remove 0s for ADD, 1s for MULTIPLY and DIVIDE
//
dcNode *dcFlatArithmetic_snip(dcNode *_node, bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);

    if (arithmetic->taffyOperator == TAFFY_ADD)
    {
        // remove 0s!
        snip(arithmetic, 0, false, _modified);
    }
    else if (arithmetic->taffyOperator == TAFFY_SUBTRACT)
    {
        // remove 0s (if it's not the first)!
        snip(arithmetic, 0, true, _modified);
    }
    else if (arithmetic->taffyOperator == TAFFY_MULTIPLY)
    {
        snipMultiply(arithmetic, _modified);
    }
    else if (arithmetic->taffyOperator == TAFFY_DIVIDE)
    {
        // remove 1s!
        snip(arithmetic, 1, true, _modified);
    }
    else if (arithmetic->taffyOperator == TAFFY_RAISE)
    {
        // remove 1s!
        snipRaise(arithmetic, _modified);
    }
    else if (arithmetic->taffyOperator == TAFFY_BIT_AND)
    {
        snipZero(arithmetic, false, _modified);
    }
    else if (arithmetic->taffyOperator == TAFFY_BIT_OR)
    {
        snip(arithmetic, 0, false, _modified);
    }
    else if (arithmetic->taffyOperator == TAFFY_LEFT_SHIFT)
    {
        snipZero(arithmetic, true, _modified);
        snip(arithmetic, 0, false, _modified);
    }
    else if (arithmetic->taffyOperator == TAFFY_RIGHT_SHIFT)
    {
        snipZero(arithmetic, true, _modified);
        snip(arithmetic, 0, false, _modified);
    }

    return popIfNotFactorial(_node, _modified);
}

dcResult dcFlatArithmetic_compareNode(dcNode *_left,
                                      dcNode *_right,
                                      dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_FAILURE;
    *_compareResult = TAFFY_LESS_THAN;

    if (_right->type == NODE_GRAPH_DATA
        && dcGraphData_getType(_right) == NODE_FLAT_ARITHMETIC)
    {
        dcFlatArithmetic *left = CAST_FLAT_ARITHMETIC(_left);
        dcFlatArithmetic *right = CAST_FLAT_ARITHMETIC(_right);
        result = TAFFY_SUCCESS;
        *_compareResult = TAFFY_LESS_THAN;

        if (left->values->size == 1
            && right->values->size == 1)
        {
            result = dcNode_compareEqual(left->values->head->object,
                                         right->values->head->object,
                                         _compareResult);
        }
        else if (left->values->size == 1)
        {
            result = dcNode_compareEqual(left->values->head->object,
                                         _right,
                                         _compareResult);
        }
        else if (right->values->size == 1)
        {
            result = dcNode_compareEqual(_left,
                                         right->values->head->object,
                                         _compareResult);
        }
        else if (left->taffyOperator == right->taffyOperator)
        {
            if (left->taffyOperator == TAFFY_MULTIPLY
                || left->taffyOperator == TAFFY_ADD
                || left->taffyOperator == TAFFY_BIT_AND
                || left->taffyOperator == TAFFY_BIT_OR)
            {
                result = TAFFY_SUCCESS;

                // can be an out-of-order match
                dcList *copy = dcList_copy(left->values, DC_SHALLOW);
                dcListElement *that;

                for (that = right->values->head;
                     that != NULL && result == TAFFY_SUCCESS;
                    )
                {
                    dcListElement *next = that->next;
                    dcListElement *found;
                    result = dcList_find(copy, that->object, &found);

                    if (result == TAFFY_SUCCESS)
                    {
                        dcList_removeElement(copy, &found, DC_SHALLOW);
                    }

                    that = next;
                }

                if (result == TAFFY_SUCCESS && copy->size == 0)
                {
                    *_compareResult = TAFFY_EQUALS;
                }

                dcList_free(&copy, DC_SHALLOW);
            }

            if (*_compareResult != TAFFY_EQUALS)
            {
                // need an exact match
                result = dcList_compare(left->values,
                                        right->values,
                                        _compareResult);
            }
        }
    }

    return result;
}

TAFFY_HIDDEN void combineCommutative(dcFlatArithmetic *_arithmetic,
                                     const ComparisonLanguage *_languages,
                                     bool *_modified)
{
    dcListElement *that;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    for (that = _arithmetic->values->head;
         (that != NULL && ! dcNodeEvaluator_abortReceived(evaluator));
        )
    {
        dcListElement *save = that;
        dcListElement *other = NULL;

        for (other = that->next;
             (other != NULL && ! dcNodeEvaluator_abortReceived(evaluator));
            )
        {
            dcListElement *otherSave = other;

            if (other != that)
            {
                combineElements(_languages,
                                _arithmetic,
                                &that,
                                _arithmetic,
                                &other,
                                _modified);
            }

            if (otherSave == other)
            {
                other = other->next;
            }
        }

        if (save == that)
        {
            that = that->next;
        }
    }
}

TAFFY_HIDDEN void combineCommutativeTextResult
    (dcFlatArithmetic *_arithmetic,
     const TextResultLanguage *_textResultLanguages,
     bool *_modified)
{
    dcListElement *that;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    for (that = _arithmetic->values->head;
         (that != NULL && ! dcNodeEvaluator_abortReceived(evaluator));
        )
    {
        dcListElement *save = that;
        dcListElement *other = NULL;

        for (other = _arithmetic->values->head;
             (other != NULL && ! dcNodeEvaluator_abortReceived(evaluator));
            )
        {
            dcListElement *otherSave = other;

            if (other != that)
            {
                combineTextResultElements(_textResultLanguages,
                                          _arithmetic,
                                          &that,
                                          _arithmetic,
                                          &other,
                                          _modified);
            }

            if (otherSave == other)
            {
                other = other->next;
            }
        }

        if (save == that)
        {
            that = that->next;
        }
    }
}

TAFFY_HIDDEN void combineRaise(dcFlatArithmetic *_arithmetic,
                               bool *_modified)
{
    dcListElement *that;

    // top-down!
    for (that = _arithmetic->values->tail->previous; that != NULL; )
    {
        dcListElement *previous = that->previous;
        dcListElement *after = that->next;
        combineElements(sRaiseLanguages,
                        _arithmetic,
                        &that,
                        _arithmetic,
                        &after,
                        _modified);
        that = previous;
    }

    // simplify x^1
    if (_arithmetic->values->size > 1
        && dcNumberClass_equalsInt32u(dcList_getTail(_arithmetic->values), 1))
    {
        setModified(_modified);
        dcList_pop(_arithmetic->values, DC_DEEP);
    }
}

TAFFY_HIDDEN bool combineLeftToRight
    (dcFlatArithmetic *_arithmetic,
     const ComparisonLanguage *_languages,
     const TextResultLanguage *_textResultLanguages,
     bool *_modified)
{
    dcListElement *that;
    bool result = false;

    if (_arithmetic->values->size <= 1)
    {
        return false;
    }

    // left to right!
    for (that = _arithmetic->values->head->next; that != NULL; )
    {
        dcListElement *previous = that->previous;
        dcListElement *save = that;

        // TODO: merge all _languages into _textResultLanguages
        if ((_textResultLanguages != NULL
             && combineTextResultElements(_textResultLanguages,
                                          _arithmetic,
                                          &previous,
                                          _arithmetic,
                                          &that,
                                          _modified))
            || combineElements(_languages,
                               _arithmetic,
                               &previous,
                               _arithmetic,
                               &that,
                               _modified))
        {
            result = true;
        }

        if (that == save)
        {
            that = that->next;
        }
    }

    return result;
}

TAFFY_HIDDEN dcNode *combine(dcNode *_arithmetic, bool *_modified)
{
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_arithmetic);
    dcContainerSizeType sizeSave;
    dcNode *result = _arithmetic;
    dcListElement *that;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    do
    {
        for (that = arithmetic->values->head;
             (that != NULL && ! dcNodeEvaluator_abortReceived(evaluator));
             that = that->next)
        {
            that->object = dcFlatArithmetic_combine(that->object, _modified);
            that->object = dcFlatArithmetic_snip(that->object, _modified);
        }

        if (dcNodeEvaluator_abortReceived(evaluator))
        {
            break;
        }

        sizeSave = arithmetic->values->size;

        if (arithmetic->taffyOperator == TAFFY_ADD)
        {
            combineCommutativeTextResult(arithmetic,
                                         sAddTextResultLanguages,
                                         _modified);
            combineCommutative(arithmetic, sAddLanguages, _modified);
        }
        else if (arithmetic->taffyOperator == TAFFY_SUBTRACT)
        {
            combineLeftToRight(arithmetic, sSubtractLanguages, NULL, _modified);
        }
        else if (arithmetic->taffyOperator == TAFFY_MULTIPLY)
        {
            combineCommutativeTextResult(arithmetic,
                                         sMultiplyTextResultLanguages,
                                         _modified);
            combineCommutative(arithmetic, sMultiplyLanguages, _modified);
        }
        else if (arithmetic->taffyOperator == TAFFY_DIVIDE)
        {
            combineLeftToRight(arithmetic,
                               sDivideLanguages,
                               sDivideTextResultLangauges,
                               _modified);
        }
        else if (arithmetic->taffyOperator == TAFFY_BIT_AND)
        {
            combineLeftToRight(arithmetic, sBitAndLanguages, NULL, _modified);
        }
        else if (arithmetic->taffyOperator == TAFFY_BIT_OR)
        {
            combineLeftToRight(arithmetic, sBitOrLanguages, NULL, _modified);
        }
        else if (arithmetic->taffyOperator == TAFFY_RAISE)
        {
            // go top-down
            combineRaise(arithmetic, _modified);
        }
    } while (arithmetic->values->size < sizeSave
             && ! dcNodeEvaluator_abortReceived(evaluator));

    if (arithmetic->values->size == 1
        && arithmetic->taffyOperator != TAFFY_FACTORIAL)
    {
        result = dcList_pop(arithmetic->values, DC_SHALLOW);
        dcNode_free(&_arithmetic, DC_DEEP);
    }

    return result;
}

//
// Combine elements in a flat arithmetic!
//
dcNode *dcFlatArithmetic_combine(dcNode *_arithmetic, bool *_modified)
{
    return (IS_FLAT_ARITHMETIC(_arithmetic)
            ? combine(_arithmetic, _modified)
            : _arithmetic);
}

void dcFlatArithmetic_sort(dcNode *_arithmetic)
{
    dcListElement *that;
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_arithmetic);

    for (that = arithmetic->values->head; that != NULL; that = that->next)
    {
        if (IS_FLAT_ARITHMETIC(that->object))
        {
            dcFlatArithmetic_sort(that->object);
        }
    }

    if (arithmetic->taffyOperator != TAFFY_RAISE)
    {
        dcList_doSelectionSort(CAST_FLAT_ARITHMETIC(_arithmetic)->values);
    }
}

static bool isMethodCall(dcNode *_node,
                         const char *_methodName,
                         const char *_receiverName)
{
    bool result = false;

    if (IS_METHOD_CALL(_node))
    {
        const dcMethodCall *call = CAST_METHOD_CALL(_node);

        if (call->arguments->size == 1
            && strcmp(call->methodName, _methodName) == 0
            && call->receiver != NULL
            && IS_IDENTIFIER(call->receiver)
            && dcIdentifier_equalsString(call->receiver, _receiverName))
        {
            result = true;
        }
    }

    return result;
}

TAFFY_HIDDEN dcNode **getMethodArgument(dcNode *_methodCall,
                                        uint32_t _wantedIndex,
                                        bool _thatsAll)
{
    const dcMethodCall *call = CAST_METHOD_CALL(_methodCall);

    if (call->arguments->size == 1
        && (strcmp(call->methodName, "#operator(()):") == 0))
    {
        dcArray *arguments = dcArrayClass_getObjects
            (call->arguments->head->object);

        if (_wantedIndex < arguments->size)
        {
            if (! _thatsAll
                || (arguments->size == _wantedIndex + 1))
            {
                return &arguments->objects[_wantedIndex];
            }
        }
    }

    return NULL;
}

dcNode *dcFlatArithmetic_simplifyMethodCall(dcNode *_node, bool *_modified)
{
    if (IS_METHOD_CALL(_node))
    {
        uint32_t i;
        dcNode **argument = NULL;

        for (i = 0, argument = getMethodArgument(_node, i, false);
             argument != NULL;
             i++, argument = getMethodArgument(_node, i, false))
        {
            SHRINK_OPERATION(shrink, *argument, _modified);
        }
    }

    return _node;
}

//
// check whether _object is -1 * x
//
static bool isNegativeOneMultiply(dcNode *_object)
{
    if (IS_FLAT_ARITHMETIC(_object))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_object);

        if (arithmetic->taffyOperator == TAFFY_MULTIPLY
            && dcNumberClass_isMe(arithmetic->values->head->object)
            && dcNumberClass_equalsInt32s(arithmetic->values->head->object, -1))
        {
            return true;
        }
    }

    return false;
}

static bool isNegativeMultiply(dcNode *_node, bool _wantJustTwo)
{
    bool result = false;

    if (IS_FLAT_ARITHMETIC(_node))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
        result = (arithmetic->taffyOperator == TAFFY_MULTIPLY
                  && (_wantJustTwo
                      ? arithmetic->values->size == 2
                      : true)
                  && dcNumberClass_isMe(arithmetic->values->head->object)
                  && (dcNumberClass_isNegative
                      (arithmetic->values->head->object)));
    }

    return result;
}

dcNode *dcFlatArithmetic_undoConvertSubtractToAdd(dcNode *_node,
                                                  bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    dcListElement *that;

    for (that = arithmetic->values->head; that != NULL; that = that->next)
    {
        that->object =
            dcFlatArithmetic_undoConvertSubtractToAdd(that->object, _modified);
    }

    if (! IS_ADD(_node)
        || FLAT_ARITHMETIC_SIZE(_node) <= 1)
    {
        return _node;
    }

    // verify there's at least one negative, we want to bail otherwise
    bool found = false;

    for (that = arithmetic->values->head->next;
         that != NULL;
         that = that->next)
    {
        if (isNegativeMultiply(that->object, false)
            || (dcNumberClass_isMe(that->object)
                && dcNumberClass_isNegative(that->object)))
        {
            found = true;
            break;
        }
    }

    if (! found)
    {
        return _node;
    }

    // it will be modified...
    setModified(_modified);

    dcNode *result = dcNode_copy(arithmetic->values->head->object, DC_DEEP);

    for (that = arithmetic->values->head->next;
         that != NULL;
         that = that->next)
    {
        dcNode *copy = dcNode_copy(that->object, DC_DEEP);

        if (IS_MULTIPLY(copy)
            && dcNumberClass_isNegativeOne(FLAT_ARITHMETIC_HEAD(copy))
            && FLAT_ARITHMETIC_SIZE(copy) > 1)
        {
            // shift away the 1, there's more on the right
            dcList_shift(CAST_FLAT_ARITHMETIC(copy)->values, DC_DEEP);
            result = CREATE_SUBTRACT(result, copy, NULL);
        }
        else if (isNegativeMultiply(copy, false))
        {
            dcNumberClass_negateHelper
                (CAST_FLAT_ARITHMETIC(copy)->values->head->object);
            result = CREATE_SUBTRACT(result, copy, NULL);
        }
        else if (dcNumberClass_isMe(copy)
                 && dcNumberClass_isNegative(copy))
        {
            dcNumberClass_inlineMultiply
                (copy, dcNumberClass_getNegativeOneNumberObject());
            result = CREATE_SUBTRACT(result, copy, NULL);
        }
        else
        {
            result = CREATE_ADD(result, copy, NULL);
        }
    }

    dcNode_free(&_node, DC_DEEP);
    return result;
}

//
// convert subtract to add, examples:
// a - b - c   => a + -b + -c
// a - (b + c) => a + -b + -c
//
dcNode *dcFlatArithmetic_convertSubtractToAdd(dcNode *_node, bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    dcNode *result = _node;
    bool thatModified = false;

    FOR_EACH_IN_NODE(_node, that)
    {
        SHRINK_OPERATION(convertSubtractToAdd, that->object, &thatModified);
    }

    if (arithmetic->taffyOperator == TAFFY_SUBTRACT
        && arithmetic->values->size > 1)
    {
        thatModified = true;

        for (that = arithmetic->values->head->next;
             that != NULL;
             that = that->next)
        {
            if (IS_FLAT_ARITHMETIC(that->object)
                && (CAST_FLAT_ARITHMETIC(that->object)->taffyOperator
                    == TAFFY_ADD))
            {
                dcFlatArithmetic *thatArithmetic =
                    CAST_FLAT_ARITHMETIC(that->object);
                thatArithmetic->taffyOperator = TAFFY_SUBTRACT;
                that->object =
                    dcFlatArithmetic_convertSubtractToAdd(that->object,
                                                          _modified);

                if (! IS_FLAT_ARITHMETIC(that->object))
                {
                    continue;
                }

                thatArithmetic = CAST_FLAT_ARITHMETIC(that->object);

                // convertSubtract only converts neck+ elements,
                // so convert the head too
                if (isNegativeOneMultiply(thatArithmetic->values->head->object))
                {
                    dcList_shift(CAST_FLAT_ARITHMETIC
                                 (thatArithmetic->values->head->object)->values,
                                 DC_DEEP);
                    thatArithmetic->values->head->object =
                        popIfNotFactorial(thatArithmetic->values->head->object,
                                          NULL);
                }
                else if (isNegativeMultiply
                         (thatArithmetic->values->head->object, false))
                {
                    dcNumberClass_negateHelper
                        (FLAT_ARITHMETIC_HEAD
                         (thatArithmetic->values->head->object));
                }
                else
                {
                    thatArithmetic->values->head->object =
                        CREATE_MULTIPLY(createNumber(-1),
                                        thatArithmetic->values->head->object,
                                        NULL);
                }
            }
            else
            {
                if (isNegativeOneMultiply(that->object))
                {
                    dcList_shift(CAST_FLAT_ARITHMETIC(that->object)->values,
                                 DC_DEEP);
                    that->object = popIfNotFactorial(that->object, NULL);
                }
                else if (isNegativeMultiply(that->object, false))
                {
                    dcNumberClass_negateHelper
                        (FLAT_ARITHMETIC_HEAD(that->object));
                }
                else
                {
                    if (dcNumberClass_isMe(that->object))
                    {
                        that->object = dcNumberClass_inlineMultiply
                            (that->object,
                             dcNumberClass_getNegativeOneNumberObject());
                    }
                    else if (IS_MULTIPLY(that->object)
                             && (dcNumberClass_isMe
                                 (FLAT_ARITHMETIC_HEAD(that->object))))
                    {
                        dcNumberClass_negateHelper
                            (FLAT_ARITHMETIC_HEAD(that->object));
                    }
                    else if (IS_DIVIDE(that->object))
                    {
                        FLAT_ARITHMETIC_HEAD(that->object) =
                            (CREATE_MULTIPLY
                             (createNumber(-1),
                              FLAT_ARITHMETIC_HEAD(that->object),
                              NULL));
                    }
                    else
                    {
                        that->object = (CREATE_MULTIPLY
                                        (createNumber(-1),
                                         that->object,
                                         NULL));
                    }
                }
            }
        }

        arithmetic->taffyOperator = TAFFY_ADD;
    }

    if (thatModified)
    {
        setModified(_modified);
    }

    return (thatModified
            ? dcFlatArithmetic_merge(result, NULL)
            : result);
}

// note: _node must be shrunk, sorted, have a single identifier,
// and must be converted to add
dcArray *dcFlatArithmetic_getOrderedPolynomialCoefficients(dcNode *_node,
                                                           bool _needIntegers)
{
    dcArray *result = NULL;
    int32_t maxDegree = 0;

    if (dcNumberClass_isMe(_node))
    {
        if (! _needIntegers
            || dcNumberClass_isWholeHelper(_node))
        {
            result = (dcArray_createWithObjects
                      (dcNumberClass_convertToInteger
                       (dcNode_copyAndTemplate(_node)),
                       NULL));
        }
    }
    else if (IS_ADD(_node)
             && dcFlatArithmetic_shrunkenDegree(_node, NULL, &maxDegree))
    {
        result = dcArray_createWithSize(maxDegree + 1);
        int32_t lastDegree = -1;

        FOR_EACH_IN_NODE(_node, that)
        {
            int32_t degree = 0;

            if (! dcFlatArithmetic_shrunkenDegree(that->object, NULL, &degree))
            {
                dcArray_free(&result, DC_DEEP);
                break;
            }

            assert(degree <= maxDegree);
            dcNode *coefficient = getCoefficient(that->object);

            if (_needIntegers && ! dcNumberClass_isWholeHelper(coefficient))
            {
                dcNode_free(&coefficient, DC_DEEP);
                dcArray_free(&result, DC_DEEP);
                result = NULL;
                break;
            }

            if (degree == -1)
            {
                // special case of 0
                degree = 0;
            }

            TAFFY_DEBUG(assert(result->objects[degree] == NULL));
            dcArray_set(result,
                        dcNumberClass_convertToInteger(coefficient),
                        degree);

            if (lastDegree != -1)
            {
                assert(lastDegree > degree);

                if (lastDegree > degree + 1)
                {
                    // we're missing some, so fill them in
                    int32_t i;

                    for (i = lastDegree - 1; i > degree; i--)
                    {
                        TAFFY_DEBUG(assert(result->objects[i] == NULL));
                        dcArray_set(result, createNumber(0), i);
                    }
                }
            }

            lastDegree = degree;
        }

        if (lastDegree > 0 && result != NULL)
        {
            int32_t i;

            for (i = lastDegree - 1; i >= 0; i--)
            {
                TAFFY_DEBUG(assert(result->objects[i] == NULL));
                dcArray_set(result, createNumber(0), i);
            }
        }
    }

    return result;
}

// note: _node must be shrunk, and must be converted to add
dcArray *dcFlatArithmetic_getPolynomialCoefficients(dcNode *_node,
                                                    bool _needIntegers)
{
    dcArray *result = NULL;

    if (dcNumberClass_isMe(_node))
    {
        if (! _needIntegers
            || dcNumberClass_isWholeHelper(_node))
        {
            result = (dcArray_createWithObjects
                      (dcNode_copyAndTemplate(_node),
                       NULL));
        }
    }
    else if (IS_ADD(_node))
    {
        result = dcArray_createWithSize(FLAT_ARITHMETIC_SIZE(_node) + 1);
        uint32_t i = 0;

        FOR_EACH_IN_NODE(_node, that)
        {
            dcNode *coefficient = getCoefficient(that->object);
            dcArray_set(result, coefficient, i);
            i++;
        }
    }
    else if (IS_MULTIPLY(_node)
             && dcNumberClass_isMe(FLAT_ARITHMETIC_HEAD(_node)))
    {
        result = (dcArray_createWithObjects
                  (dcNode_copyAndTemplate(FLAT_ARITHMETIC_HEAD(_node)),
                   NULL));
    }

    return result;
}

static dcNode *factorPolynomialByGrouping(dcNode *_first,
                                          dcNode *_a,
                                          dcNode *_b,
                                          dcNode *_c,
                                          dcNode *_d,
                                          bool *_modified)
{
    dcNode *aCopy = dcNode_copy(_a, DC_DEEP);
    dcNode *bCopy = dcNode_copy(_b, DC_DEEP);
    dcNode *cCopy = dcNode_copy(_c, DC_DEEP);
    dcNode *dCopy = dcNode_copy(_d, DC_DEEP);

    dcNode *left = CREATE_ADD(aCopy, bCopy, NULL);
    left = (popIfNotFactorial
            (dcFlatArithmetic_multiFactor
             (left, NULL),
             NULL));
    dcNode *right = CREATE_ADD(cCopy, dCopy, NULL);
    right = (popIfNotFactorial
             (dcFlatArithmetic_multiFactor
              (right, NULL),
              NULL));

    dcNode *result = NULL;
    bool success = false;

    if (IS_MULTIPLY(left)
        && IS_MULTIPLY(right)
        && IS_FLAT_ARITHMETIC(FLAT_ARITHMETIC_TAIL(left))
        && IS_FLAT_ARITHMETIC(FLAT_ARITHMETIC_TAIL(right)))
    {
        dcNode *leftTail = FLAT_ARITHMETIC_TAIL(left);
        dcNode *rightTail = FLAT_ARITHMETIC_TAIL(right);

        if (dcFlatArithmetic_equals(leftTail, rightTail))
        {
            success = true;
        }
        else
        {
            right = CREATE_MULTIPLY(createNumber(-1), right, NULL);
            FLAT_ARITHMETIC_TAIL(right) = (CREATE_MULTIPLY
                                           (createNumber(-1),
                                            FLAT_ARITHMETIC_TAIL(right),
                                            NULL));
            rightTail = FLAT_ARITHMETIC_TAIL(right);

            if (dcFlatArithmetic_equals(leftTail, rightTail))
            {
                success = true;
            }
        }

        if (success)
        {
            setModified(_modified);
            dcNode *tail = dcList_pop(CAST_FLAT_ARITHMETIC(right)->values,
                                      DC_FLOATING);
            dcList_pop(CAST_FLAT_ARITHMETIC(left)->values, DC_DEEP);

            if (_first != NULL)
            {
                result = CREATE_MULTIPLY(_first,
                                         CREATE_ADD(left, right, NULL),
                                         tail,
                                         NULL);
            }
            else
            {
                result = CREATE_MULTIPLY(CREATE_ADD(left, right, NULL),
                                         tail,
                                         NULL);
            }

            SHRINK_OPERATION(snip, result, NULL);
            SHRINK_OPERATION(convert, result, NULL);
            SHRINK_OPERATION(undoConvertSubtractToAdd, result, NULL);
        }
    }

    if (! success)
    {
        dcNode_free(&left, DC_DEEP);
        dcNode_free(&right, DC_DEEP);
    }

    return result;
}

dcNode *dcFlatArithmetic_factorPolynomialByGrouping(dcNode *_node,
                                                    bool *_modified)
{
    dcNode *result = NULL;
    bool modified = false;
    dcNode *copy = NULL;
    dcNode *first = NULL;

    if (! dcFlatArithmetic_isPolynomial(_node))
    {
        return _node;
    }

    copy = dcNode_copy(_node, DC_DEEP);
    SHRINK_OPERATION(convertSubtractToAdd, copy, NULL);

    if (! IS_ADD(copy)
        || FLAT_ARITHMETIC_SIZE(copy) != 4)
    {
        dcNode_free(&copy, DC_DEEP);
        return _node;
    }

    SHRINK_OPERATION(factorPolynomialByGcd, copy, &modified);
    SHRINK_OPERATION(convertSubtractToAdd, copy, NULL);
    first = NULL;

    if (modified)
    {
        first = dcList_shift(CAST_FLAT_ARITHMETIC(copy)->values, DC_FLOATING);
        copy = popIfNotFactorial(copy, NULL);
    }

    assert(IS_ADD(copy)
           && FLAT_ARITHMETIC_SIZE(copy) == 4);

    dcFlatArithmetic *math = CAST_FLAT_ARITHMETIC(copy);
    dcNode *a = dcList_shift(math->values, DC_FLOATING);
    dcNode *b = dcList_shift(math->values, DC_FLOATING);
    dcNode *c = dcList_shift(math->values, DC_FLOATING);
    dcNode *d = dcList_shift(math->values, DC_FLOATING);
    dcNode_free(&copy, DC_DEEP);

    modified = false;
    result = (factorPolynomialByGrouping
              (first, a, b, c, d, &modified));

    if (! modified)
    {
        result = (factorPolynomialByGrouping
                  (first, a, c, b, d, &modified));
    }

    if (! modified)
    {
        result = (factorPolynomialByGrouping
                  (first, a, d, b, c, &modified));
    }

    if (modified)
    {
        TAFFY_DEBUG(assert(result != NULL));
        dcNode_free(&_node, DC_DEEP);
        setModified(_modified);
    }
    else
    {
        dcNode_free(&first, DC_DEEP);
        dcNode_free(&result, DC_DEEP);
        dcNode_free(&copy, DC_DEEP);
        result = _node;
    }

    dcNode_free(&a, DC_DEEP);
    dcNode_free(&b, DC_DEEP);
    dcNode_free(&c, DC_DEEP);
    dcNode_free(&d, DC_DEEP);
    return result;
}

dcNode *dcFlatArithmetic_factorPolynomialByGcd(dcNode *_node, bool *_modified)
{
    if (! dcFlatArithmetic_isPolynomial(_node))
    {
        return _node;
    }

    dcNode *copy = dcNode_copy(_node, DC_DEEP);
    dcNode *result = NULL;
    SHRINK_OPERATION(convertSubtractToAdd, copy, NULL);

    dcArray *coefficients = (dcFlatArithmetic_getPolynomialCoefficients
                             (copy, true));

    if (coefficients == NULL)
    {
        // bail
        dcNode_free(&copy, DC_DEEP);
        return _node;
    }

    dcNumber *gcdResult = dcNumber_createFromInt32u(0);

    FOR_EACH_IN_ARRAY(coefficients, i, object)
    {
        dcNumber_gcd(gcdResult,
                     gcdResult,
                     dcNumberClass_getNumber(object));
    }

    if (dcNumber_isWhole(gcdResult)
        && ! dcNumber_equalsInt32u(gcdResult, 1)
        && ! dcNumber_equalsInt32u(gcdResult, 0))
    {
        setModified(_modified);
        dcNode *gcd = (dcNode_setTemplate
                       (dcNumberClass_createObject(gcdResult), true));
        dcNode *bottom = dcNode_copy(gcd, DC_DEEP);
        assert(dcFlatArithmetic_cancelTopAndBottom
               (&copy, &bottom, false));
        dcNode_free(&bottom, DC_DEEP);
        dcNode_free(&_node, DC_DEEP);
        result = dcFlatArithmetic_snip(copy, NULL);
        result = CREATE_MULTIPLY(gcd, result, NULL);
        SHRINK_OPERATION(undoConvertSubtractToAdd, result, NULL);
    }
    else
    {
        dcNumber_free(&gcdResult, DC_DEEP);
        dcNode_free(&copy, DC_DEEP);
        result = _node;
    }

    dcArray_free(&coefficients, DC_DEEP);
    return result;
}

// note: _node must be shrunken, sorted, and converted to add
dcNode *dcFlatArithmetic_factorPolynomialByRationalRoots(dcNode *_node,
                                                         bool *_modified)
{
    // the motherload //
    dcNode *identifier = NULL;
    dcNode *result = NULL;
    dcNode *copy = NULL;
    dcHash *kHash = NULL;
    dcArray *coefficients = NULL;
    dcList *lastFactors = NULL;
    dcList *firstFactors = NULL;
    dcList *kValues = NULL;
    dcNode *first = NULL;
    dcNode *last = NULL;
    dcList *symbols = NULL;
    bool success = true;
    dcNode *remainder = NULL;
    dcNode *quotient = NULL;
    dcList *divisors = NULL;
    dcListElement *kValue = NULL;
    dcListElement *iThat = NULL;

    // TODO: remove me
    if (! dcFlatArithmetic_isPolynomial(_node)
        || ! dcFlatArithmetic_hasSingleIdentifier(_node, &identifier)
        || identifier == NULL)
    {
        return _node;
    }

    identifier = dcNode_copy(identifier, DC_DEEP);
    copy = dcNode_copy(_node, DC_DEEP);
    kHash = dcHash_create();
    coefficients =
        dcFlatArithmetic_getOrderedPolynomialCoefficients(copy, true);

    if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
    {
        dcLog_logLine(FLAT_ARITHMETIC_FACTOR_LOG,
                      "%scoefficients: %s\n",
                      indent(),
                      dcArray_display(coefficients));
    }

    if (coefficients == NULL)
    {
        dcLog_logLine(FLAT_ARITHMETIC_FACTOR_LOG,
                      "%scouldn't find coefficients, returning",
                      indent());
        goto kickout;
    }

    first = dcArray_get(coefficients, coefficients->size - 1);
    last = dcArray_get(coefficients, 0);

    if (! (dcNumberClass_isMe(first)
           && dcNumberClass_isMe(last)
           && ! dcNumberClass_isZero(first)
           && ! dcNumberClass_isZero(last)))
    {
        if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
        {
            dcLog_logLine(FLAT_ARITHMETIC_FACTOR_LOG,
                          "%sfirst: %s or last: %s aren't nice numbers, "
                          "returning",
                          indent(),
                          dcNode_display(first),
                          dcNode_display(last));
        }

        goto kickout;
    }

    lastFactors = dcNumber_getFactors(dcNumberClass_getNumber(last), false);
    firstFactors = dcNumber_getFactors(dcNumberClass_getNumber(first), false);
    kValues = dcList_create();

    if (lastFactors == NULL
        || firstFactors == NULL)
    {
        dcLog_logLine(FLAT_ARITHMETIC_FACTOR_LOG,
                      "%scouldn't find first or last factors, exiting",
                      indent());
        goto kickout;
    }

    if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
    {
        dcLog_logLine(FLAT_ARITHMETIC_FACTOR_LOG,
                      "%slastFactors: %s\n",
                      indent(),
                      dcList_display(lastFactors));
        dcLog_logLine(FLAT_ARITHMETIC_FACTOR_LOG,
                      "%sfirstFactors: %s\n",
                      indent(),
                      dcList_display(firstFactors));
    }

    symbols = (dcList_createWithObjects
               (dcNode_copyAndTemplate(identifier),
                NULL));

    for (iThat = lastFactors->head; iThat != NULL; iThat = iThat->next)
    {
        if (result != NULL)
        {
            break;
        }

        FOR_EACH_IN_LIST(firstFactors, jThat)
        {
            assert(result == NULL);
            int32_t i = CAST_INT(iThat->object);
            int32_t j = CAST_INT(jThat->object);
            dcNode *k = (dcFlatArithmetic_cancel
                         (CREATE_DIVIDE(createNumber(i),
                                        createNumber(j),
                                        NULL),
                          NULL));
            char *kString = dcNode_synchronizedDisplay(k);

            if (dcHash_getValueWithStringKey(kHash,
                                             kString,
                                             NULL)
                == TAFFY_SUCCESS)
            {
                dcLog_logLine(FLAT_ARITHMETIC_FACTOR_LOG,
                              "found %s in kHash, moving along...",
                              kString);
                dcNode_free(&k, DC_DEEP);
                dcMemory_free(kString);
                continue;
            }

            dcLog_logLine(FLAT_ARITHMETIC_FACTOR_LOG,
                          "looking at i: %d, j: %d, k: %d\n",
                          i, j, k);

            dcHash_setValueWithStringKey(kHash,
                                         kString,
                                         dcVoid_createNode(NULL));
            dcMemory_free(kString);
            dcLog_logLine(FLAT_ARITHMETIC_FACTOR_LOG,
                          "looking at i: %d, j: %d, k: %s\n",
                          i, j, kString);

            if (dcFlatArithmetic_isSingleRoot
                (copy,
                 k,
                 dcIdentifier_getName(identifier)))
            {
                dcLog_logLine(FLAT_ARITHMETIC_FACTOR_LOG,
                              "found root: i: %d, j: %d, k: %d\n",
                              i, j, k);
                setModified(_modified);
                dcList_push(kValues,
                            dcPair_createNode
                            (dcUnsignedInt32_createNode(i),
                             dcUnsignedInt32_createNode(j)));
            }

            dcNode_free(&k, DC_DEEP);
            dcMemory_free(kString);
        }
    }

    divisors = dcList_create();

    for (kValue = kValues->head; kValue != NULL; kValue = kValue->next)
    {
        setModified(_modified);
        dcPair *pair = CAST_PAIR(kValue->object);
        int32_t i = CAST_INT(pair->left);
        int32_t j = CAST_INT(pair->right);

        dcNode *divisor = (CREATE_SUBTRACT
                           (CREATE_MULTIPLY
                            (dcNode_copyAndTemplate(identifier),
                             createNumber(-j),
                             NULL),
                            createNumber(-i),
                            NULL));
        SHRINK_OPERATION(convertSubtractToAdd,
                         divisor,
                         NULL);
        SHRINK_OPERATION(combine, divisor, NULL);
        SHRINK_OPERATION(convert, divisor, NULL);

        if (dcFlatArithmetic_dividePolynomials(copy,
                                               divisor,
                                               symbols,
                                               &quotient,
                                               &remainder))
        {
            if (dcNumberClass_isMe(remainder)
                && dcNumberClass_equalsInt32u(remainder, 0))
            {
                // success! //
                dcList_push(divisors, divisor);
                dcNode_free(&copy, DC_DEEP);
                copy = quotient;
                dcNode_free(&remainder, DC_DEEP);
            }
            else
            {
                // remainder is not 0, failure //
                dcNode_free(&divisor, DC_DEEP);
                dcNode_free(&quotient, DC_DEEP);
                dcNode_free(&remainder, DC_DEEP);
                success = false;
                break;
            }
        }
        else
        {
            // polynomial division failed //
            dcNode_free(&divisor, DC_DEEP);
            success = false;
            break;
        }
    }

    if (success)
    {
        // we now have a bunch of roots, so the result is them
        // all multiplied up
        result = dcFlatArithmetic_createNodeWithList(TAFFY_MULTIPLY, divisors);
        dcList_push(CAST_FLAT_ARITHMETIC(result)->values, copy);
        copy = NULL;
    }
    else
    {
        // gdi!
        dcList_free(&divisors, DC_DEEP);
    }

kickout:
    dcList_free(&symbols, DC_DEEP);
    dcList_free(&kValues, DC_DEEP);
    dcList_free(&lastFactors, DC_DEEP);
    dcList_free(&firstFactors, DC_DEEP);

    if (result == NULL)
    {
        result = _node;
    }
    else
    {
        dcNode_free(&_node, DC_DEEP);
    }

    dcNode_free(&copy, DC_DEEP);
    dcArray_free(&coefficients, DC_DEEP);
    dcHash_free(&kHash, DC_DEEP);
    dcNode_free(&identifier, DC_DEEP);
    return result;
}

dcNode *dcFlatArithmetic_convertDivideToMultiply(dcNode *_node, bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    dcListElement *that;

    //
    // convert: (x / y)^2
    //      to: x^2 * (1 / y^2)
    //
    if (arithmetic->taffyOperator == TAFFY_RAISE
        && IS_DIVIDE(arithmetic->values->head->object)
        && FLAT_ARITHMETIC_SIZE(arithmetic->values->head->object) == 2)
    {
        dcNode *divide = dcList_shift(arithmetic->values, DC_FLOATING);
        dcNode *top = FLAT_ARITHMETIC_HEAD(divide);
        dcNode *bottom = FLAT_ARITHMETIC_TAIL(divide);
        dcList_clear(CAST_FLAT_ARITHMETIC(divide)->values, DC_FLOATING);

        // create the left
        dcList *leftList = dcList_createWithObjects(top, NULL);
        dcList_concat(leftList, arithmetic->values, DC_DEEP);
        dcNode *left = dcFlatArithmetic_createNodeWithList(TAFFY_RAISE,
                                                           leftList);

        // create the right, a little more complicated
        dcList *rightList = dcList_createWithObjects(bottom, NULL);
        dcList_concat(rightList, arithmetic->values, DC_SHALLOW);
        dcList_clear(arithmetic->values, DC_FLOATING);
        dcNode *right = (CREATE_DIVIDE
                         (createNumber(1),
                          dcFlatArithmetic_createNodeWithList(TAFFY_RAISE,
                                                              rightList),
                          NULL));

        dcNode_free(&divide, DC_DEEP);
        dcNode_free(&_node, DC_DEEP);

        setModified(_modified);
        return CREATE_MULTIPLY(left, right, NULL);
    }

    for (that = arithmetic->values->head; that != NULL; that = that->next)
    {
        that->object = dcFlatArithmetic_convertDivideToMultiply(that->object,
                                                                _modified);
    }

    dcNode *result = _node;

    //
    // convert: x / y
    //      to: x * (1 / y)
    //
    if (arithmetic->taffyOperator == TAFFY_DIVIDE)
    {
        setModified(_modified);
        dcNode *head = dcList_shift(arithmetic->values, DC_FLOATING);
        result = dcFlatArithmetic_createNode(TAFFY_MULTIPLY);
        dcFlatArithmetic *resulty = CAST_FLAT_ARITHMETIC(result);

        if (! dcNumberClass_equalsNumber(head,
                                         dcNumberClass_getOneNumberObject()))
        {
            dcList_push(resulty->values, head);
        }
        else
        {
            dcNode_free(&head, DC_DEEP);
        }

        while (arithmetic->values->size > 0)
        {
            dcNode *bottom = dcList_shift(arithmetic->values, DC_FLOATING);

            if (IS_MULTIPLY(bottom))
            {
                FOR_EACH_IN_NODE(bottom, value)
                {
                    dcList_push(resulty->values,
                                CREATE_DIVIDE(createNumber(1),
                                              dcNode_copy(value->object,
                                                          DC_DEEP),
                                              NULL));
                }

                dcNode_free(&bottom, DC_DEEP);
            }
            else
            {
                dcList_push(resulty->values,
                            CREATE_DIVIDE(createNumber(1), bottom, NULL));
            }
        }

        dcNode_free(&_node, DC_DEEP);
    }

    return popIfNotFactorial(result, NULL);
}

// convert x^(y + z) to x^y * x^z
dcNode *dcFlatArithmetic_expandRaise(dcNode *_node, bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    dcListElement *that;

    for (that = arithmetic->values->head; that != NULL; that = that->next)
    {
        that->object = dcFlatArithmetic_expandRaise(that->object, _modified);
    }

    dcNode *result = _node;

    if (arithmetic->taffyOperator == TAFFY_RAISE
        && arithmetic->values->size == 2
        && IS_ADD(arithmetic->values->tail->object))
    {
        setModified(_modified);

        dcList *values = dcList_create();
        dcNode *head = dcList_shift(arithmetic->values, DC_SHALLOW);
        dcNode *addNode = dcList_shift(arithmetic->values, DC_SHALLOW);
        dcFlatArithmetic *add = CAST_FLAT_ARITHMETIC(addNode);

        while (add->values->size > 0)
        {
            dcNode *exponent = dcList_shift(add->values, DC_SHALLOW);
            dcList_push(values,
                        CREATE_RAISE(dcNode_copy(head, DC_DEEP),
                                     exponent,
                                     NULL));
        }

        dcNode_free(&addNode, DC_DEEP);
        dcNode_free(&head, DC_DEEP);
        result = dcFlatArithmetic_createNodeWithList(TAFFY_MULTIPLY, values);
        dcNode_free(&_node, DC_DEEP);
    }

    return popIfNotFactorial(result, NULL);
}

static dcNode *factorByGcd(dcNode *_node,
                           dcNode **_residual,
                           dcFlatArithmeticOperation _factorOperation,
                           bool *_modified)
{
    dcNode *result = _node;
    bool modified = false;
    SHRINK_OPERATION(factorPolynomialByGcd, result, &modified);
    SHRINK_OPERATION(convertSubtractToAdd, result, NULL);

    if (modified)
    {
        dcLog_logLine(FLAT_ARITHMETIC_VERBOSE_LOG,
                      "factored polynomial by gcd into: %s",
                      dcNode_display(result));
        *_residual = dcList_shift(CAST_FLAT_ARITHMETIC(result)->values,
                                  DC_FLOATING);
        result = popIfNotFactorial(result, NULL);
    }
    else
    {
        dcLog_logLine(FLAT_ARITHMETIC_VERBOSE_LOG,
                      "couldn't factor top polynomial by gcd");
    }

    return _factorOperation(result, _modified);
}

// for a division, try to factor using the given factor operation,
// and then cancel. it's success if that results in a change during canceling
static dcNode *factorDivide(dcNode *_node,
                            dcFlatArithmeticOperation _factorOperation,
                            bool *_modified)
 {
    bool myModified = false;
    dcNode *copy = dcNode_copy(_node, DC_DEEP);
    dcNode *result = _node;
    bool success = false;

    SHRINK_OPERATION(orderPolynomial, copy, NULL);
    SHRINK_OPERATION(convertSubtractToAdd, copy, NULL);

    dcNode *top = dcList_shift(CAST_FLAT_ARITHMETIC(copy)->values, DC_FLOATING);
    dcNode *bottom = dcList_shift(CAST_FLAT_ARITHMETIC(copy)->values,
                                  DC_FLOATING);
    dcNode_free(&copy, DC_DEEP);
    copy = NULL;

    dcNode *topResidual = NULL;
    dcNode *bottomResidual = NULL;

    top = factorByGcd(top, &topResidual, _factorOperation, &myModified);
    bottom = factorByGcd(bottom,
                         &bottomResidual,
                         _factorOperation,
                         &myModified);
    if (myModified)
    {
        myModified = false;
        copy = CREATE_DIVIDE(top, bottom, NULL);
        SHRINK_OPERATION(undoConvertSubtractToAdd, copy, NULL);
        SHRINK_OPERATION(cancel, copy, &myModified);

        if (myModified || IS_RAISE(copy))
        {
            success = true;
            setModified(_modified);
            dcNode_free(&_node, DC_DEEP);
            result = copy;

            if (topResidual != NULL)
            {
                result = CREATE_MULTIPLY(topResidual, result, NULL);
            }

            if (bottomResidual != NULL)
            {
                result = (CREATE_MULTIPLY
                          (result,
                           CREATE_DIVIDE
                           (createNumber(1),
                            bottomResidual,
                            NULL)));
            }

            SHRINK_OPERATION(snip, result, NULL);
            SHRINK_OPERATION(convertSubtractToAdd, result, NULL);
            SHRINK_OPERATION(combine, result, NULL);
            SHRINK_OPERATION(undoConvertSubtractToAdd, result, NULL);
        }
    }
    else
    {
        dcNode_free(&top, DC_DEEP);
        dcNode_free(&bottom, DC_DEEP);
    }

    if (! success)
    {
        dcNode_free(&topResidual, DC_DEEP);
        dcNode_free(&bottomResidual, DC_DEEP);
        dcNode_free(&copy, DC_DEEP);
    }

    return result;
}

static dcNode *tryToDistributeLikeAMadman(dcNode *_node, bool *_modified)
{
    if (! IS_ADD(_node))
    {
        return _node;
    }

    dcNode *copy = dcNode_copy(_node, DC_DEEP);
    dcNode *result = _node;
    bool changed = false;
    SHRINK_OPERATION(distributeLikeAMadman, copy, &changed);

    if (changed)
    {
        SHRINK_OPERATION(merge, copy, NULL);
        SHRINK_OPERATION(distribute, copy, NULL);
        SHRINK_OPERATION(merge, copy, NULL);

        changed = false;
        SHRINK_OPERATION(combine, copy, &changed);

        if (changed
            && (! IS_FLAT_ARITHMETIC(copy)
                || (IS_ADD(copy)
                    && (FLAT_ARITHMETIC_SIZE(copy)
                        < FLAT_ARITHMETIC_SIZE(result)))))
        {
            if (dcLog_isEnabled(FLAT_ARITHMETIC_VERBOSE_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_VERBOSE_LOG,
                          "Ooooh yeah, distributed like a "
                          "madman from: %s to: %s\n",
                          dcNode_display(result),
                          dcNode_display(copy));
            }

            // success!
            dcNode_free(&result, DC_DEEP);
            result = copy;
            setModified(_modified);
        }
        else
        {
            dcNode_free(&copy, DC_DEEP);
        }
    }
    else
    {
        dcNode_free(&copy, DC_DEEP);
    }

    return result;
}

dcNode *dcFlatArithmetic_factorDivideWithRationalRoots(dcNode *_arithmetic,
                                                       bool *_modified)
{
    return factorDivide(_arithmetic,
                        dcFlatArithmetic_factorPolynomialByRationalRoots,
                        _modified);
}

dcNode *dcFlatArithmetic_factorDivideWithQuadratic(dcNode *_arithmetic,
                                                   bool *_modified)
{
    return factorDivide(_arithmetic,
                        dcFlatArithmetic_factorQuadratic,
                        _modified);
}

dcNode *dcFlatArithmetic_shrink(dcNode *_arithmetic, bool *_modified)
{
    dcNode *result = _arithmetic;
    size_t maxIterations = 20;
    size_t i;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcStringCacheElement element = {0};
    bool isModified = false;

    if (_arithmetic == NULL)
    {
        return NULL;
    }

    // check whether it's cached //
    if (dcStringCache_getVoidOrNot(sShrinkCache,
                                   _arithmetic,
                                   &element,
                                   _modified)
        == TAFFY_SUCCESS)
    {
        result = element.value;
        dcStringCacheElement_free(&element);
        return result;
    }

    addCount("dcFlatArithmetic_shrink");

    if (dcLog_isEnabled(FLAT_ARITHMETIC_SHRINK_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_SHRINK_LOG,
                  "%sshrinking %s\n",
                  indent(),
                  dcNode_display(_arithmetic));
    }

    if (IS_FLAT_ARITHMETIC(_arithmetic))
    {
        bool changed = true;
        SHRINK_OPERATION(convertSubtractToAdd, result, NULL);
        SHRINK_OPERATION(combine, result, &isModified);
        SHRINK_OPERATION(cancel, result, &isModified);

        if (IS_FLAT_ARITHMETIC(result))
        {
            FOR_EACH_IN_NODE(result, that)
            {
                if (dcNodeEvaluator_abortReceived(evaluator))
                {
                    break;
                }

                SHRINK_OPERATION(shrink, that->object, &isModified);
            }
        }

        SHRINK_OPERATION(factor, result, NULL);
        SHRINK_OPERATION(convertSubtractToAdd, result, NULL);
        SHRINK_OPERATION(convertDivideToMultiply, result, NULL);

        for (i = 0;
             (changed
              && i < maxIterations // safety, shouldn't be needed
              && ! dcNodeEvaluator_abortReceived(evaluator)
              && IS_FLAT_ARITHMETIC(result));
             i++)
        {
            changed = false;
            SHRINK_OPERATION(orderSubtract, result, NULL);
            SHRINK_OPERATION(combine, result, &changed);
            SHRINK_OPERATION(merge, result, NULL);
            SHRINK_OPERATION(combine, result, NULL);
            SHRINK_OPERATION(snip, result, &changed);
            SHRINK_OPERATION(convert, result, &changed);
            SHRINK_OPERATION(cancel, result, &changed);
            SHRINK_OPERATION(simplifyTrigonometry, result, &changed);
            SHRINK_OPERATION(multiplyByDenominator, result, &changed);
            SHRINK_OPERATION(convertSubtractToAdd, result, NULL);

            // try to distribute like a madman
            if (IS_ADD(result))
            {
                result = tryToDistributeLikeAMadman(result, &changed);
            }
            else if (IS_FLAT_ARITHMETIC(result))
            {
                FOR_EACH_IN_NODE(result, that)
                {
                    that->object = (tryToDistributeLikeAMadman
                                    (that->object,
                                     &changed));
                }
            }

            SHRINK_OPERATION(distribute, result, &changed);

            if (dcLog_isEnabled(FLAT_ARITHMETIC_SHRINK_ITERATION_LOG))
            {
                dcLog_logLine(FLAT_ARITHMETIC_SHRINK_ITERATION_LOG,
                              "%s i = %u, result = %s, changed = %s",
                              indent(),
                              i,
                              dcNode_display(result),
                              (changed
                               ? "yes"
                               : "no"));
            }

            if (changed)
            {
                setModified(&isModified);
            }

#ifdef ENABLE_DEBUG
        assert(i < maxIterations);
#else
        // it's a bug, but we don't want everything to fall down because of it
        if (i >= maxIterations)
        {
            break;
        }
#endif
        }
    }

    if (result != NULL && IS_METHOD_CALL(result))
    {
        SHRINK_OPERATION(simplifyMethodCall, result, &isModified);
        SHRINK_OPERATION(simplifyTrigonometry, result, &isModified);
    }

    if (result != NULL)
    {
        //SHRINK_OPERATION(multiFactor, result, NULL);
    }

    //
    // try to factor and cancel using the rational roots theorem
    //
    if (result != NULL
        && IS_DIVIDE(result)
        && FLAT_ARITHMETIC_SIZE(result) == 2)
    {
        SHRINK_OPERATION(factorDivideWithRationalRoots, result, &isModified);
    }

    //
    // try to factor a divide using quadratic
    //
    if (result != NULL
        && IS_DIVIDE(result)
        && FLAT_ARITHMETIC_SIZE(result) == 2)
    {
        SHRINK_OPERATION(factorDivideWithQuadratic, result, &isModified);
    }

    //
    // try to factor a divide using difference of squares
    //
    //if (result != NULL
    //    && IS_DIVIDE(result)
    //    && FLAT_ARITHMETIC_SIZE(result) == 2)
    //{
    //    result = factorDivide(result,
    //                          dcFlatArithmetic_factorDifferenceOfSquares,
    //                          &isModified);
    //}

    if (result != NULL)
    {
        SHRINK_OPERATION(factorQuadratic, result, &isModified);
        SHRINK_OPERATION(cancel, result, &isModified);

        bool myModified = false;
        SHRINK_OPERATION(convertDivideToMultiply, result, &myModified);

        if (myModified)
        {
            myModified = false;
            SHRINK_OPERATION(combine, result, &myModified);

            if (myModified)
            {
                SHRINK_OPERATION(cancel, result, &isModified);
                SHRINK_OPERATION(snip, result, NULL);
            }
        }

        SHRINK_OPERATION(undoConvertSubtractToAdd, result, NULL);
        SHRINK_OPERATION(moveNumberToFront, result, NULL);
        SHRINK_OPERATION(collectPowers, result, NULL);
        SHRINK_OPERATION(orderPolynomial, result, NULL);
    }

    // TODO: don't neccessitate need for display
    char *resultDisplay = dcNode_synchronizedDisplay(result);

    if (strcmp(resultDisplay, element.keyString->string) != 0)
    {
        setModified(_modified);
        dcStringCache_add(sShrinkCache, &element, result);
    }
    else
    {
        dcStringCache_add(sShrinkCache, &element, NULL);
    }

    dcMemory_free(resultDisplay);
    dcStringCacheElement_free(&element);
    return result;
}

typedef dcNode *(*OperationFunction)(dcNode *_node, const char *_symbol);

typedef struct
{
    const char *language;
    OperationFunction operationFunction;
} OperationLanguage;

TAFFY_HIDDEN dcNode *createIdentifier(const char *_name)
{
    return dcIdentifier_createNode(_name, NO_FLAGS);
}

TAFFY_HIDDEN dcNode *createParenthesesMethodCall(const char *_receiverName,
                                                 dcNode *_argument)
{
    return dcParser_createParenthesesOperatorFunctionCall
        (createIdentifier(_receiverName),
         (dcArray_createWithObjects(_argument, NULL)));
}

typedef bool (*NewMatchFunction)(const char *_program,
                                 dcHash *_scope,
                                 const dcListElement **_i,
                                 dcNode *_node,
                                 const char *_symbol);
typedef struct
{
    const char *symbol;
    NewMatchFunction matcher;
} NewMatchFunctionMap;

TAFFY_HIDDEN bool isNumberOrNotSymbol(dcNode *_node, const char *_symbol)
{
    return ((dcGraphData_isType(_node, NODE_IDENTIFIER)
             && ! identifierEquals(_node, _symbol))
            || dcNumberClass_isMe(_node));
}

TAFFY_HIDDEN NewMatchFunction getNewMatchFunction(const char *_program);

TAFFY_HIDDEN dcNode *parseTokens(dcHash *_scope,
                                 const dcListElement **_i,
                                 const char *_symbol);

TAFFY_HIDDEN dcNode *combineTextResultElements
    (const TextResultLanguage *_languages,
     dcFlatArithmetic *_leftArithmetic,
     dcListElement **_left,
     dcFlatArithmetic *_rightArithmetic,
     dcListElement **_right,
     bool *_modified)
{
    uint32_t i;
    bool done = false;
    dcNode *result = NULL;

    for (i = 0; ! done && _languages[i].left != NULL; i++)
    {
        dcHash *scope = dcHash_create();
        const dcListElement *leftI = _languages[i].leftTokens->head;
        const dcListElement *rightI = _languages[i].rightTokens->head;

        const char *leftToken = shiftElementToken(&leftI);
        const char *rightToken = shiftElementToken(&rightI);
        NewMatchFunction leftMatcher = getNewMatchFunction(leftToken);
        NewMatchFunction rightMatcher = getNewMatchFunction(rightToken);
        assert(leftMatcher != NULL);
        assert(rightMatcher != NULL);
        dcNode *identifier = NULL;
        dcNode *arithmeticRest = NULL;

        if (leftMatcher(leftToken,
                        scope,
                        &leftI,
                        (*_left)->object,
                        NULL)
            && rightMatcher(rightToken,
                            scope,
                            &rightI,
                            (*_right)->object,
                            NULL))
        {
            dcLog_log(FLAT_ARITHMETIC_LOG,
                      "[left: '%s' right: '%s'] matched with program "
                      "left: '%s' right: '%s'\n",
                      dcNode_display((*_left)->object),
                      dcNode_display((*_right)->object),
                      _languages[i].left,
                      _languages[i].right);

            // set a breakpoint in calculusHelper for help
            TAFFY_DEBUG(calculusHelper();
                        updateCounts(dcVoidContainer_createNode
                                     ((void *)_languages), i));

            const dcListElement *resultI = _languages[i].resultTokens->head;
            result = parseTokens(scope, &resultI, NULL);
            done = true;

            dcHash_clear(scope, DC_SHALLOW);
            dcNode_free(&(*_left)->object, DC_DEEP);
            (*_left)->object = result;
            dcList_removeElement(_rightArithmetic->values, _right, DC_DEEP);

            if (_modified != NULL)
            {
                *_modified = true;
            }
        }

        dcHash_removeValueWithStringKey(scope, "residuals", NULL, DC_DEEP);
        dcHash_free(&scope, DC_SHALLOW);
        dcNode_free(&identifier, DC_DEEP);
        dcNode_free(&arithmeticRest, DC_DEEP);
    }

    return result;
}

TAFFY_HIDDEN bool matchNewSpecificNumber(const char *_program,
                                         dcHash *_scope,
                                         const dcListElement **_i,
                                         dcNode *_node,
                                         const char *_symbol)
{
    return dcNumberClass_equalsInt32u(_node, atoi(_program + 1));
}

TAFFY_HIDDEN bool matchNewNumber(const char *_program,
                                 dcHash *_scope,
                                 const dcListElement **_i,
                                 dcNode *_node,
                                 const char *_symbol)
{
    if (dcNumberClass_isMe(_node))
    {
        dcHash_setValueWithStringKey(_scope, "N", _node);
        return true;
    }

    return false;
}

TAFFY_HIDDEN bool matchNewNumberOrIdentifier(const char *_program,
                                             dcHash *_scope,
                                             const dcListElement **_i,
                                             dcNode *_node,
                                             const char *_symbol)
{
    // match anything except the symbol
    if (isNumberOrNotSymbol(_node, _symbol))
    {
        dcHash_setValueWithStringKey(_scope, _program, _node);
        return true;
    }

    return false;
}

TAFFY_HIDDEN bool matchNewAnything(const char *_program,
                                   dcHash *_scope,
                                   const dcListElement **_i,
                                   dcNode *_node,
                                   const char *_symbol)
{
    dcHash_setValueWithStringKey(_scope, _program, _node);
    return true;
}

TAFFY_HIDDEN bool matchNewMatchingAnything(const char *_program,
                                           dcHash *_scope,
                                           const dcListElement **_i,
                                           dcNode *_node,
                                           const char *_symbol)
{
    dcNode *already = NULL;
    dcResult result = dcHash_getValueWithStringKey(_scope, "C", &already);

    if (result == TAFFY_SUCCESS)
    {
        dcTaffyOperator comparison = TAFFY_UNKNOWN_OPERATOR;
        result = dcNode_compareEqual(already, _node, &comparison);

        if (result == TAFFY_SUCCESS && comparison == TAFFY_EQUALS)
        {
            return true;
        }
    }
    else if (result == TAFFY_FAILURE)
    {
        dcHash_setValueWithStringKey(_scope, _program, _node);
        return true;
    }

    return false;
}

TAFFY_HIDDEN bool matchNewRest(const char *_program,
                               dcHash *_scope,
                               const dcListElement **_i,
                               dcNode *_node,
                               const char *_symbol)
{
    // value is already set
    return true;
}

TAFFY_HIDDEN bool matchNewSymbol(const char *_program,
                                 dcHash *_scope,
                                 const dcListElement **_i,
                                 dcNode *_node,
                                 const char *_symbol)
{
    return identifierEquals(_node, _symbol);
}

TAFFY_HIDDEN dcTaffyOperator convertCharToOperator(char _character)
{
    return (_character == '+'
            ? TAFFY_ADD
            : (_character == '-'
               ? TAFFY_SUBTRACT
               : (_character == '*'
                  ? TAFFY_MULTIPLY
                  : (_character == '/'
                     ? TAFFY_DIVIDE
                     : (_character == '^'
                        ? TAFFY_RAISE
                        : TAFFY_LAST_OPERATOR)))));
}

uint32_t flatArithmeticCount = 0;

TAFFY_HIDDEN bool matchNewFlatArithmetic(const char *_program,
                                         dcHash *_scope,
                                         const dcListElement **_i,
                                         dcNode *_node,
                                         const char *_symbol)
{
    dcTaffyOperator taffyOperator = convertCharToOperator(_program[0]);
    assert(taffyOperator != TAFFY_LAST_OPERATOR);
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    bool greaterThan = false;
    bool result = false;
    const char *token = NULL;
    size_t size = 0;

    flatArithmeticCount++;

    if (! checkFlatArithmetic(_node, taffyOperator))
    {
        goto kickout;
    }

    token = shiftElementToken(_i);
    result = true;

    if (strcmp(token, ">=") == 0)
    {
        greaterThan = true;
    }
    else
    {
        // sanity
        TAFFY_DEBUG(assert(strcmp(token, "==") == 0
                           || strcmp(token, "<=") == 0
                           || strcmp(token, "<") == 0
                           || strcmp(token, ">") == 0));
    }

    // get the size
    token = shiftElementToken(_i);
    size = atoi(token);

    if ((greaterThan && arithmetic->values->size >= size)
        || (! greaterThan && arithmetic->values->size == size))
    {
        dcListElement *that;
        token = shiftElementToken(_i);

        for (that = arithmetic->values->head;
             (token != NULL
              && (strcmp(token, ")") != 0)
              && that != NULL
              && result);
             that = that->next)
        {
            NewMatchFunction matchFunction = getNewMatchFunction(token);

            if (matchFunction == NULL)
            {
                fprintf(stderr, "can't find match function for: '%s'\n", token);
                assert(false);
            }

            result = matchFunction(token,
                                   _scope,
                                   _i,
                                   that->object,
                                   _symbol);
            token = shiftElementToken(_i);
        }
    }
    else
    {
        result = false;
    }

kickout:
    return result;
}

TAFFY_HIDDEN bool matchNewLeftParentheses(const char *_program,
                                          dcHash *_scope,
                                          const dcListElement **_i,
                                          dcNode *_node,
                                          const char *_symbol)
{
    const char *token = shiftElementToken(_i);
    NewMatchFunction matchFunction = getNewMatchFunction(token);
    assert(matchFunction == matchNewFlatArithmetic);
    bool result = matchFunction(token, _scope, _i, _node, _symbol);
    return result;
}

TAFFY_HIDDEN bool matchNewList(const char *_program,
                               dcHash *_scope,
                               const dcListElement **_i,
                               dcNode *_node,
                               const char *_symbol)
{
    assert(false);
    return false;
}

TAFFY_HIDDEN bool matchNewFlatArithmeticConversion(const char *_program,
                                                   dcHash *_scope,
                                                   const dcListElement **_i,
                                                   dcNode *_node,
                                                   const char *_symbol)
{
    dcNode *residualsNode = NULL;
    assert(dcHash_getValueWithStringKey(_scope,
                                        "residuals",
                                        &residualsNode)
           == TAFFY_SUCCESS
           && residualsNode != NULL);
    dcList *residuals = CAST_LIST((dcNode *)CAST_VOID(residualsNode));

    dcTaffyOperator taffyOperator = convertCharToOperator(_program[0]);
    assert(taffyOperator != TAFFY_LAST_OPERATOR);
    bool greaterThan = false;
    bool result = false;
    const char *token = NULL;
    dcFlatArithmetic *arithmetic = NULL;
    dcHash *newScope = NULL;
    const dcListElement *iBackup = *_i;
    size_t size = 0;

    if (! checkFlatArithmetic(_node, taffyOperator))
    {
        goto kickout;
    }

    // we muck with arithmetic
    // if this fails, don't muck with it
    arithmetic = dcFlatArithmetic_copy(CAST_FLAT_ARITHMETIC(_node), DC_DEEP);

    token = shiftElementToken(&iBackup);
    result = false;

    if (strcmp(token, ">=") == 0)
    {
        greaterThan = true;
    }
    else
    {
        // sanity
        TAFFY_DEBUG(assert(strcmp(token, "==") == 0
                           || strcmp(token, "<=") == 0
                           || strcmp(token, "<") == 0
                           || strcmp(token, ">") == 0));
    }

    // get the size
    token = shiftElementToken(&iBackup);
    size = atoi(token);
    newScope = dcHash_create();

    if ((greaterThan && arithmetic->values->size >= size)
        || (! greaterThan && arithmetic->values->size == size))
    {
        token = shiftElementToken(&iBackup);

        while (token != NULL)
        {
            NewMatchFunction matchFunction = getNewMatchFunction(token);
            assert(matchFunction != NULL);
            result = false;
            dcListElement *that;

            for (that = arithmetic->values->head;
                 that != NULL && ! result;
                )
            {
                dcListElement *next = that->next;
                const dcListElement *newBackup = iBackup;
                result = matchFunction(token,
                                       newScope,
                                       &newBackup,
                                       that->object,
                                       _symbol);
                if (result)
                {
                    // free later!
                    dcList_push(residuals, that->object);
                    dcList_removeElement(arithmetic->values,
                                         &that,
                                         DC_SHALLOW);
                }
                else if (! result && IS_FLAT_ARITHMETIC(that->object))
                {
                    dcFlatArithmetic *thatArithmetic =
                        CAST_FLAT_ARITHMETIC(that->object);

                    if (thatArithmetic->taffyOperator == TAFFY_RAISE
                        && thatArithmetic->values->size == 2
                        && (dcNumberClass_isMe
                            (thatArithmetic->values->tail->object)))
                    {
                        newBackup = iBackup;
                        result = matchFunction
                            (token,
                             newScope,
                             &newBackup,
                             thatArithmetic->values->head->object,
                             _symbol);

                        if (result)
                        {
                            dcNumberClass_inlineSubtract
                                (thatArithmetic->values->tail->object,
                                 dcNumberClass_getOneNumberObject());
                        }
                    }
                }

                if (result)
                {
                    iBackup = newBackup;
                }

                that = next;
            }

            if (! result)
            {
                break;
            }

            token = shiftElementToken(&iBackup);
        }
    }

kickout:
    if (result)
    {
        dcFlatArithmetic_free(&CAST_FLAT_ARITHMETIC(_node), DC_DEEP);
        CAST_FLAT_ARITHMETIC(_node) = arithmetic;
        dcHash_merge(_scope, newScope);
        dcHash_free(&newScope, DC_SHALLOW);
    }
    else
    {
        dcHash_free(&newScope, DC_SHALLOW);
        dcFlatArithmetic_free(&arithmetic, DC_DEEP);
        dcList_clear(residuals, DC_DEEP);
    }

    return result;
}

TAFFY_HIDDEN bool matchNewE(const char *_program,
                            dcHash *_scope,
                            const dcListElement **_i,
                            dcNode *_node,
                            const char *_symbol)
{
    return identifierEquals(_node, "e");
}

static bool isWithoutSymbol(dcNode *_node, const char *_symbol)
{
    return (! identifierEquals(_node, _symbol)
            && ! dcFlatArithmetic_findIdentifier(&_node, _symbol, NULL)
            && ! identifierEquals(_node, "i"));
}

TAFFY_HIDDEN bool matchNewFunctionWithoutSymbol(const char *_program,
                                                dcHash *_scope,
                                                const dcListElement **_i,
                                                dcNode *_node,
                                                const char *_symbol)
{
    if (isWithoutSymbol(_node, _symbol))
    {
        dcHash_setValueWithStringKey(_scope, _program, _node);
        return true;
    }

    return false;
}

TAFFY_HIDDEN bool matchNewNegativeMultiplication(const char *_program,
                                                 dcHash *_scope,
                                                 const dcListElement **_i,
                                                 dcNode *_node,
                                                 const char *_symbol)
{
    if (isNegativeMultiply(_node, false)
        || (dcNumberClass_isMe(_node) && dcNumberClass_isNegative(_node)))
    {
        dcHash_setValueWithStringKey(_scope, _program, _node);
        return true;
    }

    return false;
}

TAFFY_HIDDEN bool matchNewEvenRaise(const char *_program,
                                    dcHash *_scope,
                                    const dcListElement **_i,
                                    dcNode *_node,
                                    const char *_symbol)
{
    if (IS_RAISE(_node)
        && FLAT_ARITHMETIC_SIZE(_node) == 2
        && dcNumberClass_isMe(FLAT_ARITHMETIC_TAIL(_node))
        && dcNumberClass_isEven(FLAT_ARITHMETIC_TAIL(_node))
        && IS_IDENTIFIER(FLAT_ARITHMETIC_HEAD(_node)))
    {
        dcHash_setValueWithStringKey(_scope, "EvenRaise", _node);
        return true;
    }

    return false;
}

TAFFY_HIDDEN bool matchNewMultiFunctionCall(const char *_program,
                                            dcHash *_scope,
                                            const dcListElement **_i,
                                            dcNode *_node,
                                            const char *_symbol)
{
    if (! IS_METHOD_CALL(_node))
    {
        return false;
    }

    const dcListElement *lookAhead = *_i;
    dcMethodCall *call = CAST_METHOD_CALL(_node);

    // first token is the name
    const char *token = shiftElementToken(&lookAhead);

    if (! (IS_IDENTIFIER(call->receiver)
           && dcIdentifier_equalsString(call->receiver, token)))
    {
        return false;
    }

    // verify we get left parentheses
    token = shiftElementToken(&lookAhead);
    assert(strcmp(token, "(") == 0);

    uint32_t i;
    dcHash *backup = dcHash_copy(_scope, DC_SHALLOW);
    bool result = true;

    for (i = 0; ; i++)
    {
        dcNode **argument = getMethodArgument(_node, i, false);
        token = shiftElementToken(&lookAhead);

        if (argument == NULL)
        {
            if (token != NULL && strcmp(token, ")") != 0)
            {
                result = false;
            }

            break;
        }

        NewMatchFunction matcher = getNewMatchFunction(token);
        assert(matcher != NULL);

        if (! matcher(token, _scope, &lookAhead, *argument, _symbol))
        {
            result = false;
            break;
        }
    }

    if (result)
    {
        *_i = lookAhead;
    }
    else
    {
        dcHash_clear(_scope, DC_SHALLOW);
        dcHash_merge(_scope, backup);
    }

    dcHash_free(&backup, DC_SHALLOW);
    return result;
}

TAFFY_HIDDEN bool matchNewFunctionCall(const char *_program,
                                       dcHash *_scope,
                                       const dcListElement **_i,
                                       dcNode *_node,
                                       const char *_symbol)
{
    bool result = false;

    if (IS_METHOD_CALL(_node))
    {
        if (getMethodArgument(_node, 1, false) != NULL)
        {
            return false;
        }

        dcNode **argument = getMethodArgument(_node, 0, false);
        const dcListElement *newI = *_i;
        dcMethodCall *call = CAST_METHOD_CALL(_node);

        // first token is the name
        const char *token = shiftElementToken(&newI);

        if (IS_IDENTIFIER(call->receiver)
            && dcIdentifier_equalsString(call->receiver, token))
        {
            dcHash *newScope = dcHash_copy(_scope, DC_DEEP);

            // verify we get left parentheses
            token = shiftElementToken(&newI);
            assert(strcmp(token, "(") == 0);
            token = shiftElementToken(&newI);

            NewMatchFunction matcher = getNewMatchFunction(token);
            assert(matcher != NULL);

            bool matchResult =
                matcher(token, newScope, &newI, *argument, _symbol);

            token = shiftElementToken(&newI);
            assert(strcmp(token, ")") == 0);

            if (matchResult)
            {
                result = true;
                dcHash_merge(_scope, newScope);
                dcHash_free(&newScope, DC_SHALLOW);
                *_i = newI;
            }
            else
            {
                dcHash_free(&newScope, DC_DEEP);
            }
        }
    }

    return result;
}

TAFFY_HIDDEN bool matchNewDistributable(const char *_program,
                                        dcHash *_scope,
                                        const dcListElement **_i,
                                        dcNode *_node,
                                        const char *_symbol)
{
    bool match = false;

    if (IS_FLAT_ARITHMETIC(_node))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);

        if (arithmetic->taffyOperator == TAFFY_MULTIPLY)
        {
            FOR_EACH(arithmetic, that)
            {
                if (IS_FLAT_ARITHMETIC(that->object)
                    && ((CAST_FLAT_ARITHMETIC(that->object)->taffyOperator
                         == TAFFY_ADD)
                        || (CAST_FLAT_ARITHMETIC(that->object)->taffyOperator
                            == TAFFY_SUBTRACT)))
                {
                    match = true;
                    break;
                }
            }
        }
        else if (arithmetic->taffyOperator == TAFFY_DIVIDE
                 && arithmetic->values->size >= 2)
        {
            if (IS_FLAT_ARITHMETIC(arithmetic->values->head->object))
            {
                dcFlatArithmetic *headArithmetic =
                    CAST_FLAT_ARITHMETIC(arithmetic->values->head->object);

                if (headArithmetic->taffyOperator == TAFFY_ADD
                    || headArithmetic->taffyOperator == TAFFY_SUBTRACT)
                {
                    match = true;
                }
                else if (headArithmetic->taffyOperator == TAFFY_MULTIPLY)
                {
                    FOR_EACH(headArithmetic, that)
                    {
                        if (IS_FLAT_ARITHMETIC(that->object)
                            && ((CAST_FLAT_ARITHMETIC
                                 (that->object)->taffyOperator
                                 == TAFFY_ADD)
                                || (CAST_FLAT_ARITHMETIC
                                    (that->object)->taffyOperator
                                    == TAFFY_SUBTRACT)))
                        {
                            match = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (match)
    {
        dcHash_setValueWithStringKey(_scope,
                                     "Distributable",
                                     _node);
    }

    return match;
}

TAFFY_HIDDEN bool matchNewMultiplicationWithNotX(const char *_program,
                                                 dcHash *_scope,
                                                 const dcListElement **_i,
                                                 dcNode *_node,
                                                 const char *_symbol)
{
    bool match = false;

    if (IS_FLAT_ARITHMETIC(_node)
        && CAST_FLAT_ARITHMETIC(_node)->taffyOperator == TAFFY_MULTIPLY)
    {
        FOR_EACH_IN_NODE(_node, that)
        {
            if (isWithoutSymbol(that->object, _symbol))
            {
                dcHash_setValueWithStringKey(_scope, _program, _node);
                match = true;
                break;
            }
        }
    }

    return match;
}

TAFFY_HIDDEN dcHash *sNewMatchLookup = NULL;

TAFFY_HIDDEN const NewMatchFunctionMap sNewMatchFunctionMap[] =
{
    {"s",     &matchNewSymbol},
    {"%0",    &matchNewSpecificNumber},
    {"%1",    &matchNewSpecificNumber},
    {"%-1",   &matchNewSpecificNumber},
    {"%2",    &matchNewSpecificNumber},
    {"%3",    &matchNewSpecificNumber},
    {"N",     &matchNewNumber},
    {"e",     &matchNewE},
    {"NotX",  &matchNewFunctionWithoutSymbol},
    {"NotX2", &matchNewFunctionWithoutSymbol},

    {"F",                      &matchNewFunctionCall},
    {"FM",                     &matchNewMultiFunctionCall},
    {"NegativeMultiplication", &matchNewNegativeMultiplication},
    {"EvenRaise",              &matchNewEvenRaise},

    {"a", &matchNewNumberOrIdentifier},
    {"b", &matchNewNumberOrIdentifier},

    {"A",  &matchNewAnything},
    {"B",  &matchNewAnything},
    {"C",  &matchNewMatchingAnything},
    {"CC", &matchNewMatchingAnything},
    {"R",  &matchNewRest},

    // flat arithmetics
    {"*C", &matchNewFlatArithmeticConversion},
    {"+C", &matchNewFlatArithmeticConversion},
    {"+",  &matchNewFlatArithmetic},
    {"-",  &matchNewFlatArithmetic},
    {"*",  &matchNewFlatArithmetic},
    {"/",  &matchNewFlatArithmetic},
    {"^",  &matchNewFlatArithmetic},

    // new arithmetic
    {"(", &matchNewLeftParentheses},

    {",", &matchNewList},

    {"MultiplicationWithNotX", &matchNewMultiplicationWithNotX},
    {"Distributable",          &matchNewDistributable},
};

typedef struct
{
    const char *programText;
    dcList *programTokens;
    const char *result;
    dcList *resultTokens;
} NewOperationLanguage;

#define ELEMENT(_programText, _result)          \
    {_programText, NULL, _result, NULL}

TAFFY_HIDDEN NewOperationLanguage sDeriveLanguages[] =
{
    // x ==> 1
    ELEMENT("s", "1"),

    // non-x ==> 0
    ELEMENT("NotX", "0"),

    // e^anything
    ELEMENT("^ == 2 e A", "* D ( A ) :)"),

    // x^x
    ELEMENT("^ == 2 s s", "* ( ^ s s ) ( + 1 F ln ( s ) )"),

    // add rule
    ELEMENT("+ >= 2 A R", "+ D ( A ) D ( R )"),

    // subtract rule
    ELEMENT("- >= 2 A R", "- D ( A ) D ( R )"),

    // product rule
    ELEMENT("* >= 2 A R", "+ ( * D ( A ) R ) ( * A D ( R ) )"),

    // reciprocal rule
    ELEMENT("/ == 2 a s", "/ ( * -1 a ) ( ^ s 2 )"),

    // quotient rule
    ELEMENT("/ >= 2 A R",
            "/ ( - ( * D ( A ) R ) ( * A D ( R ) ) ) ( ^ R 2 )"),

    // easy power rule
    ELEMENT("^ == 2 A a", "* a D ( A ) ( ^ A ( - a 1 ) )"),

    // generalized power rule
    ELEMENT("^ >= 2 A R",
            ("* ( ^ A R ) ( + ( * D ( A ) "
             "( / R A ) ) ( * D ( R ) F ln ( A ) ) )")),

    // logarithms
    ELEMENT("FM ln ( A )",    "/ D ( A ) A"),
    // handled already
    //{"log(a,A)", "/ D ( A ) ( * A F ln ( a ) )"),

    // trigonometric functions
    ELEMENT("FM sin ( A )", "* D ( A ) F cos ( A )"),
    ELEMENT("FM cos ( A )", "* -1 D ( A ) F sin ( A )"),
    ELEMENT("FM tan ( A )", "* D ( A ) ( ^ F sec ( A ) 2 )"),
    ELEMENT("FM csc ( A )", "* -1 D ( A ) F csc ( A ) F cot ( A )"),
    ELEMENT("FM cot ( A )", "* -1 D ( A ) ( ^ F csc ( A ) 2 )"),
    ELEMENT("FM sec ( A )", "* D ( A ) F sec ( A ) F tan ( A )"),

    // inverse trigonometric functions
    ELEMENT("FM asin ( A )", "/ D ( A ) ( ^ ( - 1 ( ^ A 2 ) ) 0.5 )"),
    ELEMENT("FM acos ( A )",
            "/ ( * -1 D ( A ) ) ( ^ ( - 1 ( ^ A 2 ) ) 0.5 )"),
    ELEMENT("FM atan ( A )", "/ D ( A ) ( + 1 ( ^ A 2 ) )"),
    ELEMENT("FM acsc ( A )",
            ("/ ( * -1 D ( A ) ) "
             "  ( * ( ^ ( - 1 "
             "              ( / 1 ( ^ A 2 ) ) ) "
             "        0.5 )"
             "      ( ^ A 2 ) )")),
    ELEMENT("FM asec ( A )",
            ("/ D ( A ) ( * ( ^ ( - 1 "
             "( / 1 ( ^ A 2 ) ) ) 0.5 ) ( ^ A 2 ) )")),
    ELEMENT("FM acot ( A )", "/ ( * -1 D ( A ) ) ( + 1 ( ^ A 2 ) )"),
    // hyperbolic functions
    ELEMENT("FM sinh ( A )", "* D ( A ) F cosh ( A )"),
    ELEMENT("FM cosh ( A )", "* D ( A ) F sinh ( A )"),
    ELEMENT("FM tanh ( A )", "* D ( A ) ( ^ F sech ( A ) 2 )"),
    ELEMENT("FM csch ( A )", "* -1 D ( A ) F coth ( A ) F csch ( A )"),
    ELEMENT("FM sech ( A )", "* -1 D ( A ) F tanh ( A ) F sech ( A )"),
    ELEMENT("FM coth ( A )", "* D ( A ) ( - 1 ( ^ F coth ( A ) 2 ) )"),

    // inverse hyperbolic functions
    ELEMENT("FM asinh ( A )",
            "* D ( A ) ( / 1 ( ^ ( + ( ^ A 2 ) 1 ) 0.5 ) )"),
    ELEMENT("FM acosh ( A )",
            ("* D ( A ) ( / 1 ( * ( ^ ( - A 1 ) 0.5 ) "
             "( ^ ( + A 1 ) 0.5 ) ) )")),
    ELEMENT("FM atanh ( A )",
            "* D ( A ) ( / 1 ( - 1 ( ^ A 2 ) ) )"),
    ELEMENT("FM acsch ( A )",
            ("* D ( A ) ( / -1 ( * ( ^ ( + ( / 1 "
             "( ^ A 2 ) ) 1 ) 0.5 ) ( ^ A 2 ) ) )")),

    ELEMENT("FM asech ( A )",
            ("* D ( A ) ( / ( ^ ( / ( - 1  A ) "
             "( + A 1 ) ) 0.5 ) ( * ( - A 1 ) A ) )")),
    ELEMENT("FM acoth ( A )", "* D ( A ) ( / 1 ( - 1 ( ^ A 2 ) ) )"),

    // absolute value
    ELEMENT("FM abs ( A )", "/ ( * A D ( A ) ) F abs ( A )"),

    ELEMENT("FM sqrt ( A )", "* ( ^ A ( / 3 2 ) ) ( / 2 3 )"),

    {NULL}
};

TAFFY_HIDDEN NewOperationLanguage sConvertLanguages[] =
{
    ELEMENT("*C >= 3 FM sin ( C ) FM cos ( C ) FM tan ( C )",
            "* ( ^ F sin ( C ) 2 ) :)"),
    {NULL}
};

TAFFY_HIDDEN NewOperationLanguage sTrigonometricSimplificationLanguages[] =
{
    ELEMENT("FM ln ( ^ == 2 e A )", "A"),

    ELEMENT("F abs ( EvenRaise )", "EvenRaise"),

    ELEMENT("FM log ( A B )", "/ F ln ( B ) F ln ( A )"),

    ELEMENT("F sin ( %0 )", "0"), // 0

    //
    // turn that frown upside down! or keep it, whatevs
    //
    ELEMENT("F sin ( NegativeMultiplication )",
            "* -1 F sin ( * -1 NegativeMultiplication )"),
    ELEMENT("F cos ( NegativeMultiplication )",
            "F cos ( * -1 NegativeMultiplication )"),
    ELEMENT("F tan ( NegativeMultiplication )",
            "* -1 F tan ( * -1 NegativeMultiplication )"),
    ELEMENT("F csc ( NegativeMultiplication )",
            "* -1 F csc ( * -1 NegativeMultiplication )"),
    ELEMENT("F sec ( NegativeMultiplication )",
            "F sec ( * -1 NegativeMultiplication )"),
    ELEMENT("F cot ( NegativeMultiplication )",
            "* -1 F cot ( * -1 NegativeMultiplication )"), // 6

    // hyperbolic
    ELEMENT("F sinh ( NegativeMultiplication )",
            "* -1 F sinh ( * -1 NegativeMultiplication )"),
    ELEMENT("F cosh ( NegativeMultiplication )",
            "F cosh ( * -1 NegativeMultiplication )"),
    ELEMENT("F tanh ( NegativeMultiplication )",
            "* -1 F tanh ( * -1 NegativeMultiplication )"),
    ELEMENT("F csch ( NegativeMultiplication )",
            "* -1 F csch ( * -1 NegativeMultiplication )"), // 10
    ELEMENT("F sech ( NegativeMultiplication )",
            "F sech ( * -1 NegativeMultiplication )"),
    ELEMENT("F coth ( NegativeMultiplication )",
            "* -1 F coth ( * -1 NegativeMultiplication )"),

    // a
    ELEMENT("F asin ( NegativeMultiplication )",
            "* -1 F asin ( * -1 NegativeMultiplication )"), // 13
    ELEMENT("F atan ( NegativeMultiplication )",
            "* -1 F atan ( * -1 NegativeMultiplication )"),
    ELEMENT("F acsc ( NegativeMultiplication )",
            "* -1 F acsc ( * -1 NegativeMultiplication )"),
    ELEMENT("F acot ( NegativeMultiplication )",
            "* -1 F acot ( * -1 NegativeMultiplication )"),

    // a hyperbolic
    ELEMENT("F asinh ( NegativeMultiplication )",
            "* -1 F asinh ( * -1 NegativeMultiplication )"),
    ELEMENT("F atanh ( NegativeMultiplication )",
            "* -1 F atanh ( * -1 NegativeMultiplication )"),
    ELEMENT("F acsch ( NegativeMultiplication )",
            "* -1 F acsch ( * -1 NegativeMultiplication )"),
    ELEMENT("F acoth ( NegativeMultiplication )",
            "* -1 F acoth ( * -1 NegativeMultiplication )"),

    //
    // convert ? / sin(x) into ? * csc(x), etc
    //
    ELEMENT("/ == 2 CC F csc ( C )", "* CC F sin ( C )"),
    ELEMENT("/ == 2 CC F sec ( C )", "* CC F cos ( C )"),
    ELEMENT("/ == 2 CC F cot ( C )", "* CC F tan ( C )"),
    ELEMENT("/ == 2 CC F sin ( C )", "* CC F csc ( C )"),
    ELEMENT("/ == 2 CC F cos ( C )", "* CC F sec ( C )"),
    ELEMENT("/ == 2 CC F tan ( C )", "* CC F cot ( C )"),

    // (e^x - e^(-x)) / 2 => sinh(x)
    ELEMENT(("/ == 2 ( - == 2 ( ^ == 2 e C ) "
             "( ^ == 2 e ( * == 2 %-1 C ) ) ) N"),
            "* ( / 2 N ) F sinh ( C )"),

    // 2 / (e^x - e^(-x)) => csch(x)
    ELEMENT(("/ == 2 N ( - == 2 ( ^ == 2 e C ) "
             "( ^ == 2 e ( * == 2 %-1 C ) ) )"),
            "* ( / N 2 ) F csch ( C )"),

    // 1 / sinh(x) => csch(x)
    ELEMENT("/ == 2 CC F sinh ( C )", "* CC F csch ( C )"),

    // (e^x + e^(-x)) / 2 => cosh(x)
    ELEMENT(("/ == 2 ( + == 2 ( ^ == 2 e C ) "
             "( ^ == 2 e ( * == 2 %-1 C ) ) ) N"),
            "* ( / 2 N ) F cosh ( C )"),

    // 2 / (e^x + e^(-x)) => sech(x)
    ELEMENT(("/ == 2 N ( + == 2 ( ^ == 2 e C ) "
             "( ^ == 2 e ( * == 2 %-1 C ) ) )"),
            "* ( / N 2 ) F sech ( C )"),

    // 1 / cosh(x) => sech(x)
    ELEMENT("/ == 2 CC FM cosh ( C )", "* CC F sech ( C )"),

    // sinh(x) / cosh(x) => tanh(x)
    ELEMENT("/ == 2 FM sinh ( C ) FM cosh ( C )", "F tanh ( C )"),

    // 1 / tanh(x) => coth(x)
    ELEMENT("/ == 2 CC FM tanh ( C )", "* CC F coth ( C )"),

    {NULL}
};

TAFFY_HIDDEN NewOperationLanguage sIntegrateSimplificationLanguages[] =
{
    ELEMENT("^ == 2 FM sin ( A ) %2",
            "* 0.5 ( - 1 F cos ( * 2 A ) )"),
    ELEMENT("^ == 2 FM cos ( A ) %2",
            "* 0.5 ( + 1 F cos ( * 2 A ) )"),
    ELEMENT("* == 2 FM sin ( A ) FM cos ( B )",
            "* 0.5 ( + F sin ( - A B ) F sin ( + A B ) )"),
    {NULL}
};

TAFFY_HIDDEN NewOperationLanguage sIntegrateLanguages[] =
{
    ELEMENT("/ == 2 a s",          "* a F ln ( F abs ( s ) )"),
    ELEMENT("^ == 2 s NotX",
            "/ ( ^ s ( + NotX 1 ) ) ( + NotX 1 )"),
    ELEMENT("NotX",                "* NotX s"),
    ELEMENT("s",                   "* 0.5 ( ^ s 2 )"),

    ELEMENT("FM sqrt ( s )",
            "* ( / 2 3 ) ( ^ s ( / 3 2 ) )"),

    ELEMENT("+ >= 2",           "SumIntegral ( :) )"),
    ELEMENT("- >= 2",           "SumIntegral ( :) )"),

    // trigonometric functions
    ELEMENT("FM sin ( s )", "* -1 F cos ( s )"),
    ELEMENT("FM cos ( s )", "F sin ( s )"),
    ELEMENT("FM tan ( s )",
            "* -1 F ln ( F abs ( F sec ( s ) ) )"),
    ELEMENT("FM sec ( s )",
            "- F ln ( + F sin ( / s 2 ) F cos ( / s 2 ) ) "
            "F ln ( - F cos ( / s 2 ) F sin ( / s 2 ) )"),
    ELEMENT("FM csc ( s )",
            "- F ln ( F sin ( / s 2 ) ) "
            "F ln ( F cos ( / s 2 ) )"),
    ELEMENT("FM cot ( s )",
            "F ln ( F sin ( s ) )"),
    ELEMENT("^ == 2 FM sin ( s ) %2",
            "* 0.5 ( - s ( * F sin ( s ) F cos ( s ) ) )"),
    ELEMENT("^ == 2 FM cos ( s ) %2",
            "* 0.5 ( + s ( * F sin ( s ) F cos ( s ) ) )"),
    ELEMENT("^ == 2 FM sec ( s ) %2",     "F tan ( s )"),
    ELEMENT("* == 2 FM sec ( s ) FM tan ( s )", "F sec ( s )"),
    ELEMENT("* == 2 FM csc ( s ) FM cot ( s )", "* -1 F csc ( s )"),
    ELEMENT("^ == 2 FM sec ( s ) %3",
            "+ ( / ( * F sec ( s ) F tan ( s ) ) 2 ) "
            "  ( / ( * F ln ( + F sec ( s ) F tan ( s ) ) ) 2 )"),

    // inverse trigonometric functions
    ELEMENT("FM asin ( s )",
            "+ ( * s F asin ( s ) ) ( ^ ( - 1 ( ^ s 2 ) ) 0.5 )"),

    ELEMENT("FM acos ( s )",
            "- ( * s F acos ( s ) ) ( ^ ( - 1 ( ^ s 2 ) ) 0.5 )"),

    ELEMENT("FM atan ( s )",
            ("- ( * s F atan ( s ) ) "
             "( * 0.5 F ln ( F abs ( + 1 ( ^ s 2 ) ) ) )")),

    // hyperbolic trigonometric functions
    ELEMENT("FM sinh ( s )", "F cosh ( s )"),
    ELEMENT("FM cosh ( s )", "F sinh ( s )"),
    ELEMENT("FM tanh ( s )", "F ln ( F cosh ( s ) )"),
    ELEMENT("FM coth ( s )", "F ln ( F sinh ( s ) )"),
    ELEMENT("FM sech ( s )", "* 2 F atan ( F tanh ( / s 2 ) )"),
    ELEMENT("FM csch ( s )", "F ln ( F tanh ( / s 2 ) )"),

    // inverse hyperbolic trigonometric functions
    ELEMENT("FM asinh ( s )",
            "- ( * s F asinh ( s ) ) F sqrt ( + ( ^ s 2 ) 1 )"),

    ELEMENT("FM acosh ( s )",
            "- ( * s F acosh ( s ) ) ( * F sqrt ( - s 1 ) "
            "F sqrt ( + s 1 ) )"),

    ELEMENT("FM atanh ( s )",
            "+ ( / F ln ( - 1 ( ^ s 2 ) ) 2 ) ( * s F atanh ( s ) )"),

    ELEMENT("FM acoth ( s )",
            "+ ( / F ln ( - 1 ( ^ s 2 ) ) 2 ) ( * s F acoth ( s ) )"),

    ELEMENT("FM asech ( s )",
            "- ( * s F asech ( s ) ) "
            "  ( / ( * 2 "
            "          F sqrt ( / ( - 1 s ) ( + s 1 ) ) "
            "          F sqrt ( - 1 ( ^ s 2 ) ) "
            "          F asin ( / F sqrt ( + s 1 ) F sqrt ( 2 ) ) ) "
            "      ( - s 1 ) )"),

    ELEMENT("FM acsch ( s )",
            "* s "
            "  ( + ( / ( * F sqrt ( + ( / 1 ( ^ s 2 ) ) 1 ) "
            "            F asinh ( s ) ) "
            "        F sqrt ( + ( ^ s 2 ) 1 ) ) "
            "      F acsch ( s ) )"),

    ELEMENT("FM ln ( s )",
            "- ( * s F ln ( s ) ) s"),

    ELEMENT("^ == 2 FM ln ( s ) %2",
            "* s "
            "  ( + ( - ( ^ F ln ( s ) 2 ) "
            "  ( * 2 s F ln ( s ) ) ) 2 )"),

    // abs
    ELEMENT("FM abs ( A )", "* Integral ( A ) F sgn ( A )"),

    // exponential functions
    ELEMENT("^ == 2 e s", ":)"), // e^x

    ELEMENT("^ == 2 a s",    "/ ( ^ a s ) F ln ( a )"),
    ELEMENT("^ == 2 NotX s", "/ ( ^ NotX s ) F ln ( NotX )"),

    // integral(n * something) = n * integral(something)
    ELEMENT("MultiplicationWithNotX",
            "IntegrateMultiplicationWithNotX"),

    // integral((not-x * x) / y) = not-x * integral(x / y)
    ELEMENT("/ == 2 MultiplicationWithNotX C",
            "* MultiplicationWithNotXValue "
            "Integral ( / MultiplicationWithX C )"),

    // integral(something / X) = 1/X * integral(something)
    ELEMENT("/ == 2 A NotX", "* ( / 1 NotX ) Integral ( A )"),

    // TODO: integral(X / something) = X * integral(1 / something)

    // 1 / x^2
    ELEMENT("/ == 2 NotX ( ^ == 2 s NotX2 )",
            "/ NotX ( * -1 ( + NotX2 1 ) ( ^ s ( + NotX2 1 ) ) )"),

    // reduction formulae
    ELEMENT("^ == 2 FM sin ( s ) N",
            "+ ( * -1 ( / 1 N ) ( ^ F sin ( s ) ( - N 1 ) ) F cos ( s ) ) "
            "  ( * ( / ( - N 1 ) N ) Integral "
            "( ^ F sin ( s ) ( - N 2 ) ) )"),

    ELEMENT("^ == 2 FM cos ( s ) N",
            "+ ( * ( / 1 N ) ( ^ F cos ( s ) ( - N 1 ) ) F sin ( s ) ) "
            "  ( * ( / ( - N 1 ) N ) Integral "
            "( ^ F cos ( s ) ( - N 2 ) ) )"),

    ELEMENT("Distributable",
            "SumIntegral ( Distribute ( Distributable ) )"),

    {NULL}
};

TAFFY_HIDDEN NewMatchFunction getNewMatchFunction(const char *_program)
{
    NewMatchFunction result = NULL;
    dcNode *resultNode = NULL;

    if (dcHash_getValueWithStringKey(sNewMatchLookup,
                                     _program,
                                     &resultNode)
        == TAFFY_SUCCESS)
    {
        result = (NewMatchFunction)CAST_VOID(resultNode);
    }

    return result;
}

TAFFY_HIDDEN bool createArithmetic(dcNode **_result, dcTaffyOperator _operator)
{
    *_result = dcFlatArithmetic_createNode(_operator);
    return true;
}

TAFFY_HIDDEN bool parseFlatArithmetic(const char *_token,
                                      dcNode **_result,
                                      dcHash *_scope,
                                      const dcListElement **_i,
                                      const char *_symbol)
{
    dcTaffyOperator taffyOperator = convertCharToOperator(_token[0]);
    assert(taffyOperator != TAFFY_LAST_OPERATOR);
    return createArithmetic(_result, taffyOperator);
}

TAFFY_HIDDEN bool parseLeftParentheses(const char *_token,
                                       dcNode **_result,
                                       dcHash *_scope,
                                       const dcListElement **_i,
                                       const char *_symbol)
{
    assert(*_result != NULL);
    dcNode *guts = parseTokens(_scope, _i, _symbol);

    if (guts == NULL)
    {
        dcNode_free(_result, DC_DEEP);
        return false;
    }
    else
    {
        dcList_push(CAST_FLAT_ARITHMETIC(*_result)->values, guts);
    }

    return true;
}

TAFFY_HIDDEN bool parseRightParentheses(const char *_token,
                                        dcNode **_result,
                                        dcHash *_scope,
                                        const dcListElement **_i,
                                        const char *_symbol)
{
    return false;
}

TAFFY_HIDDEN bool insertParsedValue(dcNode **_result, dcNode *_value)
{
    if (*_result == NULL)
    {
        *_result = _value;
    }
    else if (IS_FLAT_ARITHMETIC(*_result))
    {
        dcList_push(CAST_FLAT_ARITHMETIC(*_result)->values,
                    _value);
    }
    else if ((*_result)->type == NODE_LIST)
    {
        dcList_push(CAST_LIST(*_result), _value);
    }

    return true;
}

TAFFY_HIDDEN bool parseNumber(const char *_token,
                              dcNode **_result,
                              dcHash *_scope,
                              const dcListElement **_i,
                              const char *_symbol)
{
    int number = atoi(_token);
    return insertParsedValue(_result, createNumber(number));
}

TAFFY_HIDDEN bool parseComma(const char *_token,
                             dcNode **_result,
                             dcHash *_scope,
                             const dcListElement **_i,
                             const char *_symbol)
{
    assert(*_result == NULL);
    *_result = dcList_createNode();
    return true;
}

TAFFY_HIDDEN bool parseRealNumber(const char *_token,
                                  dcNode **_result,
                                  dcHash *_scope,
                                  const dcListElement **_i,
                                  const char *_symbol)
{
    double number = atof(_token);
    return insertParsedValue(_result, createRealNumber(number));
}

TAFFY_HIDDEN void assertLeftParentheses(const dcListElement **_i)
{
    const char *leftParentheses = shiftElementToken(_i);
    assert(strcmp(leftParentheses, "(") == 0);
}

TAFFY_HIDDEN const char *getFunctionName(const dcListElement **_i)
{
    const char *functionName = shiftElementToken(_i);
    assertLeftParentheses(_i);
    return functionName;
}

// TODO: remove parseFunction and rename parseBestFunction to parseFunction
TAFFY_HIDDEN bool parseBestFunction(const char *_token,
                                    dcNode **_result,
                                    dcHash *_scope,
                                    const dcListElement **_i,
                                    const char *_symbol)
{
    const char *functionName = getFunctionName(_i);
    dcNode *methodCall =
        createParenthesesMethodCall
        (functionName, parseTokens(_scope, _i, _symbol));
    return insertParsedValue(_result, methodCall);
}

TAFFY_HIDDEN bool parseDerivative(const char *_token,
                                  dcNode **_result,
                                  dcHash *_scope,
                                  const dcListElement **_i,
                                  const char *_symbol)
{
    assertLeftParentheses(_i);
    dcNode *input = parseTokens(_scope, _i, _symbol);
    char *logStatement = NULL;

    if (dcLog_isEnabled(FLAT_ARITHMETIC_DERIVATION_LOG))
    {
        logStatement = dcLexer_sprintf
            ("derivative of %s: ", dcNode_display(input));
    }

    if (input == NULL)
    {
        // exception!
        dcNode_free(_result, DC_DEEP);
        return false;
    }

    dcNode *derived = dcFlatArithmetic_derive(input, _symbol);
    dcNode_free(&input, DC_DEEP);

    if (dcLog_isEnabled(FLAT_ARITHMETIC_DERIVATION_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_DERIVATION_LOG,
                  "%s%s\n",
                  logStatement,
                  dcNode_display(derived));
        dcMemory_free(logStatement);
    }

    return insertParsedValue(_result, derived);
}

TAFFY_HIDDEN bool parseIntegral(const char *_token,
                                dcNode **_result,
                                dcHash *_scope,
                                const dcListElement **_i,
                                const char *_symbol)
{
    assertLeftParentheses(_i);
    dcNode *input = parseTokens(_scope, _i, _symbol);

    if (input == NULL)
    {
        // exception!
        dcNode_free(_result, DC_DEEP);
        return false;
    }

    dcNode *integral = dcFlatArithmetic_integrate(input, _symbol);
    dcNode_free(&input, DC_DEEP);

    if (integral == NULL)
    {
        dcNode_free(_result, DC_DEEP);
        return false;
    }

    return insertParsedValue(_result, integral);
}

TAFFY_HIDDEN bool parseSumIntegral(const char *_token,
                                   dcNode **_result,
                                   dcHash *_scope,
                                   const dcListElement **_i,
                                   const char *_symbol)
{
    assertLeftParentheses(_i);
    dcNode *input = parseTokens(_scope, _i, _symbol);

    if (input == NULL)
    {
        // exception!
        dcNode_free(_result, DC_DEEP);
        return false;
    }

    assert(IS_FLAT_ARITHMETIC(input));
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(input);
    assert(arithmetic->taffyOperator == TAFFY_ADD
           || arithmetic->taffyOperator == TAFFY_SUBTRACT);

    FOR_EACH(arithmetic, that)
    {
        dcNode *integral = dcFlatArithmetic_integrate(that->object, _symbol);

        if (integral != NULL)
        {
            dcNode_free(&that->object, DC_DEEP);
            that->object = integral;
        }
        else
        {
            dcNode_free(&input, DC_DEEP);
            return false;
        }
    }

    return insertParsedValue(_result, input);
}

TAFFY_HIDDEN bool parseIntegrateMultiplicationWithNotX
    (const char *_token,
     dcNode **_result,
     dcHash *_scope,
     const dcListElement **_i,
     const char *_symbol)
{
    dcNode *value;
    assert(dcHash_getValueWithStringKey(_scope, ":)", &value)
           == TAFFY_SUCCESS
           && IS_FLAT_ARITHMETIC(value));

    // TODO: don't do this copy
    dcNode *copy = dcNode_copy(value, DC_DEEP);
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(copy);

    // find the X values
    dcListElement *that;
    dcList *thoseWithout = dcList_create();

    for (that = arithmetic->values->head; that != NULL; )
    {
        dcListElement *next = that->next;

        if (isWithoutSymbol(that->object, _symbol))
        {
            dcList_push(thoseWithout, that->object);
            dcList_removeElement(arithmetic->values, &that, DC_SHALLOW);
        }

        that = next;
    }

    dcNode *left = (dcFlatArithmetic_createNodeWithList
                    (TAFFY_MULTIPLY,
                     thoseWithout));

    dcNode *integral = dcFlatArithmetic_integrate(copy, _symbol);
    dcNode *result = NULL;

    if (integral != NULL)
    {
        result = CREATE_MULTIPLY(left, integral, NULL);
    }
    else
    {
        dcNode_free(&left, DC_DEEP);
    }

    dcNode_free(&copy, DC_DEEP);
    return (result == NULL
            ? false
            : insertParsedValue(_result, result));
}

TAFFY_HIDDEN bool parseMultiplicationWithNotXValue(const char *_token,
                                                   dcNode **_result,
                                                   dcHash *_scope,
                                                   const dcListElement **_i,
                                                   const char *_symbol)
{
    dcNode *value;
    assert(dcHash_getValueWithStringKey(_scope,
                                        "MultiplicationWithNotX",
                                        &value)
           == TAFFY_SUCCESS
           && IS_FLAT_ARITHMETIC(value));

    // TODO: don't do this copy
    dcNode *copy = dcNode_copy(value, DC_DEEP);
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(copy);

    // find the X values
    dcListElement *that;
    dcList *thoseWith = dcList_create();

    for (that = arithmetic->values->head; that != NULL; )
    {
        dcListElement *next = that->next;

        if (isWithoutSymbol(that->object, _symbol))
        {
            dcList_push(thoseWith, that->object);
            dcList_removeElement(arithmetic->values, &that, DC_SHALLOW);
        }

        that = next;
    }

    dcNode_free(&copy, DC_DEEP);
    return (insertParsedValue(_result,
                              (dcFlatArithmetic_createNodeWithList
                               (TAFFY_MULTIPLY,
                                thoseWith))));
}

TAFFY_HIDDEN bool parseMultiplicationWithX(const char *_token,
                                           dcNode **_result,
                                           dcHash *_scope,
                                           const dcListElement **_i,
                                           const char *_symbol)
{
    dcNode *value;
    assert(dcHash_getValueWithStringKey(_scope,
                                        "MultiplicationWithNotX",
                                        &value)
           == TAFFY_SUCCESS
           && IS_FLAT_ARITHMETIC(value));

    // TODO: don't do this copy
    dcNode *copy = dcNode_copy(value, DC_DEEP);
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(copy);

    // find the X values
    dcListElement *that;
    dcList *thoseWithout = dcList_create();

    for (that = arithmetic->values->head; that != NULL; )
    {
        dcListElement *next = that->next;

        if (identifierEquals(that->object, _symbol)
            || dcFlatArithmetic_findIdentifier(&that->object,
                                               _symbol,
                                               NULL))
        {
            dcList_push(thoseWithout, that->object);
            dcList_removeElement(arithmetic->values, &that, DC_SHALLOW);
        }

        that = next;
    }

    dcNode_free(&copy, DC_DEEP);
    return (insertParsedValue(_result,
                              (dcFlatArithmetic_createNodeWithList
                               (TAFFY_MULTIPLY,
                                thoseWithout))));
}

static uint8_t getIntegrateDepth(void);

TAFFY_HIDDEN bool parseDistribute(const char *_token,
                                  dcNode **_result,
                                  dcHash *_scope,
                                  const dcListElement **_i,
                                  const char *_symbol)
{
    assertLeftParentheses(_i);
    dcNode *input = parseTokens(_scope, _i, _symbol);

    if (input == NULL)
    {
        // exception!
        dcNode_free(_result, DC_DEEP);
        return false;
    }

    char *preInput = NULL;

    if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
    {
        preInput = dcNode_display(input);
    }

    input = dcFlatArithmetic_distribute(input, NULL);
    input = dcFlatArithmetic_distributeDivide(input, NULL);

    dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
              "%s[%u] distributed %s to %s\n",
              indent(),
              getIntegrateDepth(),
              preInput,
              dcNode_display(input));

    return insertParsedValue(_result, input);
}

//
// a G function is a function with a comma-separated list of arguments, like:
// log(a, x)
//
TAFFY_HIDDEN bool parseGFunction(const char *_token,
                                 dcNode **_result,
                                 dcHash *_scope,
                                 const dcListElement **_i,
                                 const char *_symbol)
{
    const char *functionName = getFunctionName(_i);
    dcNode *listNode = parseTokens(_scope, _i, _symbol);
    dcNode *methodCall =
        dcParser_createParenthesesOperatorFunctionCall
        (createIdentifier(functionName),
         dcArray_createFromList(CAST_LIST(listNode), DC_SHALLOW));
    dcNode_free(&listNode, DC_SHALLOW);
    return insertParsedValue(_result, methodCall);
}

// help the debugger
TAFFY_HIDDEN dcNode *getScopedValue(dcHash *_scope, const char *_key)
{
    dcNode *result = NULL;

    if (dcHash_getValueWithStringKey(_scope, _key, &result)
        != TAFFY_SUCCESS)
    {
        fprintf(stderr, "Can't find scope value for: '%s'\n", _key);
        assert(false);
    }

    return dcNode_copy(result, DC_DEEP);
}

typedef bool (*ParseFunction)(const char *_token,
                              dcNode **_result,
                              dcHash *_scope,
                              const dcListElement **_i,
                              const char *_symbol);

typedef struct
{
    const char *token;
    ParseFunction function;
} OperationParseMap;

TAFFY_HIDDEN const OperationParseMap sOperationParseMap[] =
{
    {"+",           &parseFlatArithmetic},
    {"-",           &parseFlatArithmetic},
    {"*",           &parseFlatArithmetic},
    {"/",           &parseFlatArithmetic},
    {"^",           &parseFlatArithmetic},
    {"(",           &parseLeftParentheses},
    {")",           &parseRightParentheses},
    {"F",           &parseBestFunction},
    {"G",           &parseGFunction},
    {"D",           &parseDerivative},
    {"Integral",    &parseIntegral},
    {"SumIntegral", &parseSumIntegral},
    {"Distribute",  &parseDistribute},
    {",",           &parseComma},
    {"-1",          &parseNumber},
    {"-2",          &parseNumber},
    {"0",           &parseNumber},
    {"1",           &parseNumber},
    {"2",           &parseNumber},
    {"3",           &parseNumber},
    {"0.5",         &parseRealNumber},
    {"IntegrateMultiplicationWithNotX",
     &parseIntegrateMultiplicationWithNotX},
    {"MultiplicationWithNotXValue",
     &parseMultiplicationWithNotXValue},
    {"MultiplicationWithX",
     &parseMultiplicationWithX},
};

TAFFY_HIDDEN dcNode *parseTokens(dcHash *_scope,
                                 const dcListElement **_i,
                                 const char *_symbol)
{
    const char *token;
    dcNode *result = NULL;
    bool again = true;

    for (token = shiftElementToken(_i);
         token != NULL && again;
         )
    {
        if (strlen(token) == 0)
        {
            token = shiftElementToken(_i);
            continue;
        }

        uint8_t i;
        bool found = false;

        for (i = 0; i < dcTaffy_countOf(sOperationParseMap); i++)
        {
            if (strcmp(sOperationParseMap[i].token, token) == 0)
            {
                found = true;
                again = sOperationParseMap[i].function
                    (token, &result, _scope, _i, _symbol);

                if (result == NULL)
                {
                    // exception
                    return NULL;
                }

                break;
            }
        }

        if (! found)
        {
            dcNode *value = getScopedValue(_scope, token);

            if (result != NULL)
            {
                // yeah baby!

                if (IS_FLAT_ARITHMETIC(result))
                {
                    dcList_push(CAST_FLAT_ARITHMETIC(result)->values, value);
                }
                else if (result->type == NODE_LIST)
                {
                    dcList_push(CAST_LIST(result), value);
                }
                else
                {
                    assert(false);
                }

                dcLog_log(FLAT_ARITHMETIC_PARSE_LOG,
                          "parsed tokens result: %s\n",
                          dcNode_display(result));
            }
            else
            {
                // a standalone result, no flat arithmetic
                result = value;
            }
        }

        if (again)
        {
            token = shiftElementToken(_i);
        }
    }

    dcLog_log(FLAT_ARITHMETIC_PARSE_LOG,
              "parsed tokens output: %s\n",
              dcNode_display(result));
    return result;
}

TAFFY_HIDDEN dcNode *calculusOperation(dcNode *_node,
                                       const char *_symbol,
                                       const NewOperationLanguage *_languages,
                                       bool *_modified)
{
    uint32_t i;

    //if (dcLog_isEnabled(FLAT_ARITHMETIC_LOG))
    //{
    //   dcLog_log(FLAT_ARITHMETIC_LOG,
    //              "operation input: %s\n",
    //              dcNode_display(_node));
    //}

    //if (dcLog_isEnabled(FLAT_ARITHMETIC_LOG))
    //{
    //    dcLog_log(FLAT_ARITHMETIC_LOG, "shrunk: %s\n", dcNode_display(_node));
    //}

    bool done = false;
    dcNode *result = NULL;

    for (i = 0; ! done && _languages[i].programText != NULL; i++)
    {
        dcHash *scope = dcHash_create();
        dcNode *residuals = dcList_createNode();
        dcHash_setValueWithStringKey(scope,
                                     "residuals",
                                     dcVoidContainer_createNode(residuals));
        const dcListElement *programI = _languages[i].programTokens->head;
        const char *token = shiftElementToken(&programI);
        NewMatchFunction matcher = getNewMatchFunction(token);
        assert(matcher != NULL);
        dcNode *identifier = NULL;
        dcNode *arithmeticRest = NULL;

        if (matcher(token, scope, &programI, _node, _symbol))
        {
            identifier = createIdentifier(_symbol);
            dcHash_setValueWithStringKey(scope, "s", identifier);
            dcHash_setValueWithStringKey(scope, ":)", _node);
            dcLog_log(FLAT_ARITHMETIC_LOG,
                      "[%s] matched with program: %s\n"
                      "creating result: %s\n",
                      dcNode_display(_node),
                      _languages[i].programText,
                      _languages[i].result);

            //dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
            //          "[%s] matched with program: %s\n"
            //          "creating result: %s\n",
            //          dcNode_display(_node),
            //          _languages[i].programText,
            //          _languages[i].result);

            // set a breakpoint in calculusHelper for help
            TAFFY_DEBUG(calculusHelper();
                        updateCounts(dcVoidContainer_createNode
                                     ((void *)_languages), i));

            if (IS_FLAT_ARITHMETIC(_node))
            {
                arithmeticRest = dcNode_copy(_node, DC_DEEP);
                dcFlatArithmetic *arithmetic =
                    CAST_FLAT_ARITHMETIC(arithmeticRest);
                dcList_shift(arithmetic->values, DC_DEEP);

                // convert a 1-node flat arithmetic to a node
                if (arithmetic->values->size == 1
                    && arithmetic->taffyOperator != TAFFY_FACTORIAL)
                {
                    dcNode *tempArithmeticRest =
                        dcList_shift(arithmetic->values, DC_SHALLOW);
                    dcNode_free(&arithmeticRest, DC_DEEP);
                    arithmeticRest = tempArithmeticRest;
                }

                dcHash_setValueWithStringKey(scope, "R", arithmeticRest);
            }

            const dcListElement *resultI = _languages[i].resultTokens->head;
            result = parseTokens(scope, &resultI, _symbol);
            done = true;

            if (_modified != NULL)
            {
                *_modified = true;
            }
        }

        // hack! remove 'C' if residuals is non-empty
        if (CAST_LIST(residuals)->size > 0)
        {
            dcHash_removeValueWithStringKey(scope, "C", NULL, DC_SHALLOW);
        }

        dcHash_removeValueWithStringKey(scope, "residuals", NULL, DC_DEEP);
        dcHash_free(&scope, DC_SHALLOW);
        dcNode_free(&residuals, DC_DEEP);
        dcNode_free(&identifier, DC_DEEP);
        dcNode_free(&arithmeticRest, DC_DEEP);
    }

    return (result);
}

TAFFY_HIDDEN dcNode *convert(dcNode *_node, bool *_modified)
{
    if (dcGraphData_getType(_node) == NODE_METHOD_CALL)
    {
        dcMethodCall *call = CAST_METHOD_CALL(_node);

        //
        // convert abs(x^positive_number) into x^positive_number
        //
        if (strcmp(call->methodName, "#operator(()):") == 0
            && IS_IDENTIFIER(call->receiver)
            && dcIdentifier_equalsString(call->receiver, "abs"))
        {
            dcNode **argument = getMethodArgument(_node, 0, false);

            if (argument != NULL && IS_FLAT_ARITHMETIC(*argument))
            {
                dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(*argument);

                if (arithmetic->taffyOperator == TAFFY_RAISE
                    && arithmetic->values->size == 2
                    && dcNumberClass_isMe(arithmetic->values->tail->object)
                    && dcNumberClass_isEven(arithmetic->values->tail->object))
                {
                    setModified(_modified);
                    dcNode *result = dcNode_copy(*argument, DC_DEEP);
                    dcNode_free(&_node, DC_DEEP);
                    return result;
                }
            }
        }

        return _node;
    }
    else if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    dcNode *tail = dcList_getTail(arithmetic->values);
    dcNode *head = dcList_getHead(arithmetic->values);
    dcNode *result = _node;

    if (arithmetic->taffyOperator == TAFFY_DIVIDE)
    {
        //
        // convert 0 / x into 0
        //
        if (dcNumberClass_isMe(head)
            && dcNumberClass_equalsInt32u(head, 0))
        {
            setModified(_modified);
            result = createNumber(0);
            dcNode_free(&_node, DC_DEEP);
        }
        // convert a / b / c / ... into a / (b * c  ...)
        else if (arithmetic->taffyOperator == TAFFY_DIVIDE
                 && arithmetic->values->size > 2)
        {
            setModified(_modified);
            dcNode *top = dcList_shift(arithmetic->values, DC_SHALLOW);
            result = (CREATE_DIVIDE
                      (top,
                       dcFlatArithmetic_createNodeWithList
                       (TAFFY_MULTIPLY,
                        arithmetic->values),
                       NULL));
            arithmetic->values = NULL;
            dcNode_free(&_node, DC_DEEP);
        }
        // convert (a / b) / c into a / (b * c)
        else if (arithmetic->taffyOperator == TAFFY_DIVIDE
                 && arithmetic->values->size == 2
                 && IS_DIVIDE(arithmetic->values->head->object))
        {
            setModified(_modified);
            result = dcNode_copy(_node, DC_DEEP);
            dcFlatArithmetic *arithy = CAST_FLAT_ARITHMETIC(result);
            dcNode *taily = dcList_pop(arithy->values, DC_SHALLOW);

            // head is a divide
            dcNode *heady = FLAT_ARITHMETIC_HEAD(result);
            CAST_FLAT_ARITHMETIC(heady)->values->tail->object =
                CREATE_MULTIPLY(CAST_FLAT_ARITHMETIC
                                (heady)->values->tail->object,
                                taily,
                                NULL);
            dcNode_free(&_node, DC_DEEP);
        }
    }
    //
    // convert something like x^(-a) to: 1 / x^a
    //
    else if (arithmetic->taffyOperator == TAFFY_RAISE
             && arithmetic->values->size == 2
             && dcNumberClass_isMe(tail)
             && dcNumberClass_isNegative(tail))
    {
        setModified(_modified);
        dcNumberClass_inlineMultiply
            (tail, dcNumberClass_getNegativeOneNumberObject());
        result = (CREATE_DIVIDE
                  (createNumber(1),
                   CREATE_RAISE
                   (dcNode_copy(dcList_getHead(arithmetic->values), DC_DEEP),
                    dcNode_copy(tail, DC_DEEP),
                    NULL),
                   NULL));
        dcNode_free(&_node, DC_DEEP);
    }
    //
    // convert abs(x)^2 to x^2
    //
    else if (arithmetic->taffyOperator == TAFFY_RAISE
             && arithmetic->values->size == 2
             && dcNumberClass_isMe(tail)
             && dcNumberClass_isEven(tail)
             && IS_METHOD_CALL(head)
             && (strcmp(CAST_METHOD_CALL(head)->methodName, "#operator(()):")
                 == 0)
             && IS_IDENTIFIER(CAST_METHOD_CALL(head)->receiver)
             && dcIdentifier_equalsString(CAST_METHOD_CALL(head)->receiver,
                                          "abs"))
    {
        dcNode **argument = getMethodArgument(head, 0, false);

        // assert it?
        if (argument != NULL)
        {
            setModified(_modified);
            result = CREATE_RAISE(dcNode_copy(*argument, DC_DEEP),
                                  dcNode_copy(tail, DC_DEEP),
                                  NULL);
            dcNode_free(&_node, DC_DEEP);
        }
    }
    // convert 0 - a to -a
    // convert 0 - a - b to: -a - b
    else if (arithmetic->taffyOperator == TAFFY_SUBTRACT
             && dcNumberClass_equalsInt32u(head, 0)
             && arithmetic->values->size > 1)
    {
        setModified(_modified);
        dcList_shift(arithmetic->values, DC_DEEP);
        arithmetic->values->head->object = (CREATE_MULTIPLY
                                            (createNumber(-1),
                                             arithmetic->values->head->object,
                                             NULL));
    }
    // convert 0^x to 0
    else if (arithmetic->taffyOperator == TAFFY_RAISE
             && dcNumberClass_equalsInt32u(head, 0))
    {
        setModified(_modified);
        dcNode_free(&_node, DC_DEEP);
        result = createNumber(0);
    }
    // convert (27x^9)^(1/3) to 3x^3
    else if (arithmetic->taffyOperator == TAFFY_RAISE
             && arithmetic->values->size == 2
             && IS_MULTIPLY(arithmetic->values->head->object))
    {
        // not successors.. it's the node(s) that have had success
        dcList *successers = dcList_create();
        dcListElement *that = NULL;
        dcFlatArithmetic *headHead = (CAST_FLAT_ARITHMETIC
                                      (arithmetic->values->head->object));

        for (that = headHead->values->head; that != NULL;
            )
        {
            dcListElement *next = that->next;

            if (dcNumberClass_isMe(that->object))
            {
                bool modified = false;
                dcNode *raise = (dcFlatArithmetic_combine
                                 (CREATE_RAISE
                                  (dcNode_copy(that->object, DC_DEEP),
                                   dcNode_copy(FLAT_ARITHMETIC_TAIL(_node),
                                               DC_DEEP),
                                   NULL), // RAISE
                                  &modified));

                if (modified)
                {
                    dcList_push(successers, raise);
                    dcList_removeElement(headHead->values, &that, DC_DEEP);
                }
                else
                {
                    dcNode_free(&raise, DC_DEEP);
                }
            }

            that = next;
        }

        if (successers->size > 0)
        {
            // woot
            setModified(_modified);
            result = dcFlatArithmetic_createNodeWithList(TAFFY_MULTIPLY,
                                                         successers);

            if (headHead->values->size > 0)
            {
                dcList_push(CAST_FLAT_ARITHMETIC(result)->values, _node);
            }
            else
            {
                dcNode_free(&_node, DC_DEEP);
            }

            result = popIfNotFactorial(result, NULL);
        }
        else
        {
            dcList_free(&successers, DC_DEEP);
        }
    }
    // move numbers to the front of a multiplication.
    // this has the potential to cause an infinite loop scenario in shrink,
    // if numbers are not combined inbetwixt calls to merge.
    // thankfully they are.
    else if (arithmetic->taffyOperator == TAFFY_MULTIPLY)
    {
        dcListElement *that;

        for (that = arithmetic->values->head->next; that != NULL; )
        {
            dcListElement *next = that->next;

            if (dcNumberClass_isMe(that->object))
            {
                setModified(_modified);
                dcNode *save = that->object;
                dcList_removeElement(arithmetic->values, &that, DC_SHALLOW);
                dcList_unshift(arithmetic->values, save);
            }

            that = next;
        }
    }

    return result;
}

// convert an arithmetic into something better
// note: this can muck with _arithmetic
dcNode *dcFlatArithmetic_convert(dcNode *_arithmetic, bool *_modified)
{
    // conversion functions muck with arithmetic
    // so we need to preserve it in case they fail
    dcNode *copy = dcNode_copy(_arithmetic, DC_DEEP);
    dcNode *result = calculusOperation(copy, "", sConvertLanguages, _modified);
    dcNode_free(&copy, DC_DEEP);

    if (result == NULL)
    {
        result = convert(_arithmetic, _modified);

        if (IS_FLAT_ARITHMETIC(result))
        {
            FOR_EACH_IN_NODE(result, that)
            {
                SHRINK_OPERATION(convert, that->object, _modified);
            }
        }
    }
    else
    {
        dcNode_free(&_arithmetic, DC_DEEP);
    }

    return result;
}

static dcNode *collectTwo(dcNode *_node,
                          bool *_modified,
                          dcTaffyOperator _operator)
{
    dcNode *result = _node;
    dcNode *top = FLAT_ARITHMETIC_HEAD(_node);
    dcNode *bottom = FLAT_ARITHMETIC_TAIL(_node);

    if (IS_RAISE(top)
        && IS_RAISE(bottom))
    {
        dcList *topValues = CAST_FLAT_ARITHMETIC(top)->values;
        dcList *bottomValues = CAST_FLAT_ARITHMETIC(bottom)->values;
        dcNode *topSave = dcList_shift(topValues, DC_FLOATING);
        dcNode *bottomSave = dcList_shift(bottomValues, DC_FLOATING);

        if (dcNode_easyCompare(top, bottom) == TAFFY_EQUALS)
        {
            result = (dcFlatArithmetic_createNodeWithValues
                      (_operator, topSave, bottomSave, NULL));
            dcList_unshift(topValues, result);
            result = (dcFlatArithmetic_createNodeWithList
                      (TAFFY_RAISE,
                       topValues));
            setModified(_modified);

            CAST_FLAT_ARITHMETIC(top)->values = NULL;
            dcNode_free(&_node, DC_DEEP);
        }
        else
        {
            // oops, sorry, we'll put it back now
            dcList_unshift(topValues, topSave);
            dcList_unshift(bottomValues, bottomSave);
        }
    }

    return result;
}

dcNode *dcFlatArithmetic_collectPowers(dcNode *_node, bool *_modified)
{
    dcNode *result = _node;

    if (IS_DIVIDE(_node))
    {
        if (FLAT_ARITHMETIC_SIZE(_node) == 2)
        {
            result = collectTwo(_node, _modified, TAFFY_DIVIDE);
        }
    }
    else if (IS_MULTIPLY(_node) && false)
    {
        if (FLAT_ARITHMETIC_SIZE(_node) == 2)
        {
            result = collectTwo(_node, _modified, TAFFY_MULTIPLY);
        }
    }
    else if (IS_FLAT_ARITHMETIC(_node))
    {
        FOR_EACH_IN_NODE(_node, that)
        {
            that->object = dcFlatArithmetic_collectPowers(that->object,
                                                          _modified);
        }
    }

    return result;
}

TAFFY_HIDDEN dcResult compose(dcNode *_node,
                              const char *_identifier,
                              dcNode *_toCompose);

TAFFY_HIDDEN dcResult composeMethodCall(dcNode *_methodCall,
                                        const char *_substituteIdentifier,
                                        dcNode *_toCompose)
{
    dcNode **methodArgument = getMethodArgument(_methodCall, 0, false);
    dcResult result = TAFFY_FAILURE;

    if (methodArgument != NULL
        && IS_IDENTIFIER(*methodArgument)
        && (strcmp(CAST_IDENTIFIER(*methodArgument)->name,
                   _substituteIdentifier)
            == 0))
    {
        result = TAFFY_SUCCESS;
        dcNode_free(methodArgument, DC_DEEP);
        *methodArgument = dcNode_copy(_toCompose, DC_DEEP);
    }
    else if (methodArgument != NULL)
    {
        result = compose(*methodArgument, _substituteIdentifier, _toCompose);
    }

    return result;
}

TAFFY_HIDDEN dcResult composeFlatArithmetic(dcNode *_arithmeticNode,
                                            const char *_substituteIdentifier,
                                            dcNode *_toCompose)
{
    dcResult result = TAFFY_FAILURE;

    FOR_EACH_IN_NODE(_arithmeticNode, that)
    {
        if (IS_METHOD_CALL(that->object)
            && (composeMethodCall(that->object,
                                  _substituteIdentifier,
                                  _toCompose)
                == TAFFY_SUCCESS))
        {
            result = TAFFY_SUCCESS;
        }
        else if (IS_IDENTIFIER(that->object)
                 && (dcIdentifier_equalsString
                     (that->object, _substituteIdentifier)))
        {
            result = TAFFY_SUCCESS;
            dcNode_free(&that->object, DC_DEEP);
            that->object = dcNode_copy(_toCompose, DC_DEEP);
        }
        else if (IS_FLAT_ARITHMETIC(that->object))
        {
            result = composeFlatArithmetic(that->object,
                                           _substituteIdentifier,
                                           _toCompose);
        }
    }

    return result;
}

TAFFY_HIDDEN dcResult compose(dcNode *_node,
                              const char *_identifier,
                              dcNode *_toCompose)
{
    dcResult result = TAFFY_FAILURE;

    if (IS_FLAT_ARITHMETIC(_node))
    {
        result = composeFlatArithmetic(_node, _identifier, _toCompose);
    }
    else if (IS_METHOD_CALL(_node))
    {
        result = composeMethodCall(_node, _identifier, _toCompose);
    }

    return result;
}

TAFFY_HIDDEN dcResult removeNode(dcNode **_arithmetic, dcNode *_toRemove);

TAFFY_HIDDEN dcResult removeMatchFromList(dcList *_list, dcNode *_toRemove)
{
    dcListElement *found = NULL;
    dcResult result = dcList_findWithComparisonFunction(_list,
                                                        _toRemove,
                                                        &dcNode_compareEqual,
                                                        &found);
    if (result == TAFFY_SUCCESS)
    {
        dcList_removeElement(_list, &found, DC_DEEP);
    }

    return result;
}

TAFFY_HIDDEN dcResult removeDivide(dcNode **_arithmetic,
                                   dcNode *_toRemove)
{
    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(*_arithmetic);
    dcResult result = TAFFY_FAILURE;

    if (! IS_FLAT_ARITHMETIC(_toRemove))
    {
        result = removeNode(&arithmetic->values->head->object, _toRemove);

        // head's object will be NULL if it's entirely freed
        if (result == TAFFY_SUCCESS
            && arithmetic->values->head->object == NULL)
        {
            arithmetic->values->head->object = createNumber(1);
        }
    }
    else
    {
        dcFlatArithmetic *toRemove = CAST_FLAT_ARITHMETIC(_toRemove);

        if (toRemove->taffyOperator != TAFFY_DIVIDE)
        {
            result = removeNode(&arithmetic->values->head->object, _toRemove);

            // head's object will be NULL if it's entirely freed
            if (result == TAFFY_SUCCESS
                && arithmetic->values->head->object == NULL)
            {
                arithmetic->values->head->object = createNumber(1);
            }
        }
        else
        {
            if (toRemove->values->size > arithmetic->values->size)
            {
                return TAFFY_FAILURE;
            }

            dcListElement *here;
            dcListElement *that;
            dcNode *arithmeticCopy = dcNode_copy(*_arithmetic, DC_DEEP);
            result = TAFFY_SUCCESS;

            for (here = CAST_FLAT_ARITHMETIC(arithmeticCopy)->values->head,
                     that = toRemove->values->head;
                 here != NULL;
                 that = that->next)
            {
                // 'here' might be removed during the iteration, so store its
                // next value
                dcListElement *hereNext = here->next;

                if (dcNumberClass_equalsInt32u(that->object, 1))
                {
                    // do nothing
                }
                else if (IS_FLAT_ARITHMETIC(here->object))
                {
                    if (IS_FLAT_ARITHMETIC(that->object))
                    {
                        if (! removeNode(&here->object, that->object))
                        {
                            result = TAFFY_FAILURE;
                            break;
                        }
                    }
                    else
                    {
                        result = (removeMatchFromList
                                  (CAST_FLAT_ARITHMETIC(here->object)->values,
                                   that->object));
                    }
                }
                else
                {
                    dcTaffyOperator compareResult;
                    result = dcNode_compareEqual(here->object,
                                                 that->object,
                                                 &compareResult);

                    if (result == TAFFY_SUCCESS
                        && compareResult == TAFFY_EQUALS)
                    {
                        removeElement(CAST_FLAT_ARITHMETIC(arithmeticCopy),
                                      &here);
                    }
                    else if (result != TAFFY_SUCCESS)
                    {
                        break;
                    }
                    else if (compareResult != TAFFY_EQUALS)
                    {
                        result = TAFFY_FAILURE;
                        break;
                    }
                }

                here = hereNext;
            }

            if (result == TAFFY_SUCCESS)
            {
                dcNode_free(_arithmetic, DC_DEEP);
                *_arithmetic = arithmeticCopy;
            }
            else
            {
                dcNode_free(&arithmeticCopy, DC_DEEP);
            }
        }
    }

    return result;
}

TAFFY_HIDDEN dcResult removeCommutative(dcNode **_arithmetic,
                                        dcNode *_toRemove,
                                        int32_t _defaultValue)
                                        //dcNode **_numberResidual)
{
    dcResult result = TAFFY_FAILURE;
    dcNode *arithmeticNodeCopy = dcNode_copy(*_arithmetic, DC_DEEP);
    dcFlatArithmetic *arithmeticCopy = CAST_FLAT_ARITHMETIC(arithmeticNodeCopy);

    if (IS_FLAT_ARITHMETIC(_toRemove)
        && (CAST_FLAT_ARITHMETIC(_toRemove)->taffyOperator
            == arithmeticCopy->taffyOperator))
    {
        dcFlatArithmetic *toRemove = CAST_FLAT_ARITHMETIC(_toRemove);
        dcListElement *that;

        for (that = toRemove->values->head; that != NULL; that = that->next)
        {
            result = removeNode(&arithmeticNodeCopy, that->object);

            if (result != TAFFY_SUCCESS)
            {
                break;
            }
        }
    }
    else
    {
        dcListElement *found = NULL;
        result = dcList_findWithComparisonFunction(arithmeticCopy->values,
                                                   _toRemove,
                                                   &dcNode_compareEqual,
                                                   &found);
        if (result == TAFFY_SUCCESS)
        {
            removeElement(arithmeticCopy, &found);
        }
    }

    if (result == TAFFY_SUCCESS)
    {
        dcNode_free(_arithmetic, DC_DEEP);
        *_arithmetic = arithmeticNodeCopy;

        if (*_arithmetic == NULL)
        {
            *_arithmetic = createNumber(_defaultValue);
        }
    }
    else
    {
        dcNode_free(&arithmeticNodeCopy, DC_DEEP);
    }

    return result;
}

// it may throw an exception, but we'll catch it later
static bool dontCareCompare(dcNode *_left, dcNode *_right)
{
    if ((_left != NULL && _right == NULL)
        || (_left == NULL && _right != NULL))
    {
        return false;
    }

    dcTaffyOperator comparison;

    if (dcNode_compareEqual(_left, _right, &comparison) == TAFFY_SUCCESS
        && comparison == TAFFY_EQUALS)
    {
        return true;
    }

    return false;
}

static dcNode *expandOperations(dcNode *_program)
{
    SHRINK_OPERATION(multiplyByDenominator, _program, NULL);
    SHRINK_OPERATION(convertSubtractToAdd, _program, NULL);
    SHRINK_OPERATION(convertDivideToMultiply, _program, NULL);
    SHRINK_OPERATION(expandRaise, _program, NULL);
    SHRINK_OPERATION(merge, _program, NULL);
    return _program;
}

static bool bestRemove(dcNode **_arithmetic,
                       dcNode *_value,
                       const char *_symbol,
                       dcList *_residualSymbols,
                       dcNodeEvaluator *_evaluator)
{
    bool result = false;

    if (_symbol != NULL
        && (dcNumberClass_isMe(_value)
            || (IS_IDENTIFIER(_value)
                && ! dcIdentifier_equalsString(_value, _symbol))))
    {
        dcList_push(_residualSymbols, dcNode_copy(_value, DC_DEEP));
        result = true;
    }
    else if (! IS_MULTIPLY(*_arithmetic))
    {
        if (dcFlatArithmetic_equals(*_arithmetic, _value))
        {
            dcNode_free(_arithmetic, DC_DEEP);
            *_arithmetic = createNumber(1);
            result = true;
        }
    }
    else
    {
        FOR_EACH_IN_NODE(*_arithmetic, that)
        {
            if (dcFlatArithmetic_equals(that->object, _value))
            {
                dcList_removeElement(CAST_FLAT_ARITHMETIC(*_arithmetic)->values,
                                     &that,
                                     DC_DEEP);

                if (CAST_FLAT_ARITHMETIC(*_arithmetic)->values->size == 0)
                {
                    dcNode_free(_arithmetic, DC_DEEP);
                    *_arithmetic = createNumber(1);
                }

                result = true;
                break;
            }
        }
    }

    return result;
}

bool dcFlatArithmetic_remove(dcNode **_arithmetic, // in/out
                             dcNode *_toRemove,    // in
                             const char *_symbol)
{
    if (dcNumberClass_isOne(_toRemove)
        || dcNumberClass_isNegativeOne(_toRemove))
    {
        return false;
    }

    bool result = false;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *arithmetic = dcNode_copy(*_arithmetic, DC_DEEP);
    dcNode *toRemove = dcNode_copy(_toRemove, DC_DEEP);
    dcList *residualSymbols = dcList_create();

    arithmetic = expandOperations(arithmetic);
    toRemove = expandOperations(toRemove);

    if (! IS_MULTIPLY(toRemove))
    {
        result = bestRemove(&arithmetic,
                            _toRemove,
                            _symbol,
                            residualSymbols,
                            evaluator);
    }
    else
    {
        result = true;

        FOR_EACH_IN_NODE(toRemove, that)
        {
            if (! bestRemove(&arithmetic,
                             that->object,
                             _symbol,
                             residualSymbols,
                             evaluator))
            {
                result = false;
                break;
            }
        }
    }

    if (result)
    {
        dcNode_free(_arithmetic, DC_DEEP);
        *_arithmetic = arithmetic;

        if (residualSymbols->size > 0)
        {
            *_arithmetic = (CREATE_MULTIPLY
                            (*_arithmetic,
                             (CREATE_DIVIDE
                              (createNumber(1),
                               dcFlatArithmetic_createNodeWithList
                               (TAFFY_MULTIPLY,
                                residualSymbols),
                               NULL)), // divide
                             NULL));   // multiply
        }
        else
        {
            dcList_free(&residualSymbols, DC_DEEP);
        }
    }
    else
    {
        dcNode_free(&arithmetic, DC_DEEP);
        dcList_free(&residualSymbols, DC_DEEP);
    }

    dcNode_free(&toRemove, DC_DEEP);
    return result;
}

static bool isGoodNumber(dcNode *_node)
{
    return (dcNumberClass_isMe(_node)
            && ! dcNumberClass_isNegative(_node)
            && ! dcNumberClass_isZero(_node)
            && ! dcNumberClass_isOne(_node)
            && ! dcNumberClass_isNegativeOne(_node));
}

static bool isGoodyNumber(dcNode **_number)
{
    return (dcNumberClass_isMe(*_number)
            && ! dcNumberClass_isZero(*_number)
            && ! dcNumberClass_isOne(*_number)
            && ! dcNumberClass_isNegativeOne(*_number));
}

static void divideTopAndBottom(dcNode **_top, dcNode **_bottom)
{
    dcNumberClass_inlineDivide(*_bottom, *_top);
    dcNumber_floor(dcNumberClass_getNumber(*_bottom),
                   dcNumberClass_getNumber(*_bottom));
    dcNode_free(_top, DC_DEEP);

    if (dcNumberClass_isNegative(*_bottom))
    {
        dcNumberClass_inlineMultiply
            (*_bottom, dcNumberClass_getNegativeOneNumberObject());
        *_top = createNumber(-1);
    }
    else
    {
        *_top = createNumber(1);
    }
}

static bool cancelWithFactor(dcNode **_top,
                             dcNode **_bottom,
                             bool _factor)
{
    if (dcLog_isEnabled(FLAT_ARITHMETIC_REMOVE_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_REMOVE_LOG,
                  "removing for canceling: '%s' from: '%s'\n",
                  dcNode_display(*_top),
                  dcNode_display(*_bottom));
    }

    addCount("Cancel");
    bool result = false;

    if (IS_MULTIPLY(*_top))
    {
        FOR_EACH_IN_NODE(*_top, that)
        {
            if (dcFlatArithmetic_cancelTopAndBottom(&that->object,
                                                    _bottom,
                                                    _factor))
            {
                result = true;
            }
        }
    }

    if (IS_MULTIPLY(*_bottom))
    {
        FOR_EACH_IN_NODE(*_bottom, that)
        {
            if (dcFlatArithmetic_cancelTopAndBottom(_top,
                                                    &that->object,
                                                    _factor))
            {
                result = true;
            }
        }
    }

    if ((IS_FLAT_ARITHMETIC(*_top)
         && FLAT_ARITHMETIC_SIZE(*_top) == 0)
        || (IS_FLAT_ARITHMETIC(*_bottom)
            && FLAT_ARITHMETIC_SIZE(*_bottom) == 0))
    {
        return result;
    }

    // for use below
    dcNode *topHead = (IS_FLAT_ARITHMETIC(*_top)
                       ? FLAT_ARITHMETIC_HEAD(*_top)
                       : NULL);
    dcNode *bottomHead = (IS_FLAT_ARITHMETIC(*_bottom)
                          ? FLAT_ARITHMETIC_HEAD(*_bottom)
                          : NULL);
    dcNumber *gcdResult = dcNumber_createFromInt32u(0);
    dcNumber *topNumber = (dcNumberClass_isMe(*_top)
                           ? dcNumberClass_getNumber(*_top)
                           : NULL);
    dcNumber *bottomNumber = (dcNumberClass_isMe(*_bottom)
                              ? dcNumberClass_getNumber(*_bottom)
                              : NULL);
    bool goodNumbers = (isGoodyNumber(_top)
                        && isGoodyNumber(_bottom));

    if (dontCareCompare(*_top, *_bottom))
    {
        // cancel case 1
        result = true;
        dcNode_free(_top, DC_DEEP);
        dcNode_free(_bottom, DC_DEEP);
        *_top = createNumber(1);
        *_bottom = createNumber(1);
    }
    else if (IS_RAISE(*_top)
             && dontCareCompare(FLAT_ARITHMETIC_HEAD(*_top), *_bottom))
    {
        // cancel case 3
        result = true;
        dcNode *head = dcList_shift(CAST_FLAT_ARITHMETIC(*_top)->values,
                                    DC_SHALLOW);
        dcNode *exponent = NULL;

        if (FLAT_ARITHMETIC_SIZE(*_top) == 1
            && dcNumberClass_isMe(FLAT_ARITHMETIC_HEAD(*_top)))
        {
            dcNumberClass_decrement(FLAT_ARITHMETIC_HEAD(*_top));
            exponent = *_top;
        }
        else
        {
            exponent = CREATE_SUBTRACT(*_top, createNumber(1), NULL);
        }

        *_top = CREATE_RAISE(head, exponent, NULL);
        dcNode_free(_bottom, DC_DEEP);
        *_bottom = createNumber(1);
    }
    else if (IS_RAISE(*_bottom)
             && dontCareCompare(FLAT_ARITHMETIC_HEAD(*_bottom), *_top))
    {
        // cancel case 4
        result = true;
        dcNode *head = dcList_shift(CAST_FLAT_ARITHMETIC(*_bottom)->values,
                                    DC_SHALLOW);
        dcNode *exponent = NULL;

        if (FLAT_ARITHMETIC_SIZE(*_bottom) == 1
            && dcNumberClass_isMe(FLAT_ARITHMETIC_HEAD(*_bottom)))
        {
            dcNumberClass_decrement(FLAT_ARITHMETIC_HEAD(*_bottom));
            exponent = *_bottom;
        }
        else
        {
            exponent = CREATE_SUBTRACT(*_bottom, createNumber(1), NULL);
        }

        *_bottom = CREATE_RAISE(head, exponent, NULL);
        dcNode_free(_top, DC_DEEP);
        *_top = createNumber(1);
    }
    else if (IS_ADD(*_top) || IS_SUBTRACT(*_top))
    {
        // cancel case 2
        dcNode *topCopy = dcNode_copy(*_top, DC_DEEP);
        dcNode *bottomCopy = NULL;
        bool success = true;
        dcNode *bottom = NULL;

        FOR_EACH_IN_NODE(topCopy, that)
        {
            dcNode_free(&bottomCopy, DC_DEEP);
            bottomCopy = dcNode_copy(*_bottom, DC_DEEP);

            if (dcFlatArithmetic_cancelTopAndBottom(&that->object,
                                                    &bottomCopy,
                                                    _factor))
            {
                if (bottom == NULL)
                {
                    bottom = dcNode_copy(bottomCopy, DC_DEEP);
                }
                else if (! dontCareCompare(bottom, bottomCopy))
                {
                    // bail
                    success = false;
                    break;
                }
            }
            else
            {
                success = false;
                break;
            }
        }

        dcNode_free(&bottomCopy, DC_DEEP);

        if (success)
        {
            result = true;
            dcNode_free(_top, DC_DEEP);
            dcNode_free(_bottom, DC_DEEP);
            *_top = topCopy;
            *_bottom = bottom;
        }
        else
        {
            dcNode_free(&topCopy, DC_DEEP);
            dcNode_free(&bottomCopy, DC_DEEP);
            dcNode_free(&bottom, DC_DEEP);
        }
    }
    else if (IS_RAISE(*_top)
             && IS_RAISE(*_bottom)
             && FLAT_ARITHMETIC_SIZE(*_top) == 2
             && FLAT_ARITHMETIC_SIZE(*_bottom) == 2
             && dontCareCompare(topHead, bottomHead)
             && dcNumberClass_isMe(FLAT_ARITHMETIC_TAIL(*_top))
             && dcNumberClass_isMe(FLAT_ARITHMETIC_TAIL(*_bottom)))
    {
        if (dcNumber_lessThan
            (dcNumberClass_getNumber(FLAT_ARITHMETIC_TAIL(*_top)),
             dcNumberClass_getNumber(FLAT_ARITHMETIC_TAIL(*_bottom))))
        {
            // cancel case 5
            result = true;
            dcNode *head = dcList_shift(CAST_FLAT_ARITHMETIC(*_bottom)->values,
                                        DC_SHALLOW);
            dcList_shift(CAST_FLAT_ARITHMETIC(*_top)->values, DC_DEEP);
            dcNode *bottomNumberNode = (dcList_shift
                                        (CAST_FLAT_ARITHMETIC(*_bottom)->values,
                                         DC_SHALLOW));
            dcNode *topNumberNode = (dcList_shift
                                     (CAST_FLAT_ARITHMETIC(*_top)->values,
                                      DC_SHALLOW));
            dcNumberClass_inlineSubtract(bottomNumberNode, topNumberNode);
            dcNode_free(&topNumberNode, DC_DEEP);
            dcNode_free(_bottom, DC_DEEP);
            *_bottom = CREATE_RAISE(head, bottomNumberNode, NULL);
            dcNode_free(_top, DC_DEEP);
            *_top = createNumber(1);
        }
        else
        {
            // cancel case 6
            result = true;
            dcNode *head = dcList_shift(CAST_FLAT_ARITHMETIC(*_top)->values,
                                        DC_SHALLOW);
            dcList_shift(CAST_FLAT_ARITHMETIC(*_bottom)->values, DC_DEEP);
            dcNode *topNumberNode = (dcList_shift
                                     (CAST_FLAT_ARITHMETIC(*_top)->values,
                                      DC_SHALLOW));
            dcNode *bottomNumberNode = (dcList_shift
                                        (CAST_FLAT_ARITHMETIC(*_bottom)->values,
                                         DC_SHALLOW));
            dcNumberClass_inlineSubtract(topNumberNode, bottomNumberNode);
            dcNode_free(&bottomNumberNode, DC_DEEP);
            dcNode_free(_top, DC_DEEP);
            *_top = CREATE_RAISE(head, topNumberNode, NULL);
            dcNode_free(_bottom, DC_DEEP);
            *_bottom = createNumber(1);
        }
    }
    else if (IS_RAISE(*_top)
             && IS_RAISE(*_bottom)
             && dontCareCompare(topHead, bottomHead))
    {
        // cancel case 7
        result = true;
        dcNode *head = dcList_shift(CAST_FLAT_ARITHMETIC(*_top)->values,
                                    DC_SHALLOW);
        dcList_shift(CAST_FLAT_ARITHMETIC(*_bottom)->values, DC_DEEP);
        dcNode *exponent = CREATE_SUBTRACT(*_top,
                                           dcNode_copy(*_bottom, DC_DEEP),
                                           NULL);
        *_top = CREATE_RAISE(head, exponent, NULL);
        dcNode_free(_bottom, DC_DEEP);
        *_bottom = createNumber(1);
    }
    // convert decimal in the denominator to fraction if we can
    else if (dcNumberClass_isMe(*_top)
             && dcNumberClass_isMe(*_bottom)
             && ! dcNumberClass_isWholeHelper(*_bottom))
    {
        // cancel case 8
        dcNumber *bottom = dcNumberClass_getNumber(*_bottom);
        dcNumber *top = dcNumberClass_getNumber(*_top);
        const dcNumber *ten = dcNumber_getConstant(10);
        int32_t exponent = dcNumber_getExponent(bottom);

        // need some limit, somewhat arbitrary
        if (exponent >= -50)
        {
            result = true;
            // TODO: enfaster this by creating a single number to multiply by
            while (! dcNumber_isWhole(bottom))
            {
                assert(dcNumber_multiply(top, top, ten)
                       == TAFFY_NUMBER_SUCCESS);
                assert(dcNumber_multiply(bottom, bottom, ten)
                       == TAFFY_NUMBER_SUCCESS);
            }

            dcNumber_floor(top, top);
            dcNumber_floor(bottom, bottom);

            if (isGoodyNumber(_top)
                && isGoodyNumber(_bottom))
            {
                if (dcNumberClass_divides(*_top, *_bottom))
                {
                    divideTopAndBottom(_top, _bottom);
                }
                else if (dcNumberClass_divides(*_bottom, *_top))
                {
                    divideTopAndBottom(_bottom, _top);
                }
            }
        }
    }
    else if (goodNumbers
             && dcNumberClass_divides(*_top, *_bottom))
    {
        // cancel case 9
        result = true;
        divideTopAndBottom(_top, _bottom);
    }
    else if (goodNumbers
             && (dcNumberClass_divides(*_bottom, *_top)
                 || ! dcNumberClass_isWholeHelper(*_top)
                 || ! dcNumberClass_isWholeHelper(*_bottom)))
    {
        // cancel case 10
        result = true;
        bool isWhole = (dcNumberClass_isWholeHelper(*_top)
                        && dcNumberClass_isWholeHelper(*_bottom));
        dcNumberClass_inlineDivide(*_top, *_bottom);

        if (isWhole)
        {
            dcNumber_floor(dcNumberClass_getNumber(*_top),
                           dcNumberClass_getNumber(*_top));
        }

        dcNode_free(_bottom, DC_DEEP);
        *_bottom = createNumber(1);
    }
    else if (goodNumbers
             && (dcNumber_gcd(gcdResult, topNumber, bottomNumber)
                 == TAFFY_NUMBER_SUCCESS)
             && ! dcNumber_equalsInt32u(gcdResult, 1)
             && ! dcNumber_equalsInt32s(gcdResult, -1))
    {
        // cancel case 10
        result = true;
        dcNumber_divide(topNumber, topNumber, gcdResult);
        dcNumber_divide(bottomNumber, bottomNumber, gcdResult);
    }
    else if (dcNumberClass_isMe(*_top)
             && dcNumberClass_isMe(*_bottom)
             && dcNumberClass_isZero(*_bottom))
    {
        // cancel case 11
        result = true;
        dcNode_free(_top, DC_DEEP);
        dcNode_free(_bottom, DC_DEEP);
        *_top = (dcNode_setTemplate
                 (dcNumberClass_createObject(dcNumber_getNaN()),
                  true));
        *_bottom = createNumber(1);
    }
    else if (dcComplexNumberClass_isMe(*_top)
             && dcComplexNumberClass_isMe(*_bottom))
    {
        // cancel case 12
        // TODO: need more checks on top and bottom?
        result = true;
        dcComplexNumberClass_inlineDivide(*_top, *_bottom);
        dcNode_free(_bottom, DC_DEEP);
        *_bottom = createNumber(1);
    }
    else if (dcComplexNumberClass_isMe(*_top)
             && dcComplexNumberClass_isMe(*_bottom))
    {
        // cancel case 13
        // TODO: need more checks on top and bottom?
        result = true;
        dcComplexNumberClass_inlineDivide(*_top, *_bottom);
        dcNode_free(_bottom, DC_DEEP);
        *_bottom = createNumber(1);
    }
    else if (dcNumberClass_isMe(*_top)
             && dcComplexNumberClass_isMe(*_bottom))
    {
        // cancel case 14
        // TODO: need more checks on top and bottom?
        result = true;
        *_bottom =
            dcComplexNumberClass_inlineDivideReal(*_top, *_bottom, false);
        dcNode_free(_top, DC_DEEP);
        *_top = *_bottom;
        *_bottom = createNumber(1);
    }
    else if (dcComplexNumberClass_isMe(*_top)
             && dcNumberClass_isMe(*_bottom))
    {
        // cancel case 15
        // TODO: need more checks on top and bottom?
        result = true;
        *_top = dcComplexNumberClass_inlineDivideReal(*_top, *_bottom, true);
        dcNode_free(_bottom, DC_DEEP);
        *_bottom = createNumber(1);
    }
    // bring negative to top
    else if (isNegativeMultiply(*_bottom, false)
             || (dcNumberClass_isMe(*_bottom)
                 && dcNumberClass_isNegative(*_bottom)))
    {
        // cancel case 16
        result = true;

        if (dcNumberClass_isMe(*_top))
        {
            // 16.1
            dcNumberClass_negateHelper(*_top);
        }
        else if (isNegativeMultiply(*_top, false))
        {
            // 16.2
            dcNode *head = FLAT_ARITHMETIC_HEAD(*_top);

            if (dcNumberClass_isNegativeOne(head))
            {
                // 16.2.1
                dcList *values = CAST_FLAT_ARITHMETIC(*_top)->values;
                dcList_shift(values, DC_DEEP);
            }
            else if (dcNumberClass_isMe(head))
            {
                // 16.2.2
                dcNumberClass_negateHelper(head);
            }
            else
            {
                assert(false);
            }
        }
        else
        {
            // 16.3
            *_top = CREATE_MULTIPLY(createNumber(-1), *_top, NULL);
        }

        if (dcNumberClass_isMe(*_bottom))
        {
            // 16.4
            dcNumberClass_negateHelper(*_bottom);
        }
        else
        {
            assert(IS_FLAT_ARITHMETIC(*_bottom));
            dcNode *head = FLAT_ARITHMETIC_HEAD(*_bottom);

            if (dcNumberClass_isNegativeOne(head))
            {
                // 16.5
                dcList *values = CAST_FLAT_ARITHMETIC(*_bottom)->values;
                dcList_shift(values, DC_DEEP);
            }
            else if (dcNumberClass_isMe(head))
            {
                // 16.6
                dcNumberClass_negateHelper(head);
            }
            else
            {
                assert(false);
            }
        }
    }

    dcNumber_free(&gcdResult, DC_DEEP);
    return result;
}

bool dcFlatArithmetic_cancelTopAndBottom(dcNode **_top,
                                         dcNode **_bottom,
                                         bool _factor)
{
    addCount("Remove for Canceling");
    bool result = false;
    dcNode *topBackup = dcNode_copy(*_top, DC_DEEP);
    dcNode *bottomBackup = dcNode_copy(*_bottom, DC_DEEP);

    if (_factor)
    {
        *_top = dcFlatArithmetic_multiFactor(*_top, NULL);
        *_bottom = dcFlatArithmetic_multiFactor(*_bottom, NULL);
    }

    result = cancelWithFactor(_top, _bottom, false);

    if (result)
    {
        dcNode_free(&topBackup, DC_DEEP);
        dcNode_free(&bottomBackup, DC_DEEP);
    }
    else
    {
        dcNode_free(_top, DC_DEEP);
        dcNode_free(_bottom, DC_DEEP);
        *_top = topBackup;
        *_bottom = bottomBackup;
    }

    return result;
}

dcNode *dcFlatArithmetic_multiFactor(dcNode *_program, bool *_modified)
{
    addCount("Multi Factor");

    bool loopModified = false;
    size_t safety = 0;
    dcStringCacheElement element = {0};

    do
    {
        loopModified = false;
        SHRINK_OPERATION(factor, _program, &loopModified);
        SHRINK_OPERATION(merge, _program, NULL);
        SHRINK_OPERATION(snip, _program, NULL);

        if (loopModified)
        {
            setModified(_modified);
        }

        assert(safety++ < 30);
    }
    while (loopModified);

    dcStringCacheElement_free(&element);
    return _program;
}

// this mucks with _program
dcNode *dcFlatArithmetic_cancel(dcNode *_program, bool *_modified)
{
    addCount("dcFlatArithmetic_cancel");
    dcStringCacheElement element = {0};

    if (dcStringCache_getVoidOrNot(sCancelCache,
                                   _program,
                                   &element,
                                   _modified)
        == TAFFY_SUCCESS)
    {
        dcNode *result = element.value;
        dcStringCacheElement_free(&element);
        return result;
    }

    if (IS_FLAT_ARITHMETIC(_program))
    {
        FOR_EACH_IN_NODE(_program, that)
        {
            SHRINK_OPERATION(cancel, that->object, _modified);
        }
    }

    if (! IS_DIVIDE(_program)
        || FLAT_ARITHMETIC_SIZE(_program) != 2)
    {
        dcStringCacheElement_free(&element);
        return _program;
    }

    dcNode *result = _program;
    dcListElement *numerator = CAST_FLAT_ARITHMETIC(result)->values->head;
    dcListElement *denominator =
        CAST_FLAT_ARITHMETIC(result)->values->head->next;

    if (dcFlatArithmetic_cancelTopAndBottom(&numerator->object,
                                            &denominator->object,
                                            true))
    {
        dcStringCache_add(sCancelCache, &element, _program);
        setModified(_modified);
    }
    else
    {
        dcStringCache_add(sCancelCache, &element, NULL);
    }

    dcStringCacheElement_free(&element);
    return _program;
}

TAFFY_HIDDEN dcResult removeNode(dcNode **_arithmetic, dcNode *_toRemove)
{
    dcResult result = TAFFY_FAILURE;
    addCount("removeNode");

    if (IS_FLAT_ARITHMETIC(*_arithmetic))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(*_arithmetic);

        if (arithmetic->taffyOperator == TAFFY_DIVIDE)
        {
            result = removeDivide(_arithmetic, _toRemove);
        }
        else if (arithmetic->taffyOperator == TAFFY_MULTIPLY)
        {
            result = removeCommutative(_arithmetic, _toRemove, 1);
        }
        else if (dcNode_easyCompare(*_arithmetic, _toRemove) == TAFFY_EQUALS)
        {
            dcNode_free(_arithmetic, DC_DEEP);
            *_arithmetic = NULL;
            result = TAFFY_SUCCESS;
        }

        *_arithmetic = popIfNotFactorial(*_arithmetic, NULL);
    }
    else
    {
        dcTaffyOperator compareResult;
        result = dcNode_compareEqual(*_arithmetic, _toRemove, &compareResult);

        if (result == TAFFY_SUCCESS)
        {
            if (compareResult == TAFFY_EQUALS)
            {
                dcNode_free(_arithmetic, DC_DEEP);
                *_arithmetic = NULL;
            }
            else
            {
                // errrrrrrrrgh
                result = TAFFY_FAILURE;
            }
        }
    }

    return result;
}

typedef struct
{
    dcNode **parent;
    dcNode **ruler;
    dcNode *child;
    bool subObject;
} Chosen;

static void addChosenParent(dcNode **_parent,
                            dcNode **_ruler,
                            dcNode *_child,
                            dcHash *_parents,
                            bool _subObject)
{
    Chosen *chosen = (Chosen *)dcMemory_allocate(sizeof(Chosen));
    chosen->parent = _parent;
    chosen->ruler = _ruler;
    chosen->child = _child;
    chosen->subObject = _subObject;
    assert(dcHash_setValueWithHashValue(_parents,
                                        NULL,
                                        (dcHashType)(size_t)_child,
                                        dcVoid_createNode(chosen)));
}

static void choose(dcNode *_arithmetic,
                   const char *_symbol,
                   dcList *_list,
                   dcListElement *_position,
                   uint32_t _totalCount,
                   uint32_t _count,
                   RemoveType _chooseType,
                   dcList *_results,
                   dcHash *_parents,
                   dcNode **_parent,
                   dcNode **_ruler)
{
    bool success = true;
    dcListElement *that;

    if (_count == 0)
    {
        goto kickout;
    }

    if (_totalCount == 1
        && _chooseType == REMOVE_FOR_SUBSTITUTION)
    {
        for (that = _position; that != NULL; that = that->next)
        {
            if (IS_MULTIPLY(that->object)
                || IS_DIVIDE(that->object)
                || IS_RAISE(that->object))
            {
                choose(that->object,
                       _symbol,
                       CAST_FLAT_ARITHMETIC(that->object)->values,
                       CAST_FLAT_ARITHMETIC(that->object)->values->head,
                       _totalCount,
                       _count,
                       _chooseType,
                       _results,
                       _parents,
                       _parent,
                       (_ruler != NULL
                        ? _ruler
                        : &that->object));
            }
        }
    }

    for (that = _position; that != NULL; that = that->next)
    {
        if (_chooseType == REMOVE_FOR_SUBSTITUTION
            && ! IS_METHOD_CALL(that->object)
            && ! IS_FLAT_ARITHMETIC(that->object))
        {
            continue;
        }
        else if (_chooseType == REMOVE_BY_PARTS
                 && IS_DIVIDE(_arithmetic)
                 && that != _position)
        {
            break;
        }

        dcList *results = dcList_create();

        // if we're removing by parts, we only care about the head of a divide
        if (that->next != NULL
            && ((_chooseType == REMOVE_BY_PARTS
                 && ! IS_DIVIDE(_arithmetic))
                || _chooseType == REMOVE_FOR_SUBSTITUTION))
        {
            choose(that->next->object,
                   _symbol,
                   _list,
                   that->next,
                   _totalCount,
                   _count - 1,
                   _chooseType,
                   results,
                   _parents,
                   &that->object,
                   _ruler);
        }

        dcListElement *i;
        dcNode **argument = NULL;
        dcNode *exponent = NULL;

        if (IS_METHOD_CALL(that->object)
            && CAST_METHOD_CALL(that->object)->arguments->size == 1
            && _totalCount == 1
            && _chooseType == REMOVE_FOR_SUBSTITUTION)
        {
            dcNode **candidate = getMethodArgument(that->object, 0, true);

            if (candidate == NULL)
            {
                success = false;
                goto kickout;
            }

            // we only want arguments that are arithmetic
            if (IS_FLAT_ARITHMETIC(*candidate))
            {
                argument = candidate;
            }
        }
        else if (IS_RAISE(that->object)
                 && _totalCount == 1
                 && _chooseType == REMOVE_FOR_SUBSTITUTION)
        {
            dcFlatArithmetic *arithy = CAST_FLAT_ARITHMETIC(that->object);
            dcNode *thisExponent = arithy->values->head->next->object;

            // FIX ME for == 2
            if (arithy->values->size == 2
                && (IS_FLAT_ARITHMETIC(thisExponent)
                    || IS_METHOD_CALL(thisExponent)))
            {
                exponent = dcNode_copy(that->object, DC_DEEP);
                dcList_shift(CAST_FLAT_ARITHMETIC(exponent)->values, DC_DEEP);
                exponent = popIfNotFactorial(exponent, NULL);
            }
        }

        if (results->size == 0)
        {
            dcNode *copy = NULL;

            if (! dcNumberClass_isOne(that->object)
                && ! dcNumberClass_isNegativeOne(that->object))
            {
                copy = dcNode_copy(that->object, DC_DEEP);
                dcList_push(results, dcList_createNodeWithObjects(copy, NULL));
                addChosenParent((_parent != NULL
                                 ? _parent
                                 : &that->object),
                                (_ruler != NULL
                                 ? _ruler
                                 : &that->object),
                                copy,
                                _parents,
                                false);
            }

            if (_count == 1 && _chooseType == REMOVE_FOR_SUBSTITUTION)
            {
                if (argument != NULL)
                {
                    copy = dcNode_copy(*argument, DC_DEEP);
                    dcList_push(results,
                                dcList_createNodeWithObjects(copy, NULL));
                    addChosenParent((_parent != NULL
                                     ? _parent
                                     : &that->object),
                                    (_ruler != NULL
                                     ? _ruler
                                     : &that->object),
                                    copy,
                                    _parents,
                                    true);
                }
                else if (exponent != NULL)
                {
                    dcList_push(results,
                                dcList_createNodeWithObjects
                                (exponent, NULL));
                    addChosenParent((_parent != NULL
                                     ? _parent
                                     : &that->object),
                                    (_ruler != NULL
                                     ? _ruler
                                     : &that->object),
                                    exponent,
                                    _parents,
                                    true);
                }
            }
        }
        else
        {
            dcList *backup = dcList_copy(results, DC_DEEP);

            // if _arithmetic is divide, then only prepend the current
            // object if it's the numerator
            // if it's not divide, then prepend it!
            if (_totalCount > 1
                && ((! IS_FLAT_ARITHMETIC(_arithmetic)
                     || (CAST_FLAT_ARITHMETIC(_arithmetic)->taffyOperator
                         != TAFFY_DIVIDE))
                    || (_totalCount == 1
                        && (CAST_FLAT_ARITHMETIC(_arithmetic)->taffyOperator
                            == TAFFY_DIVIDE)
                        && that == (CAST_FLAT_ARITHMETIC
                                    (_arithmetic)->values->head)))
                && (! IS_IDENTIFIER(that->object)
                    || dcIdentifier_equalsString(that->object, _symbol)))
            {
                for (i = results->head; i != NULL; i = i->next)
                {
                    dcNode *copy = dcNode_copy(that->object, DC_DEEP);
                    dcList_unshift(CAST_LIST(i->object), copy);
                    addChosenParent((_parent != NULL
                                     ? _parent
                                     : &that->object),
                                    (_ruler != NULL
                                     ? _ruler
                                     : &that->object),
                                    copy,
                                    _parents,
                                    false);
                }
            }

            if (_count == 1 && _chooseType == REMOVE_FOR_SUBSTITUTION)
            {
                if (argument != NULL
                    && (! IS_IDENTIFIER(*argument)
                        || dcIdentifier_equalsString(*argument, _symbol)))
                {
                    dcList *newResults = dcList_copy(backup, DC_DEEP);

                    for (i = newResults->head; i != NULL; i = i->next)
                    {
                        dcNode *copy = dcNode_copy(*argument, DC_DEEP);
                        dcList_unshift(CAST_LIST(i->object), copy);
                        addChosenParent((_parent != NULL
                                         ? _parent
                                         : &that->object),
                                        (_ruler != NULL
                                         ? _ruler
                                         : &that->object),
                                        copy,
                                        _parents,
                                        true);
                    }

                    dcList_append(results, &newResults);
                }
                else if (exponent != NULL
                         && (! IS_IDENTIFIER(exponent)
                             || dcIdentifier_equalsString(exponent, _symbol)))
                {
                    dcList *newResults = dcList_copy(backup, DC_DEEP);

                    for (i = newResults->head; i != NULL; i = i->next)
                    {
                        dcNode *copy = dcNode_copy(exponent, DC_DEEP);
                        dcList_unshift(CAST_LIST(i->object), copy);
                        addChosenParent((_parent != NULL
                                         ? _parent
                                         : &that->object),
                                        (_ruler != NULL
                                         ? _ruler
                                         : &that->object),
                                        copy,
                                        _parents,
                                        true);
                    }

                    dcNode_free(&exponent, DC_DEEP);
                    dcList_append(results, &newResults);
                }
            }

            dcList_free(&backup, DC_DEEP);
        }

        dcList_append(_results, &results);
    }

kickout:
    if (! success)
    {
        dcList_clear(_results, DC_DEEP);
    }
}

void dcFlatArithmetic_choose(dcNode *_arithmetic,    // input
                             const char *_symbol,    // input
                             uint32_t _count,        // input
                             RemoveType _chooseType, // input
                             dcList *_results,       // output
                             dcHash *_parents)       // output
{
    dcNode *arithmetic = _arithmetic;
    dcFlatArithmetic *arithy = CAST_FLAT_ARITHMETIC(arithmetic);
    bool freeArithmetic = false;

    // if choosing by parts then we can only work with multiplication
    if (arithy->taffyOperator != TAFFY_MULTIPLY
        && _chooseType == REMOVE_BY_PARTS)
    {
        return;
    }

    if (_count == 1
        && _chooseType == REMOVE_FOR_SUBSTITUTION
        && IS_METHOD_CALL(_arithmetic)
        && CAST_METHOD_CALL(_arithmetic)->arguments->size == 1
        && IS_FLAT_ARITHMETIC(*getMethodArgument(_arithmetic, 0, true)))
    {
        dcNode *copy = dcNode_copy(*getMethodArgument(_arithmetic, 0, true),
                                   DC_DEEP);
        addChosenParent(&_arithmetic, &_arithmetic, copy, _parents, true);
        dcList_push(_results, dcList_createNodeWithObjects(copy, NULL));
    }
    else
    {
        choose(arithmetic,
               _symbol,
               arithy->values,
               arithy->values->head,
               _count,
               _count,
               _chooseType,
               _results,
               _parents,
               NULL,
               NULL);
    }

    dcListElement *that;

    for (that = _results->head; that != NULL; )
    {
        dcListElement *next = that->next;

        if (CAST_LIST(that->object)->size != _count)
        {
            dcList_removeElement(_results, &that, DC_DEEP);
        }

        that = next;
    }

    if (dcLog_isEnabled(FLAT_ARITHMETIC_CHOOSE_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_CHOOSE_LOG,
                  "chose '%s' from '%s'\n",
                  dcList_display(_results),
                  dcNode_display(_arithmetic));
    }

    if (freeArithmetic)
    {
        // shift off arithmetic
        dcList_shift(CAST_FLAT_ARITHMETIC(arithmetic)->values, DC_SHALLOW);
        dcNode_free(&arithmetic, DC_DEEP);
    }
}

static bool substituteWithCanceling(dcNode **_top,
                                    dcNode **_bottom,
                                    dcNode *_substitution,
                                    const char *_symbol,
                                    const char *_substituteSymbol)
{
    addCount("substituteWithCanceling");

    dcNode *top = dcNode_copy(*_top, DC_DEEP);
    dcNode *bottom = dcNode_copy(*_bottom, DC_DEEP);

    top = dcFlatArithmetic_multiFactor(top, NULL);
    bottom = dcFlatArithmetic_multiFactor(bottom, NULL);

    if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                  "after multi-factor, top: %s bottom: %s\n",
                  dcNode_display(top),
                  dcNode_display(bottom));
    }

    bool result = false;
    dcNode *substitution = dcNode_copy(_substitution, DC_DEEP);
    bool canceled = false;
    bool localCanceled = false;
    size_t safety = 0;

    do
    {
        localCanceled = dcFlatArithmetic_cancelTopAndBottom(&top,
                                                            &bottom,
                                                            true);

        SHRINK_OPERATION(snip, top, NULL);
        SHRINK_OPERATION(snip, bottom, NULL);

        if (localCanceled)
        {
            canceled = true;
        }

        assert(++safety < 100);
    } while (localCanceled);

    if (canceled)
    {
        top = dcFlatArithmetic_shrink(top, NULL);
        bottom = dcFlatArithmetic_shrink(bottom, NULL);

        if (! dcFlatArithmetic_findIdentifier(&bottom,
                                              _symbol,
                                              NULL))
        {
            if (dcFlatArithmetic_findIdentifier(&top,
                                                _symbol,
                                                NULL))
            {
                if (dcFlatArithmetic_remove(&top, substitution, _symbol))
                {
                    top = (CREATE_MULTIPLY
                           (top,
                            dcIdentifier_createNode(_substituteSymbol,
                                                    NO_FLAGS),
                            (CREATE_DIVIDE
                             (createNumber(1),
                              dcNode_copy(bottom, DC_DEEP),
                              NULL)),
                            NULL));
                    result = true;
                }
            }
            else
            {
                // we're done
                result = true;
            }
        }
    }

    dcNode_free(&substitution, DC_DEEP);

    if (result)
    {
        dcNode_free(_top, DC_DEEP);
        *_top = top;
        dcNode_free(_bottom, DC_DEEP);
        *_bottom = bottom;
    }
    else
    {
        // :(
        dcNode_free(&top, DC_DEEP);
        dcNode_free(&bottom, DC_DEEP);
    }

    return result;
}

bool dcFlatArithmetic_substituteWithCanceling(dcNode **_program,
                                              dcNode **_derived,
                                              dcNode *_substitution,
                                              const char *_symbol,
                                              const char *_substituteSymbol)
{
    addCount("dcFlatArithmetic_substituteWithCanceling");

    if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                  "substitute with canceling %s and %s\n",
                  dcNode_display(*_program),
                  dcNode_display(*_derived));
    }

    bool result = false;

    if (substituteWithCanceling(_program,
                                _derived,
                                _substitution,
                                _symbol,
                                _substituteSymbol))
    {
        result = true;
    }
    else if (substituteWithCanceling(_derived, // switcheroo
                                     _program,
                                     _substitution,
                                     _symbol,
                                     _substituteSymbol))
    {
        result = true;
    }

    if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                  "result: %u, now %s and %s\n",
                  result,
                  dcNode_display(*_program),
                  dcNode_display(*_derived));
    }

    return result;
}

bool dcFlatArithmetic_unfindableSubstitution(dcNode *_program,
                                             const char *_symbol)
{
    addCount("dcFlatArithmetic_unfindableSubstitution");

    bool result = false;
    dcNode *shrunken = dcFlatArithmetic_shrink(dcNode_copy(_program, DC_DEEP),
                                               NULL);

    if (shrunken != NULL && IS_DIVIDE(shrunken))
    {
        assert(FLAT_ARITHMETIC_SIZE(shrunken) == 2);
        dcList *symbols = (dcList_createWithObjects
                           (dcIdentifier_createNode(_symbol, NO_FLAGS),
                            NULL));
        int32_t topDegree = 0;
        int32_t bottomDegree = 0;

        if (dcFlatArithmetic_shrunkenDegree
            (FLAT_ARITHMETIC_HEAD(shrunken), symbols, &topDegree))
        {
            if (dcFlatArithmetic_shrunkenDegree
                (FLAT_ARITHMETIC_TAIL(shrunken), symbols, &bottomDegree))
            {
                if (abs(topDegree - bottomDegree) != 1)
                {
                    result = true;
                }
            }
        }

        dcList_free(&symbols, DC_DEEP);
    }

    dcNode_free(&shrunken, DC_DEEP);
    return result;
}

dcNode *dcFlatArithmetic_substitute(dcNode *_program,
                                    const char *_symbol,
                                    const char *_substituteSymbol,
                                    dcNode **_substitution)
{
    addCount("dcFlatArithmetic_substitute");

    if (dcFlatArithmetic_unfindableSubstitution(_program, _symbol))
    {
        return NULL;
    }

    dcNode *program = dcNode_copy(_program, DC_DEEP);

    if (IS_METHOD_CALL(program)
        || (! IS_MULTIPLY(program)
            && ! IS_DIVIDE(program)))
    {
        program = CREATE_MULTIPLY(program, createNumber(1), NULL);
    }

    uint32_t i;
    dcNode *result = NULL;

    SHRINK_OPERATION(convertDivideToMultiply, program, NULL);
    SHRINK_OPERATION(merge, program, NULL);

    dcFlatArithmetic *arithy = CAST_FLAT_ARITHMETIC(program);
    bool success = false;

    for (i = 1; i <= arithy->values->size - 1; i++)
    {
        //
        // get all combinations that are 'i' long
        //

        dcList *chooseResults = dcList_create();
        dcHash *parents = dcHash_create();
        dcFlatArithmetic_choose(program,
                                _symbol,
                                i,
                                REMOVE_FOR_SUBSTITUTION,
                                chooseResults,
                                parents);
        dcListElement *that;

        for (that = chooseResults->head; that != NULL; that = that->next)
        {
            //
            // cycle through each combination and try to remove the
            // derivation of it
            //

            dcNode *multy = (dcFlatArithmetic_createNodeWithList
                             (TAFFY_MULTIPLY,
                              dcList_copy(CAST_LIST(that->object), DC_DEEP)));

            if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                          "trying to substitute: '%s' from: '%s'\n",
                          dcNode_display(multy),
                          dcNode_display(program));

                dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                          "trying to derive: '%s'\n",
                          dcNode_display(multy));
            }

            dcNode *derivy = dcFlatArithmetic_derive(multy, _symbol);

            if (derivy == NULL)
            {
                dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                          "can't derive it :(\n");
                dcNode_free(&multy, DC_DEEP);
                continue;
            }
            else if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                          "derived it to: '%s'\n",
                          dcNode_display(derivy));
            }

            dcNode *parentBackup = NULL;
            dcNode *programCopy = NULL;
            Chosen *chosen = NULL;

            TAFFY_DEBUG(char *programDisplay = NULL);

            if (i == 1)
            {
                dcNode *chosenNode;
                assert(dcHash_getValueWithKeys
                       (parents,
                        NULL,
                        (dcHashType)(size_t)dcList_getHead
                        (CAST_LIST(that->object)),
                        &chosenNode));
                assert(chosenNode != NULL);
                chosen = (Chosen *)CAST_VOID(chosenNode);
                parentBackup = *chosen->parent;

                if (chosen->subObject)
                {
                    if (IS_METHOD_CALL(*chosen->parent))
                    {
                        dcNode *call = dcNode_copy(*chosen->parent, DC_DEEP);
                        dcNode **argument = getMethodArgument(call, 0, true);
                        assert(argument != NULL);
                        dcNode_free(argument, DC_DEEP);
                        *argument = dcIdentifier_createNode(_substituteSymbol,
                                                            NO_FLAGS);
                        *chosen->parent = call;
                        programCopy = dcNode_copy(program, DC_DEEP);
                    }
                    else if (IS_RAISE(*chosen->parent))
                    {
                        dcNode *raise = dcNode_copy(*chosen->parent, DC_DEEP);
                        dcNode *head = dcList_shift
                            (CAST_FLAT_ARITHMETIC(raise)->values, DC_SHALLOW);
                        *chosen->parent =
                            CREATE_RAISE(head,
                                         dcIdentifier_createNode
                                         (_substituteSymbol, NO_FLAGS),
                                         NULL);
                        programCopy = dcNode_copy(program, DC_DEEP);
                        dcNode_free(&raise, DC_DEEP);
                    }
                    else
                    {
                        assert(false);
                    }
                }
                else
                {
                    *chosen->parent =
                        dcIdentifier_createNode(_substituteSymbol, NO_FLAGS);
                    programCopy = dcNode_copy(program, DC_DEEP);
                }
            }
            else
            {
                // i != 1
                programCopy = dcNode_copy(program, DC_DEEP);

                if (dcFlatArithmetic_remove(&programCopy, multy, _symbol))
                {
                    programCopy = popIfNotFactorial(programCopy, NULL);
                }
            }

            dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                      "composed result: %s, now going to substitute\n",
                      dcNode_display(programCopy));

            TAFFY_DEBUG(programDisplay =
                        dcNode_synchronizedDisplay(programCopy));

            if (programCopy != NULL
                && ! dcFlatArithmetic_findIdentifier(&derivy,
                                                     _symbol,
                                                     NULL)
                && ! dcFlatArithmetic_findIdentifier(&programCopy,
                                                     _symbol,
                                                     NULL))
            {
                dcNode *right = CREATE_DIVIDE(createNumber(1),
                                              dcNode_copy(derivy, DC_DEEP),
                                              NULL);

                if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
                {
                    dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                              "can't find symbol %s in either (good!), "
                              "now multiplying %s with %s ",
                              _symbol,
                              dcNode_display(programCopy),
                              dcNode_display(right));
                }

                programCopy = CREATE_MULTIPLY(programCopy, right, NULL);

                if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
                {
                    dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                              "makes %s\n",
                              dcNode_display(programCopy));
                }

                success = true;
                result = programCopy;
            }
            else if (programCopy != NULL
                     && IS_FLAT_ARITHMETIC(programCopy)
                     && dcFlatArithmetic_remove(&programCopy,
                                                derivy,
                                                _symbol))
            {
                TAFFY_DEBUG
                    (dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                               "removed '%s' from '%s' makes: '%s'\n",
                               dcNode_display(derivy),
                               programDisplay,
                               dcNode_display(programCopy)));

                if (programCopy != NULL)
                {
                    if (dcFlatArithmetic_findIdentifier(&programCopy,
                                                        _symbol,
                                                        NULL))
                    {
                        dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                                  "not good enough, '%s' still exists\n",
                                  _symbol);
                    }
                    else
                    {
                        success = true;
                        result = programCopy;
                    }
                }
            }
            // try and cancel the derivative out
            else if (programCopy != NULL
                     && IS_FLAT_ARITHMETIC(programCopy)
                     && (dcFlatArithmetic_substituteWithCanceling
                         (&programCopy,
                          &derivy,
                          multy,
                          _symbol,
                          _substituteSymbol)))
            {
                success = true;
                result = programCopy;
            }
            else
            {
                dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                          "couldn't find a match '%s' from '%s'\n",
                          dcNode_display(derivy),
                          dcNode_display(programCopy));
            }

            TAFFY_DEBUG(dcMemory_free(programDisplay));

            if (! success
                && chosen != NULL
                && dcNumberClass_isMe(derivy))
            {
                //
                // try to subtract
                //
                dcNode *parentSave = *chosen->parent;
                *chosen->parent = createNumber(1);

                if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
                {
                    dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                              "trying subtract: '%s' - '%s'\n",
                              dcNode_display(program),
                              dcNode_display(multy));
                }

                dcNode *subtract =
                    dcFlatArithmetic_shrink
                    (CREATE_SUBTRACT(dcNode_copy(program, DC_DEEP),
                                     dcNode_copy(multy, DC_DEEP),
                                     NULL),
                     NULL);

                if (dcNumberClass_isMe(subtract))
                {
                    if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
                    {
                        dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                                  "result is an integer (success): '%s'\n",
                                  dcNode_display(subtract));
                    }

                    // success!
                    success = true;
                    dcNode *value = (dcFlatArithmetic_shrink
                                     (CREATE_ADD
                                      (createIdentifier(_substituteSymbol),
                                       subtract,
                                       NULL),
                                      NULL));
                    dcNode_free(chosen->parent, DC_DEEP);
                    *chosen->parent = parentSave;
                    result = CREATE_MULTIPLY(value,
                                             dcNode_copy
                                             (*chosen->ruler, DC_DEEP),
                                             NULL);
                    dcNode_free(&programCopy, DC_DEEP);

                    if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
                    {
                        dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_LOG,
                                  "result: '%s' substitution: '%s'\n",
                                  dcNode_display(result),
                                  dcNode_display(*_substitution));
                    }
                }
                else
                {
                    dcNode_free(chosen->parent, DC_DEEP);
                    *chosen->parent = parentSave;
                    dcNode_free(&subtract, DC_DEEP);
                    dcNode_free(&programCopy, DC_DEEP);
                }
            }

            dcNode_free(&derivy, DC_DEEP);

            if (chosen != NULL)
            {
                dcNode_free(chosen->parent, DC_DEEP);
                *chosen->parent = parentBackup;
            }

            if (success)
            {
                *_substitution = dcFlatArithmetic_shrink(multy, NULL);
                break;
            }

            dcNode_free(&multy, DC_DEEP);
            dcNode_free(&programCopy, DC_DEEP);
        }

        dcList_free(&chooseResults, DC_DEEP);
        dcHash_free(&parents, DC_DEEP);

        if (success)
        {
            break;
        }
    }

    dcNode_free(&program, DC_DEEP);
    return result;
}

TAFFY_HIDDEN dcNode *substitute(dcNode *_node, const char *_symbol)
{
    addCount("substitute");
    dcNode *result = NULL;
    dcStringCacheElement element = {0};

    if (dcStringCache_getWithSymbol(sSubstituteCache,
                                    _node,
                                    _symbol,
                                    &element))
    {
        if (element.value->type == NODE_VOID)
        {
            result = NULL;
        }
        else
        {
            result = dcNode_copy(element.value, DC_DEEP);
        }

        if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_CACHE_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_CACHE_LOG,
                      "%sgot %s => %s (%s) from cache\n",
                      indent(),
                      dcNode_display(_node),
                      dcNode_display(result),
                      (element.value->type == NODE_VOID
                       ? "no change"
                       : "changed"));
        }

        dcStringCacheElement_free(&element);
        return result;
    }

    dcNode *substitution = NULL;
    char *substituteIdentifier =
        dcLexer_sprintf("__%s_sub__", _symbol);

    result = dcFlatArithmetic_substitute(_node,
                                         _symbol,
                                         substituteIdentifier,
                                         &substitution);

    if (result == NULL)
    {
        dcStringCache_add(sSubstituteCache, &element, NULL);
    }
    else
    {
        if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_CACHE_LOG,
                      "substituting %s for: %s\n",
                      dcNode_display(substitution),
                      substituteIdentifier);
        }

        result = integrate(result, substituteIdentifier, substitute);

        if (result != NULL)
        {
            assert(dcFlatArithmetic_findIdentifier(&result,
                                                   substituteIdentifier,
                                                   substitution));
        }

        dcNode_free(&substitution, DC_DEEP);

        if (dcLog_isEnabled(FLAT_ARITHMETIC_SUBSTITUTION_CACHE_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_SUBSTITUTION_CACHE_LOG,
                      "%sadding %s => %s to cache\n",
                      indent(),
                      element.keyString->string,
                      dcNode_display(result));
        }

        dcStringCache_add(sSubstituteCache, &element, result);
    }

    dcStringCacheElement_free(&element);
    dcMemory_free(substituteIdentifier);
    return result;
}

TAFFY_HIDDEN dcNode *factorAndSubstitute(dcNode *_node, const char *_symbol)
{
    addCount("factor and substitute");

    bool modified = false;
    dcNode *copy = dcFlatArithmetic_multiFactor(dcNode_copy(_node, DC_DEEP),
                                                &modified);
    dcNode *result = NULL;

    dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
              "%sfactored '%s' to '%s' modified: %u\n",
              indent(),
              dcNode_display(_node),
              dcNode_display(copy),
              modified);

    if (copy != NULL && modified)
    {
        SHRINK_OPERATION(snip, copy, NULL);
        result = substitute(copy, _symbol);
    }

    dcNode_free(&copy, DC_DEEP);
    return result;
}

static IntegrationData *getIntegrationData(dcNode *_value);

static void freeIntegrationData(void)
{
    IntegrationData *data = getIntegrationData(NULL);
    dcStringCache_free(&data->history);
    dcList_free(&data->description, DC_DEEP);
    dcNode_free(&data->originalNode, DC_DEEP);
    dcString_free(&data->originalNodeDisplay, DC_DEEP);
    dcNode *me = dcThread_getSelf();
    dcHash_removeValue(sIntegrateHash, me, NULL, DC_DEEP);
    dcNode_free(&me, DC_DEEP);
}

static IntegrationData *getIntegrationData(dcNode *_value)
{
    dcNode *me = dcThread_getSelf();
    dcNode *integrateNode = NULL;
    dcMutex_lock(sIntegrateMutex);
    dcResult hashResult = dcHash_getValue(sIntegrateHash, me, &integrateNode);
    dcMutex_unlock(sIntegrateMutex);
    dcNode_free(&me, DC_DEEP);

    if (hashResult != TAFFY_SUCCESS)
    {
        if (_value == NULL)
        {
            return NULL;
        }

        dcString *display = dcString_create();

        if (dcNode_print(_value, display) == TAFFY_EXCEPTION)
        {
            dcString_free(&display, DC_DEEP);
            return NULL;
        }

        // we've just begun, so create the integration data
        IntegrationData *result =
            (IntegrationData *)(dcMemory_allocateAndInitialize
                                (sizeof(IntegrationData)));
        result->depth = 0;
        result->originalNode = dcNode_copy(_value, DC_DEEP);
        assert(dcNode_hash(_value, &result->originalNodeHashValue)
               == TAFFY_SUCCESS);
        result->originalNodeDisplay = display;
        result->history = dcStringCache_create(true);
        result->description = dcList_create();
        dcHash_setValue(sIntegrateHash,
                        dcThread_getSelf(),
                        dcVoid_createNode(result));
        return result;
    }

    assert(hashResult != TAFFY_EXCEPTION);
    return (hashResult == TAFFY_SUCCESS && integrateNode != NULL
            ? ((IntegrationData *)CAST_VOID(integrateNode))
            : NULL);
}

static bool isFunctionWithArgument(dcNode *_node,
                                   const char * const _names[],
                                   const char *_symbol)
{
    bool result = false;

    if (IS_METHOD_CALL(_node)
        && IS_IDENTIFIER(CAST_METHOD_CALL(_node)->receiver))
    {
        uint32_t i;

        for (i = 0; _names[i] != NULL; i++)
        {
            if (dcIdentifier_equalsString(CAST_METHOD_CALL(_node)->receiver,
                                          _names[i]))
            {
                dcNode **argument = getMethodArgument(_node, 0, true);

                if (argument != NULL
                    && (dcFlatArithmetic_findIdentifier
                        (argument, _symbol, NULL)))
                {
                    result = true;
                    break;
                }
            }
        }
    }

    return result;
}

bool dcFlatArithmetic_isInverseTrigonometric(dcNode *_node, const char *_symbol)
{
    const char * const names[] =
        {
            "asin",
            "acos",
            "atan",
            "acsc",
            "asec",
            "acot",
            NULL
        };

    return isFunctionWithArgument(_node, names, _symbol);
}

bool dcFlatArithmetic_isLogarithmic(dcNode *_node, const char *_symbol)
{
    const char * const names[] = {"ln", "log", NULL};
    return isFunctionWithArgument(_node, names, _symbol);
}

bool dcFlatArithmetic_isAlgebraic(dcNode *_node, const char *_symbol)
{
    bool result = false;

    if (IS_IDENTIFIER(_node) && dcIdentifier_equalsString(_node, _symbol))
    {
        result = true;
    }
    else if (IS_FLAT_ARITHMETIC(_node))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);

        if (arithmetic->taffyOperator == TAFFY_RAISE
            && arithmetic->values->size == 2
            && IS_IDENTIFIER(arithmetic->values->head->object)
            && dcIdentifier_equalsString(arithmetic->values->head->object,
                                         _symbol)
            && (! IS_IDENTIFIER(arithmetic->values->tail->object)
                || ! (dcIdentifier_equalsString
                      (arithmetic->values->tail->object, _symbol))))
        {
            result = true;
        }
    }

    return result;
}

bool dcFlatArithmetic_isTrigonometric(dcNode *_node, const char *_symbol)
{
    const char * const names[] =
        {
            "sin",
            "cos",
            "tan",
            "csc",
            "sec",
            "cot",
            NULL
        };

    return isFunctionWithArgument(_node, names, _symbol);
}

bool dcFlatArithmetic_isExponential(dcNode *_node, const char *_symbol)
{
    bool result = false;

    if (IS_FLAT_ARITHMETIC(_node))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);

        if (arithmetic->taffyOperator == TAFFY_RAISE
            && arithmetic->values->size == 2
            && IS_IDENTIFIER(arithmetic->values->tail->object)
            && dcIdentifier_equalsString(arithmetic->values->tail->object,
                                         _symbol)
            && (! IS_IDENTIFIER(arithmetic->values->head->object)
                || ! dcIdentifier_equalsString(arithmetic->values->head->object,
                                               _symbol)))
        {
            result = true;
        }
    }

    return result;
}

typedef bool (*IlateOrderFunction)(dcNode *_node, const char *_symbol);

// result of true means _left == u, _right == dv
// result of false means _left == dv, _right == u
bool dcFlatArithmetic_orderForIlate(dcNode *_left,
                                    dcNode *_right,
                                    const char *_symbol)
{
    IlateOrderFunction functions[] =
    {
        &dcFlatArithmetic_isInverseTrigonometric,
        &dcFlatArithmetic_isLogarithmic,
        &dcFlatArithmetic_isAlgebraic,
        &dcFlatArithmetic_isTrigonometric,
        &dcFlatArithmetic_isExponential
    };

    size_t indexLeft;
    size_t indexRight;

    for (indexLeft = 0; indexLeft < dcTaffy_countOf(functions); indexLeft++)
    {
        if (functions[indexLeft](_left, _symbol))
        {
            break;
        }
    }

    for (indexRight = 0; indexRight < dcTaffy_countOf(functions); indexRight++)
    {
        if (functions[indexRight](_right, _symbol))
        {
            break;
        }
    }

    bool result = true;

    if (indexLeft != dcTaffy_countOf(functions)
        && indexRight != dcTaffy_countOf(functions))
    {
        if (indexLeft > indexRight)
        {
            result = false;
        }
    }

    return result;
}

static uint8_t getIntegrateDepth(void)
{
    return getIntegrationData(NULL)->depth;
}

// TODO: use me!
//static dcList *getDescription(void)
//{
//    return getIntegrationData()->description;
//}

static const char *indent(void)
{
    if (getIntegrationData(NULL) == NULL)
    {
        return "";
    }

    uint32_t level = getIntegrateDepth();
    // sanity sanity check
    // TODO: fix me
    assert(level <= MAX_INTEGRATE_DEPTH + 1);
    return sIndents[level];
}

dcNode *dcFlatArithmetic_integrateByParts(dcNode *_node, const char *_symbol)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return NULL;
    }

    IntegrationData *integrationData = getIntegrationData(NULL);
    uint32_t depth = integrationData->depth;

    if (integrationData->integrateByPartsDepth >= MAX_INTEGRATE_BY_PARTS_DEPTH)
    {
        if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                      "%s[%u] max integrate by parts depth reached for '%s', "
                      "returning NULL\n",
                      indent(),
                      depth,
                      dcNode_display(_node));
        }

        return NULL;
    }

    integrationData->integrateByPartsDepth++;

    dcFlatArithmetic *arithy = CAST_FLAT_ARITHMETIC(_node);
    uint32_t i;
    dcNode *result = NULL;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                  "%s[%u] trying to integrate '%s' by parts\n",
                  indent(),
                  depth,
                  dcNode_display(_node));
    }

    for (i = 1; result == NULL && i < arithy->values->size; i++)
    {
        if (dcNodeEvaluator_abortReceived(evaluator))
        {
            break;
        }

        dcList *results = dcList_create();
        dcHash *parents = dcHash_create();
        dcFlatArithmetic_choose(_node,
                                _symbol,
                                i,
                                REMOVE_BY_PARTS,
                                results,
                                parents);
        dcListElement *preU;
        size_t j;

        for (preU = results->head, j = 0;
             result == NULL && preU != NULL;
             preU = preU->next, j++)
        {
            if (dcNodeEvaluator_abortReceived(evaluator))
            {
                break;
            }

            dcNode *u = (dcFlatArithmetic_createNodeWithList
                         (TAFFY_MULTIPLY,
                          dcList_copy(CAST_LIST(preU->object), DC_DEEP)));
            dcNode *v = dcNode_copy(_node, DC_DEEP);
            dcNode *uCopy = dcNode_copy(u, DC_DEEP);
            dcNode *vIntegral = NULL;
            dcNode *uDerivative = NULL;
            dcNode *right = NULL;
            dcNode *rightIntegral = NULL;
            char *rightDisplay = NULL;

            if (! dcFlatArithmetic_remove(&v, uCopy, NULL))
            {
                dcNode_free(&u, DC_DEEP);
                dcNode_free(&uCopy, DC_DEEP);
                break;
            }

            assert(v != NULL);
            dcNode_free(&uCopy, DC_DEEP);

            if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                          "%s[%u] chose u: '%s' going to integrate v: '%s'\n",
                          indent(),
                          depth,
                          dcNode_display(u),
                          dcNode_display(v));
            }

            vIntegral = dcFlatArithmetic_integrate(v, _symbol);

            if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                          "%s[%u] integrated v to: '%s'\n",
                          indent(),
                          depth,
                          dcNode_display(vIntegral));

                dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                          "%s[%u] strategy: %s * integral(%s) - "
                          "integral(derivative(%s) * integral(%s))\n",
                          indent(),
                          depth,
                          dcNode_display(u),
                          dcNode_display(v),
                          dcNode_display(u),
                          dcNode_display(v));
            }

            dcNode_free(&v, DC_DEEP);

            if (vIntegral == NULL)
            {
                if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
                {
                    dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                              "%s[%u] can't integrate v\n",
                              indent(),
                              depth);
                }

                goto kickout;
            }

            uDerivative = dcFlatArithmetic_derive(u, _symbol);

            if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                          "%s[%u] derived u to: '%s'\n",
                          indent(),
                          depth,
                          dcNode_display(uDerivative));
            }

            if (uDerivative == NULL)
            {
                goto kickout;
            }

            if (dcFlatArithmetic_find(uDerivative,
                                      integrationData->originalNode))
            {
                if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
                {
                    dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                              "%s[%u] u': '%s' contains left hand side, "
                              "bailing\n",
                              indent(),
                              depth,
                              dcNode_display(uDerivative));
                }

                goto kickout;
            }

            right = CREATE_MULTIPLY(uDerivative,
                                    dcNode_copy(vIntegral, DC_DEEP),
                                    NULL);
            uDerivative = NULL;

            rightDisplay = dcNode_synchronizedDisplay(right);

            if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                          "%s[%u] going to integrate right-hand-side: '%s'\n",
                          indent(),
                          depth,
                          rightDisplay);
            }

            if (dcFlatArithmetic_find(right,
                                      integrationData->originalNode))
            {
                if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
                {
                    dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                              "%s[%u] right: '%s' contains left hand side, "
                              "bailing\n",
                              indent(),
                              depth,
                              dcNode_display(right));
                }

                dcMemory_free(rightDisplay);
                goto kickout;
            }

            rightIntegral = dcFlatArithmetic_integrate(right, _symbol);
            dcNode_free(&right, DC_DEEP);

            if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                          "%s[%u] integrated '%s' to '%s'\n",
                          indent(),
                          depth,
                          rightDisplay,
                          dcNode_display(rightIntegral));
            }

            dcMemory_free(rightDisplay);

            if (rightIntegral == NULL)
            {
                goto kickout;
            }

            result = (CREATE_SUBTRACT
                      (CREATE_MULTIPLY
                       (u, vIntegral, NULL),
                       rightIntegral,
                       NULL));

            if (dcFlatArithmetic_containsIdentifier
                (result,
                 dcIdentifier_getName(sLeftHandSide)))
            {
                if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
                {
                    dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                              "%s[%u] adding %zu %zu '%s' to data\n",
                              indent(),
                              depth,
                              i,
                              j,
                              dcNode_display(result));
                }
            }

        kickout:
            if (result == NULL)
            {
                dcNode_free(&u, DC_DEEP);
                dcNode_free(&v, DC_DEEP);
                dcNode_free(&uCopy, DC_DEEP);
                dcNode_free(&vIntegral, DC_DEEP);
                dcNode_free(&uDerivative, DC_DEEP);
                dcNode_free(&rightIntegral, DC_DEEP);
                dcNode_free(&right, DC_DEEP);
            }
        }

        dcList_free(&results, DC_DEEP);
        dcHash_free(&parents, DC_DEEP);
    }

    integrationData->integrateByPartsDepth--;
    return result;
}

TAFFY_HIDDEN dcNode *integrateByExpanding(dcNode *_node, const char *_symbol)
{
    IntegrationData *integrationData = getIntegrationData(NULL);
    dcNode *result = NULL;

    // try to expand the arithmetic
    if (integrationData->depth == 1)
    {
        bool modified = false;
        dcNode *copy = dcNode_copy(_node, DC_DEEP);
        dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                  "%s[%u] trying to expand: %s\n",
                  indent(),
                  getIntegrateDepth(),
                  dcNode_display(copy));
        copy = dcFlatArithmetic_expand(copy, &modified);

        if (modified)
        {
            dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                      "%s[%u] expanded to: %s\n",
                      indent(),
                      getIntegrateDepth(),
                      dcNode_display(copy));
            result = integrate(copy, _symbol, NULL);
        }
        else
        {
            dcNode_free(&copy, DC_DEEP);
        }
    }

    return result;
}

TAFFY_HIDDEN dcNode *integrateByPartsWithOne(dcNode *_node, const char *_symbol)
{
    dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
              "%s[%u] integrating by parts with one: %s\n",
              indent(),
              getIntegrateDepth(),
              dcNode_display(_node));

    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    if (! (! dcNodeEvaluator_abortReceived(evaluator)
           && (dcGraphData_getType(_node) == NODE_METHOD_CALL
               || (IS_FLAT_ARITHMETIC(_node)
                   && (CAST_FLAT_ARITHMETIC(_node)->taffyOperator
                       != TAFFY_MULTIPLY)))))
    {
        return NULL;
    }

    dcNode *copy = dcNode_copy(_node, DC_DEEP);

    if (! IS_FLAT_ARITHMETIC(copy)
        || CAST_FLAT_ARITHMETIC(copy)->taffyOperator != TAFFY_MULTIPLY
        || CAST_FLAT_ARITHMETIC(copy)->values->size <= 1)
    {
        copy = dcFlatArithmetic_createNodeWithList
            (TAFFY_MULTIPLY, dcList_createWithObjects(copy, NULL));
    }

    dcFlatArithmetic *copyArithmetic = CAST_FLAT_ARITHMETIC(copy);
    dcNode *one = createNumber(1);
    dcList_push(copyArithmetic->values, one);
    dcNode *result = dcFlatArithmetic_integrateByParts(copy, _symbol);
    dcNode_free(&copy, DC_DEEP);
    return result;
}

// move arithmetic containing _symbol to the left, the rest to the right
void dcFlatArithmetic_moveLeftAndRight(dcNode *_arithmetic,
                                       const char *_symbol,
                                       dcNode **_left,
                                       dcNode **_right)
{
    dcNode *arithmetic = dcNode_copy(_arithmetic, DC_DEEP);
    SHRINK_OPERATION(convertSubtractToAdd, arithmetic, NULL);
    SHRINK_OPERATION(convertDivideToMultiply, arithmetic, NULL);

    if (IS_ADD(arithmetic))
    {
        FOR_EACH_IN_NODE(arithmetic, that)
        {
            dcNode *copy = dcNode_copy(that->object, DC_DEEP);

            if (IS_IDENTIFIER(copy)
                && dcIdentifier_equalsString(copy, _symbol))
            {
                *_left = CREATE_ADD(copy, *_left, NULL);
            }
            else
            {
                if (*_right == NULL)
                {
                    *_right = (dcNode_setTemplate
                               (dcNumberClass_createObjectFromInt32u(0),
                                true));
                }

                *_right = CREATE_SUBTRACT(*_right, copy, NULL);
            }
        }
    }
    else if (IS_MULTIPLY(arithmetic))
    {
        FOR_EACH_IN_NODE(arithmetic, that)
        {
            dcNode *copy = dcNode_copy(that->object, DC_DEEP);

            if (IS_IDENTIFIER(copy)
                && dcIdentifier_equalsString(copy, _symbol))
            {
                *_left = CREATE_MULTIPLY(copy, *_left, NULL);
            }
            else
            {
                if (*_right == NULL)
                {
                    *_right = (dcNode_setTemplate
                               (dcNumberClass_createObjectFromInt32u(1),
                                true));
                }

                *_right = CREATE_DIVIDE(*_right, copy, NULL);
            }
        }
    }

    dcNode_free(&arithmetic, DC_DEEP);
}

//
// for subtract:
// if we're 1 subtracts deep, we add neck+ and subtract head
// if we're 2 subtracts deep, we subtract neck+ and add head
// etc
//
// for add:
// if we're 1 subtracts deep, we add
// if we're 2 subtracts deep, we subtract
// etc
//
static bool moveIt(dcFlatArithmetic *_parent,
                   dcListElement *_element,
                   dcNode **_target,
                   size_t _subtractDepth)
{
    bool removeIt = false;
    bool result = true;

    if (IS_FLAT_ARITHMETIC(_element->object))
    {
        CAST_FLAT_ARITHMETIC(_element->object)->grouped = true;
    }

    if (_parent->taffyOperator == TAFFY_ADD)
    {
        removeIt = true;

        if (_subtractDepth % 2 == 0)
        {
            *_target = CREATE_SUBTRACT(*_target, _element->object, NULL);
        }
        else
        {
            *_target = CREATE_ADD(*_target, _element->object, NULL);
        }
    }
    else if (_parent->taffyOperator == TAFFY_MULTIPLY)
    {
        removeIt = true;

        // TODO: remove me
        assert(_subtractDepth == 0);
        *_target = CREATE_DIVIDE(*_target, _element->object, NULL);
    }
    else if (_parent->taffyOperator == TAFFY_SUBTRACT)
    {
        if (_subtractDepth % 2 == 0)
        {
            *_target = CREATE_SUBTRACT(*_target, _element->object, NULL);
        }
        else
        {
            *_target = CREATE_ADD(*_target, _element->object, NULL);
        }

        if (_element == _parent->values->head)
        {
            _element->object = createNumber(0);
        }
        else
        {
            removeIt = true;
        }
    }
    else
    {
        result = false;
    }

    if (removeIt)
    {
        dcList_removeElement(_parent->values, &_element, DC_SHALLOW);
    }

    return result;
}

static bool invertIt(dcNode **_leftHandSide,
                     dcNode **_target,
                     const char *_x)
{
    bool result = false;

    if (IS_FLAT_ARITHMETIC(*_leftHandSide))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(*_leftHandSide);

        if (arithmetic->taffyOperator == TAFFY_MULTIPLY)
        {
            dcListElement *that;
            dcList *bottom = dcList_create();

            for (that = arithmetic->values->head; that != NULL; )
            {
                dcListElement *next = that->next;

                if (! dcFlatArithmetic_containsIdentifier(that->object, _x))
                {
                    dcList_push(bottom, that->object);
                    dcList_removeElement(arithmetic->values, &that, DC_SHALLOW);
                }

                that = next;
            }

            if (bottom->size > 0)
            {
                result = true;
                *_target = (CREATE_DIVIDE
                            (*_target,
                             dcFlatArithmetic_createNodeWithList
                             (TAFFY_MULTIPLY, bottom),
                             NULL));
            }
            else
            {
                dcList_free(&bottom, DC_DEEP);
            }
        }
        // TODO: better
        else if (arithmetic->taffyOperator == TAFFY_RAISE
                 && arithmetic->values->size == 2
                 && dcNumberClass_isMe(arithmetic->values->tail->object))
        {
            result = true;
            dcNode *tailObject = dcList_pop(arithmetic->values, DC_SHALLOW);
            *_target = CREATE_RAISE(*_target,
                                    CREATE_DIVIDE
                                    (createNumber(1),
                                     tailObject,
                                     NULL),
                                    NULL);
        }
    }

    if (result)
    {
        *_leftHandSide = dcFlatArithmetic_shrink(*_leftHandSide, NULL);
        *_target = dcFlatArithmetic_shrink(*_target, NULL);
    }

    return result;
}

static bool newMoveIt(dcNode *_leftHandSide,
                      dcListElement *_that,
                      dcNode **_rightHandSide)
{
    bool doIt = false;

    if (IS_ADD(_leftHandSide))
    {
        *_rightHandSide = CREATE_SUBTRACT(*_rightHandSide, _that->object, NULL);
        doIt = true;
    }
    else if (IS_MULTIPLY(_leftHandSide))
    {
        *_rightHandSide = CREATE_DIVIDE(*_rightHandSide, _that->object, NULL);
        doIt = true;
    }

    if (doIt)
    {
        _that->object = NULL;
        dcList_removeElement(CAST_FLAT_ARITHMETIC(_leftHandSide)->values,
                             &_that,
                             DC_DEEP);
    }

    return doIt;
}

bool dcFlatArithmetic_newSolve(const char *_x,
                               dcNode **_left,
                               dcNode **_right)
{
    bool result = false;
    dcNode *left = dcNode_copy(*_left, DC_DEEP);
    dcNode *right = dcNode_copy(*_right, DC_DEEP);

    SHRINK_OPERATION(shrink, left, NULL);
    SHRINK_OPERATION(shrink, right, NULL);

    SHRINK_OPERATION(convertSubtractToAdd, left, NULL);
    SHRINK_OPERATION(convertSubtractToAdd, right, NULL);

    left = dcFlatArithmetic_factorWithSymbol(left, _x, NULL);
    right = dcFlatArithmetic_factorWithSymbol(right, _x, NULL);

    SHRINK_OPERATION(convertDivideToMultiply, left, NULL);
    SHRINK_OPERATION(convertDivideToMultiply, right, NULL);

    bool modified = false;

    // move all non-_x's to the right
    if (IS_FLAT_ARITHMETIC(left))
    {
        do
        {
            dcListElement *that = NULL;
            modified = false;

            for (that = CAST_FLAT_ARITHMETIC(left)->values->head;
                 that != NULL;
                )
            {
                dcListElement *next = that->next;

                if (! dcFlatArithmetic_containsIdentifier(that->object, _x))
                {
                    modified = newMoveIt(left, that, &right);
                }

                that = next;
            }
        } while (modified);
    }

    if (IS_FLAT_ARITHMETIC(left)
        && CAST_FLAT_ARITHMETIC(left)->values->size == 0)
    {
        dcNode_free(&left, DC_DEEP);
        left = createNumber(0);
    }

    SHRINK_OPERATION(shrink, right, NULL);
    SHRINK_OPERATION(convertSubtractToAdd, right, NULL);
    right = dcFlatArithmetic_factorWithSymbol(right, _x, NULL);
    SHRINK_OPERATION(convertDivideToMultiply, right, NULL);
    SHRINK_OPERATION(merge, right, NULL);

    modified = false;

    // move all _x's to the left
    if (IS_FLAT_ARITHMETIC(right))
    {
        do
        {
            dcListElement *that = NULL;
            modified = false;

            for (that = CAST_FLAT_ARITHMETIC(right)->values->head;
                 that != NULL;
                )
            {
                dcListElement *next = that->next;

                if (dcFlatArithmetic_containsIdentifier(that->object, _x))
                {
                    modified = newMoveIt(right, that, &left);
                }

                that = next;
            }
        } while (modified);
    }

    if (IS_FLAT_ARITHMETIC(right)
        && CAST_FLAT_ARITHMETIC(right)->values->size == 0)
    {
        dcNode_free(&left, DC_DEEP);
        left = createNumber(0);
    }

    if (dcFlatArithmetic_containsIdentifier(left, _x)
        && ! dcFlatArithmetic_containsIdentifier(right, _x))
    {
        result = true;
        dcNode_free(_left, DC_DEEP);
        dcNode_free(_right, DC_DEEP);
        *_left = left;
        *_right = right;
        SHRINK_OPERATION(shrink, *_left, NULL);
        SHRINK_OPERATION(shrink, *_right, NULL);
    }
    else
    {
        result = false;
        dcNode_free(&left, DC_DEEP);
        dcNode_free(&right, DC_DEEP);
    }

    return result;
}

dcNode *dcFlatArithmetic_solve(const char *_x,
                               dcNode *_leftHandSide,
                               dcNode *_rightHandSide)
{
    // move all X-values to the left-hand-side
    // move all non-X-values to the right-hand-side
    dcNode *result = NULL;
    dcNode *rightHandSide = dcNode_copy(_rightHandSide, DC_DEEP);
    dcNode *leftHandSide = dcNode_copy(_leftHandSide, DC_DEEP);

    dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
              "%sconverted RHS to: [%s]\n",
              indent(),
              dcNode_display(rightHandSide));

    // move all non-_x's to the right
    if (IS_FLAT_ARITHMETIC(leftHandSide))
    {
        bool modified = false;

        do
        {
            modified = false;
            leftHandSide = dcFlatArithmetic_shrink(leftHandSide, NULL);

            if (IS_FLAT_ARITHMETIC(leftHandSide))
            {
                dcFlatArithmetic *arithmetic =
                    CAST_FLAT_ARITHMETIC(leftHandSide);
                dcListElement *that;

                for (that = arithmetic->values->head;
                     that != NULL && rightHandSide != NULL;
                    )
                {
                    dcListElement *next = that->next;

                    if (! dcFlatArithmetic_containsIdentifier(that->object, _x))
                    {
                        modified = moveIt(arithmetic, that, &rightHandSide, 0);
                    }

                    that = next;
                }
            }
        } while (modified);
    }

    //
    // move all _x's to the left
    //

    typedef struct
    {
        dcNode **value;
        uint32_t subtractDepth;
    } ArithmeticPath;

    dcList *frontier = dcList_create();
    ArithmeticPath *headPath =
        (ArithmeticPath *)dcMemory_allocate(sizeof(ArithmeticPath));
    headPath->value = &rightHandSide;
    headPath->subtractDepth = 0;
    dcList_push(frontier, dcVoid_createNode(headPath));

    while (frontier->size > 0)
    {
        dcNode *headVoid = dcList_shift(frontier, DC_SHALLOW);
        ArithmeticPath *path = (ArithmeticPath *)CAST_VOID(headVoid);

        *path->value = dcFlatArithmetic_shrink(*path->value, NULL);
        dcNode *head = *path->value;
        dcNode_free(&headVoid, DC_SHALLOW);

        if (IS_FLAT_ARITHMETIC(head))
        {
            head = dcFlatArithmetic_distribute(head, NULL);

            dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(head);
            dcListElement *that;
            uint32_t i;

            for (that = arithmetic->values->head, i = 0;
                 that != NULL && leftHandSide != NULL;
                i++)
            {
                dcListElement *next = that->next;

                if ((arithmetic->taffyOperator == TAFFY_ADD
                     || arithmetic->taffyOperator == TAFFY_SUBTRACT)
                    && IS_FLAT_ARITHMETIC(that->object)
                    && ((CAST_FLAT_ARITHMETIC(that->object)->taffyOperator
                         == TAFFY_ADD)
                        || (CAST_FLAT_ARITHMETIC(that->object)->taffyOperator
                            == TAFFY_SUBTRACT)))
                {
                    ArithmeticPath *newPath =
                        (ArithmeticPath *)(dcMemory_allocate
                                           (sizeof(ArithmeticPath)));
                    newPath->value = &that->object;
                    newPath->subtractDepth = path->subtractDepth;

                    if (arithmetic->taffyOperator == TAFFY_SUBTRACT
                        && i >= 1)
                    {
                        newPath->subtractDepth++;
                    }

                    dcList_unshift(frontier, dcVoid_createNode(newPath));
                }
                else if (dcFlatArithmetic_containsIdentifier
                         (that->object, _x))
                {
                    moveIt(arithmetic,
                           that,
                           &leftHandSide,
                           ((arithmetic->taffyOperator == TAFFY_SUBTRACT
                             && i >= 1)
                            ? path->subtractDepth + 1
                            : path->subtractDepth));
                }

                that = next;
            }
        }

        dcMemory_free(path);
    }

    dcList_free(&frontier, DC_DEEP);

    if (leftHandSide != NULL)
    {
        leftHandSide = dcFlatArithmetic_shrink(leftHandSide, NULL);

        if ((identifierEquals(leftHandSide, _x)
             && ! dcFlatArithmetic_containsIdentifier(rightHandSide, _x))
            || invertIt(&leftHandSide, &rightHandSide, _x))
        {
            // success!
            result = rightHandSide;
        }
    }

    if (result == NULL)
    {
        //dcNode_free(&rightHandSide, DC_DEEP);
    }
    else
    {
        result = dcFlatArithmetic_shrink(result, NULL);
        TAFFY_DEBUG(assert
                    (! dcFlatArithmetic_containsIdentifier(result, _x)));
    }

    dcNode_free(&leftHandSide, DC_DEEP);
    return dcFlatArithmetic_shrink(result, NULL);
}

TAFFY_HIDDEN dcNode *integrateWithLanguages(dcNode *_node, const char *_symbol)
{
    return calculusOperation(_node, _symbol, sIntegrateLanguages, NULL);
}

TAFFY_HIDDEN dcNode *integrateWithExpand(dcNode *_node, const char *_symbol)
{
    dcNode *copy = dcNode_copy(_node, DC_DEEP);
    copy = expandOperations(copy);
    dcNode *result = integrateWithLanguages(copy, _symbol);
    dcNode_free(&copy, DC_DEEP);
    return result;
}

TAFFY_HIDDEN dcNode *integrateWithDivision(dcNode *_node, const char *_symbol)
{
    dcNode *result = NULL;

    if (IS_DIVIDE(_node) && FLAT_ARITHMETIC_SIZE(_node) == 2)
    {
        dcList *symbols = (dcList_createWithObjects
                           (dcIdentifier_createNode(_symbol, NO_FLAGS),
                            NULL));
        dcNode *quotient = NULL;
        dcNode *remainder = NULL;
        bool divideResult =
            dcFlatArithmetic_dividePolynomials(FLAT_ARITHMETIC_HEAD(_node),
                                               FLAT_ARITHMETIC_TAIL(_node),
                                               symbols,
                                               &quotient,
                                               &remainder);
        dcList_free(&symbols, DC_DEEP);

        if (divideResult)
        {
            dcNode *newNode =
                (CREATE_ADD
                 (quotient,
                  CREATE_MULTIPLY
                  (remainder,
                   CREATE_DIVIDE
                   (createNumber(1),
                    dcNode_copy(FLAT_ARITHMETIC_TAIL(_node), DC_DEEP),
                    NULL),
                   NULL),
                  NULL));

            if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                          "divided polynomial '%s' into '%s'\n",
                          dcNode_display(_node),
                          dcNode_display(newNode));
            }

            result = integrate(newNode, _symbol, NULL);
        }
        else
        {
            dcNode_free(&quotient, DC_DEEP);
            dcNode_free(&remainder, DC_DEEP);
        }
    }

    return result;
}

TAFFY_HIDDEN dcNode *integrateWithConversion(dcNode *_node, const char *_symbol)
{
    dcNode *copy = dcNode_setTemplate(dcNode_copy(_node, DC_DEEP), true);
    dcNode *result =
        calculusOperation(copy, "", sIntegrateSimplificationLanguages, NULL);

    if (result != NULL)
    {
        dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                  "simplified %s to %s and trying to integrate again\n",
                  dcNode_display(_node),
                  dcNode_display(result));
        result = dcFlatArithmetic_shrink(result, NULL);
        result = integrate(result, _symbol, NULL);
    }

    dcNode_free(&copy, DC_DEEP);
    return result;
}

bool dcFlatArithmetic_unfindableIntegral(dcNode *_node,
                                         const char *_symbol,
                                         bool _search)
{
    if (_search && IS_MULTIPLY(_node))
    {
        FOR_EACH_IN_NODE(_node, value)
        {
            if (dcFlatArithmetic_unfindableIntegral(value->object,
                                                    _symbol,
                                                    _search))
            {
                return true;
            }
        }
    }

    bool result = false;
    dcList *symbols = (dcList_createWithObjects
                       (dcIdentifier_createNode(_symbol, NO_FLAGS),
                        NULL));

    // is it e^x^n where n > 1
    if (IS_RAISE(_node)
        && FLAT_ARITHMETIC_SIZE(_node) == 2 // needed?
        && IS_IDENTIFIER(FLAT_ARITHMETIC_HEAD(_node))
        && dcIdentifier_equalsString(FLAT_ARITHMETIC_HEAD(_node), "e"))
    {
        int32_t degree = 0;
        dcNode *tail = FLAT_ARITHMETIC_TAIL(_node);

        result = (! dcFlatArithmetic_degree(tail, symbols, &degree)
                  || degree != 1);
        dcList_free(&symbols, DC_DEEP);
    }

    // f(Z) where Z is a polynomial with degree != 1
    if (IS_METHOD_CALL(_node)
        && CAST_METHOD_CALL(_node)->arguments->size == 1)
    {
        int32_t degree;
        dcNode **argument = getMethodArgument(_node, 0, true);
        result = (dcFlatArithmetic_shrunkenDegree(*argument, symbols, &degree)
                  && degree != 1);
    }

    dcList_free(&symbols, DC_DEEP);
    return result;
}

static void copyOverResult(dcNode **_node, dcNode *_evaluated)
{
    dcNode *copy = dcNode_copy(_evaluated, DC_DEEP);
    dcNode_free(_node, DC_DEEP);
    *_node = dcNode_setTemplate(copy, true);
}

static bool isReservedName(const dcNode *_identifier)
{
    const char *name = dcIdentifier_getName(_identifier);
    bool result = false;
    uint8_t i;

    // TODO: fix me, make a hash
    const char *noEvaluateNames[] =
        {
            "sin",
            "cos",
            "tan",
            "csc",
            "cot",
            "sec",
            "asin",
            "acos",
            "atan",
            "acsc",
            "asec",
            "acot",
            "sinh",
            "cosh",
            "tanh",
            "csch",
            "sech",
            "coth",
            "asinh",
            "acosh",
            "atanh",
            "acsch",
            "asech",
            "acoth",
            "abs",
            "ceiling",
            "choose",
            "e",
            "eye",
            "floor",
            "i",
            "id",
            "lcm",
            "ln",
            "lg",
            "log",
            "log10"
            "max",
            "min",
            "mod",
            "PI",
            "rand",
            "sqrt",
        };

    for (i = 0; i < dcTaffy_countOf(noEvaluateNames); i++)
    {
        if (strcmp(noEvaluateNames[i], name) == 0)
        {
            result = true;
            break;
        }
    }

    return result;
}

static dcResult compile(dcNode **_node,
                        dcList *_symbols,
                        bool _forceFindIdentifier,
                        bool *_changed)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    // want to track for stack overflow
    dcNodeEvaluator_pushCallStackPosition(evaluator);

    dcResult result = TAFFY_SUCCESS;

    if (evaluator->callStack->size > evaluator->maxStackDepth)
    {
        dcStackOverflowExceptionClass_throwObject();
        result = TAFFY_EXCEPTION;
        goto kickout;
    }

    if (IS_GRAPH_DATA_LIST(*_node))
    {
        dcListElement *that;
        dcNode *thatResult = NULL;

        for (that = CAST_GRAPH_DATA_LIST(*_node)->head;
             that != NULL;
             that = that->next)
        {
            dcMethodCall *methodCall = CAST_METHOD_CALL(that->object);

            if (compile(&methodCall->receiver,
                        _symbols,
                        _forceFindIdentifier,
                        _changed)
                == TAFFY_EXCEPTION)
            {
                result = TAFFY_EXCEPTION;
                goto kickout;
            }

            dcNode *receiver =
                dcNodeEvaluator_evaluate(evaluator, that->object);

            if (receiver == NULL)
            {
                result = TAFFY_EXCEPTION;
                goto kickout;
            }

            thatResult = receiver;

            if (that->next != NULL)
            {
                CAST_METHOD_CALL(that->next->object)->receiver = thatResult;
            }
        }

        copyOverResult(_node, thatResult);
    }
    else if (IS_METHOD_CALL(*_node))
    {
        uint32_t i;
        dcListElement *that;
        dcMethodCall *call = CAST_METHOD_CALL(*_node);
        dcNode *receiver = call->receiver;
        dcNode *evaluated = NULL;

        for (that = call->arguments->head; that != NULL; that = that->next)
        {
            if (compile(&that->object, _symbols, true, _changed)
                == TAFFY_EXCEPTION)
            {
                result = TAFFY_EXCEPTION;
                goto kickout;
            }
        }

        if (IS_IDENTIFIER(receiver) && isReservedName(receiver))
        {
            result = TAFFY_SUCCESS;
            goto kickout;
        }

        if (IS_METHOD_CALL(receiver))
        {
            if (compile(&CAST_METHOD_CALL(receiver)->receiver,
                        _symbols,
                        true,
                        _changed)
                == TAFFY_EXCEPTION)
            {
                result = TAFFY_EXCEPTION;
                goto kickout;
            }
        }

        if (compile(&call->receiver, _symbols, true, _changed)
            == TAFFY_EXCEPTION)
        {
            result = TAFFY_EXCEPTION;
            goto kickout;
        }

        evaluated = CAST_METHOD_CALL(*_node)->receiver;
        setModified(_changed);

        if (dcBlockClass_isMe(evaluated))
        {
            evaluated = dcClass_castNodeWithAssert(evaluated,
                                               dcFunctionClass_getTemplate(),
                                               false,
                                               true);
        }

        if (dcFunctionClass_isMe(evaluated))
        {
            dcList *arguments = (dcMethodHeader_getArguments
                                 (dcFunctionClass_getMethodHeader(evaluated)));
            dcNode *body = (dcNode_copy
                            (dcGraphDataTree_getContents
                             (dcFunctionClass_getBody(evaluated)),
                             DC_DEEP));
            dcNode **argument = NULL;

            for (i = 0,
                 that = arguments->head,
                 argument = getMethodArgument(*_node, i, false);
                 argument != NULL;
                 (i++,
                  argument = getMethodArgument(*_node, i, false),
                  that = that->next))
            {
                assert(IS_IDENTIFIER(that->object));
                dcFlatArithmetic_findIdentifier(&body,
                                                dcIdentifier_getName
                                                (that->object),
                                                *argument);
            }

            dcNode_free(_node, DC_DEEP);
            *_node = body;
        }
        else
        {
            copyOverResult(_node, evaluated);
        }
    }
    else if (IS_FLAT_ARITHMETIC(*_node))
    {
        FOR_EACH_IN_NODE(*_node, that)
        {
            if (dcFlatArithmetic_compile(&that->object, _symbols, _changed)
                == TAFFY_EXCEPTION)
            {
                result = TAFFY_EXCEPTION;
                goto kickout;
            }
        }
    }
    else if (IS_IDENTIFIER(*_node)
             && (_forceFindIdentifier
                 || (_symbols != NULL
                     && ! dcList_containsEqual(_symbols, *_node))))
    {
        dcNode *evaluated = NULL;

        if (isReservedName(*_node))
        {
            result = TAFFY_SUCCESS;
            goto kickout;
        }

        evaluated = dcNodeEvaluator_evaluate(evaluator, *_node);

        if (evaluated == NULL)
        {
            result = TAFFY_EXCEPTION;
            goto kickout;
        }

        setModified(_changed);
        copyOverResult(_node, evaluated);
    }
    else if (IS_CLASS(*_node))
    {
        if (dcFunctionClass_isMe(*_node))
        {
            if (dcFunctionClass_compileHelper(*_node, _changed)
                == TAFFY_EXCEPTION)
            {
                result = TAFFY_EXCEPTION;
                goto kickout;
            }
        }
        else if (dcBlockClass_isMe(*_node))
        {
            if (dcFunctionClass_compileHelper(dcClass_getSuperNode(*_node),
                                              _changed)
                == TAFFY_EXCEPTION)
            {
                result = TAFFY_EXCEPTION;
                goto kickout;
            }
        }
        else if (dcArrayClass_isMe(*_node))
        {
            if (dcArrayClass_compileHelper(*_node,
                                           _symbols,
                                           _changed)
                == TAFFY_EXCEPTION)
            {
                result = TAFFY_EXCEPTION;
                goto kickout;
            }
        }
    }

kickout:
    dcNodeEvaluator_popCallStack(evaluator, DC_DEEP);
    return result;
}

dcResult dcFlatArithmetic_compile(dcNode **_node,
                                  dcList *_symbols,
                                  bool *_changed)
{
    bool thisChanged = false;
    dcResult result = compile(_node, _symbols, false, &thisChanged);

    while (thisChanged && result != TAFFY_EXCEPTION)
    {
        setModified(_changed);
        thisChanged = false;
        result = compile(_node, _symbols, false, &thisChanged);
    }

    return result;
}

dcNode *dcFlatArithmetic_integrate(dcNode *_node, const char *_symbol)
{
    dcNode *copy = dcNode_copy(_node, DC_DEEP);

    if (dcFlatArithmetic_compile(&copy, NULL, NULL) == TAFFY_EXCEPTION)
    {
        dcNode_free(&copy, DC_DEEP);
        return NULL;
    }

    dcNode *shrunk = dcFlatArithmetic_shrink(copy, NULL);
    dcNode *result = NULL;

    if (shrunk != NULL)
    {
        result = integrate(shrunk, _symbol, NULL);
    }
    else
    {
        dcNode_free(&copy, DC_DEEP);
    }

    return result;
}

static dcNode *integrateWithMethods(dcNode *_node,
                                    const char *_symbol,
                                    dcNodeEvaluator *_evaluator,
                                    IntegrateOperation _exceptThis,
                                    IntegrationData *_integrationData)
{
    dcNode *shrunkCopy = dcNode_copy(_node, DC_DEEP);
    dcNode *simplified =
        dcFlatArithmetic_convertSubtractToAdd(shrunkCopy, NULL);
    dcNode *result = NULL;

    if (! dcFlatArithmetic_containsIdentifier(simplified, _symbol))
    {
        result = CREATE_MULTIPLY(dcNode_copy(simplified, DC_DEEP),
                                 dcIdentifier_createNode(_symbol, NO_FLAGS),
                                 NULL);
    }
    else if (! dcFlatArithmetic_unfindableIntegral(simplified, _symbol, false))
    {
        const IntegrateOperation operations[] =
            {
                &factorAndSubstitute,
                &integrateWithLanguages,
                &integrateWithExpand,
                &integrateWithDivision,
                &integrateWithConversion,
                &substitute,
                NULL,
                &dcFlatArithmetic_integrateByParts,
                &integrateByPartsWithOne,
                &integrateByExpanding
            };

        if (_integrationData->depth < MAX_INTEGRATE_DEPTH)
        {
            size_t i;

            for (i = 0;
                 (i < dcTaffy_countOf(operations)
                  && ! dcNodeEvaluator_abortReceived(_evaluator)
                  && result == NULL);
                 i++)
            {
                if (operations[i] == NULL)
                {
                    // don't continue to integrate by parts
                    // or by expanding if the arithmetic contains an
                    // unfindable integral
                    if (dcFlatArithmetic_unfindableIntegral(simplified,
                                                            _symbol,
                                                            true))
                    {
                        result = NULL;
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (_exceptThis != NULL)
                {
                    if (operations[i] == _exceptThis
                        || (_exceptThis == substitute
                            && operations[i] == factorAndSubstitute))
                    {
                        continue;
                    }
                }

                dcNode *copy = dcNode_copy(simplified, DC_DEEP);
                result = operations[i](copy, _symbol);
                dcNode_free(&copy, DC_DEEP);
            }
        }
    }

    dcNode_free(&simplified, DC_DEEP);
    return result;
}

static dcNode *integrate(dcNode *_shrunk,
                         const char *_symbol,
                         IntegrateOperation _exceptThis)
{
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    if (dcNodeEvaluator_abortReceived(evaluator))
    {
        return _shrunk;
    }

    dcNode *result = NULL;
    IntegrationData *integrationData = getIntegrationData(NULL);
    dcString *shrunkDisplayed = dcString_create();

    if (dcNode_print(_shrunk, shrunkDisplayed) == TAFFY_EXCEPTION)
    {
        dcString_free(&shrunkDisplayed, DC_DEEP);
        return NULL;
    }

    dcHashType shrunkHashValue = 0;
    assert(dcNode_hash(_shrunk, &shrunkHashValue) == TAFFY_SUCCESS);

    //
    // check if it's the original node
    //
    if (integrationData != NULL
        && shrunkHashValue == integrationData->originalNodeHashValue
        && dcString_equals(shrunkDisplayed,
                           integrationData->originalNodeDisplay))
    {
        dcNode_free(&_shrunk, DC_DEEP);
        dcString_free(&shrunkDisplayed, DC_DEEP);
        return dcNode_copy(sLeftHandSide, DC_DEEP);
    }

    dcStringCacheElement element = {0};
    element.keyString = shrunkDisplayed;
    element.keyHashValue = shrunkHashValue;

    //
    // check if it's cached
    //
    if (dcStringCache_getWithSymbol(sIntegrateCache,
                                    _shrunk,
                                    _symbol,
                                    &element)
        == TAFFY_SUCCESS)
    {
        if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                      "%s%s is cached, returning %s\n",
                      indent(),
                      dcNode_display(_shrunk),
                      dcNode_display(result));
        }

        dcNode_free(&_shrunk, DC_DEEP);
        dcStringCacheElement_free(&element);

        // special check, if it's void we couldn't find an integration
        if (element.value->type == NODE_VOID)
        {
            return NULL;
        }
        else
        {
            // result is still needed in the cache
            return dcNode_copy(element.value, DC_DEEP);
        }
    }

    if (integrationData == NULL)
    {
        // it doesn't exist yet
        integrationData = getIntegrationData(_shrunk);
        assert(integrationData != NULL);
    }

    //
    // check if it's in the history
    //
    if (dcStringCache_get(integrationData->history,
                          NULL,
                          &element)
        == TAFFY_SUCCESS)
    {
        assert(integrationData->depth > 0);

        if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                      "%s%s is in history, bailing\n",
                      indent(),
                      dcNode_display(_shrunk));
        }

        dcStringCacheElement_free(&element);
        dcNode_free(&_shrunk, DC_DEEP);
        return NULL;
    }

    integrationData->depth += 1;
    // add it to the history
    dcStringCache_add(integrationData->history, &element, NULL);

    if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                  "%s[%u] integrating: %s\n",
                  indent(),
                  getIntegrateDepth(),
                  dcNode_display(_shrunk));
    }

    integrationData->calculations++;
    result = integrateWithMethods(_shrunk,
                                  _symbol,
                                  evaluator,
                                  _exceptThis,
                                  integrationData);

    //
    // move all instances of left-hand-side (X) to the left,
    // so we can solve
    //
    while (result != NULL
           && integrationData->depth == 1
           && (dcFlatArithmetic_containsIdentifier
               (result, dcIdentifier_getName(sLeftHandSide))))
    {
        result = dcFlatArithmetic_shrink(result, NULL);
        dcNode *save = result;
        result = dcFlatArithmetic_solve(dcIdentifier_getName(sLeftHandSide),
                                        sLeftHandSide,
                                        result);
        dcNode_free(&save, DC_DEEP);

        if (result == NULL)
        {
            // try again
            dcNode *copy = dcNode_copy(_shrunk, DC_DEEP);
            result = dcFlatArithmetic_integrateByParts(copy, _symbol);
            dcNode_free(&copy, DC_DEEP);
        }
    }

    if (result != NULL)
    {
        result = dcFlatArithmetic_undoConvertSubtractToAdd(result, NULL);
    }

    //
    // add the result to the cache, if appropriate
    //
    if (result == NULL)
    {
        if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                      "%sAdding %s => VOID to cache\n",
                      indent(),
                      dcNode_display(_shrunk));
        }

        // the result could be NULL because we've reached the max
        // integrate-by-parts depth. in this case, don't trust it
        if (integrationData->integrateByPartsDepth
            < MAX_INTEGRATE_BY_PARTS_DEPTH - 1)
        {
            dcStringCache_add(sIntegrateCache, &element, NULL);
        }
    }
    else
    {
        if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                      "%sAdding %s => %s to cache\n",
                      indent(),
                      dcNode_display(_shrunk),
                      dcNode_display(result));
        }

        if (integrationData->depth > 1
            && ! (dcFlatArithmetic_containsIdentifier
                  (result, dcIdentifier_getName(sLeftHandSide))))
        {
            dcStringCache_add(sIntegrateCache, &element, result);
        }
    }

    dcStringCacheElement_free(&element);

    //
    // debug: verify the result doesn't contain left-hand-side
    //
    if (integrationData->depth == 1 && result != NULL)
    {
        TAFFY_DEBUG(assert
                    (! dcFlatArithmetic_containsIdentifier
                     (result, dcIdentifier_getName(sLeftHandSide))));
    }

    if (dcLog_isEnabled(FLAT_ARITHMETIC_INTEGRATION_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_INTEGRATION_LOG,
                  "%s[%u] integral(%s) = %s\n",
                  indent(),
                  getIntegrateDepth(),
                  dcNode_display(_shrunk),
                  (result == NULL
                   ? "NULL"
                   : dcNode_display(result)));
    }

    integrationData->depth -= 1;

    dcStringCache_pop(integrationData->history);
    dcNode_free(&_shrunk, DC_DEEP);

    //
    // if depth is 0 then we're done
    //
    if (integrationData->depth == 0)
    {
        freeIntegrationData();
    }

    return (result == NULL
            ? NULL
            : dcFlatArithmetic_shrink(result, NULL));
}

dcNode *dcFlatArithmetic_derive(dcNode *_node, const char *_symbol)
{
    dcStringCacheElement element = {0};

    if (dcStringCache_getWithSymbol(sDeriveCache, _node, _symbol, &element)
        == TAFFY_SUCCESS)
    {
        dcNode *result = _node;

        if (dcLog_isEnabled(FLAT_ARITHMETIC_DERIVE_CACHE_LOG))
        {
            dcLog_logLine(FLAT_ARITHMETIC_DERIVE_CACHE_LOG,
                          "%sgot %s from cache from %s",
                          indent(),
                          dcNode_display(element.value),
                          element.keyString);
        }

        if (element.value->type != NODE_VOID)
        {
            result = dcNode_copy(element.value, DC_DEEP);
        }

        dcStringCacheElement_free(&element);
        return result;
    }

    dcNode *copy = dcNode_copy(_node, DC_DEEP);
    dcNode *result = NULL;

    if (dcFlatArithmetic_compile(&copy, NULL, NULL) == TAFFY_EXCEPTION)
    {
        dcStringCacheElement_free(&element);
        dcNode_free(&copy, DC_DEEP);
        return NULL;
    }

    SHRINK_OPERATION(shrink, copy, NULL);
    SHRINK_OPERATION(convertSubtractToAdd, copy, NULL);
    result = calculusOperation(copy, _symbol, sDeriveLanguages, NULL);
    dcNode_free(&copy, DC_DEEP);

    if (result != NULL)
    {
        result = dcFlatArithmetic_shrink(result, NULL);
    }

    if (dcLog_isEnabled(FLAT_ARITHMETIC_DERIVE_CACHE_LOG))
    {
        dcLog_logLine(FLAT_ARITHMETIC_DERIVE_CACHE_LOG,
                      "%sadding %s to cache from %s",
                      indent(),
                      dcNode_display(result),
                      element.keyString);
    }

    dcStringCache_add(sFactorCache, &element, result);
    dcStringCacheElement_free(&element);
    return result;
}

typedef struct
{
    dcListElement *element;
    dcList *parent;
} CancelLocation;

// this function may produce exceptions
dcList *dcFlatArithmetic_findForCanceling(dcFlatArithmetic *_arithmetic,
                                          dcNode *_node)
{
    dcListElement *that;
    dcList *result = dcList_create();
    bool failure = false;

    for (that = _arithmetic->values->head; that != NULL; that = that->next)
    {
        if (IS_FLAT_ARITHMETIC(that->object)
            && ((CAST_FLAT_ARITHMETIC(that->object)->taffyOperator
                 == _arithmetic->taffyOperator)
                || (CAST_FLAT_ARITHMETIC(that->object)->taffyOperator
                    == TAFFY_MULTIPLY)))
        {
            dcList *cancels = dcFlatArithmetic_findForCanceling
                (CAST_FLAT_ARITHMETIC(that->object), _node);

            if (cancels == NULL)
            {
                failure = true;
                break;
            }
            else
            {
                dcList_concat(result, cancels, DC_SHALLOW);
                dcList_free(&cancels, DC_SHALLOW);
            }
        }
        else
        {
            dcTaffyOperator operatorResult;
            dcResult compareResult =
                dcNode_compareEqual(that->object, _node, &operatorResult);

            if (compareResult == TAFFY_SUCCESS
                && operatorResult == TAFFY_EQUALS)
            {
                CancelLocation *location =
                    (CancelLocation *)dcMemory_allocate(sizeof(CancelLocation));
                location->element = that;
                location->parent = _arithmetic->values;
                dcList_push(result, dcVoid_createNode(location));
            }
            else if ((compareResult != TAFFY_SUCCESS
                      || operatorResult != TAFFY_EQUALS)
                     && _arithmetic->taffyOperator != TAFFY_MULTIPLY)
            {
                failure = true;
                break;
            }
        }
    }

    if (result->size == 0
        || failure)
    {
        dcList_free(&result, DC_DEEP);
    }

    return result;
}

dcNode *dcFlatArithmetic_compose(const dcNode *_arithmeticNode,
                                 const dcHash *_composements)
{
    dcNode *result = dcNode_copy(_arithmeticNode, DC_DEEP);

    if (! IS_FLAT_ARITHMETIC(result))
    {
        return result;
    }

    FOR_EACH_IN_NODE(result, that)
    {
        // TODO: fix me
        dcNode *composed =
            dcFlatArithmetic_compose(that->object, _composements);
        dcNode_free(&that->object, DC_DEEP);
        that->object = composed;
    }

    dcFlatArithmetic *resultArithmetic = CAST_FLAT_ARITHMETIC(result);

    for (that = resultArithmetic->values->head; that != NULL; that = that->next)
    {
        dcHashIterator *hashIt = dcHashIterator_create(_composements);
        dcNode *composementPair;

        while ((composementPair = dcHashIterator_getNext(hashIt))
               != NULL)
        {
            dcHashElement *element = CAST_HASH_ELEMENT(composementPair);
            assert(! element->key.isNodeKey);
            const char *composementString = element->key.keyUnion.stringKey;
            const dcNode *composement = element->value;

            if (IS_IDENTIFIER(that->object)
                && dcIdentifier_equalsString(that->object, composementString))
            {
                dcNode_free(&that->object, DC_DEEP);
                that->object = dcNode_copy(composement, DC_DEEP);
            }
            else if (IS_METHOD_CALL(that->object))
            {
                // TODO
                assert(false);
            }
        }

        dcHashIterator_free(&hashIt);
    }

    return result;
}

bool dcFlatArithmetic_findIdentifier(dcNode **_node,
                                     const char *_identifier,
                                     dcNode *_replacement)
{
    bool result = false;

    if (IS_IDENTIFIER(*_node)
        && dcIdentifier_equalsString(*_node, _identifier))
    {
        result = true;

        if (_replacement != NULL)
        {
            dcNode_free(_node, DC_DEEP);
            *_node = dcNode_copy(_replacement, DC_DEEP);
        }
    }
    else if (IS_METHOD_CALL(*_node))
    {
        uint32_t i;

        for (i = 0; ; i++)
        {
            dcNode **argument = getMethodArgument(*_node, i, false);

            if (argument == NULL)
            {
                break;
            }

            if (argument != NULL)
            {
                bool thisResult = dcFlatArithmetic_findIdentifier(argument,
                                                                  _identifier,
                                                                  _replacement);
                result = (result || thisResult);
            }
            else
            {
                dcListElement *that;

                for (that = CAST_METHOD_CALL(*_node)->arguments->head;
                     that != NULL;
                     that = that->next)
                {
                    bool thisResult = dcFlatArithmetic_findIdentifier
                        (&that->object, _identifier, _replacement);
                    result = (result || thisResult);
                }
            }
        }
    }
    else if (IS_FLAT_ARITHMETIC(*_node))
    {
        dcListElement *that;

        for (that = CAST_FLAT_ARITHMETIC(*_node)->values->head;
             that != NULL;
             that = that->next)
        {
            bool thatResult = dcFlatArithmetic_findIdentifier(&that->object,
                                                              _identifier,
                                                              _replacement);
            result = (result || thatResult);
        }
    }

    return result;
}

bool dcFlatArithmetic_containsIdentifier(dcNode *_node, const char *_identifier)
{
    return (_node == NULL
            ? false
            : dcFlatArithmetic_findIdentifier(&_node, _identifier, NULL));
}

static dcNode *flipIt(dcNode *_node)
{
    dcNode *result = NULL;

    if (dcNumberClass_isMe(_node))
    {
        result = dcNode_copy(_node, DC_DEEP);
        dcNumberClass_inlineMultiply
            (result, dcNumberClass_getNegativeOneNumberObject());
    }

    return result;
}

typedef struct
{
    dcListElement *parent;
    dcFlatArithmetic *arithmetic;
    dcListElement *element;
    size_t depth;
    bool stop;
} FactorData;

static void addFactorNodes(dcFlatArithmetic *_arithmetic,
                           dcList *_factorDatas,
                           size_t _depth,
                           dcListElement *_parent)
{
    dcListElement *that;

    for (that = _arithmetic->values->head; that != NULL; that = that->next)
    {
        if (IS_MULTIPLY(that->object))
        {
            dcFlatArithmetic *arithy = CAST_FLAT_ARITHMETIC(that->object);
            dcListElement *i;

            for (i = arithy->values->head; i != NULL; i = i->next)
            {
                if (IS_MULTIPLY(i->object))
                {
                    if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
                    {
                        dcLog_log(FLAT_ARITHMETIC_FACTOR_LOG,
                                  "  delving into multiplication: '%s'\n",
                                  dcNode_display(i->object));
                    }

                    addFactorNodes(CAST_FLAT_ARITHMETIC(i->object),
                                   _factorDatas,
                                   _depth + 1,
                                   (_parent == NULL
                                    ? that
                                    : _parent));
                }
                else
                {
                    if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
                    {
                        dcLog_log(FLAT_ARITHMETIC_FACTOR_LOG,
                                  "  adding factor node in multiply: "
                                  "'%s' parent: '%s' depth: %zu\n",
                                  dcNode_display(i->object),
                                  dcNode_display(that->object),
                                  _depth);
                    }

                    FactorData *data = (FactorData *)(dcMemory_allocate
                                                      (sizeof(FactorData)));
                    data->parent = (_parent == NULL ? that : _parent);
                    data->arithmetic = arithy;
                    data->element = i;
                    data->depth = _depth + 1;
                    data->stop = false;
                    dcList_push(_factorDatas, dcVoid_createNode(data));
                }
            }

            // create a stop to breathe
            FactorData *data =
                (FactorData *)(dcMemory_allocateAndInitialize
                               (sizeof(FactorData)));
            data->stop = true;
            dcList_push(_factorDatas, dcVoid_createNode(data));
        }
        else
        {
            if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_FACTOR_LOG,
                          "  adding factor node: '%s' depth: %zu\n",
                          dcNode_display(that->object),
                          _depth);
            }

            FactorData *data = (FactorData *)(dcMemory_allocate
                                              (sizeof(FactorData)));
            data->parent = _parent;
            data->arithmetic = _arithmetic;
            data->element = that;
            data->depth = _depth;
            data->stop = false;
            dcList_push(_factorDatas, dcVoid_createNode(data));
        }
    }
}

// returns the number of factor values that are matched
static uint32_t factorValue(dcFlatArithmetic *_arithmetic,
                            dcNode *_value,
                            bool _remove,
                            dcNodeEvaluator *_evaluator)
{
    if (dcNumberClass_isMe(_value)
        && (dcNumberClass_isOne(_value)
            || dcNumberClass_isNegativeOne(_value)
            || dcNumberClass_isZero(_value)))
    {
        return 0;
    }

    uint32_t result = 0;
    dcNode *negative = flipIt(_value);
    dcList *factored = dcList_create();
    dcList *factorDatas = dcList_create();
    dcListElement *that;

    if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_FACTOR_LOG,
                  "factoring: %s\n",
                  dcNode_display(_value));
    }

    addFactorNodes(_arithmetic, factorDatas, 0, NULL);

    bool breathe = false;

    for (that = factorDatas->head; that != NULL; that = that->next)
    {
        if (dcNodeEvaluator_abortReceived(dcSystem_getCurrentNodeEvaluator()))
        {
            break;
        }

        FactorData *data = (FactorData *)CAST_VOID(that->object);

        if (breathe && data->depth > 0)
        {
            if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_FACTOR_LOG,
                          "  depth: %zu too big to test: '%s'\n",
                          data->depth,
                          dcNode_display(data->element->object));
            }

            continue;
        }

        breathe = false;

        if (data->stop)
        {
            continue;
        }

        bool isPositive = dontCareCompare(data->element->object, _value);
        bool isNegative = false;

        if (! isPositive)
        {
            isNegative = dontCareCompare(data->element->object, negative);
        }

        if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_FACTOR_LOG,
                      "  '%s' vs '%s' positive: %s negative: %s depth: %u\n",
                      dcNode_display(_value),
                      dcNode_display(data->element->object),
                      (isPositive ? "yes" : "no"),
                      (isNegative ? "yes" : "no"),
                      data->depth);
        }

        if (isPositive || isNegative)
        {
            result++;

            if (data->depth > 0)
            {
                breathe = true;
            }

            if (_remove)
            {
                if (data->depth == 0)
                {
                    dcList_removeElement(data->arithmetic->values,
                                         &data->element,
                                         DC_DEEP);
                    dcList_push(factored, createNumber(isPositive
                                                       ? 1
                                                       : -1));
                }
                else
                {
                    dcNode_free(&data->element->object, DC_DEEP);
                    data->element->object = createNumber(isPositive
                                                         ? 1
                                                         : -1);
                    dcList_push(factored, data->parent->object);
                    dcList_removeElement(_arithmetic->values,
                                         &data->parent,
                                         DC_SHALLOW);
                }
            }
        }
        else if (dcNumberClass_isMe(data->element->object)
                 && dcNumberClass_isMe(_value)
                 && dcNumberClass_divides(_value, data->element->object))
        {
            result++;

            if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_FACTOR_LOG,
                          "  '%s' divides '%s'\n",
                          dcNode_display(_value),
                          dcNode_display(data->element->object));
            }

            if (data->depth > 0)
            {
                breathe = true;
            }

            if (_remove)
            {
                dcNumber_divide(dcNumberClass_getNumber(data->element->object),
                                dcNumberClass_getNumber(data->element->object),
                                dcNumberClass_getNumber(_value));

                if (data->depth == 0)
                {
                    dcList_push(factored, data->element->object);
                    dcList_removeElement(data->arithmetic->values,
                                         &data->element,
                                         DC_SHALLOW);
                }
                else
                {
                    dcList_push(factored, data->parent->object);
                    dcList_removeElement(_arithmetic->values,
                                         &data->parent,
                                         DC_SHALLOW);
                }
            }
        }
        else if (IS_RAISE(data->element->object)
                 && FLAT_ARITHMETIC_SIZE(data->element->object) == 2
                 && isGoodNumber(FLAT_ARITHMETIC_TAIL(data->element->object))
                 && (dcNumberClass_lessThanHelper
                     (dcNumberClass_getOneNumberObject(),
                      FLAT_ARITHMETIC_TAIL(data->element->object)))
                 && dontCareCompare(FLAT_ARITHMETIC_HEAD(data->element->object),
                                    _value))
        {
            result++;

            if (data->depth > 0)
            {
                breathe = true;
            }

            if (_remove)
            {
                dcNumberClass_decrement
                    (FLAT_ARITHMETIC_TAIL(data->element->object));

                if (data->depth == 0)
                {
                    dcList_push(factored, data->element->object);
                    dcList_removeElement(data->arithmetic->values,
                                         &data->element,
                                         DC_SHALLOW);
                }
                else
                {
                    dcList_push(factored, data->parent->object);
                    dcList_removeElement(_arithmetic->values,
                                         &data->parent,
                                         DC_SHALLOW);
                }
            }
        }
        else if (IS_RAISE(data->element->object)
                 && IS_RAISE(_value)
                 && FLAT_ARITHMETIC_SIZE(data->element->object) == 2
                 && FLAT_ARITHMETIC_SIZE(_value) == 2
                 && isGoodNumber(FLAT_ARITHMETIC_TAIL(data->element->object))
                 && isGoodNumber(FLAT_ARITHMETIC_TAIL(_value))
                 && (dcNumberClass_lessThanHelper
                     (FLAT_ARITHMETIC_TAIL(_value),
                      FLAT_ARITHMETIC_TAIL(data->element->object)))
                 && dontCareCompare(FLAT_ARITHMETIC_HEAD(data->element->object),
                                    FLAT_ARITHMETIC_HEAD(_value)))
        {
            result++;

            if (data->depth > 0)
            {
                breathe = true;
            }

            if (_remove)
            {
                dcNumberClass_inlineSubtract
                    (FLAT_ARITHMETIC_TAIL(data->element->object),
                     FLAT_ARITHMETIC_TAIL(_value));

                if (data->depth == 0)
                {
                    dcList_push(factored, data->element->object);
                    dcList_removeElement(data->arithmetic->values,
                                         &data->element,
                                         DC_SHALLOW);
                }
                else
                {
                    dcList_push(factored, data->parent->object);
                    dcList_removeElement(_arithmetic->values,
                                         &data->parent,
                                         DC_SHALLOW);
                }
            }
        }
    }

    if (negative != _value)
    {
        dcNode_free(&negative, DC_DEEP);
    }

    if (_remove && result > 1)
    {
        dcList_push(_arithmetic->values,
                    CREATE_MULTIPLY
                    (_value,
                     dcFlatArithmetic_createNodeWithList(TAFFY_ADD, factored),
                     NULL));
    }
    else
    {
        dcList_free(&factored, DC_SHALLOW);
    }

    dcList_free(&factorDatas, DC_DEEP);
    return result;
}

static void addFactorDatas(dcFlatArithmetic *_parent,
                           dcNode *_node,
                           dcList *_factorDatas,
                           const char *_symbol)
{
    if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_FACTOR_LOG,
                  "adding factor value: '%s'\n",
                  dcNode_display(_node));
    }

    if (_symbol != NULL)
    {
        if (IS_IDENTIFIER(_node)
            && dcIdentifier_equalsString(_node, _symbol))
        {
            dcList_push(_factorDatas, dcNode_copy(_node, DC_DEEP));
        }
        // else don't push to factor datas, it's not what we want
    }
    else
    {
        dcList_push(_factorDatas, dcNode_copy(_node, DC_DEEP));
    }

    if (IS_FLAT_ARITHMETIC(_node)
        && (IS_ADD(_node)
            || IS_SUBTRACT(_node)))
    {
        FOR_EACH_IN_NODE(_node, that)
        {
            if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
            {
                dcLog_log(FLAT_ARITHMETIC_FACTOR_LOG,
                          "adding factor value: '%s'\n",
                          dcNode_display(that->object));
            }

            if (dcNumberClass_isMe(that->object)
                || IS_METHOD_CALL(that->object))
            {
                dcList_push(_factorDatas, dcNode_copy(that->object, DC_DEEP));
            }

            if (IS_MULTIPLY(that->object))
            {
                FOR_EACH_IN_NODE(that->object, i)
                {
                    addFactorDatas(CAST_FLAT_ARITHMETIC(that->object),
                                   i->object,
                                   _factorDatas,
                                   _symbol);
                }
            }
            else if (IS_DIVIDE(that->object))
            {
                addFactorDatas(CAST_FLAT_ARITHMETIC(that->object),
                               FLAT_ARITHMETIC_HEAD(that->object),
                               _factorDatas,
                               _symbol);
            }
        }
    }
}

dcList *dcFlatArithmetic_findQuadraticSolution(dcNode *_node)
{
    return NULL;
}

bool dcFlatArithmetic_isPolynomial(dcNode *_node)
{
    bool result = false;

    if (IS_ADD(_node)
        || IS_SUBTRACT(_node)
        || IS_MULTIPLY(_node))
    {
        FOR_EACH_IN_NODE(_node, that)
        {
            result = dcFlatArithmetic_isPolynomial(that->object);

            if (! result)
            {
                break;
            }
        }
    }
    else if (IS_IDENTIFIER(_node))
    {
        result = true;
    }
    else if (dcNumberClass_isMe(_node))
    {
        result = true;
    }
    else if (dcComplexNumberClass_isMe(_node))
    {
        result = true;
    }
    else if (isMethodCall(_node, "#operator(()):", "sqrt"))
    {
        dcNode *argument = *getMethodArgument(_node, 0, true);
        result = (dcNumberClass_isMe(argument)
                  || dcComplexNumberClass_isMe(argument));
    }
    else if (IS_RAISE(_node)
             && FLAT_ARITHMETIC_SIZE(_node) == 2
             && dcNumberClass_isMe(FLAT_ARITHMETIC_TAIL(_node))
             && dcNumberClass_isWholeHelper(FLAT_ARITHMETIC_TAIL(_node))
             && ! dcNumberClass_isNegative(FLAT_ARITHMETIC_TAIL(_node))
             // no recursive raises, please
             && ! IS_RAISE(FLAT_ARITHMETIC_HEAD(_node))
             && IS_IDENTIFIER(FLAT_ARITHMETIC_HEAD(_node)))
    {
        result = true;
    }

    return result;
}

static dcNode *getCoefficient(dcNode *_arithmetic)
{
    dcNode *result = NULL;

    if (dcNumberClass_isMe(_arithmetic))
    {
        result = dcNode_copy(_arithmetic, DC_DEEP);
    }
    else if (IS_MULTIPLY(_arithmetic))
    {
        if (dcNumberClass_isMe(FLAT_ARITHMETIC_HEAD(_arithmetic)))
        {
            result = dcNode_copy(FLAT_ARITHMETIC_HEAD(_arithmetic), DC_DEEP);
        }
        else
        {
            result = createNumber(1);
        }
    }
    else if (IS_RAISE(_arithmetic)
             || IS_IDENTIFIER(_arithmetic))
    {
        result = createNumber(1);
    }
    else
    {
        result = createNumber(0);
    }

    TAFFY_DEBUG(assert(dcNode_isTemplate(result)));
    return result;
}

bool dcFlatArithmetic_hasSingleIdentifier(dcNode *_node, dcNode **_identifier)
{
    bool result = true;

    if (IS_FLAT_ARITHMETIC(_node))
    {
        FOR_EACH_IN_NODE(_node, that)
        {
            if (! dcFlatArithmetic_hasSingleIdentifier(that->object,
                                                       _identifier))
            {
                *_identifier = NULL;
                result = false;
                break;
            }
        }
    }
    else if (IS_METHOD_CALL(_node))
    {
        uint32_t i;
        dcNode **argument = NULL;

        for (i = 0;
             ((argument = getMethodArgument(_node, i, false))
              != NULL);
             i++)
        {
            if (! dcFlatArithmetic_hasSingleIdentifier(*argument, _identifier))
            {
                *_identifier = NULL;
                result = false;
                break;
            }
        }
    }
    else if (IS_IDENTIFIER(_node))
    {
        if (! dcIdentifier_equalsString(_node, "e")
            && ! dcIdentifier_equalsString(_node, "PI")
            && ! dcIdentifier_equalsString(_node, "i"))
        {
            if (*_identifier == NULL)
            {
                *_identifier = _node;
            }
            else if (! dcIdentifier_equals(CAST_IDENTIFIER(_node),
                                           CAST_IDENTIFIER(*_identifier)))
            {
                *_identifier = NULL;
                result = false;
            }
        }
    }

    return result;
}

static int32_t degreeHelper(dcNode *_node)
{
    int32_t degree = 0;
    assert(dcFlatArithmetic_shrunkenDegree(_node, NULL, &degree));
    return degree;
}

static bool verifyReducedValues(dcNode **_left, dcNode **_right)
{
    dcNode **objects[] = {_left, _right};
    size_t i;
    bool success = true;

    for (i = 0; i < dcTaffy_countOf(objects) && success; i++)
    {
        dcNode **object = objects[i];

        if (IS_RAISE(*object))
        {
            if (FLAT_ARITHMETIC_SIZE(*object) != 2
                || ! dcNumberClass_isMe(FLAT_ARITHMETIC_TAIL(*object))
                || ! dcNumberClass_isWholeHelper(FLAT_ARITHMETIC_TAIL(*object)))
            {
                success = false;
            }
            else
            {
                dcNumberClass_chompHelper(FLAT_ARITHMETIC_TAIL(*object));
            }
        }
        else if (dcNumberClass_isMe(*object))
        {
            if (! dcNumberClass_isWholeHelper(*object))
            {
                success = false;
            }
            else
            {
                dcNumberClass_chompHelper(*object);
            }
        }
    }

    return success;
}

dcNode *dcFlatArithmetic_factorDifferenceOfSquares(dcNode *_node,
                                                   bool *_modified)
{
    if (! dcFlatArithmetic_isPolynomial(_node)
        || ! IS_SUBTRACT(_node))
    {
        return _node;
    }

    dcNode *copy = dcNode_copy(_node, DC_DEEP);
    SHRINK_OPERATION(orderPolynomial, copy, NULL);

    if (FLAT_ARITHMETIC_SIZE(copy) != 2
        || ! IS_SUBTRACT(copy)
        || ! IS_RAISE(FLAT_ARITHMETIC_HEAD(copy)))
    {
        dcNode_free(&copy, DC_DEEP);
        return _node;
    }

    dcList *values = CAST_FLAT_ARITHMETIC(copy)->values;
    dcNode *left = dcList_shift(values, DC_FLOATING);
    dcNode *right = dcList_shift(values, DC_FLOATING);
    dcNode *result = _node;
    dcNode_free(&copy, DC_DEEP);

    left = (dcFlatArithmetic_shrink
            (CREATE_RAISE(left,
                          createNumberFromDouble(0.5),
                          NULL),
             NULL));
    right = (dcFlatArithmetic_shrink
             (CREATE_RAISE(right,
                           createNumberFromDouble(0.5),
                           NULL),
              NULL));

    if (verifyReducedValues(&left, &right))
    {
        dcNode_free(&_node, DC_DEEP);
        result = (CREATE_MULTIPLY
                  (CREATE_ADD
                   (dcNode_copy(left, DC_DEEP),
                    dcNode_copy(right, DC_DEEP),
                    NULL),
                   CREATE_SUBTRACT(left, right, NULL),
                   NULL));
        setModified(_modified);
    }
    else
    {
        dcNode_free(&left, DC_DEEP);
        dcNode_free(&right, DC_DEEP);
    }

    return result;
}

dcNode *dcFlatArithmetic_factorDifferenceOfCubes(dcNode *_node, bool *_modified)
{
    if (! dcFlatArithmetic_isPolynomial(_node)
        || (! IS_SUBTRACT(_node)
            && ! IS_ADD(_node)))
    {
        return _node;
    }

    dcNode *copy = dcNode_copy(_node, DC_DEEP);
    SHRINK_OPERATION(orderPolynomial, copy, NULL);

    if (FLAT_ARITHMETIC_SIZE(copy) != 2
        || (! IS_ADD(copy)
            && ! IS_SUBTRACT(copy)))
    {
        dcNode_free(&copy, DC_DEEP);
        return _node;
    }

    dcList *values = CAST_FLAT_ARITHMETIC(copy)->values;
    dcNode *left = dcList_shift(values, DC_FLOATING);
    dcNode *right = dcList_shift(values, DC_FLOATING);
    dcTaffyOperator taffyOperator = CAST_FLAT_ARITHMETIC(copy)->taffyOperator;
    dcNode *result = _node;
    dcNode_free(&copy, DC_DEEP);

    left = (dcFlatArithmetic_shrink
            (CREATE_RAISE(left,
                          CREATE_DIVIDE(createNumber(1),
                                        createNumber(3),
                                        NULL),
                          NULL),
             NULL));
    right = (dcFlatArithmetic_shrink
             (CREATE_RAISE(right,
                           CREATE_DIVIDE(createNumber(1),
                                         createNumber(3),
                                         NULL),
                           NULL),
              NULL));

    if (verifyReducedValues(&left, &right))
    {
        dcNode_free(&_node, DC_DEEP);
        setModified(_modified);

        if (taffyOperator == TAFFY_SUBTRACT)
        {
            result = (CREATE_MULTIPLY
                      (CREATE_SUBTRACT(dcNode_copy(left, DC_DEEP),
                                       dcNode_copy(right, DC_DEEP),
                                       NULL),
                       dcFlatArithmetic_combine
                       (CREATE_ADD
                        (CREATE_RAISE
                         (dcNode_copy(left, DC_DEEP),
                          createNumber(2),
                          NULL),
                         CREATE_MULTIPLY
                         (dcNode_copy(right, DC_DEEP),
                          dcNode_copy(left, DC_DEEP),
                          NULL),
                         CREATE_RAISE(dcNode_copy(right, DC_DEEP),
                                      createNumber(2),
                                      NULL),
                         NULL), // ADD
                        NULL), // combine
                       NULL)); // MULTIPLY
        }
        else
        {
            assert(taffyOperator == TAFFY_ADD);
            result = (CREATE_MULTIPLY
                      (CREATE_ADD(dcNode_copy(left, DC_DEEP),
                                  dcNode_copy(right, DC_DEEP),
                                  NULL),
                       dcFlatArithmetic_combine
                       (CREATE_ADD
                        (CREATE_SUBTRACT
                         (CREATE_RAISE
                          (dcNode_copy(left, DC_DEEP),
                           createNumber(2),
                           NULL),
                          CREATE_MULTIPLY
                          (dcNode_copy(right, DC_DEEP),
                           dcNode_copy(left, DC_DEEP),
                           NULL),
                          NULL), // SUBTRACT
                         CREATE_RAISE
                         (dcNode_copy(right, DC_DEEP),
                          createNumber(2),
                          NULL),
                         NULL), // ADD
                        NULL), // combine
                       NULL)); // MULTIPLY
        }
    }

    dcNode_free(&left, DC_DEEP);
    dcNode_free(&right, DC_DEEP);
    return result;
}

static dcNode *factorQuadratic(dcNode *_node, bool *_modified, bool _neatAnswer)
{
    int32_t degree = 0;
    dcNode *result = _node;
    dcNode *identifier = NULL;
    dcFlatArithmetic *arithmetic = NULL;
    dcNode *a = NULL;
    dcNode *b = NULL;
    dcNode *c = NULL;
    dcNode *value = NULL;
    dcNode *first = NULL;
    dcNode *second = NULL;

    if (! (dcFlatArithmetic_isPolynomial(_node)
           && dcFlatArithmetic_shrunkenDegree(_node, NULL, &degree)
           && degree == 2
           && dcFlatArithmetic_hasSingleIdentifier(_node, &identifier)
           && (IS_ADD(_node) || IS_SUBTRACT(_node))))
    {
        return _node;
    }

    //
    // use the quadratic formula, if we can
    //
    dcNode *node = dcNode_copy(_node, DC_DEEP);
    SHRINK_OPERATION(orderPolynomial, node, NULL);
    SHRINK_OPERATION(convertSubtractToAdd, node, NULL);

    if (dcLog_isEnabled(FLAT_ARITHMETIC_QUADRATIC_LOG))
    {
        dcLog_logLine(FLAT_ARITHMETIC_QUADRATIC_LOG,
                      "%sfactoring quadratic: %s",
                      indent(),
                      dcNode_display(node));
    }

    if (FLAT_ARITHMETIC_SIZE(node) != 3
        || degreeHelper(FLAT_ARITHMETIC_HEAD(node)) != 2
        || degreeHelper(CAST_FLAT_ARITHMETIC
                        (node)->values->head->next->object) != 1
        || degreeHelper(FLAT_ARITHMETIC_TAIL(node)) != 0)
    {
        goto kickout;
    }

    arithmetic = CAST_FLAT_ARITHMETIC(node);
    a = getCoefficient(FLAT_ARITHMETIC_HEAD(node));
    b = getCoefficient(arithmetic->values->head->next->object);
    c = getCoefficient(FLAT_ARITHMETIC_TAIL(node));
    value = (CREATE_SUBTRACT
             (CREATE_RAISE(dcNode_copyAndTemplate(b),
                           createNumber(2),
                           NULL),
              CREATE_MULTIPLY
              (createNumber(4),
               dcNode_copyAndTemplate(a),
               c,
               NULL), // MULTIPLY
              NULL)); // SUBTRACT
    SHRINK_OPERATION(shrink, value, NULL);

    if (! dcNumberClass_isMe(value)
        || dcNumberClass_isNegative(value))
    {
        if (dcLog_isEnabled(FLAT_ARITHMETIC_QUADRATIC_LOG))
        {
            dcLog_logLine(FLAT_ARITHMETIC_QUADRATIC_LOG,
                          "%sbailing, because value is "
                          "not number or is negative: %s",
                          indent(),
                          dcNode_display(value));
        }

        dcNode_free(&value, DC_DEEP);
        dcNode_free(&a, DC_DEEP);
        dcNode_free(&b, DC_DEEP);
        goto kickout;
    }

    value = (dcFlatArithmetic_shrink
             (CREATE_RAISE(value,
                           createNumberFromDouble(0.5),
                           NULL),
              NULL));

    if (_neatAnswer
        && ! dcNumberClass_isWholeHelper(value))
    {
        if (dcLog_isEnabled(FLAT_ARITHMETIC_QUADRATIC_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_QUADRATIC_LOG,
                      "%sbailing, because we want a neat answer, "
                      "and value is not whole: %s",
                      indent(),
                      dcNode_display(value));
        }

        dcNode_free(&value, DC_DEEP);
        dcNode_free(&a, DC_DEEP);
        dcNode_free(&b, DC_DEEP);
        goto kickout;
    }

    first = (CREATE_DIVIDE
             (CREATE_ADD
              // -b
              (CREATE_MULTIPLY
               (createNumber(-1),
                dcNode_copyAndTemplate(b),
                NULL), // MULTIPLY
               dcNode_copyAndTemplate(value),
               NULL), // ADD
              CREATE_MULTIPLY
              (createNumber(2),
               dcNode_copyAndTemplate(a),
               NULL), // MULTIPLY
              NULL)); // DIVIDE
    first = dcFlatArithmetic_shrink(first, NULL);

    second = (CREATE_DIVIDE
              (CREATE_SUBTRACT
               // -b
               (CREATE_MULTIPLY
                (createNumber(-1),
                 // no copy this time
                 b,
                 NULL), // MULTIPLY
                // no copy this time
                value,
                NULL), // ADD
               CREATE_MULTIPLY
               (createNumber(2),
                dcNode_copyAndTemplate(a),
                NULL), // MULTIPLY
               NULL)); // DIVIDE
    second = dcFlatArithmetic_shrink(second, NULL);

    if (dcNode_easyCompare(first, second) == TAFFY_EQUALS)
    {
        //
        // first == second, so result is first^2
        //
        if (! dcNumberClass_equalsInt32u(a, 1))
        {
            result = (CREATE_MULTIPLY
                      (dcNode_copyAndTemplate(a),
                       CREATE_RAISE
                       (CREATE_SUBTRACT
                        (dcNode_copy(identifier, DC_DEEP),
                         first,
                         NULL),
                        createNumber(2),
                        NULL),
                       NULL));
        }
        else
        {
            result = (CREATE_RAISE
                      (CREATE_SUBTRACT
                       (dcNode_copy(identifier, DC_DEEP),
                        first,
                        NULL),
                       createNumber(2),
                       NULL));
        }

        result = dcFlatArithmetic_shrink(result, NULL);
        dcNode_free(&second, DC_DEEP);
    }
    else
    {
        dcNode *left = (CREATE_SUBTRACT
                        (dcNode_copy(identifier, DC_DEEP),
                         first,
                         NULL));
        dcNode *right = (CREATE_SUBTRACT
                         (dcNode_copy(identifier, DC_DEEP),
                          second,
                          NULL));
        left = dcFlatArithmetic_shrink(left, NULL);
        right = dcFlatArithmetic_shrink(right, NULL);
        result = CREATE_MULTIPLY(left, right, NULL);

        if (! dcNumberClass_equalsInt32u(a, 1))
        {
            dcList_unshift(CAST_FLAT_ARITHMETIC(result)->values,
                           dcNode_copyAndTemplate(a));
        }
    }

    dcNode_free(&_node, DC_DEEP);
    dcNode_free(&a, DC_DEEP);
    setModified(_modified);

kickout:
    dcNode_free(&node, DC_DEEP);

    if (dcLog_isEnabled(FLAT_ARITHMETIC_QUADRATIC_LOG))
    {
        dcLog_log(FLAT_ARITHMETIC_QUADRATIC_LOG,
                  "%sgot quadratic factor result: %s\n",
                  indent(),
                  dcNode_display(result));
    }

    return result;
}

dcNode *dcFlatArithmetic_factorQuadraticWhatever(dcNode *_node, bool *_modified)
{
    return factorQuadratic(_node, _modified, false);
}

dcNode *dcFlatArithmetic_factorQuadratic(dcNode *_node, bool *_modified)
{
    return factorQuadratic(_node, _modified, true);
}

dcNode *dcFlatArithmetic_factor(dcNode *_node, bool *_modified)
{
    return dcFlatArithmetic_factorWithSymbol(_node, NULL, _modified);
}

dcNode *dcFlatArithmetic_factorWithSymbol(dcNode *_node,
                                          const char *_symbol,
                                          bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcStringCacheElement element = {0};

    if (dcStringCache_getVoidOrNot(sFactorCache,
                                   _node,
                                   &element,
                                   _modified)
        == TAFFY_SUCCESS)
    {
        dcNode *result = element.value;
        dcStringCacheElement_free(&element);
        return result;
    }

    FOR_EACH_IN_NODE(_node, that)
    {
        if (dcNodeEvaluator_abortReceived(evaluator))
        {
            dcStringCacheElement_free(&element);
            return _node;
        }

        SHRINK_OPERATION(factor, that->object, _modified);
    }

    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);

    if ((! IS_ADD(_node)
         && ! IS_SUBTRACT(_node))
        || arithmetic->values->size == 1
        || dcNodeEvaluator_abortReceived(evaluator))
    {
        dcStringCacheElement_free(&element);
        return _node;
    }

    dcNode *backup = dcNode_copy(_node, DC_DEEP);
    SHRINK_OPERATION(convertSubtractToAdd, _node, NULL);
    SHRINK_OPERATION(convertDivideToMultiply, _node, NULL);

    if (! IS_FLAT_ARITHMETIC(_node))
    {
        dcStringCacheElement_free(&element);
        dcNode_free(&_node, DC_DEEP);
        return backup;
    }

    dcList *factorDatas = dcList_create();
    bool localModified = false;

    //
    // create the factor datas
    //
    addFactorDatas(NULL, _node, factorDatas, _symbol);

    //
    // factor:
    // first determine if there is more than one match per object
    // if there is, then really factor
    //
    while (factorDatas->size > 0)
    {
        if (dcNodeEvaluator_abortReceived(evaluator))
        {
            dcStringCacheElement_free(&element);
            dcList_free(&factorDatas, DC_DEEP);
            return _node;
        }

        dcNode *value = dcList_pop(factorDatas, DC_SHALLOW);
        uint32_t count = factorValue(arithmetic, value, false, evaluator);

        if (dcLog_isEnabled(FLAT_ARITHMETIC_FACTOR_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_FACTOR_LOG,
                      "%sgot %u matches for value '%s' in '%s'\n",
                      indent(),
                      count,
                      dcNode_display(value),
                      dcFlatArithmetic_display(arithmetic));
        }

        if (count > 1)
        {
            setModified(_modified);
            localModified = true;
            factorValue(arithmetic, value, true, evaluator);
            break;
        }

        dcNode_free(&value, DC_DEEP);
    }

    if (localModified)
    {
        dcStringCache_add(sFactorCache, &element, _node);
        dcNode_free(&backup, DC_DEEP);
    }
    else
    {
        dcStringCache_add(sFactorCache, &element, NULL);
        dcNode_free(&_node, DC_DEEP);
        _node = backup;
    }

    dcStringCacheElement_free(&element);
    dcList_free(&factorDatas, DC_DEEP);
    return _node;
}

bool dcFlatArithmetic_degree(dcNode *_node,
                             dcList *_symbols,
                             int32_t *_degree)
{
    dcNode *copy = dcNode_copy(_node, DC_DEEP);
    SHRINK_OPERATION(shrink, copy, NULL);
    bool result = dcFlatArithmetic_shrunkenDegree(copy, _symbols, _degree);
    dcNode_free(&copy, DC_DEEP);
    return result;
}

bool dcFlatArithmetic_shrunkenDegree(dcNode *_node,
                                     dcList *_symbols,
                                     int32_t *_degree)
{
    int32_t degree = 0;

    if (dcNumberClass_isZero(_node))
    {
        degree = -1;
    }
    else if (dcNumberClass_isMe(_node))
    {
        degree = 0;
    }
    else if (IS_IDENTIFIER(_node))
    {
        if (_symbols == NULL
            || dcList_containsEqual(_symbols, _node))
        {
            degree = 1;
        }
    }
    else if (IS_ADD(_node) || IS_SUBTRACT(_node))
    {
        FOR_EACH_IN_NODE(_node, that)
        {
            int32_t thisDegree = 0;

            if (! dcFlatArithmetic_shrunkenDegree(that->object,
                                                  _symbols,
                                                  &thisDegree))
            {
                return false;
            }

            degree = dcTaffy_max(degree, thisDegree);
        }
    }
    else if (IS_DIVIDE(_node))
    {
        int32_t topDegree = 0;
        int32_t bottomDegree = 0;

        FOR_EACH_IN_NODE(_node, that)
        {
            int32_t degreez = 0;

            if (! dcFlatArithmetic_shrunkenDegree(that->object,
                                                  _symbols,
                                                  &degreez))
            {
                return false;
            }

            if (that == CAST_FLAT_ARITHMETIC(_node)->values->head)
            {
                topDegree = degreez;
            }
            else
            {
                bottomDegree += degreez;
            }
        }

        degree = topDegree - bottomDegree;
    }
    else if (IS_MULTIPLY(_node))
    {
        int32_t thisDegree = 0;

        FOR_EACH_IN_NODE(_node, that)
        {
            int32_t degreez = 0;

            if (! dcFlatArithmetic_shrunkenDegree(that->object,
                                                  _symbols,
                                                  &degreez))
            {
                return false;
            }

            thisDegree += degreez;
        }

        degree = dcTaffy_max(degree, thisDegree);
    }
    else if (IS_RAISE(_node)
             && FLAT_ARITHMETIC_SIZE(_node) == 2)
    {
        dcNode *head = FLAT_ARITHMETIC_HEAD(_node);
        dcNode *tail = FLAT_ARITHMETIC_TAIL(_node);
        int32_t thisDegree = 0;
        int32_t thisPower = 0;

        if (dcNumberClass_isMe(tail)
            && dcNumberClass_extractInt32s(tail, &thisPower))
        {
            if (IS_IDENTIFIER(head))
            {
                if (_symbols == NULL
                    || dcList_containsEqual(_symbols, head))
                {
                    degree = dcTaffy_max(degree, thisPower);
                }
                else
                {
                    degree = 0;
                }
            }
            else if (dcFlatArithmetic_shrunkenDegree(head,
                                                     _symbols,
                                                     &thisDegree))
            {
                degree = thisDegree * thisPower;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    *_degree = degree;
    return true;
}

static int32_t polynomialDegree(dcNode *_node, dcList *_symbols)
{
    int32_t degree = 0;
    assert(dcFlatArithmetic_shrunkenDegree(_node, _symbols, &degree));
    return degree;
}

static dcNode *getHead(dcNode *_node)
{
    dcNode *head = _node;

    while (IS_ADD(head) || IS_SUBTRACT(head))
    {
        head = FLAT_ARITHMETIC_HEAD(head);
    }

    return head;
}

bool dcFlatArithmetic_dividePolynomials(dcNode *_dividend,
                                        dcNode *_divisor,
                                        dcList *_symbols,
                                        dcNode **_quotient, // OUT
                                        dcNode **_remainder) // OUT
{
    dcNode *divisor = dcNode_copy(_divisor, DC_DEEP);
    dcNode *dividend = dcNode_copy(_dividend, DC_DEEP);

    SHRINK_OPERATION(convertSubtractToAdd, divisor, NULL);
    SHRINK_OPERATION(orderPolynomial, divisor, NULL);
    SHRINK_OPERATION(distributeLikeAMadman, divisor, NULL);
    SHRINK_OPERATION(distribute, divisor, NULL);

    SHRINK_OPERATION(convertSubtractToAdd, dividend, NULL);
    SHRINK_OPERATION(orderPolynomial, dividend, NULL);
    SHRINK_OPERATION(distributeLikeAMadman, dividend, NULL);
    SHRINK_OPERATION(distribute, dividend, NULL);

    int32_t divisorDegree = 0;
    int32_t dividendDegree = 0;
    dcNode *remainder = NULL;
    dcNode *quotient = NULL;
    bool result = true;
    uint32_t indentLevel = 0;
    uint32_t i = 0;
    int32_t remainderDegree = 0;

    if (! dcFlatArithmetic_shrunkenDegree(divisor,
                                          _symbols,
                                          &divisorDegree)
        || ! dcFlatArithmetic_shrunkenDegree(dividend,
                                             _symbols,
                                             &dividendDegree))
    {
        result = false;
        goto kickout;
    }

    if (dcNumberClass_isMe(divisor)
        && dcNumberClass_equalsInt32u(divisor, 0))
    {
        // x / 0 ==> errr
        result = false;
        goto kickout;
    }
    else if (dividendDegree == -1
             && divisorDegree != -1)
    {
        // 0 / x
        remainder = createNumber(0);
        goto kickout;
    }

    remainder = dcNode_copy(dividend, DC_DEEP);
    quotient = createNumber(0);

    remainderDegree = polynomialDegree(remainder, _symbols);
    divisorDegree = polynomialDegree(divisor, _symbols);

    if (remainderDegree < divisorDegree)
    {
        result = false;
        goto kickout;
    }

    while (remainderDegree >= divisorDegree)
    {
        if (dcLog_isEnabled(FLAT_ARITHMETIC_DIVIDE_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_DIVIDE_LOG,
                      "%s[%u]: dividing '%s' by '%s'\n",
                      sIndents[i],
                      i,
                      dcNode_display(getHead(remainder)),
                      dcNode_display(getHead(divisor)));
        }

        dcNode *dividedHead =
            CREATE_DIVIDE(dcNode_copy(getHead(remainder), DC_DEEP),
                          dcNode_copy(getHead(divisor), DC_DEEP),
                          NULL);
        SHRINK_OPERATION(cancel, dividedHead, NULL);
        SHRINK_OPERATION(convertSubtractToAdd, dividedHead, NULL);
        SHRINK_OPERATION(combine, dividedHead, NULL);
        SHRINK_OPERATION(snip, dividedHead, NULL);
        SHRINK_OPERATION(undoConvertSubtractToAdd, dividedHead, NULL);

        if (dcLog_isEnabled(FLAT_ARITHMETIC_DIVIDE_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_DIVIDE_LOG,
                      "%s[%u]: dividedHead: '%s'\n",
                      sIndents[i],
                      i,
                      dcNode_display(dividedHead));
        }

        dcNode *headMultiplied =
            CREATE_MULTIPLY(dcNode_copy(dividedHead, DC_DEEP),
                            dcNode_copy(divisor, DC_DEEP),
                            NULL);
        SHRINK_OPERATION(distribute, headMultiplied, NULL);
        SHRINK_OPERATION(combine, headMultiplied, NULL);

        quotient = CREATE_ADD(dividedHead, quotient, NULL);
        remainder = CREATE_SUBTRACT(remainder, headMultiplied, NULL);

        if (dcLog_isEnabled(FLAT_ARITHMETIC_DIVIDE_LOG))
        {
            dcLog_log(FLAT_ARITHMETIC_DIVIDE_LOG,
                      "%s[%u]: quotient: '%s' remainder: '%s'\n",
                      sIndents[i],
                      i,
                      dcNode_display(quotient),
                      dcNode_display(remainder));
        }

        SHRINK_OPERATION(convertSubtractToAdd, quotient, NULL);
        SHRINK_OPERATION(combine, quotient, NULL);
        SHRINK_OPERATION(orderPolynomial, quotient, NULL);
        SHRINK_OPERATION(snip, quotient, NULL);

        SHRINK_OPERATION(multiplyByDenominator, remainder, NULL);
        SHRINK_OPERATION(convertSubtractToAdd, remainder, NULL);
        SHRINK_OPERATION(cancel, remainder, NULL);
        SHRINK_OPERATION(combine, remainder, NULL);
        SHRINK_OPERATION(snip, remainder, NULL);
        SHRINK_OPERATION(orderPolynomial, remainder, NULL);

        indentLevel++;
        int32_t newDegree = polynomialDegree(remainder, _symbols);

        if (newDegree == remainderDegree)
        {
            dcLog_logLine(FLAT_ARITHMETIC_DIVIDE_LOG,
                          "can't go any further due "
                          "to remainder being a bitch");
            result = false;
            break;
        }

        dcLog_log(FLAT_ARITHMETIC_DIVIDE_LOG,
                  "%s[%u] remainder after shrink: '%s'\n",
                  sIndents[i],
                  i,
                  dcNode_display(remainder));
        remainderDegree = newDegree;
        divisorDegree = polynomialDegree(divisor, _symbols);
        i++;
    }

kickout:
    dcNode_free(&dividend, DC_DEEP);
    dcNode_free(&divisor, DC_DEEP);

    if (result
        && (! dcNumberClass_isMe(quotient)
            || ! dcNumberClass_equalsInt32u(quotient, 0)))
    {
        // success!
        *_quotient = quotient;
        *_remainder = remainder;
    }
    else
    {
        // failure :(
        dcNode_free(&remainder, DC_DEEP);
        dcNode_free(&quotient, DC_DEEP);
        *_quotient = NULL;
        *_remainder = NULL;
    }

    return result;
}

dcNode *dcFlatArithmetic_distributeDivide(dcNode *_node, bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    dcListElement *that;
    dcNode *result = _node;

    if (arithmetic->taffyOperator == TAFFY_DIVIDE
        && arithmetic->values->size == 2)
    {
        dcNode *head = arithmetic->values->head->object;

        if (IS_FLAT_ARITHMETIC(head))
        {
            dcFlatArithmetic *headArithmetic = CAST_FLAT_ARITHMETIC(head);

            if (headArithmetic->taffyOperator == TAFFY_ADD
                || headArithmetic->taffyOperator == TAFFY_SUBTRACT)
            {
                dcList *values = dcList_create();

                while (headArithmetic->values->size > 0)
                {
                    dcNode *value = dcList_shift
                        (headArithmetic->values, DC_SHALLOW);
                    dcList_push(values,
                                CREATE_DIVIDE
                                (value,
                                 dcNode_copy(arithmetic->values->tail->object,
                                             DC_DEEP),
                                 NULL));
                }

                if (_modified != NULL)
                {
                    *_modified = true;
                }

                result = dcFlatArithmetic_createNodeWithList
                    (headArithmetic->taffyOperator, values);
                dcNode_free(&_node, DC_DEEP);
            }
         }
    }

    if (IS_FLAT_ARITHMETIC(result))
    {
        for (that = CAST_FLAT_ARITHMETIC(result)->values->head;
             that != NULL;
             that = that->next)
        {
            that->object = dcFlatArithmetic_distributeDivide(that->object,
                                                             _modified);
        }
    }

    return result;
}

dcNode *dcFlatArithmetic_distributeLikeAMadman(dcNode *_node, bool *_modified)
{
    dcNode *result = _node;
    dcNode *tail = NULL;
    dcNode *head = NULL;

    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    if (IS_RAISE(_node)
        && FLAT_ARITHMETIC_SIZE(_node) == 2
        && (head = FLAT_ARITHMETIC_HEAD(_node)) != NULL
        && IS_FLAT_ARITHMETIC(head)
        && (IS_ADD(head)
            || IS_SUBTRACT(head))
        && (tail = FLAT_ARITHMETIC_TAIL(_node)) != NULL
        && dcNumberClass_isMe(tail)
        && dcNumberClass_isWholeHelper(tail)
        && dcNumberClass_isPositive(tail)
        // need some kind of limit, but what?
        && dcNumber_lessThan(dcNumberClass_getNumber(tail),
                             dcNumber_getConstant(20)))
    {
        setModified(_modified);
        dcNode *number = (dcList_pop
                          (CAST_FLAT_ARITHMETIC(_node)->values,
                           DC_FLOATING));
        _node = popIfNotFactorial(_node, NULL);
        dcList *values = dcList_create();

        while (dcNumberClass_isPositive(number))
        {
            dcList_push(values, dcNode_copy(_node, DC_DEEP));
            dcNumberClass_decrement(number);
        }

        dcNode_free(&number, DC_DEEP);
        // TODO: don't necessitate need for free
        dcNode_free(&_node, DC_DEEP);
        result = dcFlatArithmetic_createNodeWithList(TAFFY_MULTIPLY,
                                                     values);
    }
    else
    {
        FOR_EACH_IN_NODE(_node, that)
        {
            SHRINK_OPERATION(distributeLikeAMadman, that->object, _modified);
        }
    }

    return result;
}

dcNode *dcFlatArithmetic_distribute(dcNode *_node, bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    dcListElement *that;
    dcNode *result = _node;

    if (arithmetic->taffyOperator == TAFFY_MULTIPLY)
    {
        dcNode *add = NULL;

        for (that = arithmetic->values->head; that != NULL; )
        {
            dcListElement *next = that->next;

            if (IS_FLAT_ARITHMETIC(that->object))
            {
                dcFlatArithmetic *thisArithmetic =
                    CAST_FLAT_ARITHMETIC(that->object);

                if (thisArithmetic->taffyOperator == TAFFY_ADD
                    || thisArithmetic->taffyOperator == TAFFY_SUBTRACT)
                {
                    if (_modified != NULL)
                    {
                        *_modified = true;
                    }

                    add = that->object;
                    dcList_removeElement(arithmetic->values, &that, DC_SHALLOW);
                    break;
                }
            }

            that = next;
        }

        if (add != NULL)
        {
            for (that = CAST_FLAT_ARITHMETIC(add)->values->head;
                 that != NULL;
                 that = that->next)
            {
                that->object = CREATE_MULTIPLY(that->object,
                                               (that->next == NULL
                                                ? _node
                                                : dcNode_copy(_node, DC_DEEP)),
                                               NULL);
            }

            result = add;
        }
    }

    if (IS_FLAT_ARITHMETIC(result))
    {
        for (that = CAST_FLAT_ARITHMETIC(result)->values->head;
             that != NULL;
             that = that->next)
        {
            that->object = dcFlatArithmetic_distribute(that->object, _modified);
        }
    }

    return result;
}

uint32_t dcFlatArithmetic_symbolCount(dcNode *_node, const char *_symbol)
{
    uint32_t count = 0;

    if (_node == NULL)
    {
        count = 0;
    }
    else if (identifierEquals(_node, _symbol))
    {
        count = 1;
    }
    else if (IS_METHOD_CALL(_node))
    {
        uint32_t i = 0;
        dcNode **argument = NULL;

        for (i = 0;
             (argument = getMethodArgument(_node, i, false)) != NULL;
             i++)
        {
            count += dcFlatArithmetic_symbolCount(*argument, _symbol);
        }
    }
    else if (IS_FLAT_ARITHMETIC(_node))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
        dcListElement *that;

        if (arithmetic->taffyOperator == TAFFY_RAISE
            && arithmetic->values->size == 2
            && dcNumberClass_isMe(arithmetic->values->tail->object)
            && identifierEquals(arithmetic->values->head->object, _symbol))
        {
            int32_t exponent;

            if (dcNumberClass_extractInt32s(arithmetic->values->tail->object,
                                            &exponent))
            {
                count += abs(exponent);
            }
        }
        else
        {
            for (that = arithmetic->values->head;
                 that != NULL;
                 that = that->next)
            {
                count += dcFlatArithmetic_symbolCount(that->object, _symbol);
            }
        }
    }

    return count;
}

static dcNode *expand(dcNode *_node, bool *_modified)
{
    if (IS_FLAT_ARITHMETIC(_node))
    {
        dcFlatArithmetic *thisArithmetic =
            CAST_FLAT_ARITHMETIC(_node);

        if (thisArithmetic->taffyOperator == TAFFY_RAISE
            && thisArithmetic->values->size == 2
            && dcNumberClass_isMe(thisArithmetic->values->tail->object))
        {
            uint32_t size;

            if (dcNumberClass_extractInt32u
                (thisArithmetic->values->tail->object, &size)
                && size <= 5) // arbitrary?
            {
                if (_modified != NULL)
                {
                    *_modified = true;
                }

                dcList *expanded = dcList_create();
                uint32_t i;

                for (i = 0; i < size; i++)
                {
                    dcList_push(expanded, dcNode_copy
                                (thisArithmetic->values->head->object,
                                 DC_DEEP));
                }

                dcNode_free(&_node, DC_DEEP);
                _node = dcFlatArithmetic_createNodeWithList
                    (TAFFY_MULTIPLY, expanded);
            }
        }
    }

    return _node;
}

bool dcFlatArithmetic_equals(dcNode *_left, dcNode *_right)
{
    dcNode *left = dcNode_setTemplate(dcNode_copy(_left, DC_DEEP), true);
    dcNode *right = dcNode_setTemplate(dcNode_copy(_right, DC_DEEP), true);
    dcNode *subtract = (dcFlatArithmetic_shrink
                        (dcFlatArithmetic_createNodeWithValues
                         (TAFFY_SUBTRACT, left, right, NULL),
                         NULL));
    bool result = (subtract != NULL
                   && dcNumberClass_isMe(subtract)
                   && dcNumberClass_isZero(subtract));
    dcNode_free(&subtract, DC_DEEP);
    return result;
}

bool dcFlatArithmetic_deltaEquals(dcNode *_left, dcNode *_right, dcNode *_delta)
{
    TAFFY_DEBUG(assert(dcNumberClass_isMe(_delta)
                       && dcNumberClass_isWholeHelper(_delta)));
    uint32_t delta;
    dcNode *left = dcNode_setTemplate(dcNode_copy(_left, DC_DEEP), true);
    dcNode *right = dcNode_setTemplate(dcNode_copy(_right, DC_DEEP), true);
    dcNode *subtract = (dcFlatArithmetic_shrink
                        (dcFlatArithmetic_createNodeWithValues
                         (TAFFY_SUBTRACT, left, right, NULL),
                         NULL));
    bool result = false;

    assert(dcNumberClass_extractInt32u(_delta, &delta));

    if (subtract != NULL)
    {
        if (dcNumberClass_isMe(subtract)
            && dcNumberClass_isZero(subtract))
        {
            result = true;
        }
        else if (IS_MULTIPLY(subtract))
        {
            FOR_EACH_IN_NODE(subtract, that)
            {
                if (dcNumberClass_isMe(that->object)
                    && (dcNumber_deltaEqual
                        (dcNumberClass_getNumber(that->object),
                         dcNumber_getConstant(0),
                         delta)))
                {
                    result = true;
                    break;
                }
            }
        }
    }

    dcNode_free(&subtract, DC_DEEP);
    return result;
}

dcNode *dcFlatArithmetic_expand(dcNode *_node, bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
    dcListElement *that;

    for (that = arithmetic->values->head; that != NULL; that = that->next)
    {
        that->object = expand(that->object, _modified);
    }

    return dcFlatArithmetic_merge(expand(_node, _modified), _modified);
}

dcNode *dcFlatArithmetic_moveNumberToFront(dcNode *_arithmetic,
                                           bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_arithmetic))
    {
        return _arithmetic;
    }

    FOR_EACH_IN_NODE(_arithmetic, that)
    {
        that->object =
            dcFlatArithmetic_moveNumberToFront(that->object, _modified);
    }

    if (! IS_MULTIPLY(_arithmetic)
        || FLAT_ARITHMETIC_SIZE(_arithmetic) < 2)
    {
        return _arithmetic;
    }

    dcList *values = CAST_FLAT_ARITHMETIC(_arithmetic)->values;

    for (that = values->head->next; that != NULL; )
    {
        dcListElement *next = that->next;

        if (dcNumberClass_isMe(that->object))
        {
            setModified(_modified);
            dcList_unshift(values, that->object);
            dcList_removeElement(values, &that, DC_SHALLOW);
        }

        that = next;
    }

    return _arithmetic;
}

dcNode *dcFlatArithmetic_simplifyTrigonometry(dcNode *_arithmetic,
                                              bool *_modified)
{
    dcLog_log(FLAT_ARITHMETIC_TRIGONOMETRY_LOG,
              "simplifying: '%s'\n",
              dcNode_display(_arithmetic));

    dcNode *result = calculusOperation(_arithmetic,
                                       "",
                                       sTrigonometricSimplificationLanguages,
                                       _modified);
    if (result != NULL)
    {
        dcNode_free(&_arithmetic, DC_DEEP);
    }
    else
    {
        result = _arithmetic;
    }

    return result;
}

const char *dcFlatArithmetic_getArithmeticLanguageName(dcNode *_node)
{
    void *address = CAST_VOID(_node);

    struct
    {
        const void *address;
        const char *name;
    } names[] =
      {
          {sAddLanguages,       "Add Languages"},
          {sSubtractLanguages,  "Subtract Languages"},
          {sMultiplyLanguages,  "Multiply Languages"},
          {sDivideLanguages,    "Divide Languages"},
          {sRaiseLanguages,     "Raise Languages"},
          {sDeriveLanguages,    "Derive Languages"},
          {sBitAndLanguages,    "Bit-And Languages"},
          {sBitOrLanguages,     "Bit-Or Languages"},
          {sConvertLanguages,   "Convert Languages"},
          {sIntegrateLanguages, "Integrate Languages"},
          {sIntegrateSimplificationLanguages,
           "Integrate Simplification Languages"},
          {sTrigonometricSimplificationLanguages,
           "Trigonometric Simplification Languages"},
      };

    const char *result = NULL;
    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(names); i++)
    {
        if (address == names[i].address)
        {
            result = names[i].name;
            break;
        }
    }

    assert(result != NULL);
    return result;
}

dcNode *dcFlatArithmetic_multiplyByDenominator(dcNode *_node, bool *_modified)
{
    if (! IS_FLAT_ARITHMETIC(_node))
    {
        return _node;
    }

    FOR_EACH_IN_NODE(_node, that)
    {
        that->object = dcFlatArithmetic_multiplyByDenominator(that->object,
                                                              _modified);
    }

    if (! IS_ADD(_node) && ! IS_SUBTRACT(_node))
    {
        return _node;
    }

    dcNode *newDenominator = NULL;

    FOR_EACH_IN_NODE_AGAIN(_node, that)
    {
        if (IS_DIVIDE(that->object))
        {
            setModified(_modified);

            dcFlatArithmetic *dividy = CAST_FLAT_ARITHMETIC(that->object);
            assert(dividy->values->size == 2);
            dcNode *denominator = FLAT_ARITHMETIC_TAIL(that->object);

            FOR_EACH_IN_NODE(_node, dis)
            {
                if (dis != that)
                {
                    dcNode **object = NULL;

                    if (IS_DIVIDE(dis->object))
                    {
                        object = &FLAT_ARITHMETIC_HEAD(dis->object);
                    }
                    else
                    {
                        object = &dis->object;
                    }

                    *object = CREATE_MULTIPLY(*object,
                                              dcNode_copy(denominator, DC_DEEP),
                                              NULL);
                }
            }

            newDenominator = CREATE_MULTIPLY(dcNode_copy(denominator, DC_DEEP),
                                             newDenominator,
                                             NULL);

            // turn the divide into a plain ol' node
            dcList_pop(dividy->values, DC_DEEP);
            that->object = popIfNotFactorial(that->object, NULL);
        }
    }

    return (newDenominator == NULL
            ? _node
            : CREATE_DIVIDE(_node, newDenominator, NULL));
}

// convert (x + y) / z to x / z + y / z
dcNode *dcFlatArithmetic_expandDivide(dcNode *_node, bool *_modified)
{
    dcNode *result = _node;

    if (! IS_DIVIDE(_node)
        || FLAT_ARITHMETIC_SIZE(_node) != 2)
    {
        return result;
    }

    dcListElement *numerator = CAST_FLAT_ARITHMETIC(result)->values->head;
    dcListElement *denominator =
        CAST_FLAT_ARITHMETIC(result)->values->head->next;
    dcTaffyOperator type =
        CAST_FLAT_ARITHMETIC(numerator->object)->taffyOperator;

    if (type == TAFFY_ADD || type == TAFFY_SUBTRACT)
    {
        setModified(_modified);
        dcList *values = dcList_create();
        dcFlatArithmetic *top = CAST_FLAT_ARITHMETIC(numerator->object);

        while (top->values->size > 0)
        {
            dcNode *head = dcList_shift(top->values, DC_SHALLOW);
            dcList_push(values,
                        CREATE_DIVIDE(head,
                                      dcNode_copy(denominator->object, DC_DEEP),
                                      NULL));
        }

        dcNode_free(&_node, DC_DEEP);
        result = dcFlatArithmetic_createNodeWithList(type, values);
    }

    return result;
}

static bool find(dcNode *_arithmetic, dcNode *_node)
{
    if ((dcNumberClass_isMe(_arithmetic)
         && dcNumberClass_isMe(_node))
        || dontCareCompare(_arithmetic, _node))
    {
        return true;
    }
    else if (IS_FLAT_ARITHMETIC(_arithmetic))
    {
        FOR_EACH_IN_NODE(_arithmetic, that)
        {
            if (find(that->object, _node)
                || dontCareCompare(that->object, _node)
                || (dcNumberClass_isMe(_arithmetic)
                    && dcNumberClass_isMe(_node)))
            {
                return true;
            }
        }
    }

    return false;
}

bool dcFlatArithmetic_find(dcNode *_arithmetic, dcNode *_node)
{
    dcNode *arithmetic = expandOperations(dcNode_copy(_arithmetic, DC_DEEP));
    dcNode *node = expandOperations(dcNode_copy(_node, DC_DEEP));
    bool result = find(arithmetic, node);
    dcNode_free(&arithmetic, DC_DEEP);
    dcNode_free(&node, DC_DEEP);
    return result;
}

// convert -a + b to b - a
dcNode *dcFlatArithmetic_orderSubtract(dcNode *_node, bool *_modified)
{
    dcNode *result = _node;

    if (! IS_ADD(_node))
    {
        return result;
    }

    dcList *cache = dcList_create();
    dcListElement *that;

    for (that = CAST_FLAT_ARITHMETIC(result)->values->head; that != NULL; )
    {
        dcListElement *next = that->next;

        if ((dcNumberClass_isMe(that->object)
             && dcNumberClass_isNegative(that->object))
            || isNegativeMultiply(that->object, false))
        {
            setModified(_modified);
            dcList_push(cache, that->object);
            dcList_removeElement(CAST_FLAT_ARITHMETIC(result)->values,
                                 &that,
                                 DC_SHALLOW);
        }

        that = next;
    }

    dcList_append(CAST_FLAT_ARITHMETIC(result)->values, &cache);
    return result;
}

static dcNode *orderPolynomial(dcNode *_node, bool *_modified, dcList *_symbols)
{
    dcNode *result = _node;

    if (! IS_FLAT_ARITHMETIC(result))
    {
        return result;
    }

    FOR_EACH_IN_NODE(result, that)
    {
        SHRINK_OPERATION(orderPolynomial, that->object, _modified);
    }

    if (! IS_ADD(result)
        && ! IS_SUBTRACT(result))
    {
        return result;
    }

    SHRINK_OPERATION(convertSubtractToAdd, result, NULL);
    SHRINK_OPERATION(merge, result, NULL);

    if (! IS_FLAT_ARITHMETIC(result))
    {
        return result;
    }

    bool success = true;
    dcList *values = CAST_FLAT_ARITHMETIC(result)->values;

    // selection sort
    dcListElement *max = NULL;
    dcListElement *i = NULL;
    int32_t *degrees = (int32_t *)(dcMemory_allocate
                                   (sizeof(int32_t) * values->size));
    size_t iValue = 0;
    bool outOfOrder = false;

    for (i = values->head, iValue = 0;
         i != NULL;
         i = i->next, iValue++)
    {
        if (! dcFlatArithmetic_shrunkenDegree(i->object,
                                              _symbols,
                                              &degrees[iValue]))
        {
            success = false;
            break;
        }

        if (iValue > 0 && degrees[iValue - 1] < degrees[iValue])
        {
            outOfOrder = true;
        }
    }

    if (success && outOfOrder)
    {
        for (i = values->head, iValue = 0;
             i->next != NULL;
             i = i->next, iValue++)
        {
            max = i;
            size_t maxValue = iValue;
            size_t jValue = 0;
            dcListElement *j = NULL;

            for (j = i->next, jValue = iValue + 1;
                 j != NULL;
                 j = j->next, jValue++)
            {
                if (degrees[jValue] > degrees[maxValue])
                {
                    max = j;
                    maxValue = jValue;
                }
            }

            if (max != i)
            {
                setModified(_modified);
                dcNode *temp = i->object;
                i->object = max->object;
                max->object = temp;
                int32_t tempDegree = degrees[iValue];
                degrees[iValue] = degrees[maxValue];
                degrees[maxValue] = tempDegree;
            }
        }
    }

    SHRINK_OPERATION(undoConvertSubtractToAdd, result, NULL);
    SHRINK_OPERATION(merge, result, NULL);
    dcMemory_free(degrees);
    return result;
}

void dcFlatArithmetic_getIdentifiers(dcNode *_node, dcList *_identifiers)
{
    if (IS_FLAT_ARITHMETIC(_node))
    {
        FOR_EACH_IN_NODE(_node, that)
        {
            dcFlatArithmetic_getIdentifiers(that->object, _identifiers);
        }
    }
    else if (IS_IDENTIFIER(_node))
    {
        dcList_push(_identifiers, dcNode_copy(_node, DC_DEEP));
    }
}

// convert: 2x + x^3 + 5x^2
//      to: x^3 + 5x^2 + 2x
// note: this does not shrink anything, so x^(3 - 1) has the same order as x^0
dcNode *dcFlatArithmetic_orderPolynomial(dcNode *_node, bool *_modified)
{
    dcList *identifiers = dcList_create();
    dcFlatArithmetic_getIdentifiers(_node, identifiers);
    dcNode *result = orderPolynomial(_node, _modified, identifiers);
    dcList_free(&identifiers, DC_DEEP);
    return result;
}

char *dcFlatArithmetic_displayReal(dcNode *_node)
{
    dcString result;
    dcString_initialize(&result, 10);

    if (dcFlatArithmetic_isMe(_node))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(_node);
        dcString_append(&result,
                        "[%s ",
                        dcSystem_getOperatorName(arithmetic->taffyOperator));

        FOR_EACH(arithmetic, that)
        {
            char *thatDisplay = dcFlatArithmetic_displayReal(that->object);
            dcString_append(&result, "%s", thatDisplay);
            dcMemory_free(thatDisplay);

            if (that->next != NULL)
            {
                dcString_append(&result, ", ");
            }
        }

        dcString_append(&result, "]");
    }
    else
    {
        dcNode_print(_node, &result);
    }

    return result.string;
}

static void initializeComparisonLanguage(ComparisonLanguage *_language)
{
    ComparisonLanguage *i;

    for (i = _language; i->left != NULL; i++)
    {
        i->leftTokens = dcLexer_splitString(i->left, ' ');
        i->rightTokens = dcLexer_splitString(i->right, ' ');
    }
}

static void deinitializeComparisonLanguage(ComparisonLanguage *_language)
{
    ComparisonLanguage *i;

    for (i = _language; i->left != NULL; i++)
    {
        dcList_free(&i->leftTokens, DC_DEEP);
        dcList_free(&i->rightTokens, DC_DEEP);
    }
}

static void initializeTextResultLanguage(TextResultLanguage *_language)
{
    TextResultLanguage *i;

    for (i = _language; i->left != NULL; i++)
    {
        i->leftTokens = dcLexer_splitString(i->left, ' ');
        i->rightTokens = dcLexer_splitString(i->right, ' ');
        i->resultTokens = dcLexer_splitString(i->result, ' ');
    }
}

static void deinitializeTextResultLanguage(TextResultLanguage *_language)
{
    TextResultLanguage *i;

    for (i = _language; i->left != NULL; i++)
    {
        dcList_free(&i->leftTokens, DC_DEEP);
        dcList_free(&i->rightTokens, DC_DEEP);
        dcList_free(&i->resultTokens, DC_DEEP);
    }
}

static void initializeNewOperationLanguage(NewOperationLanguage *_language)
{
    NewOperationLanguage *i;

    for (i = _language; i->programText != NULL; i++)
    {
        i->programTokens = dcLexer_splitString(i->programText, ' ');
        i->resultTokens = dcLexer_splitString(i->result, ' ');
    }
}

static void deinitializeNewOperationLanguage(NewOperationLanguage *_language)
{
    NewOperationLanguage *i;

    for (i = _language; i->programText != NULL; i++)
    {
        dcList_free(&i->programTokens, DC_DEEP);
        dcList_free(&i->resultTokens, DC_DEEP);
    }
}

static ComparisonLanguage *sComparisonLanguages[] =
    {
        sAddLanguages,
        sMultiplyLanguages,
        sDivideLanguages,
        sRaiseLanguages,
        sBitAndLanguages,
        sBitOrLanguages
    };

static TextResultLanguage *sTextResultLanguages[] =
    {
        sAddTextResultLanguages,
        sMultiplyTextResultLanguages,
        sDivideTextResultLangauges
    };

static NewOperationLanguage *sNewOperationLanguages[] =
    {
        sDeriveLanguages,
        sConvertLanguages,
        sTrigonometricSimplificationLanguages,
        sIntegrateSimplificationLanguages,
        sIntegrateLanguages
    };

void dcFlatArithmetic_initialize(void)
{
    sIntegrateMutex = dcMutex_create(false);
    sIntegrateHash = dcHash_create();
    sLeftHandSide = dcIdentifier_createNode("!leftHandSideLoL!", NO_FLAGS);

    sIntegrateCache = dcStringCache_create(true);
    sShrinkCache = dcStringCache_create(true);
    sMultiFactorCache = dcStringCache_create(true);
    sFactorCache = dcStringCache_create(true);
    sCancelCache = dcStringCache_create(true);
    sSubstituteCache = dcStringCache_create(true);
    sDeriveCache = dcStringCache_create(true);

    uint32_t i;

    for (i = 0; i <= MAX_INTEGRATE_DEPTH; i++)
    {
        uint32_t j;

        for (j = 0; j < i; j++)
        {
            sIndents[i][j] = ' ';
        }

        sIndents[i][j] = 0;
    }

    sNewMatchLookup = dcHash_create();

    for (i = 0; i < dcTaffy_countOf(sNewMatchFunctionMap); i++)
    {
        dcHash_setValueWithStringKey
            (sNewMatchLookup,
             sNewMatchFunctionMap[i].symbol,
             dcVoidContainer_createNode
             ((int32_t *)sNewMatchFunctionMap[i].matcher));
    }

    TAFFY_DEBUG(sArithmeticCounts = dcHash_create());

    for (i = 0; i < dcTaffy_countOf(sComparisonLanguages); i++)
    {
        initializeComparisonLanguage(sComparisonLanguages[i]);
    }

    for (i = 0; i < dcTaffy_countOf(sTextResultLanguages); i++)
    {
        initializeTextResultLanguage(sTextResultLanguages[i]);
    }

    for (i = 0; i < dcTaffy_countOf(sNewOperationLanguages); i++)
    {
        initializeNewOperationLanguage(sNewOperationLanguages[i]);
    }

    TAFFY_DEBUG(sCounts = dcHash_create());
}

void dcFlatArithmetic_deinitialize(void)
{
    dcMutex_free(&sIntegrateMutex);
    dcHash_free(&sIntegrateHash, DC_DEEP);
    dcNode_free(&sLeftHandSide, DC_DEEP);

    dcStringCache_free(&sIntegrateCache);
    dcStringCache_free(&sShrinkCache);
    dcStringCache_free(&sMultiFactorCache);
    dcStringCache_free(&sFactorCache);
    dcStringCache_free(&sCancelCache);
    dcStringCache_free(&sSubstituteCache);
    dcStringCache_free(&sDeriveCache);

    dcHash_free(&sNewMatchLookup, DC_DEEP);

    uint8_t i;

    for (i = 0; i < dcTaffy_countOf(sComparisonLanguages); i++)
    {
        deinitializeComparisonLanguage(sComparisonLanguages[i]);
    }

    for (i = 0; i < dcTaffy_countOf(sTextResultLanguages); i++)
    {
        deinitializeTextResultLanguage(sTextResultLanguages[i]);
    }

    for (i = 0; i < dcTaffy_countOf(sNewOperationLanguages); i++)
    {
        deinitializeNewOperationLanguage(sNewOperationLanguages[i]);
    }

    TAFFY_DEBUG(dcHash_free(&sArithmeticCounts, DC_DEEP));
    TAFFY_DEBUG(dcHash_free(&sCounts, DC_DEEP));
}

bool dcFlatArithmetic_isSingleRoot(dcNode *_node,
                                   dcNode *_value,
                                   const char *_symbol)
{
    dcNode *copy = dcNode_copy(_node, DC_DEEP);
    bool result = false;

    if (dcFlatArithmetic_findIdentifier(&copy, _symbol, _value))
    {
        copy = dcFlatArithmetic_shrink(copy, NULL);

        if (dcNumberClass_isMe(copy)
            && dcNumberClass_equalsInt32u(copy, 0))
        {
            result = true;
        }
    }

    dcNode_free(&copy, DC_DEEP);
    return result;
}

bool dcFlatArithmetic_maxPower(dcNode *_node, uint32_t *_power)
{
    bool result = false;
    uint32_t power = 0;

    if (IS_RAISE(_node)
        && FLAT_ARITHMETIC_SIZE(_node) == 2
        && dcNumberClass_isMe(FLAT_ARITHMETIC_TAIL(_node))
        && dcNumberClass_extractInt32u(FLAT_ARITHMETIC_TAIL(_node), &power)
        && power > *_power)
    {
        result = true;
        *_power = power;
    }
    else if (IS_FLAT_ARITHMETIC(_node))
    {
        FOR_EACH_IN_NODE(_node, that)
        {
            if (dcFlatArithmetic_maxPower(that->object, &power)
                && power > *_power)
            {
                result = true;
                *_power = power;
            }
        }
    }

    return result;
}

void dcFlatArithmetic_populateIdentifiers(dcNode *_arithmetic,
                                          dcList *_identifiers)
{
    if (IS_IDENTIFIER(_arithmetic))
    {
        if (! dcList_containsEqual(_identifiers, _arithmetic)
            && ! isReservedName(_arithmetic))
        {
            dcList_push(_identifiers, dcNode_copy(_arithmetic, DC_DEEP));
        }
    }
    else if (IS_METHOD_CALL(_arithmetic))
    {
        dcNode *receiver = dcMethodCall_getReceiver(_arithmetic);

        if (IS_IDENTIFIER(receiver)
            && ! dcList_containsEqual(_identifiers, receiver)
            && ! isReservedName(receiver))
        {
            dcList_push(_identifiers, dcNode_copy(receiver, DC_DEEP));
        }
        else
        {
            // check the arguments
            FOR_EACH_IN_LIST(CAST_METHOD_CALL(_arithmetic)->arguments, that)
            {
                if (IS_CLASS(that->object)
                    && dcArrayClass_isMe(that->object))
                {
                    dcArray *objects = dcArrayClass_getObjects(that->object);
                    uint32_t i;

                    for (i = 0; i < objects->size; i++)
                    {
                        if (IS_IDENTIFIER(objects->objects[i])
                            && ! dcList_containsEqual(_identifiers,
                                                      objects->objects[i])
                            && ! isReservedName(objects->objects[i]))
                        {
                            dcList_push(_identifiers,
                                        dcNode_copy(objects->objects[i],
                                                    DC_DEEP));
                        }
                        else if (IS_FLAT_ARITHMETIC(objects->objects[i]))
                        {
                            dcFlatArithmetic_populateIdentifiers
                                (objects->objects[i], _identifiers);
                        }
                    }
                }
            }
        }
    }
    else if (IS_FLAT_ARITHMETIC(_arithmetic))
    {
        FOR_EACH_IN_NODE(_arithmetic, that)
        {
            dcFlatArithmetic_populateIdentifiers(that->object, _identifiers);
        }
    }
}

bool dcFlatArithmetic_isMe(const dcNode *_node)
{
    return IS_FLAT_ARITHMETIC(_node);
}

// change (a / b) / c into a / b / c
void dcFlatArithmetic_mergeDivide(dcFlatArithmetic *_arithmetic)
{
    while (_arithmetic->values->size > 0
           && IS_DIVIDE(_arithmetic->values->head->object))
    {
        dcListElement *headElement = _arithmetic->values->head;
        dcFlatArithmetic *divide = CAST_FLAT_ARITHMETIC(headElement->object);

        while (divide->values->size > 0)
        {
            dcNode *head = dcList_shift(divide->values, DC_SHALLOW);
            dcList_insertBeforeListElement(_arithmetic->values, headElement, head);
        }

        dcListElement_free(&headElement, _arithmetic->values, DC_DEEP);
    }
}

bool dcFlatArithmetic_isGrouped(const dcNode *_node)
{
    return (IS_FLAT_ARITHMETIC(_node) && CAST_FLAT_ARITHMETIC(_node)->grouped);
}
