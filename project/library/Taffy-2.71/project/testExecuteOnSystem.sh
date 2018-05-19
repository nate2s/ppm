#!/usr/bin/bash

unamestr=`uname`

if [[ "$unamestr" == "Linux" ]]; then
    ./taffy src/tests/TestExecuteOnSystem.ty
fi
