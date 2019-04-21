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

#include "CompiledPair.h"
#include "dcPairClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcContainerClass.h"
#include "dcError.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHashClass.h"
#include "dcKernelClass.h"
#include "dcLineContainerClass.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNilClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcPair.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "left",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcPairClass_left,
        gCFunctionArgument_none
    },
    {
        "init",
        SCOPE_DATA_PUBLIC,
        &dcPairClass_init,
        gCFunctionArgument_none
    },
    {
        "right",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcPairClass_right,
        gCFunctionArgument_none
    },
    {
        "setLeft:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcPairClass_setLeft,
        gCFunctionArgument_wild
    },
    {
        "setRight:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcPairClass_setRight,
        gCFunctionArgument_wild
    },
    {
        0
    }
};

ALLOCATE_FUNCTION(dcPairClass_allocateNode);

#define CAST_PAIR_AUX(_node_) ((dcPairClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcPairClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (PAIR_PACKAGE_NAME,                      // package name
          PAIR_CLASS_NAME,                        // class name
          LINE_CONTAINER_CLASS_NAME,              // super name
          CLASS_HAS_READ_WRITE_LOCK,              // class flags
          NO_FLAGS,                               // scope data flags
          NULL,                                   // meta methods
          sMethodWrappers,                        // methods
          &dcPairClass_initialize,                // initialization function
          NULL,                                   // deinitialization function
          &dcPairClass_allocateNode,              // allocate
          &dcPairClass_deallocateNode,            // deallocate
          NULL,                                   // meta mark
          &dcPairClass_markNode,                  // mark
          &dcPairClass_copyNode,                  // copy
          &dcPairClass_freeNode,                  // free
          &dcPairClass_registerNode,              // register
          &dcPairClass_marshallNode,              // marshall
          &dcPairClass_unmarshallNode,            // unmarshall
          NULL));                                 // set template
}

static dcPairClassAux *createAux(dcNode *_left,
                                 dcNode *_right,
                                 bool _initialized)
{
    dcPairClassAux *aux = (dcPairClassAux *)(dcMemory_allocate
                                             (sizeof(dcPairClassAux)));
    aux->initialized = _initialized;
    aux->pair = dcPair_create(_left, _right);
    return aux;
}

static dcPairClassAux *createAuxFromPair(dcPair *_pair, bool _initialized)
{
    dcPairClassAux *aux = (dcPairClassAux *)(dcMemory_allocate
                                             (sizeof(dcPairClassAux)));
    aux->initialized = _initialized;
    aux->pair = _pair;
    return aux;
}

void dcPairClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux(dcNilClass_getInstance(),
                                      dcNilClass_getInstance(),
                                      true);
}

void dcPairClass_deallocateNode(dcNode *_node)
{
    dcPair_clear(dcPairClass_getPair(_node), DC_SHALLOW);
}

#define PAIR_CLASS_TAFFY_FILE_NAME "src/class/Pair.ty"

