#include <fstream>

#include "emitter.h"
#include "print.h"

Emitter::Emitter(LinearExecutable &lx_, Image &img_, Analyzer &anal_, SymbolMap *map_)
    : lx(lx_), img(img_), regions(anal_.regions), labelTypes(anal_.regions.labelTypes) {
    map = map_;
}

Emitter::~Emitter() {}

int Emitter::get_indent(Type type) {
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

std::ostream &Emitter::print_typed_address(std::ostream &os, uint32_t address, Type type) {
    if (map) {
        const SymbolMap::Properties *item = map->get_map_item(address);
        if (item) {
            return os << item->name;
        }
    }

    switch (type) {
        case FUNCTION:
            return printAddress(os, address, "_") << "_func";
        case FUNC_GUESS:
            return printAddress(os, address, "_") << "_func";  //"_funcGuess";
        case JUMP:
            return printAddress(os, address, "_") << "_jump";
        case DATA:
            return printAddress(os, address, "_") << "_data";
        case SWITCH:
            return printAddress(os, address, "_") << "_switch";
        case CASE:
            return printAddress(os, address, "_") << "_case";
        default:
            return printAddress(os, address, "_") << "_unknown";
    }
}

std::ostream &Emitter::print_label(uint32_t address, Type type, char const *prefix) {
    for (int indent = get_indent(type); indent-- > 0; std::cout << '\t') {
        ;
    }
    print_typed_address(std::cout << prefix, address, type) << ":";
    return std::cout;
}

bool Emitter::data_is_address(const ImageObject &obj, uint32_t addr, size_t len) {
    if (len >= 4) {
        const std::map<uint32_t, uint32_t> &fups = lx.fixups[obj.index()];
        return fups.find(addr - obj.base_address()) != fups.end();
    }
    return false;
}

bool Emitter::data_is_zeros(const ImageObject &obj, uint32_t addr, size_t len, size_t &rlen) {
    size_t x;
    const uint8_t *data = obj.get_data_at(addr);

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

bool Emitter::data_is_string(const ImageObject &obj, uint32_t addr, size_t len, size_t &rlen, bool &zero_terminated) {
    size_t x;
    const uint8_t *data = obj.get_data_at(addr);

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

void Emitter::print_escaped_string(const uint8_t *data, size_t len) {
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

void Emitter::complete_string_quoting(int &bytes_in_line, int resetTo) {
    if (bytes_in_line > 0) {
        std::cout << "\"\n";
        bytes_in_line = resetTo;
    }
}

size_t Emitter::get_len(const Region &reg, const ImageObject &obj,
                        std::map<uint32_t /*offset*/, uint32_t /*address*/> &fups,
                        std::map<uint32_t, uint32_t>::const_iterator &itr, uint32_t address) {
    size_t len = reg.end_address() - address;

    std::map<uint32_t, Type>::iterator label = labelTypes.upper_bound(address);
    if (labelTypes.end() != label) {
        len = std::min<size_t>(len, label->first - address);
    }

    while (fups.end() != itr and itr->first <= address - obj.base_address()) {
        ++itr;
    }

    if (itr != fups.end()) {
        len = std::min<size_t>(len, itr->first - (address - obj.base_address()));
    }
    return len;
}

void Emitter::print_data_after_fixup(const ImageObject &obj, uint32_t &address, size_t len, int &bytes_in_line) {
    size_t size;
    bool zt;

    while (len > 0) {
        /** @todo Handle 16 bit segments */
        if (data_is_address(obj, address, len)) {
            complete_string_quoting(bytes_in_line);
            uint32_t value = read_le<uint32_t>(obj.get_data_at(address));
            Type type;
            if (value != regions.get_label_type(value, &type)) {
                type = UNKNOWN;
                printAddress(std::cerr, value, "Warning: Printing address without label: 0x") << std::endl;
            }
            print_typed_address(std::cout << "\t\t.long   ", value, type) << std::endl;

            address += 4;
            len -= 4;
        } else if (data_is_zeros(obj, address, len, size)) {
            complete_string_quoting(bytes_in_line);

            std::cout << "\t\t.fill   0x" << std::hex << size << std::endl;
            address += size;
            len -= size;
        } else if (data_is_string(obj, address, len, size, zt)) {
            complete_string_quoting(bytes_in_line);

            if (zt) {
                std::cout << "\t\t.string \"";
            } else {
                std::cout << "\t\t.ascii   \"";
            }
            print_escaped_string(obj.get_data_at(address), size - zt);

            std::cout << "\"\n";

            address += size;
            len -= size;
        } else {
            char buffer[8];

            if (bytes_in_line == 0) std::cout << "\t\t.ascii  \"";

            snprintf(buffer, sizeof(buffer), "\\x%02x", *obj.get_data_at(address));
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

void Emitter::print_eip() {
    const ImageObject &obj = img.objectAt(lx.entryPointAddress());

    if (obj.bitness() == BITNESS_32BIT) {
        std::cout << ".code32" << std::endl;
    } else {
        std::cout << ".code16" << std::endl;
    }

    std::cout << ".text" << std::endl;
    std::cout << ".globl _start" << std::endl;
    std::cout << "_start:" << std::endl;

    print_typed_address(std::cout << "\t\tjmp\t", lx.entryPointAddress(), FUNCTION) << std::endl;
}

void Emitter::print_unknown_type_region(const Region &reg) {
    const ImageObject &obj = img.objectAt(reg.address());

    /* Emit unidentified region data for reference. Hex editors like wxHexEditor could be used to find and disassemble
     * the rendered raw data that could help further improve le_disasm analyzer and actual reengineering projects.
     */
    std::cout << "\n\t\t/* Skipped " << std::dec << reg.size() << " bytes of "
              << (obj.is_executable() ? "executable " : "") << reg.type() << " type data at virtual address 0x"
              << std::setfill('0') << std::setw(8) << std::hex << std::noshowbase << (uint32_t)reg.address() << ":";
    const uint8_t *data_pointer = obj.get_data_at(reg.address());
    for (uint8_t index = 0; index < reg.size() && data_pointer; ++index) {
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

std::string Emitter::replace_addresses_with_labels(Insn &inst) {
    std::ostringstream oss;
    size_t n, start;
    uint32_t addr;
    std::string addr_str;
    std::string comment;
    char prefix_symbol;
    const std::string &str = inst.text;

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
        if (inst.bitness() == BITNESS_16BIT and inst.memoryAddress == addr and inst.type == Insn::MISC) {
            /* assume that ds and cs equal segment base in 16 bit mode */
            uint32_t virtual_address = inst.base_address() + inst.memoryAddress;
            Region *reg = regions.regionContaining(virtual_address);
            if (reg and reg->type() == DATA) {
                // addr = virtual_address; GCC throws "relocation truncated to fit: R_386_16 against .data" error.
            }
        }
        std::map<uint32_t, Type>::const_iterator lab = labelTypes.find(addr);
        if (prefix_symbol != '-' /* && prefix_symbol != '$' */
            && labelTypes.end() != lab) {
            print_typed_address(oss, addr, lab->second);
        } else {
            printAddress(oss, addr);

            if (lx.fixup_addresses.find(addr) != lx.fixup_addresses.end()) {
                img.objectAt(addr);  // throws
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

void Emitter::print_instruction(Insn &inst) {
    std::string str;
    std::string::size_type n;

    str = replace_addresses_with_labels(inst);

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

void Emitter::print_code_type_region(const Region &reg) {
    const ImageObject &obj = img.objectAt(reg.address());
    DisInfo disasm;
    Insn inst(std::addressof(obj));

    for (uint32_t addr = reg.address(); addr < reg.end_address();) {
        std::map<uint32_t, Type>::iterator type = labelTypes.find(addr);
        if (labelTypes.end() != type) {
            //			if (CASE == type->second) {	// newline makes case not be part of function
            std::cout << std::endl;
            //			}
            print_label(addr, type->second) << std::endl;
        }

        disasm.disassemble(addr, obj.get_data_at(addr), reg.end_address() - addr, inst);
        if (labelTypes.end() == type && inst.size > 1) {  // hack for corrupted libraries
            type = labelTypes.find(addr + inst.size / 2);
            if (labelTypes.end() != type) {
                print_label(addr + inst.size / 2, type->second)
                    << "\t/* WARNING: instructions around this label are incorrect, generated just to workaround "
                       "corrupted library */"
                    << std::endl;
            }
        }
        print_instruction(inst);
        addr += inst.size;
    }
}

void Emitter::print_data_type_region(const Region &reg) {
    const ImageObject &obj = img.objectAt(reg.address());
    int bytes_in_line = 0;
    uint32_t addr = reg.address();
    std::map<uint32_t /*offset*/, uint32_t /*address*/> &fups = lx.fixups[obj.index()];
    for (std::map<uint32_t, uint32_t>::const_iterator itr = fups.begin(); addr < reg.end_address();) {
        std::map<uint32_t, Type>::iterator label = labelTypes.find(addr);
        if (labelTypes.end() != label) {
            complete_string_quoting(bytes_in_line);
            std::cout << std::endl;

            print_label(addr, DATA) << std::endl;
        }
        size_t len = get_len(reg, obj, fups, itr, addr);
        print_data_after_fixup(obj, addr, len, bytes_in_line);
    }
    complete_string_quoting(bytes_in_line, bytes_in_line);
}

void Emitter::print_switch_type_region(const Region &reg) {
    const ImageObject &obj = img.objectAt(reg.address());
    uint32_t func_addr, addr = reg.address();

    /* TODO: limit by relocs */
    print_label(addr, labelTypes[addr]) << std::endl;
    std::map<uint32_t, Type>::iterator next_label = labelTypes.upper_bound(addr);

    while (addr < reg.end_address()) {
        if (labelTypes.end() != next_label and addr == next_label->first) {
            print_label(addr, next_label->second) << std::endl;
            next_label = labelTypes.upper_bound(addr);
        }

        switch (obj.bitness()) {
            case Bitness::BITNESS_32BIT: {
                func_addr = read_le<uint32_t>(obj.get_data_at(addr));
                if (img.isValidAddress(func_addr)) {
                    if (addr < func_addr) {
                        if (labelTypes.end() == labelTypes.find(func_addr)) {
                            labelTypes[func_addr] = CASE;
                        }
                    }
                    print_typed_address(std::cout << "\t\t.long   ", func_addr, labelTypes[func_addr]) << std::endl;
                } else {
                    std::cout << "\t\t.long   0x" << std::hex << func_addr << std::endl;
                }
                addr += sizeof(uint32_t);
            } break;
            case Bitness::BITNESS_16BIT: {
                func_addr = read_le<uint16_t>(obj.get_data_at(addr));
                if (img.isValidAddress(func_addr)) {
                    if (addr < func_addr) {
                        if (labelTypes.end() == labelTypes.find(func_addr)) {
                            labelTypes[func_addr] = CASE;
                        }
                    }
                    print_typed_address(std::cout << "\t\t.short   ", func_addr, labelTypes[func_addr]) << std::endl;
                } else {
                    std::cout << "\t\t.short   0x" << std::hex << func_addr << std::endl;
                }
                addr += sizeof(uint16_t);
            } break;
        }
    }
    std::cout << std::endl;
}

void Emitter::print_alignment_type_region(const Region &reg) {
    Region *const next_reg = regions.nextRegion(reg);
    assert(next_reg);

    uint32_t alignment_next = next_reg->alignment();
    uint32_t counter = reg.size();
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

void Emitter::print_region(const Region &reg) {
    switch (reg.type()) {
        case UNKNOWN:
            print_unknown_type_region(reg);
            break;
        case CODE:
            print_code_type_region(reg);
            break;
        case DATA:
            print_data_type_region(reg);
            break;
        case SWITCH:
            print_switch_type_region(reg);
            break;
        case ALIGNMENT:
            print_alignment_type_region(reg);
            break;
        default:
            assert(0);
            break;
    }
}

void Emitter::print_changed_section_type(const Region &reg, const Region *const reg_prev, Type &section) {
    char sections[][6] = {"bug", ".text", ".data"};

    if (reg_prev and reg_prev->bitness() != reg.bitness()) {
        if (reg.bitness() == BITNESS_32BIT) {
            std::cout << std::endl << ".code32" << std::endl;
        } else {
            std::cout << std::endl << ".code16" << std::endl;
        }
    }

    if (reg.type() == DATA) {
        if (section != DATA) {
            std::cout << std::endl << sections[section = DATA] << std::endl;
        }
    } else {
        if (section != CODE) {
            std::cout << std::endl << sections[section = CODE] << std::endl;
        }
    }
}

void Emitter::print_code() {
    const Region *prev = NULL;
    const Region *next;
    Type section = CODE;

    std::cerr << "Region count: " << regions.regions.size() << std::endl;

    print_eip();

    for (std::map<uint32_t, Region>::const_iterator itr = regions.regions.begin(); itr != regions.regions.end();
         ++itr) {
        const Region &reg = itr->second;

        print_changed_section_type(reg, prev, section);

        print_region(reg);

        assert(prev == NULL || prev->end_address() <= reg.address());

        next = regions.nextRegion(reg);
        if (next == NULL or next->address() > reg.end_address()) {
            Type type;
            if (regions.get_label_type(reg.end_address(), &type)) {
                print_label(reg.end_address(), type) << std::endl;
            }
        }

        prev = &reg;
    }
}

void Emitter::run() { print_code(); }
