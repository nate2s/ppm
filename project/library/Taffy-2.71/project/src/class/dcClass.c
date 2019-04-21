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
#include <ctype.h>
#include <string.h>

#include "dcArray.h"
#include "dcCFunctionArgument.h"
#include "dcCharacterGraph.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHash.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcNode.h"
#include "dcNumber.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcString.h"
#include "dcMarshaller.h"
#include "dcMethodHeader.h"
#include "dcMutex.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcPackage.h"
#include "dcParser.h"
#include "dcReadWriteLock.h"
#include "dcSystem.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcStringManager.h"
#include "dcSuppliedArgumentClass.h"
#include "dcTaffyCMethodPointer.h"
#include "dcWildClass.h"

#include "dcHashClass.h"
#include "dcIOClass.h"
#include "dcObjectClass.h"
#include "dcProcedureClass.h"
#include "dcStringClass.h"
#include "dcSuppliedArgumentClass.h"
#include "dcYesClass.h"

static void instantiateSuperNodes(dcNode *_node)
{
    dcNode *that;

    for (that = _node;
         that != NULL;
         that = dcClass_getOrInstantiateSuperNode(that))
    {
    }
}

static void fillClass(dcNode *_classNode,
                      dcClassTemplate *_template,
                      bool _object,
                      dcScope *_scope,
                      dcNode *_superNode,
                      void *_auxiliaryData,
                      bool _allocateAuxiliaryData)
{
    dcClass *klass = (dcClass *)dcMemory_allocateAndInitialize(sizeof(dcClass));

    if (_template == NULL)
    {
        fprintf(stderr,
                "Fatal error: template for class is NULL, name is a mystery\n");
        assert(false);
    }

    klass->classTemplate = _template;
    klass->auxiliaryData = _auxiliaryData;
    klass->superNode = _superNode;
    klass->isObject = _object;
    klass->scope = (_scope == NULL
                    ? dcScope_create()
                    : _scope);

    // copy over the objects (and not the methods)
    // each instance has its own local copy of the class' objects
    dcScope_mergeObjects(klass->scope, klass->classTemplate->scope);

    klass->mutex = dcMutex_create(true);

    if ((_template->classFlags & CLASS_HAS_READ_WRITE_LOCK) != 0)
    {
        klass->readWriteLock = dcReadWriteLock_create();
    }

    CAST_CLASS(_classNode) = klass;

    if (_allocateAuxiliaryData
        && klass->isObject
        && _auxiliaryData == NULL
        && (_template->cTemplate != NULL
            && _template->cTemplate->allocatePointer != NULL))
    {
        _template->cTemplate->allocatePointer(_classNode);
    }
}

static dcNode *createNode(dcClassTemplate *_template,
                          dcNode *_superNode,
                          dcScope *_scope,
                          bool _object,
                          void *_auxiliaryData,
                          bool _instantiateSuperNode)
{
    dcNode *node = dcGraphData_createNode(NODE_CLASS);
    dcNode_setTemplate(node, false);
    fillClass(node,
              _template,
              _object,
              _scope,
              _superNode,
              _auxiliaryData,
              true);

    // debug
    dcNode_trackCreation(node);

    if (_instantiateSuperNode)
    {
        instantiateSuperNodes(node);
    }

    return node;
}

dcNode *dcClass_createNode(dcClassTemplate *_template,
                           dcNode *_superNode,
                           dcScope *_scope,
                           bool _object,
                           void *_auxiliaryData)
{
    // don't instantiate supers by default
    return createNode
        (_template, _superNode, _scope, _object, _auxiliaryData, false);
}

dcNode *dcClass_createBasicNode(dcClassTemplate *_template, bool _object)
{
    return dcClass_createNode(_template, NULL, NULL, _object, NULL);
}

