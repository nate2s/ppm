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

#include "dcNode.h"
#include "dcCallStackData.h"
#include "dcCharacterGraph.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcCommandLineArgument.h"
#include "dcComplexNumber.h"
#include "dcContainers.h"
#include "dcDoubleVoid.h"
#include "dcError.h"
#include "dcFloat.h"
#include "dcFilePackageData.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcInt32.h"
#include "dcLexer.h"
#include "dcMarshaller.h"
#include "dcMatrix.h"
#include "dcMemory.h"
#include "dcNodeEvaluator.h"
#include "dcNumber.h"
#include "dcPackageContents.h"
#include "dcRootMarkFunction.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcObjectStack.h"
#include "dcSocket.h"
#include "dcString.h"
#include "dcSystem.h"
#include "dcThread.h"
#include "dcTaffyCMethodPointer.h"
#include "dcUnsignedInt32.h"
#include "dcUnsignedInt64.h"
#include "dcVoid.h"
#include "dcVoidContainer.h"

//
// Function pointers for standard functions
//

/////////////
// Copying //
/////////////

static const dcNode_copyPointer sCopyPointers[] =
{
    NULL,                                // NODE_NONE
    &dcInt32_copyNode,                   // dcInt32
    &dcUnsignedInt32_copyNode,           // dcUnsignedInt32
    &dcUnsignedInt64_copyNode,           // dcUnsignedInt64
    &dcFloat_copyNode,                   // dcFloat
    &dcTaffyCMethodPointer_copyNode,     // dcTaffyCMethodPointer
    &dcMatrix_copyNode,                  // dcMatrix
    &dcNumber_copyNode,                  // dcNumber
    &dcComplexNumber_copyNode,           // dcComplexNumber
    &dcArray_copyNode,                   // dcArray
    &dcHash_copyNode,                    // dcHash
    &dcHashElement_copyNode,             // dcHashElement
    NULL,                                // dcHeap
    &dcList_copyNode,                    // dcList
    &dcPair_copyNode,                    // dcPair
    NULL,                                // dcTree
    NULL,                                // dcTreeElement
    &dcScope_copyNode,                   // dcScope
    &dcScopeData_copyNode,               // dcScopeData
    NULL,                                // dcObjectStack
    &dcCallStackData_copyNode,           // dcCallStackData
    &dcGraphData_copyNode,               // dcGraphData
    &dcLexResult_copyNode,               // dcLexResult
    NULL,                                // dcSocket
    &dcString_copyNode,                  // dcString
    NULL,                                // dcNodeEvaluator
    NULL,                                // dcRootMarkFunction
    NULL,                                // dcClassTemplate
    NULL,                                // dcPackageContents
    NULL,                                // dcFilePackageData
    NULL,                                // dcCommandLineArgument
    NULL,                                // const void *
    &dcVoid_copyNode,                    // void *
    &dcVoid_copyNode,                    // void * container
    NULL,                                // void **
};

/////////////
// Marking //
/////////////

static const dcNode_markPointer sMarkPointers[] =
{
    NULL,                         // NODE_NONE
    NULL,                         // dcInt32
    NULL,                         // dcUnsignedInt32
    NULL,                         // dcUnsignedInt64
    NULL,                         // dcFloat
    NULL,                         // dcTaffyCMethodPointer
    NULL,                         // dcMatrix
    NULL,                         // dcNumber
    NULL,                         // dcComplexNumber
    &dcArray_markNode,            // dcArray
    &dcHash_markNode,             // dcHash
    &dcHashElement_markNode,      // dcHashElement
    &dcHeap_markNode,             // dcHeap
    &dcList_markNode,             // dcList
    &dcPair_markNode,             // dcPair
    NULL,                         // dcTree
    NULL,                         // dcTreeElement
    &dcScope_markNode,            // dcScope
    &dcScopeData_markNode,        // dcScopeData
    &dcObjectStack_markNode,      // dcObjectStack
    NULL,                         // dcCallStackData
    &dcGraphData_markNode,        // dcGraphData
    NULL,                         // dcLexResult
    NULL,                         // dcSocket
    NULL,                         // dcString
    &dcNodeEvaluator_markNode,    // dcNodeEvaluator
    &dcRootMarkFunction_markNode, // dcRootMarkFunction
    NULL,                         // dcClassTemplate
    NULL,                         // dcPackageContents
    NULL,                         // dcFilePackageData
    NULL,                         // dcCommandLineArgument
    NULL,                         // const void *
    NULL,                         // void *
    NULL,                         // void * container
    NULL                          // void **
};

//////////////
// Printing //
//////////////

