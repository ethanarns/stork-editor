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
    this->musicIdDropdown = new QComboBox(this);
    this->musicIdDropdown->setObjectName("levelMusicIdDropdown");
    // Fill with music
    const int LATEST_MAX_MUSIC = 0x11; // TODO: more (beat the game first)
    // Less than or equal to because we want it to hit the max
    for (int musicId = 0; musicId <= LATEST_MAX_MUSIC; musicId++) {
        std::stringstream ssMusic;
        ssMusic << "0x" << std::hex << musicId;
        this->musicIdDropdown->addItem(tr(ssMusic.str().c_str()));
    }

    row1->addWidget(this->musicIdDropdown);
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
    this->entranceAnim = new QComboBox(this);
    // Fill with data, max known is 0x12
    for (int entranceAnimIndex = 0; entranceAnimIndex < 0x12; entranceAnimIndex++) {
        auto entranceAnimName = MapEntrance::printEntranceAnimation(static_cast<LevelSelectEnums::MapEntranceAnimation>(entranceAnimIndex));
        this->entranceAnim->addItem(tr(entranceAnimName.c_str()));
    }
    entranceOptions->addWidget(this->entranceAnim);
    // Entrance screen data
    auto entranceScreenLabel = new QLabel(tr("Entrance Screen Data"),this);
    entranceOptions->addWidget(entranceScreenLabel);
    this->entranceScreen = new QComboBox(this);
    entranceOptions->addWidget(this->entranceScreen);
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
    this->exitTypeCombo = new QComboBox(this);
    // Fix exit type with... exit types. Last known working one is 0xE
    for (int exitTypeIndex = 0; exitTypeIndex <= 0xE; exitTypeIndex++) {
        auto exitTypeName = MapExitData::printExitStartType(static_cast<LevelSelectEnums::MapExitStartType>(exitTypeIndex));
        this->exitTypeCombo->addItem(tr(exitTypeName.c_str()),exitTypeIndex);//itemData(n).toInt()
    }
    exitOptions->addWidget(this->exitTypeCombo);
    // Target map
    auto exitMapTargetLabel = new QLabel(tr("Target Map"),this);
    exitOptions->addWidget(exitMapTargetLabel);
    this->exitMapTarget = new QComboBox(this);
    exitOptions->addWidget(this->exitMapTarget);
    // Target entrance
    auto exitEntranceTargetLabel = new QLabel(tr("Target Entrance"), this);
    exitOptions->addWidget(exitEntranceTargetLabel);
    this->exitEntranceTarget = new QComboBox(this);
    exitOptions->addWidget(this->exitEntranceTarget);
    // Add to main layout
    row3->addLayout(exitOptions);
    mainLayout->addLayout(row3);

    // CONNECTIONS //
    connect(this->musicIdDropdown,&QComboBox::currentTextChanged,this,&LevelWindow::musicIdChanged);
    connect(this->entranceAnim,&QComboBox::currentTextChanged,this,&LevelWindow::entranceAnimChanged);
    connect(this->entranceScreen,&QComboBox::currentTextChanged,this,&LevelWindow::entranceScreenChanged);

    connect(this->exitTypeCombo,&QComboBox::currentTextChanged,this,&LevelWindow::exitTypeChanged);
    connect(this->exitMapTarget,&QComboBox::currentTextChanged,this,&LevelWindow::targetMapChanged);
    connect(this->exitEntranceTarget,&QComboBox::currentTextChanged,this,&LevelWindow::targetEntranceChanged);

    //currentItemChanged
    connect(this->entranceListWidget,&QListWidget::currentItemChanged,this,&LevelWindow::entranceItemChanged);
    connect(this->exitListWidget,&QListWidget::currentItemChanged,this,&LevelWindow::exitItemChanged);
}

