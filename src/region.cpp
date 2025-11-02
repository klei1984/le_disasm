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

#include "region.hpp"

#include <cassert>

Region::Region(uint32_t address, uint32_t size, ::Type type, const ImageObject* image_object_pointer) {
    m_address = address;
    m_size = size;
    m_type = type;
    m_image_object_pointer = image_object_pointer;
}

Region::Region() {
    m_address = 0;
    m_size = 0;
    m_type = UNKNOWN;
    m_image_object_pointer = NULL;
}

Region::Region(const Region& other) { *this = other; }

uint32_t Region::Address() const { return m_address; }

size_t Region::EndAddress() const { return (m_address + m_size); }

::Type Region::GetType() const { return m_type; }

void Region::SetType(::Type type) { m_type = type; }

bool Region::ContainsAddress(uint32_t address) const { return (m_address <= address and address < m_address + m_size); }

size_t Region::Size() const { return m_size; }

void Region::Size(size_t size) { m_size = size; }

::Bitness Region::GetBitness() const {
    assert(m_image_object_pointer);
    return m_image_object_pointer->GetBitness();
}

bool Region::IsExecutable() const {
    assert(m_image_object_pointer);
    return m_image_object_pointer->IsExecutable();
}

const ImageObject* Region::ImageObjectPointer() const { return m_image_object_pointer; }

void Region::ImageObjectPointer(const ImageObject* image_object_pointer) {
    m_image_object_pointer = image_object_pointer;
}

uint32_t Region::Alignment() const {
    uint32_t align, mod, address;

    assert(m_image_object_pointer);
    address = m_address - m_image_object_pointer->BaseAddress();

    if (address == 0) return 1;

    for (align = 1, mod = 0; mod == 0;) {
        mod = address % (align * 2);
        if (mod)
            break;
        else
            align *= 2;
    }
    return align;
}
