TEMPLATE = app
TARGET = parasol
INCLUDEPATH += .

# Warn if using deprecated features
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

RC_ICONS += icon.ico
ICON += icon.ico

SOURCES += parasol.cpp \
src/yidsrom.cpp src/utils.cpp src/compression.cpp src/MainWindow.cpp \
src/DisplayTable.cpp src/popups/ChartilesTable.cpp src/PixelDelegate.cpp \
src/popups/PaletteTable.cpp src/yidsrom_instructions.cpp \
src/popups/LevelSelect.cpp src/LevelObject.cpp src/FsPacker.cpp \
src/GuiObjectList.cpp src/SelectionInfoTable.cpp src/data/MapData.cpp src/data/MapData_subCons.cpp \
src/data/LevelSelectData.cpp
HEADERS += src/Chartile.h \
src/yidsrom.h src/utils.h src/compression.h src/MainWindow.h \
src/DisplayTable.h src/popups/ChartilesTable.h src/PixelDelegate.h \
src/popups/PaletteTable.h src/LevelObject.h src/SettingsEnum.h \
src/popups/LevelSelect.h src/Level.h src/FsPacker.h src/InstructionRenderer.h \
src/GuiObjectList.h src/SelectionInfoTable.h src/data/MapData.h \
src/data/LevelSelectData.h

QT += core gui widgets

DISTFILES += \
    lib/blz.py \
    lib/lzss.py \
    lib/ndstool \
    lib/ndstool.exe
