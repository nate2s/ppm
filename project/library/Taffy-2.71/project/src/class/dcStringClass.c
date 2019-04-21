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
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#include "CompiledString.h"

#include "dcStringClass.h"
#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcExceptionClass.h"
#include "dcExceptions.h"
#include "dcGarbageCollector.h"
#include "dcGraphData.h"
#include "dcHash.h"
#include "dcKernelClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMethodHeader.h"
#include "dcNilClass.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcProcedureClass.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcUnsignedInt32.h"
#include "dcUnsignedInt64.h"
#include "dcYesClass.h"

static const char * const UNINITIALIZED_STRING = "!UNINITIALIZED STRING!";

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "#operator(+):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_append,
        gCFunctionArgument_wild
    },
    {
        "append:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE
         | SCOPE_DATA_CONST),
        &dcStringClass_append,
        gCFunctionArgument_wild
    },
    {
        "#operator(+=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcStringClass_appendBang,
        gCFunctionArgument_wild
    },
    {
        "append!:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcStringClass_appendBang,
        gCFunctionArgument_wild
    },
    {
        "characterAtIndex:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_objectAtIndex,
        gCFunctionArgument_number
    },
    {
        "clear",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcStringClass_clear,
        gCFunctionArgument_none
    },
    {
        "compare:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_compare,
        gCFunctionArgument_wild
    },
    {
        "downcase",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_downcase,
        gCFunctionArgument_none
    },
    {
        "downcase!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcStringClass_downcaseBang,
        gCFunctionArgument_none
    },
    {
        "eachCharacter:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_eachCharacter,
        gCFunctionArgument_block
    },
    {
        "eachWord:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONTAINER_LOOP
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_eachWord,
        gCFunctionArgument_block
    },
    {
        "#operator(==):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_equals,
        gCFunctionArgument_wild
    },
    {
        "eval",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_BREAKTHROUGH
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_eval,
        gCFunctionArgument_none
    },
    {
        "#operator(>):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_greaterThan,
        gCFunctionArgument_string
    },
    {
        "#operator(>=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_greaterThanOrEqual,
        gCFunctionArgument_string
    },
    {
        "hash",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_hash,
        gCFunctionArgument_none
    },
    {
        "indexOf:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_indexOf,
        gCFunctionArgument_string
    },
    {
        "insertString:atIndex:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcStringClass_insertStringAtIndex,
        gCFunctionArgument_stringNumber
    },
    {
        "isNumeric",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_CONST),
        &dcStringClass_isNumeric,
        gCFunctionArgument_none
    },
    {
        "lastIndexOf:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_lastIndexOf,
        gCFunctionArgument_string
    },
    {
        "length",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_length,
        gCFunctionArgument_none
    },
    {
        "#operator(<):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_lessThan,
        gCFunctionArgument_string
    },
    {
        "#operator(<=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_lessThanOrEqual,
        gCFunctionArgument_string
    },
    {
        "#operator([]):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_objectAtIndex,
        gCFunctionArgument_number
    },
    {
        "reverse",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_reverse,
        gCFunctionArgument_none
    },
    {
        "reverse!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcStringClass_reverseBang,
        gCFunctionArgument_none
    },
    {
        "#operator([]=):",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcStringClass_setStringAtIndex,
        gCFunctionArgument_array
    },
    {
        "split:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_split,
        gCFunctionArgument_string
    },
    {
        "substring:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_substring,
        gCFunctionArgument_number
    },
    {
        "substringFrom:to:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_substringFromTo,
        gCFunctionArgument_numberNumber
    },
    {
        "swapcase",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_swapcase,
        gCFunctionArgument_none
    },
    {
        "swapcase!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcStringClass_swapcaseBang,
        gCFunctionArgument_none
    },
    {
        "upcase",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_upcase,
        gCFunctionArgument_none
    },
    {
        "upcase!",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_MODIFIES_CONTAINER
         | SCOPE_DATA_SYNCHRONIZED_WRITE),
        &dcStringClass_upcaseBang,
        gCFunctionArgument_none
    },
    {
        "withWidth:",
        (SCOPE_DATA_PUBLIC
         | SCOPE_DATA_SYNCHRONIZED_READ
         | SCOPE_DATA_CONST),
        &dcStringClass_withWidth,
        gCFunctionArgument_number
    },
    {
        0
    }
};

#define CAST_STRING_AUX(_node_) ((dcStringClassAux*)(CAST_CLASS_AUX(_node_)))

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcStringClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (STRING_PACKAGE_NAME,                  // package name
          STRING_CLASS_NAME,                    // class name
          "Object",                             // super name
          CLASS_HAS_READ_WRITE_LOCK,            // class flags
          NO_FLAGS,                             // scope data flags
          NULL,                                 // meta methods
          sMethodWrappers,                      // methods
          &dcStringClass_initialize,            // initialization function
          NULL,                                 // deinitialization function
          &dcStringClass_allocateNode,          // allocate
          NULL,                                 // deallocate
          NULL,                                 // meta mark
          NULL,                                 // mark
          &dcStringClass_copyNode,              // copy
          &dcStringClass_freeNode,              // free
          NULL,                                 // register
          &dcStringClass_marshallNode,          // marshall
          &dcStringClass_unmarshallNode,        // unmarshall
          NULL));                               // set template
};