static const dcNode_printPointer sPrintPointers[] =
{
    NULL,                                 // NODE_NONE
    &dcInt32_printNode,                   // dcInt32
    &dcUnsignedInt32_printNode,           // dcUnsignedInt32
    &dcUnsignedInt64_printNode,           // dcUnsignedInt64
    &dcFloat_printNode,                   // dcFloat
    NULL,                                 // dcTaffyCMethodPointer
    NULL,                                 // dcMatrix
    NULL,                                 // dcNumber
    NULL,                                 // dcComplexNumber
    &dcArray_printNode,                   // dcArray
    &dcHash_printNode,                    // dcHash
    &dcHashElement_printNode,             // dcHashElement
    NULL,                                 // dcHeap
    &dcList_printNode,                    // dcList
    &dcPair_printNode,                    // dcPair
    NULL,                                 // dcTree
    NULL,                                 // dcTreeElement
    &dcScope_printNode,                   // dcScope
    &dcScopeData_printNode,               // dcScopeData
    &dcObjectStack_printNode,             // dcObjectStack
    &dcCallStackData_printNode,           // dcCallStackData
    &dcGraphData_printNode,               // dcGraphData
    NULL,                                 // dcLexResult
    NULL,                                 // dcSocket
    &dcString_printNode,                  // dcString
    NULL,                                 // dcNodeEvaluator
    NULL,                                 // dcRootMarkFunction
    NULL,                                 // dcClassTemplate
    &dcPackageContents_printNode,         // dcPackageContents
    &dcFilePackageData_printNode,         // dcFilePackageData
    &dcCommandLineArgument_printNode,     // dcCommandLineArgument
    NULL,                                 // const void *
    &dcVoid_printNode,                    // void *
    &dcVoidContainer_printNode,           // void * container
    NULL                                  // void **
};

/////////////////////
// Pretty Printing //
/////////////////////

static const dcNode_prettyPrintPointer sPrettyPrintPointers[] =
{
    NULL,                                 // NODE_NONE
    NULL,                                 // dcInt32
    NULL,                                 // dcUnsignedInt32
    NULL,                                 // dcUnsignedInt64
    NULL,                                 // dcFloat
    NULL,                                 // dcTaffyCMethodPointer
    NULL,                                 // dcMatrix
    NULL,                                 // dcNumber
    NULL,                                 // dcComplexNumber
    NULL,                                 // dcArray
    NULL,                                 // dcHash
    NULL,                                 // dcHashElement
    NULL,                                 // dcHeap
    NULL,                                 // dcList
    NULL,                                 // dcPair
    NULL,                                 // dcTree
    NULL,                                 // dcTreeElement
    NULL,                                 // dcScope
    NULL,                                 // dcScopeData
    NULL,                                 // dcObjectStack
    NULL,                                 // dcCallStackData
    &dcGraphData_prettyPrintNode,         // dcGraphData
    NULL,                                 // dcLexResult
    NULL,                                 // dcSocket
    NULL,                                 // dcString
    NULL,                                 // dcNodeEvaluator
    NULL,                                 // dcRootMarkFunction
    NULL,                                 // dcClassTemplate
    NULL,                                 // dcPackageContents
    NULL,                                 // dcFilePackageData
    NULL,                                 // dcCommandLineArgument
    NULL,                                 // const void *
    NULL,                                 // void *
    NULL,                                 // void * container
    NULL                                  // void **
};

/////////////
// Freeing //
/////////////

static const dcNode_freePointer sFreePointers[] =
{
    NULL,                                // NODE_NONE
    NULL,                                // dcInt32
    NULL,                                // dcUnsignedInt32
    NULL,                                // dcUnsignedInt64
    NULL,                                // dcFloat
    NULL,                                // dcTaffyCMethodPointer
    &dcMatrix_freeNode,                  // dcMatrix
    &dcNumber_freeNode,                  // dcNumber
    &dcComplexNumber_freeNode,           // dcComplexNumber
    &dcArray_freeNode,                   // dcArray
    &dcHash_freeNode,                    // dcHash
    &dcHashElement_freeNode,             // dcHashElement
    &dcHeap_freeNode,                    // dcHeap
    &dcList_freeNode,                    // dcList
    &dcPair_freeNode,                    // dcPair
    NULL,                                // dcTree
    NULL,                                // dcTreeElement
    &dcScope_freeNode,                   // dcScope
    &dcScopeData_freeNode,               // dcScopeData
    &dcObjectStack_freeNode,             // dcObjectStack
    &dcCallStackData_freeNode,           // dcCallStackData
    &dcGraphData_freeNode,               // dcGraphData
    &dcLexResult_freeNode,               // dcLexResult
    &dcSocket_freeNode,                  // dcSocket
    &dcString_freeNode,                  // dcString
    &dcNodeEvaluator_freeNode,           // dcNodeEvaluator
    NULL,                                // dcRootMarkFunction
    &dcClassTemplate_freeNode,           // dcClassTemplate
    &dcPackageContents_freeNode,         // dcPackageContents
    &dcFilePackageData_freeNode,         // dcFilePackageData
    &dcCommandLineArgument_freeNode,     // dcCommandLineArgument
    NULL,                                // const void *
    &dcVoid_freeNode,                    // void *
    NULL,                                // void * container
    &dcDoubleVoid_freeNode,              // void **
};

/////////////////
// Registering //
/////////////////

