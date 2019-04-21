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

#include "PPMPythonModule.h"
#include <string.h>

char *renderResult = NULL;

PPMApp *PPM_new(void)
{
    return new PPMApp();
}

char *PPM_render(PPMApp *app, char *input)
{
    if (renderResult != NULL)
    {
        free(renderResult);
    }

    // initialize the random seed
    srand(time(0));
    std::string result;

    try
    {
        result = app->execute(input, "small", "alternating", true);
    }
    catch (std::exception &exception)
    {
        result = exception.what();
    }

    renderResult = strdup(result.c_str());
    return renderResult;
}
