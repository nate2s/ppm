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

#ifndef __DC_GRAPHDATA_H__
#define __DC_GRAPHDATA_H__

#include "dcDefines.h"

/**
 * Data structure for an object in an evaluation tree
 */
struct dcGraphData_t
{
    struct dcNode_t *next;

    /**
     * Various flavors this structure can take
     */
    union dcGraphDataTypes_t
    {
        struct dcAssignment_t *assignment;
        struct dcCatchBlock_t *catchBlock;
        struct dcClass_t *classData;
        struct dcFlatArithmetic_t *flatArithmetic;
        struct dcFor_t *forData;
        struct dcFunctionUpdate_t *functionUpdate;
        struct dcGraphDataTree_t *graphDataTree;
        struct dcIdentifier_t *identifier;
        struct dcIf_t *ifData;
        struct dcImport_t *import;
        struct dcIn_t *in;
        struct dcList_t *list;
        struct dcMethodCall_t *methodCall;
        struct dcMethodHeader_t *methodHeader;
        struct dcTryBlock_t *tryBlock;
        struct dcNode_t *node;
        struct dcPackage_t *package;
        struct dcPair_t *pair;
        struct dcSymbol_t *symbol;
        struct dcSynchronized_t *synchronized;
        struct dcRemoteExecution_t *remoteExecution;
    } types;

    dcGraphDataType type;
    uint32_t lineNumber;

    /**
     * @brief filenameId refers to a runtime string ID in dcSystem.
     * Its referred string can be accessed via dcString_getStringFromId()
     */
    dcStringId filenameId;
};

typedef struct dcGraphData_t dcGraphData;

#define GRAPH_DATA_TEMPLATE_FLAG 0x80
#define GRAPH_DATA_TYPE_FLAG 0x7F

//////////////
// creating //
//////////////

dcGraphData *dcGraphData_createWithGuts(dcGraphDataType _type, void *_guts);

/**
 * Creates a new dcGraphData
 * \param _type The type
 * \return A newly allocated dcGraphData with type _type
 */
dcGraphData *dcGraphData_create(dcGraphDataType _type);

/**
 * Creates a new dcGraphData-node
 * \param _type The type
 * \return A newly allocated dcGraphData-node with type _type
 */
struct dcNode_t *dcGraphData_createNode(dcGraphDataType _type);

// use wisely
struct dcNode_t *dcGraphData_createNodeWithGuts(dcGraphDataType _type,
                                                void *_data);

/////////////
// freeing //
/////////////

/**
 * Frees a dcGraphData
 * \param _graphData The dcGraphData object to free
 * \param _depth The depth of the free
 */
void dcGraphData_free(dcGraphData **_graphData, dcDepth _depth);
void dcGraphData_freeTree(struct dcNode_t **_node, dcDepth _depth);

////////////////
// displaying //
////////////////

/**
 * Prints an entire graph data tree
 * \param _graphNode The root of the tree
 * \param _indentLevel The indent of each level
 */
void dcGraphData_printTree(const struct dcNode_t *_graphNode,
                           uint32_t _indentLevel);

/**
 * Prints _node to _graph
 * \param _node A graph-data node
 * \param _graph A character graph to print to
 */
dcResult dcGraphData_printNodeToCharacterGraph
    (const struct dcNode_t *_node,
     struct dcCharacterGraph_t **_graph);

/////////////
// copying //
/////////////

void dcGraphData_copyPosition(const struct dcNode_t *_from,
                              struct dcNode_t *_to);

/**
 * Copies an entire graph data tree
 * \param _template The root of the tree to copy
 * \return A copy of the tree at _template
 */
struct dcNode_t *dcGraphData_copyTree(const struct dcNode_t *_template);

//////////////////////////
// getting and querying //
//////////////////////////

/**
 * Returns the next node in a graph data tree
 * \param _graphData The dcGraphData object to query
 * \return _graphData's next pointer
 */
