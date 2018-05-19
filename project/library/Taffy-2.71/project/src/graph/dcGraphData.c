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

#include "dcCharacterGraph.h"
#include "dcClass.h"
#include "dcContainers.h"
#include "dcError.h"
#include "dcGraphData.h"
#include "dcGraphDatas.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcSystem.h"

static const char * const sTypeNames[] =
{
    "NODE_AND",
    "NODE_ASSIGNMENT",
    "NODE_BREAK",
    "NODE_CATCH_BLOCK",
    "NODE_CLASS",
    "NODE_EXIT",
    "NODE_NO",
    "NODE_FLAT_ARITHMETIC",
    "NODE_FOR",
    "NODE_FUNCTION_UPDATE",
    "NODE_GRAPH_DATA_LIST",
    "NODE_GRAPH_DATA_NODE",
    "NODE_GRAPH_DATA_PAIR",
    "NODE_GRAPH_DATA_TREE",
    "NODE_IDENTIFIER",
    "NODE_IF",
    "NODE_IMPORT",
    "NODE_IN",
    "NODE_METHOD_CALL",
    "NODE_METHOD_HEADER",
    "NODE_NEW",
    "NODE_NIL",
    "NODE_NOT_EQUAL_CALL",
    "NODE_OR",
    "NODE_PACKAGE",
    "NODE_RETURN",
    "NODE_SELF",
    "NODE_SUPER",
    "NODE_SYMBOL",
    "NODE_SYNCHRONIZED",
    "NODE_THROW",
    "NODE_TRUE",
    "NODE_TRY_BLOCK",
    "NODE_UP_SELF",
    "NODE_WHILE",
};

static const dcNode_copyPointer sCopyPointers[] =
{
    &dcPair_copyGraphDataNode,   // dcAnd
    &dcAssignment_copyNode,      // dcAssignment
    NULL,                        // dcBreak
    &dcCatchBlock_copyNode,      // dcCatchBlock
    &dcClass_copyNode,           // dcClass
    &dcGraphDataNode_copyNode,   // dcExit
    NULL,                        // dcFalse
    &dcFlatArithmetic_copyNode,  // dcFlatArithmetic
    &dcFor_copyNode,             // dcFor
    &dcFunctionUpdate_copyNode,  // dcFunctionUpdate
    &dcList_copyGraphDataNode,   // dcList (GraphData)
    NULL,                        // dcNode (GraphData)
    &dcPair_copyGraphDataNode,   // dcPair (GraphData)
    &dcGraphDataTree_copyNode,   // dcGraphDataTree
    &dcIdentifier_copyNode,      // dcIdentifier
    &dcIf_copyNode,              // dcIf
    &dcPackage_copyNode,         // dcImport
    &dcIn_copyNode,              // dcIn
    &dcMethodCall_copyNode,      // dcMethodCall
    &dcMethodHeader_copyNode,    // dcMethodHeader
    &dcGraphDataNode_copyNode,   // dcNew
    NULL,                        // dcNil
    &dcGraphDataNode_copyNode,   // dcNotEqualCall
    &dcPair_copyGraphDataNode,   // dcOr
    &dcPackage_copyNode,         // dcPackage
    &dcGraphDataNode_copyNode,   // dcReturn
    NULL,                        // dcSelf
    NULL,                        // dcSuper
    &dcSymbol_copyNode,          // dcSymbol
    &dcPair_copyGraphDataTree,   // dcSynchronized
    &dcGraphDataNode_copyNode,   // dcThrow
    NULL,                        // dcTrue
    &dcTryBlock_copyNode,        // dcTryBlock
    NULL,                        // dcUpSelf
    &dcPair_copyGraphDataTree    // dcWhile
};

