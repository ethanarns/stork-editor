#ifndef SELECTIONTABLE_H
#define SELECTIONTABLE_H

#include "yidsrom.h"
#include "LevelObject.h"
#include "StateCommands.h"

#include <vector>

#include <QtCore>
#include <QTableWidget>
#include <QTableWidgetItem>

class SelectionInfoTable : public QTableWidget {
    Q_OBJECT
public:
    SelectionInfoTable(QWidget* parent, YidsRom* rom);
    void setText(int x, int y, std::string text, bool editable = false);
    void updateWithLevelObject(LevelObject *lo);

    LevelObject *spritePointer;
private:
    YidsRom* yidsRom;
    void cellDoubleClicked(int row, int column);
    void cellChanged(int row, int column);
    QTableWidgetItem* cellBeingEdited;
    const int XPOSROW = 4;
    const int YPOSROW = 5;
    const int SETTINGSDATAROW = 7;
    std::vector<uint8_t> hexStringToByteVector(std::string hexString);
signals:
    void updateMainWindow(LevelObject *sprite);
    void pushCommandToUndoStack(QUndoCommand *cmd);
};

#endif