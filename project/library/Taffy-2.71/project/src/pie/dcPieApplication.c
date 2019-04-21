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

//  ____ ___ _____
// |  _ \_ _| ____|
// | |_) | ||  _|
// |  __/| || |___
// |_|  |___|_____|
//

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#include "dcThread.h"

#include "CompiledCopyright.h"
#include "CompiledPieUsage.h"

#include "CompiledHelpSystem.h"
#include "CompiledImportHelperHelpTopic.h"
#include "CompiledMainHelpTopic.h"
#include "CompiledGnuLesserTermsAndConditionsHelpTopic.h"
#include "CompiledGnuTermsAndConditionsHelpTopic.h"
#include "CompiledGnuWarrantyHelpTopic.h"

#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcCommandLineArguments.h"
#include "dcDefines.h"
#include "dcError.h"
#include "dcExceptions.h"
#include "dcFileManagement.h"
#include "dcGarbageCollector.h"
#include "dcHash.h"
#include "dcIOClass.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcLog.h"
#include "dcMemory.h"
#include "dcNilClass.h"
#include "dcNode.h"
#include "dcNodeEvaluator.h"
#include "dcReadline.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringEvaluator.h"
#include "dcStringClass.h"
#include "dcSystem.h"
#include "dcTaffyApplication.h"
#include "dcTaffyCommandLineArguments.h"
#include "dcPieApplication.h"
#include "dcPieLineEvaluator.h"

#ifdef DISABLE_READLINE
#error "readline is disabled, pie cannot be built"
#endif

// define stubs for editline
#if HAVE_EDITLINE_READLINE_H
void rl_free_line_state(void) {}
void rl_cleanup_after_signal(void) {}
void clear_history(void) {}
#endif

static bool sStop = false;
static pthread_mutex_t sStopMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t sParentId;
static sigset_t sSignalMask;
static char *sTempPrompt;
static jmp_buf sRedoPrompt;
static dcPieLineEvaluator *sPieLineEvaluator;
static bool sUseColor = false;
static dcList *sHistory;
static dcList *sLoadedHistory;

// these brackets are needed for readline
#define START_IGNORE "\001"
#define END_IGNORE   "\002"

#define DEFINE_COLOR(color) START_IGNORE color END_IGNORE

// colors
#define CNORMAL  DEFINE_COLOR("\x1B[0m")
#define CRED     DEFINE_COLOR("\x1B[31m")
#define CGREEN   DEFINE_COLOR("\x1B[32m")
#define CYELLOW  DEFINE_COLOR("\x1B[33m")
#define CBLUE    DEFINE_COLOR("\x1B[34m")
#define CMAGENTA DEFINE_COLOR("\x1B[35m")
#define CCYAN    DEFINE_COLOR("\x1B[36m")
#define CWHITE   DEFINE_COLOR("\x1B[37m")
#define RESET    DEFINE_COLOR("\e[0m")

// editline seems to not have rl_completion_matches
#ifdef HAVE_READLINE_READLINE_H
static char* completionGenerator(const char* _text, int _state)
{
    dcNode *object = (dcNodeEvaluator_findObject
                      (dcSystem_getCurrentNodeEvaluator(),
                       _text,
                       false));
    if (object != NULL)
    {
        dcClass *klass = dcClass_getClass(object);
        const dcScope *metaScopes[] = {klass->classTemplate->metaScope, NULL};
        const dcScope *objectScopes[] = {klass->scope,
                                         klass->classTemplate->scope,
                                         NULL};
        const dcScope **scopes = (dcClass_isObject(object)
                                  ? objectScopes
                                  : metaScopes);
        int i = 0;

        for (i = 0; scopes[i] != NULL; i++)
        {
            int index = 0;
            dcHashIterator *it = dcHash_createIterator(scopes[i]->objects);
            dcNode *scopeDataNode;

            while ((scopeDataNode = dcHashIterator_getNextValue(it))
                   != NULL)
            {
                dcScopeData *scopeData = CAST_SCOPE_DATA(scopeDataNode);

                if ((scopeData->flags & SCOPE_DATA_PUBLIC) != 0
                    && (scopeData->flags & SCOPE_DATA_METHOD) != 0)
                {
                    if (index == _state)
                    {
                        return strdup(scopeData->name);
                    }

                    index++;
                }
            }
        }
    }

    return NULL;
}

