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

#ifndef LE_DISASM_LITTLE_ENDIAN_HPP_
#define LE_DISASM_LITTLE_ENDIAN_HPP_

#include <istream>

#include "error.hpp"

template <typename T, size_t Bytes>
static void ReadLe(const void* memory, T& value) {
    value = T(0);
    uint8_t* p = (uint8_t*)memory;
    for (size_t n = 0; n < Bytes; ++n) {
        value |= (T)p[n] << (n * 8);
    }
}

template <typename T>
void ReadLe(std::istream& is, T& ret) {
    char buffer[sizeof(T)];
    for (is.read(buffer, sizeof(T)); !is.good();) {
        throw Error() << "EOF";
    }
    ReadLe<T, sizeof(T)>(buffer, ret);
}

template <typename T>
T ReadLe(const void* memory) {
    T ret;
    ReadLe<T, sizeof(T)>(memory, ret);
    return ret;
}

template <typename T, size_t Bytes>
static void WriteLe(const void* memory, T value) {
    uint8_t* p = (uint8_t*)memory;
    for (size_t n = 0; n < Bytes; ++n) {
        p[n] = value & 0xff;
        value >>= 8;
    }
}

template <typename T>
void WriteLe(const void* memory, T value) {
    WriteLe<T, sizeof(T)>(memory, value);
}

#endif
