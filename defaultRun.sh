#!/bin/bash

# Delete prior executable
rm parasol

# Delete prior Makefile
rm Makefile

# Do separate executables
gcc src/blz.c -o blz
gcc src/lzss.c -o lzss

# Do make
qmake
make

# Remove crap
rm *.o
rm .qmake.stash

# Execute
./parasol