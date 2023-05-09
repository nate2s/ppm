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

#ifndef __DC_CLASS_TEMPLATE_H__
#define __DC_CLASS_TEMPLATE_H__

#include "dcDefines.h"

struct dcCClassTemplate_t
{
    // instance-wide method wrappers
    struct dcTaffyCMethodWrapper_t *methodWrappers;

    // meta-wide method wrappers
    struct dcTaffyCMethodWrapper_t *metaMethodWrappers;

    // this function is called when the class is first loaded (one time only)
    dcClass_initializationPointer initializer;

    // this function is called when the meta-class is unloaded
    // at system shutdown (one time only)
    dcClass_deinitializationPointer deinitializer;

    // this function is called when an object is created, and is used to
    // allocate memory for the object's auxiliary data
    dcClass_allocatePointer allocatePointer;

    // this function is called by the garbage collector
    // it detaches the node from composite registered nodes in preparation for
    // deletion
    dcClass_deallocatePointer deallocatePointer;

    // mark instance-wide objects
    dcNode_markPointer markPointer;

    // mark meta-wide objects (shared between all objects of the class type)
    dcNode_markPointer metaMarkPointer;

    // the following function pointers work on the auxiliary data of the object
    dcNode_copyPointer copyPointer;
    dcNode_freePointer freePointer;
    dcNode_registerPointer registerPointer;
    dcNode_marshallPointer marshallPointer;
    dcNode_unmarshallPointer unmarshallPointer;
    dcNode_setTemplatePointer setTemplatePointer;
};

typedef struct dcCClassTemplate_t dcCClassTemplate;

enum dcClassFlags_e
{
    CLASS_ABSTRACT             = BITS(0),
    CLASS_ATOMIC               = BITS(1),
    CLASS_FINAL                = BITS(2),
    CLASS_SINGLETON            = BITS(3),
    CLASS_SLICE                = BITS(4),
    CLASS_HAS_READ_WRITE_LOCK  = BITS(5)
};

//
// The standard class template
// Find these populated in many built-in classes,
// like dcArrayClass.c and dcBlockClass.c
//
struct dcClassTemplate_t
{
    const char *packageName;
    const char *className;
    const char *superName;

    // the class flags, like final, atomic
    dcClassFlags classFlags;

    // the scope data flags, like protected, public
    dcScopeDataFlags scopeDataFlags;

    // a struct for classes with a C basis, may be NULL
    dcCClassTemplate *cTemplate;

    // the package
    struct dcPackage_t *package;

    //
    // the template scopes that all instances have access to
    //

    // the instance-level template scope
    struct dcScope_t *scope;
    // the meta-level, template scope
    struct dcScope_t *metaScope;

    // the super template, is NULL if className == org.taffy.core.Object
    struct dcClassTemplate_t *superTemplate;

    // the parent node, may be NULL, used for convenience for protection
    // security
    struct dcNode_t *parentNode;

    // the marshall id, used for marshalling, set by the class manager
    // when the template is registered
    uint16_t marshallId;

    // the singleton id, used for marshalling if this class is a singleton
    uint16_t singletonId;
};

typedef struct dcClassTemplate_t dcClassTemplate;

/** @brief A constructor for a class with a C basis
 */
dcClassTemplate *dcClassTemplate_create
    (const char *_packageName,
     const char *_className,
     const char *_superName,
     dcClassFlags _classFlags,
     dcScopeDataFlags _scopeDataFlags,
     const struct dcTaffyCMethodWrapper_t *_metaMethodWrappers,
     const struct dcTaffyCMethodWrapper_t *_methodWrappers,
     dcClass_initializationPointer _initializer,
     dcClass_deinitializationPointer _deinitializer,
     dcClass_allocatePointer _allocatePointer,
     dcClass_deallocatePointer _deallocatePointer,
     dcNode_markPointer _markPointer,
     dcNode_markPointer _metaMarkPointer,
     dcNode_copyPointer _copyPointer,
     dcNode_freePointer _freePointer,
     dcNode_registerPointer _registerPointer,
     dcNode_marshallPointer _marshallPointer,
     dcNode_unmarshallPointer _unmarshallPointer,
     dcNode_setTemplatePointer _setTemplatePointer);

/** @brief Constructors for classes with no C basis
 */
struct dcNode_t *dcClassTemplate_createSimpleNode
    (const char *_packageName,
     const char *_className,
     const char *_superName,
     dcClassFlags _classFlags,
     dcScopeDataFlags _scopeDataFlags);

struct dcClassTemplate_t *dcClassTemplate_createSimple
    (const char *_packageName,
     const char *_className,
     const char *_superName,
     dcClassFlags _classFlags,
     dcScopeDataFlags _scopeDataFlags);

dcResult dcClassTemplate_createRuntimeValues
    (dcClassTemplate *_template,
     struct dcList_t *_packageContentsList);

struct dcNode_t *dcClassTemplate_createShell(dcClassTemplate *_template);
void dcClassTemplate_free(dcClassTemplate **_template, dcDepth _depth);

/** @brief Set the package name. Frees _template's old package.
 */
void dcClassTemplate_setPackageName(dcClassTemplate *_template,
                                    const char *_newPackageName);

// copying //
dcClassTemplate *dcClassTemplate_copy(const dcClassTemplate *_from,
                                      dcDepth _depth);

// updating //
void dcClassTemplate_update(dcClassTemplate *_to, const dcClassTemplate *_from);

FREE_FUNCTION(dcClassTemplate_freeNode);

void dcClassTemplate_clear(dcClassTemplate *_template, dcDepth _depth);

typedef dcClassTemplate *(*dcClass_templateCreator)(void);

bool dcClassTemplate_isAtomic(const dcClassTemplate *_template);
bool dcClassTemplate_isAbstract(const dcClassTemplate *_template);
bool dcClassTemplate_isFinal(const dcClassTemplate *_template);
bool dcClassTemplate_isSingleton(const dcClassTemplate *_template);

bool dcClassTemplate_isProtected(const dcClassTemplate *_template);

#define CLASS_TEMPLATE_SINGLETON_HELPER(_template, _rightHandSide)   \
    if (_template == NULL)                                           \
    {                                                                \
        _template = _rightHandSide;                                  \
    }                                                                \
                                                                     \
    return _template;

/** @brief Debug function. Returns true if the two templates are equal.
 * Returns false otherwise.
 */
bool dcClassTemplate_equals(const dcClassTemplate *_left,
                            const dcClassTemplate *_right);

#endif
