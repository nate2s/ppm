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

#include "dcCallStackData.h"
#include "dcError.h"
#include "dcLexer.h"
#include "dcNode.h"
#include "dcString.h"
#include "dcStringManager.h"
#include "dcMemory.h"

dcCallStackData *dcCallStackData_create(const char *_methodName,
                                        dcStringId _filenameId,
                                        uint32_t _lineNumber)
{
    dcCallStackData *callStackData =
        (dcCallStackData *)dcMemory_allocate(sizeof(dcCallStackData));
    callStackData->lineNumber = _lineNumber;
    TAFFY_DEBUG(dcError_assert(_filenameId != 0));
    callStackData->filenameId = _filenameId;
    callStackData->methodName = (_methodName != NULL
                                 ? dcMemory_strdup(_methodName)
                                 : NULL);
    return callStackData;
}

dcNode *dcCallStackData_createNode(const char *_methodName,
                                   dcStringId _filenameId,
                                   uint32_t _line)
{
    return dcNode_createWithGuts(NODE_CALL_STACK_DATA,
                                 dcCallStackData_create(_methodName,
                                                        _filenameId,
                                                        _line));
}

bool dcCallStackData_equals(const dcCallStackData *_left,
                            const dcCallStackData *_right)
{
    return (_left->lineNumber == _right->lineNumber
            && _left->filenameId == _right->filenameId
            && ((_left->methodName == NULL
                 && _right->methodName == NULL)
                || (_left->methodName != NULL
                    && _right->methodName != NULL
                    && (strcmp(_left->methodName, _right->methodName) == 0))));
}

void dcCallStackData_freeNode(dcNode *_node, dcDepth _depth)
{
    dcCallStackData *data = CAST_CALL_STACK_DATA(_node);
    dcMemory_free(data->methodName);
    dcMemory_free(data);
}

void dcCallStackData_copyNode(dcNode *_to,
                              const dcNode *_from,
                              dcDepth _depth)
{
    dcCallStackData *from = CAST_CALL_STACK_DATA(_from);
    CAST_CALL_STACK_DATA(_to) = dcCallStackData_create
        (from->methodName, from->filenameId, from->lineNumber);
}

dcResult dcCallStackData_print(const dcCallStackData *_callStackData,
                               dcString *_stream)
{
    char *methodOut = NULL;

    //
    // preliminary method and filename formatting
    //
    if (_callStackData->methodName != NULL
        && strlen(_callStackData->methodName) > 0)
    {
        methodOut = dcLexer_sprintf("%s, ", _callStackData->methodName);
    }
    else
    {
        methodOut = dcLexer_sprintf("");
    }

    char *filenameOut = dcLexer_sprintf
        ("%s: ", dcStringManager_getStringFromId(_callStackData->filenameId));

    //
    // create the string
    //
    char *display = dcLexer_sprintf("    near %s%sline " SIZE_T_PRINTF,
                                    filenameOut,
                                    methodOut,
                                    _callStackData->lineNumber);

    //
    // append the string
    //
    dcString_appendString(_stream, display);

    dcMemory_free(methodOut);
    dcMemory_free(filenameOut);
    dcMemory_free(display);
    return TAFFY_SUCCESS;
}

// create dcCallStackData_printNode
dcTaffy_createPrintNodeFunctionMacro(dcCallStackData, CAST_CALL_STACK_DATA);

// create dcCallStackData_display
dcTaffy_createDisplayFunctionMacro(dcCallStackData);
