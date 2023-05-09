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

#ifndef __COMMAND_LINE_ARGUMENTS_H__
#define __COMMAND_LINE_ARGUMENTS_H__

#include <string>

class CommandLineArguments
{
public:
    CommandLineArguments();
    virtual ~CommandLineArguments();

    bool parse(int argc, char **argv);

    void showMiniUsage();
    void showUsage();

    const std::string &getFontType() const;
    const std::string &getColorMode() const;
    const std::string &getText() const;
    bool useRandomColors() const;

protected:
    void showHelpLine();
    bool randomMode_;

    std::string fontType_;
    std::string colorMode_;
    std::string text_;
};

#endif
