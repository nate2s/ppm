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

#ifndef __DC_NODE_H__
#define __DC_NODE_H__

#include "dcDefines.h"

// used for node->garbageCollectionFlags //
enum dcNodeGarbageCollectionFlag_e
{
    NODE_MARKING                    = BITS(0),
    NODE_MARKED                     = BITS(1),
    NODE_REGISTERED                 = BITS(2),
    NODE_TEMPLATE                   = BITS(3),
    NODE_GARBAGE_COLLECTION_TRAPPED = BITS(4),
    NODE_FREE_TRAPPED               = BITS(5),
    NODE_REGISTER_TRAPPED           = BITS(6)
};

typedef uint8_t dcNodeGarbageCollectionFlag;

// dcNode definition //
struct dcNode_t
{
    union dcNodeTypes_t
    {
        // primitives //
        uint32_t intData;
        int32_t signedInt32;
        uint64_t int64Data;
        float floatValue;
        dcTaffyCMethodPointer taffyCMethod;
        struct dcTaffyCMethodWrapper_t *taffyCMethodWrapper;

        // maths //
        struct dcMatrix_t *matrix;
        struct dcNumber_t *number;
        struct dcComplexNumber_t *complexNumber;

        // containers //
        struct dcArray_t *array;
        struct dcHash_t *hash;
        struct dcHashElement_t *hashElement;
        struct dcHeap_t *heap;
        struct dcList_t *list;
        struct dcPair_t *pair;
        struct dcTree_t *tree;
        struct dcTreeElement_t *treeElement;

        // scope  //
        struct dcScope_t *scope;
        struct dcScopeData_t *scopeData;
        struct dcObjectStack_t *objectStack;

        struct dcCallStackData_t *callStackData;
        struct dcExecutorLoad_t *executorLoad;
        struct dcGraphData_t *graphData;
        struct dcLexer_t *lexer;
        struct dcLexResult_t *lexResult;
        struct dcSocket_t *socket;
        struct dcString_t *string;
        struct dcTemplateData_t *templateData;

        // marker //
        dcTaffy_rootMarkPointer rootMarker;
        struct dcNodeEvaluator_t *nodeEvaluator;

        struct dcClassTemplate_t *classTemplate;
        struct dcPackageContents_t *packageContents;
        struct dcFilePackageData_t *filePackageData;
        struct dcCommandLineArgument_t *commandLineArgument;

        // a const void pointer that does not need marking or
        // garbage collecting
        const void *constVoid;

        // a void pointer to some block of memory, use wisely
        void *voidPointer;
        // a double-void pointer to some block of memory, use wiselier
        void **doubleVoidPointer;
    } types;

    /**
     * The type of the types union above
     */
    dcNodeType type;

    /**
     * The garbage collection flags
     */
    dcNodeGarbageCollectionFlag flags;

    // backtraces for registration and free
    // helpful for determining original location of register and free
    TAFFY_DEBUG(struct dcList_t *creationBacktraceList;
                struct dcList_t *registerBacktraceList;
                struct dcList_t *freeBacktraceList);
};

typedef struct dcNode_t dcNode;

/*
 * @brief Create a node with a given type
 *        This function calls dcNode_createWithFlags with NO_FLAGS as the flags
 *        argument.
 * @return A node of given type
 */
dcNode *dcNode_create(dcNodeType _type);

/*
 * @brief Create a node with a given type, contents
 *        This function calls dcNode_create, and then populates its
 *        contents
 * @return A node of given type and flags
 */
dcNode *dcNode_createWithGuts(dcNodeType _type, void *_guts);

// marking //
void dcNode_mark(dcNode *_node);
void dcNode_markYesNo(dcNode *_node, bool _yesno);

// registering //
dcNode *dcNode_register(dcNode *_node);

// copying //
dcNode *dcNode_copy(const dcNode *_from, dcDepth _depth);
dcNode *dcNode_copyAndTemplate(const dcNode *_from);
dcNode *dcNode_tryCopy(const dcNode *_from, dcDepth _depth);
dcNode *dcNode_copyIfTemplate(dcNode *_node);