dcNode *dcClass_getOrInstantiateSuperNode(dcNode *_node)
{
    dcClass *klass = CAST_CLASS(_node);
    dcClass_lock(_node);
    dcError_assert(klass->classTemplate->package != NULL);

    if (klass->superNode == NULL
        && klass->classTemplate->superTemplate == NULL
        && klass->classTemplate->superName != NULL)
    {
        klass->classTemplate->superTemplate =
            dcClassManager_getClassTemplate
            (klass->classTemplate->superName,
             klass->classTemplate->package,
             dcSystem_getPackageContentsFromGraphDataNode(_node),
             NULL);
        // throw exception if not found?
    }

    if (klass->superNode == NULL
        && klass->classTemplate->superTemplate != NULL)
    {
        klass->superNode = createNode(klass->classTemplate->superTemplate,
                                      NULL,   // supernode
                                      NULL,   // scope
                                      dcClass_isObject(_node),
                                      NULL,   // aux
                                      false); // instantiate super nodes
        dcGraphData_copyPosition(_node, klass->superNode);

        // if the inherited object is registered, register its super as well
        if (dcNode_isRegistered(_node))
        {
            dcNode_register(klass->superNode);
        }
        else if (dcNode_isTemplate(_node))
        {
            dcNode_setTemplate(klass->superNode, true);
        }
        else
        {
            // that shouldn't happen -- we should either be registered
            // or a template
            dcError_assert(false);
        }
    }
    else
    {
        TAFFY_DEBUG(dcError_assert
                    (klass->superNode == NULL
                     || (dcNode_isTemplate(_node)
                         == dcNode_isTemplate(klass->superNode))));
    }

    if (klass->superNode != NULL)
    {
        CAST_CLASS(klass->superNode)->isObject = klass->isObject;
    }

    dcClass_unlock(_node);
    return klass->superNode;
}

void dcClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcClass *klass = CAST_CLASS(_node);

    if (dcClass_isObject(_node)
        && klass->classTemplate->cTemplate != NULL
        && klass->classTemplate->cTemplate->freePointer != NULL
        && klass->auxiliaryData != NULL)
    {
        klass->classTemplate->cTemplate->freePointer(_node, _depth);
        klass->auxiliaryData = NULL;
    }

    if (dcNode_isTemplate(_node))
    {
        dcNode_tryFree(&klass->superNode, DC_DEEP);
    }

    dcScope_free(&klass->scope,
                 dcNode_isRegistered(_node)
                 ? DC_SHALLOW
                 : DC_DEEP);

    dcMutex_free(&klass->mutex);
    dcReadWriteLock_free(&klass->readWriteLock);
    dcMemory_free(klass);
    CAST_CLASS(_node) = NULL;
}

void dcClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcClass *copyClass =
        (dcClass *)dcMemory_allocateAndInitialize(sizeof(dcClass));
    dcClass *fromClass = CAST_CLASS(_from);

    copyClass->classTemplate = fromClass->classTemplate;
    copyClass->auxiliaryData = NULL;
    copyClass->scope = dcScope_copy(fromClass->scope, DC_DEEP);
    copyClass->isObject = fromClass->isObject;
    copyClass->superNode = dcNode_copy(fromClass->superNode, DC_DEEP);
    copyClass->mutex = (fromClass->mutex == NULL
                        ? NULL
                        : dcMutex_create(true));
    copyClass->readWriteLock = (fromClass->readWriteLock == NULL
                                ? NULL
                                : dcReadWriteLock_create());

    CAST_CLASS(_to) = copyClass;

    if (fromClass->classTemplate->cTemplate != NULL
        && fromClass->classTemplate->cTemplate->copyPointer != NULL)
    {
        fromClass->classTemplate->cTemplate->copyPointer(_to, _from, _depth);
    }

    // debug
    dcNode_trackCreation(_to);
}

dcNode *dcClass_getSuperNode(const dcNode *_node)
{
    dcError_assert(IS_CLASS(_node));
    return CAST_CLASS(_node)->superNode;
}

