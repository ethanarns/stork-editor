#!/bin/bash

# Do separate executables
gcc lib/lzss.c -o lzss

# Do make
qmake
make

# Execute
./parasol