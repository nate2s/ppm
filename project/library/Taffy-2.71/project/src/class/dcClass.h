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

#ifndef __DC_CLASS_H__
#define __DC_CLASS_H__

#include "dcDefines.h"

struct dcClass_t
{
    // the template
    // this pointer is shared across all class objects of the same type
    struct dcClassTemplate_t *classTemplate;

    // its superclass, to the rescue //
    struct dcNode_t *superNode;

    // is this an object or meta?
    bool isObject;

    //
    // the scope
    //
    // contains the "local" variables of this object
    // variables (non-methods) are copied during instantiation from the template
    // most methods are stored in the template's scope (not this variable)
    //
    // if a method is added during runtime to this object,
    // then it will be stored in this scope
    //
    // if a method is added during runtime to the class,
    // then it will be stored in the template's scope
    //
    struct dcScope_t *scope;

    // The auxiliary data, for C-based class-template data,
    // for instance, the real guts of class Number (see dcNumberClass.h)
    void *auxiliaryData;

    // the mutex, for synchronization :-D //
    struct dcMutex_t *mutex;

    // an optional read-write lock, for success //
    struct dcReadWriteLock_t *readWriteLock;
};

typedef struct dcClass_t dcClass;

//////////////
// Creating //
//////////////

struct dcNode_t *dcClass_createNode(struct dcClassTemplate_t *_template,
                                    struct dcNode_t *_supernode,
                                    struct dcScope_t *_scope,
                                    bool _object,
                                    void *_aux);

struct dcNode_t *dcClass_createBasicNode(struct dcClassTemplate_t *_template,
                                         bool _object);

// getting //
struct dcNode_t *dcClass_getSuperNode(const struct dcNode_t *_node);
struct dcClassTemplate_t *dcClass_getTemplate(const struct dcNode_t *_node);

struct dcNode_t *dcClass_getScopeData
    (struct dcNode_t *_receiver,
     const char *_objectName,
     dcScopeDataFlags _flags,
     bool _searchUp,
     struct dcNode_t *_requestor, // may be NULL
     struct dcClassTemplate_t **_foundTemplate, // may be NULL
     struct dcScope_t **_scope); // may be NULL

#define dcClass_getScopeDataForObject(_receiver,                        \
                                      _objectName,                      \
                                      _searchUp,                        \
                                      _requestor,                       \
                                      _foundTemplate,                   \
                                      _scopeResult)                     \
    dcClass_getScopeData(_receiver,                                     \
                         _objectName,                                   \
                         SCOPE_DATA_OBJECT,                             \
                         _searchUp,                                     \
                         _requestor,                                    \
                         _foundTemplate,                                \
                         _scopeResult)

struct dcNode_t *dcClass_getObject(struct dcNode_t *_receiver,
                                   const char *_objectName);

#define dcClass_getScopeDataForMethod(_receiver,                        \
                                      _objectName,                      \
                                      _searchUp,                        \
                                      _requestor,                       \
                                      _foundTemplate,                   \
                                      _scopeResult)                     \
    dcClass_getScopeData(_receiver,                                     \
                         _objectName,                                   \
                         SCOPE_DATA_METHOD,                             \
                         _searchUp,                                     \
                         _requestor,                                    \
                         _foundTemplate,                                \
                         _scopeResult)

struct dcNode_t *dcClass_getMethod(struct dcNode_t *_receiver,
                                   const char *_methodName);

void dcClass_deallocateNode(struct dcNode_t *_node);

struct dcScope_t *dcClass_getScope(const struct dcNode_t *_class);
struct dcScope_t *dcClass_getMetaScope(const struct dcNode_t *_class);
struct dcScope_t *dcClass_getUsedScope(const struct dcNode_t *_class);
const char *dcClass_getName(const struct dcNode_t *_class);

bool dcClass_isObject(const struct dcNode_t *_class);
bool dcClass_isSlice(const struct dcNode_t *_class);
bool dcClass_isSingleton(const struct dcNode_t *_class);
dcClassFlags dcClass_getFlags(const struct dcNode_t *_class);

// setting //
void dcClass_setSuperNode(struct dcNode_t *_class, struct dcNode_t *_superNode);
void dcClass_setScope(dcClass *_class, struct dcScope_t *_scope);
void dcClass_setClassName(dcClass *_class, const char *_className);
void dcClass_setTemplate(struct dcNode_t *_class, bool _yesno);

// instantiating //
void dcClass_instantiateSuperNodes(dcClass *_class, bool _template);
struct dcNode_t *dcClass_getOrInstantiateSuperNode(struct dcNode_t *_class);

  //                                 //
 // object adding and method adding //
