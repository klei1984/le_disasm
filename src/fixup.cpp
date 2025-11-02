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

#include "fixup.hpp"

#include "error.hpp"
#include "little_endian.hpp"
#include "object_header.hpp"

uint8_t Fixup::ThrowOnInvalidAddressFlags(std::istream& is) {
    uint8_t addr_flags;
    ReadLe(is, addr_flags);
    if ((addr_flags & 0x20) != 0) {
        throw Error() << "Fixup lists not supported";
    }
    return addr_flags;
}

uint8_t Fixup::ThrowOnInvalidRelocFlags(std::istream& is) {
    uint8_t reloc_flags;
    ReadLe(is, reloc_flags);
    if ((reloc_flags & 0x3) != 0x0) {
        throw Error() << "Unsupported reloc type in 0x" << std::hex << (int)reloc_flags;
    }
    return reloc_flags;
}

uint8_t Fixup::ThrowOnInvalidObjectIndex(std::istream& is, std::vector<ObjectHeader> objects, uint32_t page_offset) {
    uint8_t obj_index;
    ReadLe(is, obj_index);
    if (obj_index < 1 || obj_index > objects.size()) {
        throw Error() << "Page at offset 0x" << std::hex << page_offset << ": unexpected object index " << std::dec
                      << (int)obj_index;
    }
    return obj_index - 1;
}

int16_t Fixup::ReadUpToSourceOffset(std::istream& is, size_t& offset, uint8_t& addr_flags, uint8_t& reloc_flags) {
    addr_flags = ThrowOnInvalidAddressFlags(is);
    ++offset;

    reloc_flags = ThrowOnInvalidRelocFlags(is);
    ++offset;

    int16_t src_off;
    ReadLe(is, src_off);
    offset += sizeof(int16_t);
    return src_off;
}

uint32_t Fixup::ReadDestOffset(std::istream& is, size_t& offset, std::vector<ObjectHeader> objects,
                               uint32_t page_offset, uint8_t addr_flags, uint8_t reloc_flags) {
    if ((reloc_flags & 0x40) != 0) {
        throw Error() << "16-bit object or module ordinal numbers are not supported";
    }

    uint8_t obj_index = ThrowOnInvalidObjectIndex(is, objects, page_offset);
    ++offset;

    uint32_t dst_off_32;
    if ((reloc_flags & 0x10) != 0) {
        ReadLe(is, dst_off_32);
        offset += sizeof(dst_off_32);
    } else if ((addr_flags & 0xf) != 0x2) {
        uint16_t dst_off_16;
        ReadLe(is, dst_off_16);
        dst_off_32 = dst_off_16;
        offset += sizeof(dst_off_16);
    } else {
        return obj_index + 1;
    }
    return objects[obj_index].base_address + dst_off_32;
}

Fixup::Fixup(std::istream& is, size_t& offset_, std::vector<ObjectHeader> objects, uint32_t page_offset,
             uint8_t addr_flags, uint8_t reloc_flags)
    : offset(page_offset + ReadUpToSourceOffset(is, offset_, addr_flags, reloc_flags)),
      address(ReadDestOffset(is, offset_, objects, page_offset, addr_flags, reloc_flags)) {}
