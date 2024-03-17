#include "StateCommands.h"

void DeleteSpriteCommand::redo() {
    YUtils::printDebug("(re)doing delete sprite");
    this->mpdzPtr->deleteSpriteByUUID(loData.uuid);
    this->gridPtr->wipeObject(loData.uuid);
    this->gridPtr->updateSprites();
}