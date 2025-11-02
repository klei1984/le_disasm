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

cmake_minimum_required(VERSION 3.10)

project(le_disasm
    VERSION 1.0.0
    DESCRIPTION "libopcodes-based linear executable (MZ/LE/LX DOS EXEs) disassembler"
    LANGUAGES CXX
)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Find required libraries
find_library(OPCODES_LIBRARY opcodes REQUIRED)
find_library(BFD_LIBRARY bfd REQUIRED)

# Find additional dependencies needed by binutils on MSYS2/MINGW64/Cygwin
# These are often required when linking against static binutils libraries
find_library(INTL_LIBRARY intl)
find_library(ICONV_LIBRARY iconv)
find_library(Z_LIBRARY z)
find_library(SFRAME_LIBRARY sframe)
find_library(IBERTY_LIBRARY iberty)
find_library(LIBERTY_LIBRARY liberty)

# Additional libraries that might be needed
set(ADDITIONAL_LIBS)

# Always try to link libintl on Windows/Cygwin/MSYS2 systems
if(INTL_LIBRARY)
    list(APPEND ADDITIONAL_LIBS ${INTL_LIBRARY})
    message(STATUS "Found libintl: ${INTL_LIBRARY}")
else()
    # Try to find gettext/intl with different names
    find_library(GETTEXT_LIBRARY gettext)
    if(GETTEXT_LIBRARY)
        list(APPEND ADDITIONAL_LIBS ${GETTEXT_LIBRARY})
        message(STATUS "Found gettext library: ${GETTEXT_LIBRARY}")
    endif()
endif()

if(ICONV_LIBRARY)
    list(APPEND ADDITIONAL_LIBS ${ICONV_LIBRARY})
    message(STATUS "Found libiconv: ${ICONV_LIBRARY}")
endif()

if(Z_LIBRARY)
    list(APPEND ADDITIONAL_LIBS ${Z_LIBRARY})
    message(STATUS "Found libz: ${Z_LIBRARY}")
endif()

if(SFRAME_LIBRARY)
    list(APPEND ADDITIONAL_LIBS ${SFRAME_LIBRARY})
    message(STATUS "Found libsframe: ${SFRAME_LIBRARY}")
endif()

if(IBERTY_LIBRARY)
    list(APPEND ADDITIONAL_LIBS ${IBERTY_LIBRARY})
    message(STATUS "Found libiberty: ${IBERTY_LIBRARY}")
elseif(LIBERTY_LIBRARY)
    list(APPEND ADDITIONAL_LIBS ${LIBERTY_LIBRARY})
    message(STATUS "Found liberty: ${LIBERTY_LIBRARY}")
endif()

# On MSYS2/MINGW64 Windows builds, add required system libraries
if(WIN32 OR MINGW OR MSYS)
    list(APPEND ADDITIONAL_LIBS ws2_32)  # Winsock library
    
    # Add zstd library (required by libbfd for compression support)
    list(APPEND ADDITIONAL_LIBS zstd)
    message(STATUS "Adding zstd library for libbfd compression support")
    
    message(STATUS "Adding Windows system libraries for MSYS2/MINGW64")
endif()

# Check for binutils-dev headers
find_path(BFD_INCLUDE_DIR bfd.h
    HINTS /usr/include /usr/local/include
    PATH_SUFFIXES bfd
)

if(BFD_INCLUDE_DIR)
    message(STATUS "Found BFD headers at: ${BFD_INCLUDE_DIR}")
else()
    message(WARNING "BFD headers not found. Make sure binutils-dev package is installed.")
endif()

# Add source directory
add_subdirectory(src)

# Create executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_INCLUDES}
)

if(BFD_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${BFD_INCLUDE_DIR})
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OPCODES_LIBRARY}
    ${BFD_LIBRARY}
    ${ADDITIONAL_LIBS}    # Additional libraries needed by binutils
    ${CMAKE_DL_LIBS}      # For -rdynamic functionality
)

# Add -rdynamic for dynamic symbol resolution (Linux/Unix only)
# MinGW and other Windows toolchains don't support this flag
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND UNIX AND NOT WIN32 AND NOT MINGW AND NOT MSYS)
    target_link_options(${PROJECT_NAME} PRIVATE -rdynamic)
endif()

# Define PACKAGE macro (required by some binutils headers)
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE)

# Platform-specific configuration
if(WIN32 OR MINGW OR MSYS)
    # Windows-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE WINDOWS_BUILD)
    message(STATUS "Configuring for Windows build")
elseif(UNIX)
    # Linux/Unix-specific settings - stacktrace functionality available
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNIX_BUILD)
    message(STATUS "Configuring for Unix/Linux build")
endif()

# Print build configuration info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

# Installation (optional)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# CPack configuration for packaging (optional)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "maintainer@example.com")
include(CPack)