void LevelWindow::refreshLists() {
    auto mapFilename = this->yidsRom->mapData->filename;
    if (mapFilename.empty()) {
        YUtils::printDebug("MPDZ filename was empty",DebugType::ERROR);
        return;
    }
    auto curLevelData = this->yidsRom->currentLevelSelectData->getLevelByMpdz(mapFilename);
    if (curLevelData == nullptr) {
        YUtils::printDebug("CRSB data with that MPDZ was null",DebugType::ERROR);
        return;
    }

    // Update music
    this->musicIdDropdown->setCurrentIndex(curLevelData->musicId);

    this->refreshEntranceList();

    // Wipe exit list widget
    this->exitListWidget->clearSelection();
    while (this->exitListWidget->count() > 0) {
        delete this->exitListWidget->takeItem(0);
    }

    uint exitIndex = 0;
    for (auto xit = curLevelData->exits.begin(); xit != curLevelData->exits.end(); xit++) {
        std::stringstream ssExit;
        uint32_t mapIndex = (*xit)->whichMapTo;
        auto levelTo = this->yidsRom->currentLevelSelectData->levels.at(mapIndex);
        auto exitType = MapExitData::printExitStartType((*xit)->exitStartType);
        ssExit << exitType << " to " << levelTo->mpdzFileNoExtension;
        ssExit << " 0x" << std::hex << (uint16_t)((*xit)->whichEntranceTo);
        QListWidgetItem* exitItem = new QListWidgetItem(tr(ssExit.str().c_str()));
        exitItem->setData(LevelWindowDataKey::EXIT_INDEX,exitIndex);
        this->exitListWidget->addItem(exitItem);
        exitIndex++;
    }

    // Populate with available maps
    this->exitMapTarget->clear(); // Will this cause a memory leak?
    auto mapList = this->yidsRom->currentLevelSelectData->levels;
    for (uint mapListIndex = 0; mapListIndex < mapList.size(); mapListIndex++) {
        this->exitMapTarget->addItem(mapList.at(mapListIndex)->mpdzFileNoExtension.c_str());
    }
    // Populate with entrances on that map
    auto currentSelectedExitMap = this->exitMapTarget->currentIndex();
    this->refreshTargetEntrances(currentSelectedExitMap);
}

void LevelWindow::musicIdChanged(const QString text) {
    Q_UNUSED(text);
    // std::cout << "musicIdChanged" << std::endl;
    // std::cout << this->musicIdDropdown->currentIndex() << std::endl;
    auto musicIdIndex = this->musicIdDropdown->currentIndex();
    if (musicIdIndex < 0) {
        // Will temporarily be -1 during changes
        return;
    }
    
    // Do updates for everything else //
    auto curLevel = this->yidsRom->currentLevelSelectData->getLevelByMpdz(this->yidsRom->mapData->filename);
    if (curLevel->musicId != musicIdIndex) {
        curLevel->musicId = static_cast<LevelSelectEnums::MapMusicId>(musicIdIndex);
        YUtils::printDebug("Modifying CRSB: Music ID",DebugType::VERBOSE);
    }
}

void LevelWindow::entranceAnimChanged(const QString text) {
    Q_UNUSED(text);
    // std::cout << "entranceAnimChanged" << std::endl;
    // std::cout << this->entranceAnim->currentIndex() << std::endl;
    auto entranceAnimIndex = this->entranceAnim->currentIndex();
    if (entranceAnimIndex < 0) {
        // Will temporarily be -1 during changes
        return;
    }

    // Do updates for everything else //
}

void LevelWindow::entranceScreenChanged(const QString text) {
    Q_UNUSED(text);
    // std::cout << "entranceScreenChanged" << std::endl;
    // std::cout << this->entranceScreen->currentIndex() << std::endl;
    auto entranceScreenComboIndex = this->entranceScreen->currentIndex();
    if (entranceScreenComboIndex < 0) {
        // Will temporarily be -1 during changes
        return;
    }

    // Do updates for everything else //
}

void LevelWindow::exitTypeChanged(const QString text) {
    Q_UNUSED(text);
    // std::cout << "exitTypeChanged" << std::endl;
    // std::cout << this->exitTypeCombo->currentIndex() << std::endl;
    auto exitTypeComboIndex = this->exitTypeCombo->currentIndex();
    if (exitTypeComboIndex < 0) {
        // Will temporarily be -1 during changes
        return;
    }

    // Do updates for everything else //
}

void LevelWindow::targetMapChanged(const QString text) {
    Q_UNUSED(text);
    //std::cout << "targetMapChanged" << std::endl;
    //std::cout << this->exitMapTarget->currentIndex() << std::endl;
    // ONLY update the available entrances
    auto currentSelectedExitMap = this->exitMapTarget->currentIndex();
    if (currentSelectedExitMap < 0) {
        // Will temporarily be -1 when updating
        //YUtils::printDebug("Invalid currentSelectedExitMap in targetMapChanged",DebugType::ERROR);
        return;
    }
    this->refreshTargetEntrances(currentSelectedExitMap);

    // Do updates for everything else //
}