// displaying //
char *dcNode_display(const dcNode *_node);
char *dcNode_prettyDisplay(const dcNode *_node);
char *dcNode_displayWithNoGarbageCollection(const dcNode *_node);
dcResult dcNode_print(const dcNode *_node, struct dcString_t *_string);
dcResult dcNode_prettyPrint(const dcNode *_node,
                            struct dcCharacterGraph_t **_graph);
dcResult dcNode_printToCharacterGraph(const dcNode *_node,
                                      struct dcCharacterGraph_t **_graph);

/*
 * @brief Display a node. API function. The caller is responsible for freeing
 * the result.
 */
char *dcNode_synchronizedDisplay(const dcNode *_node);

//
// dcNode_compare() returns TAFFY_SUCCESS upon a successful compare
// it MUST return TAFFY_EXCEPTION when an exception occurred
// it MUST not return TAFFY_FAILURE under any circumstances (it's a bug if the
// node can't be compared)
//
// possible comparison results:
//
// TAFFY_EQUALS, TAFFY_LESS_THAN, TAFFY_GREATER_THAN, TAFFY_UNKNOWN_COMPARISON
//
dcResult dcNode_compare(dcNode *_left,
                        dcNode *_right,
                        dcTaffyOperator *_comparison);

// if _left and _right are classes, then just test if they are equal
// if either isn't a class, then run dcNode_compare()
dcResult dcNode_compareEqual(dcNode *_left,
                             dcNode *_right,
                             dcTaffyOperator *_comparison);

/*
 * @brief Debug hook. Must not generate an exception.
 */
dcTaffyOperator dcNode_easyCompare(dcNode *_left, dcNode *_right);

dcHashType dcNode_easyHash(dcNode *_node);

dcResult dcNode_comparePointers(dcNode *_left,
                                dcNode *_right,
                                dcTaffyOperator *_compareResult);

// getting/querying //
bool dcNode_isMarking(const dcNode *_node);
bool dcNode_isMarked(const dcNode *_node);
bool dcNode_isContainer(const dcNode *_node);
bool dcNode_isRegistered(const dcNode *_node);
bool dcNode_isTemplate(const dcNode *_node);
bool dcNode_isGarbageCollectionTrapped(const dcNode *_node);
bool dcNode_isFreeTrapped(const dcNode *_node);
bool dcNode_isRegisterTrapped(const dcNode *_node);
dcResult dcNode_hash(dcNode *_node, dcHashType *_hashValue);

// setting //
void dcNode_setMarking(dcNode *_node, bool _yesNo);
void dcNode_setMarked(dcNode *_node, bool _yesNo);
void dcNode_setRegistered(dcNode *_node, bool _yesNo);
dcNode *dcNode_setTemplate(dcNode *_node, bool _yesNo);
void dcNode_setGarbageCollectionTrapped(dcNode *_node, bool _yesNo);
void dcNode_setFreeTrapped(dcNode *_node, bool _yesNo);
void dcNode_setRegisterTrapped(dcNode *_node, bool _yesNo);
void dcNode_trackCreation(dcNode *_node);

// freeing //
void dcNode_free(dcNode **_node, dcDepth _depth);
void dcNode_freeShell(dcNode **_node);
dcNode *dcNode_tryFree(dcNode **_node, dcDepth _depth);

// taffy misc //
bool dcTaffy_checkNullPrint(const void *_data, struct dcString_t *_output);

/////////////////
// marshalling //
/////////////////

#define NODE_NULL_MARSHALL_CHARACTER 0xFF

// api functions //
struct dcString_t *dcNode_marshall(const dcNode *_node,
                                   struct dcString_t *_string);
dcNode *dcNode_unmarshall(struct dcString_t *_stream);

/////////////////////
// debugging hooks //
/////////////////////

const char *dcNode_getTypeString(dcNodeType _type);
const char *dcNode_getNodeTypeString(const dcNode *_node);
void dcNode_printNodeType(const dcNode *_node);
void dcNode_printType(dcNodeType _type);

///////////////
// asserting //
///////////////

