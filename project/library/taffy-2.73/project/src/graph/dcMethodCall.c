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

#include "dcArray.h"
#include "dcArrayClass.h"
#include "dcCharacterGraph.h"
#include "dcError.h"
#include "dcGraphData.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcMethodCall.h"
#include "dcNode.h"
#include "dcNumberClass.h"
#include "dcPair.h"
#include "dcString.h"

dcMethodCall *dcMethodCall_create(dcNode *_receiver,
                                  const char *_methodName,
                                  dcList *_arguments)
{
    dcMethodCall *call =
        (dcMethodCall *)dcMemory_allocate(sizeof(dcMethodCall));
    assert(_methodName != NULL);
    call->receiver = _receiver;
    call->methodName = dcMemory_strdup(_methodName);
    call->arguments = (_arguments == NULL
                       ? dcList_create()
                       : _arguments);

    // debug
    TAFFY_DEBUG(dcListElement *that;
                for (that = call->arguments->head;
                     that != NULL;
                     that = that->next)
                {
                    dcError_assert(dcNode_isTemplate(that->object));
                });

    return call;
}

dcNode *dcMethodCall_createShell(dcMethodCall *_call)
{
    return dcGraphData_createNodeWithGuts(NODE_METHOD_CALL, _call);
}

dcNode *dcMethodCall_createNode(dcNode *_receiver,
                                const char *_methodName,
                                dcList *_arguments)
{
    return (dcGraphData_createNodeWithGuts
            (NODE_METHOD_CALL,
             dcMethodCall_create
             (_receiver, _methodName, _arguments)));
}

dcNode *dcMethodCall_createNodeWithArgument(dcNode *_receiver,
                                            const char *_methodName,
                                            dcNode *_argument)
{
    dcList *arguments = (_argument == NULL
                         ? NULL
                         : dcList_createWithObjects(_argument, NULL));
    return dcMethodCall_createNode(_receiver, _methodName, arguments);
}

dcNode *dcMethodCall_createNodeWithNoArguments(dcNode *_receiver,
                                               const char *_methodName)
{
    return dcMethodCall_createNode(_receiver, _methodName, NULL);
}

void dcMethodCall_markNode(dcNode *_node)
{
    dcMethodCall *call = CAST_METHOD_CALL(_node);
    dcNode_mark(call->receiver);
    dcList_mark(call->arguments);
}

void dcMethodCall_freeNode(dcNode *_node, dcDepth _depth)
{
    dcMethodCall *call = CAST_METHOD_CALL(_node);
    dcNode_tryFree(&call->receiver, _depth);
    dcList_free(&call->arguments, _depth);
    dcMemory_free(call->methodName);
    dcMemory_free(call);
}

dcMethodCall *dcMethodCall_copy(const dcMethodCall *_from, dcDepth _depth)
{
    dcError_assert(_from->receiver == NULL
                   || ! dcNode_isRegistered(_from->receiver));
    return dcMethodCall_create
        (dcNode_copy(_from->receiver, DC_DEEP),
         _from->methodName,
         dcList_copy(_from->arguments, DC_DEEP));
}

void dcMethodCall_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_METHOD_CALL(_to) = dcMethodCall_copy(CAST_METHOD_CALL(_from), _depth);
}

struct dcMethodCallLanguage_t
{
    const char *methodCall;
    const char *language;
};

typedef struct dcMethodCallLanguage_t dcMethodCallLanguage;

