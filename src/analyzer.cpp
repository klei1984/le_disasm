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

#include "analyzer.hpp"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "image.hpp"
#include "insn.hpp"
#include "linear_executable.hpp"
#include "little_endian.hpp"
#include "print.hpp"
#include "symbol_map.hpp"

Analyzer::Analyzer(LinearExecutable& lx, Image& image_, bool verbose_)
    : regions(image_.objects, verbose_), image(image_) {
    verbose = verbose_;
}

void Analyzer::AddCodeTraceAddress(uint32_t address, Type type, uint32_t refAddress) {
    if (type == FUNCTION)
        this->code_trace_queue.push_front(address);
    else
        this->code_trace_queue.push_back(address);

    regions.label_types[address] = type;
    if (refAddress > 0) {
        if (verbose) PrintAddress(PrintAddress(std::cerr, refAddress) << " schedules ", address) << std::endl;
    }
}

void Analyzer::TraceCode() {
    uint32_t address;

    while (!this->code_trace_queue.empty()) {
        address = this->code_trace_queue.front();
        this->code_trace_queue.pop_front();
        this->TraceCodeAtAddress(address);
    }
}

void Analyzer::TraceCodeAtAddress(uint32_t start_addr) {
    Region* reg = regions.RegionContaining(start_addr);
    if (reg == NULL) {
        PrintAddress(std::cerr, start_addr, "Warning: Tried to trace code at an unmapped address: 0x") << std::endl;
        return;
    }

    const ImageObject& obj = image.ObjectAt(start_addr);
    if (reg->GetType() == CODE || reg->GetType() == DATA) {
        if (reg->GetType() == CODE) {
            std::map<uint32_t, Type>::iterator label = regions.label_types.find(start_addr);
            if (regions.label_types.end() != label && label->second == FUNC_GUESS) {
                Insn inst(std::addressof(obj));
                disasm.Disassemble(start_addr, obj.GetDataAt(start_addr), reg->EndAddress() - start_addr, inst);
                label->second = (strstr(inst.text, "push") == inst.text ||
                                 (strstr(inst.text, "sub") == inst.text && strstr(inst.text, ",%esp") != NULL))
                                    ? FUNCTION
                                    : JUMP;
            }
        }
        return;
    } else if (regions.label_types.end() == regions.label_types.find(start_addr)) {
        PrintAddress(std::cerr, start_addr, "Warning: Tracing code without label: 0x") << std::endl;
    } else if (reg->GetType() == SWITCH) {
        return;
    }

    Type type = CODE;
    uint32_t nopCount = 0;
    uint32_t addr = TraceRegionUntilAnyJump(reg, start_addr, obj.GetDataAt(0), type, nopCount);
    if (nopCount == (addr - start_addr)) {
        type = DATA;
    }
    if (DATA == type) {
        std::map<uint32_t, Type>::iterator label = regions.label_types.find(start_addr);
        if (regions.label_types.end() != label) {
            label->second = DATA;
        }
    }
    regions.SplitInsert(*reg, Region(start_addr, addr - start_addr, type));
}

size_t Analyzer::TraceRegionUntilAnyJump(Region*& tracedReg, uint32_t& startAddress, const void* offset, Type& type,
                                         uint32_t& nopCount) {
    uint32_t addr = startAddress;
    for (Insn inst(tracedReg->ImageObjectPointer()); addr < tracedReg->EndAddress();) {
        Disassemble(addr, tracedReg, inst, (uint8_t*)offset + addr, type);
        for (addr += inst.size; Insn::JUMP == inst.type || Insn::RET == inst.type;) {
            return addr;
        }
        if (DATA != type) {
            char invalids[][6] = {"(bad)", "ss", "gs"};
            for (size_t i = 0; i < sizeof(invalids) / sizeof(invalids[0]); ++i) {
                if (strstr(inst.text, invalids[i]) == inst.text) {
                    type = DATA;
                    break;
                }
            }
            if (strstr(inst.text, "nop") == inst.text) {
                ++nopCount;

            } else if (DATA != type && *inst.text == 'f' && inst.memory_address > 0 &&
                       strncmp(inst.text, "fs ", strlen("fs ")) != 0) {
                Region* reg = regions.RegionContaining(inst.memory_address);
                if (reg == NULL) {
                    continue;
                } else if (reg->GetType() == UNKNOWN) {
                    if (strstr(inst.text, "t ") != NULL) {
                        regions.SplitInsert(*reg, Region(inst.memory_address, 10, DATA));
                    } else if (strstr(inst.text, "l ") != NULL) {
                        regions.SplitInsert(*reg, Region(inst.memory_address, 8, DATA));
                    } else {
                        throw Error() << "0x" << std::hex << addr - inst.size << ": unsupported FPU operand size in "
                                      << inst.text;
                    }
                    if (tracedReg == reg) {
                        tracedReg = regions.RegionContaining(inst.memory_address + 10);
                    }
                } else if (reg->GetType() != DATA) {
                    PrintAddress(std::cerr, inst.memory_address, "Warning: 0x") << " marked as data" << std::endl;
                }
                regions.label_types[inst.memory_address] = DATA;
            } else if (addr - inst.size == startAddress && strstr(inst.text, "mov    $") == inst.text) {
                uint32_t dataAddress = strtol(&inst.text[strlen("mov    $")], NULL, 16);
                if (regions.RegionContaining(dataAddress) != NULL) {
                    const ImageObject& obj = image.ObjectAt(dataAddress);
                    if (strncmp("ABNORMAL TERMINATION", (const char*)(obj.GetDataAt(dataAddress)),
                                strlen("ABNORMAL TERMINATION")) == 0) {
                        PrintAddress(PrintAddress(std::cerr, startAddress) << ": ___abort signature found at ",
                                     dataAddress)
                            << std::endl;
                        regions.label_types[startAddress] = FUNCTION;
                    }
                }
            } else if (inst.GetBitness() == BITNESS_16BIT and inst.memory_address > 0) {
                uint32_t virtual_address = inst.BaseAddress() + inst.memory_address;
                Region* reg = regions.RegionContaining(virtual_address);
                if (reg and reg->GetType() == DATA) {
                    regions.label_types[virtual_address] = DATA;
                }
            }
        }
    }
    return addr;
}

