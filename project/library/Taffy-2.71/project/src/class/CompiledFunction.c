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

const char *__compiledFunction =    "package org.taffy.core.maths\n"
    "\n"
    "import org.taffy.core.container.Array\n"
    "import org.taffy.core.exception.*\n"
    "\n"
    "atomic abstract class Function\n"
    "{\n"
    "    (@) fillMemoryFrom: _from to: _to\n"
    "    {\n"
    "        [self fillMemoryFrom: _from to: _to step: 1]\n"
    "    }\n"
    "\n"
    "    (@) fillMemoryFrom: _from to: _to step: _step\n"
    "    {\n"
    "        self setMemorySize: (abs((_to - _from) + 1) * _step)\n"
    "\n"
    "        for (a = _from; a != _to; a += _step)\n"
    "        {\n"
    "            self(a)\n"
    "        }\n"
    "    }\n"
    "\n"
    "    (@) integrate: _symbol from: _from to: _to\n"
    "    {\n"
    "        result = [self integrate: _symbol]\n"
    "\n"
    "        if (result == 'unfindableIntegral)\n"
    "        {\n"
    "            return ([self simpsonFrom: _from to: _to intervalCount: 100])\n"
    "        }\n"
    "        else\n"
    "        {\n"
    "            result = (result(_to) - result(_from))\n"
    "        }\n"
    "    }\n"
    "\n"
    "    (@) simpsonFrom: _a to: _b intervalCount: _n\n"
    "    {\n"
    "        h = (_b - _a) / _n\n"
    "        result = 0\n"
    "\n"
    "        for (j = 1; j <= (_n / 2); j++)\n"
    "        {\n"
    "            result += (self(_a + (2*j - 2)*h)\n"
    "                       + 4 * self(_a + (2*j - 1) * h)\n"
    "                       + self(_a + (2 * j)*h))\n"
    "        }\n"
    "\n"
    "        return (result * (h / 3))\n"
    "    }\n"
    "\n"
    "    (@) derive\n"
    "    #breakthrough\n"
    "    {\n"
    "        [self derive: '__x__]\n"
    "    }\n"
    "\n"
    "    (@) integrate\n"
    "    #breakthrough\n"
    "    {\n"
    "        [self integrate: '__x__]\n"
    "    }\n"
    "\n"
    "    (@) #operator([...]): _indexes\n"
    "    #breakthrough\n"
    "    {\n"
    "        if ([_indexes size] != 2)\n"
    "        {\n"
    "            throw ([InvalidIndexesException new: _indexes])\n"
    "        }\n"
    "\n"
    "        if (_indexes[0] > _indexes[1])\n"
    "        {\n"
    "            result = [Array createWithSize: (_indexes[0] - _indexes[1]) + 1]\n"
    "\n"
    "            for (j = _indexes[0], k = 0; j >= _indexes[1]; j--, k++)\n"
    "            {\n"
    "                result[k] = self(j)\n"
    "            }\n"
    "\n"
    "            return result\n"
    "        }\n"
    "        else\n"
    "        {\n"
    "            result = [Array createWithSize: (_indexes[1] - _indexes[0]) + 1]\n"
    "\n"
    "            for (j = _indexes[0], k = 0; j <= _indexes[1]; j++, k++)\n"
    "            {\n"
    "                result[k] = self(j)\n"
    "            }\n"
    "\n"
    "            return result\n"
    "        }\n"
    "    }\n"
    "}\n"
;