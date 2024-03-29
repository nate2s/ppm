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

#ifndef __DC_DEFINES_H__
#define __DC_DEFINES_H__

//
// This file contains symbols needed by Taffy header files
//

#ifdef __APPLE__
#    define LIBRARY_PREFIX "lib"
#    define LIBRARY_SUFFIX "dylib"
#    define TAFFY_APPLE true
#    define dcTaffyThreadId pthread_t
#elif __CYGWIN__
#    define LIBRARY_PREFIX "cyg"
#    define LIBRARY_SUFFIX "dll"
#    define TAFFY_CYGWIN true
#    define dcTaffyThreadId pthread_t
#elif __linux__
#    define LIBRARY_PREFIX "lib"
#    define LIBRARY_SUFFIX "so"
#    define TAFFY_LINUX true
#    define dcTaffyThreadId pthread_t
#elif (defined _WIN16) || (defined _WIN32) || (defined _WIN64)
#    define TAFFY_WINDOWS true
#    define dcTaffyThreadId DWORD
#else
#    error "Unknown platform. What's your platform? Exiting."
#endif

#ifdef TAFFY_WINDOWS
#    define false 0
#    define true 1
#    define SIZE_T_PRINTF "%Iu"
#    define TAFFY_IS_WINDOWS true
#    define DIRECTORY_SEPARATOR '\\'
#else
#    include <stdbool.h>
#    define SIZE_T_PRINTF "%zu"
#    define TAFFY_IS_WINDOWS false
#    define DIRECTORY_SEPARATOR '/'
#endif

#include <stdint.h>
// for size_t
#include <stddef.h>

// types //
typedef uint64_t dcHashType;
typedef uint32_t dcStringId;
typedef uint32_t dcContainerSizeType;

// flags //
#define NO_FLAGS 0

// compile-time configurations, see dcSystem_getConfiguration() //
typedef enum
{
    TAFFY_LAZY_INSTANTIATION = 1,
    TAFFY_HASH_LOAD_FACTOR   = 2
}  dcConfiguration;

// preliminary structs and enums //
enum dcNodeType_e
{
    NODE_NONE = 0,

    // primitives //
    NODE_INT,
    NODE_SIGNED_INT_32,
    NODE_INT_64,
    NODE_FLOAT,
    NODE_TAFFY_C_METHOD_POINTER,

    // maths //
    NODE_MATRIX,
    NODE_NUMBER,
    NODE_COMPLEX_NUMBER,

    // containers //
    NODE_ARRAY,
    NODE_HASH,
    NODE_HASH_ELEMENT,
    NODE_HEAP,
    NODE_LIST,
    NODE_PAIR,
    NODE_TREE,
    NODE_TREE_ELEMENT,

    // scope //
    NODE_SCOPE,
    NODE_SCOPE_DATA,
    NODE_OBJECT_STACK,

    // everything else //
    NODE_CALL_STACK_DATA,
    NODE_GRAPH_DATA,
    NODE_LEX_RESULT,
    NODE_SOCKET,
    NODE_STRING,
    NODE_EVALUATOR,
    NODE_ROOT_MARK_FUNCTION,

    NODE_CLASS_TEMPLATE,
    NODE_PACKAGE_CONTENTS,
    NODE_FILE_PACKAGE_DATA,
    NODE_COMMAND_LINE_ARGUMENT,

    // voids //
    NODE_CONST_VOID,
    NODE_VOID,
    NODE_VOID_CONTAINER,
    NODE_DOUBLE_VOID,

    NODE_LAST
};

typedef uint8_t dcNodeType;

#define NUMBER_OF_NODE_TYPES NODE_LAST