void dcNode_assertType(const dcNode *_node, dcNodeType _type);
void dcNode_assertEqual(dcNode *_left, dcNode *_right);

/////////////
// defines //
/////////////

#define CAST_TYPES(node) (node)->types

////////////
// issing //
////////////

#define HAS_TYPE(_node, _type) ((_node)->type == _type)
#define IS_GRAPH_DATA(_node) HAS_TYPE(_node, NODE_GRAPH_DATA)

/////////////
// casting //
/////////////

#define CAST_ARRAY(_node_)                  CAST_TYPES(_node_).array
#define CAST_CALL_STACK_DATA(_node_)        CAST_TYPES(_node_).callStackData
#define CAST_COMPLEX_NUMBER(_node_)         CAST_TYPES(_node_).complexNumber
#define CAST_EXECUTORLOAD(_node_)           CAST_TYPES(_node_).executorLoad
#define CAST_FLOAT(_node_)                  CAST_TYPES(_node_).floatValue
#define CAST_GRAPH_DATA(_node_)             CAST_TYPES(_node_).graphData
#define CAST_HASH(_node_)                   CAST_TYPES(_node_).hash
#define CAST_HASH_ELEMENT(_node_)           CAST_TYPES(_node_).hashElement
#define CAST_HEAP(_node_)                   CAST_TYPES(_node_).heap
#define CAST_INT(_node_)                    CAST_TYPES(_node_).intData
#define CAST_SIGNED_INT_32(_node_)          CAST_TYPES(_node_).signedInt32
#define CAST_INT_64(_node_)                 CAST_TYPES(_node_).int64Data
#define CAST_LEXER(_node_)                  CAST_TYPES(_node_).lexer
#define CAST_LEXRESULT(_node_)              CAST_TYPES(_node_).lexResult
#define CAST_LIST(_node_)                   CAST_TYPES(_node_).list
#define CAST_MATRIX(_node_)                 CAST_TYPES(_node_).matrix
#define CAST_NODE_EVALUATOR(_node_)         CAST_TYPES(_node_).nodeEvaluator
#define CAST_NUMBER(_node_)                 CAST_TYPES(_node_).number
#define CAST_OBJECT_STACK(_node_)           CAST_TYPES(_node_).objectStack
#define CAST_PACKAGE_CONTENTS(_node_)       CAST_TYPES(_node_).packageContents
#define CAST_FILE_PACKAGE_DATA(_node_)      CAST_TYPES(_node_).filePackageData
#define CAST_PAIR(_node_)                   CAST_TYPES(_node_).pair
#define CAST_SCOPE(_node_)                  CAST_TYPES(_node_).scope
#define CAST_SCOPE_DATA(_node_)             CAST_TYPES(_node_).scopeData
#define CAST_SOCKET(_node_)                 CAST_TYPES(_node_).socket
#define CAST_STRING(_node_)                 CAST_TYPES(_node_).string
#define CAST_TAFFY_C_METHOD_POINTER(_node_) CAST_TYPES(_node_).taffyCMethod
#define CAST_TREE(_node_)                   CAST_TYPES(_node_).tree
#define CAST_TREEELEMENT(_node_)            CAST_TYPES(_node_).treeElement
#define CAST_TEMPLATEDATA(_node_)           CAST_TYPES(_node_).templateData
#define CAST_VOID(_node_)                   CAST_TYPES(_node_).voidPointer
#define CAST_DOUBLE_VOID(_node_)            CAST_TYPES(_node_).doubleVoidPointer

#define CAST_ROOT_MARK_FUNCTION(_node_)         \
    CAST_TYPES(_node_).rootMarker

#define CAST_COMMAND_LINE_ARGUMENT(_node_)      \
    CAST_TYPES(_node_).commandLineArgument

#define CAST_TAFFY_C_METHOD_WRAPPER(_node_)     \
    CAST_TYPES(_node_).taffyCMethodWrapper

#define CAST_CLASS_TEMPLATE(_node_)     \
    CAST_TYPES(_node_).classTemplate

#define MARSHALLED_NODE 30

#endif
