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

#ifndef __DC_LOG_H__
#define __DC_LOG_H__

#include "dcDefines.h"

#define NO_LOG_TYPE                            0
#define GARBAGE_COLLECTOR_LOG                  BITS(0)
#define GARBAGE_COLLECTOR_TICK_LOG             BITS(1)
#define KERNEL_LOG                             BITS(2)
#define LEXER_LOG                              BITS(3)
#define LEXER_SEARCH_LOG                       BITS(4)
#define NODE_EVALUATOR_LOG                     BITS(5)
#define PARSER_LOG                             BITS(6)
#define CLASS_MANAGER_LOG                      BITS(7)
#define FLAT_ARITHMETIC_LOG                    BITS(8)
#define FLAT_ARITHMETIC_DERIVATION_LOG         BITS(9)
#define FLAT_ARITHMETIC_INTEGRATION_LOG        BITS(10)
#define FLAT_ARITHMETIC_PARSE_LOG              BITS(11)
#define FLAT_ARITHMETIC_SUBSTITUTION_LOG       BITS(12)
#define FLAT_ARITHMETIC_SUBSTITUTION_CACHE_LOG BITS(13)
#define FLAT_ARITHMETIC_MERGE_LOG              BITS(14)
#define FLAT_ARITHMETIC_VERBOSE_LOG            BITS(15)
#define FLAT_ARITHMETIC_FACTOR_LOG             BITS(16)
#define FLAT_ARITHMETIC_REMOVE_LOG             BITS(17)
#define FLAT_ARITHMETIC_CHOOSE_LOG             BITS(18)
#define FLAT_ARITHMETIC_TICK_LOG               BITS(19)
#define FLAT_ARITHMETIC_TEST_LOG               BITS(20)
#define FLAT_ARITHMETIC_DIVIDE_LOG             BITS(21)
#define FLAT_ARITHMETIC_CANCEL_LOG             BITS(22)
#define FLAT_ARITHMETIC_TRIGONOMETRY_LOG       BITS(23)
#define FLAT_ARITHMETIC_SHRINK_LOG             BITS(24)
#define FLAT_ARITHMETIC_SOLVE_FOR_X_LOG        BITS(25)
#define FLAT_ARITHMETIC_QUADRATIC_LOG          BITS(26)
#define FLAT_ARITHMETIC_DERIVE_CACHE_LOG       BITS(27)
#define FLAT_ARITHMETIC_SHRINK_ITERATION_LOG   BITS(28)

typedef uint32_t dcLogType;

//
// configuring
//
bool dcLog_configureFromList(const struct dcList_t *_stringList, bool _yesno);
void dcLog_configure(dcLogType _type, bool _yesno);
bool dcLog_configureFromString(const char *_type, bool _yesno);
bool dcLog_configureFromCommandLineArguments
    (struct dcCommandLineArguments_t *_arguments);

// is a log type enabled?
bool dcLog_isEnabled(dcLogType _type);

// logging
void dcLog_log(dcLogType _type, const char *_format, ...);
void dcLog_logLine(dcLogType _type, const char *_format, ...);
void dcLog_append(dcLogType _type, const char *_format, ...);

#endif // __DC_LOG_H__
