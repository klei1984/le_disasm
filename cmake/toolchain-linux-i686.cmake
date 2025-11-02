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

set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_COMPILER gcc -m32)
set(CMAKE_CXX_COMPILER g++ -m32)

set(CMAKE_SKIP_RPATH FALSE)

set(CONFIGURE_EXTRA_ARGS
	--build=i686-pc-linux-gnu
	CC=gcc
	CXX=g++
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
	-Wignored-qualifiers
	-Wshadow=local
	-Wtype-limits
	-Wlogical-op
	-fno-eliminate-unused-debug-types
	-fmessage-length=0
)
