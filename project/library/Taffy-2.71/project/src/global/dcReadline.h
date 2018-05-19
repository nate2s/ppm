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

#ifndef __DC_READLINE_H__
#define __DC_READLINE_H__

#include "dcDefines.h"

#if HAVE_EDITLINE_READLINE_H == 1
#    include <editline/readline.h>
#    ifndef TAFFY_APPLE
#        include <editline/history.h>
#    endif
#elif HAVE_READLINE_READLINE_H == 1
#    include <readline/readline.h>
#    include <readline/history.h>
#endif

#endif
