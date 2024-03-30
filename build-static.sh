#!/bin/bash

# https://doc.qt.io/qt-6/linux-deployment.html
make clean
/usr/local/Qt-6.6.3/bin/qmake -config release
make