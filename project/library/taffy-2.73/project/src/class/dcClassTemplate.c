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

#include <string.h>

#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcGraphData.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcPackage.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcStringManager.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcSystem.h"

void dcClassTemplate_setPackageName(dcClassTemplate *_template,
                                    const char *_newPackageName)
{
    dcPackage_free(&_template->package);
    _template->packageName =
        dcStringManager_getStringFromString(_newPackageName);
    _template->package = dcPackage_createFromString(_newPackageName);
}

//
// Create runtime values
//
dcResult dcClassTemplate_createRuntimeValues(dcClassTemplate *_template,
                                             dcList *_packageContentsList)
{
    dcResult result = TAFFY_SUCCESS;

    if (_template->cTemplate != NULL)
    {
        dcScope_addTaffyCMethodWrappers
            (_template->scope, _template->cTemplate->methodWrappers);
        dcScope_register(_template->scope);
        dcScope_addTaffyCMethodWrappers
            (_template->metaScope, _template->cTemplate->metaMethodWrappers);
        dcScope_register(_template->metaScope);
    }

    if (_template->superName != NULL)
    {
        _template->superTemplate =
            dcClassManager_getClassTemplate(_template->superName,
                                            _template->package,
                                            _packageContentsList,
                                            _template);

        if (_template->superTemplate == NULL)
        {
            // the super template doesn't exist, throw an exception
            dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
            uint32_t onlyEvaluateClassesSave = evaluator->onlyEvaluateClasses;
            evaluator->onlyEvaluateClasses = 0;

            // TODO: fix me, doesn't report correct filename + line
            char *callStackLocation =
                dcLexer_sprintf("%s instantiation",
                                _template->className);

            dcNodeEvaluator_pushCallStack(evaluator, callStackLocation);
            dcMemory_free(callStackLocation);

            // oh dear
            dcUnidentifiedClassExceptionClass_throwObject
                (_template->superName);
            result = TAFFY_EXCEPTION;

            evaluator->onlyEvaluateClasses = onlyEvaluateClassesSave;
            dcNodeEvaluator_popCallStack(evaluator, DC_DEEP);
        }
        else if ((_template->superTemplate->classFlags & CLASS_FINAL) != 0)
        {
            dcInvalidSuperClassExceptionClass_throwObject
                (_template->superTemplate->className);
            result = TAFFY_EXCEPTION;
        }
    }

    return result;
}

void dcClassTemplate_free(dcClassTemplate **_template, dcDepth _depth)
{
    dcClassTemplate *classTemplate = *_template;
    dcScope_free(&classTemplate->scope, _depth);
    dcScope_free(&classTemplate->metaScope, _depth);
    dcPackage_free(&classTemplate->package);
    dcMemory_free(classTemplate->cTemplate);
    dcMemory_free(classTemplate);
}

void dcClassTemplate_freeNode(dcNode *_node, dcDepth _depth)
{
    dcClassTemplate *classTemplate = CAST_CLASS_TEMPLATE(_node);
    dcClassTemplate_free(&classTemplate, _depth);
}

void dcClassTemplate_clear(dcClassTemplate *_template, dcDepth _depth)
{
    dcScope_clear(_template->scope, _depth);
    dcScope_clear(_template->metaScope, _depth);
}

dcNode *dcClassTemplate_createShell(dcClassTemplate *_template)
{
    return dcNode_createWithGuts(NODE_CLASS_TEMPLATE, _template);
}

dcClassTemplate *dcClassTemplate_copy(const dcClassTemplate *_from,
                                      dcDepth _depth)
{
    // strings are stored in the string manager, so they are shallowly copied
    dcClassTemplate *result =
        (dcClassTemplate *)dcMemory_duplicate(_from, sizeof(dcClassTemplate));
    result->scope = dcScope_copy(_from->scope, _depth);
    result->metaScope = dcScope_copy(_from->metaScope, _depth);
    result->package = dcPackage_copy(_from->package, _depth);
    return result;
}

void dcClassTemplate_update(dcClassTemplate *_to, const dcClassTemplate *_from)
{
    dcScope_merge(_to->scope, _from->scope);
    dcScope_merge(_to->metaScope, _from->metaScope);
}

