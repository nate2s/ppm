##
## This file is part of Taffy, a mathematical programming language.
## Taffy Copyright (C) 2016-2017 Arithmagic, LLC (taffy@arithmagic.com)
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

ACLOCAL_AMFLAGS = -I m4

#
# sorted in order of complexity and use of taffy utilities
#
if COMPILE_TESTS
  TEST_PROGS = memoryTest \
               arrayTest \
               hashTest \
               listTest \
               pairTest \
               heapTest \
               stringTest \
               methodCallPrintTest \
               characterGraphTest \
               commandLineArgumentsTest \
               commandLineArgumentsFuzzTest \
               lexerTest \
               numberTest \
               fileManagementTest \
               packageTest \
               parserTest \
               scopeTest \
               graphDataTest \
               stringCacheTest \
               classManagerTest \
               nodeEvaluatorTest \
               stringEvaluatorTest \
               importTest \
               garbageCollectorTest \
               assignmentTest \
               bootstrapTest \
               classTest \
               objectStackTest \
               marshallTest \
               marshallFuzzTest \
               threadTest \
               flatArithmeticTest \
               taffyFuzzTest \
               keywordFuzzTest
else
  TEST_PROGS =
endif

#
# set up pie targets!
#
if COMPILE_PIE
  PIE_PROGS = pie
  PIE_HELP_FILES = src/org/taffy/help/CompiledMainHelpTopic.c \
                   src/org/taffy/help/warranty/CompiledGnuLesserTermsAndConditionsHelpTopic.c \
                   src/org/taffy/help/warranty/CompiledGnuLesserTermsAndConditionsHelpTopic.h \
                   src/org/taffy/help/warranty/CompiledGnuTermsAndConditionsHelpTopic.c \
                   src/org/taffy/help/warranty/CompiledGnuTermsAndConditionsHelpTopic.h \
                   src/org/taffy/help/warranty/CompiledGnuWarrantyHelpTopic.c \
                   src/org/taffy/help/warranty/CompiledGnuWarrantyHelpTopic.h

  PIE_BASE_SOURCES = src/pie/dcPieApplication.c \
                     src/pie/dcPieApplication.h \
                     src/pie/text/CompiledPieUsage.c \
                     src/pie/text/CompiledPieUsage.h

  taffyAndPieArgumentsTest_SOURCES = src/tests/dcTaffyAndPieArgumentsTest.c \
                                     src/taffy/dcTaffyApplication.c \
                                     src/taffy/dcTaffyApplication.h \
                                     $(PIE_BASE_SOURCES) \
                                     $(PIE_HELP_FILES) \
                                     $(TEST_SOURCES)
  taffyAndPieArgumentsTest_LDADD = libtaffy.la libtaffyPie.la

  PIE_SOURCES = src/pie/dcPieMain.c \
                $(PIE_BASE_SOURCES) \
                $(PIE_HELP_FILES)

  pieTest_SOURCES = src/tests/dcPieTest.c \
                    $(PIE_BASE_SOURCES) \
                    $(PIE_HELP_FILES) \
                    $(TEST_SOURCES)

  pieTest_LDADD = libtaffy.la libtaffyPie.la

  pieFuzzTest_SOURCES = src/tests/dcPieFuzzTest.c \
                        $(PIE_BASE_SOURCES) \
                        $(PIE_HELP_FILES) \
                        $(TEST_SOURCES)

  pieFuzzTest_LDADD = libtaffy.la libtaffyPie.la

  libtaffyPie_la_SOURCES = src/evaluator/dcPieLineEvaluator.c \
                           src/evaluator/dcPieLineEvaluator.h \
                           src/org/taffy/help/CompiledHelpTopic.c \
                           src/org/taffy/help/CompiledHelpTopic.h \
                           src/org/taffy/help/CompiledHelpSystem.c \
                           src/org/taffy/help/CompiledHelpSystem.h \
                           src/org/taffy/help/CompiledImportHelperHelpTopic.c \
                           src/org/taffy/help/CompiledImportHelperHelpTopic.h \
                           src/org/taffy/help/warranty/CompiledIcuCopyrightHelpTopic.c \
                           src/org/taffy/help/warranty/CompiledIcuCopyrightHelpTopic.h

  libtaffyPie_la_LDFLAGS = ${MY_LDFLAGS}
  PIE_TEST_PROGS = taffyAndPieArgumentsTest pieTest pieFuzzTest
  pie_SOURCES = $(PIE_SOURCES)

  pie_LDADD = libtaffy.la libtaffyPie.la