static const dcNode_freePointer sFreePointers[] =
{
    &dcPair_freeGraphDataNode,   // dcAnd
    &dcAssignment_freeNode,      // dcAssignment
    NULL,                        // dcBreak
    &dcCatchBlock_freeNode,      // dcCatchBlock
    &dcClass_freeNode,           // dcClass
    &dcGraphDataNode_freeNode,   // dcExit
    NULL,                        // dcFalse
    &dcFlatArithmetic_freeNode,  // dcFlatArithmetic
    &dcFor_freeNode,             // dcFor
    &dcFunctionUpdate_freeNode,  // dcFunctionUpdate
    &dcList_freeGraphDataNode,   // dcList (GraphData)
    &dcGraphDataNode_freeNode,   // dcNode (GraphData)
    &dcPair_freeGraphDataNode,   // dcPair (GraphData)
    &dcGraphDataTree_freeNode,   // dcGraphData
    &dcIdentifier_freeNode,      // dcIdentifier
    &dcIf_freeNode,              // dcIf
    &dcPackage_freeNode,         // dcImport
    &dcIn_freeNode,              // dcIn
    &dcMethodCall_freeNode,      // dcMethodCall
    &dcMethodHeader_freeNode,    // dcMethodHeader
    &dcGraphDataNode_freeNode,   // dcNew
    NULL,                        // dcNil
    &dcGraphDataNode_freeNode,   // dcNotEqualCall
    &dcPair_freeGraphDataNode,   // dcOr
    &dcPackage_freeNode,         // dcPackage
    &dcGraphDataNode_freeNode,   // dcReturn
    NULL,                        // dcSelf
    NULL,                        // dcSuper
    &dcSymbol_freeNode,          // dcSymbol
    &dcPair_freeGraphDataNode,   // dcSynchronized
    &dcGraphDataNode_freeNode,   // dcThrow
    NULL,                        // dcTrue
    &dcTryBlock_freeNode,        // dcTryBlock
    NULL,                        // dcUpSelf
    &dcPair_freeGraphDataNode    // dcWhile
};

static const dcNode_markPointer sMarkPointers[] =
{
    &dcPair_markGraphDataNode,  // dcAnd
    &dcAssignment_markNode,     // dcAssignment
    NULL,                       // dcBreak
    NULL,                       // dcCatchBlock
    &dcClass_markNode,          // dcClass
    &dcGraphDataNode_markNode,  // dcExit
    NULL,                       // dcFalse
    NULL,                       // dcFlatArithmetic
    &dcFor_markNode,            // dcFor
    NULL,                       // dcFunctionUpdate
    &dcList_markGraphDataNode,  // dcList (GraphData)
    NULL,                       // dcNode (GraphData)
    &dcPair_markGraphDataNode,  // dcPair (GraphData)
    NULL,                       // dcGraphDataTree
    NULL,                       // dcIdentifier
    &dcIf_markNode,             // dcIf
    NULL,                       // dcImport
    &dcIn_markNode,             // dcIn
    &dcMethodCall_markNode,     // dcMethodCall
    NULL,                       // dcMethodHeader
    &dcGraphDataNode_markNode,  // dcNew
    NULL,                       // dcNil
    &dcGraphDataNode_markNode,  // dcNotEqualCall
    &dcPair_markGraphDataNode,  // dcOr
    NULL,                       // dcPackage
    &dcGraphDataNode_markNode,  // dcReturn
    NULL,                       // dcSelf
    NULL,                       // dcSuper
    NULL,                       // dcSymbol
    NULL,                       // dcSynchronized
    NULL,                       // dcThrow
    NULL,                       // dcTrue
    NULL,                       // dcTryBlock
    NULL,                       // dcUpSelf
    NULL                        // dcWhile
};

static const dcNode_registerPointer sRegisterPointers[] =
{
    NULL,                  // dcAnd
    NULL,                  // dcAssignment
    NULL,                  // dcBreak
    NULL,                  // dcCatchBlock
    &dcClass_registerNode, // dcClass
    NULL,                  // dcExit
    NULL,                  // dcFalse
    NULL,                  // dcFlatArithmetic
    NULL,                  // dcFor
    NULL,                  // dcFunctionUpdate
    NULL,                  // dcList (GraphData)
    NULL,                  // dcNode (GraphData)
    NULL,                  // dcPair (GraphData)
    NULL,                  // dcGraphDataTree
    NULL,                  // dcIdentifier
    NULL,                  // dcIf
    NULL,                  // dcImport
    NULL,                  // dcIn
    NULL,                  // dcMethodCall
    NULL,                  // dcMethodHeader
    NULL,                  // dcNew
    NULL,                  // dcNil
    NULL,                  // dcNotEqualCall
    NULL,                  // dcOr
    NULL,                  // dcPackage
    NULL,                  // dcReturn
    NULL,                  // dcSelf
    NULL,                  // dcSuper
    NULL,                  // dcSymbol
    NULL,                  // dcSynchronized
    NULL,                  // dcThrow
    NULL,                  // dcTrue
    NULL,                  // dcTryBlock
    NULL,                  // dcUpSelf
    NULL                   // dcWhile
};