static dcStringClassAux *createAux(const char *_string, bool _copy)
{
    dcStringClassAux *aux =
        (dcStringClassAux *)dcMemory_allocate(sizeof(dcStringClassAux));
    aux->initialized = true;
    aux->types.string = (_copy
                         ? dcMemory_strdup(_string)
                         : (char *)_string);
    return aux;
}

static dcStringClassAux *createAuxFromList(dcList *_objects)
{
    dcStringClassAux *aux =
        (dcStringClassAux *)dcMemory_allocate(sizeof(dcStringClassAux));
    aux->initialized = false;
    aux->types.objects = (_objects == NULL
                          ? dcList_create()
                          : _objects);
    return aux;
}

void dcStringClass_allocateNode(dcNode *_node)
{
    CAST_CLASS_AUX(_node) = createAux("", true);
}

dcNode *dcStringClass_createNode(const char *_string,
                                 bool _object,
                                 bool _copy)
{
    return dcClass_createNode(sTemplate,
                              NULL, // super
                              NULL, // scope
                              _object,
                              createAux(_string, _copy));
}

// this method takes ownership of _objects //
dcNode *dcStringClass_createNodeFromList(dcList *_objects,
                                         bool _object,
                                         bool _initialized)
{
    dcNode *result =
        dcClass_createNode(sTemplate,
                           NULL, // super
                           NULL, // scope
                           _object,
                           createAuxFromList(_objects));
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    if (dcNodeEvaluator_hasException(evaluator)
        || (_initialized
            && ! (dcStringClass_initializeObject(result, _objects))))
    {
        dcNode_free(&result, DC_DEEP);
        result = NULL;
    }

    return result;
}

dcNode *dcStringClass_createObjectFromList(dcList *_objects,
                                           bool _initialized)
{
    return dcStringClass_createNodeFromList(_objects, true, _initialized);
}

dcNode *dcStringClass_createObject(const char *_string, bool _copy)
{
    return dcStringClass_createNode(_string, true, _copy);
}

#define STRING_CLASS_TAFFY_FILE_NAME "src/class/String.ty"

void dcStringClass_initialize(void)
{
    dcError_assert(dcStringEvaluator_evalString(__compiledString,
                                                STRING_CLASS_TAFFY_FILE_NAME,
                                                NO_STRING_EVALUATOR_FLAGS)
                   != NULL);
}

void dcStringClass_freeNode(dcNode *_node, dcDepth _depth)
{
    dcStringClassAux *aux = CAST_STRING_AUX(_node);

    if (aux != NULL)
    {
        if (aux->initialized && aux->types.string != NULL)
        {
            dcMemory_free(aux->types.string);
        }
        else if (! aux->initialized && aux->types.objects != NULL)
        {
            dcList_free(&aux->types.objects, DC_DEEP);
        }

        dcMemory_free(aux);
    }
}

void dcStringClass_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcStringClassAux *fromAux = CAST_STRING_AUX(_from);

    if (fromAux->initialized)
    {
        CAST_CLASS_AUX(_to) = createAux(fromAux->types.string, true);
    }
    else
    {
        dcList *objectsCopy = dcList_copy(fromAux->types.objects, DC_DEEP);
        CAST_CLASS_AUX(_to) = createAuxFromList(objectsCopy);
    }
}

dcList *dcStringClass_getObjects(const dcNode *_stringNode)
{
    return CAST_STRING_AUX(_stringNode)->types.objects;
}

const char *dcStringClass_getString(const dcNode *_stringNode)
{
    return (CAST_STRING_AUX(_stringNode)->initialized
            ? CAST_STRING_AUX(_stringNode)->types.string
            : UNINITIALIZED_STRING);
}

bool dcStringClass_isInitialized(const dcNode *_string)
{
    return CAST_STRING_AUX(_string)->initialized;
}

static void printUninitialized(const dcNode *_node, dcString *_string)
{
    dcStringClassAux *aux = CAST_STRING_AUX(_node);
    const dcList *objects = aux->types.objects;
    const dcListElement *objectsIt = dcList_getHeadElement(objects);
    dcString_appendString(_string, "\"");

    while (objectsIt != NULL)
    {
        if (objectsIt->object->type == NODE_STRING)
        {
            dcNode_print(objectsIt->object, _string);
        }
        else
        {
            dcString_appendString(_string, "#[");
            dcNode_print(objectsIt->object, _string);
            dcString_appendString(_string, "]");
        }

        objectsIt = objectsIt->next;
    }

    dcString_appendString(_string, "\"");
}

dcResult dcStringClass_printNode(const dcNode *_node, dcString *_string)
{
    dcResult result = TAFFY_SUCCESS;

    if (!CAST_STRING_AUX(_node)->initialized)
    {
        printUninitialized(_node, _string);
    }
    else
    {
        const char *asString = dcStringClass_asString_helper((dcNode*)_node);

        if (asString == NULL)
        {
            result = TAFFY_EXCEPTION;
        }
        else
        {
            dcString_appendString(_string, asString);
        }
    }

    return result;
}