dcScope *dcClass_getScope(const dcNode *_class)
{
    return (_class == NULL
            ? NULL
            : CAST_CLASS(_class)->scope);
}

dcScope *dcClass_getMetaScope(const dcNode *_class)
{
    return (_class == NULL
            ? NULL
            : CAST_CLASS(_class)->classTemplate->metaScope);
}

dcScope *dcClass_getUsedScope(const dcNode *_class)
{
    dcClass *klass = CAST_CLASS(_class);
    return (klass->isObject
            ? klass->scope
            : klass->classTemplate->metaScope);
}

const char *dcClass_getName(const dcNode *_class)
{
    return CAST_CLASS(_class)->classTemplate->className;
}

//
// not shared across threads
//
void dcClass_markNode(dcNode *_node)
{
    dcClass *klass = CAST_CLASS(_node);
    dcNode_mark(klass->superNode);
    dcScope_mark(klass->scope);

    if (dcClass_isObject(_node)
        && klass->classTemplate->cTemplate != NULL
        && klass->classTemplate->cTemplate->markPointer != NULL)
    {
        klass->classTemplate->cTemplate->markPointer(_node);
    }
    else if (! dcClass_isObject(_node))
    {
        dcScope_mark(klass->classTemplate->scope);
        dcScope_mark(klass->classTemplate->metaScope);

        if (klass->classTemplate->cTemplate != NULL
            && klass->classTemplate->cTemplate->metaMarkPointer != NULL)
        {
            klass->classTemplate->cTemplate->metaMarkPointer(_node);
        }
    }
}

//
// not shared across threads
//
void dcClass_registerNode(dcNode *_node)
{
    dcClass *klass = CAST_CLASS(_node);

    if (dcClass_isObject(_node)
        && klass->classTemplate->cTemplate != NULL
        && klass->classTemplate->cTemplate->registerPointer != NULL)
    {
        klass->classTemplate->cTemplate->registerPointer(_node);
    }

    dcNode_register(klass->superNode);
    dcScope_register(klass->scope);
}

dcResult dcClass_printNode(const dcNode *_node, dcString *_string)
{
    dcResult result = TAFFY_SUCCESS;

    if (dcClass_getTemplate(_node) == dcNumberClass_getTemplate()
        && dcClass_isObject(_node))
    {
        char *display = dcNumber_display(dcNumberClass_getNumber(_node));
        dcString_appendString(_string, display);
        dcMemory_free(display);
    }
    else
    {
        const char *display = dcStringClass_asString_helper((dcNode*)_node);

        if (display == NULL)
        {
            result = TAFFY_EXCEPTION;
        }
        else
        {
            dcString_appendString(_string, display);
        }
    }

    return result;
}

dcResult dcClass_prettyPrintNode(const dcNode *_node,
                                 dcCharacterGraph **_graph)
{
    dcResult result = TAFFY_SUCCESS;
    // der
    dcNode *display =
        dcNodeEvaluator_callMethod(dcSystem_getCurrentNodeEvaluator(),
                                   (dcNode *)_node,
                                   "prettyPrint");
    if (display == NULL)
    {
        result = TAFFY_EXCEPTION;
    }
    else
    {
        dcNode *casted = dcClass_castNode(display,
                                          dcStringClass_getTemplate(),
                                          true);

        if (casted == NULL)
        {
            result = TAFFY_EXCEPTION;
        }
        else
        {
            *_graph = (dcCharacterGraph_createFromCharString
                       (dcStringClass_getString(casted)));
        }
    }

    return result;
}

void dcClass_setTemplate(dcNode *_class, bool _yesNo)
{
    if (CAST_CLASS(_class) != NULL)
    {
        dcNode *that = _class;

        for (that = CAST_CLASS(that)->superNode;
             that != NULL;
             that = CAST_CLASS(that)->superNode)
        {
            dcNode_setTemplate(that, _yesNo);
        }

        dcClass *klass = CAST_CLASS(_class);

        if (klass->classTemplate->cTemplate != NULL
            && klass->classTemplate->cTemplate->setTemplatePointer != NULL)
        {
            klass->classTemplate->cTemplate->setTemplatePointer(_class, _yesNo);
        }
    }
}

