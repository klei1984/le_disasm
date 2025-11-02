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

#ifndef LE_DISASM_SYMBOL_MAP_HPP_
#define LE_DISASM_SYMBOL_MAP_HPP_

#include <cstdint>
#include <iostream>
#include <map>
#include <string>

#include "symbol_map_properties.hpp"

class SymbolMap {
public:
    std::map<uint32_t, SymbolMapProperties> map;
    std::string file_name;

    SymbolMap(const char* path);

    std::string FindSymbolName(const uint32_t address);
    std::string GetFileName();
    const SymbolMapProperties* GetMapItem(uint32_t address);
    uint32_t GetLabelType(uint32_t address, Type* label);

private:
    std::string EscapeSymbolName(std::string name);
    std::string GetFileNameFromPath(std::string path);
};

#endif
