//
// This file is part of ppm, a pretty printer for math
// Copyright (C) 2018 Nate Smith (nat2e.smith@gmail.com)
//
// ppm is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ppm is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "TaffyBridge.h"

TaffyBridge &TaffyBridge::getInstance()
{
    static TaffyBridge theInstance;
    return theInstance;
}

TaffyBridge::TaffyBridge()
{
    dcSystem_create();
}

TaffyBridge::~TaffyBridge()
{
    dcSystem_free();
}

static void *evalString(void *argument)
{
    std::string *text = (std::string *)argument;
    dcNode *result = dcParser_parseString(text->c_str(), "PPMYO", true);

    if (result != NULL
        && dcGraphDataTree_isMe(result))
    {
        dcNode *save = result;
        dcNode *head = dcGraphDataTree_getContents(result);
        result = dcNode_setTemplate(dcNode_copy(head, DC_DEEP), true);
        dcNode_free(&save, DC_DEEP);
    }

    return result;
}

dcNode *TaffyBridge::evaluate(const std::string &text)
{
    std::string input(text);
    return (dcNode *)dcNodeEvaluator_synchronizeFunctionCall(dcSystem_getCurrentNodeEvaluator(),
                                                             &evalString,
                                                             &input);
}

NodeType TaffyBridge::getNodeType(const dcNode *node) const
{
    NodeType result = 0;

    if (IS_GRAPH_DATA(node)
        && IS_FLAT_ARITHMETIC(node))
    {
        dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(node);

        if (arithmetic->taffyOperator == TAFFY_DIVIDE)
        {
            result = NODE_FLAT_ARITHMETIC_DIVIDE;
        }
        else if (arithmetic->taffyOperator == TAFFY_RAISE)
        {
            result = NODE_FLAT_ARITHMETIC_RAISE;
        }
        else
        {
            result = NODE_FLAT_ARITHMETIC_OTHER;
        }
    }
    else if (IS_GRAPH_DATA(node))
    {
        result = dcGraphData_getType(node);
    }
    else if (dcNilClass_isMe(node))
    {
        result = NODE_CLASS_NIL;
    }
    else if (dcFunctionClass_isMe(node))
    {
        result = NODE_CLASS_FUNCTION;
    }

    return result;
}