struct dcNode_t *dcGraphData_getNext(const struct dcNode_t *_graphData);
dcGraphDataType dcGraphData_getType(const struct dcNode_t *_graphData);
const char *dcGraphData_getTypeName(dcGraphDataType _type);

/////////////
// setting //
/////////////

void dcGraphData_setPosition(dcGraphData *_graphData,
                             dcStringId _filenameId,
                             uint32_t _lineNumber);
/**
 * Sets the next field for a dcGraphData object
 * \param _graphData The dcGraphData object to modify
 * \param _next The next field for _graphData
 */
void dcGraphData_setNext(struct dcNode_t *_left, struct dcNode_t *_next);

/////////////////////
// debugging hooks //
/////////////////////

/**
 * Debugging hook: Prints a char* array representation of a graph data type
 * to stdout
 * \param _type The graph data type
 */
void dcGraphData_printType(dcGraphDataType _type);

/**
 * Debugging hook: Prints a char* array representation of a dcGraphData-node
 * TYPE to stdout
 * \param _type The type of the dcGraphData-node
 */
void dcGraphData_printNodeType(const struct dcNode_t *_graphDataNode);

void dcGraphData_assertType(const struct dcNode_t *_node,
                            dcGraphDataType _type);
bool dcGraphData_isType(const struct dcNode_t *_node, dcGraphDataType _type);

/////////////////
// marshalling //
/////////////////

#define GRAPHDATA_MARSHALL_SIZE 3
#define GRAPHDATA_TYPE_INDEX 0
#define GRAPHDATA_HAS_NEXT 1
#define GRAPHDATA_HAS_NO_NEXT 0

// standard functions //
COMPARE_FUNCTION(dcGraphDataList_compareNode);
COMPARE_FUNCTION(dcGraphData_compareNode);
COPY_FUNCTION(dcGraphData_copyNode);
DO_GRAPH_OPERATION_FUNCTION(dcGraphData_doGraphOperation);
FREE_FUNCTION(dcGraphData_freeNode);
HASH_FUNCTION(dcGraphData_hashNode);
MARK_FUNCTION(dcGraphData_markNode);
PRETTY_PRINT_FUNCTION(dcGraphData_prettyPrintNode);
PRINT_FUNCTION(dcGraphData_printNode);
SET_TEMPLATE_FUNCTION(dcGraphData_setTemplate);
REGISTER_FUNCTION(dcGraphData_registerNode);

struct dcNode_t *dcGraphData_unmarshallTree(struct dcString_t *_stream);
struct dcString_t *dcGraphData_marshallTree(const struct dcNode_t *_node,
                                            struct dcString_t *_stream);

/////////////
// defines //
/////////////

#define CAST_GRAPH_DATA_TYPES(_node) CAST_GRAPH_DATA(_node)->types

#define CAST_ASSIGNMENT(_node)      CAST_GRAPH_DATA_TYPES(_node).assignment
#define CAST_CATCHBLOCK(_node)      CAST_GRAPH_DATA_TYPES(_node).catchBlock
#define CAST_IDENTIFIER(_node)      CAST_GRAPH_DATA_TYPES(_node).identifier
#define CAST_FLAT_ARITHMETIC(_node) CAST_GRAPH_DATA_TYPES(_node).flatArithmetic
#define CAST_GRAPH_DATA_PAIR(_node) CAST_GRAPH_DATA_TYPES(_node).pair
#define CAST_GRAPH_DATA_NODE(_node) CAST_GRAPH_DATA_TYPES(_node).node
#define CAST_GRAPH_DATA_LIST(_node) CAST_GRAPH_DATA_TYPES(_node).list
#define CAST_GRAPH_DATA_TREE(_node) CAST_GRAPH_DATA_TYPES(_node).graphDataTree
#define CAST_METHOD_CALL(_node)     CAST_GRAPH_DATA_TYPES(_node).methodCall
#define CAST_METHOD_HEADER(_node)   CAST_GRAPH_DATA_TYPES(_node).methodHeader
#define CAST_NUMBERTEMPLATE(_node)                  \
    CAST_GRAPH_DATA_TYPES(_node).numberTemplate