static const dcNode_printPointer sPrintPointers[] =
{
    &dcBool_printNode,            // dcAnd
    &dcAssignment_printNode,      // dcAssignment
    &dcBreak_printNode,           // dcBreak
    NULL,                         // dcCatchBlock
    &dcClass_printNode,           // dcClass
    &dcExit_printNode,            // dcExit
    &dcFalse_printNode,           // dcFalse
    &dcFlatArithmetic_printNode,  // dcFlatArithmetic
    &dcFor_printNode,             // dcFor
    NULL,                         // dcFunctionUpdate
    &dcList_printGraphDataNode,   // dcList (GraphData)
    NULL,                         // dcNode (GraphData)
    NULL,                         // dcPair (GraphData)
    &dcGraphDataTree_printNode,   // dcGraphDataTree
    &dcIdentifier_printNode,      // dcIdentifier
    &dcIf_printNode,              // dcIf
    &dcImport_printNode,          // dcImport
    &dcIn_printNode,              // dcIn
    &dcMethodCall_printNode,      // dcMethodCall
    NULL,                         // dcMethodHeader
    &dcNew_printNode,             // dcNew
    &dcNil_printNode,             // dcNil
    &dcNotEqualCall_printNode,    // dcNotEqualCall
    &dcBool_printNode,            // dcOr
    &dcPackage_printNode,         // dcPackage
    &dcReturn_printNode,          // dcReturn
    &dcSelf_printNode,            // dcSelf
    &dcSuper_printNode,           // dcSuper
    &dcSymbol_printNode,          // dcSymbol
    NULL,                         // dcSynchronized
    NULL,                         // dcThrow
    &dcTrue_printNode,            // dcTrue
    NULL,                         // dcTryBlock
    &dcUpSelf_printNode,          // dcUpSelf
    &dcWhile_printNode            // dcWhile
};

static const dcNode_setTemplatePointer sSetTemplatePointers[] =
{
    NULL,                  // dcAnd
    NULL,                  // dcAssignment
    NULL,                  // dcBreak
    NULL,                  // dcCatchBlock
    &dcClass_setTemplate,  // dcClass
    NULL,                  // dcExit
    NULL,                  // dcFalse
    NULL,                  // dcFlatArithmetic
    NULL,                  // dcFor
    NULL,                  // dcFunctionUpdate
    NULL,                  // dcList (GraphData)
    NULL,                  // dcNode (GraphData)
    NULL,                  // dcPair (GraphData)
    NULL,                  // dcGraphDataTree
    NULL,                  // dcIdentifier
    NULL,                  // dcIf
    NULL,                  // dcImport
    NULL,                  // dcIn
    NULL,                  // dcMethodCall
    NULL,                  // dcMethodHeader
    NULL,                  // dcNew
    NULL,                  // dcNil
    NULL,                  // dcNotEqualCall
    NULL,                  // dcOr
    NULL,                  // dcPackage
    NULL,                  // dcReturn
    NULL,                  // dcSelf
    NULL,                  // dcSuper
    NULL,                  // dcSymbol
    NULL,                  // dcSynchronized
    NULL,                  // dcThrow
    NULL,                  // dcTrue
    NULL,                  // dcTryBlock
    NULL,                  // dcUpSelf
    NULL                   // dcWhile
};

static const dcNode_prettyPrintPointer sPrettyPrintPointers[] =
{
    NULL,                              // dcAnd
    NULL,                              // dcAssignment
    NULL,                              // dcBreak
    NULL,                              // dcCatchBlock
    &dcClass_prettyPrintNode,          // dcClass
    NULL,                              // dcExit
    NULL,                              // dcFalse
    &dcFlatArithmetic_prettyPrintNode, // dcFlatArithmetic
    NULL,                              // dcFor
    NULL,                              // dcFunctionUpdate
    NULL,                              // dcList (GraphData)
    NULL,                              // dcNode (GraphData)
    NULL,                              // dcPair (GraphData)
    &dcGraphDataTree_prettyPrintNode,  // dcGraphDataTree
    NULL,                              // dcIdentifier
    NULL,                              // dcIf
    NULL,                              // dcImport
    NULL,                              // dcIn
    &dcMethodCall_prettyPrintNode,     // dcMethodCall
    NULL,                              // dcMethodHeader
    NULL,                              // dcMultiply
    NULL,                              // dcNew
    NULL,                              // dcNil
    NULL,                              // dcNotEqualCall
    NULL,                              // dcOr
    NULL,                              // dcPackage
    NULL,                              // dcReturn
    NULL,                              // dcSelf
    NULL,                              // dcSuper
    NULL,                              // dcSymbol
    NULL,                              // dcSynchronized
    NULL,                              // dcThrow
    NULL,                              // dcTrue
    NULL,                              // dcTryBlock
    NULL,                              // dcUpSelf
    NULL                               // dcWhile
};