void dcClass_setSuperNode(dcNode *_class, dcNode *_superNode)
{
    CAST_CLASS(_class)->superNode = _superNode;
}

void dcClass_setScope(dcClass *_class, dcScope *_scope)
{
    dcScope_free(&_class->scope, DC_SHALLOW);
    _class->scope = _scope;
}

dcNode *dcClass_set(dcNode *_class,
                    const char *_name,
                    dcNode *_object,
                    dcScopeDataFlags _flags,
                    bool _local)
{
    dcScope *scope = NULL;

    if (_local)
    {
        dcError_assert((_flags & SCOPE_DATA_META) == 0);
        scope = CAST_CLASS(_class)->scope;
    }
    else
    {
        scope = ((_flags & SCOPE_DATA_INSTANCE) != 0
                 ? CAST_CLASS(_class)->classTemplate->scope
                 : CAST_CLASS(_class)->classTemplate->metaScope);
    }

    dcNode *result = dcScope_set(scope, _object, _name, _flags);

    if ((_flags & SCOPE_DATA_OBJECT) != 0)
    {
        if ((_flags & SCOPE_DATA_READER) != 0)
        {
            dcClass_makeReader(_class, _name, (_flags & ~SCOPE_DATA_OBJECT));
        }

        if ((_flags & SCOPE_DATA_WRITER) != 0)
        {
            dcClass_makeWriter(_class, _name, (_flags & ~SCOPE_DATA_OBJECT));
        }
    }

    return result;
}

dcNode *dcClass_getScopeData(dcNode *_receiver,
                             const char *_objectName,
                             dcScopeDataFlags _flags,
                             bool _searchUp,
                             dcNode *_requestor,
                             dcClassTemplate **_foundTemplate,
                             dcScope **_scopeResult)
{
    dcNode *result = NULL;
    dcNode *that = _receiver;
    bool exception = false;

    while (result == NULL
           && that != NULL
           && ! exception)
    {
        TAFFY_DEBUG(dcError_assert(IS_CLASS(that)));
        dcClass *klass = CAST_CLASS(that);

        // set up the correct scopes to iterate over
        dcScope *objectScopes[] = {klass->scope,
                                   klass->classTemplate->scope,
                                   NULL};
        dcScope *metaScopes[] = {klass->classTemplate->metaScope,
                                 NULL};
        dcScope **finger = NULL;
        bool metaObject = (strlen(_objectName) > 2
                           && _objectName[0] == '@'
                           && _objectName[1] == '@');

        for (finger = (klass->isObject && ! metaObject
                       ? objectScopes
                       : metaScopes);
             *finger != NULL && ! exception;
             finger++)
        {
            result = dcScope_getScopeData(*finger, _objectName, _flags);

            if (result != NULL)
            {
                dcScopeData *scopeData = CAST_SCOPE_DATA(result);

                if ((scopeData->flags & SCOPE_DATA_PROTECTED) != 0
                    && _requestor != _receiver
                    && (dcClass_castNode(_requestor,
                                         dcClass_getTemplate(_receiver),
                                         false)
                        == NULL))
                {
                    // failure!
                    result = NULL;
                    exception = true;
                    break;
                }

                // we found it!
                if (_foundTemplate != NULL)
                {
                    *_foundTemplate = klass->classTemplate;
                }
            }

            if (result != NULL)
            {
                // we're done!
                if (_scopeResult != NULL)
                {
                    *_scopeResult = *finger;
                }

                break;
            }
        }

        if (result == NULL && ! exception)
        {
            that = dcClass_getOrInstantiateSuperNode(that);

            if (that != NULL)
            {
                result = dcClass_getScopeData
                    (that,
                     _objectName,
                     _flags,
                     _searchUp,
                     _requestor,
                     _foundTemplate,
                     _scopeResult);
            }
        }
    }

    return result;
}

