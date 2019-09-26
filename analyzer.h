#ifndef LE_DISASM_ANALYZER_H_
#define LE_DISASM_ANALYZER_H_

#include <stdint.h>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <deque>
#include <iomanip>
#include <iostream>
#include <map>

#include "dis_info.h"
#include "le/image.h"
#include "le/lin_ex.h"
#include "regions.h"
#include "symbol_map.h"

#define DEFINE_PATTERN_TABLE_ENTRY(pattern, signature) \
    { (pattern), (sizeof(pattern) - 1) }

struct Analyzer {
    Regions regions;
    std::deque<uint32_t> code_trace_queue;
    Image &image;
    DisInfo disasm;
    bool verbose;

    Analyzer(LinearExecutable &lx, Image &image_, bool verbose_) : regions(image_.objects, verbose_), image(image_) {
        verbose = verbose_;
    }

    void add_code_trace_address(uint32_t address, Type type, uint32_t refAddress = 0) {
        if (type == FUNCTION)
            this->code_trace_queue.push_front(address);
        else
            this->code_trace_queue.push_back(address);

        regions.labelTypes[address] = type;
        if (refAddress > 0) {
            if (verbose) printAddress(printAddress(std::cerr, refAddress) << " schedules ", address) << std::endl;
        }
    }

    void trace_code(void) {
        uint32_t address;

        while (!this->code_trace_queue.empty()) {
            address = this->code_trace_queue.front();
            this->code_trace_queue.pop_front();
            this->trace_code_at_address(address);
        }
    }

    void trace_code_at_address(uint32_t start_addr) {
        Region *reg = regions.regionContaining(start_addr);
        if (reg == NULL) {
            printAddress(std::cerr, start_addr, "Warning: Tried to trace code at an unmapped address: 0x") << std::endl;
            return;
        }

        const ImageObject &obj = image.objectAt(start_addr);
        if (reg->type() == CODE || reg->type() == DATA) { /* already traced */
            if (reg->type() == CODE) {
                std::map<uint32_t, Type>::iterator label = regions.labelTypes.find(start_addr);
                if (regions.labelTypes.end() != label && label->second == FUNC_GUESS) {
                    Insn inst(std::addressof(obj));
                    disasm.disassemble(start_addr, obj.get_data_at(start_addr), reg->end_address() - start_addr, inst);
                    label->second = (strstr(inst.text, "push") == inst.text ||
                                     (strstr(inst.text, "sub") == inst.text && strstr(inst.text, ",%esp") != NULL))
                                        ? FUNCTION
                                        : JUMP;
                }
            }
            return;
        } else if (regions.labelTypes.end() == regions.labelTypes.find(start_addr)) {
            printAddress(std::cerr, start_addr, "Warning: Tracing code without label: 0x") << std::endl;
            // FIXME: generate label
        } else if (reg->type() == SWITCH) {
            /* avoid retracing switches */
            return;
        }

        Type type = CODE;
        uint32_t nopCount = 0;
        uint32_t addr = traceRegionUntilAnyJump(reg, start_addr, obj.get_data_at(0), type, nopCount);
        if (nopCount == (addr - start_addr)) {
            type = DATA;
        }
        if (DATA == type) {
            std::map<uint32_t, Type>::iterator label = regions.labelTypes.find(start_addr);
            if (regions.labelTypes.end() != label) {
                label->second = DATA;
            }
        }
        regions.splitInsert(*reg, Region(start_addr, addr - start_addr, type));
    }

