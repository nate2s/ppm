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

#include <cassert>
#include <sstream>
#include <random>
#include <chrono>
#include <algorithm>

#include "Lattice.h"
#include "Color.h"

Position::Position(uint32_t inX, uint32_t inY)
    : x(inX),
      y(inY)
{
}

std::ostream &operator<<(std::ostream &out, const Position &position)
{
    out << "[Position x: " << position.x << " y: " << position.y << "]";
    return out;
}

//
// Lattice::Element
//
Lattice::Element::Element(char fancy, char regular, int id, bool marked)
    : fancy_(fancy),
      regular_(regular),
      id_(id),
      marked_(marked)
{
}

Lattice::Element::Element(const Element &other)
    : fancy_(other.fancy_),
      regular_(other.regular_),
      id_(other.id_),
      marked_(other.marked_)
{
}

Lattice::Element::Element()
    : fancy_(' '),
      regular_(' '),
      id_(-1),
      marked_(false)
{
}

Lattice::Element &Lattice::Element::operator=(const Element &other)
{
    fancy_ = other.fancy_;
    regular_ = other.regular_;
    id_ = other.id_;
    marked_ = other.marked_;
    return *this;
}

//
// Lattice
//

// not thread safe
unsigned int Lattice::currentId = 0;

Lattice::Lattice()
    : lattice_({}),
      isGrouped_(false)
{
}

Lattice::Lattice(const Lattice &other)
    : lattice_(other.lattice_),
      isGrouped_(other.isGrouped_)
{
}

Lattice::Lattice(const std::vector<std::string> &fancyString, char regular)
{
    for (size_t i = 0; i < fancyString.size(); i++)
    {
        std::vector<Element> row;

        if (i > 0)
        {
            // it's an error to have lines of different lengths
            assert(fancyString[i].length() == fancyString[i - 1].length());
        }

        for (const char &fancy : fancyString[i])
        {
            row.push_back(Element(fancy, regular, Lattice::currentId, false));
        }

        lattice_.push_back(row);
    }

    Lattice::currentId++;
}

Lattice::~Lattice()
{
}

// Paste down and to the right
Position Lattice::paste(const Lattice &other, int x, int y)
{
    if (x < 0)
    {
        prependColumns(-x);
        x = 0;
    }

    if (y < 0)
    {
        prependRows(-y);
        y = 0;
    }

    size_t endX = (size_t)(x + other.getWidth());

    if (endX >= getWidth())
    {
        appendColumns(endX - getWidth());
    }

    size_t endY = (size_t)(y + other.getHeight());

    if (endY >= getHeight())
    {
        appendRows(endY - getHeight());
    }

    for (size_t row = y; row < y + other.getHeight(); row++)
    {
        for (size_t column = x; column < x + other.getWidth(); column++)
        {
            lattice_.at(row).at(column) = other.lattice_.at(row - y).at(column - x);
        }
    }

    // we aren't grouped anymore
    isGrouped_ = false;

    return Position(x, y);
}

void Lattice::prependRows(size_t count)
{
    if (lattice_.size() == 0)
    {
        lattice_.insert(lattice_.begin(), count, {});
    }
    else
    {
        std::vector<Element> row(getWidth());
        lattice_.insert(lattice_.begin(), count, row);
    }
}

void Lattice::prependColumns(size_t count)
{
    if (lattice_.size() == 0)
    {
        lattice_.push_back(std::vector<Element>(count));
    }
    else
    {
        for (auto &line : lattice_)
        {
            line.insert(line.begin(), count, Element());
        }
    }
}

void Lattice::appendRows(size_t count)
{
    std::vector<Element> row(getWidth());
    lattice_.insert(lattice_.end(), count, row);
}

void Lattice::appendColumns(size_t count)
{
    if (lattice_.size() == 0)
    {
        lattice_.push_back(std::vector<Element>(count));
    }
    else
    {
        size_t width = getWidth();

        for (auto &line : lattice_)
        {
            line.resize(width + count);
        }
    }
}

Position Lattice::addToRightMiddle(const Lattice &other, int32_t xOffset)
{
    return paste(other, getWidth() + xOffset, getHeight() / 2 - other.getHeight() / 2);
}

Position Lattice::addToRight(const Lattice &other, int32_t xOffset)
{
    return paste(other, getWidth() + xOffset, getHeight() - other.getHeight());
}

Position Lattice::addToBottomRight(const Lattice &other, uint32_t yOffset)
{
    return paste(other, getWidth(), (getHeight() - other.getHeight()) + yOffset);
}

