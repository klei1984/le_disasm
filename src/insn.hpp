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

#ifndef LE_DISASM_INSN_HPP_
#define LE_DISASM_INSN_HPP_

#include <dis-asm.h>

#include <cstdint>

#include "type.hpp"

class ImageObject;

class Insn {
public:
    enum Type { MISC, COND_JUMP, JUMP, CALL, RET };

    Type type;
    char* text;
    size_t text_length;
    uint32_t memory_address;
    uint32_t instruction_address;
    size_t size;

    explicit Insn(const ImageObject* const image_object_pointer);

    ::Bitness GetBitness();
    uint32_t BaseAddress();

    static int CallbackResetTypeAndText(void* stream, const char* fmt, ...);
    static int CallbackResetTypeAndText(void* stream, enum disassembler_style style, const char* fmt, ...);

    void Reset();
    void SetSize(size_t size);
    void SetTargetAndType(uint32_t addr, const void* data);

private:
    char m_string[128];
    static int m_count;
    const ImageObject* const m_image_object_pointer;

    int LowerCasedSpaceTrimmed(int ret, char* end);
};

#endif