if COMPILE_TESTS
  TEST_PROGS += $(PIE_TEST_PROGS)
endif
endif

#
# if we're compiling for debug, use appropriate flags
#

if DEBUG_BUILD
  DEBUG_OR_NOT_FILES = src/global/dcLog.c
else
  DEBUG_OR_NOT_FILES =
endif

MY_COMPILE_FLAGS = -Wall \
                   -Wshadow \
                   -Wsign-compare \
                   -D_GNU_SOURCE \
                   -Isrc \
                   -Isrc/class \
                   -Isrc/class/special \
                   -Isrc/class/org/taffy/core/exception \
                   -Isrc/container \
                   -Isrc/evaluator \
                   -Isrc/global \
                   -Isrc/global/text \
                   -Isrc/graph \
                   -Isrc/io \
                   -Isrc/maths \
                   -Isrc/method \
                   -Isrc/scope \
                   -Isrc/taffy \
                   -Isrc/pie \
                   -Isrc/pie/text \
                   -Isrc/taffy/text \
                   -Isrc/third_party/decNumber \
                   -Isrc/org/taffy/help \
                   -Isrc/org/taffy/help/warranty

AM_CPPFLAGS=${MY_COMPILE_FLAGS}
VERSION=2:53:0
MY_LDFLAGS=-version-info ${VERSION}

#
# main executables
#

TEST_SOURCES = src/tests/dcTestUtilities.c src/tests/dcTestUtilities.h

libtaffy_la_SOURCES = ${taffy_main_SOURCES}
libtaffy_la_LDFLAGS = ${MY_LDFLAGS}

TAFFY_BASE_SOURCES = src/taffy/dcTaffyApplication.c \
                     src/taffy/dcTaffyApplication.h
taffy_SOURCES = src/taffy/dcTaffyMain.c $(TAFFY_BASE_SOURCES)
taffy_LDADD = libtaffy.la

#
# Tests
#
arrayTest_SOURCES = src/tests/dcArrayTest.c $(TEST_SOURCES)
arrayTest_LDADD = libtaffy.la

assignmentTest_SOURCES = src/tests/dcAssignmentTest.c $(TEST_SOURCES)
assignmentTest_LDADD = libtaffy.la

bootstrapTest_SOURCES = src/tests/dcBootstrapTest.c $(TEST_SOURCES)
bootstrapTest_LDADD = libtaffy.la

characterGraphTest_SOURCES = src/tests/dcCharacterGraphTest.c $(TEST_SOURCES)
characterGraphTest_LDADD = libtaffy.la

classTest_SOURCES = src/tests/dcClassTest.c src/tests/dcTestClass.c $(TEST_SOURCES)
classTest_LDADD = libtaffy.la

classManagerTest_SOURCES = src/tests/dcClassManagerTest.c src/tests/dcTestClass.c $(TEST_SOURCES)
classManagerTest_LDADD = libtaffy.la

commandLineArgumentsFuzzTest_SOURCES = src/tests/dcCommandLineArgumentsFuzzTest.c \
                                       $(TEST_SOURCES)
commandLineArgumentsFuzzTest_LDADD = libtaffy.la

importTest_SOURCES = src/tests/dcImportTest.c \
                     src/tests/dcImportAndPackageTest.c \
                     $(TEST_SOURCES)
importTest_LDADD = libtaffy.la

methodCallPrintTest_SOURCES = src/tests/dcMethodCallPrintTest.c $(TEST_SOURCES)
methodCallPrintTest_LDADD = libtaffy.la

commandLineArgumentsTest_SOURCES = src/tests/dcCommandLineArgumentsTest.c $(TEST_SOURCES)
commandLineArgumentsTest_LDADD = libtaffy.la

