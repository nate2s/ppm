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

#include "dcSymbolClass.h"
#include "dcArray.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcObjectClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

static const dcTaffyCMethodWrapper sMetaMethodWrappers[] =
{
    {
        "new:",
        SCOPE_DATA_PUBLIC,
        &dcSymbolMetaClass_new,
        gCFunctionArgument_string
    },
    {
        0
    }
};

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "asString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcSymbolClass_asString,
        gCFunctionArgument_none
    },
    {
        "getString",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcSymbolClass_getString,
        gCFunctionArgument_none
    },
    {
        0
    }
};

static dcClassTemplate *sTemplate = NULL;

#define CAST_SYMBOL_AUX(_node_) ((dcSymbolClassAux*)(CAST_CLASS_AUX(_node_)))

dcClassTemplate *dcSymbolClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (SYMBOL_PACKAGE_NAME,                  // package name
          SYMBOL_CLASS_NAME,                    // class name
          OBJECT_CLASS_NAME,                    // super name
          CLASS_ABSTRACT,                       // class flags
          NO_FLAGS,                             // scope data flags
          sMetaMethodWrappers,                  // meta methods
          sMethodWrappers,                      // methods
          NULL,                                 // initialization function
          NULL,                                 // deinitialization function
          &dcSymbolClass_allocateNode,          // allocate
          NULL,                                 // deallocate
          NULL,                                 // meta mark
          NULL,                                 // mark
          &dcSymbolClass_copyNode,              // copy
          &dcSymbolClass_freeNode,              // free
          NULL,                                 // register
          &dcSymbolClass_marshallNode,          // marshall
          &dcSymbolClass_unmarshallNode,        // unmarshall
          NULL));                               // set template
}

static dcSymbolClassAux *createAux(const char *_string)
{
    dcSymbolClassAux *aux = (dcSymbolClassAux *)(dcMemory_allocate
                                                 (sizeof(dcSymbolClassAux)));
    aux->string = dcMemory_strdup(_string);
    return aux;
}

void dcSymbolClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux("");
}

dcNode *dcSymbolClass_createNode(const char *_string, bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL, // super
                              NULL, // scope
                              _object,
                              createAux(_string));
}

dcNode *dcSymbolClass_createObject(const char *_string)
{
    return dcSymbolClass_createNode(_string, true);
}

void dcSymbolClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcSymbolClassAux *aux = (dcSymbolClassAux *)CAST_CLASS_AUX(_node);
    dcMemory_free(aux->string);
    dcMemory_free(CAST_CLASS_AUX(_node));
}

void dcSymbolClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcSymbolClassAux *fromAux = CAST_SYMBOL_AUX(_from);
    CAST_CLASS_AUX(_to) = createAux(fromAux->string);
}

bool dcSymbolClass_compareToString(const dcNode *_symbol, const char *_string)
{
    return (strcmp(dcSymbolClass_getString_helper(_symbol), _string) == 0);
}

dcNode *dcSymbolMetaClass_new(dcNode *_receiver, dcArray *_arguments)
{
    return (dcKernelClass_getOrCreateSymbol
            (dcStringClass_getString(dcArray_get(_arguments, 0))));
}

dcNode *dcSymbolClass_asString(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcStringClass_createObject
             (dcLexer_sprintf("'%s", CAST_SYMBOL_AUX(_receiver)->string),
              false)));
}

const char *dcSymbolClass_getString_helper(const dcNode *_symbol)
{
    return CAST_SYMBOL_AUX(_symbol)->string;
}

dcNode *dcSymbolClass_getString(dcNode *_node, dcArray *_arguments)
{
    return (dcNode_register
            (dcStringClass_createObject
             (dcSymbolClass_getString_helper(_node), true)));
}

bool dcSymbolClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    char *string = dcString_unmarshallCharArray(_stream);

    if (string != NULL)
    {
        result = true;
        CAST_CLASS_AUX(_node) = createAux(string);
        dcMemory_free(string);
    }

    return result;
}

dcString *dcSymbolClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcString_marshallCharArray(dcSymbolClass_getString_helper(_node), _stream);
    return _stream;
}

bool dcSymbolClass_isMe(const dcNode *_node)
{
    return dcClass_hasTemplate(_node, sTemplate, true);
}
