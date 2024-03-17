#pragma once

#include "LevelObject.h"
#include "DisplayTable.h"
#include "data/MapData.h"

#include <QString>
#include <QUndoCommand>

class DeleteSpriteCommand : public QUndoCommand {
public:
    DeleteSpriteCommand(LevelObject lo, DisplayTable* grid, MapData* mpdz, QUndoCommand* parent = nullptr) :
        QUndoCommand(parent), loData(lo), mpdzPtr(mpdz), gridPtr(grid)
    {
        this->setText(QString("delete sprite"));
    }
    void undo() override {
        YUtils::printDebug("Undo delete sprite");
        this->mpdzPtr->addSpriteData(loData,false);
        this->gridPtr->updateSprites();
    }
    void redo() override;
private:
    LevelObject loData;
    DisplayTable* gridPtr;
    MapData* mpdzPtr;
};