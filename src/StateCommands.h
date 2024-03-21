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
    YidsRom* rom;
    DisplayTable* gridPtr;
};

class MoveSpriteCommand : public QUndoCommand {
public:
    MoveSpriteCommand(LevelObject lo, uint32_t xTo, uint32_t yTo, DisplayTable* grid, YidsRom* yidsrom, QUndoCommand* parent = nullptr) :
        QUndoCommand(parent), sprite(lo), rom(yidsrom), gridPtr(grid)
    {
        // This sets up the data stored in the command, but does not do the move itself
        this->beforePoint = QPoint((int)lo.xPosition,(int)lo.yPosition);
        this->afterPoint = QPoint((int)xTo,(int)yTo);
        std::stringstream ss;
        auto spriteMeta = this->rom->getSpriteMetadata(lo.objectId);
        ss << "move " << std::hex << spriteMeta.name;
        this->setText(QString(ss.str().c_str()));
    }
    void undo() override;
    void redo() override;
private:
    LevelObject sprite;
    YidsRom* rom;
    DisplayTable* gridPtr;
    QPoint beforePoint;
    QPoint afterPoint;
};

class AddSpriteCommand : public QUndoCommand {
public:
    AddSpriteCommand(LevelObject lo, DisplayTable* grid, YidsRom* yidsrom, QUndoCommand* parent = nullptr) :
        QUndoCommand(parent), loData(lo), rom(yidsrom), gridPtr(grid)
    {
        std::stringstream ss;
        auto spriteMeta = this->rom->getSpriteMetadata(lo.objectId);
        ss << "create " << std::hex << spriteMeta.name;
        this->setText(QString(ss.str().c_str()));
    }
    void undo() override;
    void redo() override;
private:
    LevelObject loData;
    YidsRom* rom;
    DisplayTable* gridPtr;
};