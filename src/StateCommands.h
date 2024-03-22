#pragma once

#include "LevelObject.h"
#include "DisplayTable.h"
#include "data/MapData.h"
#include "yidsrom.h"

#include <QString>
#include <QUndoCommand>
#include <sstream>
#include <vector>

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

class SpriteSettingsChangeCommand : public QUndoCommand {
public:
    SpriteSettingsChangeCommand(uint32_t loUuid, std::vector<uint8_t> newSettings, YidsRom* yidsrom, QUndoCommand* parent = nullptr) :
        QUndoCommand(parent), uuid(loUuid), newSets(newSettings), rom(yidsrom)
    {
        std::stringstream ss;
        auto loPtr = this->rom->mapData->getLevelObjectByUuid(uuid);
        if (loPtr == nullptr) {
            ss << "Sprite with UUID 0x" << std::hex << uuid;
            ss << " not found when changing settings";
            YUtils::printDebug(ss.str(),DebugType::ERROR);
            return;
        }
        auto spriteMeta = this->rom->getSpriteMetadata(loPtr->objectId);
        ss << "modify " << std::hex << spriteMeta.name;
        this->setText(QString(ss.str().c_str()));
        this->oldSets = loPtr->settings;
    }
    void undo() override;
    void redo() override;
private:
    uint32_t uuid;
    std::vector<uint8_t> newSets;
    std::vector<uint8_t> oldSets;
    YidsRom* rom;
};