    size_t traceRegionUntilAnyJump(Region *&tracedReg, uint32_t &startAddress, const void *offset, Type &type,
                                   uint32_t &nopCount) {
        uint32_t addr = startAddress;
        for (Insn inst(tracedReg->image_object_pointer()); addr < tracedReg->end_address();) {
            disassemble(addr, tracedReg, inst, (uint8_t *)offset + addr, type);
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

                    /* Any FPU instruction-referenced fixup has to be data.
                     *
                     * Note that the FS segment override instruction prefix byte may be applied to disassembled
                     * instructions.
                     * E.g. 0x647Fxx converts to FS JG rel8, which should not be misinterpreted as an FPU instruction.
                     */
                } else if (DATA != type && *inst.text == 'f' && inst.memoryAddress > 0 &&
                           strncmp(inst.text, "fs ", strlen("fs ")) != 0) {
                    Region *reg = regions.regionContaining(inst.memoryAddress);
                    if (reg == NULL) {
                        continue;
                    } else if (reg->type() == UNKNOWN) {
                        if (strstr(inst.text, "t ") != NULL) {
                            regions.splitInsert(*reg, Region(inst.memoryAddress, 10, DATA));
                        } else if (strstr(inst.text, "l ") != NULL) {
                            regions.splitInsert(*reg, Region(inst.memoryAddress, 8, DATA));
                        } else {
                            throw Error() << "0x" << std::hex << addr - inst.size
                                          << ": unsupported FPU operand size in " << inst.text;
                        }
                        if (tracedReg == reg) {
                            tracedReg = regions.regionContaining(inst.memoryAddress + 10);
                        }
                    } else if (reg->type() != DATA) {
                        printAddress(std::cerr, inst.memoryAddress, "Warning: 0x") << " marked as data" << std::endl;
                    }
                    regions.labelTypes[inst.memoryAddress] = DATA;
                } else if (addr - inst.size == startAddress && strstr(inst.text, "mov    $") == inst.text) {
                    uint32_t dataAddress = strtol(&inst.text[strlen("mov    $")], NULL, 16);
                    if (regions.regionContaining(dataAddress) != NULL) {
                        const ImageObject &obj = image.objectAt(dataAddress);
                        if (strncmp("ABNORMAL TERMINATION", (const char *)(obj.get_data_at(dataAddress)),
                                    strlen("ABNORMAL TERMINATION")) == 0) {
                            printAddress(printAddress(std::cerr, startAddress) << ": ___abort signature found at ",
                                         dataAddress)
                                << std::endl;
                            regions.labelTypes[startAddress] = FUNCTION;  // eases further script-based transformation
                        }
                    }
                } else if (inst.bitness() == BITNESS_16BIT and inst.memoryAddress > 0) {
                    /* assume that ds and cs equal segment base in 16 bit mode */
                    uint32_t virtual_address = inst.base_address() + inst.memoryAddress;
                    Region *reg = regions.regionContaining(virtual_address);
                    if (reg and reg->type() == DATA) {
                        regions.labelTypes[virtual_address] = DATA;
                    }
                }
            }
        }
        return addr;
    }

    void disassemble(uint32_t addr, Region *&tracedReg, Insn &inst, const void *data_ptr, Type type) {
        uint32_t end_addr = tracedReg->end_address();
        for (disasm.disassemble(addr, data_ptr, end_addr - addr, inst); inst.memoryAddress == 0 || DATA == type;) {
            return;
        }
        if ((Insn::COND_JUMP == inst.type || Insn::JUMP == inst.type) && strstr(inst.text, "*") == NULL) {
            add_code_trace_address(inst.memoryAddress, JUMP, addr);
        } else if (Insn::CALL == inst.type) {
            add_code_trace_address(inst.memoryAddress, FUNCTION, addr);
        }
    }

    size_t addSwitchAddresses(std::map<uint32_t /*offset*/, uint32_t /*address*/> &fixups, size_t size,
                              const ImageObject &obj, uint32_t address) {
        size_t count = 0;
        uint32_t offset = address - obj.base_address();
        const uint8_t *data_ptr = obj.get_data_at(address);
        switch (obj.bitness()) {
            case Bitness::BITNESS_32BIT: {
                for (size_t off = 0; off + sizeof(uint32_t) <= size; off += sizeof(uint32_t), ++count) {
                    uint32_t case_address = read_le<uint32_t>(data_ptr + off);
                    Region *reg = regions.regionContaining(case_address);
                    if (reg == NULL) {
                        break;
                    }
                    if (case_address != 0) {
                        if (fixups.find(offset + off) == fixups.end()) {
                            break;
                        }
                        if (reg->type() == DATA) {
                            add_code_trace_address(case_address, DATA);
                        } else {
                            add_code_trace_address(case_address, CASE);
                        }
                    }
                }
            } break;
            case Bitness::BITNESS_16BIT:
                for (size_t off = 0; off + sizeof(uint16_t) <= size; off += sizeof(uint16_t), ++count) {
                    uint32_t case_address = read_le<uint16_t>(data_ptr + off) + obj.base_address();
                    Region *reg = regions.regionContaining(case_address);
                    if (reg == NULL) {
                        break;
                    }
                    if (case_address != 0) {
                        if (fixups.find(offset + off) == fixups.end()) {
                            break;
                        }
                        if (reg->type() == DATA) {
                            add_code_trace_address(case_address, DATA);
                        } else {
                            add_code_trace_address(case_address, CASE);
                        }
                    }
                }
                break;
        }
        return count;
    }

    void traceRegionSwitches(LinearExecutable &lx, std::map<uint32_t /*offset*/, uint32_t /*address*/> &fixups,
                             Region &reg, uint32_t address) {
        const ImageObject &obj = image.objectAt(reg.address());
        if (!obj.is_executable()) {
            return;
        }
        size_t size = reg.end_address() - address;
        std::set<uint32_t>::iterator iter = lx.fixup_addresses.upper_bound(address);
        if (lx.fixup_addresses.end() != iter) {
            size = std::min<size_t>(size, *iter - address);
        }
        size_t count = addSwitchAddresses(fixups, size, obj, address);
        if (count > 0) {
            if (reg.bitness() == Bitness::BITNESS_32BIT) {
                size = sizeof(uint32_t) * count;
            } else {
                size = sizeof(uint16_t) * count;
            }
            regions.splitInsert(reg, Region(address, size, SWITCH));
            regions.labelTypes[address] = SWITCH;
            trace_code();  // TODO: is returning not enough?
        }
    }

    void traceSwitches(LinearExecutable &lx, std::map<uint32_t /*offset*/, uint32_t /*address*/> &fixups) {
        for (std::map<uint32_t, uint32_t>::const_iterator itr = fixups.begin(); itr != fixups.end(); ++itr) {
            Region *reg = regions.regionContaining(itr->second);
            if (reg == NULL) {
                printAddress(std::cerr, itr->second, "Warning: Removing reloc pointing to unmapped memory at 0x")
                    << std::endl;
                lx.fixup_addresses.erase(itr->second);
                continue;
            } else if (reg->type() == UNKNOWN) {
                traceRegionSwitches(lx, fixups, *reg, itr->second);
            }
        }
    }

    void traceSwitches(LinearExecutable &lx) {
        for (size_t n = 0; n < lx.objects.size(); ++n) {
            traceSwitches(lx, lx.fixups[n]);
        }
    }

    void addAddress(size_t &guess_count, uint32_t address) {
        Type &type = regions.labelTypes[address];
        if (FUNCTION != type and JUMP != type) {
            if (verbose) printAddress(std::cerr, address, "Guessing that 0x") << " is a function" << std::endl;
            ++guess_count;
            type = FUNC_GUESS;
        }
        add_code_trace_address(address, type);
    }

    void addAddressesFromUnknownRegions(size_t &guess_count,
                                        std::map<uint32_t /*offset*/, uint32_t /*address*/> &fixups) {
        for (std::map<uint32_t, uint32_t>::const_iterator itr = fixups.begin(); itr != fixups.end(); ++itr) {
            Region *reg = regions.regionContaining(itr->second);
            if (reg == NULL) {
                continue;
            } else if (reg->type() == UNKNOWN) {
                addAddress(guess_count, itr->second);
            } else if (reg->type() == DATA) {
                regions.labelTypes[itr->second] = DATA;
            }
        }
    }

    void trace_remaining_relocs(LinearExecutable &lx) {
        size_t guess_count = 0;
        for (size_t n = 0; n < image.objects.size(); ++n) {
            addAddressesFromUnknownRegions(guess_count, lx.fixups[n]);
        }
        if (verbose) std::cerr << std::dec << guess_count << " guess(es) to investigate" << std::endl;
    }

    void process_map(SymbolMap *map) {
        for (std::map<uint32_t, SymbolMap::Properties>::iterator it = map->map.begin(); it != map->map.end(); ++it) {
            const SymbolMap::Properties &item = it->second;
            const Region *const reg = regions.regionContaining(item.address);

            if (item.type == FUNCTION) {
                Type label;
                if (regions.get_label_type(item.address, &label) && label == FUNCTION) {
                    continue;
                }
                add_code_trace_address(item.address, item.type);
                if (verbose)
                    printAddress(std::cerr << "Map file " << map->getFileName() << " schedules ", item.address)
                        << std::endl;
            } else if (item.type == SWITCH) {
                const uint32_t address = item.address;
                const ImageObject &obj = image.objectAt(item.address);
                const uint8_t *data_ptr = obj.get_data_at(item.address);
                const size_t size = std::min<size_t>(item.size, reg->end_address() - address);

                switch (reg->bitness()) {
                    case Bitness::BITNESS_32BIT: {
                        size_t count = 0;

                        if (size < item.size)
                            std::cerr << "Map file object at address 0x" << std::hex << address
                                      << " does not fit into containing region." << std::endl;

                        for (size_t offset = 0; offset + sizeof(uint32_t) <= size;
                             offset += sizeof(uint32_t), ++count) {
                            uint32_t case_address = read_le<uint32_t>(data_ptr + offset);
                            Type label;
                            if (0 == regions.get_label_type(case_address, &label)) {
                                if (map->get_label_type(case_address, &label)) {
                                    if (label != DATA) {
                                        add_code_trace_address(case_address, label);
                                        if (verbose)
                                            printAddress(
                                                std::cerr << "Map file " << map->getFileName() << " schedules ",
                                                item.address)
                                                << std::endl;
                                    }
                                }
                            }
                        }
                        regions.splitInsert((Region &)*reg, Region(address, sizeof(uint32_t) * count, SWITCH));
                        regions.labelTypes[address] = SWITCH;
                    } break;
                    case Bitness::BITNESS_16BIT: {
                        size_t count = 0;

                        if (size < item.size)
                            std::cerr << "Warning: Map file object at address 0x" << std::hex << address
                                      << " does not fit into containing region." << std::endl;

                        for (size_t offset = 0; offset + sizeof(uint16_t) <= size;
                             offset += sizeof(uint16_t), ++count) {
                            uint32_t case_address = read_le<uint16_t>(data_ptr + offset) + obj.base_address();
                            Type label;
                            if (regions.get_label_type(case_address, &label)) {
                                continue;
                            }

                            if (map->get_label_type(case_address, &label)) {
                                if (label != DATA) {
                                    add_code_trace_address(case_address, label);
                                    if (verbose)
                                        printAddress(std::cerr << "Map file " << map->getFileName() << " schedules ",
                                                     item.address)
                                            << std::endl;
                                }
                            }
                        }
                        regions.splitInsert((Region &)*reg, Region(address, sizeof(uint16_t) * count, SWITCH));
                        regions.labelTypes[address] = SWITCH;
                    } break;
                    default:
                        break;
                }
            } else if (item.type == DATA) {
                Type label;
                if (regions.get_label_type(item.address, &label) && label == DATA) {
                    continue;
                }
                const size_t size = std::min<size_t>(item.size, reg->end_address() - item.address);
                if (size < item.size)
                    std::cerr << "Warning: Map file object at address 0x" << std::hex << item.address
                              << " does not fit into containing region." << std::endl;
                regions.splitInsert((Region &)*reg, Region(item.address, size, DATA));
            }
        }
        trace_code();
    }

    bool is_align_pattern(uint32_t size, const uint8_t data[]) {
        const struct {
            const char *pattern;
            size_t size;
        } patterns[] = {/*
                         * table elements must be ordered by size from longest to shortest
                         */
                        DEFINE_PATTERN_TABLE_ENTRY("\x8d\x92\x00\x00\x00\x00", "leal 0x00000000(%edx), %edx"),
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

    void trace_align() {
        for (std::map<uint32_t, Region>::iterator itr = regions.regions.begin(); itr != regions.regions.end(); ++itr) {
            Region &reg = itr->second;

            if ((regions.regions.end() != itr) && (reg.type() == UNKNOWN)) {
                const std::map<uint32_t, Region>::const_iterator next_itr = std::next(itr);
                if (regions.regions.end() != next_itr) {
                    const Region &next_reg = next_itr->second;
                    if (next_reg.type() != UNKNOWN && next_reg.type() != ALIGNMENT) {
                        uint32_t function_alignment = next_reg.alignment();

                        if (function_alignment >= reg.size()) {
                            const uint8_t *data_ptr = reg.image_object_pointer()->get_data_at(reg.address());
                            if (is_align_pattern(reg.size(), data_ptr)) {
                                reg.type(ALIGNMENT);
                            }
                        }
                    }
                }
            }
        }
    }

public:
    void run(LinearExecutable &lx, SymbolMap *map) {
        uint32_t eip = lx.entryPointAddress();
        add_code_trace_address(eip, FUNCTION);  // TODO: name it "_start"
        if (verbose)
            printAddress(std::cerr, eip, "Tracing code directly accessible from the entry point at 0x") << std::endl;
        trace_code();

        if (map) {
            std::cerr << "Loading map file..." << std::endl;
            process_map(map);
        }

        std::cerr << "Tracing text relocs for switches..." << std::endl;
        traceSwitches(lx);

        std::cerr << "Tracing remaining relocs for functions and data..." << std::endl;
        trace_remaining_relocs(lx);
        trace_code();

        trace_align();
    }
};

#endif /* LE_DISASM_ANALYZER_H_ */
