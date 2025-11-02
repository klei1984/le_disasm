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

#ifndef LE_DISASM_ANALYZER_HPP_
#define LE_DISASM_ANALYZER_HPP_

#include <cstdint>
#include <deque>
#include <map>

#include "dis_info.hpp"
#include "regions.hpp"

class LinearExecutable;
class Image;
class SymbolMap;
class Region;
class ImageObject;

#define DEFINE_PATTERN_TABLE_ENTRY(pattern, signature) {(pattern), (sizeof(pattern) - 1)}

class Analyzer {
public:
    Regions regions;
    std::deque<uint32_t> code_trace_queue;
    Image& image;
    DisInfo disasm;
    bool verbose;

    Analyzer(LinearExecutable& lx, Image& image_, bool verbose_);

    void Run(LinearExecutable& lx, SymbolMap* map);
    void AddCodeTraceAddress(uint32_t address, Type type, uint32_t refAddress = 0);

private:
    void TraceCode();
    void TraceCodeAtAddress(uint32_t start_addr);
    size_t TraceRegionUntilAnyJump(Region*& tracedReg, uint32_t& startAddress, const void* offset, Type& type,
                                   uint32_t& nopCount);
    void Disassemble(uint32_t addr, Region*& tracedReg, Insn& inst, const void* data_ptr, Type type);
    size_t AddSwitchAddresses(std::map<uint32_t, uint32_t>& fixups, size_t size, const ImageObject& obj,
                              uint32_t address);
    void TraceRegionSwitches(LinearExecutable& lx, std::map<uint32_t, uint32_t>& fixups, Region& reg, uint32_t address);
    void TraceSwitches(LinearExecutable& lx, std::map<uint32_t, uint32_t>& fixups);
    void TraceSwitches(LinearExecutable& lx);
    void AddAddress(size_t& guess_count, uint32_t address);
    void AddAddressesFromUnknownRegions(size_t& guess_count, std::map<uint32_t, uint32_t>& fixups);
    void TraceRemainingRelocs(LinearExecutable& lx);
    void ProcessMap(SymbolMap* map, LinearExecutable& lx);
    bool IsAlignPattern(uint32_t size, const uint8_t data[]);
    void TraceAlign();
};

#endif
