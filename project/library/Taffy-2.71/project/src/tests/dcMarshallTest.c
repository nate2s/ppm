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

#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcComplexNumber.h"
#include "dcComplexNumberClass.h"
#include "dcContainers.h"
#include "dcError.h"
#include "dcFloat.h"
#include "dcFutureClass.h"
#include "dcGarbageCollector.h"
#include "dcGraphDatas.h"
#include "dcHashClass.h"
#include "dcInt32.h"
#include "dcListClass.h"
#include "dcMarshaller.h"
#include "dcMatrix.h"
#include "dcMatrixClass.h"
#include "dcMemory.h"
#include "dcNilClass.h"
#include "dcNumber.h"
#include "dcNoClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcNumberClass.h"
#include "dcPairClass.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcStringClass.h"
#include "dcSymbolClass.h"
#include "dcSystem.h"
#include "dcTestUtilities.h"
#include "dcUnsignedInt32.h"
#include "dcYesClass.h"

#define MARSHALL_TEST_FILE_NAME "src/tests/dcMarshallTest.c"

#define DEBUG(x)

static dcNode *sNilObject1 = NULL;
static dcNode *sNilObject2 = NULL;
static dcNode *sStringObject1 = NULL;

static dcNode *sIdentifier1 = NULL;
static dcNode *sIdentifier2 = NULL;

typedef dcNode *(*CreateTwo)(dcNode *_first, dcNode *_second);
typedef dcNode *(*CreateZero)(void);
typedef dcNode *(*CreateNode)(dcNode *_node);

static void regressString(const dcString *_string, bool _register)
{
    dcString *string = dcString_copy(_string, DC_DEEP);

    DEBUG(int iteration = 0);

    while (string->length >= 1)
    {
        dcString_resetIndex(string);
        dcNode *node = dcNode_unmarshall(string);

        if (node != NULL)
        {
            if (_register)
            {
                dcNode_register(node);
            }
            else
            {
                dcNode_free(&node, DC_DEEP);
            }
        }

        dcString_resize(string, string->length - 1);

        DEBUG(iteration++);
        DEBUG(fprintf(stderr, "regressString iteration: %d, length: %d\n",
                      iteration,
                      string->length));
        break;
    }

    dcString_free(&string, DC_DEEP);
}

static void regressScopeString(const dcString *_string)
{
    dcString *string = dcString_copy(_string, DC_DEEP);

    DEBUG(int iteration = 0);

    while (string->length > 0)
    {
        dcScope *scope = dcScope_unmarshall(string);

        if (scope)
        {
            dcScope_free(&scope, DC_DEEP);
        }

        dcString_resize(string, string->length - 1);

        DEBUG(iteration++);
        DEBUG(fprintf(stderr, "regressString iteration: %d, length: %d\n",
                      iteration,
                      string->length));
    }

    dcString_free(&string, DC_DEEP);
}

static void testGraphDataPair(CreateTwo _function,
                              dcNode *_left,
                              dcNode *_right)
{
    dcNode *node = (*_function)(_left, _right);
    dcString *marshalled = dcNode_marshall(node, NULL);
    dcNode *unmarshalled = dcNode_unmarshall(dcString_resetIndex(marshalled));

    dcTestUtilities_assert(unmarshalled != NULL);

    dcPair *pair = CAST_GRAPH_DATA_PAIR(unmarshalled);
    dcNode *leftUnmarshalled = pair->left;
    dcNode *rightUnmarshalled = pair->right;

    dcTaffyOperator comparison1;
    dcTaffyOperator comparison2;

    dcError_assert((dcNode_compare(_left, leftUnmarshalled, &comparison1)
                    == TAFFY_SUCCESS)
                   && comparison1 == TAFFY_EQUALS);
    dcError_assert((dcNode_compare(_right, rightUnmarshalled, &comparison2)
                    == TAFFY_SUCCESS)
                   && comparison2 == TAFFY_EQUALS);

    regressString(marshalled, false);
    dcNode_free(&node, DC_SHALLOW);
    dcString_free(&marshalled, DC_DEEP);
    dcNode_free(&unmarshalled, DC_DEEP);
}

static void testGraphDataZero(CreateZero _function)
{
    dcNode *node = (*_function)();
    dcString *marshalled = dcNode_marshall(node, NULL);
    dcNode *unmarshalled = dcNode_unmarshall(dcString_resetIndex(marshalled));

    dcGraphDataType type = dcGraphData_getType(node);
    dcGraphData_assertType(unmarshalled, type);

    regressString(marshalled, false);
    dcNode_free(&node, DC_DEEP);
    dcNode_free(&unmarshalled, DC_DEEP);
    dcString_free(&marshalled, DC_DEEP);
}

static void testGraphDataNode(CreateNode _function, dcNode *_argument)
{
    dcNode *node = (*_function)(_argument);
    dcString *marshalled = dcNode_marshall(node, NULL);
    dcNode *unmarshalled = dcNode_unmarshall(dcString_resetIndex(marshalled));

    dcGraphDataType nodeType = dcGraphData_getType(node);
    dcGraphData_assertType(unmarshalled, nodeType);

    if (_argument)
    {
        dcGraphDataType argumentType = dcGraphData_getType(_argument);
        dcNode *unmarshalledArgument = CAST_GRAPH_DATA_NODE(unmarshalled);
        dcGraphData_assertType(unmarshalledArgument, argumentType);
    }
    else
    {
        dcTestUtilities_assert(CAST_GRAPH_DATA_NODE(unmarshalled) == NULL);
    }

    regressString(marshalled, false);
    dcNode_free(&node, DC_SHALLOW);
    dcNode_free(&unmarshalled, DC_DEEP);
    dcString_free(&marshalled, DC_DEEP);
}

static void assertGraphDataTreeEquals(dcNode *root, dcNode *unmarshalled)
{
    dcGraphData_assertType(unmarshalled, NODE_GRAPH_DATA_TREE);
    dcNode_assertEqual(root, unmarshalled);
}

///////////////////
//               //
// The Tests!!1  //
//               //
///////////////////

static void testArray(void)
{
    {
        dcNode *array =
            dcArray_createNodeWithObjects(sNilObject1, sStringObject1, NULL);

        dcString *arrayMarshalled = dcNode_marshall(array, NULL);
        dcNode *arrayUnmarshalled =
            dcNode_unmarshall(dcString_resetIndex(arrayMarshalled));

        dcNode_assertType(arrayUnmarshalled, NODE_ARRAY);
        dcTestUtilities_assert(dcArray_getSize(arrayUnmarshalled) == 2);
        dcClass_assertHasTemplate(dcArray_get(CAST_ARRAY(arrayUnmarshalled), 0),
                                  dcNilClass_getTemplate());
        dcClass_assertHasTemplate(dcArray_get(CAST_ARRAY(arrayUnmarshalled), 1),
                                  dcStringClass_getTemplate());

        regressString(arrayMarshalled, false);
        dcNode_free(&array, DC_SHALLOW);
        dcString_free(&arrayMarshalled, DC_DEEP);
        dcNode_free(&arrayUnmarshalled, DC_DEEP);
    }
}

static void testArrayObject(void)
{
    dcArray *array =
        dcArray_createWithObjects(sNilObject1, sNilObject2, NULL);

    dcNode *arrayObject = dcNode_setTemplate
        (dcArrayClass_createObject(array, true), true);
    dcString *arrayMarshalled = dcNode_marshall(arrayObject, NULL);

    dcTestUtilities_assert(arrayMarshalled != NULL);

    dcNode *arrayUnmarshalled =
        dcNode_unmarshall(dcString_resetIndex(arrayMarshalled));
    dcTestUtilities_assert(arrayUnmarshalled != NULL);
    regressString(arrayMarshalled, true);
    dcClass_assertHasTemplate(arrayUnmarshalled,
                              dcArrayClass_getTemplate());
    dcNode_free(&arrayUnmarshalled, DC_DEEP);
    dcNode_free(&arrayObject, DC_DEEP);
    dcString_free(&arrayMarshalled, DC_DEEP);
}

