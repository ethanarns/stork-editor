#pragma once

#include <string>

#include "../yidsrom.h"

class ProjectManager {
public:
    ProjectManager(YidsRom* rom);
    std::string latestProjectName = "";
    QJsonObject generateNewJson(std::string projectName);
    bool saveJson(QJsonObject jsonData, std::string fullFileName);
private:
    YidsRom* yidsRom;
};