static dcNode *extractObject(dcNode *_scopeData)
{
    return (_scopeData == NULL
            ? NULL
            : dcScopeData_getObject(_scopeData));
}

dcNode *dcClass_getObject(dcNode *_receiver, const char *_objectName)
{
    return extractObject(dcClass_getScopeDataForObject(_receiver,
                                                       _objectName,
                                                       true,
                                                       _receiver, // requestor
                                                       NULL,   // found template
                                                       NULL)); // scope result
}

dcNode *dcClass_getMethod(dcNode *_receiver, const char *_methodName)
{
    return extractObject(dcClass_getScopeDataForMethod(_receiver,
                                                       _methodName,
                                                       true,
                                                       _receiver, // requestor
                                                       NULL,   // found template
                                                       NULL)); // scope result
}

void dcClass_makeReader(dcNode *_class,
                        const char *_variableName,
                        dcScopeDataFlags _flags)
{
    //TAFFY_DEBUG(dcError_assert(dcNode_isTemplate(_class)););

    dcScope *scope = NULL;
    const char *suppliedArguments[2] = {0};

    if (_flags & SCOPE_DATA_META)
    {
        // get past the @@ for meta, index 2
        suppliedArguments[0] = dcMemory_strdup(&_variableName[2]);
        scope = CAST_CLASS(_class)->classTemplate->metaScope;
    }
    else
    {
        // get past the @ for instance, index 1
        suppliedArguments[0] = dcMemory_strdup(&_variableName[1]);
        scope = CAST_CLASS(_class)->classTemplate->scope;
    }

    if (dcScope_getScopeDataForMethod(scope, suppliedArguments[0]) == NULL)
    {
        dcCFunctionArgument argumentTypes[2] =
            {MAKE_FULLY_QUALIFIED(SUPPLIED_ARGUMENT), NULL};
        dcScopeDataFlags flags = _flags | SCOPE_DATA_SYNCHRONIZED;

        dcNode *methodClass =
            dcNode_setTemplate
            (dcProcedureClass_createObject
             (dcNode_setTemplate
              (dcTaffyCMethodPointer_createNode(&dcObjectClass_getVariable),
               true),
              dcMethodHeader_createCDefinition(suppliedArguments[0],
                                               argumentTypes,
                                               suppliedArguments,
                                               1)),
             true); // is a template yup yup

        dcScope_setMethod(scope, methodClass, suppliedArguments[0], flags);
    }

    dcMemory_free(suppliedArguments[0]);
}

void dcClass_makeWriter(dcNode *_class,
                        const char *_variableName,
                        dcScopeDataFlags _flags)
{
    //TAFFY_DEBUG(dcError_assert(dcNode_isTemplate(_class)););

    dcScope *scope = NULL;
    dcCFunctionArgument argumentTypes[3] =
        {MAKE_FULLY_QUALIFIED(SUPPLIED_ARGUMENT),
         MAKE_FULLY_QUALIFIED(WILD),
         NULL};
    char *suppliedArguments[2] = {0};

    if ((_flags & SCOPE_DATA_META) != 0)
    {
        // get past @@, index 2
        suppliedArguments[0] = dcMemory_strdup(&_variableName[2]);
        scope = CAST_CLASS(_class)->classTemplate->metaScope;
    }
    else
    {
        // get past @, index 1
        suppliedArguments[0] = dcMemory_strdup(&_variableName[1]);
        scope = CAST_CLASS(_class)->classTemplate->scope;
    }

    char *setterName =
        dcLexer_sprintf
        ("set%c%s:",
         toupper((unsigned char)suppliedArguments[0][0]),
         suppliedArguments[0] + 1);

    if (dcScope_getScopeDataForMethod(scope, setterName) == NULL)
    {
        dcScopeDataFlags flags = _flags | SCOPE_DATA_SYNCHRONIZED;
        dcNode *methodClass = dcNode_setTemplate
            (dcProcedureClass_createObject
             (dcNode_setTemplate
              (dcTaffyCMethodPointer_createNode(&dcObjectClass_setVariable),
               true),
              dcMethodHeader_createCDefinition(setterName,
                                               argumentTypes,
                                               (const char **)suppliedArguments,
                                               1)),
             true); // is a template, yes yes
        dcScope_setMethod(scope, methodClass, setterName, flags);
    }

    dcMemory_free(setterName);
    dcMemory_free(suppliedArguments[0]);
}