void Analyzer::Disassemble(uint32_t addr, Region*& tracedReg, Insn& inst, const void* data_ptr, Type type) {
    uint32_t end_addr = tracedReg->EndAddress();
    for (disasm.Disassemble(addr, data_ptr, end_addr - addr, inst); inst.memory_address == 0 || DATA == type;) {
        return;
    }
    if ((Insn::COND_JUMP == inst.type || Insn::JUMP == inst.type) && strstr(inst.text, "*") == NULL) {
        AddCodeTraceAddress(inst.memory_address, JUMP, addr);
    } else if (Insn::CALL == inst.type) {
        AddCodeTraceAddress(inst.memory_address, FUNCTION, addr);
    }
}

size_t Analyzer::AddSwitchAddresses(std::map<uint32_t, uint32_t>& fixups, size_t size, const ImageObject& obj,
                                    uint32_t address) {
    size_t count = 0;
    uint32_t offset = address - obj.BaseAddress();
    const uint8_t* data_ptr = obj.GetDataAt(address);
    switch (obj.GetBitness()) {
        case Bitness::BITNESS_32BIT: {
            for (size_t off = 0; off + sizeof(uint32_t) <= size; off += sizeof(uint32_t), ++count) {
                uint32_t case_address = ReadLe<uint32_t>(data_ptr + off);
                Region* reg = regions.RegionContaining(case_address);
                if (reg == NULL) {
                    break;
                }
                if (case_address != 0) {
                    if (fixups.find(offset + off) == fixups.end()) {
                        break;
                    }
                    if (reg->GetType() == DATA) {
                        AddCodeTraceAddress(case_address, DATA);
                    } else {
                        AddCodeTraceAddress(case_address, CASE);
                    }
                }
            }
        } break;
        case Bitness::BITNESS_16BIT:
            for (size_t off = 0; off + sizeof(uint16_t) <= size; off += sizeof(uint16_t), ++count) {
                uint32_t case_address = ReadLe<uint16_t>(data_ptr + off) + obj.BaseAddress();
                Region* reg = regions.RegionContaining(case_address);
                if (reg == NULL) {
                    break;
                }
                if (case_address != 0) {
                    if (fixups.find(offset + off) == fixups.end()) {
                        break;
                    }
                    if (reg->GetType() == DATA) {
                        AddCodeTraceAddress(case_address, DATA);
                    } else {
                        AddCodeTraceAddress(case_address, CASE);
                    }
                }
            }
            break;
    }
    return count;
}

void Analyzer::TraceRegionSwitches(LinearExecutable& lx, std::map<uint32_t, uint32_t>& fixups, Region& reg,
                                   uint32_t address) {
    const ImageObject& obj = image.ObjectAt(reg.Address());
    if (!obj.IsExecutable()) {
        return;
    }
    size_t size = reg.EndAddress() - address;
    std::set<uint32_t>::iterator iter = lx.fixup_addresses.upper_bound(address);
    if (lx.fixup_addresses.end() != iter) {
        size = std::min<size_t>(size, *iter - address);
    }
    size_t count = AddSwitchAddresses(fixups, size, obj, address);
    if (count > 0) {
        if (reg.GetBitness() == Bitness::BITNESS_32BIT) {
            size = sizeof(uint32_t) * count;
        } else {
            size = sizeof(uint16_t) * count;
        }
        regions.SplitInsert(reg, Region(address, size, SWITCH));
        regions.label_types[address] = SWITCH;
        TraceCode();
    }
}

