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

#include "dcTryBlock.h"
#include "dcGraphData.h"
#include "dcList.h"
#include "dcMarshaller.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcString.h"

dcTryBlock *dcTryBlock_create(dcNode *_statement, dcList *_catches)
{
    dcTryBlock *exceptionBlock =
        (dcTryBlock *)dcMemory_allocate(sizeof(dcTryBlock));
    exceptionBlock->statement = _statement;
    exceptionBlock->catches = _catches;
    return exceptionBlock;
}

dcNode *dcTryBlock_createNode(dcNode *_statement, dcList *_catches)
{
    dcNode *exceptionBlockNode = dcGraphData_createNode(NODE_TRY_BLOCK);
    CAST_TRYBLOCK(exceptionBlockNode) = dcTryBlock_create(_statement, _catches);
    return exceptionBlockNode;
}

void dcTryBlock_freeNode(dcNode *_node, dcDepth _depth)
{
    dcTryBlock *data = CAST_TRYBLOCK(_node);

    if (_depth == DC_DEEP)
    {
        dcNode_free(&data->statement, _depth);
        dcList_free(&data->catches, _depth);
    }

    dcMemory_free(data);
}

void dcTryBlock_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    dcTryBlock *fromBlock = CAST_TRYBLOCK(_from);
    dcTryBlock *toBlock = (dcTryBlock *)dcMemory_allocate(sizeof(dcTryBlock));
    toBlock->statement = dcGraphData_copyTree(fromBlock->statement);
    toBlock->catches = dcList_copy(fromBlock->catches, _depth);
    CAST_TRYBLOCK(_to) = toBlock;
}

dcNode *dcTryBlock_getStatement(const dcNode *_tryBlock)
{
    return CAST_TRYBLOCK(_tryBlock)->statement;
}

dcList *dcTryBlock_getCatches(const dcNode *_tryBlock)
{
    return CAST_TRYBLOCK(_tryBlock)->catches;
}

dcString *dcTryBlock_marshallNode(const dcNode *_tryBlock, dcString *_stream)
{
    return dcMarshaller_marshall(_stream,
                                 "nl",
                                 dcTryBlock_getStatement(_tryBlock),
                                 dcTryBlock_getCatches(_tryBlock));
}

bool dcTryBlock_unmarshallNode(dcNode *_node, dcString *_stream)
{
    bool result = false;
    dcNode *statement;
    dcList *catches;

    if (dcMarshaller_unmarshallNoNull(_stream,
                                      "nl",
                                      &statement,
                                      &catches))
    {
        result = true;
        CAST_TRYBLOCK(_node) = dcTryBlock_create(statement, catches);
    }

    return result;
}
