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

#ifndef LE_DISASM_FIXUP_HPP_
#define LE_DISASM_FIXUP_HPP_

#include <cstdint>
#include <istream>
#include <vector>

class ObjectHeader;

class Fixup {
public:
    const uint32_t offset;
    const uint32_t address;

    Fixup(std::istream& is, size_t& offset_, std::vector<ObjectHeader> objects, uint32_t page_offset,
          uint8_t addr_flags = 0, uint8_t reloc_flags = 0);

private:
    static uint8_t ThrowOnInvalidAddressFlags(std::istream& is);
    static uint8_t ThrowOnInvalidRelocFlags(std::istream& is);
    static uint8_t ThrowOnInvalidObjectIndex(std::istream& is, std::vector<ObjectHeader> objects, uint32_t page_offset);
    static int16_t ReadUpToSourceOffset(std::istream& is, size_t& offset, uint8_t& addr_flags, uint8_t& reloc_flags);
    static uint32_t ReadDestOffset(std::istream& is, size_t& offset, std::vector<ObjectHeader> objects,
                                   uint32_t page_offset, uint8_t addr_flags, uint8_t reloc_flags);
};

#endif