fileManagementTest_SOURCES = src/tests/dcFileManagementTest.c $(TEST_SOURCES)
fileManagementTest_LDADD = libtaffy.la

flatArithmeticTest_SOURCES = src/tests/dcFlatArithmeticTest.c $(TEST_SOURCES)
flatArithmeticTest_LDADD = libtaffy.la

garbageCollectorTest_SOURCES = src/tests/dcGarbageCollectorTest.c $(TEST_SOURCES)
garbageCollectorTest_LDADD = libtaffy.la

graphDataTest_SOURCES = src/tests/dcGraphDataTest.c $(TEST_SOURCES)
graphDataTest_LDADD = libtaffy.la

hashTest_SOURCES = src/tests/dcHashTest.c $(TEST_SOURCES)
hashTest_LDADD = libtaffy.la

heapTest_SOURCES = src/tests/dcHeapTest.c $(TEST_SOURCES)
heapTest_LDADD = libtaffy.la

keywordFuzzTest_SOURCES = src/tests/dcKeywordFuzzTest.c \
                          $(TAFFY_BASE_SOURCES) \
                          $(TEST_SOURCES)
keywordFuzzTest_LDADD = libtaffy.la

lexerTest_SOURCES = src/tests/dcLexerTest.c $(TEST_SOURCES)
lexerTest_LDADD = libtaffy.la

listTest_SOURCES = src/tests/dcListTest.c $(TEST_SOURCES)
listTest_LDADD = libtaffy.la

marshallTest_SOURCES = src/tests/dcMarshallTest.c $(TEST_SOURCES)
marshallTest_LDADD = libtaffy.la

marshallFuzzTest_SOURCES = src/tests/dcMarshallFuzzTest.c $(TEST_SOURCES)
marshallFuzzTest_LDADD = libtaffy.la

memoryTest_SOURCES = src/tests/dcMemoryTest.c $(TEST_SOURCES)
memoryTest_LDADD = libtaffy.la

nodeEvaluatorTest_SOURCES = src/tests/dcNodeEvaluatorTest.c $(TEST_SOURCES)
nodeEvaluatorTest_LDADD = libtaffy.la

numberTest_SOURCES = src/tests/dcNumberTest.c $(TEST_SOURCES)
numberTest_LDADD = libtaffy.la

objectStackTest_SOURCES = src/tests/dcObjectStackTest.c src/tests/dcTestClass.c $(TEST_SOURCES)
objectStackTest_LDADD = libtaffy.la

packageTest_SOURCES = src/tests/dcPackageTest.c \
                      src/tests/dcImportAndPackageTest.c \
                      $(TEST_SOURCES)
packageTest_LDADD = libtaffy.la

pairTest_SOURCES = src/tests/dcPairTest.c $(TEST_SOURCES)
pairTest_LDADD = libtaffy.la

parserTest_SOURCES = src/tests/dcParserTest.c $(TEST_SOURCES)
parserTest_LDADD = libtaffy.la

scopeTest_SOURCES = src/tests/dcScopeTest.c $(TEST_SOURCES)
scopeTest_LDADD = libtaffy.la

stringTest_SOURCES = src/tests/dcStringTest.c $(TEST_SOURCES)
stringTest_LDADD = libtaffy.la

stringCacheTest_SOURCES = src/tests/dcStringCacheTest.c $(TEST_SOURCES)
stringCacheTest_LDADD = libtaffy.la

stringEvaluatorTest_SOURCES = src/tests/dcStringEvaluatorTest.c $(TEST_SOURCES)
stringEvaluatorTest_LDADD = libtaffy.la

taffyFuzzTest_SOURCES = src/tests/dcTaffyFuzzTest.c \
                        $(TAFFY_BASE_SOURCES) \
                        $(TEST_SOURCES)
taffyFuzzTest_LDADD = libtaffy.la

threadTest_SOURCES = src/tests/dcThreadTest.c $(TEST_SOURCES)
threadTest_LDADD = libtaffy.la

lib_LTLIBRARIES = libtaffy.la libtaffyPie.la

taffy_main_SOURCES = ${taffy_base_SOURCES}

