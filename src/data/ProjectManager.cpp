#include "ProjectManager.h"

#include <QJsonObject>

ProjectManager::ProjectManager(YidsRom *rom) {
    this->yidsRom = rom;
}

QJsonObject ProjectManager::generateNewJson() {
    QJsonObject result;
    return result;
}