static char **completionToTheMax(const char * _text, int _start, int _end)
{
    return rl_completion_matches((char *)_text, &completionGenerator);
}
#endif

static void printColor(const char *_color)
{
    if (sUseColor)
    {
        dcIOClass_printFormat(_color);
    }
}

static void lockStop(void)
{
    pthread_mutex_lock(&sStopMutex);
}

static void unlockStop(void)
{
    pthread_mutex_unlock(&sStopMutex);
}

static void fillMask(sigset_t *_mask)
{
    sigfillset(_mask);
    sigdelset(_mask, SIGTSTP);  // stop
    sigdelset(_mask, SIGCONT);  // continue
    sigdelset(_mask, SIGHUP);   // hangup
    sigdelset(_mask, SIGUSR1);  // user signal
    sigdelset(_mask, SIGCHLD);  // child status has changed
    sigdelset(_mask, SIGWINCH); // window changed
}

static void *signalHandlerThread(void *_data)
{
    sigset_t mask;
    fillMask(&mask);
    dcError_assert(pthread_sigmask(SIG_BLOCK, &mask, NULL) == 0);
    bool localStop = false;

    while (! localStop)
    {
        int caught;
        dcError_assert(sigwait(&mask, &caught) == 0);

        // stop might have been set while sleeping
        lockStop();
        localStop = sStop;
        unlockStop();

        if (! localStop)
        {
            if (caught == SIGINT
                || caught == SIGABRT
                || caught == SIGSEGV)
            {
                dcIOClass_printFormat("^C\n");

                if (dcSystem_isLive())
                {
                    // prime the system and node evaluators for
                    // execution abortion
                    dcSystem_abortExecution(caught);

                    // send the abort signal to the parent process
                    pthread_kill(sParentId, SIGUSR1);
                }
            }
            else
            {
                TAFFY_DEBUG(dcIOClass_printFormat("received uncatchable signal "
                                                  "(this is a bug): (%u), %s\n",
                                                  caught,
                                                  sys_siglist[caught]));
            }
        }

        lockStop();
        localStop = sStop;
        unlockStop();
    }

    return NULL;
}

static void dcPieApplication_displayHelp(void)
{
    dcIOClass_printFormat("%s", __compiledPieUsage);
}

static void unblock(void)
{
    if (pthread_sigmask(SIG_UNBLOCK, &sSignalMask, NULL) != 0)
    {
        perror("sigset_t UNBLOCK in dcPieApplication.c");
    }
}

static void block(void)
{
    if (pthread_sigmask(SIG_BLOCK, &sSignalMask, NULL) != 0)
    {
        perror("sigset_t BLOCK in dcPieApplication.c");
    }
}

static char *myReadLine(void)
{
    block();
    sTempPrompt = dcPieLineEvaluator_getPrompt(sPieLineEvaluator, "pie", true);

    if (sUseColor)
    {
        sTempPrompt = dcLexer_sprintf(CYELLOW "%s" RESET, sTempPrompt);
    }

    unblock();
    char *line = NULL;

    do
    {
        line = readline(sTempPrompt);

        if (line == NULL)
        {
            dcIOClass_printFormat("\n");
        }
    } while (line == NULL);

    dcMemory_free(sTempPrompt);
    return line;
}

static void sigUsr1Handler(int _signal)
{
    dcMemory_free(sTempPrompt);
    dcPieLineEvaluator_resetState(sPieLineEvaluator);
    rl_free_line_state();
    rl_cleanup_after_signal();
    longjmp(sRedoPrompt, 1);
}

static bool nonBlank(const char *_line)
{
    size_t i;
    size_t length = strlen(_line);
    bool result = false;

    for (i = 0; i < length; i++)
    {
        if (_line[i] != ' '
            && _line[i] != '\t'
            && _line[i] != '\n')
        {
            result = true;
            break;
        }
    }

    return (result);
}

