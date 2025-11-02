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

#include "object_header.hpp"

#include "little_endian.hpp"

void ObjectHeader::ReadFrom(std::istream& is) {
    ReadLe(is, virtual_size);
    ReadLe(is, base_address);
    ReadLe(is, flags);
    ReadLe(is, first_page_index);
    ReadLe(is, page_count);
    ReadLe(is, reserved);
    --first_page_index;
}

bool ObjectHeader::IsExecutable() const { return (flags & EXECUTABLE) != 0; }

bool ObjectHeader::Is32BitObject() const { return (flags & BIG_DEFAULT) != 0; }