decNumber_SOURCES = \
    src/third_party/decNumber/decContext.c \
    src/third_party/decNumber/decContext.h \
    src/third_party/decNumber/decNumber.c \
    src/third_party/decNumber/decNumber.h

PLUGIN_SOURCES = \
    src/class/dcDateClass.c \
    src/class/dcFileClass.c

taffy_base_SOURCES = \
    ${DEBUG_OR_NOT_FILES} \
    ${decNumber_SOURCES} \
    ${PLUGIN_SOURCES} \
    src/class/dcArrayClass.c \
    src/class/dcArrayClass.h \
    src/class/CompiledArray.c \
    src/class/CompiledArray.h \
    src/class/CompiledInvalidArraySizeException.c \
    src/class/CompiledInvalidArraySizeException.h \
    src/class/dcBlockClass.c \
    src/class/dcBlockClass.h \
    src/class/dcClass.c \
    src/class/dcClass.h \
    src/class/dcClassManager.c \
    src/class/dcClassManager.h \
    src/class/dcClassTemplate.c \
    src/class/dcClassTemplate.h \
    src/class/CompiledComplexNumber.c \
    src/class/CompiledComplexNumber.h \
    src/class/dcComplexNumberClass.c \
    src/class/dcComplexNumberClass.h \
    src/class/dcContainerClass.c \
    src/class/dcContainerClass.h \
    src/class/CompiledContainer.c \
    src/class/CompiledContainer.h \
    src/class/dcEquationClass.c \
    src/class/dcEquationClass.h \
    src/class/dcExceptionClass.c \
    src/class/dcExceptionClass.h \
    src/class/dcExceptions.c \
    src/class/dcExceptions.h \
    src/class/CompiledExceptions.c \
    src/class/CompiledExceptions.h \
    src/class/dcFunctionClass.c \
    src/class/dcFunctionClass.h \
    src/class/CompiledFunction.c \
    src/class/CompiledFunction.h \
    src/class/dcFunctionRepeater.c \
    src/class/dcFunctionRepeater.h \
    src/class/dcFutureClass.c \
    src/class/dcFutureClass.h \
    src/class/CompiledFuture.c \
    src/class/CompiledFuture.h \
    src/class/dcHashClass.c \
    src/class/dcHashClass.h \
    src/class/CompiledHash.c \
    src/class/CompiledHash.h \
    src/class/dcHeapClass.c \
    src/class/dcHeapClass.h \
	src/class/CompiledHeap.c \
	src/class/CompiledHeap.h \
    src/class/dcIOClass.c \
    src/class/dcIOClass.h \
    src/class/CompiledIO.c \
    src/class/CompiledIO.h \
    src/class/dcKernelClass.c \
    src/class/dcKernelClass.h \
    src/class/CompiledKernel.c \
    src/class/CompiledKernel.h \
    src/class/dcListClass.c \
    src/class/dcListClass.h \
    src/class/CompiledList.c \
    src/class/CompiledList.h \
    src/class/dcLineContainerClass.c \
    src/class/dcLineContainerClass.h \
    src/class/CompiledLineContainer.c \
    src/class/CompiledLineContainer.h \
    src/class/dcMainClass.c \
    src/class/dcMainClass.h \
    src/class/CompiledMain.c \
    src/class/CompiledMain.h \
    src/class/dcMathClass.c \
    src/class/dcMathClass.h \
    src/class/CompiledMath.c \
    src/class/CompiledMath.h \
    src/class/CompiledMathConstants.c \
    src/class/CompiledMathConstants.h \
    src/class/dcConditionClass.c \
    src/class/dcConditionClass.h \
    src/class/dcMatrixClass.c \
    src/class/dcMatrixClass.h \
    src/class/CompiledMatrix.c \
    src/class/CompiledMatrix.h \
    src/class/dcMutexClass.c \
    src/class/dcMutexClass.h \
    src/class/CompiledNil.c \
    src/class/CompiledNil.h \
    src/class/dcNilClass.c \
    src/class/dcNilClass.h \
    src/class/dcNoClass.c \
    src/class/dcNoClass.h \
    src/class/dcNumberClass.c \
    src/class/dcNumberClass.h \
    src/class/CompiledNumber.c \
    src/class/CompiledNumber.h \
    src/class/dcObjectClass.c \
    src/class/dcObjectClass.h \
    src/class/dcPairClass.c \
    src/class/dcPairClass.h \
    src/class/CompiledPair.c \
    src/class/CompiledPair.h \
    src/class/dcPackageContents.c \
    src/class/dcPackageContents.h \
    src/class/dcProcedureClass.c \
    src/class/dcProcedureClass.h \
    src/class/CompiledReentrantMutex.c \
    src/class/CompiledReentrantMutex.h \
    src/class/dcSeriesClass.c \
    src/class/dcSeriesClass.h \
    src/class/CompiledSeries.c \
    src/class/CompiledSeries.h \
    src/class/dcSequenceClass.c \
    src/class/dcSequenceClass.h \
    src/class/CompiledSequence.c \
    src/class/CompiledSequence.h \
    src/class/dcStringClass.c \
    src/class/dcStringClass.h \
    src/class/CompiledString.c \
    src/class/CompiledString.h \
    src/class/dcSymbolClass.c \
    src/class/dcSymbolClass.h \
    src/class/CompiledThreader.c \
    src/class/CompiledThreader.h \
    src/class/dcThreadClass.c \
    src/class/dcThreadClass.h \
    src/class/dcYesClass.c \
    src/class/dcYesClass.h \
    src/class/dcConstantClass.c \
    src/class/dcConstantClass.h \
    src/class/org/taffy/core/exception/dcParseErrorExceptionClass.c \
    src/class/org/taffy/core/exception/dcParseErrorExceptionClass.h \
    src/class/special/dcWildClass.c \
    src/class/special/dcWildClass.h \
    src/class/special/dcSuppliedArgumentClass.c \
    src/class/special/dcSuppliedArgumentClass.h \
    src/container/dcArray.c \
    src/container/dcArray.h \
    src/container/dcAVLTree.c \
    src/container/dcAVLTree.h \
    src/container/dcBinarySearchTree.c \
    src/container/dcBinarySearchTree.h \
    src/container/dcHash.c \
    src/container/dcHash.h \
    src/container/dcHeap.c \
    src/container/dcHeap.h \
    src/container/dcList.c \
    src/container/dcList.h \
    src/container/dcPair.c \
    src/container/dcPair.h \
    src/container/dcTree.c \
    src/container/dcTree.h \
    src/evaluator/dcCallStackData.c \
    src/evaluator/dcCallStackData.h \
    src/evaluator/dcFileEvaluator.c \
    src/evaluator/dcFileEvaluator.h \
    src/evaluator/dcNodeEvaluator.c \
    src/evaluator/dcNodeEvaluator.h \
    src/evaluator/dcStringEvaluator.c \
    src/evaluator/dcStringEvaluator.h \
    src/global/dcCFunctionArgument.c \
    src/global/dcCFunctionArgument.h \
    src/global/dcCondition.c \
    src/global/dcCondition.h \
    src/global/dcDefines.h \
    src/global/dcDoubleVoid.c \
    src/global/dcDoubleVoid.h \
    src/global/dcError.c \
    src/global/dcError.h \
    src/global/dcFilePackageData.c \
    src/global/dcFilePackageData.h \
    src/global/dcFutureManager.c \
    src/global/dcFutureManager.h \
    src/global/dcGarbageCollector.c \
    src/global/dcGarbageCollector.h \
    src/global/dcLog.h \
    src/global/dcMarshaller.c \
    src/global/dcMarshaller.h \
    src/global/dcMemory.c \
    src/global/dcMemory.h \
    src/global/dcMutex.c \
    src/global/dcMutex.h \
    src/global/dcReadWriteLock.c \
    src/global/dcReadWriteLock.h \
    src/global/dcNode.c \
    src/global/dcNode.h \
    src/global/dcObjectStack.c \
    src/global/dcObjectStack.h \
    src/global/dcObjectStackList.c \
    src/global/dcObjectStackList.h \
    src/global/dcReadline.h \
    src/global/dcRootMarkFunction.c \
    src/global/dcRootMarkFunction.h \
    src/global/dcStringCache.c \
    src/global/dcStringCache.h \
    src/global/dcStringManager.c \
    src/global/dcStringManager.h \
    src/global/dcSystem.c \
    src/global/dcSystem.h \
    src/global/dcTaffyCMethodPointer.c \
    src/global/dcTaffyCMethodPointer.h \
    src/global/dcTaffyCMethodWrapper.h \
    src/global/dcThread.c \
    src/global/dcThread.h \
    src/global/dcThreadId.c \
    src/global/dcThreadId.h \
    src/global/text/CompiledCopyright.c \
    src/global/text/CompiledCopyright.h \
    src/global/dcVoid.c \
    src/global/dcVoid.h \
    src/global/dcVoidContainer.c \
    src/graph/dcAnd.c \
    src/graph/dcAnd.h \
    src/graph/dcAssignment.c \
    src/graph/dcAssignment.h \
    src/graph/dcBreak.c \
    src/graph/dcBreak.h \
    src/graph/dcBool.c \
    src/graph/dcBool.h \
    src/graph/dcCatchBlock.c \
    src/graph/dcCatchBlock.h \
    src/graph/dcCharacterGraph.c \
    src/graph/dcCharacterGraph.h \
    src/graph/dcTryBlock.c \
    src/graph/dcTryBlock.h \
    src/graph/dcExit.c \
    src/graph/dcExit.h \
    src/graph/dcFalse.c \
    src/graph/dcFalse.h \
    src/graph/dcFlatArithmetic.c \
    src/graph/dcFlatArithmetic.h \
    src/graph/dcFor.c \
    src/graph/dcFor.h \
    src/graph/dcFunctionUpdate.c \
    src/graph/dcFunctionUpdate.h \
    src/graph/dcGraphData.c \
    src/graph/dcGraphData.h \
    src/graph/dcGraphDataTree.c \
    src/graph/dcGraphDataTree.h \
    src/graph/dcGraphDataNode.c \
    src/graph/dcGraphDataNode.h \
    src/graph/dcIdentifier.c \
    src/graph/dcIdentifier.h \
    src/graph/dcIf.c \
    src/graph/dcIf.h \
    src/graph/dcImport.c \
    src/graph/dcImport.h \
    src/graph/dcIn.c \
    src/graph/dcIn.h \
    src/graph/dcLexer.c \
    src/graph/dcLexer.h \
    src/graph/dcMethodCall.c \
    src/graph/dcMethodCall.h \
    src/graph/dcMethodHeader.c \
    src/graph/dcMethodHeader.h \
    src/graph/dcNew.c \
    src/graph/dcNew.h \
    src/graph/dcNil.c \
    src/graph/dcNil.h \
    src/graph/dcNotEqualCall.c \
    src/graph/dcNotEqualCall.h \
    src/graph/dcOr.c \
    src/graph/dcOr.h \
    src/graph/dcPackage.c \
    src/graph/dcPackage.h \
    src/graph/dcParser.c \
    src/graph/dcParser.h \
    src/graph/dcReturn.c \
    src/graph/dcReturn.h \
    src/graph/dcSelf.c \
    src/graph/dcSelf.h \
    src/graph/dcString.c \
    src/graph/dcString.h \
    src/graph/dcSuper.c \
    src/graph/dcSuper.h \
    src/graph/dcSymbol.c \
    src/graph/dcSymbol.h \
    src/graph/dcSynchronized.c \
    src/graph/dcSynchronized.h \
    src/graph/dcThrow.c \
    src/graph/dcThrow.h \
    src/graph/dcTrue.c \
    src/graph/dcTrue.h \
    src/graph/dcUpSelf.c \
    src/graph/dcUpSelf.h \
    src/graph/dcWhile.c \
    src/graph/dcWhile.h \
    src/io/dcCommandLineArgument.c \
    src/io/dcCommandLineArgument.h \
    src/io/dcCommandLineArguments.c \
    src/io/dcCommandLineArguments.h \
    src/io/dcFileManagement.c \
    src/io/dcFileManagement.h \
    src/io/dcSocket.c \
    src/io/dcSocket.h \
    src/maths/dcComplexNumber.c \
    src/maths/dcComplexNumber.h \
    src/maths/dcDecNumber.c \
    src/maths/dcDecNumber.h \
    src/maths/dcFloat.c \
    src/maths/dcFloat.h \
    src/maths/dcInt32.c \
    src/maths/dcInt32.h \
    src/maths/dcMatrix.c \
    src/maths/dcMatrix.h \
    src/maths/dcNumber.c \
    src/maths/dcNumber.h \
    src/maths/dcUnsignedInt32.c \
    src/maths/dcUnsignedInt32.h \
    src/maths/dcUnsignedInt64.c \
    src/maths/dcUnsignedInt64.h \
    src/scope/dcScope.c \
    src/scope/dcScope.h \
    src/scope/dcScopeData.c \
    src/scope/dcScopeData.h \
    src/taffy/dcTaffyCommandLineArguments.c \
    src/taffy/dcTaffyCommandLineArguments.h \
    src/taffy/text/CompiledTaffyUsage.c \
    src/taffy/text/CompiledTaffyUsage.h \
    src/taffy/text/CompiledTaffyVersion.c \
    src/taffy/text/CompiledTaffyVersion.h

