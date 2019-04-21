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

#include "dcFunctionUpdate.h"
#include "dcGraphData.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcFunctionUpdate *dcFunctionUpdate_create(dcNode *_identifier,
                                          dcList *_arguments,
                                          dcNode *_arithmetic)
{
    dcFunctionUpdate *functionUpdate =
        (dcFunctionUpdate *)dcMemory_allocate(sizeof(dcFunctionUpdate));
    functionUpdate->identifier = _identifier;
    functionUpdate->arguments = _arguments;
    functionUpdate->arithmetic = _arithmetic;
    return functionUpdate;
}

dcNode *dcFunctionUpdate_createNode(dcNode *_identifier,
                                    dcList *_arguments,
                                    dcNode *_arithmetic)
{
    return dcGraphData_createNodeWithGuts
        (NODE_FUNCTION_UPDATE,
         dcFunctionUpdate_create(_identifier, _arguments, _arithmetic));
}

void dcFunctionUpdate_freeNode(dcNode *_node, dcDepth _depth)
{
    dcFunctionUpdate *functionUpdate = CAST_FUNCTION_UPDATE(_node);
    dcNode_free(&functionUpdate->identifier, DC_DEEP);
    dcList_free(&functionUpdate->arguments, DC_DEEP);
    dcNode_free(&functionUpdate->arithmetic, DC_DEEP);
    dcMemory_free(functionUpdate);
}

dcFunctionUpdate *dcFunctionUpdate_copy(dcFunctionUpdate *_from,
                                        dcDepth _depth)
{
    return dcFunctionUpdate_create(dcNode_copy(_from->identifier, DC_DEEP),
                                   dcList_copy(_from->arguments, DC_DEEP),
                                   dcNode_copy(_from->arithmetic, DC_DEEP));
}

void dcFunctionUpdate_copyNode(dcNode *_to,
                               const dcNode *_from,
                               dcDepth _depth)
{
    dcFunctionUpdate *functionUpdate = CAST_FUNCTION_UPDATE(_from);
    CAST_FUNCTION_UPDATE(_to) = dcFunctionUpdate_copy(functionUpdate, _depth);
}

dcNode *dcFunctionUpdate_getIdentifier(const dcNode *_functionUpdate)
{
    return CAST_FUNCTION_UPDATE(_functionUpdate)->identifier;
}

dcList *dcFunctionUpdate_getArguments(const dcNode *_functionUpdate)
{
    return CAST_FUNCTION_UPDATE(_functionUpdate)->arguments;
}

dcNode *dcFunctionUpdate_getArithmetic(const dcNode *_functionUpdate)
{
    return CAST_FUNCTION_UPDATE(_functionUpdate)->arithmetic;
}

dcString *dcFunctionUpdate_marshallNode(const dcNode *_functionUpdate,
                                        dcString *_stream)
{
    return dcMarshaller_marshall
        (_stream,
         "nln",
         dcFunctionUpdate_getIdentifier(_functionUpdate),
         dcFunctionUpdate_getArguments(_functionUpdate),
         dcFunctionUpdate_getArithmetic(_functionUpdate));
}

bool dcFunctionUpdate_unmarshallNode(dcNode *_node, dcString *_stream)
{
    dcNode *identifier = NULL;
    dcList *arguments = NULL;
    dcNode *arithmetic = NULL;
    bool result = dcMarshaller_unmarshallNoNull(_stream,
                                                "nln",
                                                &identifier,
                                                &arguments,
                                                &arithmetic);
    if (result)
    {
        CAST_FUNCTION_UPDATE(_node) =
            dcFunctionUpdate_create(identifier, arguments, arithmetic);
    }

    return result;
}
