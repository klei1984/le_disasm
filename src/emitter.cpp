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

#include "emitter.hpp"

#include <cassert>
#include <fstream>

#include "analyzer.hpp"
#include "dis_info.hpp"
#include "image.hpp"
#include "insn.hpp"
#include "linear_executable.hpp"
#include "little_endian.hpp"
#include "print.hpp"
#include "symbol_map.hpp"
#include "symbol_map_properties.hpp"

Emitter::Emitter(LinearExecutable& lx_, Image& img_, Analyzer& anal_, SymbolMap* map_)
    : m_lx(lx_), m_img(img_), m_regions(anal_.regions), m_label_types(anal_.regions.label_types) {
    m_map = map_;
}

Emitter::~Emitter() {}

int Emitter::GetIndent(Type type) {
    if (JUMP == type || CASE == type) {
        return 1;
    } else if (FUNCTION == type || FUNC_GUESS == type) {
        std::cout << "\n\n";
        //		print_separator();
    } else if (SWITCH == type) {
        std::cout << '\n';
    }
    return 0;
}

std::ostream& Emitter::PrintTypedAddress(std::ostream& os, uint32_t address, Type type) {
    if (m_map) {
        const SymbolMapProperties* item = m_map->GetMapItem(address);
        if (item) {
            return os << item->name;
        }
    }

    switch (type) {
        case FUNCTION:
            return PrintAddress(os, address, "_") << "_func";
        case FUNC_GUESS:
            return PrintAddress(os, address, "_") << "_func";
        case JUMP:
            return PrintAddress(os, address, "_") << "_jump";
        case DATA:
            return PrintAddress(os, address, "_") << "_data";
        case SWITCH:
            return PrintAddress(os, address, "_") << "_switch";
        case CASE:
            return PrintAddress(os, address, "_") << "_case";
        default:
            return PrintAddress(os, address, "_") << "_unknown";
    }
}

std::ostream& Emitter::PrintLabel(uint32_t address, Type type, char const* prefix) {
    for (int indent = GetIndent(type); indent-- > 0; std::cout << '\t') {
        ;
    }
    PrintTypedAddress(std::cout << prefix, address, type) << ":";
    return std::cout;
}

bool Emitter::DataIsAddress(const ImageObject& obj, uint32_t addr, size_t len) {
    if (len >= 4) {
        const std::map<uint32_t, uint32_t>& fups = m_lx.fixups[obj.Index()];
        return fups.find(addr - obj.BaseAddress()) != fups.end();
    }
    return false;
}

bool Emitter::DataIsZeros(const ImageObject& obj, uint32_t addr, size_t len, size_t& rlen) {
    size_t x;
    const uint8_t* data = obj.GetDataAt(addr);

    for (x = 0; x < len; x++) {
        if (data[x] != 0) {
            break;
        }
    }

    if (x < 4) {
        return false;
    }
    rlen = x;
    return true;
}

bool Emitter::DataIsString(const ImageObject& obj, uint32_t addr, size_t len, size_t& rlen, bool& zero_terminated) {
    size_t x;
    const uint8_t* data = obj.GetDataAt(addr);

    for (x = 0; x < len; x++) {
        if ((data[x] < 0x20 or data[x] >= 0x7f) and not(data[x] == '\t' or data[x] == '\n' or data[x] == '\r')) {
            break;
        }
    }

    if (x < 4) {
        return false;
    }

    if (x < len and data[x] == 0) {
        zero_terminated = true;
        x += 1;
    } else {
        zero_terminated = false;
    }
    rlen = x;
    return true;
}

void Emitter::PrintEscapedString(const uint8_t* data, size_t len) {
    size_t n;

    for (n = 0; n < len; n++) {
        if (data[n] == '\t')
            std::cout << "\\t";
        else if (data[n] == '\r')
            std::cout << "\\r";
        else if (data[n] == '\n')
            std::cout << "\\n";
        else if (data[n] == '\\')
            std::cout << "\\\\";
        else if (data[n] == '"')
            std::cout << "\\\"";
        else
            std::cout << (char)data[n];
    }
}

void Emitter::CompleteStringQuoting(int& bytes_in_line, int resetTo) {
    if (bytes_in_line > 0) {
        std::cout << "\"\n";
        bytes_in_line = resetTo;
    }
}

size_t Emitter::GetLen(const Region& reg, const ImageObject& obj,
                       std::map<uint32_t /*offset*/, uint32_t /*address*/>& fups,
                       std::map<uint32_t, uint32_t>::const_iterator& itr, uint32_t address) {
    size_t len = reg.EndAddress() - address;

    std::map<uint32_t, Type>::iterator label = m_label_types.upper_bound(address);
    if (m_label_types.end() != label) {
        len = std::min<size_t>(len, label->first - address);
    }

    while (fups.end() != itr and itr->first <= address - obj.BaseAddress()) {
        ++itr;
    }

    if (itr != fups.end()) {
        len = std::min<size_t>(len, itr->first - (address - obj.BaseAddress()));
    }
    return len;
}