#define CAST_SYMBOL(_node)          CAST_GRAPH_DATA_TYPES(_node).symbol
#define CAST_TRYBLOCK(_node)        CAST_GRAPH_DATA_TYPES(_node).tryBlock

// class //
#define CAST_CLASS(_node) CAST_GRAPH_DATA_TYPES(_node).classData

// keywords //
#define CAST_ADD(_node)            CAST_GRAPH_DATA_PAIR(_node)
#define CAST_AND(_node)            CAST_GRAPH_DATA_PAIR(_node)
#define CAST_ARITHMETIC(_node)     CAST_GRAPH_DATA_PAIR(_node)
#define CAST_ATTACH(_node)         CAST_GRAPH_DATA_NODE(_node)
#define CAST_BREAK(_node)          CAST_GRAPH_DATA_NODE(_node)
#define CAST_EXIT(_node)           CAST_GRAPH_DATA_NODE(_node)
#define CAST_FOR(_node)            CAST_GRAPH_DATA_TYPES(_node).forData
#define CAST_FUNCTION_UPDATE(node) CAST_GRAPH_DATA(node)->types.functionUpdate
#define CAST_IF(_node)             CAST_GRAPH_DATA_TYPES(_node).ifData
#define CAST_IMPORT(node)          CAST_PACKAGE(node)
#define CAST_IN(_node)             CAST_GRAPH_DATA_TYPES(_node).in
#define CAST_JOIN(_node)           CAST_GRAPH_DATA_NODE(_node)
#define CAST_JOINLIB(_node)        CAST_GRAPH_DATA_NODE(_node)
#define CAST_MULTIPLY(_node)       CAST_GRAPH_DATA_PAIR(_node)
#define CAST_NEW(_node)            CAST_GRAPH_DATA_NODE(_node)
#define CAST_OR(_node)             CAST_GRAPH_DATA_PAIR(_node)
#define CAST_PACKAGE(_node)        CAST_GRAPH_DATA_TYPES(_node).package
#define CAST_RETURN(_node)         CAST_GRAPH_DATA_NODE(_node)
#define CAST_SUBTRACT(_node)       CAST_GRAPH_DATA_PAIR(_node)
#define CAST_THROW(_node)          CAST_GRAPH_DATA_NODE(_node)
#define CAST_UNLESS(_node)         CAST_GRAPH_DATA_PAIR(_node)
#define CAST_WHILE(_node)          CAST_GRAPH_DATA_PAIR(_node)
#define CAST_LEFT_SHIFT(_node)     CAST_GRAPH_DATA_PAIR(_node)
#define CAST_RIGHT_SHIFT(_node)    CAST_GRAPH_DATA_PAIR(_node)

#define CAST_REMOTE_EXECUTION(_node)               \
    CAST_GRAPH_DATA_TYPES(_node).remoteExecution
#define CAST_NOTEQUALCALL(_node)  CAST_GRAPH_DATA_NODE(_node)
#define CAST_GRAPH_DATA_TYPE(_node)                          \
    (CAST_GRAPH_DATA(_node)->type)

// is //
#define IS_AND(_node)        (CAST_GRAPH_DATA_TYPE(_node) == NODE_AND)
#define IS_ASSIGNMENT(_node) (CAST_GRAPH_DATA_TYPE(_node) == NODE_ASSIGNMENT)
#define IS_ATTACH(_node)     (CAST_GRAPH_DATA_TYPE(_node) == NODE_ATTACH)
#define IS_BREAK(_node)      (CAST_GRAPH_DATA_TYPE(_node) == NODE_BREAK)
#define IS_CATCHBLOCK(_node) (CAST_GRAPH_DATA_TYPE(_node) == NODE_CATCH_BLOCK)
#define IS_GRAPH_DATA_LIST(_node)                           \
    (CAST_GRAPH_DATA_TYPE(_node) == NODE_GRAPH_DATA_LIST)
