TEMPLATE = app
TARGET = parasol
INCLUDEPATH += .

# Warn if using deprecated features
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

SOURCES += parasol.cpp \
src/yidsrom.cpp src/utils.cpp src/compression.cpp src/MainWindow.cpp \
src/DisplayTable.cpp src/popups/ChartilesTable.cpp src/PixelDelegate.cpp \
src/popups/PaletteTable.cpp src/yidsrom_instructions.cpp src/yidsrom_metadata.cpp \
src/popups/LevelSelect.cpp src/LevelObject.cpp
HEADERS += src/Chartile.h \
src/yidsrom.h src/utils.h src/compression.h src/MainWindow.h \
src/DisplayTable.h src/popups/ChartilesTable.h src/PixelDelegate.h \
src/popups/PaletteTable.h src/LevelObject.h src/SettingsEnum.h \
src/popups/LevelSelect.h

QT += core gui widgets