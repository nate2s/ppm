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

#ifndef __PPM_APP_H__
#define __PPM_APP_H__

#include "Font.h"
#include "CommandLineArguments.h"

class PPMApp
{
public:
    PPMApp() noexcept;

    bool execute(int argc, char **argv);

    std::string execute(const std::string &maths,
                        const std::string &fontTypeString,
                        const std::string &colorModeString,
                        bool useRandomColors);
};

extern "C"
{
    int testMeToo(void);

    PPMApp *PPM_new(void);
    char *PPM_render(PPMApp *app, char *input);
    char *testMe(void);
}

#endif
