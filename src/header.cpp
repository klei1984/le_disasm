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

#include "header.hpp"

#include <cstring>

#include "error.hpp"
#include "little_endian.hpp"

void Header::ThrowOnInvalidSignature(std::istream& is, uint32_t& header_offset) {
    char id[3] = {"??"};
    is.seekg(0);
    is.read(id, 2);
    if (strcmp(id, "MZ") && strcmp(id, "LE") && strcmp(id, "LX")) {
        throw Error() << "Invalid MZ signature: " << id;
    } else if (!strcmp(id, "MZ")) {
        is.seekg(0x18);
        uint8_t byte;
        ReadLe(is, byte);
        if (byte < 0x40) {
            throw Error() << "Not a LE executable, at offset 0x18: expected 0x40 or more, got 0x" << std::hex << byte;
        }
        is.seekg(0x3c);
        ReadLe(is, header_offset);
        is.seekg(header_offset);
        is.read(id, 2);
        if (strcmp(id, "LE")) {
            throw Error() << "Invalid LE signature: " << id;
        }
    }
}

Header::Header(std::istream& is, uint32_t& header_offset) {
    ThrowOnInvalidSignature(is, header_offset);
    ReadLe(is, byte_order);
    if (byte_order != 0) {
        throw Error() << "Only LITTLE_ENDIAN byte order supported: " << byte_order;
    }
    ReadLe(is, word_order);
    if (word_order != 0) {
        throw Error() << "Only LITTLE_ENDIAN word order supported: " << word_order;
    }
    ReadLe(is, format_version);
    if (format_version > 0) {
        throw Error() << "Unknown LE format version: " << format_version;
    }
    ReadLe(is, cpu_type);
    ReadLe(is, os_type);
    ReadLe(is, module_version);
    ReadLe(is, module_flags);
    ReadLe(is, page_count);
    ReadLe(is, eip_object_index);
    ReadLe(is, eip_offset);
    ReadLe(is, esp_object_index);
    ReadLe(is, esp_offset);
    ReadLe(is, page_size);
    ReadLe(is, last_page_size);
    ReadLe(is, fixup_section_size);
    ReadLe(is, fixup_section_check_sum);
    ReadLe(is, loader_section_size);
    ReadLe(is, loader_section_check_sum);
    ReadLe(is, object_table_offset);
    ReadLe(is, object_count);
    ReadLe(is, object_page_table_offset);
    ReadLe(is, object_iterated_pages_offset);
    ReadLe(is, resource_table_offset);
    ReadLe(is, resource_entry_count);
    ReadLe(is, resident_name_table_offset);
    ReadLe(is, entry_table_offset);
    ReadLe(is, module_directives_offset);
    ReadLe(is, module_directives_count);
    ReadLe(is, fixup_page_table_offset);
    ReadLe(is, fixup_record_table_offset);
    ReadLe(is, import_module_name_table_offset);
    ReadLe(is, import_module_name_entry_count);
    ReadLe(is, import_procedure_name_table_offset);
    ReadLe(is, per_page_check_sum_table_offset);
    ReadLe(is, data_pages_offset);
    ReadLe(is, preload_pages_count);
    ReadLe(is, non_resident_name_table_offset);
    ReadLe(is, non_resident_name_entry_count);
    ReadLe(is, non_resident_name_table_check_sum);
    ReadLe(is, auto_data_segment_object_index);
    ReadLe(is, debug_info_offset);
    ReadLe(is, debug_info_size);
    ReadLe(is, instance_pages_count);
    ReadLe(is, instance_pages_demand_count);
    ReadLe(is, heap_size);
    --eip_object_index;
    --esp_object_index;
}
