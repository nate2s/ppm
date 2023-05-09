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

#include "Color.h"

const Color Color::black("1", "30");
const Color Color::red("1", "31");
const Color Color::green("1", "32");
const Color Color::yellow("1", "33");
const Color Color::blue("1", "34");
const Color Color::magenta("1", "35");
const Color Color::cyan("1", "36");
const Color Color::darkGray("1", "90");
const Color Color::end("\033[0m");

Color::Color(const std::string &code)
    : code_(code)
{
}

Color::Color(const std::string &code, const std::string &color)
{
    code_ = std::string("\033[") + code + ";" + color + "m";
}

Color::~Color()
{
}

const std::string &Color::getCode() const
{
    return code_;
}