static const dcNode_registerPointer sRegisterPointers[] =
{
    NULL,                        // NODE_NONE
    NULL,                        // dcInt32
    NULL,                        // dcUnsignedInt32
    NULL,                        // dcUnsignedInt64
    NULL,                        // dcFloat
    NULL,                        // dcTaffyCMethodPointer
    NULL,                        // dcMatrix
    NULL,                        // dcNumber
    NULL,                        // dcComplexNumber
    &dcArray_registerNode,       // dcArray
    &dcHash_registerNode,        // dcHash
    &dcHashElement_registerNode, // dcHashElement
    &dcHeap_registerNode,        // dcHeap
    &dcList_registerNode,        // dcList
    &dcPair_registerNode,        // dcPair
    NULL,                        // dcTree
    NULL,                        // dcTreeElement
    NULL,                        // dcScope
    &dcScopeData_registerNode,   // dcScopeData
    NULL,                        // dcObjectStack
    NULL,                        // dcCallStackData
    &dcGraphData_registerNode,   // dcGraphData
    NULL,                        // dcLexResult
    NULL,                        // dcSocket
    NULL,                        // dcString
    NULL,                        // dcNodeEvaluator
    NULL,                        // dcRootMarkFunction
    NULL,                        // dcClassTemplate
    NULL,                        // dcPackageContents
    NULL,                        // dcFilePackageData
    NULL,                        // dcCommandLineArgument
    NULL,                        // const void *
    NULL,                        // void *
    NULL,                        // void * container
    NULL                         // void **
};

/////////////////
// Marshalling //
/////////////////

static const dcNode_marshallPointer sMarshallPointers[] =
{
    NULL,                          // NODE_NONE
    &dcInt32_marshallNode,         // dcInt32s
    &dcUnsignedInt32_marshallNode, // dcUnsignedInt32
    &dcUnsignedInt64_marshallNode, // dcUnsignedInt64
    &dcFloat_marshallNode,         // dcFloat
    NULL,                          // dcTaffyCMethodPointer
    NULL,                          // dcMatrix
    NULL,                          // dcNumber
    NULL,                          // dcComplexNumber
    &dcArray_marshallNode,         // dcArray
    &dcHash_marshallNode,          // dcHash
    NULL,                          // dcHashElement
    NULL,                          // dcHeap
    &dcList_marshallNode,          // dcList
    &dcPair_marshallNode,          // dcPair
    NULL,                          // dcTree
    NULL,                          // dcTreeElement
    &dcScope_marshallNode,         // dcScope
    &dcScopeData_marshallNode,     // dcScopeData
    NULL,                          // dcObjectStack
    NULL,                          // dcCallStackData
    &dcGraphData_marshallNode,     // dcGraphData
    NULL,                          // dcLexResult
    NULL,                          // dcSocket
    &dcString_marshallNode,        // dcString
    NULL,                          // dcNodeEvaluator
    NULL,                          // dcRootMarkFunction
    NULL,                          // dcClassTemplate
    NULL,                          // dcPackageContents
    NULL,                          // dcFilePackageData
    NULL,                          // dcCommandLineArgument
    NULL,                          // const void *
    NULL,                          // void *
    NULL,                          // void * container
    NULL                           // void **
};

///////////////////
// Unmarshalling //
///////////////////

static const dcNode_unmarshallPointer sUnmarshallPointers[] =
{
    NULL,                            // NODE_NONE
    &dcInt32_unmarshallNode,         // dcInt32
    &dcUnsignedInt32_unmarshallNode, // dcUnsignedInt32
    &dcUnsignedInt64_unmarshallNode, // dcUnsignedInt64
    &dcFloat_unmarshallNode,         // dcFloat
    NULL,                            // dcTaffyCMethodPointer
    NULL,                            // dcMatrix
    NULL,                            // dcNumber
    NULL,                            // dcComplexNumber
    &dcArray_unmarshallNode,         // dcArray
    &dcHash_unmarshallNode,          // dcHash
    NULL,                            // dcHashElement
    NULL,                            // dcHeap
    &dcList_unmarshallNode,          // dcList
    &dcPair_unmarshallNode,          // dcPair
    NULL,                            // dcTree
    NULL,                            // dcTreeElement
    &dcScope_unmarshallNode,         // dcScope
    &dcScopeData_unmarshallNode,     // dcScopeData
    NULL,                            // dcObjectStack
    NULL,                            // dcCallStackData
    &dcGraphData_unmarshallNode,     // dcGraphData
    NULL,                            // dcLexResult
    NULL,                            // dcSocket
    &dcString_unmarshallNode,        // dcString
    NULL,                            // dcNodeEvaluator
    NULL,                            // dcRootMarkFunction
    NULL,                            // dcClassTemplate
    NULL,                            // dcPackageContents
    NULL,                            // dcFilePackageData
    NULL,                            // dcCommandLineArgument
    NULL,                            // const void *
    NULL,                            // void *
    NULL,                            // void * container
    NULL                             // void **
};

////////////////
// Comaparing //
////////////////

