TEMPLATE = app
TARGET = parasol
INCLUDEPATH += .

# Warn if using deprecated features
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

SOURCES += parasol.cpp src/yidsrom.h src/yidsrom.cpp src/utils.h src/utils.cpp src/compression.h src/compression.cpp # src/lzss.c src/blz.c
QT += widgets core gui