dcClassTemplate *dcClassTemplate_createSimple(const char *_packageName,
                                              const char *_className,
                                              const char *_superName,
                                              dcClassFlags _classFlags,
                                              dcScopeDataFlags _scopeDataFlags)
{
    dcClassTemplate *classTemplate =
        (dcClassTemplate *)dcMemory_allocateAndInitialize
        (sizeof(dcClassTemplate));
    dcClassTemplate_setPackageName(classTemplate, _packageName);
    classTemplate->className = dcStringManager_getStringFromString(_className);
    classTemplate->superName = dcStringManager_getStringFromString(_superName);
    classTemplate->classFlags = _classFlags;
    classTemplate->scopeDataFlags = _scopeDataFlags;
    classTemplate->scope = dcScope_create();
    classTemplate->metaScope = dcScope_create();
    return classTemplate;
}

dcNode *dcClassTemplate_createSimpleNode(const char *_packageName,
                                         const char *_className,
                                         const char *_superName,
                                         dcClassFlags _classFlags,
                                         dcScopeDataFlags _scopeDataFlags)
{
    return dcNode_createWithGuts(NODE_CLASS_TEMPLATE,
                                 dcClassTemplate_createSimple(_packageName,
                                                              _className,
                                                              _superName,
                                                              _classFlags,
                                                              _scopeDataFlags));
}

// the main constructor
dcClassTemplate *dcClassTemplate_create
    (const char *_packageName,
     const char *_className,
     const char *_superName,
     dcClassFlags _classFlags,
     dcScopeDataFlags _scopeDataFlags,
     const dcTaffyCMethodWrapper *_metaMethodWrappers,
     const dcTaffyCMethodWrapper *_methodWrappers,
     dcClass_initializationPointer _initializer,
     dcClass_deinitializationPointer _deinitializer,
     dcClass_allocatePointer _allocatePointer,
     dcClass_deallocatePointer _deallocatePointer,
     dcNode_markPointer _metaMarkPointer,
     dcNode_markPointer _markPointer,
     dcNode_copyPointer _copyPointer,
     dcNode_freePointer _freePointer,
     dcNode_registerPointer _registerPointer,
     dcNode_marshallPointer _marshallPointer,
     dcNode_unmarshallPointer _unmarshallPointer,
     dcNode_setTemplatePointer _setTemplatePointer)
{
    dcClassTemplate *result = dcClassTemplate_createSimple(_packageName,
                                                           _className,
                                                           _superName,
                                                           _classFlags,
                                                           _scopeDataFlags);
    result->cTemplate =
        (dcCClassTemplate *)dcMemory_allocateAndInitialize
        (sizeof(dcCClassTemplate));
    result->cTemplate->metaMethodWrappers =
        (dcTaffyCMethodWrapper*)_metaMethodWrappers;
    result->cTemplate->methodWrappers = (dcTaffyCMethodWrapper*)_methodWrappers;
    result->cTemplate->initializer = _initializer;
    result->cTemplate->deinitializer = _deinitializer;
    result->cTemplate->allocatePointer = _allocatePointer;
    result->cTemplate->deallocatePointer = _deallocatePointer;
    result->cTemplate->markPointer = _markPointer;
    result->cTemplate->metaMarkPointer = _metaMarkPointer;
    result->cTemplate->copyPointer = _copyPointer;
    result->cTemplate->freePointer = _freePointer;
    result->cTemplate->registerPointer = _registerPointer;
    result->cTemplate->marshallPointer = _marshallPointer;
    result->cTemplate->unmarshallPointer = _unmarshallPointer;
    result->cTemplate->setTemplatePointer = _setTemplatePointer;
    result->marshallId = 0;
    return result;
}

bool dcClassTemplate_isAtomic(const dcClassTemplate *_template)
{
    return (_template->classFlags & CLASS_ATOMIC) != 0;
}

bool dcClassTemplate_isAbstract(const dcClassTemplate *_template)
{
    return (_template->classFlags & CLASS_ABSTRACT) != 0;
}

bool dcClassTemplate_isFinal(const dcClassTemplate *_template)
{
    return (_template->classFlags & CLASS_FINAL) != 0;
}

bool dcClassTemplate_isSingleton(const dcClassTemplate *_template)
{
    return (_template->classFlags & CLASS_SINGLETON) != 0;
}

bool dcClassTemplate_isProtected(const dcClassTemplate *_template)
{
    return (_template->scopeDataFlags & SCOPE_DATA_PROTECTED) != 0;
}

// debug function
bool dcClassTemplate_equals(const dcClassTemplate *_left,
                            const dcClassTemplate *_right)
{
    return (strcmp(_left->packageName, _right->packageName) == 0
            && strcmp(_left->className, _right->className) == 0
            && _left->classFlags == _right->classFlags
            && _left->scopeDataFlags == _right->scopeDataFlags
            && dcPackage_equals(_left->package, _right->package));
}
