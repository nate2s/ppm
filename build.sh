#!/bin/bash

echo ""
echo "** Welcome to the ppm build system **"
echo ""

declare -a programs=("make")

# check that required programs exist
for program in "${programs[@]}"
do
    hash $program
    result=$?

    if [ $result != 0 ]; then
        echo "Error: ${program} not found"
        exit
    fi
done

cd project/library/Taffy-2.71/project

echo ""
echo "** Building the Taffy library **"
echo ""

./configure --disable-external-io
make

if [ $? != 0 ]; then
    echo ""
    echo "** Error during make of taffy library, exiting **"
    exit
fi

cp .libs/libtaffy* ../../..
cd ../../..

echo ""
echo "** Building ppm **"
echo ""

./configure
make

if [ $? != 0 ]; then
    echo ""
    echo "** Error during make of ppm, exiting **"
    exit
fi

cp ppm ..
cd ..

rm .libs
ln -s project/.libs .libs

echo ""
echo "** Build complete **"
echo "** Run examples.sh to see ppm in action **"
echo ""