static const dcMethodCallLanguage sMethodCallLanguages[] =
{
    {"#operator(()):",      "R(C)"},
    {"#operator((...)):",   "R(C)"},
    {"#operator([]):",      "R[A]"},
    {"#operator([]=):",     "R['1] = '2"},
    {"#operator([...]):",   "R[C]"},
    {"#operator([...]=):",  "R[H] = T"},
    {"#operator(==):",      "R == 1"},
    {"#operator(~=):",      "R ~=<'2> '1"},
    {"#operator(<):",       "R < 1"},
    {"#operator(<=):",      "R <= 1"},
    {"#operator(>):",       "R > 1"},
    {"#operator(>=):",      "R >= 1"},
    {"#prefixOperator(!)",  "!(R)"},
    {"#prefixOperator(~)",  "~ (R)"},
    {"#operator(<<):",      "R << 1"},
    {"#operator(<<=):",     "R <<= 1"},
    {"#operator(>>):",      "R >> 1"},
    {"#operator(>>=):",     "R >>= 1"},
    {"#operator(++)",       "R++"},
    {"#operator(--)",       "R--"},
    {"#operator(+=):",      "R += 1"},
    {"#operator(-=):",      "R -= 1"},
    {"#operator(*=):",      "R *= 1"},
    {"#operator(/=):",      "R /= 1"},
    {"#operator(^=):",      "R ^= 1"},
    {"#operator(^^=):",     "R ^^= 1"},
    {"#operator(%=):",      "R %= 1"},
    {"#operator(&):",       "R & 1"},
    {"#operator(&=):",      "R &= 1"},
    {"#operator(|):",       "R | 1"},
    {"#operator(|=):",      "R |= 1"},
    {"#operator(!)",        "(R)!"},
    {"#prefixOperator(+)",  "+R"},
    {NULL,                  NULL}
};

static const char *getMethodCallLanguage(const char *_methodCall)
{
    const dcMethodCallLanguage *finger = sMethodCallLanguages;

    while (finger->methodCall != NULL
           && strcmp(finger->methodCall, _methodCall) != 0)
    {
        finger++;
    }

    return finger->language;
}

