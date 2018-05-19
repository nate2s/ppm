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

#include <sstream>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <list>

#include "dcTaffy.h"

#include "Renderer.h"
#include "TaffyBridge.h"

Renderer::ColorMode Renderer::getColorMode(std::string input)
{
    const std::unordered_map<std::string, Renderer::ColorMode> lookup = {
        {"",            COLOR_MODE_NONE},
        {"none",        COLOR_MODE_NONE},
        {"alternating", COLOR_MODE_ALTERNATING},
        {"grouped",     COLOR_MODE_GROUPED}
    };

    std::transform(input.begin(), input.end(), input.begin(), ::tolower);

    const auto found = lookup.find(input);

    if (found == lookup.end())
    {
        throw std::runtime_error(std::string("Unknown color mode: ") + input);
    }

    return found->second;
}

Renderer::Renderer(Font *font, ColorMode colorMode, bool randomColors, int defaultSpacing)
    : font_(font),
      colorMode_(colorMode),
      randomColors_(randomColors),
      defaultSpacing_(defaultSpacing)
{
}

Renderer::~Renderer()
{
}

// The main entry point for Renderer
std::string Renderer::render(const std::string &maths)
{
    // an assignment won't always be parsible by Taffy -- think: y^2 = x
    // so treat assignments (and equalities) as a special case
    LatticePtr graph = tryToRenderAssignment(maths);

    if (graph == NULL)
    {
        // it's not an assignment, so proceed
        graph = compileAndRenderString(maths);
    }

    return graph.get()->convertToString(colorMode_ != COLOR_MODE_NONE, randomColors_);
}

LatticePtr Renderer::renderDivideBar(LatticePtr &top, LatticePtr &bottom)
{
    LatticePtr result = std::unique_ptr<Lattice>(new Lattice());
    const StringVector *fancy = font_->get('-');

    if (fancy == NULL)
    {
        return result;
    }

    Lattice bar(*fancy, '-');
    bar.removeBlankLines();

    while (result->getWidth() <= std::max(top->getWidth(), bottom->getWidth()))
    {
        result->addToRightSquished(bar);

        if (colorMode_ == COLOR_MODE_ALTERNATING)
        {
            bar.setNewId();
        }
    }

    return result;
}

LatticePtr Renderer::renderString(const std::string &maths)
{
    LatticePtr result = std::unique_ptr<Lattice>(new Lattice());
    char previousValue = 0;

    for (char value : maths)
    {
        const StringVector *fancy = font_->get(value);

        if (fancy == NULL)
        {
            fancy = font_->get('?');
        }

        Lattice graph(*fancy, value);

        if (previousValue == '-')
        {
            result->addToRightMiddle(graph);
        }
        else
        {
            result->addToRight(graph);
        }

        previousValue = value;
    }

    if (colorMode_ == COLOR_MODE_GROUPED)
    {
        result->setNewId();
    }

    return result;
}

bool Renderer::isSingleLine(LatticePtr &graph) const
{
    return graph->getHeight() <= font_->getMaxHeight();
}

void Renderer::engroup(LatticePtr &graph)
{
    if (graph->isGrouped())
    {
        return;
    }

    //
    // depending on the height of 'graph', the grouping will either be
    // parentheses, or a constructed enclosure of underscores and pipes
    //

    if (graph->getHeight() < font_->getMaxHeight())
    {
        // 'graph' isn't very tall, so just enclose it in parentheses, like: (x)

        LatticePtr left = renderString("(");
        LatticePtr right = renderString(")");

        if (colorMode_ == COLOR_MODE_GROUPED)
        {
            left->setId(*right);
        }

        graph->addToLeft(*left.get());
        graph->addToRight(*right.get());
    }
    else
    {
        //
        // 'graph' is too tall for parentheses, so create an enclosure like:
        //
        //  -     -
        //  |  x  |
        //  | --- |
        //  |  2  |
        //  -     -
        //

        LatticePtr cap = renderString("_");
        cap->removeBlankLines();
        LatticePtr pipe = renderString("|");
        pipe->removeBlankLines();
        Lattice leftBar(*cap);
        Lattice rightBar(*cap);

        const int x = rightBar.getWidth() - pipe->getWidth();

        // add a little breathing room (+1)
        while (leftBar.getHeight() <= graph->getHeight() + 1)
        {
            leftBar.addToBottom(*pipe);
            rightBar.addToBottom(*pipe, x);
        }

        leftBar.addToBottom(*cap);
        rightBar.addToBottom(*cap);

        if (colorMode_ == COLOR_MODE_GROUPED)
        {
            leftBar.setId(rightBar);
        }

        graph->engroup(leftBar, rightBar);
    }
}

bool Renderer::isTall(LatticePtr &lattice) const
{
    return lattice->getHeight() > (2 * font_->getMaxHeight());
}

