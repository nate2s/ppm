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

#include <ctype.h>
#include <string.h>

#include "dcObjectClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcFilePackageData.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHash.h"
#include "dcUnsignedInt32.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcPackage.h"
#include "dcPairClass.h"
#include "dcProcedureClass.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcSymbolClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcYesClass.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "address",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_address,
        gCFunctionArgument_none
    },
    {
        "allMethods",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_allMethods,
        gCFunctionArgument_none
    },
    {
        "allObjects",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_allObjects,
        gCFunctionArgument_none
    },
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_asString,
        gCFunctionArgument_none
    },
    {
        "attach:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_NO_CAST),
        &dcObjectClass_attach,
        gCFunctionArgument_wild
    },
    {
        "castAs:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_castAs,
        gCFunctionArgument_wild
    },
    {
        "copy",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_copy,
        gCFunctionArgument_none
    },
    {
        "class",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_class,
        gCFunctionArgument_none
    },
    {
        "className",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_className,
        gCFunctionArgument_none
    },
    {
        "#operator(==):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_equals,
        gCFunctionArgument_wild
    },
    {
        "get",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_CONST),
        &dcObjectClass_getVariable,
        gCFunctionArgument_wild
    },
    {
        "hash",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_hash,
        gCFunctionArgument_none
    },
    {
        "hasMethod?:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_hasMethod,
        gCFunctionArgument_string
    },
    {
        "init",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST),
        &dcObjectClass_init,
        gCFunctionArgument_none
    },
    {
        "kindOf?:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_kindOf,
        gCFunctionArgument_wild
    },
    {
        "objects",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_objects,
        gCFunctionArgument_none
    },
    {
        "methods",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_methods,
        gCFunctionArgument_none
    },
    {
        "perform:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST),
        &dcObjectClass_perform,
        gCFunctionArgument_string
    },
    {
        "perform:with:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST),
        &dcObjectClass_performWith,
        gCFunctionArgument_stringArray
    },
    {
        "prettyPrint",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_prettyPrint,
        gCFunctionArgument_none
    },
    {
        "set",
        (SCOPE_DATA_PROTECTED
         | SCOPE_DATA_SYNCHRONIZED),
        &dcObjectClass_setVariable,
        gCFunctionArgument_wildWild
    },
    {
        "setValue:forObject:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_NO_CAST),
        &dcObjectClass_setValueForObject,
        gCFunctionArgument_wildString
    },
    {
        "super",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_NO_CAST
         | SCOPE_DATA_CONST),
        &dcObjectClass_super,
        gCFunctionArgument_none
    },
    {
        0
    }
};

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcObjectClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (OBJECT_PACKAGE_NAME,                  // package name
          OBJECT_CLASS_NAME,                    // class name
          NULL,                                 // super name
          CLASS_ABSTRACT,                       // class flags
          NO_FLAGS,                             // scope data flags
          sMethodWrappers,                      // meta methods
          sMethodWrappers,                      // method wrappers
          NULL,                                 // initialization function
          NULL,                                 // deinitialization function
          NULL,                                 // allocate
          NULL,                                 // deallocate
          NULL,                                 // meta mark
          NULL,                                 // mark
          NULL,                                 // copy
          NULL,                                 // free
          NULL,                                 // register
          NULL,                                 // marshall
          NULL,                                 // unmarshall
          NULL));                               // set template
};

dcNode *dcObjectClass_createNode(void)
{
    return dcClass_createBasicNode(sTemplate, false);
}

dcNode *dcObjectClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcStringClass_createObject
             (dcLexer_sprintf("#%s%s",
                              dcClass_getName(_receiver),
                              dcClass_isObject(_receiver)
                              ? ""
                              : "-meta"),
              false)));
}

dcNode *dcObjectClass_equals(dcNode *_receiver, dcArray *_arguments)
{
    return (_receiver == dcArray_get(_arguments, 0)
            ? dcYesClass_getInstance()
            : dcNoClass_getInstance());
}