bin_PROGRAMS = taffy $(PIE_PROGS)
noinst_PROGRAMS = $(TEST_PROGS) $(PIE_TEST_PROGS)

everything: convertTaffy parser all taffy pie

if LINUX
parser:
	yacc -dv src/graph/dcParser.y -o src/graph/dcParser.c --defines=src/graph/dcParser.h --report-file=dcParserOutput.log
else
parser:
	yacc -dv src/graph/dcParser.y -o src/graph/dcParser.c --defines=src/graph/dcParser.h
endif
	cat src/graph/dcParserPreamble.txt > src/graph/dcParserTemp.txt
	cat src/graph/dcParser.h >> src/graph/dcParserTemp.txt
	mv src/graph/dcParserTemp.txt src/graph/dcParser.h

compileText:
	perl CompileText.pl src/global/text Copyright
	perl CompileText.pl src/pie/text    PieUsage
	perl CompileText.pl src/taffy/text  TaffyUsage
	perl CompileText.pl src/taffy/text  TaffyVersion

convertTaffy:
	perl ConvertTaffy.pl src/class Array
	perl ConvertTaffy.pl src/class ComplexNumber
	perl ConvertTaffy.pl src/class Container
	perl ConvertTaffy.pl src/class Exceptions
	perl ConvertTaffy.pl src/class Function
	perl ConvertTaffy.pl src/class Future
	perl ConvertTaffy.pl src/class Hash
	perl ConvertTaffy.pl src/class Heap
	perl ConvertTaffy.pl src/class InvalidArraySizeException
	perl ConvertTaffy.pl src/class IO
	perl ConvertTaffy.pl src/class Kernel
	perl ConvertTaffy.pl src/class List
	perl ConvertTaffy.pl src/class LineContainer
	perl ConvertTaffy.pl src/class Math
	perl ConvertTaffy.pl src/class MathConstants
	perl ConvertTaffy.pl src/class Matrix
	perl ConvertTaffy.pl src/class Nil
	perl ConvertTaffy.pl src/class Number
	perl ConvertTaffy.pl src/class Pair
	perl ConvertTaffy.pl src/class ReentrantMutex
	perl ConvertTaffy.pl src/class Series
	perl ConvertTaffy.pl src/class Sequence
	perl ConvertTaffy.pl src/class String
	perl ConvertTaffy.pl src/class Threader
	perl ConvertTaffy.pl src/org/taffy/help HelpSystem
	perl ConvertTaffy.pl src/org/taffy/help HelpTopic
	perl ConvertTaffy.pl src/org/taffy/help ImportHelperHelpTopic
	perl ConvertTaffy.pl src/org/taffy/help MainHelpTopic
	perl ConvertTaffy.pl src/org/taffy/help/warranty GnuLesserTermsAndConditionsHelpTopic
	perl ConvertTaffy.pl src/org/taffy/help/warranty GnuTermsAndConditionsHelpTopic
	perl ConvertTaffy.pl src/org/taffy/help/warranty GnuWarrantyHelpTopic
	perl ConvertTaffy.pl src/org/taffy/help/warranty IcuCopyrightHelpTopic

