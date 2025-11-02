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

#ifndef LE_DISASM_EMITTER_HPP_
#define LE_DISASM_EMITTER_HPP_

#include <cstdint>
#include <map>

#include "type.hpp"

class LinearExecutable;
class Image;
class Analyzer;
class SymbolMap;
class Regions;
class Region;
class ImageObject;
class Insn;

class Emitter {
public:
    Emitter(LinearExecutable& lx, Image& img, Analyzer& anal, SymbolMap* map);
    virtual ~Emitter();
    void Run();

private:
    LinearExecutable& m_lx;
    Image& m_img;
    Regions& m_regions;
    std::map<uint32_t, Type>& m_label_types;
    SymbolMap* m_map;

    static int GetIndent(Type type);
    std::ostream& PrintTypedAddress(std::ostream& os, uint32_t address, Type type);
    std::ostream& PrintLabel(uint32_t address, Type type, char const* prefix = "");
    bool DataIsAddress(const ImageObject& obj, uint32_t addr, size_t len);
    bool DataIsZeros(const ImageObject& obj, uint32_t addr, size_t len, size_t& rlen);
    bool DataIsString(const ImageObject& obj, uint32_t addr, size_t len, size_t& rlen, bool& zero_terminated);
    static void PrintEscapedString(const uint8_t* data, size_t len);
    static void CompleteStringQuoting(int& bytes_in_line, int resetTo = 0);
    size_t GetLen(const Region& reg, const ImageObject& obj, std::map<uint32_t, uint32_t>& fups,
                  std::map<uint32_t, uint32_t>::const_iterator& itr, uint32_t address);
    void PrintDataAfterFixup(const ImageObject& obj, uint32_t& address, size_t len, int& bytes_in_line);
    void PrintEip();
    void PrintCode();
    void PrintRegion(const Region& reg);
    void PrintUnknownTypeRegion(const Region& reg);
    std::string ReplaceAddressesWithLabels(Insn& inst);
    void PrintInstruction(Insn& inst);
    void PrintCodeTypeRegion(const Region& reg);
    void PrintDataTypeRegion(const Region& reg);
    void PrintSwitchTypeRegion(const Region& reg);
    void PrintAlignmentTypeRegion(const Region& reg);
    void PrintChangedSectionType(const Region& reg, const Region* const reg_prev, Type& section);
};

#endif