static const dcNode_marshallPointer sMarshallPointers[] =
{
    &dcPair_marshallGraphDataNode,   // dcAnd
    &dcAssignment_marshallNode,      // dcAssignment
    NULL,                            // dcBreak
    &dcCatchBlock_marshallNode,      // dcCatchBlock
    &dcClass_marshallNode,           // dcClass
    &dcGraphDataNode_marshallNode,   // dcExit
    NULL,                            // dcFalse
    &dcFlatArithmetic_marshallNode,  // dcFlatArithmetic
    &dcFor_marshallNode,             // dcFor
    &dcFunctionUpdate_marshallNode,  // dcFunctionUpdate
    &dcList_marshallGraphDataNode,   // dcList (GraphData)
    &dcGraphDataNode_marshallNode,   // dcNode (GraphData)
    &dcPair_marshallGraphDataNode,   // dcPair (GraphData)
    &dcGraphDataTree_marshallNode,   // dcGraphDataTree
    &dcIdentifier_marshallNode,      // dcIdentifier
    &dcIf_marshallNode,              // dcIf
    NULL,                            // dcImport
    &dcIn_marshallNode,              // dcIn
    &dcMethodCall_marshallNode,      // dcMethodCall
    &dcMethodHeader_marshallNode,    // dcMethodHeader
    &dcGraphDataNode_marshallNode,   // dcNew
    NULL,                            // dcNil
    &dcGraphDataNode_marshallNode,   // dcNotEqualCall
    &dcPair_marshallGraphDataNode,   // dcOr
    NULL,                            // dcPackage
    &dcGraphDataNode_marshallNode,   // dcReturn
    NULL,                            // dcSelf
    NULL,                            // dcSuper
    NULL,                            // dcSymbol
    &dcPair_marshallGraphDataTree,   // dcSynchronized
    &dcGraphDataNode_marshallNode,   // dcThrow
    NULL,                            // dcTrue
    &dcTryBlock_marshallNode,        // dcTryBlock
    NULL,                            // dcUpSelf
    &dcPair_marshallGraphDataTree    // dcWhile
};

static const dcNode_unmarshallPointer sUnmarshallPointers[] =
{
    &dcPair_unmarshallGraphDataNode,   // dcAnd
    &dcAssignment_unmarshallNode,      // dcAssignment
    NULL,                              // dcBreak
    &dcCatchBlock_unmarshallNode,      // dcCatchBlock
    &dcClass_unmarshallNode,           // dcClass
    &dcGraphDataNode_unmarshallNode,   // dcExit
    NULL,                              // dcFalse
    &dcFlatArithmetic_unmarshallNode,  // dcFlatArithmetic
    &dcFor_unmarshallNode,             // dcFor
    &dcFunctionUpdate_unmarshallNode,  // dcFunctionUpdate
    &dcList_unmarshallGraphDataNode,   // dcList (GraphData)
    &dcGraphDataNode_unmarshallNode,   // dcNode (GraphData)
    &dcPair_unmarshallGraphDataNode,   // dcPair (GraphData)
    &dcGraphDataTree_unmarshallNode,   // dcGraphDataTree
    &dcIdentifier_unmarshallNode,      // dcIdentifier
    &dcIf_unmarshallNode,              // dcIf
    NULL,                              // dcImport
    &dcIn_unmarshallNode,              // dcIn
    &dcMethodCall_unmarshallNode,      // dcMethodCall
    &dcMethodHeader_unmarshallNode,    // dcMethodHeader
    &dcGraphDataNode_unmarshallNode,   // dcNew
    NULL,                              // dcNil
    &dcGraphDataNode_unmarshallNode,   // dcNotEqualCall
    &dcPair_unmarshallGraphDataNode,   // dcOr
    NULL,                              // dcPackage
    &dcGraphDataNode_unmarshallNode,   // dcReturn
    NULL,                              // dcSelf
    NULL,                              // dcSuper
    NULL,                              // dcSymbol
    &dcPair_unmarshallGraphDataTree,   // dcSynchronized
    &dcGraphDataNode_unmarshallNode,   // dcThrow
    NULL,                              // dcTrue
    &dcTryBlock_unmarshallNode,        // dcTryBlock
    NULL,                              // dcUpSelf
    &dcPair_unmarshallGraphDataTree    // dcWhile
};