LatticePtr Renderer::renderArguments(const dcArray *arguments)
{
    LatticePtr comma = renderString(",");
    const uint32_t bumpUp = comma->getHeight() - 1;
    LatticePtr renderedArguments = std::unique_ptr<Lattice>(new Lattice());
    bool needGroup = false;

    for (size_t i = 0; i < arguments->size; i++)
    {
        LatticePtr rendered = render(arguments->objects[i]);
        Position added = renderedArguments->addToRight(*rendered);

        if (i > 0 || ! rendered->isGrouped())
        {
            needGroup = true;
        }

        if (i < arguments->size - 1)
        {
            // we need to add a comma, but we can't do it right away
            // mark where the comma will go, and raise it up a little
            renderedArguments->appendColumns(comma->getWidth());
            renderedArguments->mark(Position(added.x + rendered->getWidth(),
                                             added.y + rendered->getHeight() - bumpUp));
        }
    }

    if (needGroup)
    {
        engroup(renderedArguments);
    }

    // finally insert the commas
    renderedArguments->paste(*comma);
    return renderedArguments;
}

LatticePtr Renderer::renderAssignment(const dcNode *assignment, bool topLevel)
{
    dcNode *value = dcAssignment_getValue(assignment);
    LatticePtr left = render(dcAssignment_getIdentifier(assignment), topLevel);
    LatticePtr equals = renderString("=");
    LatticePtr right = render(value, topLevel);

    left->addToRight(*equals);

    NodeType type = TaffyBridge::getInstance().getNodeType(value);

    if (type != NODE_FLAT_ARITHMETIC_RAISE && isTall(right))
    {
        left->addToRightMiddle(*right);
    }
    else
    {
        left->addToRight(*right);
    }

    return left;
}

LatticePtr Renderer::renderNilClass(const dcNode *node, bool topLevel)
{
    return renderString("");
}

LatticePtr Renderer::renderFunctionClass(const dcNode *node, bool topLevel)
{
    // dcFunctionClass_getBody() can modify its argument so we must pass in
    // a non-const dcNode*
    dcNode *copy = dcNode_copy(node, DC_DEEP);
    LatticePtr result = render(dcFunctionClass_getBody(copy));
    dcNode_free(&copy, DC_DEEP);
    return result;
}

LatticePtr Renderer::renderIdentifier(const dcNode *node, bool topLevel)
{
    return renderString(dcIdentifier_getName(node));
}

LatticePtr Renderer::renderGraphDataTree(const dcNode *node, bool topLevel)
{
    return render(dcGraphDataTree_getContents(node));
}

LatticePtr Renderer::renderMethodCall(const dcNode *node, bool topLevel)
{
    const dcMethodCall *call = CAST_METHOD_CALL(node);

    // only render a method call that has 1 argument, and that argument
    // must be an array object
    if (! (call->arguments->size == 1
           && dcArrayClass_isMe(dcList_getHead(call->arguments))))
    {
        return nullptr;
    }

    LatticePtr name = render(call->receiver);
    LatticePtr renderedArguments = renderArguments(dcArrayClass_getObjects(dcList_getHead(call->arguments)));

    // add the arguments to the name
    name->addToRightMiddle(*renderedArguments);

    return name;
}

LatticePtr Renderer::renderFlatArithmeticDivide(const dcNode *node, bool topLevel)
{
    dcFlatArithmetic *arithmetic = dcFlatArithmetic_copy(CAST_FLAT_ARITHMETIC(node), DC_DEEP);

    // collapse head divides
    dcFlatArithmetic_mergeDivide(arithmetic);

    LatticePtr result = std::unique_ptr<Lattice>(new Lattice());
    LatticePtr top = render(arithmetic->values->head->object);
    result->addToBottomCenter(*top.get());

    for (dcListElement *that = arithmetic->values->head->next; that != NULL; that = that->next)
    {
        LatticePtr bottom = render(that->object);
        LatticePtr divideBar = renderDivideBar(top, bottom);

        result->addToBottomCenter(*divideBar.get());
        result->addToBottomCenter(*bottom.get());

        top = std::move(bottom);
    }

    // engroup if we're tall and not embedded within other things (! topLevel)
    if (! topLevel && isTall(result))
    {
        engroup(result);
    }

    dcFlatArithmetic_free(&arithmetic, DC_DEEP);
    return result;
}

LatticePtr Renderer::renderFlatArithmeticRaise(const dcNode *node, bool topLevel)
{
    LatticePtr result = std::unique_ptr<Lattice>(new Lattice());
    const dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(node);

    FOR_EACH_IN_LIST(arithmetic->values, value)
    {
        LatticePtr rendered = render(value->object);

        if (dcFlatArithmetic_isGrouped(value->object))
        {
            engroup(rendered);
        }

        result->addToTopRight(*rendered);
    }

    return result;
}

