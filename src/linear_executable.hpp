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

#ifndef LE_DISASM_LINEAR_EXECUTABLE_HPP_
#define LE_DISASM_LINEAR_EXECUTABLE_HPP_

#include <cstdint>
#include <istream>
#include <map>
#include <set>
#include <vector>

#include "header.hpp"
#include "object_header.hpp"
#include "object_page_header.hpp"

class LinearExecutable {
public:
    Header header;
    std::vector<ObjectHeader> objects;
    std::vector<ObjectPageHeader> object_pages;
    std::vector<std::map<uint32_t, uint32_t> > fixups;
    std::set<uint32_t> fixup_addresses;
    bool verbose;

    LinearExecutable(std::istream& is, bool verbose, uint32_t header_offset = 0);

    uint32_t EntryPointAddress();
    size_t OffsetOfPageInFile(size_t index) const;

private:
    template <typename T>
    void LoadTable(std::istream& is, uint32_t count, std::vector<T>& ret);
    void LoadObjectFixups(std::istream& is, std::vector<uint32_t>& fixup_record_offsets, size_t table_offset,
                          size_t oi);
    void LoadFixupTable(std::istream& is, std::vector<uint32_t>& fixup_record_offsets, size_t table_offset);
};

#endif