bool dcStringClass_initializeObject(dcNode *_object, dcList *_evaluatedList)
{
    dcString *evaluatedString = dcString_create();
    dcStringClassAux *aux = CAST_STRING_AUX(_object);
    dcListElement *evaluatedListElement = dcList_getHeadElement(_evaluatedList);
    bool exception = false;
    bool result = true;
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    while (evaluatedListElement != NULL)
    {
        dcNode *arrayItNode = evaluatedListElement->object;

        // can be a dcString or dcClass* //
        if (arrayItNode->type == NODE_STRING)
        {
            char *asString = dcString_getString(arrayItNode);
            dcString_appendString(evaluatedString, asString);
        }
        else
        {
            dcNode *evaluated =
                dcNodeEvaluator_evaluate(evaluator, arrayItNode);

            if (evaluated == NULL)
            {
                exception = true;
                break;
            }

            char *asString =
                dcStringClass_asString_noQuotes_helper(evaluated);

            if (asString != NULL)
            {
                // put quotes here for string-within-a-string
                dcString_appendString(evaluatedString, asString);
            }
            else
            {
                exception = true;
                break;
            }

            dcMemory_free(asString);
        }

        evaluatedListElement = evaluatedListElement->next;
    }

    if (exception)
    {
        dcString_free(&evaluatedString, DC_DEEP);
        result = false;
    }
    else
    {
        // free the old list //
        dcList_free(&aux->types.objects, DC_DEEP);

        // set the string //
        aux->types.string = evaluatedString->string;

        // free shallowly //
        dcString_free(&evaluatedString, DC_SHALLOW);

        // set the new type //
        aux->initialized = true;
    }

    return result;
}

// We must be inside of an evaluation before this function is called
// If you want to display a node outside of an evaluation loop, call
// dcNode_synchronizedDisplay()
const char *dcStringClass_asString_helper(dcNode *_node)
{
    dcNode *node = _node;
    const char *result = NULL;
    dcError_assert(node != NULL);
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();
    dcError_assert(evaluator->evaluationCount > 0);

    if (! IS_CLASS(node))
    {
        node = dcNodeEvaluator_evaluate(evaluator, node);
    }

    if (node != NULL)
    {
        bool kindaString = dcClass_isKindOfTemplate(node, sTemplate);
        dcNode *display = (dcClass_isObject(node)
                           ? dcClass_castNode(node, sTemplate, false)
                           : NULL);

        if (! dcClass_isObject(node) || display == NULL)
        {
            display = dcNodeEvaluator_setPrinting(evaluator, node, true);

            if (display == NULL)
            {
                display = dcNodeEvaluator_callMethod
                    (evaluator, node, "asString");
            }

            dcNodeEvaluator_setPrinting(evaluator, node, false);
        }

        if (display != NULL)
        {
            if (dcClass_hasTemplate(display, sTemplate, true))
            {
                if (kindaString)
                {
                    if (! CAST_STRING_AUX(display)->initialized)
                    {
                        dcString output;
                        dcString_initialize(&output, 15);
                        printUninitialized(display, &output);
                        result = output.string;
                    }
                    else
                    {
                        result = dcLexer_sprintf
                            ("\"%s\"", CAST_STRING_AUX(display)->types.string);
                    }

                    // give it to the garbage collector
                    dcNode_register(dcStringClass_createObject(result, false));
                }
                else
                {
                    result = CAST_STRING_AUX(display)->types.string;
                }
            }
            else
            {
                dcInvalidCastExceptionClass_throwObject
                    (dcClass_getName(display), "String");
            }
        }
    }

    return result;
}

//
// note: a hack, better solution: call asString and return contents
//
char *dcStringClass_asString_noQuotes_helper(dcNode *_object)
{
    const char *display = dcStringClass_asString_helper(_object);
    char *result = NULL;

    if (display != NULL)
    {
        const size_t displayLength = strlen(display);
        char *displayNoQuotes = NULL;

        if (displayLength >= 2
            && display[0] == '"'
            && display[displayLength - 1] == '"')
        {
            displayNoQuotes = (char *)(dcMemory_allocateAndInitialize
                                       (displayLength - 1));
            memcpy(displayNoQuotes, display + 1, displayLength - 2);
        }
        else
        {
            displayNoQuotes = dcMemory_strdup(display);
        }

        result = displayNoQuotes;
    }

    return result;
}

dcNode *dcStringClass_objectAtIndex(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *indexNode = dcArray_get(_arguments, 0);
    dcNode *result = NULL;
    uint64_t stringIndex = 0;
    const char *string = dcStringClass_getString(_receiver);
    const size_t stringLength = strlen(string);

    if (dcNumberClass_extract64BitIndex(indexNode, &stringIndex, stringLength))
    {
        char resultString[2] = {0};
        resultString[0] = string[stringIndex];
        result = dcNode_register
            (dcStringClass_createObject(resultString, true));
    }

    return result;
}

