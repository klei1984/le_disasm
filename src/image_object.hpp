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

#ifndef LE_DISASM_IMAGE_OBJECT_HPP_
#define LE_DISASM_IMAGE_OBJECT_HPP_

#include <cstdint>
#include <vector>

#include "type.hpp"

class ImageObject {
public:
    void Init(size_t index, uint32_t base_address, bool executable, bool bitness, const std::vector<uint8_t>& data);

    const uint8_t* GetDataAt(uint32_t address) const;
    uint32_t BaseAddress() const;
    uint32_t Size() const;
    Bitness GetBitness() const;
    bool IsExecutable() const;
    size_t Index() const;

private:
    size_t m_index;
    uint32_t m_base_address;
    bool m_executable;
    ::Bitness m_bitness;
    std::vector<uint8_t> m_data;
};

#endif