static const dcNode_comparePointer sComparePointers[] =
{
    NULL,                                   // NODE_NONE
    &dcInt32_compareNode,                   // dcInt32
    &dcUnsignedInt32_compareNode,           // dcUnsignedInt32
    &dcUnsignedInt64_compareNode,           // dcUnsignedInt64
    &dcFloat_compareNode,                   // dcFloat
    NULL,                                   // dcTaffyCMethodPointer
    NULL,                                   // dcMatrix
    NULL,                                   // dcNumber
    NULL,                                   // dcComplexNumber
    NULL,                                   // dcArray
    NULL,                                   // dcHash
    NULL,                                   // dcHashElement
    NULL,                                   // dcHeap
    &dcList_compareNode,                    // dcList
    &dcPair_compareNode,                    // dcPair
    NULL,                                   // dcTree
    NULL,                                   // dcTreeElement
    NULL,                                   // dcScope
    NULL,                                   // dcScopeData
    NULL,                                   // dcObjectStack
    NULL,                                   // dcCallStackData
    &dcGraphData_compareNode,               // dcGraphData
    NULL,                                   // dcLexResult
    NULL,                                   // dcSocket
    &dcString_compareNode,                  // dcString
    &dcNodeEvaluator_compareNode,           // dcNodeEvaluator
    &dcRootMarkFunction_compareNode,        // dcRootMarkFunction
    NULL,                                   // dcClassTemplate
    NULL,                                   // dcPackageContents
    NULL,                                   // dcFilePackageData
    NULL,                                   // dcCommandLineArgument
    NULL,                                   // const void *
    &dcVoid_compareNode,                    // void *
    &dcVoid_compareNode,                    // void * container
    NULL                                    // void **
};

/////////////
// Hashing //
/////////////

static const dcNode_hashPointer sHashPointers[] =
{
    NULL,                                   // NODE_NONE
    &dcInt32_hashNode,                      // dcInt32
    &dcUnsignedInt32_hashNode,              // dcUnsignedInt32
    &dcUnsignedInt64_hashNode,              // dcUnsignedInt64
    &dcFloat_hashNode,                      // dcFloat
    NULL,                                   // dcTaffyCMethodPointer
    NULL,                                   // dcMatrix
    NULL,                                   // dcNumber
    NULL,                                   // dcComplexNumber
    NULL,                                   // dcArray
    NULL,                                   // dcHash
    NULL,                                   // dcHashElement
    NULL,                                   // dcHeap
    NULL,                                   // dcList
    NULL,                                   // dcPair
    NULL,                                   // dcTree
    NULL,                                   // dcTreeElement
    NULL,                                   // dcScope
    NULL,                                   // dcScopeData
    NULL,                                   // dcObjectStack
    NULL,                                   // dcCallStackData
    &dcGraphData_hashNode,                  // dcGraphData
    NULL,                                   // dcLexResult
    NULL,                                   // dcSocket
    &dcString_hashNode,                     // dcString
    &dcNodeEvaluator_hashNode,              // dcNodeEvaluator
    NULL,                                   // dcRootMarkFunction
    NULL,                                   // dcClassTemplate
    NULL,                                   // dcPackageContents
    NULL,                                   // dcFilePackageData
    NULL,                                   // dcCommandLineArgument
    NULL,                                   // const void *
    &dcVoid_hashNode,                       // void *
    &dcVoid_hashNode,                       // void * container
    NULL                                    // void **
};

////////////////
// Templating //
////////////////

static const dcNode_setTemplatePointer sSetTemplatePointers[] =
{
    NULL,                          // NODE_NONE
    NULL,                          // dcInt32
    NULL,                          // dcUnsignedInt32
    NULL,                          // dcUnsignedInt64
    NULL,                          // dcFloat
    NULL,                          // dcTaffyCMethodPointer
    NULL,                          // dcMatrix
    NULL,                          // dcNumber
    NULL,                          // dcComplexNumber
    NULL,                          // dcArray
    NULL,                          // dcHash
    NULL,                          // dcHashElement
    NULL,                          // dcHeap
    NULL,                          // dcList
    NULL,                          // dcPair
    NULL,                          // dcTree
    NULL,                          // dcTreeElement
    NULL,                          // dcScope
    NULL,                          // dcScopeData
    NULL,                          // dcObjectStack
    NULL,                          // dcCallStackData
    &dcGraphData_setTemplate,      // dcGraphData
    NULL,                          // dcLexResult
    NULL,                          // dcSocket
    NULL,                          // dcString
    NULL,                          // dcNodeEvaluator
    NULL,                          // dcRootMarkFunction
    NULL,                          // dcClassTemplate
    NULL,                          // dcPackageContents
    NULL,                          // dcFilePackageData
    NULL,                          // dcCommandLineArgument
    NULL,                          // const void *
    NULL,                          // void *
    NULL,                          // void * container
    NULL                           // void **
};

