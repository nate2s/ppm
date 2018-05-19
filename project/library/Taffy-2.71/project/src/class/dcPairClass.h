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

#ifndef __DC_PAIR_CLASS_H__
#define __DC_PAIR_CLASS_H__

#include "dcDefines.h"

////////////////////
// dcPairClassAux //
////////////////////

struct dcPairClassAux_t
{
    bool initialized;
    struct dcPair_t *pair;
};

typedef struct dcPairClassAux_t dcPairClassAux;

/////////////////
// dcPairClass //
/////////////////

// creating //
struct dcNode_t *dcPairClass_createNode(struct dcNode_t *_left,
                                        struct dcNode_t *_right,
                                        bool _initialized,
                                        bool _object);

struct dcNode_t *dcPairClass_createNodeFromPair(struct dcPair_t *_pair,
                                                bool _initialized,
                                                bool _object);

struct dcNode_t *dcPairClass_createObject(struct dcNode_t *_left,
                                          struct dcNode_t *_right,
                                          bool _initialized);

struct dcNode_t *dcPairClass_createObjectFromPair(struct dcPair_t *_pair,
                                                  bool _initialized);

// initializing //
void dcPairClass_initializeObject(struct dcNode_t *_pairNode,
                                  struct dcNode_t *_left,
                                  struct dcNode_t *_right);

char *dcPairClass_displayNodeWithoutQuotes(const struct dcNode_t *_pair);

// getting //
struct dcPair_t *dcPairClass_getPair(const struct dcNode_t *_node);
struct dcNode_t *dcPairClass_getLeft(const struct dcNode_t *_node);
struct dcNode_t *dcPairClass_getRight(const struct dcNode_t *_node);
bool dcPairClass_isInitialized(const struct dcNode_t *_node);

// standard functions //
ALLOCATE_FUNCTION(dcPairClass_allocateNode);
DEALLOCATE_FUNCTION(dcPairClass_deallocateNode);
COPY_FUNCTION(dcPairClass_copyNode);
FREE_FUNCTION(dcPairClass_freeNode);
GET_TEMPLATE_FUNCTION(dcPairClass_getTemplate);
INITIALIZE_FUNCTION(dcPairClass_initialize);
MARK_FUNCTION(dcPairClass_markNode);
MARSHALL_FUNCTION(dcPairClass_marshallNode);
REGISTER_FUNCTION(dcPairClass_registerNode);
UNMARSHALL_FUNCTION(dcPairClass_unmarshallNode);

// taffy methods //
TAFFY_C_METHOD(dcPairClass_asString);
TAFFY_C_METHOD(dcPairClass_left);
TAFFY_C_METHOD(dcPairClass_init);
TAFFY_C_METHOD(dcPairClass_right);
TAFFY_C_METHOD(dcPairClass_setLeft);
TAFFY_C_METHOD(dcPairClass_setRight);

TAFFY_C_METHOD(dcPairMetaClass_leftRight);

#define PAIR_PACKAGE_NAME CONTAINER_PACKAGE_NAME
#define PAIR_CLASS_NAME "Pair"

#endif
