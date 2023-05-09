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

#include "CommandLineArguments.h"

#include <iostream>
#include <unordered_map>
#include <regex>

CommandLineArguments::CommandLineArguments()
{
}

CommandLineArguments::~CommandLineArguments()
{
}

void CommandLineArguments::showMiniUsage()
{
    std::cout << "ppm: missing input\n";
    showHelpLine();
}

void CommandLineArguments::showHelpLine()
{
    std::cout << "Try 'ppm --help' for more information.\n";
}

void CommandLineArguments::showUsage()
{
    std::cout << "Usage: ppm [options] input\n"
              << "\n"
              << "input is a string, like: \"sin(x) + e^2\"\n"
              << "\n"
              << "Optional Options:\n"
              << "\n"
              << "--font       Specify font. small is the default. Options:\n"
              << "               banner big ivrit small smscript\n"
              << "\n"
              << "--color      Specify color mode. alternating is the default. Options:\n"
              << "               none         no color\n"
              << "               alternating  each character gets a new color\n"
              << "               grouped      character groups share a color\n"
              << "\n"
              << "--no-random  Do not use random colors. Random colors are enabled by default.\n"
              << "\n"
              << "--input   The input to render (default argument)\n";
}

bool CommandLineArguments::parse(int argc, char **argv)
{
    // the default font
    fontType_ = "small";

    // the default color
    colorMode_ = "alternating";

    bool noRandom = false;

    if (argc < 2)
    {
        showMiniUsage();
        return false;
    }

    bool result = true;

    const std::unordered_map<std::string, std::string *> arguments = {
        {"--input",  &text_},
        {"--font",   &fontType_},
        {"--color",  &colorMode_},
        {"--help",   NULL}
    };

    const std::unordered_map<std::string, bool *> soleArguments = {
        {"--no-random", &noRandom}
    };

    // parse the arguments
    for (int i = 1; i < argc; i++)
    {
        std::string argument(argv[i]);
        const auto found = arguments.find(argument);
        const auto soleFound = soleArguments.find(argument);

        if (found != arguments.end())
        {
            if (i >= argc - 1
                || (*found).second == NULL)
            {
                // a required argument was not given, or we just want to show usage
                showUsage();
                result = false;
                break;
            }

            *(*found).second = argv[i + 1];
            i++;
        }
        else if (soleFound != soleArguments.end())
        {
            *(*soleFound).second = true;
        }
        else if (argument.length() >= 1 && argument[0] == '-')
        {
            std::cout << "Error: unknown argument: " << argument << "\n";
            showHelpLine();
            result = false;
            break;
        }
        else
        {
            // default argument
            text_ = argument;
        }
    }

    // convert local representation to member
    randomMode_ = ! noRandom;

    // remove trailing whitespace
    while (text_.length() > 0 && text_[text_.length() - 1] == ' ')
    {
        text_ = text_.substr(0, text_.length() - 1);
    }

    // remove leading whitespace
    while (text_.length() > 0 && text_[0] == ' ')
    {
        text_ = text_.substr(1, text_.length());
    }

    if (text_ == "" && result)
    {
        // no error has been given yet since result is true
        std::cout << "Error: input must be provided\n";
        showHelpLine();
        result = false;
    }

    return result;
}

const std::string &CommandLineArguments::getFontType() const
{
    return fontType_;
}

const std::string &CommandLineArguments::getColorMode() const
{
    return colorMode_;
}

const std::string &CommandLineArguments::getText() const
{
    return text_;
}

bool CommandLineArguments::useRandomColors() const
{
    return randomMode_;
}
