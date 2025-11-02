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

#ifndef LE_DISASM_OBJECT_HEADER_HPP_
#define LE_DISASM_OBJECT_HEADER_HPP_

#include <cstdint>
#include <istream>

class ObjectHeader {
public:
    enum {
        READABLE = 1 << 0,
        WRITABLE = 1 << 1,
        EXECUTABLE = 1 << 2,
        RESOURCE = 1 << 3,
        DISCARDABLE = 1 << 4,
        SHARED = 1 << 5,
        PRELOADED = 1 << 6,
        INVALID = 1 << 7,
        ZERO_FILL = 1 << 8,
        ALIAS_REQUIRED = 1 << 12,
        BIG_DEFAULT = 1 << 13
    };

    uint32_t virtual_size;
    uint32_t base_address;
    uint32_t flags;
    uint32_t first_page_index;
    uint32_t page_count;
    uint32_t reserved;

    void ReadFrom(std::istream& is);
    bool IsExecutable() const;
    bool Is32BitObject() const;
};

#endif