dcNode *dcStringClass_withWidth(dcNode *_receiver, dcArray *_arguments)
{
    uint32_t width = 0;
    dcNode *result = NULL;

    if (dcNumberClass_extractInt32u_withException(dcArray_get(_arguments, 0),
                                                  &width))
    {
        result = (dcNode_register
                  (dcStringClass_createObject
                   (dcLexer_sprintf("%-*s",
                                    width,
                                    dcStringClass_getString(_receiver)),
                    false)));
    }

    return result;
}

static void setString(dcNode *_stringNode, const char *_string)
{
    dcStringClassAux *aux = CAST_STRING_AUX(_stringNode);

    if (aux->types.string != NULL)
    {
        dcMemory_free(aux->types.string);
    }

    CAST_STRING_AUX(_stringNode)->types.string = dcMemory_strdup(_string);
}

static dcNode *setStringAtIndex(dcNode *_receiver,
                                dcNode *_preIndex,
                                dcNode *_preObject)
{
    dcNode *result = NULL;
    dcNode *indexNode = dcClass_castNode(_preIndex,
                                         dcNumberClass_getTemplate(),
                                         true);
    if (indexNode != NULL)
    {
        char *insertString = dcStringClass_asString_noQuotes_helper
            (_preObject);

        if (insertString != NULL)
        {
            const char *receiverString = dcStringClass_getString(_receiver);
            const size_t receiverStringLength = strlen(receiverString);
            const size_t insertStringLength = strlen(insertString);
            uint64_t insertIndex = 0;

            if (dcNumberClass_extract64BitIndex(indexNode,
                                                &insertIndex,
                                                receiverStringLength))
            {
                // allocate newString //
                char *newString = (char *)(dcMemory_allocateAndInitialize
                                           (receiverStringLength
                                            + insertStringLength + 1));

                // fill the insertString //
                strncpy(&(newString[insertIndex]),
                        insertString,
                        insertStringLength);

                // fill the left of string //
                strncpy(newString, receiverString, insertIndex);

                // fill the right of string //
                strncpy(&(newString[insertIndex + insertStringLength]),
                        &receiverString[insertIndex],
                        receiverStringLength - insertIndex);

                // newString is already null-terminated //

                setString(_receiver, newString);
                dcMemory_free(newString);
                result = _receiver;
            }
        }

        dcMemory_free(insertString);
    }

    return result;
}

//
// #operator([]=)
//
dcNode *dcStringClass_setStringAtIndex(dcNode *_receiver, dcArray *_arguments)
{
    dcArray *arguments = dcArrayClass_getObjects(dcArray_get(_arguments, 0));
    return setStringAtIndex(_receiver,
                            dcArray_get(arguments, 0),
                            dcArray_get(arguments, 1));
}

//
// insertString:atIndex:
//
dcNode *dcStringClass_insertStringAtIndex(dcNode *_receiver,
                                          dcArray *_arguments)
{
    return setStringAtIndex(_receiver,
                            dcArray_get(_arguments, 1),
                            dcArray_get(_arguments, 0));
}

dcNode *dcStringClass_isNumeric(dcNode *_receiver, dcArray *_arguments)
{
    size_t i;
    const char *string = dcStringClass_getString(_receiver);
    size_t length = strlen(string);

    for (i = 0; i < length; i++)
    {
        if (! isdigit(string[i]))
        {
            return dcNoClass_getInstance();
        }
    }

    return dcYesClass_getInstance();
}

static char *appendHelper(dcNode *_receiver, dcArray *_arguments)
{
    char *appendValue =
        dcStringClass_asString_noQuotes_helper(dcArray_get(_arguments, 0));
    char *result = appendValue;

    if (result != NULL)
    {
        result = (dcLexer_sprintf("%s%s",
                                  dcStringClass_getString(_receiver),
                                  appendValue));
    }

    dcMemory_free(appendValue);
    return result;
}

dcNode *dcStringClass_append(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    char *newString = appendHelper(_receiver, _arguments);

    if (newString != NULL)
    {
        result = dcStringClass_createObject(newString, false);
        dcNode_register(result);
    }

    return result;
}

dcNode *dcStringClass_appendBang(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = NULL;
    char *newString = appendHelper(_receiver, _arguments);

    if (newString != NULL)
    {
        dcMemory_free(CAST_STRING_AUX(_receiver)->types.string);
        CAST_STRING_AUX(_receiver)->types.string = newString;
        result = _receiver;
    }

    return result;
}

