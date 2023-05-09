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
#include <string.h>

#include "dcError.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHash.h"
#include "dcIdentifier.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcLog.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcNumberClass.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringManager.h"
#include "dcPackage.h"
#include "dcPackageContents.h"
#include "dcVoid.h"
#include "dcVoidContainer.h"

typedef struct
{
    // the package contents head
    dcNode *packageContentsHead;

    // we store the class templates in a separate list so we can clear them
    // before freeing packageContentsHead, it's an ordering thing
    dcList *classTemplates;

    uint16_t maxMarshallId;
    uint16_t maxSingletonId;

    // map marshall id to template
    dcHash *idToTemplateMap;

    // map singleton id to node
    dcHash *singletonIdToNodeMap;

    dcMutex *mutex;
} dcClassManager;

static dcClassManager *sTheManager = NULL;

//
// The Class Manager
//

void dcClassManager_create(void)
{
    if (sTheManager != NULL)
    {
        return;
    }

    sTheManager = (dcClassManager *)dcMemory_allocate(sizeof(dcClassManager));
    sTheManager->packageContentsHead =
        dcPackageContents_createNode(dcPackage_create(dcList_create()));
    sTheManager->classTemplates = dcList_create();
    sTheManager->mutex = dcMutex_create(true);

    // 0 is an invalid id
    sTheManager->maxMarshallId = 1;
    sTheManager->maxSingletonId = 1;

    sTheManager->idToTemplateMap = dcHash_create();
    sTheManager->singletonIdToNodeMap = dcHash_create();
    dcGarbageCollector_addRoot(&dcClassManager_mark);
}

void dcClassManager_free(void)
{
    dcClassManager_clearClassTemplates();
    dcList_free(&sTheManager->classTemplates, DC_DEEP);
    dcHash_free(&sTheManager->idToTemplateMap, DC_DEEP);
    dcHash_free(&sTheManager->singletonIdToNodeMap, DC_DEEP);
    dcMutex_free(&sTheManager->mutex);
    dcMemory_free(sTheManager);
}

void dcClassManager_clearClassTemplates(void)
{
    dcMutex_lock(sTheManager->mutex);

    dcListElement *that = NULL;

    for (that = sTheManager->classTemplates->head;
         that != NULL;
         that = that->next)
    {
        dcClassTemplate_clear(CAST_CLASS_TEMPLATE(that->object), DC_DEEP);
    }

    dcNode_free(&sTheManager->packageContentsHead, DC_SHALLOW);
    dcMutex_unlock(sTheManager->mutex);
}

static bool behindProtectedBoundary(dcNode *_node,
                                    const dcClassTemplate *_requestorTemplate)
{
    bool result = false;
    bool isProtected = false;
    dcNode *parentNode;

    if (_requestorTemplate != NULL)
    {
        for (parentNode = _node;
             parentNode != NULL;
             parentNode = dcClass_getTemplate(parentNode)->parentNode)
        {
            if (dcClassTemplate_isProtected(dcClass_getTemplate(parentNode)))
            {
                isProtected = true;
            }

            if (dcClass_castNode(parentNode, _requestorTemplate, false)
                != NULL)
            {
                result = true;
                break;
            }
        }
    }

    return (isProtected && ! result);
}

static dcNode *findPackageContents(dcNode *_startContents,
                                   const dcPackage *_package,
                                   bool _create,
                                   const dcClassTemplate *_requestorTemplate)
{
    // create a path to the class
    const dcListElement *that = NULL;

    // the result
    dcNode *thatContents = _startContents;
    dcList *constructedPath = dcList_create();

    for (that = _package->path->head;
         that != NULL;
         that = that->next)
    {
        dcNode *next = NULL;
        dcList_push(constructedPath, that->object);
        const char *partName = dcString_getString(that->object);

        dcError_assert(dcHash_getValueWithStringKey
                       (CAST_PACKAGE_CONTENTS(thatContents)->subMap,
                        partName,
                        &next)
                       != TAFFY_EXCEPTION);

        if (next == NULL)
        {
            if (! _create)
            {
                // didn't find it
                thatContents = NULL;
                break;
            }

            next = dcPackageContents_createNode
                (dcPackage_create(dcList_copy(constructedPath, DC_DEEP)));

            dcError_assert(dcHash_setValueWithStringKey
                           (CAST_PACKAGE_CONTENTS(thatContents)->subMap,
                            dcString_getString(that->object),
                            next)
                           == TAFFY_SUCCESS);
        }
        else if (_requestorTemplate != NULL)
        {
            //
            // check security
            //
            dcNode *klass = NULL;

            dcError_assert(dcHash_getValueWithStringKey
                           (CAST_PACKAGE_CONTENTS(thatContents)->classes,
                            partName,
                            &klass)
                           != TAFFY_EXCEPTION);

            if (klass != NULL
                && behindProtectedBoundary(klass, _requestorTemplate))
            {
                // egggggggg
                // invalid access to a protected member
                thatContents = NULL;
                break;
            }
        }

        thatContents = next;
    }

    dcList_free(&constructedPath, DC_SHALLOW);
    return thatContents;
}