void dcPairClass_initialize(void)
{
    dcError_assert(dcStringEvaluator_evalString(__compiledPair,
                                                PAIR_CLASS_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

dcNode *dcPairClass_createNode(dcNode *_left,
                               dcNode *_right,
                               bool _initialized,
                               bool _object)
{
    return dcPairClass_createNodeFromPair(dcPair_create(_left, _right),
                                          _initialized,
                                          _object);
}

dcNode *dcPairClass_createNodeFromPair(dcPair *_pair,
                                       bool _initialized,
                                       bool _object)
{
    return (dcClass_createNode
            (sTemplate,
             dcLineContainerClass_createNode(_object),
             NULL, // scope
             _object,
             createAuxFromPair(_pair, _initialized)));
}

dcNode *dcPairClass_createObject(dcNode *_left,
                                 dcNode *_right,
                                 bool _initialized)
{
    return dcPairClass_createNode(_left, _right, _initialized, true);
}

dcNode *dcPairClass_createObjectFromPair(dcPair *_pair,
                                         bool _initialized)
{
    return dcPairClass_createNodeFromPair(_pair, _initialized, true);
}

void dcPairClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcPairClassAux *aux = CAST_PAIR_AUX(_node);

    if (aux != NULL)
    {
        dcPair_free(&aux->pair, DC_DEEP);
        dcMemory_free(CAST_CLASS_AUX(_node));
    }
}

void dcPairClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcPairClassAux *fromAux = CAST_PAIR_AUX(_from);
    dcPair *pairCopy = NULL;

    // small itty bitty hax //
    if (fromAux->pair->left == NULL)
    {
        pairCopy = dcPair_create(dcNilClass_getInstance(),
                                 dcNilClass_getInstance());
    }
    else
    {
        pairCopy = dcPair_copy(fromAux->pair, _depth);
    }

    CAST_CLASS_AUX(_to) = createAuxFromPair(pairCopy, fromAux->initialized);
}

void dcPairClass_markNode(dcNode *_pairNode)
{
    dcPair *pair = CAST_PAIR_AUX(_pairNode)->pair;
    dcNode_mark(pair->left);
    dcNode_mark(pair->right);
}

void dcPairClass_registerNode(dcNode *_pairNode)
{
    dcPair *pair = CAST_PAIR_AUX(_pairNode)->pair;
    dcNode_register(pair->left);
    dcNode_register(pair->right);
}

void dcPairClass_initializeObject(dcNode *_pairNode,
                                  dcNode *_left,
                                  dcNode *_right)
{
    dcPairClassAux *aux = CAST_PAIR_AUX(_pairNode);
    aux->pair->left = _left;
    aux->pair->right = _right;
    aux->initialized = true;
}

dcPair *dcPairClass_getPair(const dcNode *_pairObject)
{
    return CAST_PAIR_AUX(_pairObject)->pair;
}

dcNode *dcPairClass_getLeft(const dcNode *_pairObject)
{
    return dcPairClass_getPair(_pairObject)->left;
}

dcNode *dcPairClass_getRight(const dcNode *_pairObject)
{
    return dcPairClass_getPair(_pairObject)->right;
}

bool dcPairClass_isInitialized(const dcNode *_node)
{
    return CAST_PAIR_AUX(_node)->initialized;
}

////////////////////////////////////////////////
//                                            //
// Pair#"init"                                //
//                                            //
// Initializes the values of _receiver to nil //
//                                            //
// p = [Pair new]                             //
// p set left: 6 right: 7                   //
// p init                                     //
// ==> <nil, nil>                             //
//                                            //
////////////////////////////////////////////////

dcNode *dcPairClass_init(dcNode *_receiver, dcArray *_arguments)
{
    dcPair *pair = CAST_PAIR_AUX(_receiver)->pair;

    pair->left = dcNode_copy(dcNilClass_getInstance(), DC_DEEP);
    pair->right = dcNode_copy(dcNilClass_getInstance(), DC_DEEP);
    dcNode_register(pair->left);
    dcNode_register(pair->right);

    CAST_PAIR_AUX(_receiver)->initialized = true;

    return _receiver;
}

/////////////////////////////////////
//
// Pair#"left"
//
// Gets the left value of _receiver
//
// p = [Pair new]
// p setLeft: 6
// p left
// ==> 6
//
/////////////////////////////////////
dcNode *dcPairClass_left(dcNode *_receiver, dcArray *_arguments)
{
    return CAST_PAIR_AUX(_receiver)->pair->left;
}

///////////////////////////////////////
//
// Pair#"right"
//
// Gets the right value of _receiver
//
// p = [Pair new]
// p setRight: 8
// p right
// ==> 8
//
///////////////////////////////////////
dcNode *dcPairClass_right(dcNode *_receiver, dcArray *_arguments)
{
    return CAST_PAIR_AUX(_receiver)->pair->right;
}

//////////////////////////////////////
//
// Pair#"setLeft:"
//
// Sets the left value of _receiver
//
// p = [Pair new]
// p setLeft: 6
// ==> [6,nil]
//
//////////////////////////////////////
dcNode *dcPairClass_setLeft(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *insertNode = dcNode_copyIfTemplate(dcArray_get(_arguments, 0));
    CAST_PAIR_AUX(_receiver)->pair->left = insertNode;
    return insertNode;
}

///////////////////////////////////////
//
// Pair#"setRight:"
//
// Sets the right value of _receiver
//
// p = [Pair new]
// p setRight: 8
// ==> [nil,8]
//
///////////////////////////////////////
dcNode *dcPairClass_setRight(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *insertNode = dcNode_copyIfTemplate(dcArray_get(_arguments, 0));
    CAST_PAIR_AUX(_receiver)->pair->right = insertNode;
    return insertNode;
}

bool dcPairClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    bool isInitialized;
    dcPair *objects;

    if (dcMarshaller_unmarshallNoNull(_stream,
                                      "bp",
                                      &isInitialized,
                                      &objects))
    {
        result = true;
        CAST_CLASS_AUX(_node) = createAuxFromPair(objects, isInitialized);
    }

    return result;
}

dcString *dcPairClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    return dcMarshaller_marshall(_stream,
                                 "bp",
                                 dcPairClass_isInitialized(_node),
                                 dcPairClass_getPair(_node));
}
