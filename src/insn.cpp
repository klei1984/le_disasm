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

#include "insn.hpp"

#include <unistd.h>

#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstring>

#include "error.hpp"
#include "image_object.hpp"
#include "little_endian.hpp"

int Insn::m_count = 0;

Insn::Insn(const ImageObject* const image_object_pointer) : m_image_object_pointer(image_object_pointer) {
    assert(m_image_object_pointer);
}

::Bitness Insn::GetBitness() { return m_image_object_pointer->GetBitness(); }

uint32_t Insn::BaseAddress() { return m_image_object_pointer->BaseAddress(); }

int Insn::LowerCasedSpaceTrimmed(int ret, char* end) {
    for (text = &m_string[0]; text < &m_string[text_length] + ret && isspace(*text); ++text);
    for (char* i = text; i <= end; *i = tolower(*i), ++i);
    *(end + 1) = 0;
    text_length = end + 1 - text;
    return ret;
}

int Insn::CallbackResetTypeAndText(void* stream, const char* fmt, ...) {
    va_list list;
    Insn* insn = (Insn*)stream;
    va_start(list, fmt);
    int ret = vsnprintf(&insn->m_string[insn->text_length], sizeof(insn->m_string) - 1 - insn->text_length, fmt, list);
    va_end(list);
    insn->type = MISC;

    return insn->LowerCasedSpaceTrimmed(ret, &insn->m_string[insn->text_length] + ret - 1);
}

int Insn::CallbackResetTypeAndText(void* stream, enum disassembler_style style, const char* fmt, ...) {
    va_list list;
    Insn* insn = (Insn*)stream;
    va_start(list, fmt);
    int ret = vsnprintf(&insn->m_string[insn->text_length], sizeof(insn->m_string) - 1 - insn->text_length, fmt, list);
    va_end(list);

    insn->type = MISC;

    return insn->LowerCasedSpaceTrimmed(ret, &insn->m_string[insn->text_length] + ret - 1);
}

void Insn::Reset() {
    memory_address = 0;
    text_length = 0;
    instruction_address = 0;
}

void Insn::SetSize(size_t size) { this->size = size; }

void Insn::SetTargetAndType(uint32_t addr, const void* data) {
    bool have_target = true;
    uint8_t data0 = ((uint8_t*)data)[0], data1 = 0;

    this->instruction_address = addr;

    if (data0 == 0x2e) {
        if (size > 1) {
            data0 = ((uint8_t*)data)[1];
        }
        if (size > 2) {
            data1 = ((uint8_t*)data)[2];
        }
    } else if (size > 1) {
        data1 = ((uint8_t*)data)[1];
    }

    if (data0 == 0x0f) {
        if (data1 >= 0x80 and data1 < 0x90) {
            type = COND_JUMP;
        }
    } else if (data0 == 0xe8) {
        type = CALL;
    } else if (data0 == 0xe9) {
        type = JUMP;
    } else if (data0 == 0x67 and data1 == 0xe3) {
        type = COND_JUMP;
    } else if (data0 == 0xc2) {
        type = RET;
    } else if (data0 == 0xca) {
        type = RET;
    } else if (data0 == 0xeb) {
        type = JUMP;
    } else if (data0 >= 0x70 and data0 < 0x80) {
        type = COND_JUMP;
    } else if (data0 >= 0xe0 and data0 <= 0xe3) {
        type = COND_JUMP;
    } else if (data0 == 0xe3) {
        type = JUMP;
    } else if (data0 == 0xcf) {
        type = RET;
    } else if (data0 == 0xc3) {
        type = RET;
    } else if (data0 == 0xcb) {
        type = RET;
    } else if (data0 == 0xff) {
        uint8_t reg_field = (data1 & 0x38) >> 3;
        if (reg_field == 2 or reg_field == 3) {
            type = CALL;
        } else if (reg_field == 4 or reg_field == 5) {
            type = JUMP;
        }
        have_target = false;
    }

    if (have_target and (type == COND_JUMP or type == JUMP or type == CALL)) {
        uint32_t address;

        if (GetBitness() == BITNESS_32BIT) {
            if (size < 5) {
                address = addr + size + ReadLe<int8_t>((uint8_t*)data + size - sizeof(int8_t));
            } else {
                address = addr + size + ReadLe<int32_t>((uint8_t*)data + size - sizeof(int32_t));
            }
        } else {
            if (size < 3) {
                address = addr + size + ReadLe<int8_t>((uint8_t*)data + size - sizeof(int8_t));
            } else {
                address = addr + size + ReadLe<int16_t>((uint8_t*)data + size - sizeof(int16_t));
            }
        }

        if (memory_address != 0 && address != memory_address) {
            throw Error() << "0x" << std::hex << memory_address << " discarded for 0x" << address;
        }
        memory_address = address;
    } else if (memory_address == 0) {
        char* addrStr = strstr(text, "s:0x");
        if (NULL != addrStr) {
            memory_address = strtol(addrStr + 2, NULL, 16);
        }
    }
}
