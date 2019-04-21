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

const char *__compiledMath =    "package org.taffy.core.maths\n"
    "\n"
    "import org.taffy.core.container.Array\n"
    "\n"
    "class Math\n"
    "{\n"
    "    @sinCoefficients\n"
    "    @sinExponents\n"
    "\n"
    "    (@) initialize\n"
    "    {\n"
    "        memorySize = 100\n"
    "        Number pushDigitLimit: 500\n"
    "\n"
    "        @sinCoefficients = [Array createWithSize: memorySize]\n"
    "        @sinExponents = [Array createWithSize: memorySize]\n"
    "\n"
    "        for (n = 0; n < memorySize; n++)\n"
    "        {\n"
    "            @sinCoefficients[n] = (-1)^n / (2n + 1)!\n"
    "            @sinExponents[n] = 2n + 1\n"
    "        }\n"
    "\n"
    "        Number popDigitLimit\n"
    "        return (self)\n"
    "    }\n"
    "\n"
    "    (@) log: _logValue\n"
    "       base: _baseValue\n"
    "    #const\n"
    "    {\n"
    "        return (log10(_logValue) / log10(_baseValue))\n"
    "    }\n"
    "\n"
    "    (@) lg: _logValue\n"
    "    #const\n"
    "    {\n"
    "        return (log10(_logValue) / log10(2))\n"
    "    }\n"
    "\n"
    "    (@) choose: _n and: _k\n"
    "    {\n"
    "        result = 1\n"
    "        bottom = 1\n"
    "        doBottom = false\n"
    "\n"
    "        while (bottom <= _k)\n"
    "        {\n"
    "            if (doBottom)\n"
    "            {\n"
    "                result /= bottom\n"
    "                doBottom = false\n"
    "                bottom++\n"
    "            }\n"
    "            else\n"
    "            {\n"
    "                result *= _n\n"
    "                doBottom = true\n"
    "                _n--\n"
    "            }\n"
    "        }\n"
    "\n"
    "        return ([result chomp])\n"
    "    }\n"
    "}\n"
;