typedef enum
{
    NODE_AND,
    NODE_ASSIGNMENT,
    NODE_BREAK,
    NODE_CATCH_BLOCK,
    NODE_CLASS,
    NODE_EXIT,
    NODE_NO,
    NODE_FLAT_ARITHMETIC,
    NODE_FOR,
    NODE_FUNCTION_UPDATE,
    NODE_GRAPH_DATA_LIST,
    NODE_GRAPH_DATA_NODE,
    NODE_GRAPH_DATA_PAIR,
    NODE_GRAPH_DATA_TREE,
    NODE_IDENTIFIER,
    NODE_IF,
    NODE_IMPORT,
    NODE_IN,
    NODE_METHOD_CALL,
    NODE_METHOD_HEADER,
    NODE_NEW,
    NODE_NIL,
    NODE_NOT_EQUAL_CALL,
    NODE_OR,
    NODE_PACKAGE,
    NODE_RETURN,
    NODE_SELF,
    NODE_SUPER,
    NODE_SYMBOL,
    NODE_SYNCHRONIZED,
    NODE_THROW,
    NODE_TRUE,
    NODE_TRY_BLOCK,
    NODE_UP_SELF,
    NODE_WHILE,

    NODE_GRAPH_DATA_LAST
} dcGraphDataType;

#define NUMBER_OF_GRAPHDATA_TYPES NODE_GRAPH_DATA_LAST

#define dcTaffy_countOf(array) (sizeof(array) / sizeof(array[0]))

typedef enum
{
    DC_NO_DEPTH,
    DC_SPACE,
    DC_FLOATING,
    DC_SHALLOW,
    DC_DEEP
} dcDepth;

typedef enum
{
    DC_GRAPH_SNIP,
    DC_GRAPH_DERIVE,
    DC_GRAPH_INTEGRATE
} dcGraphOperation;

typedef enum
{
    TAFFY_EXCEPTION = -1,
    TAFFY_FAILURE   = 0,
    TAFFY_SUCCESS   = 1
} dcResult;

typedef enum
{
    TAFFY_ADD,
    TAFFY_SUBTRACT,
    TAFFY_MULTIPLY,
    TAFFY_DIVIDE,
    TAFFY_RAISE,
    TAFFY_MODULUS,
    TAFFY_LESS_THAN,
    TAFFY_LESS_THAN_OR_EQUAL,
    TAFFY_GREATER_THAN,
    TAFFY_GREATER_THAN_OR_EQUAL,
    TAFFY_LEFT_SHIFT,
    TAFFY_LEFT_SHIFT_EQUAL,
    TAFFY_RIGHT_SHIFT,
    TAFFY_RIGHT_SHIFT_EQUAL,
    TAFFY_AND,
    TAFFY_OR,
    TAFFY_EQUALS,
    TAFFY_PLUS_PLUS,
    TAFFY_MINUS_MINUS,
    TAFFY_BIT_AND,
    TAFFY_BIT_AND_EQUAL,
    TAFFY_BIT_OR,
    TAFFY_BIT_OR_EQUAL,
    TAFFY_BIT_XOR,
    TAFFY_BIT_XOR_EQUAL,
    TAFFY_BIT_NOT,
    TAFFY_DELTA_EQUALS,
    TAFFY_PLUS_EQUAL,
    TAFFY_MINUS_EQUAL,
    TAFFY_MULTIPLY_EQUAL,
    TAFFY_DIVIDE_EQUAL,
    TAFFY_RAISE_EQUAL,
    TAFFY_MODULUS_EQUAL,
    TAFFY_PARENTHESES,
    TAFFY_BRACKETS,
    TAFFY_BRACKETS_EQUAL,
    TAFFY_BRACKETS_DOTS,
    TAFFY_BRACKETS_DOTS_EQUAL,
    TAFFY_PREFIX_MINUS,
    TAFFY_PREFIX_PLUS,
    TAFFY_FACTORIAL,
    TAFFY_UNKNOWN_OPERATOR,
    TAFFY_LAST_OPERATOR
} dcTaffyOperator;

