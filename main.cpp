#include <cstring>
#include <fstream>
#define PACKAGE

#include "analyzer.h"
#include "emitter.h"
#include "options.h"
#include "symbol_map.h"

int main(int argc, char **argv) {
    Options options = Options(argc, argv);
    SymbolMap *map_ptr = 0;

    if (options.is_version()) {
        std::cout << "le_disasm version" << __DATE__ << std::endl;
        return 1;
    }

    if (options.is_help() || (argc < 2)) {
        std::cout << argv[0] << " <option(s)> <executable-file>\n";
        std::cout << "OPTIONS:\n"
                  << "  -v, --verbose\tPrint lots of debug information\n"
                  << "  -d <file>, --dump-image=<file>\tDump flat linear executable image to <file>\n"
                  << "  -m <map-file>, --map-file=<map-file>\tUse <map-file> to help <executable-file> analysis\n"
                  << "  -h, --help\tPrint this help message\n"
                  << "      --version\tPrint version information\n"
                  << std::endl;
        return 1;
    }

    try {
        std::ifstream is(options.get_executable_file());
        if (!is.is_open()) {
            std::cerr << "Error opening executable-file: " << options.get_executable_file() << std::endl;
            return -1;
        }

        LinearExecutable lx(is, options.is_verbose());
        Image image(is, lx);

        if (options.get_binary_image_file().compare("") != 0) {
            if (image.outputFlatMemoryDump(options.get_binary_image_file())) {
                std::cerr << "Dumped flat linear executable image to " << options.get_binary_image_file() << std::endl;
            }
        }

        if (options.get_map_file().compare("") != 0) {
            map_ptr = new SymbolMap(options.get_map_file());
        }

        Analyzer analyzer(lx, image, options.is_verbose());
        analyzer.run(lx, map_ptr);

        Emitter emitter(lx, image, analyzer, map_ptr);
        emitter.run();

        if (map_ptr) {
            delete map_ptr;
        }
    } catch (const std::exception &e) {
        std::cerr << std::dec << e.what() << std::endl;
    }
}
