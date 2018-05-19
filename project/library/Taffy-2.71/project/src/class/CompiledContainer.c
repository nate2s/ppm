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

const char *__compiledContainer =    "package org.taffy.core.container\n"
    "\n"
    "abstract class Container\n"
    "{\n"
    "    (@) hash\n"
    "    #const,\n"
    "    #containerLoop\n"
    "    {\n"
    "        result = 0\n"
    "\n"
    "        [self each: ^{ <value>\n"
    "            result += [value hash]\n"
    "        }]\n"
    "\n"
    "        return (result)\n"
    "    }\n"
    "\n"
    "    (@) isEmpty?\n"
    "    #const,\n"
    "    #synchronized\n"
    "    {\n"
    "        return ([self size] == 0)\n"
    "    }\n"
    "\n"
    "    (@) contains?: _block\n"
    "    #const,\n"
    "    #breakthrough\n"
    "    {\n"
    "        result = no\n"
    "\n"
    "        [self each: ^{ <__my_value__>\n"
    "            if ([_block callWith: [__my_value__]])\n"
    "            {\n"
    "                result = true\n"
    "                break\n"
    "            }\n"
    "        }]\n"
    "\n"
    "        return (result)\n"
    "    }\n"
    "\n"
    "    (@) find: _block\n"
    "    #const,\n"
    "    #breakthrough\n"
    "    {\n"
    "        result = nil\n"
    "\n"
    "        [self each: ^{ <value>\n"
    "            if ([_block callWith: [value]])\n"
    "            {\n"
    "                result = value\n"
    "                break\n"
    "            }\n"
    "        }]\n"
    "\n"
    "        return (result)\n"
    "    }\n"
    "}\n"
;