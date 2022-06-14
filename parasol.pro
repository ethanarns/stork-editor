TEMPLATE = app
TARGET = parasol
INCLUDEPATH += .

# Warn if using deprecated features
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

SOURCES += parasol.cpp src/yidsrom.cpp src/utils.cpp src/compression.cpp src/MainWindow.cpp src/DisplayTable.cpp src/ChartilesTable.cpp
HEADERS += src/yidsrom.h src/utils.h src/compression.h src/MainWindow.h src/DisplayTable.h src/ChartilesTable.h
QT += widgets core gui