static const dcNode_comparePointer sComparePointers[] =
{
    NULL,                          // dcAnd
    NULL,                          // dcAssignment
    NULL,                          // dcBreak
    NULL,                          // dcCatchBlock
    &dcClass_compareNode,          // dcClass
    NULL,                          // dcExit
    NULL,                          // dcFalse
    &dcFlatArithmetic_compareNode, // dcFlatArithmetic
    NULL,                          // dcFor
    NULL,                          // dcFunctionUpdate
    &dcGraphDataList_compareNode,  // dcList (GraphData)
    NULL,                          // dcNode (GraphData)
    NULL,                          // dcPair (GraphData)
    &dcGraphDataTree_compareNode,  // dcGraphDataTree
    &dcIdentifier_compareNode,     // dcIdentifier
    NULL,                          // dcIf
    NULL,                          // dcImport
    NULL,                          // dcIn
    &dcMethodCall_compareNode,     // dcMethodCall
    NULL,                          // dcMethodHeader
    NULL,                          // dcNew
    NULL,                          // dcNil
    NULL,                          // dcNotEqualCall
    NULL,                          // dcOr
    &dcPackage_compareNode,        // dcPackage
    NULL,                          // dcReturn
    NULL,                          // dcSelf
    NULL,                          // dcSuper
    NULL,                          // dcSymbol
    NULL,                          // dcSynchronized
    NULL,                          // dcThrow
    NULL,                          // dcTrue
    NULL,                          // dcTryBlock
    NULL,                          // dcUpSelf
    NULL                           // dcWhile
};

static const dcNode_hashPointer sHashPointers[] =
{
    NULL,                         // dcAnd
    NULL,                         // dcAssignment
    NULL,                         // dcBreak
    NULL,                         // dcCatchBlock
    &dcClass_hashNode,            // dcClass
    NULL,                         // dcExit
    NULL,                         // dcFalse
    &dcFlatArithmetic_hashNode,   // dcFlatArithmetic
    NULL,                         // dcFor
    NULL,                         // dcFunctionUpdate
    NULL,                         // dcList (GraphData)
    NULL,                         // dcNode (GraphData)
    NULL,                         // dcPair (GraphData)
    NULL,                         // dcGraphDataTree
    &dcIdentifier_hashNode,       // dcIdentifier
    NULL,                         // dcIf
    NULL,                         // dcImport
    NULL,                         // dcIn
    &dcMethodCall_hashNode,       // dcMethodCall
    NULL,                         // dcMethodHeader
    NULL,                         // dcMultiply
    NULL,                         // dcNew
    NULL,                         // dcNil
    NULL,                         // dcNotEqualCall
    NULL,                         // dcOr
    NULL,                         // dcPackage
    NULL,                         // dcReturn
    NULL,                         // dcSelf
    NULL,                         // dcSuper
    NULL,                         // dcSymbol
    NULL,                         // dcSynchronized
    NULL,                         // dcThrow
    NULL,                         // dcTrue
    NULL,                         // dcTryBlock
    NULL,                         // dcUpSelf
    NULL                          // dcWhile
};

GRAPH_DATA_SIZE_CHECKER(sComparePointers);
GRAPH_DATA_SIZE_CHECKER(sCopyPointers);
GRAPH_DATA_SIZE_CHECKER(sFreePointers);
GRAPH_DATA_SIZE_CHECKER(sMarkPointers);
GRAPH_DATA_SIZE_CHECKER(sSetTemplatePointers);
GRAPH_DATA_SIZE_CHECKER(sTypeNames);
GRAPH_DATA_SIZE_CHECKER(sRegisterPointers);
GRAPH_DATA_SIZE_CHECKER(sMarshallPointers);
GRAPH_DATA_SIZE_CHECKER(sUnmarshallPointers);

dcGraphData *dcGraphData_createWithGuts(dcGraphDataType _type, void *_guts)
{
    dcGraphData *graphData = (dcGraphData *)(dcMemory_allocate
                                             (sizeof(dcGraphData)));

    graphData->type = _type;
    graphData->next = NULL;

    // attempt to set the line number and file name from the current state
    dcLexer *currentLexer = dcParser_getLexer();

    if (currentLexer == NULL)
    {
        graphData->lineNumber = 0;
        graphData->filenameId = 0;
    }
    else
    {
        graphData->lineNumber = currentLexer->previousLineNumber;
        graphData->filenameId = currentLexer->filenameId;
    }

    // set the union, any type will do
    graphData->types.list = (dcList *)_guts;
    return graphData;
}

dcNode *dcGraphData_createNodeWithGuts(dcGraphDataType _type, void *_guts)
{
    return (dcNode_setTemplate
            (dcNode_createWithGuts
             (NODE_GRAPH_DATA, dcGraphData_createWithGuts(_type, _guts)),
             true));
}

