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

#include <stdio.h>

#include "dcClass.h"
#include "dcDefines.h"
#include "dcArrayClass.h"
#include "dcBlockClass.h"
#include "dcCFunctionArgument.h"
#include "dcComplexNumberClass.h"
#include "dcContainerClass.h"
#include "dcFunctionClass.h"
#include "dcHashClass.h"
#include "dcListClass.h"
#include "dcMatrixClass.h"
#include "dcMutexClass.h"
#include "dcNumberClass.h"
#include "dcProcedureClass.h"
#include "dcStringClass.h"
#include "dcSuppliedArgumentClass.h"
#include "dcSymbolClass.h"
#include "dcWildClass.h"

#define makeArgument(name, ...)                                         \
    dcCFunctionArgument gCFunctionArgument_##name[] = {__VA_ARGS__, NULL}

// make the code below a little easier to read
#define Q_Q(NAME) MAKE_FULLY_QUALIFIED(NAME)

//
// the makeArgument() macro below creates variables for use in class method
// definitions
//
// example:
//
// makeArgument(fooBarBaz, Q_Q(FOO), Q_Q(BAR), Q_Q(BAZ))
//
// creates a variable:
//
// dcCFunctionArgument gCFunctionArgument_fooBarBaz[] =
//     {MAKE_FULLY_QUALIFIED(FOO),
//      MAKE_FULLY_QUALIFIED(BAR),
//      MAKE_FULLY_QUALIFIED(BAZ),
//      NULL}
//
// see dcDefines.h for the implementation of MAKE_FULLY_QUALIFIED
//

// no arguments
makeArgument(none, NULL);

// single argument
makeArgument(array,         Q_Q(ARRAY));
makeArgument(block,         Q_Q(BLOCK));
makeArgument(complexNumber, Q_Q(COMPLEX_NUMBER));
makeArgument(function,      Q_Q(FUNCTION));
makeArgument(list,          Q_Q(LIST));
makeArgument(matrix,        Q_Q(MATRIX));
makeArgument(mutex,         Q_Q(MUTEX));
makeArgument(number,        Q_Q(NUMBER));
makeArgument(procedure,     Q_Q(PROCEDURE));
makeArgument(string,        Q_Q(STRING));
makeArgument(symbol,        Q_Q(SYMBOL));
makeArgument(wild,          Q_Q(WILD));

// two arguments
makeArgument(blockArray,       Q_Q(BLOCK),     Q_Q(ARRAY));
makeArgument(blockBlock,       Q_Q(BLOCK),     Q_Q(BLOCK));
makeArgument(functionArray,    Q_Q(FUNCTION),  Q_Q(ARRAY));
makeArgument(functionFunction, Q_Q(FUNCTION),  Q_Q(FUNCTION));
makeArgument(numberBlock,      Q_Q(NUMBER),    Q_Q(BLOCK));
makeArgument(numberNumber,     Q_Q(NUMBER),    Q_Q(NUMBER));
makeArgument(numberString,     Q_Q(NUMBER),    Q_Q(STRING));
makeArgument(numberWild,       Q_Q(NUMBER),    Q_Q(WILD));
makeArgument(procedureArray,   Q_Q(PROCEDURE), Q_Q(ARRAY));
makeArgument(stringArray,      Q_Q(STRING),    Q_Q(ARRAY));
makeArgument(stringBlock,      Q_Q(STRING),    Q_Q(BLOCK));
makeArgument(stringNumber,     Q_Q(STRING),    Q_Q(NUMBER));
makeArgument(stringString,     Q_Q(STRING),    Q_Q(STRING));
makeArgument(stringSymbol,     Q_Q(STRING),    Q_Q(SYMBOL));
makeArgument(stringWild,       Q_Q(STRING),    Q_Q(WILD));
makeArgument(symbolWild,       Q_Q(SYMBOL),    Q_Q(WILD));
makeArgument(wildArray,        Q_Q(WILD),      Q_Q(ARRAY));
makeArgument(wildBlock,        Q_Q(WILD),      Q_Q(BLOCK));
makeArgument(wildHash,         Q_Q(WILD),      Q_Q(HASH));
makeArgument(wildString,       Q_Q(WILD),      Q_Q(STRING));
makeArgument(wildWild,         Q_Q(WILD),      Q_Q(WILD));
makeArgument(wildSymbol,       Q_Q(WILD),      Q_Q(SYMBOL));
makeArgument(wildNumber,       Q_Q(WILD),      Q_Q(NUMBER));

// three arguments
makeArgument(stringNumberNumber, Q_Q(STRING), Q_Q(NUMBER), Q_Q(NUMBER));
makeArgument(stringNumberString, Q_Q(STRING), Q_Q(NUMBER), Q_Q(STRING));
makeArgument(stringNumberSymbol, Q_Q(STRING), Q_Q(NUMBER), Q_Q(SYMBOL));
makeArgument(symbolNumberNumber, Q_Q(SYMBOL), Q_Q(NUMBER), Q_Q(SYMBOL));

// four
makeArgument(numberNumberNumberNumber,
             Q_Q(NUMBER),
             Q_Q(NUMBER),
             Q_Q(NUMBER),
             Q_Q(NUMBER));
