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

#include "linear_executable.hpp"

#include <iostream>

#include "fixup.hpp"
#include "little_endian.hpp"

uint32_t LinearExecutable::EntryPointAddress() {
    return objects[header.eip_object_index].base_address + header.eip_offset;
}

size_t LinearExecutable::OffsetOfPageInFile(size_t index) const {
    if (object_pages.size() <= index) {
        return 0;
    }
    const ObjectPageHeader& hdr = object_pages[index];
    return (hdr.first_number + hdr.second_number - 1) * header.page_size + header.data_pages_offset;
}

template <typename T>
void LinearExecutable::LoadTable(std::istream& is, uint32_t count, std::vector<T>& ret) {
    ret.resize(count);
    for (uint32_t n = 0; n < count; ++n) {
        ret[n].ReadFrom(is);
    }
}

void LinearExecutable::LoadObjectFixups(std::istream& is, std::vector<uint32_t>& fixup_record_offsets,
                                        size_t table_offset, size_t oi) {
    ObjectHeader& obj = objects[oi];
    if (verbose) std::cerr << "Loading fixups for object " << oi + 1 << std::endl;
    for (size_t n = obj.first_page_index; n < obj.first_page_index + obj.page_count; ++n) {
        size_t offset = table_offset + fixup_record_offsets[n];
        size_t end = table_offset + fixup_record_offsets[n + 1];
        size_t page_offset = (n - obj.first_page_index) * header.page_size;
        for (is.seekg(offset); offset < end;) {
            if (verbose)
                std::cerr << "Loading fixup 0x" << offset << " at page " << std::dec << (n + 1 - obj.first_page_index)
                          << "/" << obj.page_count << ", offset 0x" << std::hex << page_offset << ": ";
            Fixup fixup(is, offset, objects, page_offset);
            fixups[oi][fixup.offset] = fixup.address;
            fixup_addresses.insert(fixup.address);
            if (verbose) std::cerr << "0x" << fixup.offset << " -> 0x" << fixup.address << std::endl;
        }
    }
}

void LinearExecutable::LoadFixupTable(std::istream& is, std::vector<uint32_t>& fixup_record_offsets,
                                      size_t table_offset) {
    fixups.resize(objects.size());
    for (size_t oi = 0; oi < objects.size(); ++oi) {
        LoadObjectFixups(is, fixup_record_offsets, table_offset, oi);
    }
}

LinearExecutable::LinearExecutable(std::istream& is, bool verbose, uint32_t header_offset) : header(is, header_offset) {
    this->verbose = verbose;
    is.seekg(header_offset + header.object_table_offset);
    LoadTable(is, header.object_count, objects);

    is.seekg(header_offset + header.object_page_table_offset);
    LoadTable(is, header.page_count, object_pages);

    std::vector<uint32_t> fixup_record_offsets;
    is.seekg(header_offset + header.fixup_page_table_offset);
    fixup_record_offsets.resize(header.page_count + 1);
    for (size_t n = 0; n <= header.page_count; ++n) {
        ReadLe(is, fixup_record_offsets[n]);
    }

    LoadFixupTable(is, fixup_record_offsets, header_offset + header.fixup_record_table_offset);
}
