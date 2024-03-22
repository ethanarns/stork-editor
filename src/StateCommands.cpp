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

void AddSpriteCommand::undo() {
    //YUtils::printDebug("Undo create sprite");
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
    // Clear selections
    this->gridPtr->clearSelection();
    this->gridPtr->selectedObjects.clear();
    this->gridPtr->clearVisualSpriteSelection();
    // Delete the sprite
    this->rom->mapData->deleteSpriteByUUID(loData.uuid);
    this->gridPtr->wipeObject(loData.uuid);
    this->gridPtr->updateSprites();
}

void AddSpriteCommand::redo() {
    //YUtils::printDebug("(Re)do create sprite");
    if (this->rom == nullptr) {
        YUtils::printDebug("ROM null, can't re(do)",DebugType::ERROR);
        return;
    }
    if (this->rom->mapData == nullptr) {
        YUtils::printDebug("MapData null, can't re(do)",DebugType::ERROR);
        return;
    }
    if (this->gridPtr == nullptr) {
        YUtils::printDebug("DisplayTable null, can't re(do)",DebugType::ERROR);
        return;
    }
    this->gridPtr->clearSelection();
    this->gridPtr->selectedObjects.clear();
    this->gridPtr->clearVisualSpriteSelection();
    // Make sure to create a new UUID
    auto newSpritePtr = this->rom->mapData->addSpriteData(this->loData,true);
    this->gridPtr->updateSprites();
    this->gridPtr->selectItemByUuid(newSpritePtr->uuid);
    this->loData = *newSpritePtr;
}

void SpriteSettingsChangeCommand::undo() {
    auto loPtr = this->rom->mapData->getLevelObjectByUuid(this->uuid);
    if (loPtr == nullptr) {
        YUtils::printDebug("Couldn't undo sprite settings modification, uuid not found",DebugType::ERROR);
        return;
    }
    if (loPtr->settings.size() != this->oldSets.size()) {
        YUtils::printDebug("Mismatch in changed settings size",DebugType::WARNING);
    }
    loPtr->settings = this->oldSets;
    // Update grid later
}

void SpriteSettingsChangeCommand::redo() {
    auto loPtr = this->rom->mapData->getLevelObjectByUuid(this->uuid);
    if (loPtr == nullptr) {
        YUtils::printDebug("Couldn't (re)do sprite settings modification, uuid not found",DebugType::ERROR);
        return;
    }
    if (loPtr->settings.size() != this->newSets.size()) {
        YUtils::printDebug("Mismatch in changed settings size",DebugType::WARNING);
    }
    loPtr->settings = this->newSets;
    // Update grid later
}