// used for debugging mainly //
static const char * const sTypeNames[] =
{
    "NONE (no type)",
    "NODE_SIGNED_INT_32",
    "NODE_UNSIGNED_INT_32",
    "NODE_UNSIGNED_INT_64",
    "NODE_FLOAT",
    "NODE_TAFFY_C_METHOD_POINTER",
    "NODE_MATRIX",
    "NODE_NUMBER",
    "NODE_COMPLEX_NUMBER",
    "NODE_ARRAY",
    "NODE_HASH",
    "NODE_HASH_ELEMENT",
    "NODE_HEAP",
    "NODE_LIST",
    "NODE_PAIR",
    "NODE_TREE",
    "NODE_TREE_ELEMENT",
    "NODE_SCOPE",
    "NODE_SCOPE_DATA",
    "NODE_SCOPE_STACK",
    "NODE_CALL_STACK_DATA",
    "NODE_GRAPH_DATA",
    "NODE_LEX_RESULT",
    "NODE_SOCKET",
    "NODE_STRING",
    "NODE_EVALUATOR",
    "NODE_ROOT_MARK_FUNCTION",
    "NODE_CLASS_TEMPLATE",
    "NODE_PACKAGE_CONTENTS",
    "NODE_FILE_PACKAGE_CONTENTS",
    "NODE_COMMAND_LINE_ARGUMENT",
    "NODE_CONST_VOID",
    "NODE_VOID",
    "NODE_VOID_CONTAINER",
    "NODE_DOUBLE_VOID"
};

#define SIZE_CHECKER(_name_)                                        \
    int my##_name_[dcTaffy_countOf(_name_)                          \
                   == NUMBER_OF_NODE_TYPES                          \
                   ? 1                                              \
                   : -1]

struct dcNodeStructSizeSuperAssurerLol_t
{
    SIZE_CHECKER(sComparePointers);
    SIZE_CHECKER(sCopyPointers);
    SIZE_CHECKER(sFreePointers);
    SIZE_CHECKER(sHashPointers);
    SIZE_CHECKER(sMarkPointers);
    SIZE_CHECKER(sMarshallPointers);
    SIZE_CHECKER(sPrintPointers);
    SIZE_CHECKER(sRegisterPointers);
    SIZE_CHECKER(sSetTemplatePointers);
    SIZE_CHECKER(sTypeNames);
    SIZE_CHECKER(sUnmarshallPointers);
};

dcNode *dcNode_create(dcNodeType _type)
{
    dcNode *node = (dcNode *)dcMemory_allocateAndInitialize(sizeof(dcNode));
    node->type = _type;
    node->flags = NO_FLAGS;

    TAFFY_DEBUG(if (dcGarbageCollector_trackRegistration())
                {
                    node->registerBacktraceList = dcList_create();
                }

                if (! dcMemory_isEnabled())
                {
                    node->freeBacktraceList = dcList_create();
                });

    return node;
}

dcNode *dcNode_createWithGuts(dcNodeType _type, void *_guts)
{
    dcNode *result = dcNode_create(_type);
    // populate the union, any pointer type will do
    result->types.scope = (dcScope *)_guts;
    return result;
}

void dcNode_free(dcNode **_node, dcDepth _depth)
{
    if (*_node != NULL)
    {
        TAFFY_DEBUG(dcError_assert(! dcNode_isFreeTrapped(*_node)));
        dcNode *node = *_node;

        //
        // check if we can free, and also if the types union is populated
        //
        if (sFreePointers[node->type] != NULL
            && CAST_STRING(node) != NULL)
        {
            sFreePointers[node->type](node, _depth);
        }

#ifdef ENABLE_DEBUG
        if (! dcMemory_isEnabled())
        {
            dcMemory_createBacktrace(node->freeBacktraceList);
        }

        dcList_free(&node->registerBacktraceList, DC_DEEP);
        dcList_free(&node->creationBacktraceList, DC_DEEP);
#endif // ENABLE_DEBUG

        if (dcMemory_isEnabled())
        {
            dcMemory_free(node);
        }

        *_node = NULL;
    }
}

dcNode *dcNode_tryFree(dcNode **_node, dcDepth _depth)
{
    dcNode *result = NULL;

    if (*_node != NULL)
    {
        if (_depth == DC_FLOATING)
        {
            result = *_node;
        }
        else if (_depth == DC_SHALLOW)
        {
            if (dcNode_isContainer(*_node))
            {
                dcNode_free(_node, _depth);
            }
            else
            {
                result = *_node;
            }
        }
        else if (_depth == DC_DEEP)
        {
            if (! dcNode_isRegistered(*_node))
            {
                dcNode_free(_node, _depth);
            }
            else
            {
                result = *_node;
            }
        }
        else
        {
            result = *_node;
        }
    }

    return result;
}

void dcNode_freeShell(dcNode **_node)
{
    dcMemory_free(*_node);
}

dcNode *dcNode_copy(const dcNode *_from, dcDepth _depth)
{
    dcNode *to = NULL;

    if (_from != NULL)
    {
        to = dcNode_create(_from->type);

        // template is sticky, but registration is not
        dcNode_setTemplate(to, dcNode_isTemplate(_from));

        if (sCopyPointers[_from->type] != NULL)
        {
            sCopyPointers[_from->type](to, _from, _depth);
        }
    }

    return to;
}