static dcNode *realSubstring(dcNode *_receiver,
                             const char *_string,
                             uint64_t _stringLength,
                             uint64_t _index1,
                             uint64_t _index2)
{
    dcNode *result = NULL;

    if (_index1 == _index2)
    {
        result = dcNilClass_getInstance();
    }
    else
    {
        char *newString = NULL;

        if (_index1 < _index2)
        {
            newString =
                (char *)dcMemory_allocateAndInitialize((_index2 - _index1) + 1);
            memcpy(newString, &(_string[_index1]), _index2 - _index1);
        }
        else if (_index1 > _index2)
        {
            newString = (char *)(dcMemory_allocateAndInitialize
                                 ((_stringLength - _index1) + _index2 + 2));

            uint64_t stringIt = 0;
            uint64_t newStringIt = 0;

            // stuff the beginning
            for (stringIt = _index1, newStringIt = 0;
                 stringIt < _stringLength;
                 stringIt++, newStringIt++)
            {
                newString[newStringIt] = _string[stringIt];
            }

            // stuff the end
            for (stringIt = 0;
                 stringIt < _index2 + 1;
                 stringIt++, newStringIt++)
            {
                newString[newStringIt] = _string[stringIt];
            }

            newString[newStringIt] = 0;
        }

        result = dcNode_register
            (dcStringClass_createObject(newString, false));
    }

    return result;
}

dcNode *dcStringClass_substring(dcNode *_receiver, dcArray *_arguments)
{
    const char *string = dcStringClass_getString(_receiver);
    const size_t stringLength = strlen(string);
    dcNode *result = NULL;
    dcNode *indexNode = dcArray_get(_arguments, 0);
    uint64_t indexValue = 0;

    // the substring ranges from 0 to index //

    if (dcNumberClass_extract64BitIndex(indexNode, &indexValue, stringLength))
    {
        result = realSubstring(_receiver, string, stringLength, 0, indexValue);
    }

    return result;
}

dcNode *dcStringClass_substringFromTo(dcNode *_receiver, dcArray *_arguments)
{
    const char *string = dcStringClass_getString(_receiver);
    const size_t stringLength = strlen(string);
    dcNode *result = NULL;
    dcNode *index1Node = dcArray_get(_arguments, 0);
    dcNode *index2Node = dcArray_get(_arguments, 1);
    uint64_t index1 = 0;
    uint64_t index2 = 0;

    // the substring ranges from index1 to index2 //

    if (dcNumberClass_extract64BitIndex(index1Node, &index1, stringLength)
        && dcNumberClass_extract64BitIndex(index2Node, &index2, stringLength))
    {
        result = realSubstring(_receiver, string, stringLength, index1, index2);
    }

    return result;
}

dcNode *dcStringClass_length(dcNode *_receiver, dcArray *_arguments)
{
    return (dcNode_register
            (dcNumberClass_createObjectFromInt64u
             (strlen(dcStringClass_getString(_receiver)))));
}

dcNode *dcStringClass_equals(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcNoClass_getInstance();
    dcNode *candidate = dcClass_castNode(dcArray_get(_arguments, 0),
                                         sTemplate,
                                         false);
    if (candidate != NULL)
    {
        result = ((dcMemory_taffyStringCompare
                   (dcStringClass_getString(_receiver),
                    dcStringClass_getString(candidate))
                   == TAFFY_EQUALS)
                  ? dcYesClass_getInstance()
                  : dcNoClass_getInstance());
    }

    return result;
}

dcNode *dcStringClass_hash(dcNode *_receiver, dcArray *_arguments)
{
    dcHashType hash;
    assert(dcString_hashCharArray(dcStringClass_getString(_receiver), &hash)
           == TAFFY_SUCCESS);
    return dcNode_register(dcNumberClass_createObjectFromInt64u(hash));
}

static dcList *findSpaces(const char *_string)
{
    dcList *result = dcList_create();
    const size_t stringLength = strlen(_string);
    size_t stringIt = 0;

    for (stringIt = 0; stringIt < stringLength; stringIt++)
    {
        if (_string[stringIt] == ' ')
        {
            dcList_push(result, dcUnsignedInt64_createNode(stringIt));
        }
    }

    dcList_push(result, dcUnsignedInt64_createNode(stringLength));
    return result;
}

dcNode *dcStringClass_eachWord(dcNode *_receiver, dcArray *_arguments)
{
    const char *string = CAST_STRING_AUX(_receiver)->types.string;
    dcList *spaces = findSpaces(string);
    dcList *callArguments = dcList_create();
    dcNode *blockNode = dcArray_get(_arguments, 0);
    dcNode *procedureNode =
        dcClass_castNodeWithAssert(blockNode,
                               dcProcedureClass_getTemplate(),
                               false,
                               true);
    dcMethodHeader *methodHeader =
        dcProcedureClass_getMethodHeader(procedureNode);

    dcList *blockArguments = dcMethodHeader_getArguments(methodHeader);
    dcNode *result =  dcNilClass_getInstance();
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    if (blockArguments->size == 1)
    {
        dcListElement *spacesElement = NULL;
        uint64_t index1 = 0;
        uint64_t index2 = 0;
        dcNodeEvaluator_startLoop(evaluator);

        for (spacesElement = spaces->head;
             spacesElement != NULL;
             spacesElement = spacesElement->next)
        {
            // set the start //
            index1 = index2;

            // set the end //
            index2 = dcUnsignedInt64_getInt(spacesElement->object);

            // initialize the word, and account for the space //
            char *word =
                (char *)dcMemory_allocateAndInitialize(index2 - index1 + 1);
            memcpy(word, &(string[index1]), index2 - index1);
            dcNode *wordNode =
                dcNode_register(dcStringClass_createObject(word, false));
            dcError_assert(dcNodeEvaluator_pushMark(evaluator, wordNode) == 1);

            dcList_setHead(callArguments, wordNode);
            bool failure = false;

            if (dcNodeEvaluator_evaluateProcedure(evaluator,
                                                  NULL,
                                                  procedureNode,
                                                  (SCOPE_DATA_BREAKTHROUGH
                                                   | SCOPE_DATA_CONST),
                                                  callArguments)
                == NULL)
            {
                result = NULL;
                failure = true;
            }
            else if (! dcNodeEvaluator_canContinueEvaluating(evaluator))
            {
                failure = true;
            }

            dcNodeEvaluator_popMark(evaluator);

            if (failure)
            {
                break;
            }

            // skip over the space //
            index2++;
        }

        dcNodeEvaluator_stopLoop(evaluator);
    }

    dcList_free(&spaces, DC_DEEP);
    dcList_free(&callArguments, DC_SHALLOW);
    return result;
}

