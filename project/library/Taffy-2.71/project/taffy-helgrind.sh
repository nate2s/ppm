#!/usr/bin/bash

unamestr=`uname`

if [[ "$unamestr" == "Linux" ]]; then
    export LD_LIBRARY_PATH=.libs
    valgrind --tool=helgrind --num-callers=50 .libs/taffy src/tests/SimpleThreadProgram.ty
fi
