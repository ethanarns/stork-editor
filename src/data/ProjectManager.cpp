#include "ProjectManager.h"

#include "../utils.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QString>
#include <QFile>

ProjectManager::ProjectManager(YidsRom *rom) {
    this->yidsRom = rom;
}

QJsonObject ProjectManager::generateNewJson(std::string projectName) {
    QJsonObject result;
    result["uuid"] = QString::fromStdString(YUtils::generateUuid());
    result["projectName"] = QString::fromStdString(projectName);
    result["folderName"] = QString::fromStdString(projectName);
    return result;
}

bool ProjectManager::saveJson(QJsonObject jsonData, std::string fullFileName) {
    if (fullFileName.empty()) {
        YUtils::printDebug("Cannot save, filename empty",DebugType::ERROR);
        return false;
    }
    QFile saveFile(QString::fromStdString(fullFileName));
    if (!saveFile.open(QIODevice::WriteOnly)) {
        YUtils::printDebug("Could not open save file",DebugType::ERROR);
        return false;
    }
    saveFile.write(QJsonDocument(jsonData).toJson());
    return true;
}