dcResult dcMethodCall_prettyPrintNode(const dcNode *_node,
                                      dcCharacterGraph **_graph)
{
    dcCharacterGraph *graph = dcCharacterGraph_create(1, 1);
    dcMethodCall *call = CAST_METHOD_CALL(_node);
    const char *language = getMethodCallLanguage(call->methodName);
    dcResult result = TAFFY_SUCCESS;

    if (language != NULL)
    {
        size_t i;
        size_t length = strlen(language);
        bool addParens = false;
        bool addBraces = false;

        for (i = 0; i < length && result == TAFFY_SUCCESS; i++)
        {
            char symbol = language[i];

            if (symbol == 'R')
            {
                //
                // print the (R)eceiver
                //
                if (call->receiver != NULL)
                {
                    dcCharacterGraph *receiverGraph = NULL;
                    result = dcNode_prettyPrint(call->receiver, &receiverGraph);

                    if (result == TAFFY_SUCCESS)
                    {
                        if (receiverGraph->height > 1)
                        {
                            dcCharacterGraph_addParens(receiverGraph);
                        }

                        dcCharacterGraph_appendMiddle(graph, receiverGraph);
                    }

                    dcCharacterGraph_free(&receiverGraph);
                }
            }
            else if (symbol == 'A')
            {
                //
                // print (A)ll of the first argument (array)
                //
                dcCharacterGraph *argumentGraph = NULL;

                result = (dcNode_prettyPrint
                          (dcList_get(call->arguments, 0),
                           &argumentGraph));

                if (result == TAFFY_SUCCESS)
                {
                    if (addParens)
                    {
                        dcCharacterGraph_addParens(argumentGraph);
                    }
                    else if (addBraces)
                    {
                        dcCharacterGraph_addBraces(argumentGraph);
                    }

                    dcCharacterGraph_appendMiddle(graph, argumentGraph);
                }

                dcCharacterGraph_free(&argumentGraph);
            }
            else if (symbol == 'C'
                     || symbol == 'H')
            {
                //
                // print a (C)omma separated list
                //
                uint32_t j;
                uint32_t size = 0;

                dcCharacterGraph *listGraph = dcCharacterGraph_create(1, 1);
                assert(call->arguments->size == 1);
                dcArray *arguments =
                    dcArrayClass_getObjects(dcList_getHead(call->arguments));
                size = arguments->size;

                if (symbol == 'H')
                {
                    // just look at the "header", that is, everything
                    // except the tail
                    size--;
                }


                for (j = 0; j < size && result == TAFFY_SUCCESS; j++)
                {
                    dcNode *node = dcArray_get(arguments, j);
                    result = (dcCharacterGraph_appendNodeMiddleParens
                              (listGraph, node));

                    if (result == TAFFY_SUCCESS && j < size - 1)
                    {
                        dcCharacterGraph_appendCharStringMiddle
                            (listGraph, ", ");
                    }
                }

                if (addParens)
                {
                    dcCharacterGraph_addParens(listGraph);
                }
                else if (addBraces)
                {
                    dcCharacterGraph_addBraces(listGraph);
                }

                dcCharacterGraph_appendMiddle(graph, listGraph);
                dcCharacterGraph_free(&listGraph);
            }
            else if (symbol == 'T')
            {
                //
                // print the (T)ail of the first argument
                //
                assert(call->arguments->size == 1);
                dcArray *arguments =
                    dcArrayClass_getObjects(dcList_getHead(call->arguments));
                result = (dcCharacterGraph_appendNodeMiddleParens
                          (graph,
                           (dcArray_get(arguments, arguments->size - 1))));
            }
            else if (symbol == '\'')
            {
                assert(i < length - 1);
                i++;

                symbol = language[i];
                dcNode *arrayObject = dcList_get(call->arguments, 0);

                if (symbol == '1')
                {
                    // print the first object from an array object
                    result = dcCharacterGraph_appendNodeMiddleParens
                        (graph,
                         dcArray_get(dcArrayClass_getObjects(arrayObject), 0));
                }
                else if (symbol == '2')
                {
                    // print the second object from an array object
                    result = dcCharacterGraph_appendNodeMiddleParens
                        (graph,
                         dcArray_get(dcArrayClass_getObjects(arrayObject), 1));
                }
                else
                {
                    assert(false);
                }
            }
            else if (symbol == '1')
            {
                //
                // print the first
                //
                dcCharacterGraph_appendNodeMiddleParens
                    (graph, dcList_get(call->arguments, 0));
            }
            else if (symbol == '2')
            {
                //
                // print the second
                //
                dcCharacterGraph_appendNodeMiddleParens
                    (graph, dcList_get(call->arguments, 1));
            }
            else if (symbol == '(')
            {
                addParens = true;
            }
            else if (symbol == ')')
            {
                // do nothing
            }
            else if (symbol == '[')
            {
                addBraces = true;
            }
            else if (symbol == ']')
            {
                // do nothing
            }
            else
            {
                dcCharacterGraph_appendCharMiddle(graph, symbol);
            }
        }
    }
    else
    {
        dcNode *argument = NULL;
        dcListIterator *argumentsIt =
            dcList_createHeadIterator(call->arguments);
        dcList *splits = dcLexer_splitString(call->methodName, ':');
        dcListIterator *splitsIt = dcList_createHeadIterator(splits);
        dcNode *split = NULL;

        if (call->receiver != NULL)
        {
            // method call lists have NULL receivers, and we don't
            // want to print [ for each call since they are embedded
            dcCharacterGraph_appendCharStringMiddle(graph, "[");
            result = dcCharacterGraph_appendNodeMiddle(graph, call->receiver);
        }

        dcCharacterGraph_appendCharStringMiddle(graph, " ");

        if (call->arguments->size > 0)
        {
            while ((split = dcListIterator_getNext(splitsIt))
                   != NULL
                   && argumentsIt != NULL
                   && result == TAFFY_SUCCESS
                   && (argument = dcListIterator_getNext(argumentsIt)))
            {
                result = dcCharacterGraph_appendNodeMiddle(graph, split);

                if (result == TAFFY_SUCCESS)
                {
                    dcCharacterGraph_appendCharStringMiddle(graph, ": ");
                    result = dcCharacterGraph_appendNodeMiddle(graph, argument);

                    if (dcListIterator_hasNext(argumentsIt))
                    {
                        dcCharacterGraph_appendCharStringMiddle(graph, " ");
                    }
                }
            }
        }
        else
        {
            dcCharacterGraph_appendCharStringMiddle(graph, call->methodName);
        }

        dcCharacterGraph_appendCharStringMiddle(graph, "]");

        dcListIterator_free(&argumentsIt);
        dcListIterator_free(&splitsIt);
        dcList_free(&splits, DC_DEEP);
    }

    if (result == TAFFY_SUCCESS)
    {
        *_graph = graph;
    }
    else
    {
        dcCharacterGraph_free(&graph);
    }

    return result;
}

