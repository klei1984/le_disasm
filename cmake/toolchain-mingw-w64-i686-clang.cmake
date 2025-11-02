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

set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

# Detect MINGW prefix from environment or use default
if(DEFINED ENV{MINGW_PREFIX})
    set(PREFIX $ENV{MINGW_PREFIX})
else()
    set(PREFIX c:/msys64/mingw32)
endif()

set(TOOLSET "i686-w64-mingw32")

set(CMAKE_FIND_ROOT_PATH ${PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

find_program(CMAKE_RC_COMPILER NAMES llvm-windres REQUIRED)
find_program(CMAKE_C_COMPILER NAMES clang REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES clang++ REQUIRED)

set(CONFIGURE_EXTRA_ARGS
	--target=${TOOLSET}
	--host=${TOOLSET}
	--build=${TOOLSET}
	CC=${CMAKE_C_COMPILER}
	CXX=${CMAKE_CXX_COMPILER}
	CFLAGS=-m32
	CXXFLAGS=-m32
	LDFLAGS=-m32
)

add_compile_options(
	-m32
	$<$<COMPILE_LANGUAGE:CXX>:-Wno-reorder>
	-Wall
	-Wno-switch
	-Wno-unused-function
	-Wno-unused-variable
	-Wno-unused-parameter
	-fmessage-length=0
)