dcGraphData *dcGraphData_create(dcGraphDataType _type)
{
    return dcGraphData_createWithGuts(_type, NULL);
}

dcNode *dcGraphData_createNode(dcGraphDataType _type)
{
    return dcGraphData_createNodeWithGuts(_type, NULL);
}

void dcGraphData_freeTree(dcNode **_node, dcDepth _depth)
{
    dcNode *node = *_node;

    if (node != NULL)
    {
        if (node->type == NODE_GRAPH_DATA)
        {
            dcNode *iterator = node;
            dcNode *next = CAST_GRAPH_DATA(iterator)->next;
            dcError_assert(! dcNode_isRegistered(iterator));
            dcGraphData_setNext(iterator, NULL);
            dcNode_free(&iterator, _depth);

            while (next != NULL)
            {
                iterator = next;
                next = (iterator->type == NODE_GRAPH_DATA
                        ? dcGraphData_getNext(iterator)
                        : NULL);

                dcError_assert(! dcNode_isRegistered(iterator));
                dcGraphData_setNext(iterator, NULL);
                dcNode_free(&iterator, _depth);
            }
        }
        else
        {
            dcNode_free(_node, _depth);
        }
    }
}

void dcGraphData_free(dcGraphData **_data, dcDepth _depth)
{
    dcGraphData *data = *_data;

    if (_depth == DC_DEEP && data->next != NULL)
    {
        dcNode_free(&(data->next), _depth);
    }

    dcMemory_free(*_data);
}

#define ASSERT_TYPE(node)                                           \
    assert(CAST_GRAPH_DATA_TYPE(node) < NUMBER_OF_GRAPHDATA_TYPES)

dcResult dcGraphDataList_compareNode(dcNode *_left,
                                     dcNode *_right,
                                     dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_FAILURE;

    if (dcGraphData_getType(_right) == NODE_GRAPH_DATA_LIST)
    {
        result = dcList_compare(CAST_GRAPH_DATA_LIST(_left),
                                CAST_GRAPH_DATA_LIST(_right),
                                _compareResult);
    }

    return result;
}

dcResult dcGraphData_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    dcGraphDataType type = CAST_GRAPH_DATA_TYPE(_node);
    return (sHashPointers[type] != NULL
            ? sHashPointers[type](_node, _hashResult)
            : TAFFY_FAILURE);
}

dcResult dcGraphData_compareNode(dcNode *_left,
                                 dcNode *_right,
                                 dcTaffyOperator *_compareResult)
{
    dcGraphDataType leftType = dcGraphData_getType(_left);
    dcGraphDataType rightType = dcGraphData_getType(_right);
    dcResult result = TAFFY_SUCCESS;

    if (leftType == rightType
        && sComparePointers[leftType] == NULL)
    {
        // there's no more to compare, so they are equal
        *_compareResult = TAFFY_EQUALS;
    }
    else if (rightType == NODE_CLASS
             && leftType != NODE_CLASS) // ??????
    {
        *_compareResult = TAFFY_GREATER_THAN;
    }
    else if (leftType == NODE_CLASS
             && rightType != NODE_CLASS) // ?? derr
    {
        *_compareResult = TAFFY_LESS_THAN;
    }
    else if (sComparePointers[leftType] != NULL)
    {
        result = sComparePointers[leftType]
            (_left, _right, _compareResult);
    }
    else
    {
        *_compareResult = TAFFY_LESS_THAN;
    }

    return result;
}

void dcGraphData_freeNode(dcNode *_node, dcDepth _depth)
{
    // CAST_GRAPH_DATA(_node) may be NULL due to a failed marshall
    if (CAST_GRAPH_DATA(_node) != NULL)
    {
        dcGraphDataType type = dcGraphData_getType(_node);
        TAFFY_DEBUG(dcError_assert(type < NODE_GRAPH_DATA_LAST));

        if (sFreePointers[type] != NULL)
        {
            // check if the union is not NULL
            if (CAST_IDENTIFIER(_node) != NULL)
            {
                sFreePointers[type](_node, _depth);
            }
        }

        dcGraphData_free(&(CAST_GRAPH_DATA(_node)), _depth);
        CAST_GRAPH_DATA(_node) = NULL;
    }
}

dcResult dcGraphData_printNode(const dcNode *_graphNode, dcString *_string)
{
    dcGraphDataType type = dcGraphData_getType(_graphNode);
    assert(type < NODE_GRAPH_DATA_LAST);
    dcResult result = TAFFY_SUCCESS;

    if (sPrintPointers[type] != NULL)
    {
        result = sPrintPointers[type](_graphNode, _string);
    }

    return result;
}