// protected above
static dcNode *findClassInPackageContents
    (dcNode *_startContents,
     const dcPackage *_package,
     const char *_className,
     const dcClassTemplate *_requestorTemplate)
{
    dcNode *contents = (_package == NULL
                        ? _startContents
                        : findPackageContents(_startContents,
                                              _package,
                                              false,
                                              _requestorTemplate));
    dcNode *result = (contents == NULL
                      ? NULL
                      : ((dcHash_getValueWithStringKey
                          (CAST_PACKAGE_CONTENTS(contents)->classes,
                           _className,
                           &result)
                          == TAFFY_SUCCESS)
                         ? result
                         : NULL));


    // check that we aren't invalidly accessing a protected class
    if (result != NULL
        && behindProtectedBoundary(result, _requestorTemplate))
    {
        result = NULL;
    }

    return result;
}

dcClassTemplate *dcClassManager_getClassTemplate
    (const char *_fullyQualifiedClassName,
     const dcPackage *_package,
     dcList *_packageContentsList,
     const dcClassTemplate *_requestorTemplate)
{
    dcNode *klass = dcClassManager_getClass(_fullyQualifiedClassName,
                                            _package,
                                            _packageContentsList,
                                            _requestorTemplate);
    return (klass == NULL
            ? NULL
            : dcClass_getTemplate(klass));
}

//
// _fullyQualitifiedClassCame: the object in question, like
//                             org.yourPackage.yourClass.yourSubClass
// _package: the current package, like org.taffy.core.container
// _packageContentsList: the package contents collected from previous import
// directives
//
dcNode *dcClassManager_getClass(const char *_fullyQualifiedClassName,
                                const dcPackage *_package,
                                dcList *_packageContentsList,
                                const dcClassTemplate *_requestorTemplate)
{
    dcPackage *classPackage =
        dcPackage_createFromString(_fullyQualifiedClassName);
    dcNode *result = dcClassManager_getClassFromPackage(classPackage,
                                                        _package,
                                                        _packageContentsList,
                                                        _requestorTemplate);
    dcPackage_free(&classPackage);
    return result;
}

dcNode *dcClassManager_getClassFromPackage
    (const dcPackage *_classNamePackage,
     const dcPackage *_package,
     dcList *_packageContentsList,
     const dcClassTemplate *_requestorTemplate)
{
    //
    // this entire operation must be protected
    //
    dcMutex_lock(sTheManager->mutex);

    dcPackage *classNamePackage = dcPackage_copy(_classNamePackage, DC_DEEP);
    char *className = dcMemory_strdup(dcPackage_getTail(classNamePackage));
    dcPackage_pop(classNamePackage);
    char *classBase = (classNamePackage->path->size == 0
                       ? NULL
                       : dcPackage_getPathString(classNamePackage));

    // can be found in:
    // packageContentsHead + classNamePackage + className
    // {each of importedPackageContentsList} + classNamePackage + className
    // baseContents + classNamePackage + className

    dcNode *result = findClassInPackageContents
        (sTheManager->packageContentsHead,
         classNamePackage,
         className,
         _requestorTemplate);

    if (result == NULL && _package != NULL)
    {
        dcNode *contents = dcClassManager_getPackageContents(_package);

        if (contents != NULL)
        {
            result = findClassInPackageContents
                (contents, classNamePackage, className, _requestorTemplate);
        }
    }

    if (result == NULL && _packageContentsList != NULL)
    {
        dcListElement *that = NULL;

        for (that = _packageContentsList->head;
             that != NULL && result == NULL;
             that = that->next)
        {
            if (that->object->type == NODE_PACKAGE_CONTENTS)
            {
                result = findClassInPackageContents
                    (that->object,
                     classNamePackage,
                     className,
                     _requestorTemplate);

                if (result != NULL)
                {
                    break;
                }
            }
            else if (IS_CLASS(that->object))
            {
                dcClass *metaInQuestion = CAST_CLASS(that->object);

                if (strcmp(metaInQuestion->classTemplate->className,
                           className)
                    == 0)
                {
                    if (! behindProtectedBoundary(that->object,
                                                  _requestorTemplate))
                    {
                        result = that->object;
                    }

                    break;
                }
            }
            else
            {
                dcError_assert(false);
            }
        }
    }

    if (result == NULL && _requestorTemplate != NULL)
    {
        //
        // search within the requestor template for the class
        // this covers the case when class Y is defined within class X,
        // and we instantiate class Y inside class X:
        //
        // class X
        // {
        //     class Y {}
        //
        //     (@) init
        //     {
        //         new Y // <<----
        //     }
        // }
        //
        dcPackage *package =
            dcPackage_copy(_requestorTemplate->package, DC_DEEP);
        dcPackage_append(package, _requestorTemplate->className);

        if (classBase != NULL)
        {
            dcPackage_append(package, classBase);
        }

        result = findClassInPackageContents(sTheManager->packageContentsHead,
                                            package,
                                            className,
                                            NULL);
        dcPackage_free(&package);
    }

    dcMutex_unlock(sTheManager->mutex);
    dcPackage_free(&classNamePackage);
    dcMemory_free(className);

    if (classBase != NULL)
    {
        dcMemory_free(classBase);
    }

    return result;
}

