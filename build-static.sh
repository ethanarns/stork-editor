#!/bin/bash

# https://doc.qt.io/qt-6/linux-deployment.html
./clean.sh
/usr/local/Qt-6.6.3-static/bin/qmake -config release
make
# sanity check
./stork --version
# create AppImage structure
mkdir -p AppDir/usr/bin
cp stork AppDir/usr/bin
echo "AppImage structure generated"
~/Programs/appimagetool-833-x86_64.AppImage -s deploy AppDir/usr/share/applications/com.ethanarns.storkeditor.desktop
# ARCH=x86_64 ~/Programs/appimagetool-x86_64.AppImage AppDir StorkEditor.AppImage
VERSION=1.0 ~/Programs/appimagetool-833-x86_64.AppImage ./AppDir # turn AppDir into AppImage
chmod +x Stork_Editor-1.0-x86_64.AppImage
# sanity check again
Stork_Editor-1.0-x86_64.AppImage --version

zip StorkEditor.X.Y.Z-linux.zip Stork_Editor-1.0-x86_64.AppImage lib/* sample_brushes/*/* UserGuide.md