static void executeLine(const char *_line)
{
    // execute the line //
    char *result = dcPieLineEvaluator_evaluateLine
        (sPieLineEvaluator, _line, NULL, 0, NULL);

    if (result != NULL)
    {
        // print the result //
        printColor(CBLUE);
        dcIOClass_printFormat("==> ");
        const char *baseColor = CCYAN;
        printColor(baseColor);

        // colorize the strings
        if (sUseColor)
        {
            size_t i;
            size_t length = strlen(result);
            bool inString = false;

            for (i = 0; i < length; i++)
            {
                if (result[i] == '"')
                {
                    inString = ! inString;

                    if (inString)
                    {
                        printColor(CGREEN);
                    }
                    else
                    {
                        printColor(baseColor);
                    }
                }

                dcIOClass_printFormat("%c", result[i]);
            }

            printColor(RESET);
            dcIOClass_printFormat("\n");
        }
        else
        {
            dcIOClass_printFormat("%s\n", result);
        }

        // we're done with color, reset
        printColor(RESET);
        dcMemory_free(result);
    }
}

static void execute(void)
{
    dcNodeEvaluator *nodeEvaluator = dcSystem_getCurrentNodeEvaluator();
    dcNodeEvaluator_resetState(nodeEvaluator);

    //
    // signal handling initialization
    //
    // set up the jump for control-c (SIGUSR1) handling
    // unblock the signal mask so we can hear it
    // set up the handler for the SIGUSR1 signal
    //
    sigemptyset(&sSignalMask);
    sigaddset(&sSignalMask, SIGUSR1);
    setjmp(sRedoPrompt); // <-- jumps to here
    unblock();
    signal(SIGUSR1, sigUsr1Handler);

    // read the prompt
    char *lineRead = myReadLine();

    while (strcmp(lineRead, "quit") != 0
           && strcmp(lineRead, "exit") != 0)
    {
        block();

        //
        // control-c (SIGUSR1) is now blocked
        // if control-c is hit, pie sends an abort exception, via System,
        // to all running dcNodeEvaluatorS which forces execution haltation
        //

        if (nonBlank(lineRead))
        {
            // add the line to readline's history
            add_history(lineRead);

            // add the line to the local history for save purposes
            dcList_push(sHistory,
                        dcString_createNodeWithString(lineRead, true));

            executeLine(lineRead);

            if (nodeEvaluator->exit)
            {
                break;
            }

            if (sLoadedHistory != NULL)
            {
                dcListElement *that;
                const char save[] = "%save_history";
                const char load[] = "%load_history";

                for (that = sLoadedHistory->head;
                     that != NULL;
                     that = that->next)
                {
                    const char *string = CAST_STRING(that->object)->string;

                    if (strncmp(string, save, strlen(save)) != 0
                        && strncmp(string, load, strlen(load)) != 0)
                    {
                        dcIOClass_printFormat("Loaded(%s)\n", string);
                        executeLine(string);
                    }
                }

                dcList_free(&sLoadedHistory, DC_DEEP);
            }

            // control-c (SIGUSR1) is now unblocked
            free(lineRead);
        }

        unblock();
        lineRead = myReadLine();
    }

    clear_history();
    // don't use dcMemory_free(), since lineRead was created using readline's
    // readline()
    free(lineRead);
}

static int goWithArguments(dcCommandLineArguments *_arguments)
{
    //
    // setup the signal handling!
    //
    sigset_t mask;
    pthread_t signalThreadId = {0};

    // create the system //
    if (dcSystem_createWithArguments(_arguments) == NULL)
    {
        // catastrophic failure
        return 1;
    }

    sHistory = dcList_create();
    sParentId = pthread_self();

    //
    // if we can handle signals (--noSignalCatch was not set), then handle them
    //
    if (! dcTaffyCommandLineArguments_getNoSignalCatch(_arguments))
    {
        //
        // set up the thread that hears and handles all other signals
        //
        pthread_create(&signalThreadId, NULL, signalHandlerThread, NULL);

        fillMask(&mask);

        if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0)
        {
            perror("sigmask");
        }
    }

    // create the line evaluator //
    sPieLineEvaluator = dcPieLineEvaluator_create();

    // print the copyright //
    dcIOClass_printFormat("%s", __compiledCopyright);

    //
    // <load> the help system
    //
    dcPieLineEvaluator_loadHelp();

    const char *helpStrings[] =
        {
            __compiledGnuLesserTermsAndConditionsHelpTopic,
            __compiledGnuTermsAndConditionsHelpTopic,
            __compiledImportHelperHelpTopic,
            __compiledMainHelpTopic,
            __compiledHelpSystem
        };

    assert(dcStringEvaluator_evalStringArray(helpStrings,
                                             dcTaffy_countOf(helpStrings))
           != NULL);
    //
    // </load>
    //

    // go loop! //
    execute();

    //
    // stop the signal handler thread
    //
    lockStop();
    sStop = true;
    unlockStop();
    pthread_kill(signalThreadId, SIGINT);
    pthread_join(signalThreadId, NULL);

    // free up //
    dcList_free(&sHistory, DC_DEEP);
    dcList_free(&sLoadedHistory, DC_DEEP);
    dcPieLineEvaluator_free(&sPieLineEvaluator);
    dcSystem_free();
    return 0;
}