dcNode *dcObjectClass_address(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcStringClass_createObject
             (dcLexer_sprintf("%p", _receiver), false)));
}

dcNode *dcObjectClass_class(dcNode *_receiver, dcArray *_arguments)
{
    dcClassTemplate *classTemplate = dcClass_getTemplate(_receiver);
    dcNode *result = dcClassManager_getClass(classTemplate->className,
                                             classTemplate->package,
                                             NULL,
                                             NULL);
    dcError_assert(result != NULL);
    return result;
}

dcNode *dcObjectClass_className(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcStringClass_createObject(dcClass_getName(_receiver), true)));
}

dcNode *dcObjectClass_getVariable(dcNode *_receiver, dcArray *_arguments)
{
    // extract the variable name out of _arguments //
    const char *preVariableName =
        dcStringClass_getString(dcArray_get(_arguments, 0));
    char *variableName = NULL;

    if (dcClass_isObject(_receiver))
    {
        variableName = dcLexer_sprintf("@%s", preVariableName);
    }
    else
    {
        variableName = dcLexer_sprintf("@@%s", preVariableName);
    }

    // search for the variable in _receiver's scope //
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcNode *result = dcClass_getScopeData(_receiver,
                                          variableName,
                                          SCOPE_DATA_OBJECT,
                                          true,  // do search up
                                          NULL,  // no requestor
                                          NULL,  // don't care for template
                                          NULL); // don't care for scope
    if (result == NULL)
    {
        dcUnidentifiedObjectExceptionClass_throwObject(variableName);
    }
    else
    {
        result = dcNodeEvaluator_evaluate
            (evaluator, dcScopeData_getObject(result));
    }

    dcMemory_free(variableName);
    return result;
}

static char *variablize(dcNode *_receiver, const char *_preVariableName)
{
    char *variableName = NULL;

    if (dcClass_isObject(_receiver))
    {
        variableName = dcLexer_sprintf("@%s", _preVariableName);
        //variableName[1] = tolower((unsigned char)variableName[1]);
    }
    else
    {
        variableName = dcLexer_sprintf("@@%s", _preVariableName);
        //variableName[2] = tolower((unsigned char)variableName[2]);
    }

    return variableName;
}

//
// dcObjectClass_setVariable
//
// the generic function called via a set function created via @w
//
dcNode *dcObjectClass_setVariable(dcNode *_receiver, dcArray *_arguments)
{
    char *variableName = variablize
        (_receiver, dcStringClass_getString(dcArray_get(_arguments, 0)));
    dcNode *result = dcScope_setObject
        (dcClass_getUsedScope(_receiver),
         dcNode_copyIfTemplate(dcArray_get(_arguments, 1)),
         variableName,
         NO_FLAGS);
    dcMemory_free(variableName);
    return dcScopeData_getObject(result);
}

//
// dcObjectClass_setValueForObject
//
// The C implementation of Object.setValue:forObject:
//
dcNode *dcObjectClass_setValueForObject(dcNode *_receiver, dcArray *_arguments)
{
    const char *name = dcStringClass_getString(dcArray_get(_arguments, 1));
    dcNode *scopeDataNode = dcClass_getScopeData(_receiver,
                                                 name,
                                                 SCOPE_DATA_OBJECT,
                                                 true,  // do search up
                                                 NULL,  // requestor
                                                 NULL,  // template
                                                 NULL); // scope
    dcNode *result = NULL;
    dcScopeDataFlags flags = NO_FLAGS;

    if (scopeDataNode != NULL)
    {
        flags = dcScopeData_getFlags(scopeDataNode);
    }

    if (scopeDataNode == NULL
        || (flags & SCOPE_DATA_WRITER) == 0
        || (flags & SCOPE_DATA_PUBLIC) == 0)
    {
        dcUnidentifiedObjectExceptionClass_throwObject(name);
    }
    else
    {
        result = dcNode_copyIfTemplate(dcArray_get(_arguments, 0));
        dcScope_setObject(dcClass_getUsedScope(_receiver), result, name, flags);
    }

    return result;
}

