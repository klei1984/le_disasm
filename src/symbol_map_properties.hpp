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

#ifndef LE_DISASM_SYMBOL_MAP_PROPERTIES_HPP_
#define LE_DISASM_SYMBOL_MAP_PROPERTIES_HPP_

#include <cstdint>
#include <string>

#include "type.hpp"

class SymbolMapProperties {
public:
    std::string name;
    uint32_t address;
    uint32_t size;
    Type type;

    SymbolMapProperties(uint32_t address_, uint32_t size_, std::string name_, Type type_);
    SymbolMapProperties(const SymbolMapProperties& other);
    SymbolMapProperties();
};

#endif
