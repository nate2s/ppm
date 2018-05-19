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

#ifndef __DC_SERIES_CLASS_H__
#define __DC_SERIES_CLASS_H__

#include "dcDefines.h"

// creating //
struct dcNode_t *dcSeriesClass_createNode(struct dcNode_t *_body,
                                          struct dcMethodHeader_t *_header,
                                          bool _object);

struct dcNode_t *dcSeriesClass_createObject(struct dcNode_t *_body,
                                            struct dcMethodHeader_t *_header);

// standard functions //
COPY_FUNCTION(dcSeriesClass_copyNode);
FREE_FUNCTION(dcSeriesClass_freeNode);
GET_TEMPLATE_FUNCTION(dcSeriesClass_getTemplate);
INITIALIZE_FUNCTION(dcSeriesClass_initialize);
REGISTER_FUNCTION(dcSeriesClass_registerNode);
DO_GRAPH_OPERATION_FUNCTION(dcSeriesClass_doGraphOperation);

#define SERIES_CLASS_MARSHALL_SIZE 5

#define SERIES_PACKAGE_NAME MATHS_PACKAGE_NAME
#define SERIES_CLASS_NAME "Series"

#endif
