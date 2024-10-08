#!/bin/bash

# https://doc.qt.io/qt-6/linux-deployment.html
./clean.sh
/usr/local/Qt-6.7.3/bin/qmake6 -config release
make
# sanity check
./stork --version
# create AppImage structure
mkdir -p AppDir/usr/bin
cp stork AppDir/usr/bin
echo "AppImage structure generated"
ARCH=x86_64 appimagetool AppDir StorkEditor.AppImage
chmod +x StorkEditor.AppImage
# sanity check again
./StorkEditor.AppImage --version

zip StorkEditor.X.Y.Z-linux.zip StorkEditor.AppImage lib/* sample_brushes/*/* UserGuide.md