all-local:
	cp config.h src/global/dcGlobalConfig.h

cleanTilde:
	find . -name "*~" -print0 | xargs -0 rm

.PHONY: runMarshallTest

#########
# TESTS #
#########

TEST_SUITE = org/taffy/core/tests/TestSuite.ty -i src
CYGWIN_TEST_SUITE = ./taffy.exe $(TEST_SUITE)
LINUX_TEST_SUITE = ./taffy $(TEST_SUITE)

VALGRIND = valgrind --leak-check=full --track-origins=yes --undef-value-errors=yes --read-var-info=yes --num-callers=50
HELGRIND = valgrind --tool = helgrind --num-callers=50

if COMPILE_EXTERNAL_IO
COPY_IO_TESTS=cp src/org/taffy/core/tests/holding/FileTest.ty src/org/taffy/core/tests
RUN_IO_TESTS=$(RUN_TEST_SUITE) -a FileTest
RUN_IO_TESTS_TICK=$(RUN_IO_TESTS) --log "garbage-collector-tick"
else
COPY_IO_TESTS=rm -rf src/org/taffy/core/tests/FileTest.ty
RUN_IO_TESTS=
RUN_IO_TESTS_TICK=
endif

if APPLE
# valgrind doesn't work well, or at all, on OS X
RUN_TEST_SUITE = $(LINUX_TEST_SUITE)
RUN_VALGRIND_TEST = $(RUN_TEST_SUITE)
RUN_HELGRIND_TEST =
else
if CYGWIN
RUN_TEST_SUITE = $(CYGWIN_TEST_SUITE)
RUN_VALGRIND_TEST = $(RUN_TEST_SUITE)
RUN_HELGRIND_TEST =
else
RUN_TEST_SUITE = $(LINUX_TEST_SUITE)
RUN_VALGRIND_TEST = $(VALGRIND) .libs/$(LINUX_TEST_SUITE)
RUN_HELGRIND_TEST = $(HELGRIND) $(LINUX_TEST_SUITE)
endif
endif