int dcPieApplication_go(int _argc, char **_argv)
{
    dcMemory_initialize();

    dcGarbageCollector_create();
    int result = 0;

    // parse the command-line arguments //
    dcCommandLineArguments *arguments =
        dcTaffyCommandLineArguments_parseAndCreate(_argc, _argv);

    if (arguments == NULL)
    {
        // error
        result = 1;
        dcPieApplication_displayHelp();
    }
    else if (dcTaffyCommandLineArguments_displayHelp(arguments))
    {
        dcPieApplication_displayHelp();
    }
    else if (dcTaffyCommandLineArguments_displayVersion(arguments))
    {
        dcSystem_displayVersion("Pie");
    }
    else
    {
        if (dcTaffyCommandLineArguments_useColor(arguments))
        {
#ifdef TAFFY_APPLE
            sUseColor = dcTaffyCommandLineArguments_useColor(arguments);
#else
            dcIOClass_printFormat("Error: color is not supported "
                                  "on your platform\n");
            exit(1);
#endif
        }

// editline seems to not have this
#ifdef HAVE_READLINE_READLINE_H
        rl_attempted_completion_function = completionToTheMax;
#endif

        result = goWithArguments(arguments);
    }

    dcMemory_deinitialize();
    return result;
}

char *dcPieApplication_saveHistory(const char *_filename)
{
    dcString *marshalled = NULL;
    FILE *file = fopen(_filename, "w");
    char *result = NULL;

    if (file == NULL)
    {
        result = dcLexer_sprintf("Error: Can't open file: %s", _filename);
        goto kickout;
    }

    marshalled = dcList_marshall(sHistory, NULL);

    if (marshalled == NULL)
    {
        result = dcLexer_sprintf("Error: Can't marshall history, "
                                 "stopping %save_history.");
        goto kickout;
    }

    if (fwrite(marshalled->string, 1, marshalled->length, file)
        == marshalled->length)
    {
        result = dcLexer_sprintf("Saved %u bytes to file: %s",
                                 marshalled->length,
                                 _filename);
    }
    else
    {
        result = dcLexer_sprintf("Error: Failed to write to file.");
    }

kickout:
    dcString_free(&marshalled, DC_DEEP);
    fclose(file);
    return result;
}

char *dcPieApplication_loadHistory(const char *_filename)
{
    FILE *file = fopen(_filename, "r");
    char *result = NULL;
    dcString *input = NULL;
    assert(sLoadedHistory == NULL);

    if (file == NULL)
    {
        result = dcLexer_sprintf("Error: Can't open file: %s", _filename);
        goto kickout;
    }

    input = dcFileManagement_extractAllInputFromFile(file);

    if (input == NULL)
    {
        result = dcLexer_sprintf("Error: Can't read from file: %s", _filename);
        goto kickout;
    }

    sLoadedHistory = dcList_unmarshall(input);

    if (sLoadedHistory == NULL)
    {
        result = dcLexer_sprintf("Error: Invalid file: %s", _filename);
        goto kickout;
    }

kickout:
    dcString_free(&input, DC_DEEP);
    fclose(file);
    return result;
}

void *dcPieApplication_executeHelpString(void *_argument)
{
    char *helpString =
        (dcLexer_sprintf
         ("[[org.taffy.help.HelpSystem getInstance] executeLine: \"%s\"]",
          (char*)_argument));
    dcStringEvaluator_evalString(helpString,
                                 "<console>",
                                 STRING_EVALUATOR_HANDLE_EXCEPTION);
    dcMemory_free(helpString);
    return dcNode_synchronizedDisplay(dcNilClass_getInstance());
}
