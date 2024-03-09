#include "LevelWindow.h"

#include "../yidsrom.h"

#include <string>
#include <sstream>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QListWidget>
#include <QLabel>

enum LevelWindowDataKey {
    ENTRANCE_INDEX = 0x20,
    EXIT_INDEX = 0x21
};

LevelWindow::LevelWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
    this->setWindowTitle(tr("Level Window"));
    this->setObjectName(tr("levelWindow"));
    auto mainLayout = new QVBoxLayout(this);
    // Bar 1: Music ID //
    auto row1 = new QHBoxLayout(this);
    auto musicIdLabel = new QLabel(tr("Music ID"),this);
    row1->addWidget(musicIdLabel);
    auto musicIdDropdown = new QComboBox(this);
    musicIdDropdown->setObjectName("levelMusicIdDropdown");
    row1->addWidget(musicIdDropdown);
    mainLayout->addLayout(row1);
    // Bar 2: Level Entrances //
    auto row2 = new QHBoxLayout(this);
    // Column 1: list
    this->entranceListWidget = new QListWidget(this);
    row2->addWidget(this->entranceListWidget);
    // Column 2: Options
    auto entranceOptions = new QVBoxLayout(this);
    // Entrance Animation
    auto entranceAnimLabel = new QLabel(tr("Entrance Animation"),this);
    entranceOptions->addWidget(entranceAnimLabel);
    auto entranceAnim = new QComboBox(this);
    entranceOptions->addWidget(entranceAnim);
    // Entrance screen data
    auto entranceScreenLabel = new QLabel(tr("Entrance Screen Data"),this);
    entranceOptions->addWidget(entranceScreenLabel);
    auto entranceScreen = new QComboBox(this);
    entranceOptions->addWidget(entranceScreen);
    row2->addLayout(entranceOptions);
    mainLayout->addLayout(row2);
    // Bar 3: Level Exits //
    auto row3 = new QHBoxLayout(this);
    // Column 1: Exit list
    this->exitListWidget = new QListWidget(this);
    row3->addWidget(this->exitListWidget);
    // Column 2: Exit options
    auto exitOptions = new QVBoxLayout(this);
    // Exit Type
    auto exitTypeLabel = new QLabel(tr("Exit Type"),this);
    exitOptions->addWidget(exitTypeLabel);
    auto exitTypeCombo = new QComboBox(this);
    exitOptions->addWidget(exitTypeCombo);
    // Target map
    auto exitMapTargetLabel = new QLabel(tr("Target Map"),this);
    exitOptions->addWidget(exitMapTargetLabel);
    auto exitMapTarget = new QComboBox(this);
    exitOptions->addWidget(exitMapTarget);
    // Target entrance
    auto exitEntranceTargetLabel = new QLabel(tr("Target Entrance"), this);
    exitOptions->addWidget(exitEntranceTargetLabel);
    auto exitEntranceTarget = new QComboBox(this);
    exitOptions->addWidget(exitEntranceTarget);
    // Add to main layout
    row3->addLayout(exitOptions);
    mainLayout->addLayout(row3);
}

void LevelWindow::refreshLists() {
    auto mapFilename = this->yidsRom->mapData->filename;
    if (mapFilename.empty()) {
        YUtils::printDebug("MPDZ filename was empty",DebugType::ERROR);
        return;
    }
    auto curLevelData = this->yidsRom->latestLevelSelectData->getLevelByMpdz(mapFilename);
    if (curLevelData == nullptr) {
        YUtils::printDebug("CRSB data with that MPDZ was null",DebugType::ERROR);
        return;
    }

    // Wipe entrance list widget
    this->entranceListWidget->clearSelection();
    for (int entranceDelIndex = 0; entranceDelIndex < this->entranceListWidget->count(); entranceDelIndex++) {
        delete this->entranceListWidget->item(entranceDelIndex);
    }
    this->entranceListWidget->clear();

    // Is this the first map?
    bool isFirstMap = false; // TODO: find out

    uint entranceIndex = 0;
    for (auto enit = curLevelData->entrances.begin(); enit != curLevelData->entrances.end(); enit++) {
        std::stringstream ssEnter;
        if (entranceIndex == 0 && isFirstMap == true) {
            ssEnter << "0x0: Level Start"; // Different settings than normal
        } else {
            auto entranceTypeStr = MapEntrance::printEntranceAnimation((*enit)->enterMapAnimation);
            ssEnter << "0x" << std::hex << entranceIndex << ": " << entranceTypeStr;
        }
        QListWidgetItem* entranceItem = new QListWidgetItem(tr(ssEnter.str().c_str()));
        entranceItem->setData(LevelWindowDataKey::ENTRANCE_INDEX,entranceIndex);
        this->entranceListWidget->addItem(entranceItem);
        entranceIndex++;
    }

    // Wipe exit list widget
    this->exitListWidget->clearSelection();
    for (int exitDelIndex = 0; exitDelIndex < this->exitListWidget->count(); exitDelIndex++) {
        delete this->exitListWidget->item(exitDelIndex);
    }
    this->exitListWidget->clear();

    uint exitIndex = 0;
    for (auto xit = curLevelData->exits.begin(); xit != curLevelData->exits.end(); xit++) {
        std::stringstream ssExit;
        uint32_t mapIndex = (*xit)->whichMapTo;
        auto levelTo = this->yidsRom->latestLevelSelectData->levels.at(mapIndex);
        auto exitType = MapExitData::printExitStartType((*xit)->exitStartType);
        ssExit << exitType << " to " << levelTo->mpdzFileNoExtension;
        ssExit << " 0x" << std::hex << (uint16_t)((*xit)->whichEntranceTo);
        QListWidgetItem* exitItem = new QListWidgetItem(tr(ssExit.str().c_str()));
        exitItem->setData(LevelWindowDataKey::EXIT_INDEX,exitIndex);
        this->exitListWidget->addItem(exitItem);
        exitIndex++;
    }
}