dcResult dcMethodCall_printNode(const dcNode *_node, dcString *_string)
{
    dcMethodCall *call = CAST_METHOD_CALL(_node);
    const char *language = getMethodCallLanguage(call->methodName);
    dcResult result = TAFFY_SUCCESS;

    if (language != NULL)
    {
        size_t i;
        size_t length = strlen(language);

        for (i = 0; i < length && result == TAFFY_SUCCESS; i++)
        {
            char symbol = language[i];

            if (symbol == 'R')
            {
                //
                // print the (R)eceiver
                //
                if (call->receiver != NULL)
                {
                    result = dcNode_print(call->receiver, _string);
                }
            }
            else if (symbol == 'A')
            {
                //
                // print (A)ll of the first argument (array)
                //
                result = dcNode_print(dcList_getHead(call->arguments), _string);
            }
            else if (symbol == 'C'
                     || symbol == 'H')
            {
                //
                // print a (C)omma separated list
                //
                uint32_t j;
                uint32_t size = 0;

                assert(call->arguments->size == 1);
                dcArray *arguments =
                    dcArrayClass_getObjects(dcList_getHead(call->arguments));
                size = arguments->size;

                if (symbol == 'H')
                {
                    // just look at the "header", that is, everything
                    // except the tail
                    size--;
                }

                for (j = 0; j < size && result == TAFFY_SUCCESS; j++)
                {
                    dcNode *node = dcArray_get(arguments, j);
                    result = dcNode_print(node, _string);

                    if (j < size - 1)
                    {
                        dcString_appendString(_string, ", ");
                    }
                }
            }
            else if (symbol == 'T')
            {
                //
                // print the (T)ail of the first argument
                //
                assert(call->arguments->size == 1);
                dcArray *arguments =
                    dcArrayClass_getObjects(dcList_getHead(call->arguments));
                result = (dcNode_print
                          (dcArray_get(arguments, arguments->size - 1),
                           _string));
            }
            else if (symbol == '\'')
            {
                assert(i < length - 1);
                i++;

                symbol = language[i];
                dcNode *arrayObject = dcList_get(call->arguments, 0);

                if (symbol == '1')
                {
                    // print the first object from an array object
                    result = dcNode_print
                        (dcArray_get(dcArrayClass_getObjects(arrayObject), 0),
                         _string);
                }
                else if (symbol == '2')
                {
                    // print the second object from an array object
                    result = dcNode_print
                        (dcArray_get(dcArrayClass_getObjects(arrayObject), 1),
                         _string);
                }
                else
                {
                    assert(false);
                }
            }
            else if (symbol == '1')
            {
                //
                // print the first
                //
                result = dcNode_print(dcList_get(call->arguments, 0), _string);
            }
            else if (symbol == '2')
            {
                //
                // print the second
                //
                result = dcNode_print(dcList_get(call->arguments, 1), _string);
            }
            else
            {
                dcString_appendCharacter(_string, symbol);
            }
        }
    }
    else
    {
        dcNode *argument = NULL;
        dcListIterator *argumentsIt =
            dcList_createHeadIterator(call->arguments);
        dcList *splits = dcLexer_splitString(call->methodName, ':');
        dcListIterator *splitsIt = dcList_createHeadIterator(splits);
        dcNode *split = NULL;

        if (call->receiver != NULL)
        {
            // method call lists have NULL receivers, and we don't
            // want to print [ for each call since they are embedded
            dcString_appendCharacter(_string, '[');
            result = dcNode_print(call->receiver, _string);
        }

        dcString_appendCharacter(_string, ' ');

        if (call->arguments->size > 0)
        {
            while ((split = dcListIterator_getNext(splitsIt))
                   != NULL
                   && argumentsIt != NULL
                   && result == TAFFY_SUCCESS
                   && (argument = dcListIterator_getNext(argumentsIt)))
            {
                result = dcNode_print(split, _string);

                if (result == TAFFY_SUCCESS)
                {
                    dcString_appendString(_string, ": ");
                    result = dcNode_print(argument, _string);

                    if (dcListIterator_hasNext(argumentsIt))
                    {
                        dcString_appendCharacter(_string, ' ');
                    }
                }
            }
        }
        else
        {
            dcString_appendString(_string, call->methodName);
        }

        dcString_appendCharacter(_string, ']');

        dcListIterator_free(&argumentsIt);
        dcListIterator_free(&splitsIt);
        dcList_free(&splits, DC_DEEP);
    }

    return result;
}

