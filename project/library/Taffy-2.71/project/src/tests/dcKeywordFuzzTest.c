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

#include <assert.h>
#include <string.h>

#include "dcCommandLineArguments.h"
#include "dcError.h"
#include "dcLexer.h"
#include "dcFileEvaluator.h"
#include "dcGarbageCollector.h"
#include "dcIOClass.h"
#include "dcLexer.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcTestUtilities.h"
#include "dcNodeEvaluator.h"
#include "dcString.h"
#include "dcStringEvaluator.h"

void outputIt(const char *_output)
{
    // silencio....
}

extern const SymbolData sKeywordDatas[];
extern const SymbolData sTokenDatas[];

static void testIt(uint32_t _length,
                   const char *_word,
                   uint32_t _wordLength)
{
    dcString *string = dcString_createWithLength(_length);
    uint32_t i = 0;

    for (i = 0; i < 5000; i++)
    {
        uint32_t j;

        for (j = 0; j < string->length - 1; j++)
        {
            if (j < _wordLength)
            {
                string->string[j] = _word[j];
            }
            else if (j == _wordLength)
            {
                if (rand() % 2 == 0)
                {
                    string->string[j] = ' ';
                }
                else
                {
                    string->string[j] = rand();
                }
            }
            else
            {
                string->string[j] = rand();
            }
        }

        // null terminate
        string->string[j] = 0;

        dcNode *result = (dcStringEvaluator_evalString
                          (string->string,
                           "dcKeywordFuzztest.c",
                           STRING_EVALUATOR_HANDLE_EXCEPTION));
        dcNode_tryFree(&result, DC_DEEP);

        if (i % 10000 == 0)
        {
            fprintf(stderr, ".");
        }
    }

    dcString_free(&string, DC_DEEP);
}

static void testKeywordDatas(void)
{
    uint32_t i;

    for (i = 0; sKeywordDatas[i].description != NULL; i++)
    {
        testIt(10, sKeywordDatas[i].name, strlen(sKeywordDatas[i].name));
    }
}

static void testTokenDatas(void)
{
    uint32_t i;

    for (i = 0; sTokenDatas[i].description != NULL; i++)
    {
        testIt(10,
               sTokenDatas[i].description,
               strlen(sTokenDatas[i].description));
    }
}

static const dcTestFunctionMap sTests[] =
{
    {"Test Keyword Datas", testKeywordDatas},
    {"Test Token Datas",   testTokenDatas},
    {NULL}
};

int main(int _argc, char **_argv)
{
    dcMemory_initialize();
    dcSystem_create();
    srand(0);
    dcIOClass_setOutputFunction(&outputIt);
    dcTestUtilities_go("Keyword Fuzz Test", _argc, _argv, NULL, sTests, false);
    dcSystem_free();
    dcMemory_deinitialize();
    return 0;
}
