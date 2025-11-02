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

#ifndef LE_DISASM_STACKTRACE_HPP_
#define LE_DISASM_STACKTRACE_HPP_

#include <cstdio>
#include <cstdlib>

#ifdef UNIX_BUILD
#include <cxxabi.h>
#include <execinfo.h>
#endif

static inline void PrintStacktrace(FILE* out = stderr, unsigned int max_frames = 63) {
#ifdef UNIX_BUILD
    fprintf(out, "\nstack trace:\n");

    void* addrlist[max_frames + 1];

    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    if (addrlen == 0) {
        fprintf(out, "  <empty, possibly corrupt>\n");
        return;
    }

    char** symbollist = backtrace_symbols(addrlist, addrlen);

    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    for (int i = 1; i < addrlen; i++) {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        for (char* p = symbollist[i]; *p; ++p) {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            int status;
            char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
            if (status == 0) {
                funcname = ret;
                fprintf(out, "  %s : %s+%s\n", symbollist[i], funcname, begin_offset);
            } else {
                fprintf(out, "  %s : %s()+%s\n", symbollist[i], begin_name, begin_offset);
            }
        } else {
            fprintf(out, "  %s\n", symbollist[i]);
        }
    }

    free(funcname);
    free(symbollist);
#else
    fprintf(out, "\nStack trace not available on this platform\n");
#endif
}

#endif
