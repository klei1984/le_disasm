#!/usr/bin/python3
# vim:sw=4
#
# Copyright (C) 2019-2025  klei1984 <53688147+klei1984@users.noreply.github.com>
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

from getopt import gnu_getopt, GetoptError
import re
import sys
from os import listdir, path


class SignalProcessor:
    def __init__(self):
        self.output_filename = ""
        self.input_source_file = ""
        self.input_signal_file = ""

    @staticmethod
    def print_help():
        print(
            """Usage: %s OPTIONS SOURCE_FILE SIGNATURE

    SOURCE_FILE:        path to disassembly source file (*.S)
    SIGNATURE:          path to signature file created by omfLibraryToSignatureList.py (*.sig)
    OPTIONS:
      -h  --help		shows this help text
      -o FILE		    outputs signature matches to FILE instead of stdout"""
            % (sys.argv[0]))

    def process_source_file(self):
        text = ""
        with open(self.input_source_file, "r", encoding="iso-8859-1") as f:
            text = f.read()

        with open(self.input_signal_file, "r", encoding="iso-8859-1") as f:
            for line in f:
                if line[0] == "\t":
                    (name, pattern) = line[1:].split("\t")
                    print(".", file=sys.stderr, end="", flush=True)
                    if pattern.find("??") != -1:
                        print("\nSignature for %s contains unknown opcodes" % name, file=sys.stderr, flush=True)
                        continue
                    m = re.search(pattern, text)
                    if m:
                        mm = re.match(r"_([0-9a-fA-F]+)_func:", m.group(0))
                        if mm and len(mm.groups()) == 1:
                            print("\nFound %s at %s" % (name, mm.group(1)), file=sys.stderr)
                            print(name, mm.group(1))

    def main(self):
        try:
            opts, args = gnu_getopt(sys.argv[1:], "h:o:",
                                    ("help", "output-file"))

        except GetoptError as message:
            print('Error: ', message, file=sys.stderr)
            sys.exit(1)

        for opt, arg in opts:
            if opt in ('-h', '--help'):
                self.print_help()
                sys.exit(0)
            elif opt in ('-o',):
                self.output_filename = arg

        if len(args) == 2:
            self.input_source_file = args[0]
            self.input_signal_file = args[1]
        elif len(args) > 2:
            print('Error: Too many arguments', file=sys.stderr)
            sys.exit(1)
        else:
            self.print_help()
            sys.exit(1)

        self.process_source_file()


sig = SignalProcessor()
sig.main()
