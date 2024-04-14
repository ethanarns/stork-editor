TEMPLATE = app
TARGET = stork
INCLUDEPATH += .

# Warn if using deprecated features
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

RC_ICONS += icon.ico
ICON += icon.ico

SOURCES += stork.cpp \
src/yidsrom.cpp src/utils.cpp src/compression.cpp src/MainWindow.cpp \
src/DisplayTable.cpp src/popups/ChartilesTable.cpp src/PixelDelegate.cpp \
src/popups/PaletteTable.cpp src/yidsrom_instructions.cpp \
src/LevelObject.cpp src/FsPacker.cpp \
src/GuiObjectList.cpp src/SelectionInfoTable.cpp src/data/MapData.cpp src/data/MapData_subCons.cpp \
src/data/LevelSelectData.cpp src/cue_lzss.cpp src/cue_blz.cpp \
src/popups/ObjTilesTable.cpp src/data/ObjectRenderFile.cpp src/popups/BrushWindow.cpp \
src/popups/BrushTable.cpp src/popups/ColWindow.cpp src/popups/ColTable.cpp \
src/popups/MapSelect.cpp src/popups/SpritePickerWindow.cpp src/popups/LevelWindow.cpp \
src/GridOverlay.cpp src/StateCommands.cpp src/data/ProjectManager.cpp src/popups/TriggerWindow.cpp

HEADERS += src/Chartile.h \
src/yidsrom.h src/utils.h src/compression.h src/MainWindow.h \
src/DisplayTable.h src/popups/ChartilesTable.h src/PixelDelegate.h \
src/popups/PaletteTable.h src/LevelObject.h src/GlobalSettings.h \
src/Level.h src/FsPacker.h \
src/GuiObjectList.h src/SelectionInfoTable.h src/data/MapData.h \
src/data/LevelSelectData.h src/cue_lzss.h src/cue_blz.h \
src/popups/ObjTilesTable.h src/data/ObjectRenderFile.h src/popups/BrushWindow.h \
src/popups/BrushTable.h src/popups/ColWindow.h src/popups/ColTable.h src/PixelDelegateEnums.h \
src/popups/MapSelect.h src/popups/SpritePickerWindow.h src/popups/LevelWindow.h \
src/GridOverlay.h src/StateCommands.h src/data/ProjectManager.h src/popups/TriggerWindow.h

QT += core gui widgets

# version.majorRelease.minorRelease.hotfix
VERSION = 0.5.0.1

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

RESOURCES += \
    images.qrc