// predefinitions //
struct dcArray_t;
struct dcCallStackData_t;
struct dcCatch_t;
struct dcCharacterGraph_t;
struct dcClass_t;
struct dcClassTemplate_t;
struct dcComplexNumber_t;
struct dcCondition_t;
struct dcContainerData_t;
struct dcCommandLineArguments_t;
struct dcCommandLineArgument_t;
struct dcFilePackageData_t;
struct dcFlatArithmetic_t;
struct dcFor_t;
struct dcGarbageCollector_t;
struct dcGraphData_t;
struct dcGraphDatatTree_t;
struct dcHash_t;
struct dcHashElement_t;
struct dcHeap_t;
struct dcLexer_t;
struct dcLexResult_t;
struct dcList_t;
struct dcListElement_t;
struct dcListIterator_t;
struct dcMutex_t;
struct dcNode_t;
struct dcNodeEvaluator_t;
struct dcNumber_t;
struct dcMethodHeader_t;
struct dcObjectStack_t;
struct dcObjectStackList_t;
struct dcPackage_t;
struct dcPair_t;
struct dcRuntimeClassTemplate_t;
struct dcScope_t;
struct dcScopeData_t;
struct dcSocket_t;
struct dcString_t;
struct dcStringCache_t;
struct dcStringCacheElement_t;
struct dcSystem_t;
struct dcTaffyCMethodWrapper_t;
struct dcTemplateData_t;
struct dcThreadId_t;
struct dcTree_t;
struct dcTreeElement_t;

struct decNumber_t;

#define TAFFY_C_METHOD(_name_)                              \
    struct dcNode_t *_name_(struct dcNode_t *_caller,       \
                            struct dcArray_t *_arguments);

typedef struct dcNode_t *(*dcTaffyCMethodPointer)
    (struct dcNode_t *_receiver,
     struct dcArray_t *_arguments);

typedef struct dcNode_t *(*dcGraphDataMathOperation)(struct dcNode_t *_left,
                                                     struct dcNode_t *_right);

typedef const char * const dcCFunctionArgument;

//
// Standard Function defines (you will see these in many .h files)
//
#define ALLOCATE_FUNCTION(_name_)               \
    void _name_(struct dcNode_t *_node);

#define COMPARE_FUNCTION(_name_)                        \
    dcResult _name_(struct dcNode_t *_left,             \
                    struct dcNode_t *_right,            \
                    dcTaffyOperator *_compareResult);

#define COPY_FUNCTION(_name_)                   \
    void _name_(struct dcNode_t *_to,           \
                const struct dcNode_t *_from,   \
                dcDepth _depth)

#define CREATE_INSTANCE_FUNCTION(_name_)        \
    struct dcNode_t *_name_(bool _object);

#define DEALLOCATE_FUNCTION(_name_)             \
    void _name_(struct dcNode_t *_node);

#define DEINITIALIZE_FUNCTION(_name_)           \
    void _name_(void);

#define DERIVE_FUNCTION(_name_)                         \
    struct dcNode_t *_name_(struct dcNode_t *_node,     \
                            struct dcNode_t *_token,    \
                            struct dcNode_t **_result);

#define DO_GRAPH_OPERATION_FUNCTION(_name_)                 \
    struct dcNode_t *_name_(struct dcNode_t *_node,         \
                            struct dcNode_t *_token,        \
                            dcGraphOperation _operation,    \
                            struct dcNode_t **_result);

#define FREE_FUNCTION(_name_)                           \
    void _name_(struct dcNode_t *_node, dcDepth _depth)

#define GET_TEMPLATE_FUNCTION(_name_)           \
    struct dcClassTemplate_t *_name_(void)

#define HASH_FUNCTION(_name_)                                           \
    dcResult _name_(struct dcNode_t *_node, dcHashType *_hashResult)

#define INITIALIZE_FUNCTION(_name_)             \
    void _name_(void)

#define IS_ME_FUNCTION(_name_)                  \
    bool _name_(const struct dcNode_t *_node)

