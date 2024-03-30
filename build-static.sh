#!/bin/bash

make clean
PATH=/usr/local/Qt-6.6.3/bin:$PATH
export PATH
qmake -config release
make