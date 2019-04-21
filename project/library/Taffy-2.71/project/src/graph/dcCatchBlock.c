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

#include "dcCatchBlock.h"
#include "dcGraphData.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcCatchBlock *dcCatchBlock_create(dcNode *_identifier,
                                  dcNode *_type,
                                  dcNode *_statement)
{
    dcCatchBlock *blockData =
        (dcCatchBlock *)dcMemory_allocate(sizeof(dcCatchBlock));
    blockData->identifier = _identifier;
    blockData->type = _type;
    blockData->statement = _statement;
    return blockData;
}

dcNode *dcCatchBlock_createNode(dcNode *_identifier,
                                dcNode *_type,
                                dcNode *_statement)
{
    dcNode *catchBlockNode = dcGraphData_createNode(NODE_CATCH_BLOCK);
    CAST_CATCHBLOCK(catchBlockNode) =
        dcCatchBlock_create(_identifier, _type, _statement);
    return catchBlockNode;
}

void dcCatchBlock_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcCatchBlock *toBlock =
        (dcCatchBlock *)dcMemory_allocate(sizeof(dcCatchBlock));
    dcCatchBlock *fromBlock = CAST_CATCHBLOCK(_from);
    toBlock->identifier = dcNode_copy(fromBlock->identifier, _depth);
    toBlock->type = dcNode_copy(fromBlock->type, _depth);
    toBlock->statement = dcGraphData_copyTree(fromBlock->statement);
    CAST_CATCHBLOCK(_to) = toBlock;
}

void dcCatchBlock_freeNode(dcNode *_node, dcDepth _depth)
{
    dcCatchBlock *data = CAST_CATCHBLOCK(_node);

    if (_depth == DC_DEEP)
    {
        dcNode_free(&data->identifier, _depth);
        dcNode_free(&data->type, _depth);
        dcNode_free(&data->statement, _depth);
    }

    dcMemory_free(data);
}

dcString *dcCatchBlock_marshallNode(const dcNode *_node, dcString *_stream)
{
    dcCatchBlock *catchBlock = CAST_CATCHBLOCK(_node);
    return dcMarshaller_marshall(_stream,
                                 "nnt",
                                 catchBlock->identifier,
                                 catchBlock->type,
                                 catchBlock->statement);
}

bool dcCatchBlock_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcNode *identifier = NULL;
    dcNode *type = NULL;
    dcNode *statement = NULL;

    if ((result = dcMarshaller_unmarshallNoNull(_stream,
                                                "nnt",
                                                &identifier,
                                                &type,
                                                &statement)))
    {
        CAST_CATCHBLOCK(_node) =
            dcCatchBlock_create(identifier, type, statement);
    }

    return result;
}
