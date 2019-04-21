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

const char *__compiledString =    "package org.taffy.core\n"
    "\n"
    "class String\n"
    "{\n"
    "    (@@) prettyPrint: _value\n"
    "    #synchronizedRead\n"
    "    {\n"
    "        result = _value\n"
    "\n"
    "        if ([_value kindOf?: String])\n"
    "        {\n"
    "            // double backslash needed for C conversion\n"
    "            result = \"\\\"\" + _value + \"\\\"\"\n"
    "        }\n"
    "\n"
    "        return (result)\n"
    "    }\n"
    "\n"
    "    (@) asString\n"
    "    #const,\n"
    "    #synchronizedRead\n"
    "    {\n"
    "        return (self)\n"
    "    }\n"
    "\n"
    "    (@) asSymbol\n"
    "    #const,\n"
    "    #synchronizedRead\n"
    "    {\n"
    "        return ([kernel eval: \"'#[self]\"])\n"
    "    }\n"
    "\n"
    "    (@) #operator(<<): _value\n"
    "    #const,\n"
    "    #synchronizedRead\n"
    "    {\n"
    "        return (self + _value)\n"
    "    }\n"
    "}\n"
;