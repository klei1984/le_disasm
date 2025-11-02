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

#include "image.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>

#include "error.hpp"
#include "linear_executable.hpp"
#include "little_endian.hpp"

const ImageObject& Image::ObjectAt(uint32_t address) const {
    for (size_t n = 0; n < objects.size(); ++n) {
        const ImageObject& obj = objects[n];
        if (obj.BaseAddress() <= address and address < obj.BaseAddress() + obj.Size()) {
            return obj;
        }
    }
    throw Error() << "BUG: address out of image range: 0x" << std::setfill('0') << std::setw(6) << std::hex
                  << std::noshowbase << address;
}

bool Image::IsValidAddress(const uint32_t address) {
    for (size_t n = 0; n < objects.size(); ++n) {
        const ImageObject& obj = objects[n];
        if (obj.BaseAddress() <= address and address < obj.BaseAddress() + obj.Size()) {
            return true;
        }
    }
    return false;
}

void Image::LoadObjectData(std::istream& is, LinearExecutable& lx, std::vector<uint8_t>& data, Header& hdr,
                           ObjectHeader& ohdr) {
    size_t data_off = 0, page_end = std::min<size_t>(ohdr.first_page_index + ohdr.page_count, hdr.page_count);
    for (size_t page_idx = ohdr.first_page_index; page_idx < page_end; ++page_idx) {
        size_t size = std::min<size_t>(ohdr.virtual_size - data_off,
                                       (page_idx + 1 < hdr.page_count) ? hdr.page_size : hdr.last_page_size);
        is.seekg(lx.OffsetOfPageInFile(page_idx));
        if (!is.read((char*)&data.front() + data_off, size).good()) {
            throw Error() << "EOF";
        }
        data_off += size;
    }
}

void Image::ApplyFixups(std::map<uint32_t, uint32_t>& fixups, std::vector<uint8_t>& data) {
    for (std::map<uint32_t, uint32_t>::iterator itr = fixups.begin(); itr != fixups.end(); ++itr) {
        if (itr->first + 4 >= data.size()) {
            throw Error() << "Fixup points outside object boundaries";
        }
        void* ptr = &data.front() + itr->first;
        if (itr->second < 256) {
            WriteLe<uint16_t>(ptr, itr->second);
        } else {
            WriteLe<uint32_t>(ptr, itr->second);
        }
    }
}

bool Image::OutputFlatMemoryDump(std::string& path, bool trim_padding) {
    std::ofstream ofs(path, std::ofstream::binary);
    if (ofs.is_open()) {
        uint32_t min_address = 0;
        
        // Find the minimum base address across all objects if trim_padding is enabled
        if (trim_padding && !objects.empty()) {
            min_address = objects[0].BaseAddress();
            for (size_t oi = 1; oi < objects.size(); ++oi) {
                if (objects[oi].BaseAddress() < min_address) {
                    min_address = objects[oi].BaseAddress();
                }
            }
        }
        
        for (size_t oi = 0; oi < objects.size(); ++oi) {
            ofs.seekp(objects[oi].BaseAddress() - min_address);
            ofs.write((const char*)objects[oi].GetDataAt(objects[oi].BaseAddress()), objects[oi].Size());
        }
        ofs.close();
        return true;
    }
    return false;
}

Image::Image(std::istream& is, LinearExecutable& lx) {
    std::vector<uint8_t> data;
    objects.resize(lx.objects.size());
    for (size_t oi = 0; oi < lx.objects.size(); ++oi) {
        ObjectHeader& ohdr = lx.objects[oi];
        data.clear();
        data.resize(ohdr.virtual_size);
        LoadObjectData(is, lx, data, lx.header, ohdr);
        ApplyFixups(lx.fixups[oi], data);
        objects[oi].Init(oi, ohdr.base_address, ohdr.IsExecutable(), ohdr.Is32BitObject(), data);
    }
}
