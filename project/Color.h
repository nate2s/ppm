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

#ifndef __COLOR_H__
#define __COLOR_H__

#include <string>

class Color
{
public:
    static const Color black;
    static const Color red;
    static const Color green;
    static const Color yellow;
    static const Color blue;
    static const Color magenta;
    static const Color cyan;
    static const Color lightGray;
    static const Color darkGray;
    static const Color end;

    Color(const std::string &code, const std::string &color);
    Color(const std::string &code);
    virtual ~Color();

    // get a character
    const std::string &getCode() const;

protected:
    std::string code_;
};

#endif
