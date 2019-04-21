#!/usr/bin/bash
export LD_LIBRARY_PATH=$(pwd)/.libs
export DYLD_LIBRARY_PATH=$(pwd)/.libs

exec ./taffy src/org/taffy/core/tests/TestSuite.ty -i src -a ArrayTest ArrayTest ArrayTest HashTest NumberTest NumberTest StringTest StringTest
