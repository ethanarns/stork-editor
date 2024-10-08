#!/bin/bash
make clean
rm Makefile
rm stork
rm *.o
rm .qmake.stash
rm moc_*
rm bin9_decomp_temp.bin
rm -r yromfs
rm rom_uncomp_temp.nds
rm stork_plugin_import.cpp
rm AppDir/usr/bin/stork
rm clean
rm *.AppImage
rm -rf pytools/mespack_files
# These are auto-generated
rm AppDir/stork-editor.desktop
rm AppDir/AppRun
rm AppDir/.DirIcon
rm -rf AppDir/etc
rm -rf AppDir/usr/lib32
rm -rf AppDir/usr/lib64
rm -rf AppDir/usr/plugins
rm -rf AppDir/usr/translations
rm -rf AppDir/lib64
rm -rf AppDir/usr/bin
rm -rf AppDir/usr/lib
rm qrc_images.cpp
rm stork.pro.user