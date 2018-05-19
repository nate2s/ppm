#!/usr/bin/bash
export LD_LIBRARY_PATH=$(pwd)/.libs
export DYLD_LIBRARY_PATH=$(pwd)/.libs

unamestr=`uname`

if [[ "$unamestr" == "Linux" ]]; then
    make test-valgrind
else
    make test
fi
