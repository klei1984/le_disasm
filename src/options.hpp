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

#ifndef LE_DISASM_OPTIONS_HPP_
#define LE_DISASM_OPTIONS_HPP_

#include <string>

class Options {
public:
    Options(int argc, char** argv);

    bool IsVerbose();
    bool IsHelp();
    bool IsVersion();
    bool IsTrimPadding();
    std::string& GetMapFile();
    std::string& GetBinaryImageFile();
    std::string& GetExecutableFile();

private:
    enum { DUMP_IMAGE = 4, MAP_FILE = 5 };

    int m_verbose;
    int m_version;
    int m_help;
    int m_trim_padding;
    std::string m_binary_image_file;
    std::string m_map_file;
    std::string m_executable_file;
};

#endif
