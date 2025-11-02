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

#ifndef LE_DISASM_HEADER_HPP_
#define LE_DISASM_HEADER_HPP_

#include <cstdint>
#include <istream>

class Header {
public:
    uint8_t byte_order;
    uint8_t word_order;
    uint32_t format_version;
    uint16_t cpu_type;
    uint16_t os_type;
    uint32_t module_version;
    uint32_t module_flags;
    uint32_t page_count;
    uint32_t eip_object_index;
    uint32_t eip_offset;
    uint32_t esp_object_index;
    uint32_t esp_offset;
    uint32_t page_size;
    uint32_t last_page_size;
    uint32_t fixup_section_size;
    uint32_t fixup_section_check_sum;
    uint32_t loader_section_size;
    uint32_t loader_section_check_sum;
    uint32_t object_table_offset;
    uint32_t object_count;
    uint32_t object_page_table_offset;
    uint32_t object_iterated_pages_offset;
    uint32_t resource_table_offset;
    uint32_t resource_entry_count;
    uint32_t resident_name_table_offset;
    uint32_t entry_table_offset;
    uint32_t module_directives_offset;
    uint32_t module_directives_count;
    uint32_t fixup_page_table_offset;
    uint32_t fixup_record_table_offset;
    uint32_t import_module_name_table_offset;
    uint32_t import_module_name_entry_count;
    uint32_t import_procedure_name_table_offset;
    uint32_t per_page_check_sum_table_offset;
    uint32_t data_pages_offset;
    uint32_t preload_pages_count;
    uint32_t non_resident_name_table_offset;
    uint32_t non_resident_name_entry_count;
    uint32_t non_resident_name_table_check_sum;
    uint32_t auto_data_segment_object_index;
    uint32_t debug_info_offset;
    uint32_t debug_info_size;
    uint32_t instance_pages_count;
    uint32_t instance_pages_demand_count;
    uint32_t heap_size;

    Header(std::istream& is, uint32_t& header_offset);

private:
    void ThrowOnInvalidSignature(std::istream& is, uint32_t& header_offset);
};

#endif