dcNode *dcStringClass_eachCharacter(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *block = dcArray_get(_arguments, 0);
    dcNode *procedureNode = dcClass_castNode(block,
                                             dcProcedureClass_getTemplate(),
                                             false);
    dcError_assert(procedureNode != NULL);
    dcList *blockArguments =
        dcMethodHeader_getArguments
        (dcProcedureClass_getMethodHeader(procedureNode));
    dcNode *result = dcNilClass_getInstance();
    dcNodeEvaluator *evaluator = dcSystem_getCurrentNodeEvaluator();

    if (blockArguments->size == 1)
    {
        char copyString[2] = {0};
        const char *string = dcStringClass_getString(_receiver);
        size_t stringLength = strlen(string);
        size_t i;
        dcList *callArguments = dcList_create();

        dcNodeEvaluator_startLoop(evaluator);

        for (i = 0; i < stringLength; i++)
        {
            copyString[0] = string[i];

            // create the argument and register it and mark it
            dcNode *stringNode = dcStringClass_createObject(copyString, true);
            dcNode_register(stringNode);
            dcError_assert(dcNodeEvaluator_pushMark(evaluator, stringNode)
                           == 1);
            dcList_setHead(callArguments, stringNode);

            if (! dcNodeEvaluator_evaluateProcedure(evaluator,
                                                    NULL,
                                                    procedureNode,
                                                    (SCOPE_DATA_BREAKTHROUGH
                                                     | SCOPE_DATA_CONST),
                                                    callArguments))
            {
                result = NULL;
                break;
            }
            else if (!dcNodeEvaluator_canContinueEvaluating(evaluator))
            {
                break;
            }

            dcNodeEvaluator_popMark(evaluator);
        }

        dcList_free(&callArguments, DC_SHALLOW);
        dcNodeEvaluator_stopLoop(evaluator);
    }
    else
    {
        dcInvalidNumberArgumentsExceptionClass_throwObject
            (1, blockArguments->size);
        result = NULL;
    }

    return result;
}

dcNode *dcStringClass_compare(dcNode *_receiver, dcArray *_arguments)
{
    dcNode *result = dcKernelClass_getUncomparableSymbol();
    dcNode *rightObject = dcArray_get(_arguments, 0);

    if (dcClass_hasTemplate(rightObject, sTemplate, true))
    {
        int comparisonResult = strcmp(dcStringClass_getString(_receiver),
                                      dcStringClass_getString(rightObject));

        if (comparisonResult < 0)
        {
            result = dcKernelClass_getLessThanSymbol();
        }
        else if (comparisonResult == 0)
        {
            result = dcKernelClass_getEqualSymbol();
        }
        else
        {
            result = dcKernelClass_getGreaterThanSymbol();
        }
    }

    return result;
}

dcNode *dcStringClass_clear(dcNode *_receiver, dcArray *_arguments)
{
    char *string = CAST_STRING_AUX(_receiver)->types.string;
    memset(string, 0, strlen(string));
    return _receiver;
}

dcNode *dcStringClass_downcase(dcNode *_receiver, dcArray *_arguments)
{
    char *string = dcMemory_strdup(CAST_STRING_AUX(_receiver)->types.string);
    size_t stringLength = strlen(string);
    size_t i = 0;

    for (i = 0; i < stringLength; i++)
    {
        string[i] = tolower((unsigned char)string[i]);
    }

    return dcNode_register(dcStringClass_createObject(string, false));
}

dcNode *dcStringClass_downcaseBang(dcNode *_receiver, dcArray *_arguments)
{
    char *string = CAST_STRING_AUX(_receiver)->types.string;
    const size_t stringLength = strlen(string);
    size_t i = 0;

    for (i = 0; i < stringLength; i++)
    {
        string[i] = tolower((unsigned char)string[i]);
    }

    return _receiver;
}

