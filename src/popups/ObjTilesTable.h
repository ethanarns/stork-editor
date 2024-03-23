#pragma once

#include "../yidsrom.h"
#include "../data/ObjectRenderFile.h"

#include <QtCore>
#include <QTableWidget>
#include <QByteArray>

#include <string>

class ObjTilesTable : public QTableWidget {
    Q_OBJECT
public:
    ObjTilesTable(QWidget *parent, YidsRom* rom);
    void tableClicked(int row, int column);
    void wipeTiles();
    // Triggers
    void doFileLoad(const QString text);
    void objbValueChanged(int i);
    void frameValueChanged(int i);
    void refreshWithCurrentData(bool guessTileCount = true);
    void widthChanged(int i);
    void heightChanged(int i);
    void paletteChanged(int i);
private:
    const static int OBJTILES_CELL_SIZE_PX = 32;
    const static int OBJTILES_ROW_COUNT_DEFAULT = 0x1;
    YidsRom* yidsRom;
    ObjectRenderArchive* currentObar;
    uint32_t objbIndex = 0;
    uint32_t frameIndex = 0;
    std::string currentFileName = "";
    QByteArray currentPalette;
    uint32_t getSpriteTilesWidth(uint32_t buildFlags);
};