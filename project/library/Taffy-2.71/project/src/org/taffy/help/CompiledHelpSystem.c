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

const char *__compiledHelpSystem =    "package org.taffy.help\n"
    "\n"
    "import org.taffy.help.MainHelpTopic\n"
    "import org.taffy.core.io.IO\n"
    "\n"
    "class HelpSystem\n"
    "{\n"
    "    @@theInstance\n"
    "\n"
    "    @mainHelpTopic\n"
    "\n"
    "    (@@) getInstance\n"
    "    #synchronized\n"
    "    {\n"
    "        if (@@theInstance == nil)\n"
    "        {\n"
    "            @@theInstance = new HelpSystem\n"
    "        }\n"
    "\n"
    "        return (@@theInstance)\n"
    "    }\n"
    "\n"
    "    (@) init\n"
    "    {\n"
    "        @mainHelpTopic = new MainHelpTopic\n"
    "    }\n"
    "\n"
    "    (@) executeLine: _line\n"
    "    {\n"
    "        splits = [_line split: \" \"]\n"
    "        this = @mainHelpTopic\n"
    "        thisTopic = nil\n"
    "\n"
    "        if ([splits size] > 1\n"
    "            or ([splits size] == 1\n"
    "                and splits[0] != \"\"))\n"
    "        {\n"
    "            splits each: ^{ <value>\n"
    "                if (thisTopic == nil)\n"
    "                {\n"
    "                    thisTopic = this\n"
    "                }\n"
    "\n"
    "                thisTopic = [[thisTopic subtopics] objectForKey: value]\n"
    "\n"
    "                if (thisTopic == nil)\n"
    "                {\n"
    "                    io putLine: \"Unable to find help topic for '#[value]'\"\n"
    "                    break\n"
    "                }\n"
    "            }\n"
    "\n"
    "            if (thisTopic != nil)\n"
    "            {\n"
    "                // success!\n"
    "                io putLine: \"#[thisTopic]\"\n"
    "            }\n"
    "        }\n"
    "        else\n"
    "        {\n"
    "            io putLine: \"#[@mainHelpTopic]\"\n"
    "        }\n"
    "    }\n"
    "}\n"
;