static void testFutureObject(void)
{
    // empty
    {
        dcNode *emptyFutureObject = (dcNode_setTemplate
                                     (dcFutureClass_createObject(), true));
        dcString *futureMarshalled = dcNode_marshall(emptyFutureObject, NULL);

        dcTestUtilities_assert(futureMarshalled != NULL);

        dcNode *futureUnmarshalled =
            dcNode_unmarshall(dcString_resetIndex(futureMarshalled));
        dcTestUtilities_assert(futureUnmarshalled != NULL);
        regressString(futureMarshalled, true);
        dcClass_assertHasTemplate(futureUnmarshalled,
                                  dcFutureClass_getTemplate());
        dcNode_free(&futureUnmarshalled, DC_DEEP);
        dcNode_free(&emptyFutureObject, DC_DEEP);
        dcString_free(&futureMarshalled, DC_DEEP);
    }

    // populated
    {
        dcNode *futureObject = (dcNode_setTemplate
                                (dcFutureClass_createObject(), true));
        dcFutureClass_setValue(futureObject,
                               dcNode_setTemplate(dcNode_copy(sNilObject1,
                                                              DC_DEEP),
                                                  true));

        dcString *futureMarshalled = dcNode_marshall(futureObject, NULL);

        dcTestUtilities_assert(futureMarshalled != NULL);

        dcNode *futureUnmarshalled =
            dcNode_unmarshall(dcString_resetIndex(futureMarshalled));
        dcTestUtilities_assert(futureUnmarshalled != NULL);
        regressString(futureMarshalled, true);
        dcClass_assertHasTemplate(futureUnmarshalled,
                                  dcFutureClass_getTemplate());
        dcNode_free(&futureUnmarshalled, DC_DEEP);
        dcNode_free(&futureObject, DC_DEEP);
        dcString_free(&futureMarshalled, DC_DEEP);
    }
}

static void testAssignment(void)
{
    dcNode *assignmentNode =
        dcAssignment_createNode(sNilObject1, sStringObject1, SCOPE_DATA_GLOBAL);
    dcString *marshalled = dcNode_marshall(assignmentNode, NULL);
    dcNode *unmarshalled = dcNode_unmarshall(dcString_resetIndex(marshalled));

    dcGraphData_assertType(unmarshalled, NODE_ASSIGNMENT);
    dcAssignment *assignment = CAST_ASSIGNMENT(unmarshalled);

    dcTestUtilities_assert(assignment != NULL);
    dcTestUtilities_assert(dcAssignment_isGlobal(assignment));
    dcTestUtilities_assert(! dcAssignment_isConstant(assignment));

    dcNode *identifier = dcAssignment_getIdentifier(unmarshalled);
    dcNode *value = dcAssignment_getValue(unmarshalled);

    dcClass_assertHasTemplate(identifier, dcNilClass_getTemplate());
    dcClass_assertHasTemplate(value, dcStringClass_getTemplate());

    dcNode_free(&assignmentNode, DC_SHALLOW);
    dcNode_free(&unmarshalled, DC_DEEP);
    dcString_free(&marshalled, DC_DEEP);
}

static void testBasicMarshall(const char *_program,
                              const dcClassTemplate *_template)
{
    dcNode *root = dcParser_parseString(_program,
                                        MARSHALL_TEST_FILE_NAME,
                                        true);
    dcString *marshalled = dcNode_marshall(root, NULL);
    dcNode *unmarshalled = dcNode_unmarshall(dcString_resetIndex(marshalled));

    regressString(marshalled, false);
    dcGraphData_assertType(unmarshalled, NODE_GRAPH_DATA_TREE);
    dcClass_assertHasTemplate(dcGraphDataTree_getContents(unmarshalled),
                              _template);
    dcNode_free(&root, DC_DEEP);
    dcString_free(&marshalled, DC_DEEP);
    dcNode_free(&unmarshalled, DC_DEEP);
}

static const char *blockProgram1 = "^{ <a, b, c> a + b + c }";
static const char *blockProgram2 = "^{ <foob> foob + 2 }";

static void testBlockObject(void)
{
    testBasicMarshall(blockProgram1, dcBlockClass_getTemplate());
    testBasicMarshall(blockProgram2, dcBlockClass_getTemplate());
}

static void testBreak(void)
{
    testGraphDataZero(&dcBreak_createNode);
}

static void testExit(void)
{
    testGraphDataNode(&dcExit_createNode, sNilObject1);
    testGraphDataNode(&dcExit_createNode, NULL);
}

static void testFalse(void)
{
    testGraphDataZero(&dcFalse_createNode);
}

static const char *functionProgram = "f(a) = a + f(a-1); f(0) = 0";

