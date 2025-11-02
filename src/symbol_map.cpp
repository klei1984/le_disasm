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

#include "symbol_map.hpp"

#include <fstream>
#include <regex>

std::string SymbolMap::EscapeSymbolName(std::string name) {
    const std::regex re("[\\[\\]\\?\\(\\)@]{1}");
    return std::regex_replace(name, re, "_");
}

std::string SymbolMap::GetFileNameFromPath(std::string path) {
    const size_t last_slash_idx = path.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
        path.erase(0, last_slash_idx + 1);
    }

    return path;
}

const SymbolMapProperties* SymbolMap::GetMapItem(uint32_t address) {
    const std::map<uint32_t, SymbolMapProperties>::const_iterator item = map.find(address);
    if (map.end() != item) {
        return &item->second;
    }

    return 0;
}

uint32_t SymbolMap::GetLabelType(uint32_t address, Type* label) {
    const std::map<uint32_t, SymbolMapProperties>::iterator item = map.find(address);
    if (map.end() != item) {
        *label = item->second.type;
        return item->first;
    }
    return 0;
}

SymbolMap::SymbolMap(const char* path) {
    std::ifstream is(path, std::ofstream::in);

    if (is.is_open()) {
        file_name = GetFileNameFromPath(std::string(path));

        const std::regex re("^([^\\s]+)\\s+([^\\s]+)\\s+([0-9a-fA-F]+)\\s+([0-9a-fA-F]+)$");
        std::smatch m;
        std::string line;

        is.seekg(std::ios::beg);

        while (std::getline(is, line)) {
            if (std::regex_match(line, m, re)) {
                if (m.size() == 5) {
                    std::string name = EscapeSymbolName(m[1]);
                    std::string type = m[2];
                    uint32_t address = std::stol(m[3], 0, 16);
                    uint32_t size = std::stol(m[4], 0, 16);

                    if ((type.find("LUT") != std::string::npos) and (size % sizeof(uint32_t) == 0)) {
                        map[address] = SymbolMapProperties(address, size, name, SWITCH);
                    } else if (type.find("FUNC") != std::string::npos) {
                        map[address] = SymbolMapProperties(address, size, name, FUNCTION);
                    } else if (type.find("DATA") != std::string::npos) {
                        map[address] = SymbolMapProperties(address, size, name, DATA);
                    } else if (type.find("ASCII") != std::string::npos) {
                        map[address] = SymbolMapProperties(address, size, name, DATA);
                    } else if (type.find("JUMP") != std::string::npos) {
                        map[address] = SymbolMapProperties(address, size, name, JUMP);
                    }
                }
            }
        }
        is.close();
    }
}

std::string SymbolMap::FindSymbolName(const uint32_t address) {
    return map.count(address) ? map[address].name : std::string("");
}

std::string SymbolMap::GetFileName() { return file_name; }