dcNode *dcNode_copyAndTemplate(const dcNode *_from)
{
    dcNode *result = dcNode_copy(_from, DC_DEEP);
    dcNode_setTemplate(result, true);
    return result;
}

dcNode *dcNode_tryCopy(const dcNode *_from, dcDepth _depth)
{
    dcNode *result = NULL;

    if (_from != NULL)
    {
        if (_depth == DC_FLOATING)
        {
            result = (dcNode *)_from;
        }
        else if (_depth == DC_SHALLOW)
        {
            if (dcNode_isContainer(_from))
            {
                result = dcNode_copy(_from, DC_SHALLOW);
            }
            else
            {
                result = (dcNode *)_from;
            }
        }
        else if (_depth == DC_DEEP)
        {
            result = dcNode_copy(_from, DC_DEEP);
        }
    }

    return result;
}

dcNode *dcNode_copyIfTemplate(dcNode *_node)
{
    dcNode *result = NULL;

    if (_node != NULL)
    {
        result = _node;

        if (dcNode_isTemplate(_node))
        {
            result = dcNode_copy(_node, DC_DEEP);
            dcNode_setTemplate(result, false);
            dcNode_register(result);
        }
    }

    return result;
}

void dcNode_mark(dcNode *_node)
{
    dcNode_markYesNo(_node, true);
}

void dcNode_markYesNo(dcNode *_node, bool _yesno)
{
    if (_node != NULL)
    {
        if (_yesno)
        {
            if (sMarkPointers[_node->type] != NULL && ! dcNode_isMarking(_node))
            {
                dcNode_setMarking(_node, true);
                sMarkPointers[_node->type](_node);
                dcNode_setMarking(_node, false);

                if (! dcNode_isMarked(_node))
                {
                    dcSystem_nodeWasMarked();
                }
            }
        }

        dcNode_setMarked(_node, _yesno);
    }
}

dcNode *dcNode_register(dcNode *_node)
{
    if (_node != NULL
        && ! dcNode_isRegistered(_node))
    {
        if (sRegisterPointers[_node->type] != NULL)
        {
            sRegisterPointers[_node->type](_node);
        }

        if (! dcNode_isContainer(_node))
        {
            dcGarbageCollector_registerNode(_node);
        }
    }

    return _node;
}

dcResult dcNode_prettyPrint(const dcNode *_node, dcCharacterGraph **_graph)
{
    dcResult result = TAFFY_SUCCESS;

    if (_node == NULL)
    {
        *_graph = dcCharacterGraph_createFromCharString("??NULL??");
    }
    else if (sPrettyPrintPointers[_node->type] != NULL)
    {
        result = sPrettyPrintPointers[_node->type](_node, _graph);
    }
    else
    {
        dcString *string = dcString_create();
        result = dcNode_print(_node, string);

        if (result == TAFFY_SUCCESS)
        {
            *_graph = dcCharacterGraph_createFromString(string);
        }

        dcString_free(&string, DC_DEEP);
    }

    return result;
}

dcResult dcNode_print(const dcNode *_node, dcString *_string)
{
    dcResult result = TAFFY_SUCCESS;

    if (_node == NULL)
    {
        dcString_appendString(_string, "NULL??");
    }
    else if (sPrintPointers[_node->type] != NULL)
    {
        result = sPrintPointers[_node->type](_node, _string);
    }
    else
    {
        dcString_append(_string, "#(%s)", sTypeNames[_node->type]);
    }

    return result;
}

static dcNode *displayPrint(const dcNode *_node)
{
    dcNode *result = dcString_createNode();

    if (dcNode_print(_node, CAST_STRING(result)) == TAFFY_EXCEPTION)
    {
        dcNode_free(&result, DC_DEEP);
        result = NULL;
    }

    return result;
}

dcResult dcNode_printToCharacterGraph(const dcNode *_node,
                                      dcCharacterGraph **_graph)
{
    dcString *string = dcString_create();
    dcResult result = dcNode_print(_node, string);

    if (result == TAFFY_SUCCESS)
    {
        *_graph = dcCharacterGraph_createFromString(string);
    }

    dcString_free(&string, DC_DEEP);
    return result;
}

char *dcNode_display(const dcNode *_node)
{
    dcNode *result = displayPrint(_node);
    // no memory leaks please
    dcNode_register(result);

    return (result == NULL
            ? NULL
            : CAST_STRING(result)->string);
}

char *dcNode_prettyDisplay(const dcNode *_node)
{
    dcCharacterGraph *graph;
    dcResult printResult = dcNode_prettyPrint(_node, &graph);
    char *result = NULL;

    if (printResult == TAFFY_SUCCESS)
    {
        result = dcCharacterGraph_display(graph);

        // no memory leaks please
        dcNode_register(dcString_createNodeWithString(result, false));
    }

    return result;
}

static void *display(void *_argument)
{
    char *result = dcNode_display((dcNode *)_argument);
    return (result != NULL
            ? dcMemory_strdup(result)
            : result);
}