static void testFunctionObject(void)
{
    // simple function
    {
        dcNode *root = dcParser_parseString("f(a) = a",
                                            MARSHALL_TEST_FILE_NAME,
                                            true);
        dcString *marshalled = dcNode_marshall(root, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        assert(unmarshalled != NULL);
        assertGraphDataTreeEquals(root, unmarshalled);
        dcNode_free(&root, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }

    // two functions-ish
    {
        dcNode *root = dcParser_parseString(functionProgram,
                                            MARSHALL_TEST_FILE_NAME,
                                            true);
        dcString *marshalled = dcGraphData_marshallTree(root, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        assert(unmarshalled != NULL);
        assertGraphDataTreeEquals(root, unmarshalled);
        dcNode_free(&root, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }
}

static const char *singleStatement = "a = 1";
static const char *simpleProgram = "a = 1; a and b; a or c";
static const char *simpleMethodCall = "[a b]";
static const char *simpleMethodCalls = "[[a b] c]";

static void testGraphDataTree(void)
{
    // simple statement
    {
        dcNode *root = dcParser_parseString(singleStatement,
                                            MARSHALL_TEST_FILE_NAME,
                                            true);
        dcString *marshalled = dcNode_marshall(root, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));

        assertGraphDataTreeEquals(root, unmarshalled);

        dcNode_free(&root, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }

    // simple program
    {
        dcNode *root = dcParser_parseString(simpleProgram,
                                            MARSHALL_TEST_FILE_NAME,
                                            true);
        dcString *marshalled = dcNode_marshall(root, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));

        assertGraphDataTreeEquals(root, unmarshalled);

        dcNode_free(&root, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }

    // method call
    {
        dcNode *root = dcParser_parseString(simpleMethodCall,
                                            MARSHALL_TEST_FILE_NAME,
                                            true);
        dcString *marshalled = dcNode_marshall(root, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));

        assertGraphDataTreeEquals(root, unmarshalled);

        dcNode_free(&root, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }

    // method calls
    {
        dcNode *root = dcParser_parseString(simpleMethodCalls,
                                            MARSHALL_TEST_FILE_NAME,
                                            true);
        dcString *marshalled = dcNode_marshall(root, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));

        assertGraphDataTreeEquals(root, unmarshalled);

        dcNode_free(&root, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }
}

static void testHash(void)
{
    // dcHash with numbers
    {
        dcHash *hash = dcHash_create();
        dcNode *one = dcNumberClass_createObjectFromInt32s(1);
        dcNode *two = dcNumberClass_createObjectFromInt32s(2);
        dcTestUtilities_assert
            (dcHash_setValueWithStringKey(hash, "one", one)
             == TAFFY_SUCCESS);
        dcTestUtilities_assert
            (dcHash_setValueWithStringKey(hash, "two", two)
             == TAFFY_SUCCESS);

        dcString *marshalled = dcHash_marshall(hash, NULL);
        dcTestUtilities_assert(marshalled != NULL);

        dcHash *unmarshalled =
            dcHash_unmarshall(dcString_resetIndex(marshalled));
        dcTestUtilities_assert(unmarshalled != NULL);
        dcTestUtilities_assert(unmarshalled->size == 2);

        dcNode *oneObject;
        dcTestUtilities_assert
            (dcHash_getValueWithStringKey(unmarshalled, "one", &oneObject)
             == TAFFY_SUCCESS);
        dcNode *twoObject;
        dcTestUtilities_assert
            (dcHash_getValueWithStringKey(unmarshalled, "two", &twoObject)
             == TAFFY_SUCCESS);

        dcClass_assertHasTemplate(oneObject, dcNumberClass_getTemplate());
        dcClass_assertHasTemplate(twoObject, dcNumberClass_getTemplate());

        int32_t oneNumber;
        dcTestUtilities_assert(dcNumberClass_extractInt32s
                               (oneObject, &oneNumber));
        dcTestUtilities_assert(oneNumber == 1);
        int32_t twoNumber;
        dcTestUtilities_assert(dcNumberClass_extractInt32s
                               (twoObject, &twoNumber));
        dcTestUtilities_assert(twoNumber == 2);

        dcHash_free(&hash, DC_SHALLOW);
        dcHash_free(&unmarshalled, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&one, DC_DEEP);
        dcNode_free(&two, DC_DEEP);
    }

    // dcHash
    {
        dcHash *hash = dcHash_create();
        dcNode *one = dcNumberClass_createObjectFromInt32s(1);
        dcNode *two = dcNumberClass_createObjectFromInt32s(2);
        dcTestUtilities_assert
            (dcHash_setValueWithStringKey(hash, "nil1", sNilObject1)
             == TAFFY_SUCCESS);
        dcTestUtilities_assert
            (dcHash_setValueWithStringKey(hash, "string1", sStringObject1)
             == TAFFY_SUCCESS);
        dcTestUtilities_assert
            (dcHash_setValueWithStringKey(hash, "one", one)
             == TAFFY_SUCCESS);
        dcTestUtilities_assert
            (dcHash_setValueWithStringKey(hash, "two", two)
             == TAFFY_SUCCESS);

        dcString *marshalled = dcHash_marshall(hash, NULL);
        dcTestUtilities_assert(marshalled != NULL);

        dcHash *unmarshalled =
            dcHash_unmarshall(dcString_resetIndex(marshalled));
        dcTestUtilities_assert(unmarshalled != NULL);
        dcTestUtilities_assert(unmarshalled->size == 4);

        dcNode *object1;
        dcNode *object2;
        dcNode *oneObject;
        dcNode *twoObject;

        dcTestUtilities_assert
            (dcHash_getValueWithStringKey(unmarshalled, "nil1", &object1)
             == TAFFY_SUCCESS);
        dcTestUtilities_assert
            (dcHash_getValueWithStringKey(unmarshalled, "string1", &object2)
             == TAFFY_SUCCESS);
        dcTestUtilities_assert
            (dcHash_getValueWithStringKey(unmarshalled, "one", &oneObject)
             == TAFFY_SUCCESS);
        dcTestUtilities_assert
            (dcHash_getValueWithStringKey(unmarshalled, "two", &twoObject)
             == TAFFY_SUCCESS);

        dcClass_assertHasTemplate(object1, dcNilClass_getTemplate());
        dcClass_assertHasTemplate(object2, dcStringClass_getTemplate());
        dcClass_assertHasTemplate(oneObject, dcNumberClass_getTemplate());
        dcClass_assertHasTemplate(twoObject, dcNumberClass_getTemplate());

        int32_t oneNumber;
        dcTestUtilities_assert
            (dcNumberClass_extractInt32s(oneObject, &oneNumber));
        dcTestUtilities_assert(oneNumber == 1);
        int32_t twoNumber;
        dcTestUtilities_assert
            (dcNumberClass_extractInt32s(twoObject, &twoNumber));
        dcTestUtilities_assert(twoNumber == 2);

        dcHash_free(&hash, DC_SHALLOW);
        dcHash_free(&unmarshalled, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&one, DC_DEEP);
        dcNode_free(&two, DC_DEEP);
    }

    // dcHash(dcNode)
    {
        dcNode *hash = dcHash_createNode();
        dcHash_setValueWithStringKey(CAST_HASH(hash), "nil1", sNilObject1);
        dcHash_setValueWithStringKey(CAST_HASH(hash),
                                     "string1",
                                     sStringObject1);

        dcString *marshalled = dcNode_marshall(hash, NULL);
        dcTestUtilities_assert(marshalled != NULL);

        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        dcTestUtilities_assert(unmarshalled != NULL);
        dcTestUtilities_assert(dcHash_getSize(unmarshalled) == 2);

        dcNode *object1;
        dcNode *object2;

        dcTestUtilities_assert
            (dcHash_getValueWithStringKey
             (CAST_HASH(unmarshalled), "nil1", &object1)
             == TAFFY_SUCCESS);
        dcTestUtilities_assert
            (dcHash_getValueWithStringKey
             (CAST_HASH(unmarshalled), "string1", &object2)
             == TAFFY_SUCCESS);

        dcClass_assertHasTemplate(object1, dcNilClass_getTemplate());
        dcClass_assertHasTemplate(object2, dcStringClass_getTemplate());

        dcNode_free(&hash, DC_SHALLOW);
        dcNode_free(&unmarshalled, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
    }

    // hash with marshaller, 1 object
    {
        dcHash *hash = dcHash_create();
        dcNode *one = dcNumberClass_createObjectFromInt32s(1);
        dcTestUtilities_assert
            (dcHash_setValueWithStringKey(hash, "one", one)
             == TAFFY_SUCCESS);

        dcString *marshalled = dcMarshaller_marshall(NULL, "h", hash);
        dcHash *unmarshalled = NULL;

        dcTestUtilities_assert(marshalled != NULL
                               && (dcMarshaller_unmarshall
                                   (dcString_resetIndex(marshalled),
                                    "h",
                                    &unmarshalled)));
        dcTestUtilities_assert(unmarshalled->size == 1);
        dcHash_free(&hash, DC_DEEP);
        dcHash_free(&unmarshalled, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
    }

    // hash with marshaller, 2 objects
    {
        dcHash *hash = dcHash_create();
        dcNode *one = dcNumberClass_createObjectFromInt32s(1);
        dcNode *two = dcNumberClass_createObjectFromInt32s(2);
        dcTestUtilities_assert
            (dcHash_setValueWithStringKey(hash, "one", one)
             == TAFFY_SUCCESS);
        dcTestUtilities_assert
            (dcHash_setValueWithStringKey(hash, "two", two)
             == TAFFY_SUCCESS);

        dcString *marshalled = dcMarshaller_marshall(NULL, "h", hash);
        dcHash *unmarshalled = NULL;

        dcTestUtilities_assert(marshalled != NULL
                               && (dcMarshaller_unmarshall
                                   (dcString_resetIndex(marshalled),
                                    "h",
                                    &unmarshalled)));
        dcTestUtilities_assert(unmarshalled->size == 2);
        dcHash_free(&hash, DC_DEEP);
        dcHash_free(&unmarshalled, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
    }
}

static void testHashObject(void)
{
    // initialized
    {
        dcHash *hash = dcHash_create();
        dcHash_setValueWithStringKey(hash,
                                     "nil1",
                                     dcNode_copy(sNilObject1, DC_DEEP));
        dcHash_setValueWithStringKey(hash,
                                     "string1",
                                     dcNode_copy(sStringObject1, DC_DEEP));

        dcNode *hashObject = dcHashClass_createInitializedNode(hash, true);
        dcString *marshalled = dcNode_marshall(hashObject, NULL);
        dcTestUtilities_assert(marshalled != NULL);

        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        dcClass_assertHasTemplate(unmarshalled, dcHashClass_getTemplate());
        dcTestUtilities_assert(dcHashClass_isInitialized(unmarshalled));
        dcNode_register(unmarshalled);

        dcHash *unmarshalledHash = dcHashClass_getHash(unmarshalled);
        dcTestUtilities_assert(unmarshalledHash->size == 2);

        dcNode *object1;
        dcNode *object2;

        dcTestUtilities_assert
            (dcHash_getValueWithStringKey(unmarshalledHash, "nil1", &object1)
             == TAFFY_SUCCESS);
        dcTestUtilities_assert
            (dcHash_getValueWithStringKey(unmarshalledHash, "string1", &object2)
             == TAFFY_SUCCESS);

        dcClass_assertHasTemplate(object1, dcNilClass_getTemplate());
        dcClass_assertHasTemplate(object2, dcStringClass_getTemplate());

        regressString(marshalled, true);
        dcNode_register(hashObject);
        dcString_free(&marshalled, DC_DEEP);
    }

    // uninitialized
    {
        dcList *keys = dcList_createWithObjects
            (dcNode_copy(sStringObject1, DC_DEEP), NULL);
        dcList *values =
            dcList_createWithObjects(dcNode_copy(sNilObject1, DC_DEEP), NULL);
        dcNode *hashObject =
            dcNode_setTemplate(dcHashClass_createUninitializedNode
                               (keys, values, true),
                               true);

        dcString *marshalled = dcNode_marshall(hashObject, NULL);
        dcTestUtilities_assert(marshalled != NULL);

        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        dcClass_assertHasTemplate(unmarshalled, dcHashClass_getTemplate());
        dcTestUtilities_assert(!dcHashClass_isInitialized(unmarshalled));

        dcList *unmarshalledKeys = dcHashClass_getTempKeys(unmarshalled);
        dcTestUtilities_assert(unmarshalledKeys != NULL);
        dcTestUtilities_assert(unmarshalledKeys->size == 1);

        dcNode *keyObject = dcList_getHead(unmarshalledKeys);

        dcList *unmarshalledValues = dcHashClass_getTempValues(unmarshalled);
        dcTestUtilities_assert(unmarshalledValues != NULL);
        dcTestUtilities_assert(unmarshalledValues->size == 1);

        dcNode *valueObject = dcList_getHead(unmarshalledValues);

        dcClass_assertHasTemplate(keyObject, dcStringClass_getTemplate());
        dcClass_assertHasTemplate(valueObject, dcNilClass_getTemplate());

        regressString(marshalled, false);
        dcNode_free(&hashObject, DC_SHALLOW);
        dcNode_free(&unmarshalled, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
    }
}

static void testIdentifier(void)
{
    dcNode *identifierNode =
        dcIdentifier_createNode("testIdentifier", NO_FLAGS);

    dcString *identifierMarshalled = dcNode_marshall(identifierNode, NULL);
    dcTestUtilities_assert(identifierMarshalled != NULL);

    dcNode *identifierUnmarshalled =
        dcNode_unmarshall(dcString_resetIndex(identifierMarshalled));
    dcGraphData_assertType(identifierUnmarshalled, NODE_IDENTIFIER);

    regressString(identifierMarshalled, false);
    dcNode_free(&identifierNode, DC_SHALLOW);
    dcNode_free(&identifierUnmarshalled, DC_DEEP);
    dcString_free(&identifierMarshalled, DC_DEEP);
}

static void testIf(void)
{
    {
        dcNode *ifNode = dcIf_createNode(sNilObject1, sStringObject1, NULL);

        dcString *ifMarshalled = dcNode_marshall(ifNode, NULL);
        dcTestUtilities_assert(ifMarshalled != NULL);

        dcNode *ifUnmarshalled =
            dcNode_unmarshall(dcString_resetIndex(ifMarshalled));
        dcGraphData_assertType(ifUnmarshalled, NODE_IF);

        dcIf *ifData = CAST_IF(ifUnmarshalled);
        dcTestUtilities_assert(ifData != NULL);
        dcTestUtilities_assert(dcIf_getCondition(ifUnmarshalled) != NULL);
        dcTestUtilities_assert(dcIf_getStatement(ifUnmarshalled) != NULL);
        dcTestUtilities_assert(dcIf_getNext(ifUnmarshalled) == NULL);
        dcClass_assertHasTemplate(dcIf_getCondition(ifUnmarshalled),
                                  dcNilClass_getTemplate());
        dcClass_assertHasTemplate(dcIf_getStatement(ifUnmarshalled),
                                  dcStringClass_getTemplate());

        regressString(ifMarshalled, false);
        dcNode_free(&ifNode, DC_SHALLOW);
        dcNode_free(&ifUnmarshalled, DC_DEEP);
        dcString_free(&ifMarshalled, DC_DEEP);
    }

    {
        dcNode *ifNode1 = dcIf_createNode(sStringObject1, sNilObject1, NULL);
        dcNode *ifNode2 = dcIf_createNode(sNilObject1, sStringObject1, ifNode1);
        dcString *ifMarshalled = dcNode_marshall(ifNode2, NULL);
        dcTestUtilities_assert(ifMarshalled != NULL);

        dcNode *ifUnmarshalled =
            dcNode_unmarshall(dcString_resetIndex(ifMarshalled));
        dcGraphData_assertType(ifUnmarshalled, NODE_IF);

        dcIf *ifData = CAST_IF(ifUnmarshalled);
        dcTestUtilities_assert(ifData != NULL);
        dcTestUtilities_assert(dcIf_getCondition(ifUnmarshalled) != NULL);
        dcTestUtilities_assert(dcIf_getStatement(ifUnmarshalled) != NULL);
        dcTestUtilities_assert(dcIf_getNext(ifUnmarshalled) != NULL);
        dcClass_assertHasTemplate(dcIf_getCondition(ifUnmarshalled),
                                  dcNilClass_getTemplate());
        dcClass_assertHasTemplate(dcIf_getStatement(ifUnmarshalled),
                                  dcStringClass_getTemplate());
        dcGraphData_assertType(dcIf_getNext(ifUnmarshalled), NODE_IF);

        dcNode *ifIfNode = dcIf_getNext(ifUnmarshalled);
        dcIf *ifIf = CAST_IF(ifIfNode);

        dcTestUtilities_assert(ifIf != NULL);
        dcTestUtilities_assert(dcIf_getCondition(ifIfNode) != NULL);
        dcTestUtilities_assert(dcIf_getStatement(ifIfNode) != NULL);
        dcTestUtilities_assert(dcIf_getNext(ifIfNode) == NULL);
        dcClass_assertHasTemplate(dcIf_getCondition(ifIfNode),
                                  dcStringClass_getTemplate());
        dcClass_assertHasTemplate(dcIf_getStatement(ifIfNode),
                                  dcNilClass_getTemplate());

        regressString(ifMarshalled, false);
        dcNode_free(&ifNode2, DC_SHALLOW);
        dcNode_free(&ifUnmarshalled, DC_DEEP);
        dcString_free(&ifMarshalled, DC_DEEP);
    }
}

static void int32Tester(int32_t _value)
{
    dcString *intMarshalled = dcInt32_marshall(_value, NULL);
    int32_t unmarshalled;
    assert(dcInt32_unmarshall(dcString_resetIndex(intMarshalled),
                              &unmarshalled));
    assert(unmarshalled == _value);
    dcString_free(&intMarshalled, DC_DEEP);
}

static void testInt32(void)
{
    int32Tester(0);
    int32Tester(0xFFEE);
    int32Tester(0xFFEEAA);
    int32Tester(123456);
    int32Tester(-1);
    int32Tester(-123456);
    int32Tester(0xFFE);
    int32Tester(1);
}

static void unsignedInt32Tester(uint32_t _value)
{
    dcString *intMarshalled = dcUnsignedInt32_marshall(_value, NULL);
    uint32_t unmarshalled;
    assert(dcUnsignedInt32_unmarshall(dcString_resetIndex(intMarshalled),
                                      &unmarshalled));
    assert(unmarshalled == _value);
    dcString_free(&intMarshalled, DC_DEEP);
}

static void testUnsignedInt32(void)
{
    unsignedInt32Tester(0);
    unsignedInt32Tester(0xFFEE);
    unsignedInt32Tester(0xFFEEAA);
    unsignedInt32Tester(123456);
    unsignedInt32Tester(-1);
    unsignedInt32Tester(-123456);
    unsignedInt32Tester(0xFFE);
    unsignedInt32Tester(1);
}

static void floatTester(float _value)
{
    dcString *marshalled = dcFloat_marshall(_value, NULL);
    float unmarshalled;

    dcTestUtilities_assert(dcFloat_unmarshall
                           (dcString_resetIndex(marshalled),
                            &unmarshalled));
    dcTestUtilities_assert(unmarshalled == _value);

    dcString_free(&marshalled, DC_DEEP);
}

static void testFloat(void)
{
    floatTester(0);
    floatTester(0.1234);
    floatTester(0.1111);
    floatTester(0.1);
    floatTester(-1.2345);
    floatTester(-1.1111);
}

static void testList(void)
{
    // no objects
    {
        dcNode *list = dcList_createNode();
        dcString *listMarshalled = dcNode_marshall(list, NULL);
        dcNode *listUnmarshalled =
            dcNode_unmarshall(dcString_resetIndex(listMarshalled));

        dcNode_assertType(listUnmarshalled, NODE_LIST);
        dcTestUtilities_assert(dcList_getSize(listUnmarshalled) == 0);

        dcNode_free(&list, DC_DEEP);
        dcString_free(&listMarshalled, DC_DEEP);
        dcNode_free(&listUnmarshalled, DC_DEEP);
    }

    // one object
    {
        dcNode *list = dcList_createNodeWithObjects(sNilObject1, NULL);
        dcString *listMarshalled = dcNode_marshall(list, NULL);
        dcNode *listUnmarshalled =
            dcNode_unmarshall(dcString_resetIndex(listMarshalled));

        dcNode_assertType(listUnmarshalled, NODE_LIST);
        assert(dcList_getSize(listUnmarshalled) == 1);
        dcClass_assertHasTemplate(dcList_get(CAST_LIST(listUnmarshalled), 0),
                                  dcNilClass_getTemplate());

        regressString(listMarshalled, false);
        dcNode_free(&list, DC_SHALLOW);
        dcString_free(&listMarshalled, DC_DEEP);
        dcNode_free(&listUnmarshalled, DC_DEEP);
    }

    // more objects
    {
        dcNode *list =
            dcList_createNodeWithObjects(sNilObject1, sStringObject1, NULL);

        dcString *listMarshalled = dcNode_marshall(list, NULL);
        dcNode *listUnmarshalled =
            dcNode_unmarshall(dcString_resetIndex(listMarshalled));

        dcNode_assertType(listUnmarshalled, NODE_LIST);
        dcTestUtilities_assert(dcList_getSize(listUnmarshalled) == 2);
        dcClass_assertHasTemplate(dcList_get(CAST_LIST(listUnmarshalled), 0),
                                  dcNilClass_getTemplate());
        dcClass_assertHasTemplate(dcList_get(CAST_LIST(listUnmarshalled), 1),
                                  dcStringClass_getTemplate());

        regressString(listMarshalled, false);
        dcNode_free(&list, DC_SHALLOW);
        dcString_free(&listMarshalled, DC_DEEP);
        dcNode_free(&listUnmarshalled, DC_DEEP);
    }
}

static void testListObject(void)
{
    dcList *list =
        dcList_createWithObjects(sNilObject1, sNilObject2, NULL);
    dcNode *listObject =
        dcNode_setTemplate(dcListClass_createObject(list, true), true);
    dcString *listMarshalled = dcNode_marshall(listObject, NULL);

    dcTestUtilities_assert(listMarshalled != NULL);

    dcNode *listUnmarshalled =
        dcNode_unmarshall(dcString_resetIndex(listMarshalled));
    dcTestUtilities_assert(dcListClass_getSize(listUnmarshalled) == 2);

    regressString(listMarshalled, false);
    dcClass_assertHasTemplate(listUnmarshalled, dcListClass_getTemplate());
    dcNode_free(&listUnmarshalled, DC_DEEP);
    dcNode_free(&listObject, DC_DEEP);
    dcString_free(&listMarshalled, DC_DEEP);
}

static const char *matrixProgram = "||1,2,3,4,5|| == ||1,2,3,3+1,6-1||";
static const char *matrixString = "||1,2,3,4;5,6,7,8||";

static void testMatrix(void)
{
    {
        dcNode *root = dcParser_parseString(matrixProgram,
                                            MARSHALL_TEST_FILE_NAME,
                                            true);
        dcString *marshalled = dcNode_marshall(root, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));

        dcGraphData_assertType(unmarshalled, NODE_GRAPH_DATA_TREE);
        dcTaffyOperator comparison;
        dcTestUtilities_assert((dcNode_compare(unmarshalled, root, &comparison)
                                == TAFFY_SUCCESS)
                               && comparison == TAFFY_EQUALS);

        dcNodeEvaluator *evaluator = dcSystem_getNodeEvaluator();
        dcNode *result = dcNodeEvaluator_evaluate(evaluator, unmarshalled);

        dcTestUtilities_assert(result != NULL);
        dcClass_assertHasTemplate(result, dcYesClass_getTemplate());

        regressString(marshalled, false);
        dcNode_free(&root, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }

    {
        dcNode *root = dcParser_parseString(matrixString,
                                            MARSHALL_TEST_FILE_NAME,
                                            true);
        dcString *marshalled = dcNode_marshall(root, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));

        dcGraphData_assertType(unmarshalled, NODE_GRAPH_DATA_TREE);
        dcTaffyOperator comparison;
        dcTestUtilities_assert((dcNode_compare(unmarshalled, root, &comparison)
                                == TAFFY_SUCCESS)
                               && comparison == TAFFY_EQUALS);

        dcNode *matrixNode = dcGraphDataTree_getContents(unmarshalled);
        dcMatrix *matrix = dcMatrixClass_getMatrix(matrixNode);
        assert(matrix->rowCount == 2);
        assert(matrix->columnCount == 4);

        dcNode *container = dcClass_getSuperNode(matrixNode);
        assert(CAST_CLASS_AUX(container) != NULL);

        regressString(marshalled, false);
        dcNode_free(&root, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }
}

static void testMethodCall(void)
{
    // NULL arguments
    {
        const char *baseName = "testMethodCall";
        dcNode *methodCall =
            dcMethodCall_createNode(sStringObject1, baseName, NULL);
        dcString *marshalled = dcNode_marshall(methodCall, NULL);
        dcTestUtilities_assert(marshalled != NULL);

        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        dcGraphData_assertType(unmarshalled, NODE_METHOD_CALL);

        dcNode *receiver = dcMethodCall_getReceiver(unmarshalled);
        const char *methodName = dcMethodCall_getMethodName(unmarshalled);
        dcList *arguments = dcMethodCall_getArguments(unmarshalled);

        dcClass_assertHasTemplate(receiver, dcStringClass_getTemplate());
        dcTestUtilities_assert(strcmp(methodName, baseName) == 0);
        dcTestUtilities_assert(arguments->size == 0);

        regressString(marshalled, false);
        dcNode_free(&methodCall, DC_SHALLOW);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }

    // empty arguments
    {
        const char *baseName = "testMethodCall";
        dcList *arguments = dcList_create();
        dcNode *methodCall =
            dcMethodCall_createNode(sStringObject1, baseName, arguments);
        dcString *marshalled = dcNode_marshall(methodCall, NULL);
        dcTestUtilities_assert(marshalled != NULL);

        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        dcGraphData_assertType(unmarshalled, NODE_METHOD_CALL);

        dcNode *receiver = dcMethodCall_getReceiver(unmarshalled);
        const char *methodName = dcMethodCall_getMethodName(unmarshalled);
        arguments = dcMethodCall_getArguments(unmarshalled);

        dcClass_assertHasTemplate(receiver, dcStringClass_getTemplate());
        assert(strcmp(methodName, baseName) == 0);
        assert(arguments->size == 0);

        regressString(marshalled, false);
        dcNode_free(&methodCall, DC_SHALLOW);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }

    // populated arguments
    {
        const char *baseName = "testMethodCall";
        dcNode *identifier = dcIdentifier_createNode("fooyou", NO_FLAGS);
        dcList *baseArguments = dcList_createWithObjects
            (identifier, NULL);
        dcNode *methodCall =
            dcMethodCall_createNode(sStringObject1, baseName, baseArguments);
        dcString *marshalled = dcNode_marshall(methodCall, NULL);
        dcTestUtilities_assert(marshalled != NULL);

        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        dcGraphData_assertType(unmarshalled, NODE_METHOD_CALL);

        dcNode *receiver = dcMethodCall_getReceiver(unmarshalled);
        const char *methodName = dcMethodCall_getMethodName(unmarshalled);
        dcList *arguments = dcMethodCall_getArguments(unmarshalled);

        dcClass_assertHasTemplate(receiver, dcStringClass_getTemplate());
        dcTestUtilities_assert(strcmp(methodName, baseName) == 0);
        dcTestUtilities_assert(arguments != NULL);

        regressString(marshalled, false);
        dcNode_free(&methodCall, DC_SHALLOW);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
        dcNode_free(&identifier, DC_DEEP);
    }
}

static void testMethodHeader(void)
{
    const char *baseName = "testMethodHeader";

    // no arguments
    {
        dcNode *methodHeader = dcMethodHeader_createNode(baseName, NULL);
        dcString *marshalled = dcNode_marshall(methodHeader, NULL);
        dcTestUtilities_assert(marshalled != NULL);

        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        dcGraphData_assertType(unmarshalled, NODE_METHOD_HEADER);
        const char *methodName = dcMethodHeader_getName(unmarshalled);
        dcTestUtilities_assert(strcmp(baseName, methodName) == 0);

        dcNode_free(&methodHeader, DC_SHALLOW);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }

    // arguments
    {
        dcList *arguments = dcList_createWithObjects(sNilObject1, NULL);
        dcNode *methodHeader = dcMethodHeader_createNode(baseName, arguments);
        dcString *marshalled = dcNode_marshall(methodHeader, NULL);
        dcTestUtilities_assert(marshalled != NULL);

        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        dcGraphData_assertType(unmarshalled, NODE_METHOD_HEADER);
        const char *methodName = dcMethodHeader_getName(unmarshalled);
        dcTestUtilities_assert(strcmp(baseName, methodName) == 0);

        dcList *unmarshalledArguments =
            dcMethodHeader_getArguments(CAST_METHOD_HEADER(unmarshalled));

        dcTestUtilities_assert(unmarshalledArguments != NULL);
        dcTestUtilities_assert(unmarshalledArguments->size == 1);

        regressString(marshalled, false);
        dcNode_free(&methodHeader, DC_SHALLOW);
        dcList_free(&arguments, DC_SHALLOW);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }
}

static void testNew(void)
{
    dcNode *node = dcNew_createNode
        (dcStringClass_createObject("testNew", true));
    dcString *marshalled = dcNode_marshall(node, NULL);
    dcNode *unmarshalled = dcNode_unmarshall(dcString_resetIndex(marshalled));

    dcGraphData_assertType(unmarshalled, NODE_NEW);

    dcNode_free(&node, DC_DEEP);
    dcNode_free(&unmarshalled, DC_DEEP);
    dcString_free(&marshalled, DC_DEEP);
}

static void testNil(void)
{
    testGraphDataZero(&dcNil_createNode);
}

static void testNull(void)
{
    dcString *nullString = dcNode_marshall(NULL, NULL);
    dcError_assert(dcNode_unmarshall(nullString) == NULL);
    dcString_free(&nullString, DC_DEEP);
}

static void testNotEqualCall(void)
{
    testGraphDataNode(&dcNotEqualCall_createNode, sStringObject1);
}

static void testComplexNumber(void)
{
    struct
    {
        double real;
        double imaginary;
    } tests[] =
    {
        {0, 0},
        {1, 1},
        {-1, -1},
        {1, 2},
        {2, 3}
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcComplexNumber *number = dcComplexNumber_createFromDoubles
            (tests[i].real, tests[i].imaginary);
        dcString *marshalled = dcComplexNumber_marshall(number, NULL);
        dcComplexNumber *unmarshalled =
            dcComplexNumber_unmarshall(dcString_resetIndex(marshalled));
        dcTestUtilities_assert(dcComplexNumber_equals(number, unmarshalled));
        dcComplexNumber_free(&number, DC_DEEP);
        dcComplexNumber_free(&unmarshalled, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
    }
}

static void testNumberObject(void)
{
    struct
    {
        int32_t value;
    } tests [] =
    {
        {0},
        {1},
        {100},
        {-1},
        {-100}
    };

    size_t i;

    for (i = 0; i < dcTaffy_countOf(tests); i++)
    {
        dcNode *object = dcNumberClass_createObjectFromInt32u(tests[i].value);
        dcNode_setTemplate(object, true);
        dcString *marshalled = dcNode_marshall(object, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        dcNode_assertEqual(object, unmarshalled);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&object, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }
}

static const char *simpleNumberProgram =
    "a = 1;"
    "a += 3;"
    "a += 0.1";

static void testNumberProgram(void)
{
    dcNode *root = dcParser_parseString(simpleNumberProgram,
                                        MARSHALL_TEST_FILE_NAME,
                                        true);
    dcTestUtilities_assert(root != NULL);
    dcString *marshalled = dcGraphData_marshallTree(root, NULL);
    dcNode *unmarshalled =
        dcGraphData_unmarshallTree(dcString_resetIndex(marshalled));

    dcNodeEvaluator *evaluator = dcSystem_getNodeEvaluator();
    dcNode *result = dcNodeEvaluator_evaluate(evaluator, unmarshalled);

    dcTestUtilities_assert(result != NULL);
    dcClass_assertHasTemplate(result, dcNumberClass_getTemplate());

    dcNumber *number = dcNumber_createFromDouble(4.1);
    dcNumber *resultNumber = dcNumberClass_getNumber(result);

    dcTestUtilities_assert(dcNumber_equals(number, resultNumber));

    regressString(marshalled, false);
    dcNumber_free(&number, DC_DEEP);
    dcNode_free(&root, DC_DEEP);
    dcString_free(&marshalled, DC_DEEP);
    dcNode_free(&unmarshalled, DC_DEEP);
}

static void testOr(void)
{
    testGraphDataPair(&dcOr_createNode, sNilObject1, sStringObject1);
}

static void testPair(void)
{
    dcNode *pair = dcPair_createNode(sNilObject1, sStringObject1);
    dcString *pairMarshalled = dcNode_marshall(pair, NULL);
    dcNode *pairNodeUnmarshalled =
        dcNode_unmarshall(dcString_resetIndex(pairMarshalled));

    dcTestUtilities_assert(pairNodeUnmarshalled != NULL);
    dcNode_assertType(pairNodeUnmarshalled, NODE_PAIR);

    dcClass_assertHasTemplate(dcPair_getLeft(pairNodeUnmarshalled),
                              dcNilClass_getTemplate());
    dcClass_assertHasTemplate(dcPair_getRight(pairNodeUnmarshalled),
                              dcStringClass_getTemplate());

    regressString(pairMarshalled, false);
    dcNode_free(&pair, DC_SHALLOW);
    dcString_free(&pairMarshalled, DC_DEEP);
    dcNode_free(&pairNodeUnmarshalled, DC_DEEP);
}

static void testPairObject(void)
{
    dcNode *pairObject =
        dcNode_setTemplate(dcPairClass_createObject(sNilObject1,
                                                    sStringObject1,
                                                    true),
                           true);

    dcString *pairMarshalled = dcNode_marshall(pairObject, NULL);

    dcTestUtilities_assert(pairMarshalled != NULL);

    dcNode *unmarshalled =
        dcNode_unmarshall(dcString_resetIndex(pairMarshalled));
    dcClass_assertHasTemplate(unmarshalled, dcPairClass_getTemplate());

    dcNode *left = dcPairClass_getLeft(unmarshalled);
    dcNode *right = dcPairClass_getRight(unmarshalled);

    dcClass_assertHasTemplate(left, dcNilClass_getTemplate());
    dcClass_assertHasTemplate(right, dcStringClass_getTemplate());

    dcNode_register(unmarshalled);
    regressString(pairMarshalled, true);

    dcNode_free(&pairObject, DC_DEEP);
    dcString_free(&pairMarshalled, DC_DEEP);
}

static void testReturn(void)
{
    dcNode *identifier = dcIdentifier_createNode("testReturn", NO_FLAGS);
    testGraphDataNode(&dcReturn_createNode, identifier);
    dcNode_free(&identifier, DC_DEEP);
}

static void testBlankScope(void)
{
    // with dcScope_marshall() and dcScope_unmarshall()
    {
        dcScope *scope = dcScope_create();
        dcString *marshalled = dcScope_marshall(scope, NULL);
        dcScope *unmarshalled =
            dcScope_unmarshall(dcString_resetIndex(marshalled));

        dcTestUtilities_assert(unmarshalled != NULL);
        dcHash *objects = unmarshalled->objects;

        dcTestUtilities_assert(objects != NULL && objects->size == 0);
        regressScopeString(marshalled);
        dcScope_free(&scope, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcScope_free(&unmarshalled, DC_DEEP);
    }

    // with dcMarshaller_marshall() and dcMarshaller_unmarshall()
    {
        dcScope *scope = dcScope_create();
        dcString *marshalled = dcMarshaller_marshall(NULL, "S", scope);
        dcScope *unmarshalled;
        dcError_assert(dcMarshaller_unmarshall(dcString_resetIndex(marshalled),
                                               "S",
                                               &unmarshalled));
        dcTestUtilities_assert(unmarshalled != NULL);
        dcHash *objects = unmarshalled->objects;

        dcTestUtilities_assert(objects != NULL && objects->size == 0);
        regressScopeString(marshalled);
        dcScope_free(&scope, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcScope_free(&unmarshalled, DC_DEEP);
    }
}

static void testNonBlankScope(void)
{
    // one object
    {
        dcNode *one = dcNumberClass_createObjectFromInt32s(1);
        dcScope *scope = dcScope_create();
        dcScope_setObject(scope, one, "one", NO_FLAGS);
        dcString *marshalled = dcScope_marshall(scope, NULL);
        dcScope *unmarshalled =
            dcScope_unmarshall(dcString_resetIndex(marshalled));

        dcTestUtilities_assert(unmarshalled != NULL);
        dcHash *objects = unmarshalled->objects;

        dcTestUtilities_assert(objects != NULL);
        dcTestUtilities_assert(objects->size == 1);

        regressScopeString(marshalled);
        dcScope_free(&scope, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcScope_free(&unmarshalled, DC_DEEP);
    }

    // two objects
    {
        dcNode *one = dcNumberClass_createObjectFromInt32s(1);
        dcNode *two = dcNumberClass_createObjectFromInt32s(2);
        dcScope *scope = dcScope_create();
        dcScope_setObject(scope, one, "one", NO_FLAGS);
        dcScope_setObject(scope, two, "two", NO_FLAGS);
        dcString *marshalled = dcMarshaller_marshall(NULL, "S", scope);
        dcScope *unmarshalled;
        dcError_assert(dcMarshaller_unmarshall(dcString_resetIndex(marshalled),
                                               "S",
                                               &unmarshalled));
        dcTestUtilities_assert(unmarshalled != NULL);
        dcHash *objects = unmarshalled->objects;

        dcTestUtilities_assert(objects != NULL);
        dcTestUtilities_assert(objects->size == 2);

        regressScopeString(marshalled);
        dcScope_free(&scope, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcScope_free(&unmarshalled, DC_DEEP);
    }
}

static void testSelf(void)
{
    testGraphDataZero(&dcSelf_createNode);
}

static void testString(void)
{
    // node marshall
    {
        const char *contents = "testString";
        dcNode *string = dcString_createNodeWithString(contents, true);
        dcString *stringMarshalled = dcNode_marshall(string, NULL);
        dcNode *stringUnmarshalled =
            dcNode_unmarshall(dcString_resetIndex(stringMarshalled));

        dcNode_assertType(stringUnmarshalled, NODE_STRING);

        dcTestUtilities_assert
            (memcmp(dcString_getString(stringUnmarshalled),
                    contents,
                    strlen(contents))
             == 0);

        dcNode_free(&string, DC_DEEP);
        dcString_free(&stringMarshalled, DC_DEEP);
        dcNode_free(&stringUnmarshalled, DC_DEEP);
    }

    // marshaller
    {
        const char *contents = "hi there";
        dcString *string = dcString_createWithString(contents, true);

        dcString *marshalled = dcMarshaller_marshall(NULL, "X", string);
        dcString *unmarshalled = NULL;

        dcTestUtilities_assert(marshalled != NULL
                               && (dcMarshaller_unmarshall
                                   (dcString_resetIndex(marshalled),
                                    "X",
                                    &unmarshalled)));

        dcTestUtilities_assert(strcmp(unmarshalled->string, contents) == 0);

        dcString_free(&string, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcString_free(&unmarshalled, DC_DEEP);
    }
}

static const char *stringProgram = "\"1+1 is #[1 + 1]\"";

static void testStringObject(void)
{
    {
        const char *contents = "testStringObject";
        dcNode *sStringObject = dcNode_setTemplate
            (dcStringClass_createNode(contents, true, true), true);
        dcString *stringMarshalled = dcNode_marshall(sStringObject, NULL);
        dcNode *stringUnmarshalled =
            dcNode_unmarshall(dcString_resetIndex(stringMarshalled));

        dcTestUtilities_assert(stringUnmarshalled != NULL);
        dcClass_assertHasTemplate(stringUnmarshalled,
                                  dcStringClass_getTemplate());
        dcTestUtilities_assert
            (memcmp(dcStringClass_getString(stringUnmarshalled),
                    contents,
                    strlen(contents))
             == 0);

        regressString(stringMarshalled, false);
        dcNode_free(&sStringObject, DC_DEEP);
        dcString_free(&stringMarshalled, DC_DEEP);
        dcNode_free(&stringUnmarshalled, DC_DEEP);
    }

    {
        dcNode *stringRoot = dcParser_parseString(stringProgram,
                                                  MARSHALL_TEST_FILE_NAME,
                                                  true);
        dcTestUtilities_assert(stringRoot != NULL);

        dcString *marshalled = dcNode_marshall(stringRoot, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));

        dcNode *contents = dcGraphDataTree_getContents(unmarshalled);
        dcClass_assertHasTemplate(contents, dcStringClass_getTemplate());

        dcList *objects = dcStringClass_getObjects(contents);
        dcTestUtilities_assert(objects != NULL);
        dcTestUtilities_assert(objects->size == 2);

        dcNode *left = dcList_get(objects, 1);
        dcGraphData_assertType(left, NODE_FLAT_ARITHMETIC);

        regressString(marshalled, false);
        dcNode_free(&stringRoot, DC_DEEP);
        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
    }
}

static void testSuper(void)
{
    testGraphDataZero(&dcSuper_createNode);
}

static void testSymbol(void)
{
    const char *contents = "testSymbol";
    dcNode *symbol =
        dcNode_setTemplate(dcSymbolClass_createObject(contents), true);
    dcString *marshalled = dcNode_marshall(symbol, NULL);
    dcNode *unmarshalled = dcNode_unmarshall(dcString_resetIndex(marshalled));

    dcClass_assertHasTemplate(unmarshalled, dcSymbolClass_getTemplate());
    dcTestUtilities_assert
        (dcSymbolClass_compareToString
         (unmarshalled, contents));

    dcNode_free(&symbol, DC_DEEP);
    dcString_free(&marshalled, DC_DEEP);
    dcNode_free(&unmarshalled, DC_DEEP);
}

const char *taffyClassProgram =
    "class Test             "
    "{                      "
    "  (@) foo              "
    "  {                    "
    "    io put: 1          "
    "  }                    "
    "};                     ";

const char *taffyClassWithVariablesProgram =    \
    "class Test             "
    "{                      "
    "  @x, @rw            \n"
    "  @y, @rw            \n"
    "                       "
    "  (@) foo              "
    "  {                    "
    "    io put: 1          "
    "  }                    "
    "};                     ";

static void testThrow(void)
{
    testGraphDataNode(&dcThrow_createNode, sStringObject1);
}

static void testTrue(void)
{
    testGraphDataZero(&dcTrue_createNode);
}

const char *tryCatchProgram =                   \
    "try                   "
    "{                     "
    "  a = 1               "
    "}                     "
    "catch (Exception e)   "
    "{                     "
    "  io put: \"oh dear\" "
    "}                     ";

static void testTryCatch(void)
{
    dcNode *tryCatchRoot = dcParser_parseString(tryCatchProgram,
                                                MARSHALL_TEST_FILE_NAME,
                                                true);
    dcTestUtilities_assert(tryCatchRoot != NULL);

    dcString *marshalled = dcNode_marshall(tryCatchRoot, NULL);
    dcNode *unmarshalled = dcNode_unmarshall(dcString_resetIndex(marshalled));

    dcGraphData_assertType(unmarshalled, NODE_GRAPH_DATA_TREE);
    dcNode *contents = dcGraphDataTree_getContents(unmarshalled);
    dcGraphData_assertType(contents, NODE_TRY_BLOCK);

    regressString(marshalled, false);
    dcNode_free(&tryCatchRoot, DC_DEEP);
    dcString_free(&marshalled, DC_DEEP);
    dcNode_free(&unmarshalled, DC_DEEP);
}

const char *simpleWhileProgram = "while ( a == 1 ) { a and b; a or c }";

static void testWhile(void)
{
    dcNode *whileRoot = dcParser_parseString(simpleWhileProgram,
                                             MARSHALL_TEST_FILE_NAME,
                                             true);
    dcTestUtilities_assert(whileRoot != NULL);

    dcString *marshalled = dcNode_marshall(whileRoot, NULL);
    dcNode *unmarshalled = dcNode_unmarshall(dcString_resetIndex(marshalled));

    dcGraphData_assertType(unmarshalled, NODE_GRAPH_DATA_TREE);
    dcNode *contents = dcGraphDataTree_getContents(unmarshalled);
    dcGraphData_assertType(contents, NODE_WHILE);

    dcNode *statementIt = dcWhile_getStatement(contents);
    dcGraphData_assertType(statementIt, NODE_AND);
    statementIt = dcGraphData_getNext(statementIt);
    dcGraphData_assertType(statementIt, NODE_OR);

    dcNode_free(&unmarshalled, DC_DEEP);
    dcString_free(&marshalled, DC_DEEP);
    dcNode_free(&whileRoot, DC_DEEP);
}

static void testFlatArithmetic(void)
{
    const char *programs[] =
        {
            "1 + x",
            "1 + x + y",
            "2 * x",
            "2 * x * y",
            "3 / x",
            "4 ^ x",
            "a | b",
            "a & b",
            "a << b",
            "a >> b",
            "a % b"
        };

    uint32_t i;

    for (i = 0; i < dcTaffy_countOf(programs); i++)
    {
        dcNode *head = dcParser_parseString(programs[i],
                                            MARSHALL_TEST_FILE_NAME,
                                            true);
        dcNode *flatArithmetic = dcGraphDataTree_getContents(head);
        dcTestUtilities_assert(IS_FLAT_ARITHMETIC(flatArithmetic));

        dcString *marshalled = dcNode_marshall(flatArithmetic, NULL);
        dcNode *unmarshalled =
            dcNode_unmarshall(dcString_resetIndex(marshalled));
        dcGraphData_assertType(unmarshalled, NODE_FLAT_ARITHMETIC);

        dcTestUtilities_assert(dcNode_easyCompare(unmarshalled, flatArithmetic)
                               == TAFFY_EQUALS);

        dcString_free(&marshalled, DC_DEEP);
        dcNode_free(&unmarshalled, DC_DEEP);
        dcNode_free(&head, DC_DEEP);
    }
}

static const dcTestFunctionMap sTests[] =
{
    {"Unsigned Int32",             &testUnsignedInt32},
    {"Int32",                      &testInt32},
    {"Float",                      &testFloat},
    {"List",                       &testList},
    {"Flat Arithmetic",            &testFlatArithmetic},
    {"Array Object",               &testArrayObject},
    {"Identifier",                 &testIdentifier},
    {"Assignment",                 &testAssignment},
    {"Graph Data Tree",            &testGraphDataTree},
    {"String",                     &testString},
    {"String Object",              &testStringObject},
    {"Array",                      &testArray},
    {"Block Object",               &testBlockObject},
    {"Break",                      &testBreak},
    {"Complex Number",             &testComplexNumber},
    {"Exit",                       &testExit},
    {"False",                      &testFalse},
    {"Function Object",            &testFunctionObject},
    {"Future Object",              &testFutureObject},
    {"Hash",                       &testHash},
    {"Hash Object",                &testHashObject},
    {"If",                         &testIf},
    {"List Object",                &testListObject},
    {"Matrix",                     &testMatrix},
    {"Method Call",                &testMethodCall},
    {"Method Header",              &testMethodHeader},
    {"New",                        &testNew},
    {"Nil",                        &testNil},
    {"NULL",                       &testNull},
    {"Not Equal Call",             &testNotEqualCall},
    {"Number Object",              &testNumberObject},
    {"Number Program",             &testNumberProgram},
    {"Or",                         &testOr},
    {"Pair",                       &testPair},
    {"Pair Object",                &testPairObject},
    {"Return",                     &testReturn},
    {"Blank Scope",                &testBlankScope},
    {"Non-Blank Scope",            &testNonBlankScope},
    {"Self",                       &testSelf},
    {"Super",                      &testSuper},
    {"Symbol",                     &testSymbol},
    {"Throw",                      &testThrow},
    {"True",                       &testTrue},
    {"Try/Catch",                  &testTryCatch},
    {"While",                      &testWhile},
    {NULL,                         NULL}
};

static void marker(void)
{
    dcNode_mark(sNilObject1);
    dcNode_mark(sNilObject2);
    dcNode_mark(sStringObject1);
    dcNode_mark(sIdentifier1);
    dcNode_mark(sIdentifier2);
}

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_create();
    dcTestUtilities_start("Marshall Test", _argc, _argv);

    // initialize the globals //
    sNilObject1 = dcNilClass_createObject();
    sNilObject2 = dcNilClass_createObject();
    sStringObject1 = dcStringClass_createObject("sStringObject1", true);
    sIdentifier1 = dcIdentifier_createNode("identifier1", SCOPE_DATA_INSTANCE);
    sIdentifier2 = dcIdentifier_createNode("identifier2", SCOPE_DATA_META);

    dcGarbageCollector_addRoot(marker);
    dcNode_register(sNilObject1);
    dcNode_register(sNilObject2);
    dcNode_register(sStringObject1);
    dcNode_register(sIdentifier1);
    dcNode_register(sIdentifier2);

    dcTestUtilities_runTests(NULL, sTests, true);

    dcTestUtilities_end();
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
