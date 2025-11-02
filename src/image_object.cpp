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

#include "image_object.hpp"

void ImageObject::Init(size_t index, uint32_t base_address, bool executable, bool bitness,
                       const std::vector<uint8_t>& data) {
    m_index = index;
    m_base_address = base_address;
    m_executable = executable;
    m_bitness = bitness ? BITNESS_32BIT : BITNESS_16BIT;
    m_data = data;
}

const uint8_t* ImageObject::GetDataAt(uint32_t address) const { return (&m_data.front() + address - m_base_address); }

uint32_t ImageObject::BaseAddress() const { return m_base_address; }

uint32_t ImageObject::Size() const { return m_data.size(); }

::Bitness ImageObject::GetBitness() const { return m_bitness; }

bool ImageObject::IsExecutable() const { return m_executable; }

size_t ImageObject::Index() const { return m_index; }
