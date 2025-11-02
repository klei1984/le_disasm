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

#include "print.hpp"

std::ostream& PrintAddress(std::ostream& os, uint32_t address, const char* prefix) {
    FlagsRestorer _(os);
    return os << prefix << std::setfill('0') << std::setw(6) << std::hex << std::noshowbase << address;
}

std::ostream& operator<<(std::ostream& os, Type type) {
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

std::ostream& operator<<(std::ostream& os, const Region& reg) {
    FlagsRestorer _(os);
    return PrintAddress(os << std::setw(7) << reg.GetType(), reg.Address(), " @ 0x")
           << "[" << std::setw(6) << std::setfill(' ') << std::dec << reg.Size() << "]";
}
