#!/bin/bash

# https://doc.qt.io/qt-6/linux-deployment.html
rm stork
make clean
/usr/local/Qt-6.6.3/bin/qmake -config release
make
# sanity check
./stork --version
# create AppImage structure
mkdir -p AppDir/usr/bin
cp stork AppDir/usr/bin