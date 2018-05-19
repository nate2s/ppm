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

#ifndef __FONT_FACTORY_H__
#define __FONT_FACTORY_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "Font.h"

// A singleton factory that creates Font objects from font types
class FontFactory
{
public:
    static FontFactory &getInstance();

    // can't copy
    FontFactory(const FontFactory &other) = delete;
    FontFactory &operator=(const FontFactory &other) = delete;

    Font *createFont(const std::string &type);

protected:
    FontFactory();
    virtual ~FontFactory();

    Font *findFont(const std::string &type) const;
    std::string buildPath(const std::vector<std::string> &path) const;

private:
    // allocated objects
    std::unordered_map<std::string, Font *> allocated_;
};

#endif
