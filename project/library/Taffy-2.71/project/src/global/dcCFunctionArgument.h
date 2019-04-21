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

#ifndef __DC_C_FUNCTION_ARGUMENT_H__
#define __DC_C_FUNCTION_ARGUMENT_H__

#include "dcDefines.h"

#define externArgument(name)                            \
    extern dcCFunctionArgument gCFunctionArgument_##name[];

// no arguments
externArgument(none);

// one argument
externArgument(array);
externArgument(block);
externArgument(complexNumber);
externArgument(function);
externArgument(list);
externArgument(matrix);
externArgument(mutex);
externArgument(number);
externArgument(slice);
externArgument(string);
externArgument(symbol);
externArgument(procedure);
externArgument(wild);

// two arguments
externArgument(blockArray);
externArgument(blockBlock);
externArgument(functionArgument);
externArgument(functionArray);
externArgument(functionFunction);
externArgument(numberBlock);
externArgument(numberNumber);
externArgument(numberString);
externArgument(numberWild);
externArgument(procedureArray);
externArgument(stringArray);
externArgument(stringBlock);
externArgument(stringNumber);
externArgument(stringString);
externArgument(stringSymbol);
externArgument(stringWild);
externArgument(symbolWild);
externArgument(wildArray);
externArgument(wildBlock);
externArgument(wildHash);
externArgument(wildNumber);
externArgument(wildString);
externArgument(wildSymbol);
externArgument(wildWild);

// three arguments
externArgument(stringNumberNumber);
externArgument(stringNumberString);
externArgument(stringNumberSymbol);
externArgument(stringStringNumber);
externArgument(symbolNumberNumber);

// four arguments
externArgument(numberNumberNumberNumber);

#endif
