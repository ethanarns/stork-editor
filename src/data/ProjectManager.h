#pragma once

#include <string>

#include "../yidsrom.h"

class ProjectManager {
public:
    ProjectManager(YidsRom* rom);
    std::string latestProjectName = "";
    QJsonObject generateNewJson();
private:
    YidsRom* yidsRom;
};