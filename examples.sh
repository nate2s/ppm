#!/bin/bash

hash ./ppm
result=$?

if [ $result != 0 ]; then
    echo "Please build ppm via build.sh"
    exit
fi

declare -a maths=("unformatted string"
                  "x"
                  "x + 1"
                  "x + y"
                  "x - 1"
                  "x - y"
                  "x + 3 + 4"
                  "x / 2"
                  "x / y"
                  "a^5+7"
                  "e^(x/2/3)"
                  "e^x/2/3"
                  "xx / yyy"
                  "xxx / yy"
                  "x^e"
                  "xx^ee / yy"
                  "xx^ee / yy - zz"
                  "x^y^z^w - y"
                  "x^4^3 - 3/6"
                  "x^(-3/6)"
                  "(-3/6)"
                  "(-1 - 3/6)"
                  "(-1/x - 3)"
                  "3 + x + y + 3/4/5/6"
                  "sin(x)"
                  "sin(x^2)"
                  "sin(x + 3.1)"
                  "sin(x) + 1"
                  "sin(x^2) + 1"
                  "sin(x/y/z)"
                  "sin(x, x)"
                  "sin(x, x, x)"
                  "cos(x, y^2, x)"
                  "cos(x, x, Y^2^3)"
                  "sin(x) = 1"
                  "sin(2) = 1"
                  "sin(x / 2) = 1"
                  "(3 + (4/5/6) + 5) / 7"
                  "x = y^2"
                  "x = sin(x)"
                  "x = y^2^sin(y)"
                  "y^2 = x"
                  "y^2 = x^2"
                  "y == x"
                  "y^2 = sin(x^2)")

for math in "${maths[@]}"; do
    ./ppm --font smscript "$math"
    echo ""
    ./ppm --font small "$math"
    echo ""
    ./ppm --font big "$math"
    echo ""
    ./ppm --font banner "$math"
    echo ""
done

declare -a inputs=("-4 + 5^2^3^5 + 5")

for math in "${inputs[@]}"; do
    ./ppm --input "$math"
done

declare -a bigs=("x/y/z + 55.1")

for math in "${bigs[@]}"; do
    ./ppm --font big "$math"
done
