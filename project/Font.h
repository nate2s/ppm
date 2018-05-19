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

#ifndef __FONT_H__
#define __FONT_H__

#include <string>
#include <unordered_map>
#include <vector>

typedef std::vector<std::string> StringVector;
typedef std::unordered_map<char, StringVector> FontMap;

// A Font knows how to read and store data from an flf file
class Font
{
public:
    Font(std::ifstream &filename);
    virtual ~Font();

    // get a character
    const StringVector *get(char character);

    uint32_t getMaxHeight() const;

protected:
    // parse a file and fill 'fonts' and 'chompedFonts'
    void parse(std::ifstream &filename);

    // the fonts
    FontMap fonts_;

    // the max height of a font
    uint32_t maxHeight_;
};

#endif
