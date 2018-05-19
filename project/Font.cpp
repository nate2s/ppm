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

#include <fstream>
#include <sstream>

#include "Font.h"

Font::Font(std::ifstream &infile)
{
    parse(infile);
}

Font::~Font()
{
}

uint32_t Font::getMaxHeight() const
{
    return maxHeight_;
}

const StringVector *Font::get(char character)
{
    const auto found = fonts_.find(character);

    if (found == fonts_.end())
    {
        return NULL;
    }

    return &found->second;
}

void Font::parse(std::ifstream &infile)
{
    // get the header
    std::string line;
    std::getline(infile, line);

    // header information
    std::string signature;
    int height;
    int baseline;
    int maxLength;
    int oldLayout;
    int commentLines;
    int printDirection;
    int fullLayout;
    int codetagCount;

    std::istringstream iss(line);

    // read the header
    if (! (iss
           >> signature
           >> height
           >> baseline
           >> maxLength
           >> oldLayout
           >> commentLines
           >> printDirection
           >> fullLayout
           >> codetagCount))
    {
        throw std::runtime_error("can't read font header");
    }

    char hardblank = signature[signature.length() - 1];
    signature = signature.substr(0, signature.length() - 1);

    if (signature != "flf2a")
    {
        throw std::runtime_error("invalid font header signature");
    }

    // read past the comment lines
    for (int i = 0; i < commentLines; i++)
    {
        std::getline(infile, line);
    }

    // parse the ASCII characters
    for (char key = 32; key <= 126; key++)
    {
        std::vector<std::string> font;

        for (int i = 0; i < height; i++)
        {
            if (! std::getline(infile, line))
            {
                throw std::runtime_error("can't read line in font");
            }

            // each line ends in a @, except the last line which ends in @@
            // we don't want any of these characters
            line = line.substr(0, (i < height - 1
                                   ? line.length() - 1
                                   : line.length() - 2));

            bool allSpaces = true;

            // replace hardblanks (usually $) with a space -- we don't need them
            for (size_t blank = line.find(hardblank);
                 blank != std::string::npos;
                 blank = line.find(hardblank))
            {
                line.replace(blank, 1, " ");
            }

            // check if the line has only spaces
            for (const char &value : line)
            {
                if (value != ' ')
                {
                    allSpaces = false;
                    break;
                }
            }

            // the space character is only spaces, so disregard the allSpaces variable
            // in that case
            if (! allSpaces || key == ' ')
            {
                font.push_back(line);
            }
        }

        fonts_[key] = font;
    }

    maxHeight_ = height;
}