dcNode *dcStringClass_upcase(dcNode *_receiver, dcArray *_arguments)
{
    char *string = dcMemory_strdup(dcStringClass_getString(_receiver));
    const size_t stringLength = strlen(string);
    size_t i = 0;

    for (i = 0; i < stringLength; i++)
    {
        string[i] = toupper((unsigned char)string[i]);
    }

    return dcNode_register(dcStringClass_createObject(string, false));
}

dcNode *dcStringClass_upcaseBang(dcNode *_receiver, dcArray *_arguments)
{
    // access directly, no const //
    char *string = CAST_STRING_AUX(_receiver)->types.string;
    const size_t stringLength = strlen(string);
    size_t i = 0;

    for (i = 0; i < stringLength; i++)
    {
        string[i] = toupper((unsigned char)string[i]);
    }

    return _receiver;
}

dcNode *dcStringClass_swapcase(dcNode *_receiver, dcArray *_arguments)
{
    char *newString = dcMemory_strdup(dcStringClass_getString(_receiver));
    const size_t stringLength = strlen(newString);
    size_t i = 0;

    for (i = 0; i < stringLength; i++)
    {
        if (isupper((unsigned char)newString[i]))
        {
            newString[i] = tolower((unsigned char)newString[i]);
        }
        else
        {
            newString[i] = toupper((unsigned char)newString[i]);
        }
    }

    return dcNode_register(dcStringClass_createObject(newString, false));
}

dcNode *dcStringClass_swapcaseBang(dcNode *_receiver, dcArray *_arguments)
{
    // access it directly, no const //
    char *string = CAST_STRING_AUX(_receiver)->types.string;
    const size_t stringLength = strlen(string);
    size_t i = 0;

    for (i = 0; i < stringLength; i++)
    {
        if (isupper((unsigned char)string[i]))
        {
            string[i] = tolower((unsigned char)string[i]);
        }
        else
        {
            string[i] = toupper((unsigned char)string[i]);
        }
    }

    return _receiver;
}

dcNode *dcStringClass_indexOf(dcNode *_receiver, dcArray *_arguments)
{
    bool gotMatch = false;
    dcNode *indexStringNode = dcArray_get(_arguments, 0);
    const char *indexString = dcStringClass_getString(indexStringNode);
    const char *string = dcStringClass_getString(_receiver);
    const size_t stringLength = strlen(string);
    const size_t indexStringLength = strlen(indexString);
    uint64_t foundIndex = 0;
    size_t i = 0;
    size_t indexStringIt = 0;
    size_t matchLength = 0;

    for (i = 0; i < stringLength; i++)
    {
        if (string[i] == indexString[indexStringIt])
        {
            if (! gotMatch)
            {
                foundIndex = i;
                gotMatch = true;
            }

            matchLength++;
            indexStringIt++;

            if (matchLength == indexStringLength)
            {
                break;
            }
        }
        else
        {
            gotMatch = false;
            matchLength = 0;
            foundIndex = -1;
            indexStringIt = 0;
        }
    }

    if (gotMatch && matchLength != indexStringLength)
    {
        foundIndex = -1;
    }

    dcNode *result = (gotMatch
                      ? dcNumberClass_createObjectFromInt64u(foundIndex)
                      : dcNumberClass_createObjectFromInt32s(-1));
    return dcNode_register(result);
}

dcNode *dcStringClass_lastIndexOf(dcNode *_receiver, dcArray *_arguments)
{
    bool gotMatch = false;
    dcNode *indexStringNode = dcArray_get(_arguments, 0);
    const char *indexString = dcStringClass_getString(indexStringNode);
    const char *string = dcStringClass_getString(_receiver);
    const size_t stringLength = strlen(string);
    const size_t indexStringLength = strlen(indexString);
    uint64_t foundIndex = 0;
    int64_t i = 0;
    int64_t indexStringIt = indexStringLength - 1;
    int64_t matchLength = 0;

    for (i = stringLength - 1; i >= 0; i--)
    {
        if (string[i] == indexString[indexStringIt])
        {
            if (! gotMatch)
            {
                gotMatch = true;
            }

            matchLength++;

            if (indexStringIt == 0)
            {
                foundIndex = i;
                break;
            }
            else
            {
                indexStringIt--;
            }
        }
        else
        {
            gotMatch = false;
            matchLength = 0;
            indexStringIt = 0;
        }
    }

    dcNode *result = (gotMatch
                      ? dcNumberClass_createObjectFromInt64u(foundIndex)
                      : dcNumberClass_createObjectFromInt32s(-1));
    return dcNode_register(result);
}

TAFFY_HIDDEN dcNode *compare(dcNode *_receiver,
                             dcArray *_arguments,
                             int _wantedResult,
                             bool _reverse)
{
    bool gotIt = (strcmp
                  (dcStringClass_getString(_receiver),
                   dcStringClass_getString(dcArray_get(_arguments, 0)))
                  == _wantedResult);

    if (gotIt)
    {
        return (_reverse
                ? dcNoClass_getInstance()
                : dcYesClass_getInstance());
    }
    else
    {
        return (_reverse
                ? dcYesClass_getInstance()
                : dcNoClass_getInstance());
    }
}

