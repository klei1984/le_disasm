/* Copyright (C) 2019-2025  klei1984 <53688147+klei1984@users.noreply.github.com>
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

#include "symbol_map_properties.hpp"

SymbolMapProperties::SymbolMapProperties(uint32_t address_, uint32_t size_, std::string name_, Type type_) {
    address = address_;
    size = size_;
    name = name_;
    type = type_;
}

SymbolMapProperties::SymbolMapProperties(const SymbolMapProperties& other) { *this = other; }

SymbolMapProperties::SymbolMapProperties() {
    address = 0;
    size = 0;
    name = std::string("");
    type = UNKNOWN;
}
