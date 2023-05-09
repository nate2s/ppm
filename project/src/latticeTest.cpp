//
// This file is part of ppm, a pretty printer for math
// Copyright (C) 2018 Nate Smith (nat2e.smith@gmail.com)
//
// ppm is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ppm is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <iostream>
#include <cassert>
#include <vector>

#include "Lattice.h"

#define expectEqual(left, right)                \
    expect(left, right, __func__, __LINE__)

static bool totalSuccess = true;

static void expect(const Lattice &left, const Lattice &right, const char *func, int line)
{
    bool success = true;

    if (left.getWidth() != right.getWidth()
        || left.getHeight() != right.getHeight())
    {
        std::cout << "-----------\n"
                  << "Error at: " << func
                  << ", latticeTest.cpp:" << line
                  << ":\n\n";
        success = false;

        // sticky
        totalSuccess = false;
    }

    if (left.getWidth() != right.getWidth())
    {
        std::cout << "Left width: " << left.getWidth()
                  << " != right width: " << right.getWidth()
                  << "\n";
    }

    if (left.getHeight() != right.getHeight())
    {
        std::cout << "Left height: " << left.getHeight()
                  << " != right height: " << right.getHeight()
                  << "\n\n";
    }

    if (! success)
    {
        std::cout << "Left:\n" << left << "\n"
                  << "Right:\n" << right
                  << "\n";
    }
}

void testPaste()
{
    Lattice lattice({ "ccc", "ccc" }, 'c');
    Lattice as({ "a" }, 'a');
    lattice.mark(Position(1, 1));
    lattice.paste(as);

    expectEqual(lattice, Lattice({ "ccc", "cac"}, 'z'));
}

void testPrepend()
{
    Lattice lattice({ "a" }, 'a');

    lattice.prependColumns(3);
    expectEqual(lattice, Lattice({ "   a" }, 'a'));

    lattice.prependRows(2);
    expectEqual(lattice, Lattice({ "    ", "    ", "   a" }, 'a'));

    lattice.prependColumns(2);
    expectEqual(lattice, Lattice({ "      ", "      ", "     a" }, 'a'));
}

void testAppend()
{
    Lattice lattice({ "a" }, 'a');

    lattice.appendColumns(2);
    expectEqual(lattice, Lattice({"a  "}, 'a'));

    lattice.appendRows(3);
    expectEqual(lattice, Lattice({"a  ", "   ", "   ", "   "}, 'a'));
}

void testAddToRight()
{
    Lattice lattice({ "ccc", "ccc" }, 'c');
    Lattice add({ "a", "a", "a", "a" }, 'a');
    lattice.addToRight(add);
    expectEqual(lattice, Lattice({"   a", "   a", "ccca", "ccca"}, 'z'));
}

void testSquish()
{
    Lattice left({"---"}, '-');
    Lattice right({" - "}, '-');
    left.addToRightSquished(right);
    expectEqual(left, Lattice({"----"}, '-'));
}

int main()
{
    typedef void (*Test)(void);

    const std::vector<Test> tests = {&testPaste,
                                     &testSquish,
                                     &testPrepend,
                                     &testAppend,
                                     &testAddToRight};

    for (const Test test : tests)
    {
        std::cout << ".";
        test();
    }

    std::cout << "\n";
    return 0;
}
