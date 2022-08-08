#!/bin/bash

# Delete prior executables
rm parasol
rm blz
rm lzss

# Do separate executables
gcc src/blz.c -o blz
gcc src/lzss.c -o lzss

# Do make
qmake
make

# Execute
./parasol

rm Makefile