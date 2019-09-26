#ifndef LE_DISASM_OPTIONS_H_
#define LE_DISASM_OPTIONS_H_

#include <getopt.h>
#include <string>

class Options {
    int verbose;
    int version;
    int help;
    std::string binary_image_file;
    std::string map_file;
    std::string executable_file;
    enum { DUMP_IMAGE = 4, MAP_FILE = 5 };

public:
    Options(int argc, char **argv) {
        verbose = 0;
        version = 0;
        help = 0;
        binary_image_file = "";
        map_file = "";
        executable_file = "";

        struct option long_options[] = {/* long options */
                                        {"verbose", no_argument, &verbose, 1},
                                        {"brief", no_argument, &verbose, 0},
                                        {"version", no_argument, &version, 1},
                                        {"help", no_argument, &help, 1},
                                        {"dump-image", required_argument, 0, 'd'},
                                        {"map-file", required_argument, 0, 'm'},
                                        {0, 0, 0, 0}};

        {
            int c;

            for (;;) {
                int option_index = 0;

                c = getopt_long(argc, argv, "vbhd:m:", long_options, &option_index);
                if (c == -1) break;

                switch (c) {
                    case 0:
                        /* If this option set a flag, do nothing else now. */
                        if (long_options[option_index].flag != 0) break;
                        switch (option_index) {
                            case DUMP_IMAGE:
                                binary_image_file = optarg ? std::string(optarg) : "";
                                break;
                            case MAP_FILE:
                                map_file = optarg ? std::string(optarg) : "";
                                break;
                        }
                        break;

                    case 'v':
                        version = 1;
                        break;

                    case 'b':
                        version = 0;
                        break;

                    case 'c':
                        help = 1;
                        break;

                    case 'd':
                        binary_image_file = optarg ? std::string(optarg) : "";
                        break;

                    case 'm':
                        map_file = optarg ? std::string(optarg) : "";
                        break;

                    case '?':
                        /* getopt_long already printed an error message. */
                        break;

                    default:
                        /* skip unknown options */
                        break;
                }
            }

            if (optind < argc) {
                /* only take one input file */
                executable_file = std::string(argv[optind++]);
            }
        }
    }

    bool is_verbose() { return verbose ? true : false; }
    bool is_help() { return help ? true : false; }
    bool is_version() { return version ? true : false; }
    std::string &get_map_file() { return map_file; }
    std::string &get_binary_image_file() { return binary_image_file; }
    std::string &get_executable_file() { return executable_file; }
};

#endif /* LE_DISASM_OPTIONS_H_ */
