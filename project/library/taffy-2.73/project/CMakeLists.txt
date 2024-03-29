cmake_minimum_required(VERSION 3.16)
project(taffy)

add_library(taffy
        src/evaluator/dcNodeEvaluator.c
        src/evaluator/dcFileEvaluator.c
        src/evaluator/dcCallStackData.c
        src/evaluator/dcStringEvaluator.c
        src/org/taffy/core/exception/dcParseErrorExceptionClass.c
        src/io/dcCommandLineArguments.c
        src/io/dcCommandLineArgument.c
        src/io/dcFileManagement.c
        src/io/dcSocket.c
	src/io/dcCommandLineArguments.c
	src/io/dcCommandLineArgument.c
	src/graph/dcAssignment.c
	src/graph/dcParser.c
	src/graph/dcUpSelf.c
	src/graph/dcNew.c
	src/graph/dcNotEqualCall.c
	src/graph/dcReturn.c
	src/graph/dcTryBlock.c
	src/graph/dcSynchronized.c
	src/graph/dcIn.c
	src/graph/dcPackage.c
	src/graph/dcBool.c
	src/graph/dcFalse.c
	src/graph/dcFor.c
	src/graph/dcAnd.c
	src/graph/dcGraphData.c
	src/graph/dcGraphDataNode.c
	src/graph/dcIdentifier.c
	src/graph/dcSymbol.c
	src/graph/dcCatchBlock.c
	src/graph/dcGraphDataTree.c
	src/graph/dcImport.c
	src/graph/dcMethodCall.c
	src/graph/dcThrow.c
	src/graph/dcBreak.c
	src/graph/dcIf.c
	src/graph/dcString.c
	src/graph/dcLexer.c
	src/graph/dcExit.c
	src/graph/dcCharacterGraph.c
	src/graph/dcOr.c
	src/graph/dcFlatArithmetic.c
	src/graph/dcFunctionUpdate.c
	src/graph/dcMethodHeader.c
	src/graph/dcSuper.c
	src/graph/dcWhile.c
	src/graph/dcSelf.c
	src/graph/dcNil.c
	src/graph/dcTrue.c
	src/container/dcAVLTree.c
	src/container/dcTree.c
	src/container/dcList.c
	src/container/dcBinarySearchTree.c
	src/container/dcPair.c
	src/container/dcHeap.c
	src/container/dcArray.c
	src/container/dcHash.c
	src/global/dcCFunctionArgument.c
	src/global/dcStringCache.c
	src/global/dcObjectStackList.c
	src/global/dcGarbageCollector.c
	src/global/dcTaffyCMethodPointer.c
	src/global/dcVoid.c
	src/global/dcThreadId.c
	src/global/dcStringManager.c
	src/global/dcReadWriteLock.c
	src/global/dcNode.c
	src/global/dcVoidContainer.c
	src/global/dcMarshaller.c
	src/global/dcFutureManager.c
	src/global/dcDoubleVoid.c
	src/global/dcFilePackageData.c
	src/global/dcSystem.c
	src/global/dcMutex.c
	src/global/dcObjectStack.c
	src/global/text/CompiledCopyright.c
	src/global/dcThread.c
	src/global/dcLog.c
	src/global/dcRootMarkFunction.c
	src/global/dcCondition.c
	src/global/dcMemory.c
	src/scope/dcScope.c
	src/scope/dcScopeData.c
	src/taffy/dcTaffyCommandLineArguments.c
	src/taffy/text/CompiledTaffyUsage.c
	src/taffy/text/CompiledTaffyVersion.c
	src/third_party/decNumber/decNumber.c
	src/third_party/decNumber/decContext.c
	src/maths/dcUnsignedInt32.c
	src/maths/dcMatrix.c
	src/maths/dcFloat.c
	src/maths/dcNumber.c
	src/maths/dcComplexNumber.c
	src/maths/dcUnsignedInt64.c
	src/maths/dcDecNumber.c
	src/maths/dcInt32.c
	src/class/dcMutexClass.c
	src/class/dcProcedureClass.c
	src/class/dcMatrixClass.c
	src/class/dcNumberClass.c
	src/class/CompiledMatrix.c
	src/class/CompiledHash.c
	src/class/dcThreadClass.c
	src/class/CompiledFunction.c
	src/class/CompiledReentrantMutex.c
	src/class/CompiledContainer.c
	src/class/dcIOClass.c
	src/class/CompiledList.c
	src/class/dcExceptions.c
	src/class/CompiledMathConstants.c
	src/class/CompiledFuture.c
	src/class/dcFileClass.c
	src/class/dcPairClass.c
	src/class/dcYesClass.c
	src/class/CompiledSequence.c
	src/class/dcSymbolClass.c
	src/class/CompiledMath.c
	src/class/dcContainerClass.c
	src/class/CompiledPair.c
	src/class/CompiledArray.c
	src/class/CompiledHeap.c
	src/class/special/dcSuppliedArgumentClass.c
	src/class/special/dcWildClass.c
	src/class/CompiledString.c
	src/class/dcMathClass.c
	src/class/CompiledIO.c
	src/class/dcBlockClass.c
	src/class/CompiledThreader.c
	src/class/dcKernelClass.c
	src/class/dcConstantClass.c
	src/class/CompiledSeries.c
	src/class/dcSequenceClass.c
	src/class/dcFunctionRepeater.c
	src/class/dcListClass.c
	src/class/CompiledNil.c
	src/class/dcClassManager.c
	src/class/dcNoClass.c
	src/class/dcDateClass.c
	src/class/CompiledNumber.c
	src/class/dcFutureClass.c
	src/class/dcConditionClass.c
	src/class/dcHeapClass.c
	src/class/dcClassTemplate.c
	src/class/dcFunctionClass.c
	src/class/dcComplexNumberClass.c
	src/class/dcClass.c
	src/class/CompiledMain.c
	src/class/CompiledKernel.c
	src/class/CompiledLineContainer.c
	src/class/CompiledInvalidArraySizeException.c
	src/class/dcObjectClass.c
	src/class/dcSeriesClass.c
	src/class/dcNilClass.c
	src/class/dcExceptionClass.c
	src/class/dcArrayClass.c
	src/class/dcPackageContents.c
	src/class/dcEquationClass.c
	src/class/dcStringClass.c
	src/class/CompiledExceptions.c
	src/class/CompiledComplexNumber.c
	src/class/dcHashClass.c
	src/class/dcMainClass.c
	src/class/dcLineContainerClass.c)

if (WINDOWS)
        add_executable(taffy
        	src/global/dcReadWriteLockWindows.c
               	src/global/dcConditionWindows.c
        	src/global/dcThreadWindows.c
                src/io/dcSocketWindows.c
               	src/global/dcMutexWindows.c)
endif (WINDOWS)

include_directories(
        src/evaluator
        src/org/taffy/core/exception
        src/io
	src/scope
	src/taffy
	src/container
	src/third_party/decNumber
	src/maths
	src/class
	src/global
	src/graph
	src/container
        src/class/special
        src/org/taffy/help
        src
        src/taffy/text)

if (UNIX)
  target_link_libraries(taffy pthread dl)
endif (UNIX)
