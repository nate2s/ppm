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

//
// A Lattice is a character lattice graph
//
#ifndef __LATTICE_H__
#define __LATTICE_H__

#include <string>
#include <vector>

struct Position
{
    Position(uint32_t x, uint32_t y);

    uint32_t x;
    uint32_t y;
};

std::ostream &operator<<(std::ostream &out, const Position &position);

class Lattice
{
public:
    struct Element
    {
        Element();
        Element(char fancy, char regular, int id, bool marked);
        Element(const Element &other);
        Element &operator=(const Element &other);

        char fancy_;
        char regular_;
        int id_;
        bool marked_;
    };

    static unsigned int currentId;

    // constructing
    Lattice();
    Lattice(const Lattice &other);
    Lattice(const std::vector<std::string> &fancyString, char regular);

    // destructing
    virtual ~Lattice();

    // mutability
    Position addToLeft(const Lattice &other);
    Position addToRight(const Lattice &other, int32_t xOffset = 0);
    Position addToRightMiddle(const Lattice &other, int32_t xOffset = 0);
    Position addToBottomRight(const Lattice &other, uint32_t yOffset = 0);
    Position addToRightSquished(const Lattice &other);
    Position addToBottom(const Lattice &other, uint32_t x = 0);
    Position addToBottomCenter(const Lattice &other);
    Position addToTopRight(const Lattice &other);
    void prependRows(size_t count);
    void prependColumns(size_t count);
    void appendRows(size_t count);
    void appendColumns(size_t count);
    Lattice *removeBlankLines();

    // dimensions
    size_t getWidth() const;
    size_t getHeight() const;

    // grouping
    void engroup(const Lattice &left, const Lattice &right);
    bool isGrouped() const;

    // marking and pasting
    void mark(const Position &position);

    // replace the marked elements with 'other'
    void paste(const Lattice &other);

    // paste 'other' at position (x, y)
    Position paste(const Lattice &other, int x, int y);

    // id setting
    void setId(const Lattice &other);
    void flattenIds();
    void setNewId();

    // output
    std::string convertToString(bool colorize = true, bool randomColors = true) const;
    friend std::ostream &operator<<(std::ostream &out, const Lattice &lattice);

protected:
    std::vector<std::vector<Element>> lattice_;
    bool isGrouped_;
};

#endif
