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

#include <cstring>
#include <fstream>

#ifndef PACKAGE
#define PACKAGE
#endif

#include "analyzer.hpp"
#include "emitter.hpp"
#include "image.hpp"
#include "linear_executable.hpp"
#include "options.hpp"
#include "symbol_map.hpp"

int main(int argc, char** argv) {
    Options options = Options(argc, argv);
    SymbolMap* map_ptr = 0;

    if (options.IsVersion()) {
        std::cout << "le_disasm version" << __DATE__ << std::endl;
        return 1;
    }

    if (options.IsHelp() || (argc < 2)) {
        std::cout << argv[0] << " <option(s)> <executable-file>\n";
        std::cout << "OPTIONS:\n"
                  << "  -v, --verbose\t\t\tPrint lots of debug information\n"
                  << "  -d <file>, --dump-image=<file>\tDump flat linear executable image to <file>\n"
                  << "  -t, --trim-padding\t\tTrim zero padding bytes from the start of dumped image (use with -d)\n"
                  << "  -m <map-file>, --map-file=<map-file>\tUse <map-file> to help <executable-file> analysis\n"
                  << "  -h, --help\t\t\tPrint this help message\n"
                  << "  -V, --version\t\t\tPrint version information\n"
                  << std::endl;
        return 1;
    }

    try {
        std::ifstream is(options.GetExecutableFile(), std::ios::binary);
        if (!is.is_open()) {
            std::cerr << "Error opening executable-file: " << options.GetExecutableFile() << std::endl;
            return -1;
        }

        LinearExecutable lx(is, options.IsVerbose());
        Image image(is, lx);

        if (options.GetBinaryImageFile().compare("") != 0) {
            if (image.OutputFlatMemoryDump(options.GetBinaryImageFile(), options.IsTrimPadding())) {
                std::cerr << "Dumped flat linear executable image to " << options.GetBinaryImageFile() << std::endl;
            }
        }

        if (options.GetMapFile().compare("") != 0) {
            map_ptr = new SymbolMap(options.GetMapFile().c_str());
        }

        Analyzer analyzer(lx, image, options.IsVerbose());
        analyzer.Run(lx, map_ptr);

        Emitter emitter(lx, image, analyzer, map_ptr);
        emitter.Run();

        if (map_ptr) {
            delete map_ptr;
        }
    } catch (const std::exception& e) {
        std::cerr << std::dec << e.what() << std::endl;
    }
}
