# le_disasm

## Overview
libopcodes-based (AT&T syntax) linear executable (MZ/LE/LX DOS EXEs) disassembler modified from http://swars.vexillium.org/files/swdisasm-1.0.tar.bz2

## Requirements
- CMake 3.10 or later
- C++ compiler with C++11 support
- GNU Binutils development libraries (libopcodes and libbfd)

## Installing Dependencies

### Windows (MSYS2 MINGW64)

1. **Install MSYS2** from https://www.msys2.org/ (if not already installed)

2. **Open MSYS2 MINGW64 shell**:
   - From Start Menu: "MSYS2 MINGW64"
   - Or run: `C:\msys64\msys2_shell.cmd -mingw64`

3. **Verify environment** (should output `MINGW64`):
   ```bash
   echo $MSYSTEM
   ```

4. **Update package database**:
   ```bash
   pacman -Syu
   ```

5. **Install required packages**:
   ```bash
   pacman -S mingw-w64-x86_64-toolchain \
             mingw-w64-x86_64-cmake \
             mingw-w64-x86_64-ninja \
             mingw-w64-x86_64-binutils \
             mingw-w64-x86_64-zlib \
             mingw-w64-x86_64-zstd \
             mingw-w64-x86_64-gettext-runtime \
             make
   ```

### Ubuntu / Debian

```bash
sudo apt-get update
sudo apt-get install build-essential cmake ninja-build binutils-dev file
```

### Arch Linux

```bash
sudo pacman -S base-devel cmake ninja binutils
```

## Building

**Windows (MSYS2):**
```bash
# 64-bit (recommended for development)
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw-w64-x86_64.cmake \
      -DCMAKE_BUILD_TYPE=Debug -B Debug -S .
cmake --build Debug

# 64-bit (recommended for end users)
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw-w64-x86_64.cmake \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo -B RelWithDebInfo -S .
cmake --build RelWithDebInfo

# 32-bit
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw-w64-i686.cmake \
      -DCMAKE_BUILD_TYPE=Debug -B Debug -S .
cmake --build Debug
```

**Linux:**
```bash
# 64-bit (recommended for development)
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-linux-x86_64.cmake \
      -DCMAKE_BUILD_TYPE=Debug -B Debug -S .
cmake --build Debug

# 64-bit (recommended for end users)
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-linux-x86_64.cmake \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo -B RelWithDebInfo -S .
cmake --build RelWithDebInfo

# 32-bit
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-linux-i686.cmake \
      -DCMAKE_BUILD_TYPE=Debug -B Debug -S .
cmake --build Debug
```

### Build Output

Executables are placed in directories matching the build type:

- **RelWithDebInfo**: `RelWithDebInfo/le_disasm[.exe]`
- **Debug**: `Debug/le_disasm[.exe]`
- **Release**: `Release/le_disasm[.exe]`

## Usage

```bash
# Show help
./le_disasm --help

# Show version
./le_disasm --version

# Verbose output
./le_disasm --verbose executable.le

# Use input map file for symbols
./le_disasm --map-file=mapfile.map executable.le > output.S 2> stderr.txt

# Dump flat linear executable image
./le_disasm --dump-image=image.bin executable.le
```

## License

See [LICENSE](LICENSE) file for details.