//                                 //

// returns node of dcScopeData //
struct dcNode_t *dcClass_set(struct dcNode_t *_class,
                             const char *_name,
                             struct dcNode_t *_object,
                             dcScopeDataFlags _flags,
                             bool _local);

//////////////////////////////////////////////////////////
//                                                      //
// Accessors for the class                              //
//                                                      //
// These are created when the user defines a            //
// variable like:                                       //
//                                                      //
// @myVar, rw                                           //
//                                                      //
// rw indicates both a reader and writer should be      //
// created                                              //
//                                                      //
//////////////////////////////////////////////////////////

void dcClass_makeReader(struct dcNode_t *_class,
                        const char *_variableName,
                        dcScopeDataFlags _flags);

void dcClass_makeWriter(struct dcNode_t *_class,
                        const char *_variableName,
                        dcScopeDataFlags _flags);

bool dcClass_isKindOfTemplate(const struct dcNode_t *_node,
                              const struct dcClassTemplate_t *_template);

bool dcClass_isType(const struct dcNode_t *_node,
                    const char *_packageName,
                    const char *_className);

bool dcClass_hasTemplate(const struct dcNode_t *_node,
                         const struct dcClassTemplate_t *_template,
                         bool _object);

bool dcClass_isMetaClass(const struct dcNode_t *_node);

#define dcClass_isTypeObject(_node, _nameId)    \
    (dcClass_isType(_node, _nameId) && dcClass_isObject(_node))

/** @brief Cast a node to the type given by _to
 * If _exceptionCatch is true, then throw an InvalidCastException if _node
 * can't be casted appropriately
 * If _assertCatch is true, assert(false) if _node can't be casted
 * appropriately
 * This function may instantiate supers of _node as required
 */
struct dcNode_t *dcClass_castNodeWithAssert
    (struct dcNode_t *_node,
     const struct dcClassTemplate_t *_toTemplate,
     bool _exceptionCatch,
     bool _assertCast);

/** @brief A helper for dcClass_reallyCastNode
 */
#define dcClass_castNode(_node, _template, _throwException) \
    dcClass_castNodeWithAssert(_node, _template, _throwException, false)

typedef struct dcClassTemplate_t* (*dcClass_getTemplateFunction)(void);

// for synchronization //
void dcClass_lock(struct dcNode_t *_node);
void dcClass_unlock(struct dcNode_t *_node);
void dcClass_lockForRead(struct dcNode_t *_node);
void dcClass_lockForWrite(struct dcNode_t *_node);
void dcClass_unlockReadWriteLock(struct dcNode_t *_node);

// debug hooks //
dcClass *dcClass_getClass(struct dcNode_t *_node);
void dcClass_printNodeType(const struct dcNode_t *_node);
#define dcClass_assertHasTemplate(_node, _type)     \
    dcError_assert(dcClass_hasTemplate(_node, _type, true))

/////////////
// Casting //
/////////////

// Getters //
#define CLASSNAME(node) CAST_CLASS(node)->name

// Casting to aux //
#define CAST_CLASS_AUX(node) CAST_CLASS(node)->auxiliaryData
// helper
void *dcClass_getAux(struct dcNode_t *_node);

// standard functions //
COPY_FUNCTION(dcClass_copyNode);
FREE_FUNCTION(dcClass_freeNode);
HASH_FUNCTION(dcClass_hashNode);
MARK_FUNCTION(dcClass_markNode);
REGISTER_FUNCTION(dcClass_registerNode);
PRINT_FUNCTION(dcClass_printNode);
PRETTY_PRINT_FUNCTION(dcClass_prettyPrintNode);

/////////////////
// marshalling //
/////////////////

#define CLASS_MARSHALL_SIZE 1
#define CLASS_HAS_POPULATED_SCOPE 0x10
#define CLASS_HAS_UNPOPULATED_SCOPE 0x20

MARSHALL_FUNCTION(dcClass_marshallNode);
UNMARSHALL_FUNCTION(dcClass_unmarshallNode);

// standard functions //
COMPARE_FUNCTION(dcClass_compareNode);

dcResult dcClass_compareEqual(struct dcNode_t *_left,
                              struct dcNode_t *_right,
                              dcTaffyOperator *_compareResult);

struct dcNode_t *dcClass_copyIfTemplateOrAtomic(struct dcNode_t *_class);
struct dcNode_t *dcClass_copyIfAtomic(struct dcNode_t *_class);

// debugging hooks //
void dcClass_printInformation(const struct dcNode_t *_class);

dcClass *dcClass_castMe(struct dcNode_t *_class);

#endif
