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

#ifndef LE_DISASM_IMAGE_HPP_
#define LE_DISASM_IMAGE_HPP_

#include <cstdint>
#include <istream>
#include <map>
#include <string>
#include <vector>

#include "image_object.hpp"

class LinearExecutable;
class Header;
class ObjectHeader;

class Image {
public:
    std::vector<ImageObject> objects;

    Image(std::istream& is, LinearExecutable& lx);

    const ImageObject& ObjectAt(uint32_t address) const;
    bool IsValidAddress(const uint32_t address);
    bool OutputFlatMemoryDump(std::string& path, bool trim_padding = false);

private:
    void LoadObjectData(std::istream& is, LinearExecutable& lx, std::vector<uint8_t>& data, Header& hdr,
                        ObjectHeader& ohdr);
    void ApplyFixups(std::map<uint32_t, uint32_t>& fixups, std::vector<uint8_t>& data);
};

#endif
