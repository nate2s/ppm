//
// This file is part of Taffy, a mathematical programming language.
// Copyright (C) 2016-2017 Arithmagic, LLC
//
// Taffy is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Taffy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "dcMemory.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_createWithArguments
        (dcTaffyCommandLineArguments_parseAndCreateWithFailure(_argc,
                                                               _argv,
                                                               false));
    dcTestUtilities_go("Bootstrap Test", _argc, _argv, NULL, NULL, true);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
