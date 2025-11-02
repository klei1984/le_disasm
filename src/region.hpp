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

#ifndef LE_DISASM_REGION_HPP_
#define LE_DISASM_REGION_HPP_

#include <cstdint>
#include <iostream>

#include "image_object.hpp"

class Region {
public:
    Region(uint32_t address, uint32_t size, Type type, const ImageObject* image_object_pointer = NULL);
    Region();
    Region(const Region& other);

    uint32_t Address() const;
    size_t EndAddress() const;
    Type GetType() const;
    void SetType(::Type type);
    bool ContainsAddress(uint32_t address) const;
    size_t Size() const;
    void Size(size_t size);
    ::Bitness GetBitness() const;
    bool IsExecutable() const;
    const ImageObject* ImageObjectPointer() const;
    void ImageObjectPointer(const ImageObject* image_object_pointer);
    uint32_t Alignment() const;

private:
    const ImageObject* m_image_object_pointer;
    uint32_t m_address;
    uint32_t m_size;
    ::Type m_type;
};

std::ostream& operator<<(std::ostream& os, const Region& reg);

#endif