dcNode *dcMethodCall_getReceiver(const dcNode *_methodCall)
{
    return CAST_METHOD_CALL(_methodCall)->receiver;
}

const char *dcMethodCall_getMethodName(const dcNode *_methodCall)
{
    return CAST_METHOD_CALL(_methodCall)->methodName;
}

void dcMethodCall_setMethodName(dcMethodCall *_methodCall,
                                const char *_newName)
{
    dcMemory_free(_methodCall->methodName);
    _methodCall->methodName = dcMemory_strdup(_newName);
}

dcList *dcMethodCall_getArguments(const dcNode *_methodCall)
{
    return CAST_METHOD_CALL(_methodCall)->arguments;
}

dcResult dcMethodCall_compareNode(dcNode *_left,
                                  dcNode *_right,
                                  dcTaffyOperator *_compareResult)
{
    dcResult result = TAFFY_FAILURE;
    *_compareResult = TAFFY_LESS_THAN;

    if (IS_METHOD_CALL(_right))
    {
        result = dcNode_compare(dcMethodCall_getReceiver(_left),
                                dcMethodCall_getReceiver(_right),
                                _compareResult);

        if (result == TAFFY_SUCCESS && *_compareResult == TAFFY_EQUALS)
        {
            if (dcMemory_taffyStringCompare(dcMethodCall_getMethodName(_left),
                                            dcMethodCall_getMethodName(_right))
                == TAFFY_EQUALS)
            {
                result = dcList_compare(dcMethodCall_getArguments(_left),
                                        dcMethodCall_getArguments(_right),
                                        _compareResult);
            }
        }
    }

    return result;
}

bool dcMethodCall_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcNode *receiver = NULL;
    dcList *arguments = NULL;
    char *methodName = NULL;

    if (dcMarshaller_unmarshallNoNull(_stream,
                                      "sl",
                                      &methodName,
                                      &arguments)
        && dcMarshaller_unmarshall(_stream, "n", &receiver))
    {
        result = true;

        if (receiver != NULL)
        {
            dcNode_setTemplate(receiver, true);
        }

        dcListElement *that;

        for (that = arguments->head; that != NULL; that = that->next)
        {
            dcNode_setTemplate(that->object, true);
        }

        CAST_METHOD_CALL(_node) =
            dcMethodCall_create(receiver, methodName, arguments);
        dcMemory_free(methodName);
    }

    return result;
}

dcString *dcMethodCall_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcMethodCall *methodCall = CAST_METHOD_CALL(_node);
    return dcMarshaller_marshall(_stream,
                                 "sln",
                                 methodCall->methodName,
                                 methodCall->arguments,
                                 methodCall->receiver);
}

dcMethodCall *dcMethodCall_castMe(dcNode *_node)
{
    return CAST_METHOD_CALL(_node);
}

dcResult dcMethodCall_hashNode(dcNode *_node, dcHashType *_hashResult)
{
    dcMethodCall *methodCall = CAST_METHOD_CALL(_node);
    dcHashType methodNameHash = 0;

    assert(dcString_hashCharArray(methodCall->methodName, &methodNameHash)
           == TAFFY_SUCCESS);

    *_hashResult = 57 + methodNameHash;

    // TODO: add hash for receiver and arguments

    return TAFFY_SUCCESS;
}
