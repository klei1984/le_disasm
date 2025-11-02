# Copyright (C) 2025  klei1984 <53688147+klei1984@users.noreply.github.com>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Collect all header files
file(GLOB LOCAL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

# List all source files
set(LOCAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/analyzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dis_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/emitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fixup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flags_restorer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/header.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/insn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/linear_executable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/object_header.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/object_page_header.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/options.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/print.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/region.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/regions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/symbol_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/symbol_map_properties.cpp
)

# Export sources and headers to parent scope
set(PROJECT_SOURCES
    ${PROJECT_SOURCES}
    ${LOCAL_SOURCES}
    ${LOCAL_HEADERS}
    PARENT_SCOPE
)

# Export include directories to parent scope
set(PROJECT_INCLUDES
    ${PROJECT_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
)