// Render a flat arithmetic that isn't a divide or raise
LatticePtr Renderer::renderFlatArithmetic(const dcNode *node, bool topLevel)
{
    LatticePtr result = std::unique_ptr<Lattice>(new Lattice());
    const dcFlatArithmetic *arithmetic = CAST_FLAT_ARITHMETIC(node);

    FOR_EACH_IN_LIST(arithmetic->values, value)
    {
        LatticePtr rendered = render(value->object);
        NodeType type = TaffyBridge::getInstance().getNodeType(value->object);

        // engroup appropriate objects
        if ((dcFlatArithmetic_isGrouped(value->object)
             && type != NODE_FLAT_ARITHMETIC_RAISE
             && type != NODE_FLAT_ARITHMETIC_DIVIDE)
            || (isTall(rendered) && type != NODE_METHOD_CALL))
        {
            engroup(rendered);
        }

        if (value->previous == NULL)
        {
            // it's the first, so just add it
            result->addToRight(*rendered);
        }
        else
        {
            // it's not the first, so add the next element
            LatticePtr separator = renderString(dcSystem_getOperatorSymbol(arithmetic->taffyOperator));

            separator->addToRightMiddle(*rendered, defaultSpacing_);

            if (isSingleLine(result) && isSingleLine(separator))
            {
                // everything is a 'single line', so add to the bottom
                result->addToRight(*separator);
            }
            else
            {
                // it's not a 'single line', so center everything around the middle
                result->addToRightMiddle(*separator, defaultSpacing_);
            }
        }
    }

    return result;
}

// Render a taffy dcNode
LatticePtr Renderer::render(const dcNode *node, bool topLevel)
{
    typedef LatticePtr (Renderer::*NodeRenderFunction)(const dcNode *node, bool topLevel);
    LatticePtr result = nullptr;

    const std::unordered_map<NodeType, NodeRenderFunction> renderMap = {
        {NODE_CLASS_NIL,              &Renderer::renderNilClass},
        {NODE_CLASS_FUNCTION,         &Renderer::renderFunctionClass},
        {NODE_FLAT_ARITHMETIC_DIVIDE, &Renderer::renderFlatArithmeticDivide},
        {NODE_FLAT_ARITHMETIC_RAISE,  &Renderer::renderFlatArithmeticRaise},
        {NODE_FLAT_ARITHMETIC_OTHER,  &Renderer::renderFlatArithmetic},
        {NODE_FUNCTION_UPDATE,        &Renderer::renderFunctionUpdate},
        {NODE_ASSIGNMENT,             &Renderer::renderAssignment},
        {NODE_GRAPH_DATA_TREE,        &Renderer::renderGraphDataTree},
        {NODE_METHOD_CALL,            &Renderer::renderMethodCall}
    };

    auto found = renderMap.find(TaffyBridge::getInstance().getNodeType(node));

    if (found != renderMap.end())
    {
        result = (this->*found->second)(node, topLevel);
    }
    else
    {
        // this type doesn't require any special rendering, or we failed rendering
        char *displayed = dcNode_synchronizedDisplay(node);
        result = renderString(displayed);
        dcMemory_free(displayed);
    }

    return result;
}

LatticePtr Renderer::renderFunctionUpdate(const dcNode *node, bool topLevel)
{
    const dcFunctionUpdate *update = CAST_FUNCTION_UPDATE(node);
    dcArray *arguments = dcArray_createFromList(update->arguments, DC_SHALLOW);

    LatticePtr result = render(update->identifier);
    LatticePtr renderedArguments = renderArguments(arguments);
    LatticePtr equals = compileAndRenderString("=");
    LatticePtr renderedArithmetic = render(update->arithmetic, topLevel);

    result->addToRightMiddle(*renderedArguments);
    result->addToRightMiddle(*equals);
    result->addToRightMiddle(*renderedArithmetic);

    dcArray_free(&arguments, DC_SHALLOW);
    return result;
}

LatticePtr Renderer::tryToRenderAssignment(const std::string &maths)
{
    size_t found = maths.find('=');
    LatticePtr result(nullptr);

    if (maths == "=")
    {
        result = renderString("=");
    }
    else if (found != std::string::npos)
    {
        // we have at least =

        std::string equalsString = "=";
        size_t foundEnd = found + 1;

        // check if we have ==
        if (foundEnd < maths.length()
            && maths[foundEnd] == '=')
        {
            foundEnd++;
            equalsString = "==";
        }

        std::string leftString = maths.substr(0, found - 1);
        std::string rightString = maths.substr(foundEnd, maths.length());
        LatticePtr left = compileAndRenderString(leftString);
        LatticePtr right = compileAndRenderString(rightString);
        LatticePtr equals = compileAndRenderString(equalsString);

        left->addToRightMiddle(*equals.get(), defaultSpacing_);
        left->addToRightMiddle(*right.get(), defaultSpacing_);

        result = std::move(left);
    }

    return result;
}

LatticePtr Renderer::compileAndRenderString(const std::string &maths)
{
    dcNode *output = TaffyBridge::getInstance().evaluate(maths);
    LatticePtr graph(nullptr);

    if (output != NULL)
    {
        graph = render(output, true);
    }

    if (output == NULL || graph == nullptr)
    {
        // evaluation failed, so just render the string
        graph = renderString(maths);
    }

    return graph;
}