#define MARK_FUNCTION(_name_)                   \
    void _name_(struct dcNode_t *_node)

#define MARSHALL_FUNCTION(_name_)                           \
    struct dcString_t *_name_(const struct dcNode_t *_node, \
                              struct dcString_t *_stream)

#define UNMARSHALL_FUNCTION(_name_)                                     \
    bool _name_(struct dcNode_t *_node, struct dcString_t *_stream)

#define PRINT_FUNCTION(_name_)                      \
    dcResult _name_(const struct dcNode_t *_node,   \
                    struct dcString_t *_string)

#define PRETTY_PRINT_FUNCTION(_name_)                   \
    dcResult _name_(const struct dcNode_t *_node,       \
                    struct dcCharacterGraph_t **_graph)

#define REGISTER_FUNCTION(_name_)               \
    void _name_(struct dcNode_t *_node)

#define SET_TEMPLATE_FUNCTION(_name_)           \
    void _name_(struct dcNode_t *_node, bool _yesNo)

//
// Standard function creators (you will find these in many .c files)
//
#define dcTaffy_createCompareFunctionMacro(_type, _cast)                \
    dcResult _type##_compareNode(dcNode *_left,                         \
                                 dcNode *_right,                        \
                                 dcTaffyOperator *_compareResult)       \
    {                                                                   \
        return _type##_compare(_cast(_left), _cast(_right), _compareResult); \
    }

#define dcTaffy_createDisplayFunctionMacro(_type)       \
    const char *_type##_display(const _type *_node)     \
    {                                                   \
        dcNode *result = dcString_createNode();         \
                                                        \
        if (_node == NULL)                              \
        {                                               \
            dcString_appendString(CAST_STRING(result),  \
                                  "??NULL??");          \
        }                                               \
        else                                            \
        {                                               \
            _type##_print(_node, CAST_STRING(result));  \
        }                                               \
                                                        \
        dcNode_register(result);                        \
        return CAST_STRING(result)->string;             \
    }

#define dcTaffy_createPrintNodeFunctionMacro(_type, _cast)              \
    dcResult _type##_printNode(const dcNode *_node, dcString *_stream)  \
    {                                                                   \
        return _type##_print(_cast(_node), _stream);                    \
    }

#define dcTaffy_createPrettyPrintNodeFunctionMacro(_type, _cast)        \
    dcResult _type##_prettyPrintNode(const dcNode *_node,               \
                                     struct dcCharacterGraph_t **_graph) \
    {                                                                   \
        return _type##_prettyPrint(_cast(_node), _graph);               \
    }

#define dcTaffy_createCopyNodeFunctionMacro(_type, _cast)   \
    void _type##_copyNode(dcNode *_to,                      \
                          const dcNode *_from,              \
                          dcDepth _depth)                   \
    {                                                       \
        _cast(_to) = _type##_copy(_cast(_from), _depth);    \
    }

#define dcTaffy_createCastFunctionMacro(_type, _cast)               \
    _type *_type##_castMe(const dcNode *_node)                      \
    {                                                               \
        return _cast(_node);                                        \
    }

#define dcTaffy_createIsMeFunctionMacro(_type)                      \
    bool _type##_isMe(const dcNode *_node)                          \
    {                                                               \
        return (IS_CLASS(_node)                                     \
                && dcClass_hasTemplate(_node, sTemplate, true));    \
    }

#define dcTaffy_createFreeNodeFunctionMacro(_type, _cast)   \
    void _type##_freeNode(dcNode *_node, dcDepth _depth)    \
    {                                                       \
        _type##_free(&(_cast(_node)), _depth);              \
    }

#define dcTaffy_createMarkNodeFunctionMacro(_type, _cast)   \
    void _type##_markNode(dcNode *_node)                    \
    {                                                       \
        _type##_mark(_cast(_node));                         \
    }

#define dcTaffy_createRegisterNodeFunctionMacro(_type, _cast)   \
    void _type##_registerNode(dcNode *_node)                    \
    {                                                           \
        _type##_register(_cast(_node));                         \
    }

