#!/usr/bin/bash
./configure --enable-tests --enable-debug CFLAGS="-O0 -g3"
make clean
make