dcResult dcGraphData_prettyPrintNode(const dcNode *_graphNode,
                                     dcCharacterGraph **_graph)
{
    dcGraphDataType type = dcGraphData_getType(_graphNode);
    assert(type < NODE_GRAPH_DATA_LAST);
    dcResult result = TAFFY_SUCCESS;

    if (sPrettyPrintPointers[type] != NULL)
    {
        result = sPrettyPrintPointers[type](_graphNode, _graph);
    }
    else
    {
        result = dcGraphData_printNodeToCharacterGraph(_graphNode, _graph);
    }

    return result;
}

void dcGraphData_printTree(const dcNode *_graphNode, uint32_t _indentLevel)
{
    printf("--graph data printing graph tree--\n");
    const dcNode *iterator = _graphNode;
    size_t indentMultiplier = 0;
    size_t indentIterator = 0;

    while (iterator != NULL)
    {
        for (indentIterator = 0;
             indentIterator < _indentLevel * indentMultiplier;
             indentIterator++)
        {
            printf(" ");
        }

        printf("%s\n", dcNode_display(iterator));

        if (iterator->type == NODE_GRAPH_DATA)
        {
            iterator = dcGraphData_getNext(iterator);
        }
        else
        {
            iterator = NULL;
        }

        indentMultiplier++;
    }

    printf("--done--\n\n");
}

const char *dcGraphData_getTypeName(dcGraphDataType _type)
{
    return (_type < NODE_GRAPH_DATA_LAST
            ? sTypeNames[_type]
            : NULL);
}

dcGraphDataType dcGraphData_getType(const dcNode *_node)
{
    return CAST_GRAPH_DATA_TYPE(_node);
}

dcNode *dcGraphData_getNext(const dcNode *_graphData)
{
    return CAST_GRAPH_DATA(_graphData)->next;
}

dcGraphData *dcGraphData_castMe(dcNode *_graphDataNode)
{
    return CAST_GRAPH_DATA(_graphDataNode);
}

void dcGraphData_setPosition(dcGraphData *_graphData,
                             dcStringId _filenameId,
                             uint32_t _lineNumber)
{
    _graphData->filenameId = _filenameId;
    _graphData->lineNumber = _lineNumber;
}

uint32_t dcGraphData_getLineNumber(const dcNode *_graphDataNode)
{
    return CAST_GRAPH_DATA(_graphDataNode)->lineNumber;
}

void dcGraphData_setNext(dcNode *_graphData, dcNode *_next)
{
    CAST_GRAPH_DATA(_graphData)->next = _next;
}

dcNode *dcGraphData_copyTree(const dcNode *_template)
{
    dcNode *result = NULL;

    if (_template != NULL)
    {
        result = dcNode_copy(_template, DC_DEEP);
        dcNode *iterator = result;
        const dcNode *templateIterator = _template;

        if (_template->type == NODE_GRAPH_DATA)
        {
            templateIterator = CAST_GRAPH_DATA(_template)->next;

            while (templateIterator
                   && templateIterator->type == NODE_GRAPH_DATA)
            {
                CAST_GRAPH_DATA(iterator)->next =
                    dcNode_copy(templateIterator, DC_DEEP);
                iterator = CAST_GRAPH_DATA(iterator)->next;
                templateIterator = CAST_GRAPH_DATA(templateIterator)->next;
            }
        }
    }

    return result;
}

void dcGraphData_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcGraphDataType type = dcGraphData_getType(_from);
    dcGraphData *toData = dcGraphData_create(type);
    dcGraphData *fromData = CAST_GRAPH_DATA(_from);

    CAST_GRAPH_DATA(_to) = toData;
    assert(type < NODE_GRAPH_DATA_LAST);

    if (sCopyPointers[type] != NULL)
    {
        sCopyPointers[type](_to, _from, _depth);
    }

    toData->filenameId = fromData->filenameId;
    toData->lineNumber = fromData->lineNumber;
}

void dcGraphData_copyPosition(const dcNode *_from, dcNode *_to)
{
    dcGraphData *fromData = CAST_GRAPH_DATA(_from);
    dcGraphData_setPosition(CAST_GRAPH_DATA(_to),
                            fromData->filenameId,
                            fromData->lineNumber);
}

void dcGraphData_markNode(dcNode *_node)
{
    dcGraphDataType type = dcGraphData_getType(_node);
    TAFFY_DEBUG(assert(type < NODE_GRAPH_DATA_LAST););

    if (sMarkPointers[type] != NULL)
    {
        sMarkPointers[type](_node);
    }
}