#define IS_CLASS(_node)                             \
    ((_node)->type == NODE_GRAPH_DATA               \
     && CAST_GRAPH_DATA_TYPE(_node) == NODE_CLASS)
#define IS_EXCEPTIONBLOCK(_node)                            \
    (CAST_GRAPH_DATA_TYPE(_node) == NODE_EXCEPTIONBLOCK)
#define IS_EXIT(_node)           (CAST_GRAPH_DATA_TYPE(_node) == NODE_EXIT)
#define IS_FLAT_ARITHMETIC(_node)                           \
    (((_node)->type == NODE_GRAPH_DATA)                     \
     && dcGraphData_getType(_node) == NODE_FLAT_ARITHMETIC)
#define IS_FUNCTION_UPDATE(_node)                           \
    (CAST_GRAPH_DATA_TYPE(_node) == NODE_FUNCTION_UPDATE)
#define IS_IDENTIFIER(_node)                            \
    (IS_GRAPH_DATA(_node)                               \
     && CAST_GRAPH_DATA_TYPE(_node) == NODE_IDENTIFIER)
#define IS_IF(_node)             (CAST_GRAPH_DATA_TYPE(_node) == NODE_IF)
#define IS_JOIN(_node)           (CAST_GRAPH_DATA_TYPE(_node) == NODE_JOIN)
#define IS_JOINLIB(_node)        (CAST_GRAPH_DATA_TYPE(_node) == NODE_JOINLIB)
#define IS_METHOD_CALL(_node)    (CAST_GRAPH_DATA_TYPE(_node) == NODE_METHOD_CALL)
#define IS_METHODHEADER(_node)   (CAST_GRAPH_DATA_TYPE(_node) == NODE_METHOD_HEADER)
#define IS_NIL(_node)            (CAST_GRAPH_DATA_TYPE(_node) == NODE_NIL)
#define IS_NUMBERTEMPLATE(_node) (CAST_GRAPH_DATA_TYPE(_node) == NODE_NUMBER_TEMPLATE)
#define IS_OR(_node)             (CAST_GRAPH_DATA_TYPE(_node) == NODE_OR)
#define IS_RETURN(_node)         (CAST_GRAPH_DATA_TYPE(_node) == NODE_RETURN)
#define IS_SYMBOL(_node)         (CAST_GRAPH_DATA_TYPE(_node) == NODE_SYMBOL)
#define IS_THROW(_node)          (CAST_GRAPH_DATA_TYPE(_node) == NODE_THROW)
#define IS_UNLESS(_node)         (CAST_GRAPH_DATA_TYPE(_node) == NODE_UNLESS)
#define IS_WHILE(_node)          (CAST_GRAPH_DATA_TYPE(_node) == NODE_WHILE)

extern const dcNode_unmarshallPointer gGraphData_unmarshallPointers[];
extern const dcNode_marshallPointer gGraphData_marshallPointers[];

dcGraphDataMathOperation dcGraphData_getMathOperation(uint16_t _operation);

#define GRAPH_DATA_SIZE_CHECKER(_name_)                                 \
    int my##_name_[dcTaffy_countOf(_name_)                              \
                   == NUMBER_OF_GRAPHDATA_TYPES                         \
                   ? 1                                                  \
                   : -1] = {0}

struct dcString_t *dcGraphData_marshallNode(const struct dcNode_t *_node,
                                            struct dcString_t *_stream);
bool dcGraphData_unmarshallNode(struct dcNode_t *_node,
                                  struct dcString_t *_stream);

uint32_t dcGraphData_getLineNumber(const struct dcNode_t *_graphDataNode);

#endif