// an API function, you can call this outside of a taffy evaluation loop
char *dcNode_synchronizedDisplay(const dcNode *_node)
{
    return (char *)(dcNodeEvaluator_synchronizeFunctionCall
                    (dcSystem_getCurrentNodeEvaluator(),
                     &display,
                     (void *)_node));
}

dcTaffyOperator dcNode_easyCompare(dcNode *_left, dcNode *_right)
{
    dcTaffyOperator taffyOperator = TAFFY_LAST_OPERATOR;
    assert(dcNode_compare(_left, _right, &taffyOperator) != TAFFY_EXCEPTION);
    return taffyOperator;
}

dcResult dcNode_compareEqual(dcNode *_left,
                             dcNode *_right,
                             dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_FAILURE;

    if (_left == _right)
    {
        result = TAFFY_SUCCESS;
        *_compareResult = TAFFY_EQUALS;
    }
    else if (IS_CLASS(_left)
             && IS_CLASS(_right))
    {
        result = dcClass_compareEqual(_left, _right, _compareResult);
    }
    else
    {
        result = dcNode_compare(_left, _right, _compareResult);
    }

    return result;
}

dcResult dcNode_compare(dcNode *_left,
                        dcNode *_right,
                        dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_FAILURE;
    *_compareResult = TAFFY_UNKNOWN_OPERATOR;

    if (_left == _right)
    {
        result = TAFFY_SUCCESS;
        *_compareResult = TAFFY_EQUALS;
    }
    else if (sComparePointers[_left->type] != NULL)
    {
        result = sComparePointers[_left->type](_left, _right, _compareResult);
    }

    return result;
}

dcResult dcNode_comparePointers(dcNode *_left,
                                dcNode *_right,
                                dcTaffyOperator *_compareResult)
{
    *_compareResult = (_left == _right
                       ? TAFFY_EQUALS
                       : (_left < _right
                          ? TAFFY_LESS_THAN
                          : TAFFY_GREATER_THAN));
    return TAFFY_SUCCESS;
}

bool dcTaffy_checkNullPrint(const void *_data, dcString *_output)
{
    if (_data == NULL)
    {
        dcString_appendString(_output, "??NULL??");
    }

    return (_data == NULL ? false : true);
}

static bool readFlag(const dcNode *_node, int _flag)
{
    return ((_node->flags & _flag) != 0);
}

bool dcNode_isMarking(const dcNode *_node)
{
    return readFlag(_node, NODE_MARKING);
}

bool dcNode_isMarked(const dcNode *_node)
{
    return readFlag(_node, NODE_MARKED);
}

bool dcNode_isRegistered(const dcNode *_node)
{
    return readFlag(_node, NODE_REGISTERED);
}

bool dcNode_isTemplate(const dcNode *_node)
{
    return readFlag(_node, NODE_TEMPLATE);
}

bool dcNode_isGarbageCollectionTrapped(const dcNode *_node)
{
    return readFlag(_node, NODE_GARBAGE_COLLECTION_TRAPPED);
}

bool dcNode_isRegisterTrapped(const dcNode *_node)
{
    return readFlag(_node, NODE_REGISTER_TRAPPED);
}

bool dcNode_isFreeTrapped(const dcNode *_node)
{
    return readFlag(_node, NODE_FREE_TRAPPED);
}

static void setFlag(dcNode *_node, int _flag, bool _yesNo)
{
    SET_BITS(_node->flags, _flag, _yesNo);
}

void dcNode_setMarking(dcNode *_node, bool _yesNo)
{
    setFlag(_node, NODE_MARKING, _yesNo);
}

void dcNode_setMarked(dcNode *_node, bool _yesNo)
{
    setFlag(_node, NODE_MARKED, _yesNo);
}

void dcNode_setRegistered(dcNode *_node, bool _yesNo)
{
    setFlag(_node, NODE_REGISTERED, _yesNo);
}

dcNode *dcNode_setTemplate(dcNode *_node, bool _yesNo)
{
    setFlag(_node, NODE_TEMPLATE, _yesNo);

    if (sSetTemplatePointers[_node->type] != NULL)
    {
        sSetTemplatePointers[_node->type](_node, _yesNo);
    }

    return _node;
}

void dcNode_setGarbageCollectionTrapped(dcNode *_node, bool _yesNo)
{
    setFlag(_node, NODE_GARBAGE_COLLECTION_TRAPPED, _yesNo);
}

void dcNode_setFreeTrapped(dcNode *_node, bool _yesNo)
{
    setFlag(_node, NODE_FREE_TRAPPED, _yesNo);
}

void dcNode_setRegisterTrapped(dcNode *_node, bool _yesNo)
{
    setFlag(_node, NODE_REGISTER_TRAPPED, _yesNo);
}

const char *const sInvalidType = "invalid type %d";

const char *dcNode_getTypeString(dcNodeType _type)
{
    return (_type < NUMBER_OF_NODE_TYPES
            ? sTypeNames[_type]
            : sInvalidType);
}

const char *dcNode_getNodeTypeString(const dcNode *_node)
{
    return dcNode_getTypeString(_node->type);
}