bool dcClass_isKindOfTemplate(const dcNode *_node,
                              const dcClassTemplate *_template)
{
    dcClassTemplate *that = CAST_CLASS(_node)->classTemplate;

    if (_template == NULL)
    {
        dcGarbageCollector_forceStop();
        dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

        fprintf(stderr, "Fatal error: Can't determine type of template\n");

        if (evaluator->exception != NULL)
        {
            fprintf(stderr,
                    "Current exception text:\n%s\n",
                    dcNode_display(evaluator->exception));

            char *callStackDisplay =
                dcNodeEvaluator_displayExceptionCallStack(evaluator);

            fprintf(stderr, "Callstack:\n%s\n", callStackDisplay);
            dcMemory_free(callStackDisplay);
        }

        assert(false);
    }

    while (that != NULL && that != _template)
    {
        that = that->superTemplate;
    }

    return (that == _template);
}

bool dcClass_isType(const dcNode *_node,
                      const char *_packageName,
                      const char *_className)
{
    dcClass *klass = CAST_CLASS(_node);

    return (strcmp(klass->classTemplate->packageName, _packageName) == 0
            && strcmp(klass->classTemplate->className, _className) == 0);
}

bool dcClass_hasTemplate(const dcNode *_node,
                         const dcClassTemplate *_template,
                         bool _object)
{
    return (IS_CLASS(_node)
            && CAST_CLASS(_node)->classTemplate == _template
            && dcClass_isObject(_node) == _object);
}

bool dcClass_isMetaClass(const dcNode *_node)
{
    return (dcGraphData_isType(_node, NODE_CLASS)
            && ! dcClass_isObject(_node));
}

dcNode *dcClass_castNodeWithAssert(dcNode *_node,
                                   const dcClassTemplate *_toTemplate,
                                   bool _throwException,
                                   bool _assertCast)
{
    dcError_assert(_toTemplate != NULL);
    dcNode *casted = _node;

    // if toTemplate is wild, then we won't cast
    if (_toTemplate != dcWildClass_getTemplate())
    {
        //
        // if casted can't be casted, then it will become NULL
        //
        while (casted != NULL && dcClass_getTemplate(casted) != _toTemplate)
        {
            casted = dcClass_getOrInstantiateSuperNode(casted);
        }

        if (casted == NULL)
        {
            if (_throwException)
            {
                // cast failed! //
                dcInvalidCastExceptionClass_throwObject
                    (dcClass_getName(_node), _toTemplate->className);
            }

            dcError_assert(! _assertCast);
        }
        else
        {
            TAFFY_DEBUG(dcError_assert(dcClass_getTemplate(casted)
                                       == _toTemplate));
        }
    }

    return casted;
}