Position Lattice::addToRightSquished(const Lattice &other)
{
    Lattice copy = other;

    for (auto &row : copy.lattice_)
    {
        row.erase(row.begin());
    }

    return paste(copy, getWidth() - 1, getHeight() / 2 - other.getHeight() / 2);
}

Position Lattice::addToLeft(const Lattice &other)
{
    return paste(other, -other.getWidth(), getHeight() / 2 - other.getHeight() / 2);
}

Position Lattice::addToBottom(const Lattice &other, uint32_t x)
{
    return paste(other, x, getHeight());
}

Position Lattice::addToBottomCenter(const Lattice &other)
{
    return paste(other, getWidth() / 2 - other.getWidth() / 2, getHeight());
}

Position Lattice::addToTopRight(const Lattice &other)
{
    return paste(other, getWidth(), -other.getHeight() + 1);
}

std::string Lattice::convertToString(bool colorize, bool randomColors) const
{
    std::vector<const Color *> colors = {
        &Color::red,
        &Color::green,
        &Color::yellow,
        &Color::blue,
        &Color::magenta,
        &Color::cyan
    };

    if (randomColors)
    {
        // obtain a time-based seed:
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(colors.begin(), colors.end(), std::default_random_engine(seed));
    }

    std::ostringstream oss;

    for (size_t i = 0; i < lattice_.size(); i++)
    {
        const auto &row = lattice_[i];

        for (const Element &value : row)
        {
            if (colorize)
            {
                // start the color
                oss << colors[value.id_ % colors.size()]->getCode();
            }

            // print the character
            oss << value.fancy_;

            if (colorize)
            {
                // end the color
                oss << Color::end.getCode();
            }
        }

        if (i < lattice_.size() - 1)
        {
            oss << "\n";
        }
    }

    return oss.str();
}

size_t Lattice::getWidth() const
{
    return (lattice_.size() > 0
            ? lattice_[0].size()
            : 0);
}

size_t Lattice::getHeight() const
{
    return lattice_.size();
}

std::ostream &operator<<(std::ostream &out, const Lattice &lattice)
{
    out << "Lattice (" << lattice.getHeight() << "x" << lattice.getWidth() << "):\n";

    for (size_t i = 0; i < lattice.lattice_.size(); i++)
    {
        out << "[" << i << "]: " << lattice.lattice_[i].size() << " columns: ";

        for (auto &element : lattice.lattice_[i])
        {
            out << element.fancy_;
        }

        out << "\n";
    }

    return out;
}

Lattice *Lattice::removeBlankLines()
{
    for (ssize_t i = lattice_.size() - 1; i >= 0; i--)
    {
        bool allBlank = true;
        const auto &row = lattice_[i];

        for (size_t j = 0; j < row.size(); j++)
        {
            if (row[j].fancy_ != ' ')
            {
                allBlank = false;
                break;
            }
        }

        if (allBlank)
        {
            lattice_.erase(lattice_.begin() + i);
        }
    }

    return this;
}

void Lattice::engroup(const Lattice &left, const Lattice &right)
{
    addToLeft(left);
    addToRight(right);
    isGrouped_ = true;
}

bool Lattice::isGrouped() const
{
    return isGrouped_;
}

void Lattice::flattenIds()
{
    int idNow = -1;

    // first find how many non spaces there are
    for (auto &row : lattice_)
    {
        for (auto &element : row)
        {
            if (idNow == -1)
            {
                idNow = element.id_;
            }
            else
            {
                element.id_ = idNow;
            }
        }
    }
}

void Lattice::setNewId()
{
    // first find how many non spaces there are
    for (auto &row : lattice_)
    {
        for (auto &element : row)
        {
            element.id_ = Lattice::currentId;
        }
    }

    Lattice::currentId++;
}

void Lattice::setId(const Lattice &other)
{
    for (uint32_t y = 0; y < lattice_.size(); y++)
    {
        auto &row = lattice_[y];

        for (uint32_t x = 0; x < row.size(); x++)
        {
            row[x].id_ = other.lattice_[y][x].id_;
        }
    }
}

void Lattice::mark(const Position &position)
{
    lattice_.at(position.y).at(position.x).marked_ = true;
}

#include <iostream>

// Paste 'other' at every marked element
void Lattice::paste(const Lattice &other)
{
    for (uint32_t y = lattice_.size() - 1; y != static_cast<unsigned>(-1); y--)
    {
        auto &row = lattice_[y];

        for (uint32_t x = row.size() - 1; x != static_cast<unsigned>(-1); x--)
        {
            auto &element = row[x];

            if (element.marked_)
            {
                paste(other, x, y);
                element.marked_ = false;
            }
        }
    }
}