void dcGraphData_registerNode(dcNode *_node)
{
    dcGraphDataType type = dcGraphData_getType(_node);
    dcNode_setTemplate(_node, false);
    TAFFY_DEBUG(assert(type < NODE_GRAPH_DATA_LAST););

    if (sRegisterPointers[type] != NULL)
    {
        sRegisterPointers[type](_node);
    }
}

void dcGraphData_setTemplate(dcNode *_node, bool _yesNo)
{
    if (CAST_GRAPH_DATA(_node) != NULL)
    {
        dcGraphDataType type = dcGraphData_getType(_node);
        TAFFY_DEBUG(assert(type < NODE_GRAPH_DATA_LAST););

        if (sSetTemplatePointers[type] != NULL)
        {
            sSetTemplatePointers[type](_node, _yesNo);
        }
    }
}

void dcGraphData_printNodeType(const dcNode *_node)
{
    dcGraphData_printType(dcGraphData_getType(_node));
}

void dcGraphData_printType(dcGraphDataType _type)
{
    if (_type < NODE_GRAPH_DATA_LAST)
    {
        printf("%s\n", sTypeNames[_type]);
    }
    else
    {
        printf("(invalid type %d)\n", _type);
    }
}

dcNode *dcGraphData_unmarshallTree(dcString *_stream)
{
    dcNode *that = NULL;
    dcNode *previous = NULL;
    dcNode *root = NULL;

    if (dcMarshaller_unmarshall(_stream, "n", &that))
    {
        while (that != NULL)
        {
            if (! IS_GRAPH_DATA(that))
            {
                dcNode_free(&that, DC_DEEP);
                dcNode_free(&root, DC_DEEP);
                root = NULL;
                break;
            }

            if (root == NULL)
            {
                root = that;
            }

            if (previous != NULL)
            {
                dcGraphData_setNext(previous, that);
            }

            previous = that;

            that = NULL;
            dcNode *newThat;

            if (dcMarshaller_unmarshall(_stream, "n", &newThat))
            {
                that = newThat;
            }
        }
    }

    return root;
}

dcString *dcGraphData_marshallTree(const dcNode *_node, dcString *_stream)
{
    const dcNode *iterator = _node;

    while (iterator != NULL)
    {
        _stream = dcMarshaller_marshall(_stream, "n", iterator);
        iterator = (iterator->type == NODE_GRAPH_DATA
                    ? dcGraphData_getNext(iterator)
                    : NULL);
    }

    dcString_appendCharacter(_stream, NODE_NULL_MARSHALL_CHARACTER);
    return _stream;
}

void dcGraphData_assertType(const dcNode *_node, dcGraphDataType _type)
{
    dcNode_assertType(_node, NODE_GRAPH_DATA);
    assert(dcGraphData_getType(_node) == _type);
}

bool dcGraphData_isType(const dcNode *_node, dcGraphDataType _type)
{
    return (dcGraphData_getType(_node) == _type);
}

dcString *dcGraphData_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcGraphDataType type = dcGraphData_getType(_node);
    dcString *result = dcMarshaller_marshall(_stream, "c", type);

    if (sMarshallPointers[type] != NULL)
    {
        result = sMarshallPointers[type](_node, _stream);
    }

    return result;
}

bool dcGraphData_unmarshallNode(dcNode *_node, dcString *_stream)
{
    TAFFY_DEBUG(dcError_assert(_node != NULL));
    dcGraphDataType type;
    bool result = false;

    if (dcMarshaller_unmarshall(_stream, "c", &type)
        && type < NODE_GRAPH_DATA_LAST)
    {
        result = true;
        CAST_GRAPH_DATA(_node) = dcGraphData_create(type);

        if (sUnmarshallPointers[type] != NULL)
        {
            result = sUnmarshallPointers[type](_node, _stream);
        }
    }

    if (! result)
    {
        dcMemory_free(CAST_GRAPH_DATA(_node));
    }
    else if (_node != NULL)
    {
        dcNode_setTemplate(_node, true);
    }

    return result;
}

dcResult dcGraphData_printNodeToCharacterGraph(const dcNode *_node,
                                               dcCharacterGraph **_graph)
{
    dcString *string = dcString_create();
    dcResult result = dcGraphData_printNode(_node, string);

    if (result == TAFFY_SUCCESS)
    {
        *_graph = dcCharacterGraph_createFromString(string);
    }

    dcString_free(&string, DC_DEEP);
    return result;
}