if CYGWIN
tests:
	$(foreach test, $(TEST_PROGS), make $(test).exe;)
else
tests: $(TEST_PROGS)
endif

run-unit-tests:
	$(foreach test, $(TEST_PROGS), make $(test) ; ./$(test) --run-system;)

run-unit-tests-tick:
	$(foreach test, $(TEST_PROGS), make $(test) ; ./$(test) --run-system --log "garbage-collector-tick";)

run-unit-valgrind-tests:
	$(foreach test, $(TEST_PROGS), make $(test) ; $(VALGRIND) .libs/$(test) --run-system;)

run-unit-helgrind-tests:
	$(foreach test, $(TEST_PROGS), make $(test) ; $(HELGRIND) .libs/$(test) --run-system;)

test:
	make tests
	make run-unit-tests
	$(COPY_IO_TESTS)
	$(RUN_IO_TESTS_TICK)
	$(RUN_TEST_SUITE) --log "garbage-collector-tick"

test-fast:
	make tests
	make run-unit-tests
	$(COPY_IO_TESTS)
	$(RUN_IO_TESTS_TICK)
	$(RUN_TEST_SUITE)

run-test-suite:
	make taffy
	$(COPY_IO_TESTS)
	$(RUN_IO_TESTS)
	$(RUN_TEST_SUITE)

test-tick:
	make tests
	make run-unit-tests-tick
	$(COPY_IO_TESTS)
	$(RUN_IO_TESTS_TICK)
	$(RUN_TEST_SUITE) --log "garbage-collector-tick"

test-valgrind:
	make tests
	make run-unit-valgrind-tests
	$(RUN_VALGRIND_TEST)

test-valgrind-end-only:
	make tests
	make run-unit-tests
	$(RUN_VALGRIND_TEST)

test-helgrind:
	make tests
	make run-unit-helgrind-tests
	$(RUN_HELGRIND_TEST)

test-command-line:
	expect -f taffyCommandLineTest.sh
	expect -f pieCommandLineTest.sh
	expect -f kernelExitTest.sh
	expect -f ioTest.sh
