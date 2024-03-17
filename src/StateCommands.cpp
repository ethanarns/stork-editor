#include "StateCommands.h"

void DeleteSpriteCommand::undo() {
    YUtils::printDebug("Undo delete sprite");
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
    YUtils::printDebug("(re)doing delete sprite");
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