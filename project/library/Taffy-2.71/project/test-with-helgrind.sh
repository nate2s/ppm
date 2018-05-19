#!/usr/bin/bash

unamestr=`uname`

if [[ "$unamestr" == "Linux" ]]; then
    export LD_LIBRARY_PATH=.libs
    valgrind --tool=helgrind --num-callers=50 .libs/taffy src/org/taffy/core/tests/TestSuite.ty -i src -a KernelTest KernelTest
    valgrind --tool=helgrind --num-callers=50 .libs/taffy src/org/taffy/core/tests/TestSuite.ty -i src -a MathTest KernelTest
    valgrind --tool=helgrind --num-callers=50 .libs/taffy src/org/taffy/core/tests/TestSuite.ty -i src -a ArrayTest KernelTest
fi
