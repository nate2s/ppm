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

const char *__compiledThreader =    "import org.taffy.core.container.Array\n"
    "import org.taffy.core.maths.Series\n"
    "import org.taffy.core.threading.Thread\n"
    "\n"
    "package org.taffy.core.threading\n"
    "\n"
    "class Threader\n"
    "{\n"
    "    (@@) startWithBlock: _block\n"
    "              arguments: _arguments\n"
    "    {\n"
    "        result = [Array createWithSize: [_arguments size]]\n"
    "        threads = [Array createWithSize: [_arguments size]]\n"
    "        j = 0\n"
    "\n"
    "        _arguments each: ^{ <_argument>\n"
    "            threads[j] = [Thread new: [_block copy]]\n"
    "            threads[j] startWith: _argument\n"
    "            j++\n"
    "        }\n"
    "\n"
    "        j = 0\n"
    "\n"
    "        threads each: ^{ <_thread>\n"
    "            _thread wait\n"
    "            result[j] = [_thread result]\n"
    "            j++\n"
    "        }\n"
    "\n"
    "        return result\n"
    "    }\n"
    "}\n"
;