#define dcTaffy_createMarshallNodeFunctionMacro(_type, _cast)           \
    dcString *_type##_marshallNode(const dcNode *_node, dcString *_stream) \
    {                                                                   \
        return _type##_marshall(_cast(_node), _stream);                 \
    }

#define dcTaffy_createUnmarshallNodeFunctionMacro(_type, _cast)     \
    bool _type##_unmarshallNode(dcNode *_node, dcString *_stream) \
    {                                                               \
        _cast(_node) = _type##_unmarshall(_stream);                 \
            return (_cast(_node) != NULL);                          \
    }

#define dcTaffy_createSimpleNodeGetterMacro(_name, _object) \
    dcNode *_name(void)                                     \
    {                                                       \
        return _object;                                     \
    }

#define TAFFY_UNINITIALIZED_DIRECTION -1

typedef enum
{
    TAFFY_LEFT = 0,
    TAFFY_RIGHT = 1
} dcDirection;

//
// Typedefs for types in class templates
//

// see the enum dcClassFlags_e in src/class/dcClassTemplate.h
typedef uint8_t dcClassFlags;

// see the enum dcScopeDataFlags_e in src/scope/dcScopeData.h
typedef uint32_t dcScopeDataFlags;

typedef struct dcClassTemplate_t *(*dcTaffy_getTemplatePointer)(void);

typedef void (*dcClass_initializationPointer)(void);
typedef void (*dcClass_deinitializationPointer)(void);
typedef void (*dcClass_deallocatePointer)(struct dcNode_t *_self);
typedef void (*dcNode_metaInitPointer)(void);
typedef void (*dcNode_metaDeallocPointer)(void);

typedef void (*dcClass_allocatePointer)(struct dcNode_t *_self);
typedef void (*dcNode_freePointer)(struct dcNode_t *_node, dcDepth _depth);
typedef void (*dcNode_copyPointer)(struct dcNode_t *_to,
                                   const struct dcNode_t *_from,
                                   dcDepth _depth);
typedef char *(*dcNode_displayPointer)(const struct dcNode_t *_node);
typedef dcResult (*dcNode_printPointer)(const struct dcNode_t *_node,
                                        struct dcString_t *_string);
typedef dcResult (*dcNode_prettyPrintPointer)
    (const struct dcNode_t *_node,
     struct dcCharacterGraph_t **_graph);
typedef void (*dcTaffy_rootMarkPointer)(void);
typedef void (*dcNode_markPointer)(struct dcNode_t *_node);
typedef void (*dcNode_registerPointer)(struct dcNode_t *_node);
typedef void (*dcNode_setTemplatePointer)(struct dcNode_t *_node,
                                          bool _yesNo);

typedef struct dcString_t *(*dcNode_marshallPointer)
    (const struct dcNode_t *_node,
     struct dcString_t *_stream);
typedef bool (*dcNode_unmarshallPointer)(struct dcNode_t *_node,
                                           struct dcString_t *_stream);
typedef dcResult (*dcNode_comparePointer)(struct dcNode_t *_left,
                                          struct dcNode_t *_right,
                                          dcTaffyOperator *_compareResult);
typedef struct dcNode_t *(*dcNode_doGraphOperationPointer)
    (struct dcNode_t *_node,
     struct dcNode_t *_token,
     dcGraphOperation _operation,
     struct dcNode_t **_result);
typedef dcResult (*dcNode_hashPointer)(struct dcNode_t *_node,
                                       dcHashType *_hashResult);

#define FLAG_CHECK(_flag) (((_flag) == 0) ? false : true)

#define BITS(x) (1 << x)
#define SET_BITS(_location, _flags, _yesno)     \
    if (_yesno)                                 \
    {                                           \
        _location |= _flags;                    \
    }                                           \
    else                                        \
    {                                           \
        _location &= ~_flags;                   \
    }