dcNode *dcObjectClass_init(dcNode *_receiver, dcArray *_arguments)
{
    return _receiver;
}

dcNode *dcObjectClass_initWithFields(dcNode *_receiver, dcArray *_arguments)
{
    return _receiver;
}

dcNode *dcObjectClass_hash(dcNode *_receiver, dcArray *_arguments)
{
    return dcNode_register
        (dcNumberClass_createObjectFromInt32u((uint32_t)(size_t)_receiver));
}

//
// dcObjectClass_hasMethod
//
// For taffy method: 'hasMethod?:
//
// Returns true if the receiver has _method as a method
// Returns false otherwise
//
dcNode *dcObjectClass_hasMethod(dcNode *_receiver, dcArray *_arguments)
{
    return (dcClass_getScopeData
            (_receiver,
             dcStringClass_getString(dcArray_get(_arguments, 0)),
             SCOPE_DATA_METHOD,
             true,      // search up
             _receiver, // requestor
             NULL,      // template
             NULL)      // scope
            == NULL
            ? dcNoClass_getInstance()
            : dcYesClass_getInstance());
}

dcNode *dcObjectClass_copy(dcNode *_receiver, dcArray *_arguments)
{
    return dcNode_register(dcNode_copy(_receiver, DC_DEEP));
}

dcNode *dcObjectClass_kindOf(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    dcNode *argument = dcArray_get(_arguments, 0);

    if (! dcClass_isObject(argument))
    {
        result = (dcClass_isKindOfTemplate
                  (_receiver,
                   dcClass_getTemplate(argument))
                  ? dcYesClass_getInstance()
                  : dcNoClass_getInstance());
    }
    else
    {
        dcNeedMetaClassExceptionClass_throwObject();
    }

    return result;
}

dcNode *dcObjectClass_castAs(dcNode *_receiver, dcArray *_arguments)
{
    return dcClass_castNode(_receiver,
                            dcClass_getTemplate(dcArray_get(_arguments, 0)),
                            true);
}

dcNode *dcObjectClass_super(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcClass_getOrInstantiateSuperNode(_receiver);

    if (result == NULL)
    {
        // we're Object, so our super is ourself //
        result = _receiver;
    }

    return result;
}

static void putObjectsIntoArray(dcNode *_receiver,
                                dcScopeDataFlags _type,
                                dcArray *_objectsArray)
{
    // grab the scope out of _receiver //
    dcClass *klass = dcClass_getClass(_receiver);

    const dcScope *metaScopes[] =
        {klass->classTemplate->metaScope, NULL};
    const dcScope *objectScopes[] =
        {klass->scope, klass->classTemplate->scope, NULL};

    const dcScope **scopes = (dcClass_isObject(_receiver)
                              ? objectScopes
                              : metaScopes);

    size_t i = 0;
    dcHash *names = dcHash_create();
    dcNode *yes = dcUnsignedInt32_createNode(1);

    for (i = 0; scopes[i] != NULL; i++)
    {
        // temp variables used below //
        dcNode *scopeDataNode = NULL;
        dcHashIterator *that = dcScope_createIterator(scopes[i]);

        while ((scopeDataNode = dcScope_getNext(that, _type))
               != NULL)
        {
            dcScopeData *scopeData = CAST_SCOPE_DATA(scopeDataNode);

            // only add the public objects that are of the same type
            // and that haven't been added before.
            // we might get a collision in our local scope and the
            // template scope.
            if ((scopeData->flags & SCOPE_DATA_PUBLIC) != 0
                && (scopeData->flags & _type) != 0
                && (dcHash_getValueWithStringKey(names, scopeData->name, NULL)
                    == TAFFY_FAILURE))
            {
                if ((scopeData->flags & SCOPE_DATA_METHOD) != 0)
                {
                    dcArray_add(_objectsArray,
                                dcStringClass_createObject
                                (scopeData->name, true));
                }
                else
                {
                    dcArray_add(_objectsArray,
                                dcPairClass_createObject
                                (dcStringClass_createObject
                                 (scopeData->name, true),
                                 scopeData->object,
                                 true));
                }

                dcHash_setValueWithStringKey(names, scopeData->name, yes);
            }
        }

        // free the methods iterator //
        dcHashIterator_free(&that);
    }

    dcHash_free(&names, DC_SHALLOW);
    dcNode_free(&yes, DC_DEEP);
}