void Emitter::PrintDataAfterFixup(const ImageObject& obj, uint32_t& address, size_t len, int& bytes_in_line) {
    size_t size;
    bool zt;

    while (len > 0) {
        /** @todo Handle 16 bit segments */
        if (DataIsAddress(obj, address, len)) {
            CompleteStringQuoting(bytes_in_line);
            uint32_t value = ReadLe<uint32_t>(obj.GetDataAt(address));
            Type type;
            if (value != m_regions.GetLabelType(value, &type)) {
                type = UNKNOWN;
                PrintAddress(std::cerr, value, "Warning: Printing address without label: 0x") << std::endl;
            }
            PrintTypedAddress(std::cout << "\t\t.long   ", value, type) << std::endl;

            address += 4;
            len -= 4;
        } else if (DataIsZeros(obj, address, len, size)) {
            CompleteStringQuoting(bytes_in_line);

            std::cout << "\t\t.fill   0x" << std::hex << size << std::endl;
            address += size;
            len -= size;
        } else if (DataIsString(obj, address, len, size, zt)) {
            CompleteStringQuoting(bytes_in_line);

            if (zt) {
                std::cout << "\t\t.string \"";
            } else {
                std::cout << "\t\t.ascii   \"";
            }
            PrintEscapedString(obj.GetDataAt(address), size - zt);

            std::cout << "\"\n";

            address += size;
            len -= size;
        } else {
            char buffer[8];

            if (bytes_in_line == 0) std::cout << "\t\t.ascii  \"";

            snprintf(buffer, sizeof(buffer), "\\x%02x", *obj.GetDataAt(address));
            std::cout << buffer;

            bytes_in_line += 1;

            if (bytes_in_line == 8) {
                std::cout << "\"\n";
                bytes_in_line = 0;
            }

            address++;
            len--;
        }
    }
}

void Emitter::PrintEip() {
    const ImageObject& obj = m_img.ObjectAt(m_lx.EntryPointAddress());

    if (obj.GetBitness() == BITNESS_32BIT) {
        std::cout << ".code32" << std::endl;
    } else {
        std::cout << ".code16" << std::endl;
    }

    std::cout << ".text" << std::endl;
    std::cout << ".globl _start" << std::endl;
    std::cout << "_start:" << std::endl;

    PrintTypedAddress(std::cout << "\t\tjmp\t", m_lx.EntryPointAddress(), FUNCTION) << std::endl;
}

void Emitter::PrintUnknownTypeRegion(const Region& reg) {
    const ImageObject& obj = m_img.ObjectAt(reg.Address());

    /* Emit unidentified region data for reference. Hex editors like wxHexEditor could be used to find and disassemble
     * the rendered raw data that could help further improve le_disasm analyzer and actual reengineering projects.
     */
    std::cout << "\n\t\t/* Skipped " << std::dec << reg.Size() << " bytes of "
              << (obj.IsExecutable() ? "executable " : "") << reg.GetType() << " type data at virtual address 0x"
              << std::setfill('0') << std::setw(8) << std::hex << std::noshowbase << (uint32_t)reg.Address() << ":";
    const uint8_t* data_pointer = obj.GetDataAt(reg.Address());
    for (uint8_t index = 0; index < reg.Size() && data_pointer; ++index) {
        if (index >= 16) {
            std::cout << "\n\t\t * ...";
            break;
        }
        if (index % 8 == 0) {
            std::cout << "\n\t\t *\t";
        }
        std::cout << std::setfill('0') << std::setw(2) << std::hex << std::noshowbase << (uint32_t)data_pointer[index];
    }
    std::cout << "\n\t\t */" << std::endl;
}

