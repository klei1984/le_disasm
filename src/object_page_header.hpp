/* Copyright (C) 2010  Unavowed <unavowed@vexillium.org>
 * Copyright (C) 2017  samunders-core <samunders-core@users.noreply.github.com>
 * Copyright (C) 2019-2025  klei1984 <53688147+klei1984@users.noreply.github.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LE_DISASM_OBJECT_PAGE_HEADER_HPP_
#define LE_DISASM_OBJECT_PAGE_HEADER_HPP_

#include <cstdint>
#include <istream>

class ObjectPageHeader {
public:
    enum ObjectPageType { LEGAL = 0, ITERATED = 1, INVALID = 2, ZERO_FILLED = 3, LAST = 4 };

    uint16_t first_number;
    uint8_t second_number;
    ObjectPageType type;

    void ReadFrom(std::istream& is);
};

#endif