void Analyzer::TraceSwitches(LinearExecutable& lx, std::map<uint32_t, uint32_t>& fixups) {
    for (std::map<uint32_t, uint32_t>::const_iterator itr = fixups.begin(); itr != fixups.end(); ++itr) {
        Region* reg = regions.RegionContaining(itr->second);
        if (reg == NULL) {
            PrintAddress(std::cerr, itr->second, "Warning: Removing reloc pointing to unmapped memory at 0x")
                << std::endl;
            lx.fixup_addresses.erase(itr->second);
            continue;
        } else if (reg->GetType() == UNKNOWN) {
            TraceRegionSwitches(lx, fixups, *reg, itr->second);
        }
    }
}

void Analyzer::TraceSwitches(LinearExecutable& lx) {
    for (size_t n = 0; n < lx.objects.size(); ++n) {
        TraceSwitches(lx, lx.fixups[n]);
    }
}

void Analyzer::AddAddress(size_t& guess_count, uint32_t address) {
    Type& type = regions.label_types[address];
    if (FUNCTION != type and JUMP != type) {
        if (verbose) PrintAddress(std::cerr, address, "Guessing that 0x") << " is a function" << std::endl;
        ++guess_count;
        type = FUNC_GUESS;
    }
    AddCodeTraceAddress(address, type);
}

void Analyzer::AddAddressesFromUnknownRegions(size_t& guess_count, std::map<uint32_t, uint32_t>& fixups) {
    for (std::map<uint32_t, uint32_t>::const_iterator itr = fixups.begin(); itr != fixups.end(); ++itr) {
        Region* reg = regions.RegionContaining(itr->second);
        if (reg == NULL) {
            continue;
        } else if (reg->GetType() == UNKNOWN) {
            AddAddress(guess_count, itr->second);
        } else if (reg->GetType() == DATA) {
            regions.label_types[itr->second] = DATA;
        }
    }
}

void Analyzer::TraceRemainingRelocs(LinearExecutable& lx) {
    size_t guess_count = 0;
    for (size_t n = 0; n < image.objects.size(); ++n) {
        AddAddressesFromUnknownRegions(guess_count, lx.fixups[n]);
    }
    if (verbose) std::cerr << std::dec << guess_count << " guess(es) to investigate" << std::endl;
}

void Analyzer::ProcessMap(SymbolMap* map, LinearExecutable& lx) {
    for (std::map<uint32_t, SymbolMapProperties>::iterator it = map->map.begin(); it != map->map.end(); ++it) {
        const SymbolMapProperties& item = it->second;
        const Region* const reg = regions.RegionContaining(item.address);

        if (item.type == FUNCTION) {
            Type label;
            if (regions.GetLabelType(item.address, &label) && label == FUNCTION) {
                continue;
            }
            AddCodeTraceAddress(item.address, item.type);
            if (verbose)
                PrintAddress(std::cerr << "Map file " << map->GetFileName() << " schedules ", item.address)
                    << std::endl;
        } else if (item.type == SWITCH) {
            const uint32_t address = item.address;
            const ImageObject& obj = image.ObjectAt(item.address);
            const uint8_t* data_ptr = obj.GetDataAt(item.address);
            const size_t size = std::min<size_t>(item.size, reg->EndAddress() - address);

            switch (reg->GetBitness()) {
                case Bitness::BITNESS_32BIT: {
                    size_t count = 0;

                    if (size < item.size)
                        std::cerr << "Map file object at address 0x" << std::hex << address
                                  << " does not fit into containing region." << std::endl;

                    for (size_t offset = 0; offset + sizeof(uint32_t) <= size; offset += sizeof(uint32_t), ++count) {
                        uint32_t case_address = ReadLe<uint32_t>(data_ptr + offset);
                        Type label;
                        if (0 == regions.GetLabelType(case_address, &label)) {
                            if (map->GetLabelType(case_address, &label)) {
                                if (label != DATA) {
                                    AddCodeTraceAddress(case_address, label);
                                    if (verbose)
                                        PrintAddress(std::cerr << "Map file " << map->GetFileName() << " schedules ",
                                                     item.address)
                                            << std::endl;
                                }
                            }
                        }
                    }
                    regions.SplitInsert((Region&)*reg, Region(address, sizeof(uint32_t) * count, SWITCH));
                    regions.label_types[address] = SWITCH;
                } break;
                case Bitness::BITNESS_16BIT: {
                    size_t count = 0;

                    if (size < item.size)
                        std::cerr << "Warning: Map file object at address 0x" << std::hex << address
                                  << " does not fit into containing region." << std::endl;

                    for (size_t offset = 0; offset + sizeof(uint16_t) <= size; offset += sizeof(uint16_t), ++count) {
                        uint32_t case_address = ReadLe<uint16_t>(data_ptr + offset) + obj.BaseAddress();
                        Type label;
                        if (regions.GetLabelType(case_address, &label)) {
                            continue;
                        }

                        if (map->GetLabelType(case_address, &label)) {
                            if (label != DATA) {
                                AddCodeTraceAddress(case_address, label);
                                if (verbose)
                                    PrintAddress(std::cerr << "Map file " << map->GetFileName() << " schedules ",
                                                 item.address)
                                        << std::endl;
                            }
                        }
                    }
                    regions.SplitInsert((Region&)*reg, Region(address, sizeof(uint16_t) * count, SWITCH));
                    regions.label_types[address] = SWITCH;
                } break;
                default:
                    break;
            }
        } else if (item.type == DATA) {
            Type label;
            if (regions.GetLabelType(item.address, &label) && label == DATA) {
                continue;
            }
            const size_t size = std::min<size_t>(item.size, reg->EndAddress() - item.address);
            if (size < item.size)
                std::cerr << "Warning: Map file object at address 0x" << std::hex << item.address
                          << " does not fit into containing region." << std::endl;
            regions.SplitInsert((Region&)*reg, Region(item.address, size, DATA));
        } else if (item.type == JUMP) {
            if (lx.fixup_addresses.find(item.address) != lx.fixup_addresses.end()) {
                regions.label_types[item.address] = JUMP;
            }
        }
    }
    TraceCode();
}