void dcNode_printNodeType(const dcNode *_node)
{
    printf("%d: %s\n", _node->type, dcNode_getNodeTypeString(_node));
}

void dcNode_printType(dcNodeType _type)
{
    printf("%d: %s\n", _type, dcNode_getTypeString(_type));
}

#define CLASS_SINGLETON_FLAG 0xFE

dcNode *dcNode_unmarshall(dcString *_stream)
{
    dcNode *result = NULL;

    if (dcString_getLengthLeft(_stream) > 0)
    {
        if (dcString_peek(_stream) == 0xFF)
        {
            // result = NULL
            dcString_incrementIndex(_stream, 1);
        }
        else if (dcString_peek(_stream) == CLASS_SINGLETON_FLAG)
        {
            dcString_incrementIndex(_stream, 1);
            uint16_t singletonId;

            if (dcMarshaller_unmarshall(_stream, "v", &singletonId))
            {
                result = dcClassManager_getSingletonFromId(singletonId);

                if (result != NULL)
                {
                    result = dcNode_copy(result, DC_DEEP);
                }
            }
        }
        else
        {
            dcNodeType type = dcString_getCharacter(_stream);

            if (type < NODE_LAST)
            {
                if (sUnmarshallPointers[type] != NULL
                    && dcString_getLengthLeft(_stream) > 0)
                {
                    result = dcNode_create(type);

                    if (! sUnmarshallPointers[type](result, _stream))
                    {
                        dcMemory_free(result);
                    }
                }
            }
        }
    }

    return result;
}

dcString *dcNode_marshall(const dcNode *_node, dcString *_stream)
{
    dcString *result = _stream;

    if (_node == NULL)
    {
        result = dcMarshaller_marshall(_stream, "n", NULL);
    }
    else if (IS_CLASS(_node)
             && dcClassTemplate_isSingleton(dcClass_getTemplate(_node)))
    {
        assert(dcClass_getTemplate(_node)->singletonId != 0);
        result = dcMarshaller_marshall(_stream,
                                       "uv",
                                       CLASS_SINGLETON_FLAG,
                                       dcClass_getTemplate(_node)->singletonId);
    }
    else
    {
        result = dcMarshaller_marshall(_stream, "c", _node->type);

        if (sMarshallPointers[_node->type] != NULL)
        {
            result = sMarshallPointers[_node->type](_node, result);
        }
    }

    return result;
}

void dcNode_assertType(const dcNode *_node, dcNodeType _type)
{
    assert(_node != NULL && _node->type == _type);
}

void dcNode_assertEqual(dcNode *_left, dcNode *_right)
{
    dcTaffyOperator comparison;
    dcError_assert(_left != NULL
                   && _right != NULL
                   && (dcNode_compare(_left, _right, &comparison)
                       == TAFFY_SUCCESS)
                   && comparison == TAFFY_EQUALS);
}

dcResult dcNode_hash(dcNode *_node, dcHashType *_hashResult)
{
    return (sHashPointers[_node->type] != NULL
            ? sHashPointers[_node->type](_node, _hashResult)
            : TAFFY_FAILURE);
}

dcHashType dcNode_easyHash(dcNode *_node)
{
    dcHashType result = 0;
    assert(dcNode_hash(_node, &result) == TAFFY_SUCCESS);
    return result;
}

// TODO: change types to have no gaps so only simple < and > is needed
bool dcNode_isContainer(const dcNode *_node)
{
    const dcNodeType containerTypes[] = {NODE_ARRAY,
                                         NODE_HASH,
                                         NODE_HASH_ELEMENT,
                                         NODE_HEAP,
                                         NODE_LIST,
                                         NODE_PAIR,
                                         NODE_TREE,
                                         NODE_TREE_ELEMENT,
                                         NODE_SCOPE_DATA};
    size_t i;
    bool result = false;

    for (i = 0; i < dcTaffy_countOf(containerTypes); i++)
    {
        if (containerTypes[i] == _node->type)
        {
            result = true;
            break;
        }
    }

    return result;
}

#ifdef ENABLE_DEBUG
char *d(const dcNode *_node)
{
    return dcNode_display(_node);
}

void dcNode_printRegisterBacktrace(const dcNode *_node)
{
    dcMemory_printBacktrace(_node->registerBacktraceList);
}

void dcNode_printFreeBacktrace(const dcNode *_node)
{
    dcMemory_printBacktrace(_node->freeBacktraceList);
}

void dcNode_printCreationBacktrace(const dcNode *_node)
{
    dcMemory_printBacktrace(_node->creationBacktraceList);
}

void dcNode_trackCreation(dcNode *_node)
{
    if (dcMemory_trackCreations())
    {
        _node->creationBacktraceList = dcList_create();
        dcMemory_createBacktrace(_node->creationBacktraceList);
    };
}
#else // ENABLE_DEBUG
void dcNode_printRegisterBacktrace(const dcNode *_node) {}
void dcNode_printFreeBacktrace(const dcNode *_node) {}
void dcNode_printCreationBacktrace(const dcNode *_node) {}
void dcNode_trackCreation(dcNode *_node) {}
#endif