bool dcClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    CAST_CLASS(_node) = NULL;
    dcScope *scope = NULL;
    bool isObject = false;
    char *className = NULL;
    bool allocateAuxiliaryData = true;
    uint16_t templateId = 0;
    dcClassTemplate *classTemplate = NULL;

    if (dcString_getLengthLeft(_stream) < CLASS_MARSHALL_SIZE)
    {
        // FAILURE //
        goto kickout;
    }

    if (! dcMarshaller_unmarshall(_stream, "v", &templateId))
    {
        // FAILURE //
        goto kickout;
    }

    classTemplate = dcClassManager_getClassTemplateFromId(templateId);

    if (classTemplate == NULL
        || ! dcMarshaller_unmarshall(_stream, "cS", &isObject, &scope))
    {
        // FAILURE //
        goto kickout;
    }

    allocateAuxiliaryData =
        (classTemplate->cTemplate == NULL
         || classTemplate->cTemplate->unmarshallPointer == NULL);

    fillClass(_node,
              classTemplate,
              isObject,
              scope,
              NULL,
              NULL,
              allocateAuxiliaryData);

    result = true;

    if (! allocateAuxiliaryData)
    {
        result = classTemplate->cTemplate->unmarshallPointer(_node, _stream);
    }

    // allow for NULL superNode
    if (! result
        || ! dcMarshaller_unmarshall(_stream,
                                     "n",
                                     &CAST_CLASS(_node)->superNode)
        || (CAST_CLASS(_node)->superNode != NULL
            && ! IS_CLASS(CAST_CLASS(_node)->superNode)))
    {
        if (CAST_CLASS(_node)->superNode != NULL
            && ! IS_CLASS(CAST_CLASS(_node)->superNode))
        {
            dcNode_free(&CAST_CLASS(_node)->superNode, DC_DEEP);
            CAST_CLASS(_node)->superNode = NULL;
        }

        // set the node as a template so class-specific free functions
        // can free all their contents
        dcNode_setTemplate(_node, true);
        dcClass_freeNode(_node, DC_DEEP);
        scope = NULL;
        result = false;
    }

 kickout:

    if (! result)
    {
        if (scope != NULL && CAST_CLASS(_node) == NULL)
        {
            dcScope_free(&scope, DC_DEEP);
        }
    }

    dcMemory_free(className);
    return result;
}

dcString *dcClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcClass *klass = CAST_CLASS(_node);
    dcString_needBytes(_stream, CLASS_MARSHALL_SIZE);

    _stream = dcMarshaller_marshall(_stream,
                                    "vcS",
                                    klass->classTemplate->marshallId,
                                    klass->isObject,
                                    klass->scope);

    if (klass->classTemplate != NULL
        && klass->classTemplate->cTemplate != NULL
        && klass->classTemplate->cTemplate->marshallPointer != NULL)
    {
        klass->classTemplate->cTemplate->marshallPointer(_node, _stream);
    }

    dcNode_marshall(klass->superNode, _stream);
    return _stream;
}

void dcClass_lock(dcNode *_node)
{
    assert(CAST_CLASS(_node)->mutex != NULL);
    dcMutex_lock(CAST_CLASS(_node)->mutex);
}

void dcClass_lockForRead(dcNode *_node)
{
    assert(CAST_CLASS(_node)->readWriteLock != NULL);
    dcReadWriteLock_lockForRead(CAST_CLASS(_node)->readWriteLock);
}

void dcClass_lockForWrite(dcNode *_node)
{
    assert(CAST_CLASS(_node)->readWriteLock != NULL);
    dcReadWriteLock_lockForWrite(CAST_CLASS(_node)->readWriteLock);
}

void dcClass_unlock(dcNode *_node)
{
    assert(CAST_CLASS(_node)->mutex != NULL);
    dcMutex_unlock(CAST_CLASS(_node)->mutex);
}

void dcClass_unlockReadWriteLock(dcNode *_node)
{
    assert(CAST_CLASS(_node)->readWriteLock != NULL);
    dcReadWriteLock_unlock(CAST_CLASS(_node)->readWriteLock);
}

dcResult dcClass_compareNode(dcNode *_left,
                             dcNode *_right,
                             dcTaffyOperator *_compareResult)
{
    return dcNodeEvaluator_compareObjects(dcSystem_getCurrentNodeEvaluator(),
                                          _left,
                                          _right,
                                          _compareResult);
}


