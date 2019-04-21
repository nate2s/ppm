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

const char *__compiledMainHelpTopic =    "package org.taffy.help\n"
    "\n"
    "import org.taffy.help.warranty.*\n"
    "\n"
    "class MainHelpTopic(HelpTopic)\n"
    "{\n"
    "    (@) init\n"
    "    {\n"
    "        [super init]\n"
    "        [self loadTitle: \"Taffy Help\"]\n"
    "\n"
    "        @description = (\"Type %help followed by a topic to view it.\n\n\"\n"
    "                        + \"See www.taffy-lang.org for help with the Taffy \"\n"
    "                        + \"language.\")\n"
    "        [self loadSubtopics: [new ImportHelperHelpTopic,\n"
    "                              new GnuTermsAndConditionsHelpTopic,\n"
    "                              new GnuLesserTermsAndConditionsHelpTopic,\n"
    "                              new GnuWarrantyHelpTopic,\n"
    "                              new IcuCopyrightHelpTopic]]\n"
    "    }\n"
    "}\n"
;