dcNode *dcClassManager_getPackageContents(const struct dcPackage_t *_package)
{
    dcMutex_lock(sTheManager->mutex);
    dcNode *result = findPackageContents(sTheManager->packageContentsHead,
                                         _package,
                                         false,
                                         NULL);
    dcMutex_unlock(sTheManager->mutex);
    return result;
}

dcResult dcClassManager_registerClassTemplate(dcClassTemplate *_template,
                                              dcList *_packageContents,
                                              bool _initializePlease,
                                              dcNode **_metaResult)
{
    //
    // this entire operation needs to be protected
    //
    dcMutex_lock(sTheManager->mutex);

    dcNode *metaResult = NULL;
    dcResult result = TAFFY_SUCCESS;
    dcPackage *package = dcPackage_createFromString(_template->packageName);
    dcPackageContents *packageContents = CAST_PACKAGE_CONTENTS
        (findPackageContents(sTheManager->packageContentsHead,
                             package,
                             true,
                             NULL));
    // sanity
    if ((dcHash_getValueWithStringKey
         (packageContents->classes, _template->className, NULL)
         == TAFFY_FAILURE))
    {
        //
        // success
        //
        // create a meta object
        metaResult = dcNode_register(dcClass_createBasicNode(_template, false));

        // der?
        dcGraphData_setPosition(CAST_GRAPH_DATA(metaResult),
                                1,
                                dcStringManager_getStringId("ClassManager.ty"));

        // place the object into the classes hash
        dcHash_setValueWithStringKey(packageContents->classes,
                                     _template->className,
                                     metaResult);

        // add the class to the id maps for marshall help
        _template->marshallId = sTheManager->maxMarshallId;
        dcHash_setValueWithHashValue
            (sTheManager->idToTemplateMap,
             NULL,
             sTheManager->maxMarshallId,
             dcVoidContainer_createNode(_template));
        sTheManager->maxMarshallId++;

        dcMutex_unlock(sTheManager->mutex);

        if (_initializePlease)
        {
            result = dcClassTemplate_createRuntimeValues
                (_template, _packageContents);

            // call the meta initializer if it exists
            if (result == TAFFY_SUCCESS)
            {
                if (_template->cTemplate != NULL
                    && _template->cTemplate->initializer != NULL)
                {
                    _template->cTemplate->initializer();
                }

                //
                // now <create> the class template runtime values for each
                // sub-class
                //
                dcHashIterator *i = dcScope_createIterator
                    (dcClass_getMetaScope(metaResult));
                dcNode *scopeData = NULL;

                while ((scopeData = dcScope_getNext(i, SCOPE_DATA_OBJECT))
                       != NULL)
                {
                    dcNode *object = dcScopeData_getObject(scopeData);

                    if (dcGraphData_isType(object, NODE_CLASS)
                        && ! dcClass_isObject(object))
                    {
                        dcClassTemplate_createRuntimeValues
                            (dcClass_getTemplate(object), _packageContents);
                    }
                }

                dcHashIterator_free(&i);
                // /<create>
            }
        }

        dcMutex_lock(sTheManager->mutex);

        if (result == TAFFY_SUCCESS)
        {
            dcList_push(sTheManager->classTemplates,
                        dcClassTemplate_createShell(_template));
        }
        else
        {
            metaResult = NULL;
            dcHash_removeValueWithStringKey(packageContents->classes,
                                            _template->className,
                                            NULL,
                                            DC_DEEP);
        }

        dcMutex_unlock(sTheManager->mutex);

        if (_metaResult != NULL)
        {
            *_metaResult = metaResult;
        }
    }
    else
    {
        dcMutex_unlock(sTheManager->mutex);

        //
        // failure
        //
        result = TAFFY_FAILURE;
    }

    dcPackage_free(&package);

    if (result != TAFFY_SUCCESS
        && _metaResult != NULL)
    {
        *_metaResult = NULL;
    }

    return result;
}