std::string Emitter::ReplaceAddressesWithLabels(Insn& inst) {
    std::ostringstream oss;
    size_t n, start;
    uint32_t addr;
    std::string addr_str;
    std::string comment;
    char prefix_symbol;
    const std::string& str = inst.text;

    /* Many opcodes support displacement in indirect addressing modes.
     * Example: mov    %edx,-0x10(%ebp) .
     * Displacement constants are signed literals and should not be misinterpreted
     * as unsigned fixup addresses.
     */

    n = str.find("0x");
    if (n == std::string::npos) return str;

    start = 0;

    do {
        prefix_symbol = *str.substr(n - sizeof(char), sizeof(char)).c_str();
        oss << str.substr(start, n - start);

        if (n + 2 >= str.length()) break;

        n += 2;
        start = n;

        while (n < str.length() and isxdigit(str[n])) n++;

        addr_str = str.substr(start, n - start);
        addr = strtol(addr_str.c_str(), NULL, 16);
        if (inst.GetBitness() == BITNESS_16BIT and inst.memory_address == addr and inst.type == Insn::MISC) {
            /* assume that ds and cs equal segment base in 16 bit mode */
            uint32_t virtual_address = inst.BaseAddress() + inst.memory_address;
            Region* reg = m_regions.RegionContaining(virtual_address);
            if (reg and reg->GetType() == DATA) {
                // addr = virtual_address; GCC throws "relocation truncated to fit: R_386_16 against .data" error.
            }
        }
        std::map<uint32_t, Type>::const_iterator lab = m_label_types.find(addr);
        if (prefix_symbol != '-' /* && prefix_symbol != '$' */
            && m_label_types.end() != lab) {
            PrintTypedAddress(oss, addr, lab->second);
        } else {
            PrintAddress(oss, addr);

            if (m_lx.fixup_addresses.find(addr) != m_lx.fixup_addresses.end()) {
                m_img.ObjectAt(addr);
                comment = " /* Warning: address points to a valid object/reloc, but no label found */";
            }
        }

        start = n;
        n = str.find("0x", start);
    } while (n != std::string::npos);

    if (start < str.length()) {
        oss << str.substr(start);
    }
    if (!comment.empty()) {
        oss << comment;
    }
    return oss.str();
}

void Emitter::PrintInstruction(Insn& inst) {
    std::string str;
    std::string::size_type n;

    str = ReplaceAddressesWithLabels(inst);

    n = str.find("(287 only)");
    if (n != std::string::npos) {
        std::cout << "\t\t/* " << str << " -- ignored */\n";
        return;
    }

    n = str.find("(8087 only)");
    if (n != std::string::npos) {
        std::cout << "\t\t/* " << str << " -- ignored */\n";
        return;
    }

    /* Work around buggy libopcodes */
    if (str == "lar    %cx,%ecx") {
        str = "lar    %ecx,%ecx";
    } else if (str == "lsl    %ax,%eax") {
        str = "lsl    %eax,%eax";
    } else if (str == "lea    0x000000(%eax,%eiz,1),%eax") {
        str = "lea    0x000000(%eax),%eax";
    } else if (str == "lea    0x000000(%edx,%eiz,1),%edx") {
        str = "lea    0x000000(%edx),%edx";
    }
    std::cout << "\t\t" << str;

    if (str == "data16" or str == "data32") {
        std::cout << " ";
    } else {
        std::cout << "\n";
    }
}

void Emitter::PrintCodeTypeRegion(const Region& reg) {
    const ImageObject& obj = m_img.ObjectAt(reg.Address());
    DisInfo disasm;
    Insn inst(std::addressof(obj));

    for (uint32_t addr = reg.Address(); addr < reg.EndAddress();) {
        std::map<uint32_t, Type>::iterator type = m_label_types.find(addr);
        if (m_label_types.end() != type) {
            //			if (CASE == type->second) {	// newline makes case not be part of function
            std::cout << std::endl;
            //			}
            PrintLabel(addr, type->second) << std::endl;
        }

        disasm.Disassemble(addr, obj.GetDataAt(addr), reg.EndAddress() - addr, inst);
        if (m_label_types.end() == type && inst.size > 1) {  // hack for corrupted libraries
            type = m_label_types.find(addr + inst.size / 2);
            if (m_label_types.end() != type) {
                PrintLabel(addr + inst.size / 2, type->second)
                    << "\t/* WARNING: instructions around this label are incorrect, generated just to workaround "
                       "corrupted library */"
                    << std::endl;
            }
        }
        PrintInstruction(inst);
        addr += inst.size;
    }
}

void Emitter::PrintDataTypeRegion(const Region& reg) {
    const ImageObject& obj = m_img.ObjectAt(reg.Address());
    int bytes_in_line = 0;
    uint32_t addr = reg.Address();
    std::map<uint32_t /*offset*/, uint32_t /*address*/>& fups = m_lx.fixups[obj.Index()];
    for (std::map<uint32_t, uint32_t>::const_iterator itr = fups.begin(); addr < reg.EndAddress();) {
        std::map<uint32_t, Type>::iterator label = m_label_types.find(addr);
        if (m_label_types.end() != label) {
            CompleteStringQuoting(bytes_in_line);
            std::cout << std::endl;

            PrintLabel(addr, DATA) << std::endl;
        }
        size_t len = GetLen(reg, obj, fups, itr, addr);
        PrintDataAfterFixup(obj, addr, len, bytes_in_line);
    }
    CompleteStringQuoting(bytes_in_line, bytes_in_line);
}