void LevelWindow::targetEntranceChanged(const QString text) {
    Q_UNUSED(text);
    // std::cout << "targetEntranceChanged" << std::endl;
    // std::cout << this->exitEntranceTarget->currentIndex() << std::endl;
    if (this->exitEntranceTarget == nullptr) {
        YUtils::printDebug("exitEntranceTarget was null in targetEntranceChanged",DebugType::ERROR);
        YUtils::popupAlert("exitEntranceTarget was null in targetEntranceChanged");
        return;
    }
    auto currentSelectedExitMap = this->exitEntranceTarget->currentIndex();
    if (currentSelectedExitMap < 0) {
        // Will temporarily be -1
        return;
    }

    // Do updates for everything else //
}

void LevelWindow::refreshTargetEntrances(int currentSelectedExitMap) {
    auto entranceTargetList = this->yidsRom->currentLevelSelectData->levels.at(currentSelectedExitMap)->entrances;
    if (entranceTargetList.size() < 1) {
        YUtils::printDebug("entranceTargetList in targetMapChanged is empty",DebugType::WARNING);
        return;
    }
    this->exitEntranceTarget->clear();
    for (uint entranceTargetIndex = 0; entranceTargetIndex < entranceTargetList.size(); entranceTargetIndex++) {
        std::stringstream ssEntranceTarget;
        ssEntranceTarget << "0x" << std::hex << entranceTargetIndex << ": ";
        ssEntranceTarget << MapEntrance::printEntranceAnimation(entranceTargetList.at(entranceTargetIndex)->enterMapAnimation);
        this->exitEntranceTarget->addItem(ssEntranceTarget.str().c_str(),entranceTargetIndex);
    }
}

void LevelWindow::refreshEntranceList() {
    //std::cout << "refreshEntranceList" << std::endl;
    auto mapFilename = this->yidsRom->mapData->filename;
    if (mapFilename.empty()) {
        YUtils::printDebug("MPDZ filename was empty",DebugType::ERROR);
        return;
    }
    auto curLevelData = this->yidsRom->currentLevelSelectData->getLevelByMpdz(mapFilename);
    if (curLevelData == nullptr) {
        YUtils::printDebug("CRSB data with that MPDZ was null",DebugType::ERROR);
        return;
    }

    // Wipe entrance list widget
    this->entranceListWidget->clearSelection();
    while (this->entranceListWidget->count() > 0) {
        delete this->entranceListWidget->takeItem(0);
    }

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
}

void LevelWindow::entranceItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    Q_UNUSED(previous);

    if (current == nullptr) {
        // Will do this a lot while updating
        return;
    }

    auto mapFilename = this->yidsRom->mapData->filename;
    if (mapFilename.empty()) {
        YUtils::printDebug("MPDZ filename was empty",DebugType::ERROR);
        return;
    }
    auto curLevelData = this->yidsRom->currentLevelSelectData->getLevelByMpdz(mapFilename);
    if (curLevelData == nullptr) {
        YUtils::printDebug("CRSB data with that MPDZ was null",DebugType::ERROR);
        return;
    }

    auto entranceIndexQVariant = current->data(LevelWindowDataKey::ENTRANCE_INDEX);
    if (entranceIndexQVariant.isNull()) {
        YUtils::printDebug("No data in entrance item",DebugType::ERROR);
        return;
    }
    auto entranceIndex = entranceIndexQVariant.toUInt();
    //YUtils::printDebug("entranceItemChanged");
    auto entranceData = curLevelData->entrances.at(entranceIndex);
    // Animation update
    auto entranceAnimation = static_cast<int>(entranceData->enterMapAnimation);
    this->entranceAnim->setCurrentIndex(entranceAnimation);
    
    // TODO: update screen?
}

void LevelWindow::exitItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    Q_UNUSED(previous);

    if (current == nullptr) {
        // Will do this during updates
        return;
    }

    auto mapFilename = this->yidsRom->mapData->filename;
    if (mapFilename.empty()) {
        YUtils::printDebug("MPDZ filename was empty",DebugType::ERROR);
        return;
    }
    auto curLevelData = this->yidsRom->currentLevelSelectData->getLevelByMpdz(mapFilename);
    if (curLevelData == nullptr) {
        YUtils::printDebug("CRSB data with that MPDZ was null",DebugType::ERROR);
        return;
    }
    auto exitIndex = current->data(LevelWindowDataKey::EXIT_INDEX).toUInt();
    
    auto exitData = curLevelData->exits.at(exitIndex);
    this->exitTypeCombo->setCurrentIndex(exitData->exitStartType);
    this->exitMapTarget->setCurrentIndex(exitData->whichMapTo);
    this->refreshTargetEntrances(exitData->whichMapTo);
}
