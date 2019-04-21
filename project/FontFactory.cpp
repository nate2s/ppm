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

#include "FontFactory.h"
#include "config.h"

#include <sstream>
#include <fstream>

#define xStringize(s) stringize(s)
#define stringize(s) #s

#define QUOTED_INSTALL_DIR xStringize(INSTALL_DIR)

FontFactory &FontFactory::getInstance()
{
    static FontFactory theInstance;
    return theInstance;
}

FontFactory::FontFactory()
{
}

FontFactory::~FontFactory()
{
    // delete all the allocated Fonts
    for (auto &object : allocated_)
    {
        delete object.second;
    }
}

std::string FontFactory::buildPath(const std::vector<std::string> &path) const
{
    std::ostringstream oss;
    size_t size = path.size();

    for (size_t i = 0; i < size; i++)
    {
        oss << path[i];

        if (i < size - 1)
        {
            oss << "/";
        }
    }

    return oss.str();
}

Font *FontFactory::findFont(const std::string &type) const
{
    std::string fontFile = type + ".flf";

    const std::vector<std::string> dirs = {
        buildPath({std::string(QUOTED_INSTALL_DIR), "share", std::string(PACKAGE_NAME), fontFile}),
        buildPath({"fonts", fontFile}),
        buildPath({"project", "fonts", fontFile})
    };

    Font *result = NULL;

    for (const std::string &directory : dirs)
    {
        std::ifstream infile(directory);

        if (infile)
        {
            result = new Font(infile);
            break;
        }
    }

    if (result == NULL)
    {
        throw std::runtime_error(std::string("can't open font: ") + type);
    }

    return result;
}

Font *FontFactory::createFont(const std::string &type)
{
    auto found = allocated_.find(type);
    Font *result = NULL;

    if (found == allocated_.end())
    {
        result = findFont(type);
        allocated_[type] = result;
    }
    else
    {
        result = (*found).second;
    }

    return result;
}
