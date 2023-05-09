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

//
// A renderer turns a regular, boring string into a fancy, stylized string
//
#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <string>
#include <memory>

#include "Font.h"
#include "Lattice.h"
#include "Color.h"

using LatticePtr = std::unique_ptr<Lattice>;

class Renderer
{
public:
    enum ColorMode
    {
        COLOR_MODE_NONE        = 0,
        COLOR_MODE_ALTERNATING = 1,
        COLOR_MODE_GROUPED     = 2
    };

    // throws std::runtime_error
    static ColorMode getColorMode(std::string input);

    Renderer(Font *font, ColorMode colorMode, bool randomColors = true, int defaultSpacing = 1);
    virtual ~Renderer();

    Renderer &operator=(const Renderer &other) = delete;
    Renderer(const Renderer &other) = delete;

    std::string render(const std::string &maths);

protected:
    bool isSingleLine(LatticePtr &graph) const;

    void engroup(LatticePtr &graph);

    // 'top level' render
    LatticePtr render(const dcNode *node, bool topLevel = false);

    LatticePtr tryToRenderAssignment(const std::string &maths);
    LatticePtr renderString(const std::string &maths);
    LatticePtr renderDivideBar(LatticePtr &top, LatticePtr &bottom);
    LatticePtr renderArguments(const dcArray *arguments);

    LatticePtr renderNilClass(const dcNode *node, bool topLevel);
    LatticePtr renderFunctionClass(const dcNode *node, bool topLevel);
    LatticePtr renderFlatArithmeticDivide(const dcNode *node, bool topLevel);
    LatticePtr renderFlatArithmeticRaise(const dcNode *node, bool topLevel);
    LatticePtr renderFlatArithmetic(const dcNode *node, bool topLevel);
    LatticePtr renderFunctionUpdate(const dcNode *node, bool topLevel);
    LatticePtr renderAssignment(const dcNode *node, bool topLevel);
    LatticePtr renderGraphDataTree(const dcNode *node, bool topLevel);
    LatticePtr renderMethodCall(const dcNode *node, bool topLevel);
    LatticePtr renderIdentifier(const dcNode *node, bool topLevel);

    LatticePtr compileAndRenderString(const std::string &input);

    bool isTall(LatticePtr &lattice) const;

    Font *font_;
    ColorMode colorMode_;
    bool randomColors_;
    int defaultSpacing_;
};

#endif