#define READ_BITS(_location, _flags)            \
    ((_location & _flags) != 0)

#ifndef TAFFY_WINDOWS

    #define dcTaffy_max(aaa, bbb)                   \
        ({ __typeof__ (aaa) _aaa_ = (aaa);          \
            __typeof__ (bbb) _bbb_ = (bbb);         \
            _aaa_ > _bbb_ ? _aaa_ : _bbb_; })

    #define dcTaffy_min(aaa, bbb)                   \
        ({ __typeof__ (aaa) _aaa_ = (aaa);          \
            __typeof__ (bbb) _bbb_ = (bbb);         \
            _aaa_ < _bbb_ ? _aaa_ : _bbb_; })

#else // we have windows

    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;

    // this makes me sad
    #define dcTaffy_max(aaa, bbb) (aaa > bbb ? aaa : bbb)
    #define dcTaffy_min(aaa, bbb) (aaa < bbb ? aaa : bbb)

    #define GARBAGE_COLLECTOR_OBJECT_TIP 6000000

#endif

typedef dcResult (*dcContainerEachFunction)(struct dcNode_t *_object,
                                            struct dcNode_t *_token);

#define TAFFY_FILE_NAME_SUFFIX "ty"

#define CORE_PACKAGE_NAME "org.taffy.core"

#define CONTAINER_PACKAGE_NAME CORE_PACKAGE_NAME ".container"
#define EXCEPTION_PACKAGE_NAME CORE_PACKAGE_NAME ".exception"
#define IO_PACKAGE_NAME        CORE_PACKAGE_NAME ".io"
#define MATHS_PACKAGE_NAME     CORE_PACKAGE_NAME ".maths"
#define SPECIAL_PACKAGE_NAME   CORE_PACKAGE_NAME ".special"
#define THREADING_PACKAGE_NAME CORE_PACKAGE_NAME ".threading"

#define MAKE_FULLY_QUALIFIED(type) type##_PACKAGE_NAME "." type##_CLASS_NAME

#ifdef ENABLE_DEBUG
#    define TAFFY_DEBUG(x) x
#    define TAFFY_HIDDEN
#else
#    define TAFFY_DEBUG(x)
#    define TAFFY_HIDDEN static
#endif

#ifdef COMPILE_TESTS
#    define TAFFY_TEST(x) x
#else
#    define TAFFY_TEST(x)
#endif

typedef enum
{
    TAFFY_NUMBER_SUCCESS            = 0,
    TAFFY_NUMBER_NEED_INTEGER       = BITS(1),
    TAFFY_NUMBER_INEXACT            = BITS(2),
    TAFFY_NUMBER_OUT_OF_MEMORY      = BITS(3),
    TAFFY_NUMBER_INPUT_OUT_OF_RANGE = BITS(4),
    TAFFY_NUMBER_NOT_IMPLEMENTED    = BITS(5)
} dcNumberResult;

enum dcPieLineEvaluatorOutFlag_e
{
    PARSER_GOT_EXCEPTION      = BITS(0),
    PARSER_IS_ASSIGNMENT      = BITS(1),
    PARSER_IS_COMMENT         = BITS(2),
    PARSER_IS_FUNCTION_UPDATE = BITS(3)
};

typedef uint8_t dcPieLineEvaluatorOutFlag;

enum dcGarbageCollectLevel_e
{
    TAFFY_GARBAGE_COLLECTOR_LEVEL_1 = 1,
    TAFFY_GARBAGE_COLLECTOR_LEVEL_2 = 2,
    TAFFY_GARBAGE_COLLECTOR_LEVEL_3 = 3,

    // collect at shutdown
    TAFFY_GARBAGE_COLLECTOR_LEVEL_X = 4
};

typedef uint8_t dcGarbageCollectorLevel;

typedef void *(*dcGenericFunction)(void *_argument);

#define MIN_MARSHALL_SIZE 3

#endif
