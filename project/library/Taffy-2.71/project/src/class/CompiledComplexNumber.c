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

const char *__compiledComplexNumber =    "package org.taffy.core.maths\n"
    "\n"
    "atomic class ComplexNumber\n"
    "{\n"
    "    (@) absoluteValue\n"
    "    {\n"
    "        real = [self real]\n"
    "        imaginary = [self imaginary]\n"
    "        return (sqrt(real^2 + imaginary^2))\n"
    "    }\n"
    "\n"
    "    // re^(is)^(x + iy)\n"
    "    (@) #operator(^): _other\n"
    "    {\n"
    "        // self = r*e^(i*theta)\n"
    "        r = abs(self)\n"
    "        theta = angle(self)\n"
    "        return (r^_other * e^(i * theta * _other))\n"
    "    }\n"
    "\n"
    "    (@) #operator(^=): _other\n"
    "    {\n"
    "        // TODO: speed me up\n"
    "        value = (self ^ _other)\n"
    "\n"
    "        if ([value class] == ComplexNumber)\n"
    "        {\n"
    "            self setValue: value\n"
    "        }\n"
    "        else\n"
    "        {\n"
    "            self setReal: value\n"
    "            self setImaginary: 0\n"
    "        }\n"
    "\n"
    "        return (self)\n"
    "    }\n"
    "\n"
    "    (@) #prefixOperator(-)\n"
    "    {\n"
    "        return (self * -1)\n"
    "    }\n"
    "\n"
    "    (@) negate\n"
    "    {\n"
    "       return (self * -1)\n"
    "    }\n"
    "\n"
    "    (@) #operator(==): _other\n"
    "    {\n"
    "        real = [self real]\n"
    "        imaginary = [self imaginary]\n"
    "\n"
    "        if ([_other class] == Number)\n"
    "        {\n"
    "            if (_other == NaN and (real == NaN or imaginary == NaN))\n"
    "            {\n"
    "                return (true)\n"
    "            }\n"
    "            else if (imaginary == 0)\n"
    "            {\n"
    "                return (real == _other)\n"
    "            }\n"
    "            else\n"
    "            {\n"
    "                return (false)\n"
    "            }\n"
    "        }\n"
    "        else\n"
    "        {\n"
    "            return ([self trulyEquals: _other])\n"
    "        }\n"
    "    }\n"
    "\n"
    "    (@) #operator(~=): _arguments\n"
    "    {\n"
    "        other = _arguments[0]\n"
    "\n"
    "        if ([other class] == Number)\n"
    "        {\n"
    "            real = [self real]\n"
    "            imaginary = [self imaginary]\n"
    "\n"
    "            if (other == NaN and (real == NaN or imaginary == NaN))\n"
    "            {\n"
    "                return (true)\n"
    "            }\n"
    "            else if (! (imaginary ~= 0))\n"
    "            {\n"
    "                return (false)\n"
    "            }\n"
    "\n"
    "            return ([real perform: \"#operator(~=):\"\n"
    "                             with: [_arguments]])\n"
    "        }\n"
    "        else\n"
    "        {\n"
    "            return ([self deltaEquals: _arguments])\n"
    "        }\n"
    "    }\n"
    "\n"
    "    (@) doOperation: _operation\n"
    "                for: _value\n"
    "    {\n"
    "        value = [_value castAs: Number]\n"
    "        return ([[self real] perform: _operation with: [value]]\n"
    "                + ([[self imaginary] perform: _operation with: [value]]) * i)\n"
    "    }\n"
    "\n"
    "    (@) #operator(%): _other\n"
    "    {\n"
    "        return ([self doOperation: \"realModulus:\"\n"
    "                              for: _other])\n"
    "    }\n"
    "\n"
    "    (@) #operator(<<): _other\n"
    "    {\n"
    "        return ([self doOperation: \"realLeftShift:\"\n"
    "                              for: _other])\n"
    "    }\n"
    "\n"
    "    (@) #operator(>>): _other\n"
    "    {\n"
    "        return ([self doOperation: \"realRightShift:\"\n"
    "                              for: _other])\n"
    "    }\n"
    "\n"
    "    (@) #operator(&): _other\n"
    "    {\n"
    "        return ([self doOperation: \"realBitAnd:\"\n"
    "                              for: _other])\n"
    "    }\n"
    "\n"
    "    (@) #operator(|): _other\n"
    "    {\n"
    "        return ([self doOperation: \"realBitOr:\"\n"
    "                              for: _other])\n"
    "    }\n"
    "}\n"
;