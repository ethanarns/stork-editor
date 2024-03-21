#include "StateCommands.h"

void DeleteSpriteCommand::undo() {
    //YUtils::printDebug("Undo delete sprite");
    if (this->rom == nullptr) {
        YUtils::printDebug("ROM null, can't undo",DebugType::ERROR);
        return;
    }
    if (this->rom->mapData == nullptr) {
        YUtils::printDebug("MapData null, can't undo",DebugType::ERROR);
        return;
    }
    if (this->gridPtr == nullptr) {
        YUtils::printDebug("DisplayTable null, can't undo",DebugType::ERROR);
        return;
    }
    this->gridPtr->clearSelection();
    this->gridPtr->selectedObjects.clear();
    this->gridPtr->clearVisualSpriteSelection();
    this->rom->mapData->addSpriteData(loData,false);
    this->gridPtr->updateSprites();
}

void DeleteSpriteCommand::redo() {
    //YUtils::printDebug("(re)doing delete sprite");
    if (this->rom == nullptr) {
        YUtils::printDebug("ROM null, can't redo",DebugType::ERROR);
        return;
    }
    if (this->rom->mapData == nullptr) {
        YUtils::printDebug("MapData null, can't redo",DebugType::ERROR);
        return;
    }
    if (this->gridPtr == nullptr) {
        YUtils::printDebug("DisplayTable null, can't redo",DebugType::ERROR);
        return;
    }
    this->gridPtr->clearSelection();
    this->gridPtr->selectedObjects.clear();
    this->gridPtr->clearVisualSpriteSelection();
    this->rom->mapData->deleteSpriteByUUID(loData.uuid);
    this->gridPtr->wipeObject(loData.uuid);
    this->gridPtr->updateSprites();
}

void MoveSpriteCommand::undo() {
    //YUtils::printDebug("Undo move sprite");
    this->gridPtr->wipeObject(sprite.uuid);
    this->rom->moveObjectTo(sprite.uuid,this->beforePoint.x(),this->beforePoint.y());
    this->gridPtr->updateSprites();
    this->gridPtr->clearVisualSpriteSelection();
    this->gridPtr->selectedObjects.clear();
    this->gridPtr->clearSelection();
    this->gridPtr->selectItemByUuid(sprite.uuid);
}

void MoveSpriteCommand::redo() {
    //YUtils::printDebug("(Re)do move sprite");
    this->gridPtr->wipeObject(sprite.uuid);
    this->rom->moveObjectTo(sprite.uuid,this->afterPoint.x(),this->afterPoint.y());
    this->gridPtr->updateSprites();
    this->gridPtr->clearVisualSpriteSelection();
    this->gridPtr->selectedObjects.clear();
    this->gridPtr->clearSelection();
    this->gridPtr->selectItemByUuid(sprite.uuid);
}
