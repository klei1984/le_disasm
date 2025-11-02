/* Copyright (C) 2019-2025  klei1984 <53688147+klei1984@users.noreply.github.com>
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

#include "options.hpp"

#include <getopt.h>

Options::Options(int argc, char** argv) {
    m_verbose = 0;
    m_version = 0;
    m_help = 0;
    m_trim_padding = 0;
    m_binary_image_file = "";
    m_map_file = "";
    m_executable_file = "";

    struct option long_options[] = {{"verbose", no_argument, &m_verbose, 1},
                                    {"brief", no_argument, &m_verbose, 0},
                                    {"version", no_argument, &m_version, 1},
                                    {"help", no_argument, &m_help, 1},
                                    {"dump-image", required_argument, 0, 'd'},
                                    {"map-file", required_argument, 0, 'm'},
                                    {"trim-padding", no_argument, 0, 't'},
                                    {0, 0, 0, 0}};

    {
        int c;

        for (;;) {
            int option_index = 0;

            c = getopt_long(argc, argv, "vbhVd:m:t", long_options, &option_index);
            if (c == -1) break;

            switch (c) {
                case 0:
                    if (long_options[option_index].flag != 0) break;
                    switch (option_index) {
                        case DUMP_IMAGE:
                            m_binary_image_file = optarg ? std::string(optarg) : "";
                            break;
                        case MAP_FILE:
                            m_map_file = optarg ? std::string(optarg) : "";
                            break;
                    }
                    break;

                case 'v':
                    m_verbose = 1;
                    break;

                case 'b':
                    m_verbose = 0;
                    break;

                case 'V':
                    m_version = 1;
                    break;

                case 'h':
                    m_help = 1;
                    break;

                case 'd':
                    m_binary_image_file = optarg ? std::string(optarg) : "";
                    break;

                case 'm':
                    m_map_file = optarg ? std::string(optarg) : "";
                    break;

                case 't':
                    m_trim_padding = 1;
                    break;

                case '?':
                    break;

                default:
                    break;
            }
        }

        if (optind < argc) {
            m_executable_file = std::string(argv[optind++]);
        }
    }
}

bool Options::IsVerbose() { return m_verbose ? true : false; }

bool Options::IsHelp() { return m_help ? true : false; }

bool Options::IsVersion() { return m_version ? true : false; }

bool Options::IsTrimPadding() { return m_trim_padding ? true : false; }

std::string& Options::GetMapFile() { return m_map_file; }

std::string& Options::GetBinaryImageFile() { return m_binary_image_file; }

std::string& Options::GetExecutableFile() { return m_executable_file; }