void Emitter::PrintSwitchTypeRegion(const Region& reg) {
    const ImageObject& obj = m_img.ObjectAt(reg.Address());
    uint32_t func_addr, addr = reg.Address();

    /* TODO: limit by relocs */
    PrintLabel(addr, m_label_types[addr]) << std::endl;
    std::map<uint32_t, Type>::iterator next_label = m_label_types.upper_bound(addr);

    while (addr < reg.EndAddress()) {
        if (m_label_types.end() != next_label and addr == next_label->first) {
            PrintLabel(addr, next_label->second) << std::endl;
            next_label = m_label_types.upper_bound(addr);
        }

        switch (obj.GetBitness()) {
            case Bitness::BITNESS_32BIT: {
                func_addr = ReadLe<uint32_t>(obj.GetDataAt(addr));
                if (m_img.IsValidAddress(func_addr)) {
                    if (addr < func_addr) {
                        if (m_label_types.end() == m_label_types.find(func_addr)) {
                            m_label_types[func_addr] = CASE;
                        }
                    }
                    PrintTypedAddress(std::cout << "\t\t.long   ", func_addr, m_label_types[func_addr]) << std::endl;
                } else {
                    std::cout << "\t\t.long   0x" << std::hex << func_addr << std::endl;
                }
                addr += sizeof(uint32_t);
            } break;
            case Bitness::BITNESS_16BIT: {
                func_addr = ReadLe<uint16_t>(obj.GetDataAt(addr));
                if (m_img.IsValidAddress(func_addr)) {
                    if (addr < func_addr) {
                        if (m_label_types.end() == m_label_types.find(func_addr)) {
                            m_label_types[func_addr] = CASE;
                        }
                    }
                    PrintTypedAddress(std::cout << "\t\t.short   ", func_addr, m_label_types[func_addr]) << std::endl;
                } else {
                    std::cout << "\t\t.short   0x" << std::hex << func_addr << std::endl;
                }
                addr += sizeof(uint16_t);
            } break;
        }
    }
    std::cout << std::endl;
}

void Emitter::PrintAlignmentTypeRegion(const Region& reg) {
    Region* const next_reg = m_regions.NextRegion(reg);
    assert(next_reg);

    uint32_t alignment_next = next_reg->Alignment();
    uint32_t counter = reg.Size();
    uint32_t alignment;

    for (; counter;) {
        alignment = counter;
        counter = counter & (counter - 1);
    }
    alignment *= 2;
    if (alignment > alignment_next) {
        alignment = alignment_next;
    }
    std::cout << std::endl << ".align " << std::dec << alignment << std::endl;
}

void Emitter::PrintRegion(const Region& reg) {
    switch (reg.GetType()) {
        case UNKNOWN:
            PrintUnknownTypeRegion(reg);
            break;
        case CODE:
            PrintCodeTypeRegion(reg);
            break;
        case DATA:
            PrintDataTypeRegion(reg);
            break;
        case SWITCH:
            PrintSwitchTypeRegion(reg);
            break;
        case ALIGNMENT:
            PrintAlignmentTypeRegion(reg);
            break;
        default:
            assert(0);
            break;
    }
}

void Emitter::PrintChangedSectionType(const Region& reg, const Region* const reg_prev, Type& section) {
    char sections[][6] = {"bug", ".text", ".data"};

    if (reg_prev and reg_prev->GetBitness() != reg.GetBitness()) {
        if (reg.GetBitness() == BITNESS_32BIT) {
            std::cout << std::endl << ".code32" << std::endl;
        } else {
            std::cout << std::endl << ".code16" << std::endl;
        }
    }

    if (reg.GetType() == DATA) {
        if (section != DATA) {
            std::cout << std::endl << sections[section = DATA] << std::endl;
        }
    } else {
        if (section != CODE) {
            if (!reg.IsExecutable()) {
                section = DATA;
            } else {
                section = CODE;
            }
            std::cout << std::endl << sections[section] << std::endl;
        }
    }
}

void Emitter::PrintCode() {
    const Region* prev = NULL;
    const Region* next;
    Type section = CODE;

    std::cerr << "Region count: " << m_regions.regions.size() << std::endl;

    PrintEip();

    for (std::map<uint32_t, Region>::const_iterator itr = m_regions.regions.begin(); itr != m_regions.regions.end();
         ++itr) {
        const Region& reg = itr->second;

        PrintChangedSectionType(reg, prev, section);

        PrintRegion(reg);

        assert(prev == NULL || prev->EndAddress() <= reg.Address());

        next = m_regions.NextRegion(reg);
        if (next == NULL or next->Address() > reg.EndAddress()) {
            Type type;
            if (m_regions.GetLabelType(reg.EndAddress(), &type)) {
                PrintLabel(reg.EndAddress(), type) << std::endl;
            }
        }

        prev = &reg;
    }
}

void Emitter::Run() { PrintCode(); }
