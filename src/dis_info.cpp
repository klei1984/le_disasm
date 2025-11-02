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

#include "dis_info.hpp"

#include "error.hpp"
#include "insn.hpp"
#include "type.hpp"

void DisInfo::CallbackPrintAddress(bfd_vma address, disassemble_info* info) {
    info->fprintf_func(info->stream, "0x00%llx", address);
    ((Insn*)info->stream)->memory_address = address;
}

DisInfo::DisInfo() {
    INIT_DISASSEMBLE_INFO(*this, NULL, &Insn::CallbackResetTypeAndText, &Insn::CallbackResetTypeAndText);
    print_address_func = CallbackPrintAddress;
}

void DisInfo::Disassemble(uint32_t addr, const void* data, size_t length, Insn& insn) {
    buffer = (bfd_byte*)data;
    buffer_length = length;
    buffer_vma = addr;
    stream = &insn;
    mach = insn.GetBitness() == BITNESS_32BIT ? bfd_mach_i386_i386 : bfd_mach_i386_i8086;

    insn.Reset();

    disassembler_ftype disasm_fn = disassembler(bfd_arch_i386, FALSE, mach, NULL);

    if (!disasm_fn) {
        throw Error() << "Failed to get disassembler function";
    }

    int size = disasm_fn(addr, this);
    if (size < 0) {
        throw Error() << "Failed to disassemble instruction";
    }
    insn.SetSize(size);
    if (size > 0) {
        insn.SetTargetAndType(addr, data);
    }
}