static dcNode *getObjects(dcNode *_receiver,
                          dcScopeDataFlags _type,
                          bool _goUp)
{
    dcNode *that = _receiver;
    dcArray *objectsArray = dcArray_createWithSize(10);

    putObjectsIntoArray(that, _type, objectsArray);

    if (_goUp)
    {
        for (that = dcClass_getSuperNode(that);
             that != NULL;
             that = dcClass_getSuperNode(that))
        {
            putObjectsIntoArray(that, _type, objectsArray);
        }
    }

    // create the return Array, already initialized //
    return dcNode_register(dcArrayClass_createObject(objectsArray, true));
}

dcNode *dcObjectClass_methods(dcNode *_receiver, dcArray *_arguments)
{
    return getObjects(_receiver, SCOPE_DATA_METHOD, false);
}

dcNode *dcObjectClass_objects(dcNode *_receiver, dcArray *_arguments)
{
    return getObjects(_receiver, SCOPE_DATA_OBJECT, false);
}

dcNode *dcObjectClass_allMethods(dcNode *_receiver, dcArray *_arguments)
{
    return getObjects(_receiver, SCOPE_DATA_METHOD, true);
}

dcNode *dcObjectClass_allObjects(dcNode *_receiver, dcArray *_arguments)
{
    return getObjects(_receiver, SCOPE_DATA_OBJECT, true);
}

static dcNode *performHelper(dcNode *_receiver,
                             dcArray *_arguments,
                             bool _with)
{
    // extract the method name out of _arguments //
    const char *methodName =
        dcStringClass_getString(dcArray_get(_arguments, 0));
    dcClassTemplate *foundTemplate = NULL;

    // get the method node out of receiver's scope //
    dcNode *methodScopeData = dcClass_getScopeData(_receiver,
                                                   methodName,
                                                   SCOPE_DATA_METHOD,
                                                   true,
                                                   _receiver,
                                                   &foundTemplate,
                                                   NULL); // scope
    dcNode *result = NULL;
    dcList *arguments = NULL;

    if (_with)
    {
        // fixme
        arguments = dcList_createFromArray
            (dcArrayClass_getObjects(dcArray_get(_arguments, 1)));
    }
    else
    {
        arguments = dcList_create();
    }

    if (methodScopeData != NULL)
    {
        // got a good method //
        result = dcNodeEvaluator_evaluateProcedure
            (dcSystem_getCurrentNodeEvaluator(),
             _receiver,
             dcScopeData_getObject(methodScopeData),
             NO_FLAGS,
             arguments);
    }
    else
    {
        dcUnidentifiedMethodExceptionClass_throwObject
            (dcClass_getName(_receiver), methodName);
    }

    dcList_free(&arguments, DC_SHALLOW);
    return result;
}

dcNode *dcObjectClass_perform(dcNode *_receiver, dcArray *_arguments)
{
    return performHelper(_receiver, _arguments, false);
}

dcNode *dcObjectClass_performWith(dcNode *_receiver, dcArray *_arguments)
{
    return performHelper(_receiver, _arguments, true);
}

