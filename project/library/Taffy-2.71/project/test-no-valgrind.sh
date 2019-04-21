#!/usr/bin/bash
export LD_LIBRARY_PATH=$(pwd)/.libs
export DYLD_LIBRARY_PATH=$(pwd)/.libs

unamestr=`uname`

make test