dcNode *dcStringClass_lessThan(dcNode *_receiver, dcArray *_arguments)
{
    return compare(_receiver, _arguments, -1, false);
}

dcNode *dcStringClass_greaterThan(dcNode *_receiver, dcArray *_arguments)
{
    return compare(_receiver, _arguments, 1, false);
}

dcNode *dcStringClass_lessThanOrEqual(dcNode *_receiver, dcArray *_arguments)
{
    return compare(_receiver, _arguments, 1, true);
}

dcNode *dcStringClass_greaterThanOrEqual(dcNode *_receiver, dcArray *_arguments)
{
    return compare(_receiver, _arguments, -1, true);
}

dcNode *dcStringClass_reverse(dcNode *_receiver, dcArray *_arguments)
{
    const char *string = dcStringClass_getString(_receiver);
    size_t stringLength = strlen(string);
    char *reverseString = (char *)(dcMemory_allocateAndInitialize
                                   (stringLength + 1));
    size_t i = 0;

    for (i = 0; i < stringLength; i++)
    {
        reverseString[stringLength - i - 1] = string[i];
    }

    return dcNode_register(dcStringClass_createObject(reverseString, false));
}

dcNode *dcStringClass_reverseBang(dcNode *_receiver, dcArray *_arguments)
{
    char *string = CAST_STRING_AUX(_receiver)->types.string;
    char *reverseString = dcMemory_strdup(string);
    const size_t stringLength = strlen(string);
    size_t i = 0;

    for (i = 0; i < stringLength; i++)
    {
        string[i] = reverseString[stringLength - i - 1];
    }

    dcMemory_free(reverseString);
    return _receiver;
}

static dcArray *splitHelper(const char *_splitString,
                            const char *_splitterString)
{
    dcArray *result = NULL;
    size_t length = strlen(_splitterString);

    if (length == 1)
    {
        dcList *splits = dcLexer_splitString(_splitString, _splitterString[0]);
        dcArray *splitsArray = dcArray_createWithSize(splits->size);
        dcListIterator *splitsIterator = dcList_createHeadIterator(splits);
        dcNode *splitNode = NULL;

        while ((splitNode = dcListIterator_getNext(splitsIterator)))
        {
            dcNode *addNode = dcStringClass_createObject
                (dcString_getString(splitNode), true);
            dcArray_add(splitsArray, addNode);
        }

        result = splitsArray;
        dcList_free(&splits, DC_DEEP);
        dcListIterator_free(&splitsIterator);
    }
    else
    {
        dcInvalidNumberArgumentsExceptionClass_createObject(1, length);
        result = NULL;
    }

    return result;
}

dcNode *dcStringClass_split(dcNode *_receiver, dcArray *_arguments)
{
    const char *splitString = dcStringClass_getString(_receiver);
    dcNode *splitterNode = dcArray_get(_arguments, 0);
    const char *splitterString = dcStringClass_getString(splitterNode);
    dcNode *result = NULL;
    dcArray *splitsArray = splitHelper(splitString, splitterString);

    if (splitsArray != NULL)
    {
        result = dcArrayClass_createObject(splitsArray, true);
        dcNode_register(result);
    }

    return result;
}

dcNode *dcStringClass_eval(dcNode *_receiver, dcArray *_arguments)
{
    char *toEval = dcStringClass_asString_noQuotes_helper(_receiver);
    // propagate exceptions up
    dcNode *result = (dcStringEvaluator_evalString
                      (toEval,
                       dcNodeEvaluator_getCurrentFileName
                       (dcSystem_getCurrentNodeEvaluator()),
                       NO_STRING_EVALUATOR_FLAGS));
    dcMemory_free(toEval);
    return result;
}

bool dcStringClass_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;

    if (dcString_getLengthLeft(_stream) >= STRING_MARSHALL_LENGTH)
    {
        uint8_t type = dcString_getCharacter(_stream);

        if (type == STRING_CLASS_INITIALIZED)
        {
            char *string = dcString_unmarshallCharArray(_stream);

            if (string != NULL)
            {
                result = true;
                CAST_CLASS_AUX(_node) = createAux(string, false);
            } // else FAILURE //
        }
        else if (type == STRING_CLASS_UNINITIALIZED)
        {
            dcList *objects = dcList_unmarshall(_stream);

            if (objects != NULL)
            {
                result = true;
                CAST_CLASS_AUX(_node) = createAuxFromList(objects);
            } // else FAILURE //
        } // else FAILURE //
    } // else FAILURE //
    return result;
}

dcString *dcStringClass_marshallNode(const dcNode *_node, dcString *_stream)
{
    if (dcStringClass_isInitialized(_node))
    {
        dcString_appendCharacter(_stream, STRING_CLASS_INITIALIZED);
        dcString_marshallCharArray(dcStringClass_getString(_node), _stream);
    }
    else
    {
        dcString_appendCharacter(_stream, STRING_CLASS_UNINITIALIZED);
        dcList_marshall(dcStringClass_getObjects(_node), _stream);
    }

    return _stream;
}

dcTaffy_createIsMeFunctionMacro(dcStringClass);