bool Analyzer::IsAlignPattern(uint32_t size, const uint8_t data[]) {
    const struct {
        const char* pattern;
        size_t size;
    } patterns[] = {DEFINE_PATTERN_TABLE_ENTRY("\x8d\x92\x00\x00\x00\x00", "leal 0x00000000(%edx), %edx"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x8d\x80\x00\x00\x00\x00", "leal 0x00000000(%eax), %eax"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x8d\x54\x22\x00", "leal 0x00(%edx), %edx"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x8d\x44\x20\x00", "leal (%eax), %eax"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x8d\x52\x00", "leal (%edx), %edx"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x8d\x40\x00", "leal (%eax), %eax"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x8B\xDB", "mov	%ebx, %ebx"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x8B\xD2", "mov	%edx, %edx"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x8B\xC9", "mov	%ecx, %ecx"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x8B\xC0", "mov %eax, %eax"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x87\xDB", "xchg %ebx, %ebx"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x90", "nop"),
                    DEFINE_PATTERN_TABLE_ENTRY("\x00", "null")};

    for (size_t index, offset = 0; offset < size;) {
        for (index = 0; index < sizeof(patterns) / sizeof(patterns[0]); index++) {
            if (patterns[index].size > size) continue;

            if (0 == std::memcmp(&data[offset], patterns[index].pattern, patterns[index].size)) {
                offset += patterns[index].size;
                break;
            }
        }
        if (index == (sizeof(patterns) / sizeof(patterns[0]))) {
            return false;
        }
    }
    return true;
}

void Analyzer::TraceAlign() {
    for (std::map<uint32_t, Region>::iterator itr = regions.regions.begin(); itr != regions.regions.end(); ++itr) {
        Region& reg = itr->second;

        if ((regions.regions.end() != itr) && (reg.GetType() == UNKNOWN)) {
            const std::map<uint32_t, Region>::const_iterator next_itr = std::next(itr);
            if (regions.regions.end() != next_itr) {
                const Region& next_reg = next_itr->second;
                if (next_reg.GetType() != UNKNOWN && next_reg.GetType() != ALIGNMENT) {
                    uint32_t function_alignment = next_reg.Alignment();

                    if (function_alignment >= reg.Size()) {
                        const uint8_t* data_ptr = reg.ImageObjectPointer()->GetDataAt(reg.Address());
                        if (IsAlignPattern(reg.Size(), data_ptr)) {
                            reg.SetType(ALIGNMENT);
                        }
                    }
                }
            }
        }
    }
}

void Analyzer::Run(LinearExecutable& lx, SymbolMap* map) {
    uint32_t eip = lx.EntryPointAddress();
    AddCodeTraceAddress(eip, FUNCTION);
    if (verbose)
        PrintAddress(std::cerr, eip, "Tracing code directly accessible from the entry point at 0x") << std::endl;
    TraceCode();

    if (map) {
        std::cerr << "Loading map file..." << std::endl;
        ProcessMap(map, lx);
    }

    std::cerr << "Tracing text relocs for switches..." << std::endl;
    TraceSwitches(lx);

    std::cerr << "Tracing remaining relocs for functions and data..." << std::endl;
    TraceRemainingRelocs(lx);
    TraceCode();

    TraceAlign();
}