dcResult dcClass_compareEqual(dcNode *_left,
                              dcNode *_right,
                              dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_SUCCESS;
    dcNode *resultNode = (dcNodeEvaluator_callMethodWithArgument
                          (dcSystem_getCurrentNodeEvaluator(),
                           _left,
                           dcSystem_getOperatorName(TAFFY_EQUALS),
                           _right));

    if (resultNode == NULL)
    {
        result = TAFFY_EXCEPTION;
    }
    else
    {
        result = TAFFY_SUCCESS;
        *_compareResult = (dcYesClass_isMe(resultNode)
                           ? TAFFY_EQUALS
                           : TAFFY_LESS_THAN);
    }

    return result;
}

dcResult dcClass_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    dcResult result = TAFFY_SUCCESS;

    // special speedup for number class, since it's final
    if (dcClass_getTemplate(_node) == dcNumberClass_getTemplate())
    {
        *_hashResult = dcNumberClass_hashHelper(_node);
    }
    else
    {
        result = dcHashClass_hashifyNode(_node, _hashResult);
    }

    return result;
}

dcClassTemplate *dcClass_getTemplate(const dcNode *_classNode)
{
    return CAST_CLASS(_classNode)->classTemplate;
}

// debugging hooks
void dcClass_printNodeType(const dcNode *_node)
{
    dcIOClass_printFormat("%s\n", dcClass_getName(_node));
}

dcClass *dcClass_getClass(dcNode *_node)
{
    return CAST_CLASS(_node);
}

void dcClass_printInformation(const dcNode *_class)
{
    dcClassTemplate *classTemplate = CAST_CLASS(_class)->classTemplate;

    dcIOClass_printFormat("Class: '%s'\n"
                          "Super: '%s'\n"
                          "Package Name: '%s'\n"
                          "Scope count: %lu\n"
                          "Meta Scope count: %lu\n",
                          classTemplate->className,
                          classTemplate->superName,
                          classTemplate->packageName,
                          dcScope_getSize(classTemplate->scope),
                          dcScope_getSize(classTemplate->metaScope));
}

void dcClass_deallocateNode(dcNode *_node)
{
    dcClass *klass = CAST_CLASS(_node);

    if (dcClass_isObject(_node)
        && klass->classTemplate->cTemplate != NULL
        && klass->classTemplate->cTemplate->deallocatePointer != NULL)
    {
        klass->classTemplate->cTemplate->deallocatePointer(_node);
    }

    dcScope_clear(dcClass_getClass(_node)->scope, DC_SHALLOW);
}

bool dcClass_isObject(const dcNode *_node)
{
    return CAST_CLASS(_node)->isObject;
}

dcClassFlags dcClass_getFlags(const dcNode *_class)
{
    return (CAST_CLASS(_class)->classTemplate->classFlags);
}

bool dcClass_isSlice(const dcNode *_class)
{
    return ((dcClass_getFlags(_class) & CLASS_SLICE) != 0);
}

bool dcClass_isSingleton(const dcNode *_class)
{
    return ((CAST_CLASS(_class)->classTemplate->classFlags
             & CLASS_SINGLETON)
            != 0);
}

bool dcClass_isMe(const dcNode *_class)
{
    return IS_CLASS(_class);
}

dcClass *dcClass_castMe(dcNode *_class)
{
    return CAST_CLASS(_class);
}

dcNode *dcClass_copyIfAtomic(dcNode *_class)
{
    return (dcClassTemplate_isAtomic(dcClass_getTemplate(_class))
            ? dcNode_register(dcNode_copy(_class, DC_DEEP))
            : _class);
}

dcNode *dcClass_copyIfTemplateOrAtomic(dcNode *_class)
{
    dcNode *result = dcNode_copyIfTemplate(_class);

    if (result == _class)
    {
        result = dcClass_copyIfAtomic(result);
    }

    return result;
}

void *dcClass_getAux(dcNode *_node)
{
    return CAST_CLASS_AUX(_node);
}