static void attach(dcNode *_parent,
                   dcNode *_scopeDataNode,
                   dcNodeEvaluator *_evaluator)
{
    dcScopeData *scopeData =
        dcScopeData_copy(CAST_SCOPE_DATA(_scopeDataNode), DC_DEEP);
    dcNode *object = scopeData->object;

    if (dcGraphData_isType(object, NODE_CLASS)
        && ! dcClass_isObject(object))
    {
        // it's a class! it has a new parent so we need
        // to set its package name accordingly
        dcClassTemplate *classTemplate = dcClass_getTemplate(object);
        dcClassTemplate *parentTemplate = dcClass_getTemplate(_parent);
        char *pathString =
            dcPackage_getPathString(parentTemplate->package);
        char *newPackageName =
            dcLexer_sprintf("%s%s%s",
                            pathString,
                            strlen(pathString) == 0 ? "" : ".",
                            parentTemplate->className);
        dcClassTemplate_setPackageName(classTemplate, newPackageName);
        dcMemory_free(newPackageName);
        dcMemory_free(pathString);

        dcClassTemplate *copy =
            dcClassTemplate_copy(classTemplate, DC_DEEP);
        dcNode *newMeta = NULL;

        if (dcClassManager_registerClassTemplate
            (copy,
             dcNodeEvaluator_getPackageContents(_evaluator),
             true,
             &newMeta)
            == TAFFY_SUCCESS)
        {
            // :)
            // do it again
            dcScope *allScopes[] =
                {CAST_CLASS(object)->classTemplate->scope,
                 CAST_CLASS(object)->classTemplate->metaScope,
                 NULL};
            dcScope **thisScope;

            for (thisScope = allScopes; *thisScope != NULL; thisScope++)
            {
                dcHash *objects = (*thisScope)->objects;

                if (objects != NULL)
                {
                    dcHashIterator *thisObject = dcHash_createIterator(objects);
                    dcNode *scopeDataNode = NULL;

                    while ((scopeDataNode =
                            dcHashIterator_getNextValue(thisObject))
                           != NULL)
                    {
                        attach(newMeta, scopeDataNode, _evaluator);
                    }

                    dcHashIterator_free(&thisObject);
                }
            }
        }
        else
        {
            // :(
            dcClassTemplate_free(&copy, DC_DEEP);
        }
    }

    dcNode_register(scopeData->object);
    dcScope_set(((scopeData->flags & SCOPE_DATA_INSTANCE)
                 ? (dcClass_isObject(_parent)
                    ? CAST_CLASS(_parent)->scope
                    : CAST_CLASS(_parent)->classTemplate->scope)
                 : CAST_CLASS(_parent)->classTemplate->metaScope),
                scopeData->object,
                scopeData->name,
                scopeData->flags);
    dcScopeData_free(&scopeData, DC_SHALLOW);
}

dcNode *dcObjectClass_attach(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *slice = dcArray_get(_arguments, 0);
    dcNode *result = _receiver;

    if (dcClass_isSlice(slice))
    {
        dcScope *allScopes[] =
            {CAST_CLASS(slice)->classTemplate->scope,
             CAST_CLASS(slice)->classTemplate->metaScope,
             NULL};
        dcScope *instanceScopes[] =
            {CAST_CLASS(slice)->classTemplate->scope,
             NULL};
        dcScope **thisScope = (dcClass_isObject(_receiver)
                               ? instanceScopes
                               : allScopes);
        dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

        while (thisScope != NULL && *thisScope != NULL)
        {
            dcHash *objects = (*thisScope)->objects;

            if (objects != NULL)
            {
                dcHashIterator *thisObject = dcHash_createIterator(objects);
                dcNode *scopeDataNode = NULL;

                while ((scopeDataNode = dcHashIterator_getNextValue(thisObject))
                       != NULL)
                {
                    attach(_receiver, scopeDataNode, evaluator);
                }

                dcHashIterator_free(&thisObject);
            }

            thisScope++;
        }
    }
    else
    {
        // failure
        result = NULL;
        dcNeedSliceExceptionClass_throwObject();
    }

    return result;
}

dcNode *dcObjectClass_prettyPrint(dcNode *_receiver, dcArray *_arguments)
{
    return dcNodeEvaluator_callMethod(dcSystem_getCurrentNodeEvaluator(),
                                      _receiver,
                                      "asString");
}
