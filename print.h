#ifndef LE_DISASM_PRINT_H_
#define LE_DISASM_PRINT_H_

#include "flags_restorer.h"

std::ostream &printAddress(std::ostream &os, uint32_t address, const char *prefix) {
    FlagsRestorer _(os);
    return os << prefix << std::setfill('0') << std::setw(6) << std::hex << std::noshowbase << address;
}

std::ostream &operator<<(std::ostream &os, Type type) {
    switch (type) {
        case UNKNOWN:
            return os << "unknown";
        case CODE:
            return os << "code";
        case DATA:
            return os << "data";
        case SWITCH:
            return os << "switch";
        case ALIGNMENT:
            return os << "alignment";
        default:
            return os << "(unknown " << type << ")";
    }
}

std::ostream &operator<<(std::ostream &os, const Region &reg) { /* %7s @ 0x%06x[%6d] */
    FlagsRestorer _(os);
    return printAddress(os << std::setw(7) << reg.type(), reg.address(), " @ 0x")
           << "[" << std::setw(6) << std::setfill(' ') << std::dec << reg.size() << "]";
}

#endif /* LE_DISASM_PRINT_H_ */