void dcClassManager_addClassTemplateTemplate(dcClassTemplate *_template)
{
    dcMutex_lock(sTheManager->mutex);
    dcList_push(sTheManager->classTemplates,
                dcClassTemplate_createShell(_template));
    dcMutex_unlock(sTheManager->mutex);
}

typedef void (EachFunction)(dcPackageContents *_contents);

static void markPackage(dcPackageContents *_contents)
{
    if (dcLog_isEnabled(CLASS_MANAGER_LOG))
    {
        char *pathString = dcPackage_getPathString(_contents->package);

        dcLog_log(CLASS_MANAGER_LOG,
                  "marking package: '%s'\n",
                  pathString);

        dcHashIterator *i = dcHashIterator_create(_contents->classes);
        dcNode *meta = NULL;

        while ((meta = dcHashIterator_getNextValue(i))
               != NULL)
        {
            dcLog_log(CLASS_MANAGER_LOG,
                      "marking class with name: %s\n",
                      dcClass_getName(meta));
        }

        dcHashIterator_free(&i);
        dcLog_log(CLASS_MANAGER_LOG,
                  "done marking '%s'\n",
                  pathString);

        dcMemory_free(pathString);
    }

    dcHash_mark(_contents->classes);
}

static dcResult eachPackageHelper(dcNode *_package, dcNode *_token)
{
    EachFunction *function = (EachFunction*)CAST_VOID(_token);
    function(CAST_PACKAGE_CONTENTS(_package));
    dcHash_eachValue(CAST_PACKAGE_CONTENTS(_package)->subMap,
                     &eachPackageHelper,
                     _token);
    return TAFFY_SUCCESS;
}

static void eachPackage(EachFunction *_function)
{
    dcNode *token = dcVoid_createNode((void*)_function);
    eachPackageHelper(sTheManager->packageContentsHead, token);
    dcHash_eachValue(CAST_PACKAGE_CONTENTS
                     (sTheManager->packageContentsHead)->subMap,
                     &eachPackageHelper,
                     token);
    dcNode_free(&token, DC_SHALLOW);
}

void dcClassManager_mark(void)
{
    // no locking needed here since only the GC accesses this code
    eachPackage(&markPackage);
}

bool dcClassManager_unmarshall(dcString *_stream)
{
    return true;
}

dcString *dcClassManager_marshall(dcString *_stream)
{
    return _stream;
}

char *dcClassManager_displayClasses(void)
{
    dcString *result = dcString_create();

    FOR_EACH_IN_LIST(sTheManager->classTemplates, that)
    {
        dcClassTemplate *classTemplate = CAST_CLASS_TEMPLATE(that->object);
        dcString_append(result,
                        "%s.%s\n",
                        classTemplate->packageName,
                        classTemplate->className);
    }

    return dcString_freeAndReturn(&result);
}

dcClassTemplate *dcClassManager_getClassTemplateFromId(uint16_t _id)
{
    dcNode *value;
    dcClassTemplate *result = NULL;

    if (dcHash_getValueWithKeys(sTheManager->idToTemplateMap,
                                NULL,
                                _id,
                                &value)
        == TAFFY_SUCCESS)
    {
        result = (dcClassTemplate *)CAST_VOID(value);
    }

    return result;
}

void dcClassManager_registerSingleton(dcNode *_node, const char *_name)
{
    assert(IS_CLASS(_node));
    dcClassTemplate *classTemplate = dcClass_getTemplate(_node);
    assert(classTemplate->singletonId == 0);
    classTemplate->singletonId = sTheManager->maxSingletonId;
    dcHash_setValueWithHashValue
        (sTheManager->singletonIdToNodeMap,
         NULL,
         sTheManager->maxSingletonId,
         dcIdentifier_createNode(_name, NO_FLAGS));
    assert(sTheManager->maxSingletonId < 0xFFFF);
    sTheManager->maxSingletonId++;
}

dcNode *dcClassManager_getSingletonFromId(uint16_t _id)
{
    dcNode *result;
    dcHash_getValueWithKeys(sTheManager->singletonIdToNodeMap,
                            NULL,
                            _id,
                            &result);
    return result;
}
