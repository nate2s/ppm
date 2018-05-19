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

const char *__compiledInvalidArraySizeException =    "package org.taffy.core.exception\n"
    "\n"
    "class InvalidArraySizeException(Exception)\n"
    "{\n"
    "    @expected, @rw\n"
    "    @given, @rw\n"
    "\n"
    "    (@@) expected: _expected given: _given\n"
    "    {\n"
    "        exception = new InvalidArraySizeException\n"
    "        exception setExpected: _expected\n"
    "        exception setGiven: _given\n"
    "        exception setData: (\"invalid array size, expected: #[_expected] \"\n"
    "                            + \"given: #[_given]\")\n"
    "        return (exception)\n"
    "    }\n"
    "}\n"
;