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

#ifndef __DC_FUNCTION_REPEATER_H__
#define __DC_FUNCTION_REPEATER_H__

#include "dcDefines.h"

typedef bool (dcFunctionRepeaterExecuteFunction)(struct dcNode_t *_receiver,
                                                 struct dcNode_t *_result,
                                                 struct dcNode_t **_token);

bool dcFunctionRepeater_executeHelper
    (struct dcNode_t *_receiver,
     struct dcNode_t *_from,
     struct dcNode_t *_to,
     struct dcHash_t *_memory,
     dcFunctionRepeaterExecuteFunction *_function,
     struct dcNode_t **_token);

bool dcFunctionRepeater_execute(struct dcNode_t *_receiver,
                                struct dcArray_t *_arguments,
                                struct dcHash_t *_memory,
                                dcFunctionRepeaterExecuteFunction *_function,
                                struct dcNode_t **_token);

#endif
