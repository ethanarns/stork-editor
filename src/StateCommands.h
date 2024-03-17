#pragma once

#include "LevelObject.h"
#include "DisplayTable.h"
#include "data/MapData.h"
#include "yidsrom.h"

#include <QString>
#include <QUndoCommand>
#include <sstream>

class DeleteSpriteCommand : public QUndoCommand {
public:
    DeleteSpriteCommand(LevelObject lo, DisplayTable* grid, YidsRom* yidsrom, QUndoCommand* parent = nullptr) :
        QUndoCommand(parent), loData(lo), rom(yidsrom), gridPtr(grid)
    {
        std::stringstream ss;
        auto spriteMeta = this->rom->getSpriteMetadata(lo.objectId);
        ss << "delete " << std::hex << spriteMeta.name;
        this->setText(QString(ss.str().c_str()));
    }
    void undo() override;
    void redo() override;
private:
    LevelObject loData;
    DisplayTable* gridPtr;
    YidsRom* rom;
};