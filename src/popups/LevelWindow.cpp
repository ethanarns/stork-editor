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
    ENTRANCE_INDEX_WINDOW = 0x20,
    EXIT_INDEX_WINDOW = 0x21
};

LevelWindow::LevelWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
    this->detectChanges = false;
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
    for (int entranceScreenIndex = 0; entranceScreenIndex <= 3; entranceScreenIndex++) {
        auto entranceScreenName = MapEntrance::printScreenData(static_cast<LevelSelectEnums::StartingDsScreen>(entranceScreenIndex));
        this->entranceScreen->addItem(tr(entranceScreenName.c_str()));
    }

    auto entranceCoordsLabel = new QLabel(tr("Entrance Coordinates"),this);
    entranceOptions->addWidget(entranceCoordsLabel);
    auto entranceCoords = new QHBoxLayout(this);
    this->entranceX = new QSpinBox(this);
    this->entranceX->setDisplayIntegerBase(16);
    this->entranceX->setToolTip(tr("X Position"));
    this->entranceX->setMaximum(0xffff);
    // connect(chartilePaletteSelect,QOverload<int>::of(&QSpinBox::valueChanged),this->chartilesTable,&ChartilesTable::paletteValueChanged);
    connect(this->entranceX,QOverload<int>::of(&QSpinBox::valueChanged),this,&LevelWindow::entrancePositionChangedX);
    entranceCoords->addWidget(this->entranceX);
    this->entranceY = new QSpinBox(this);
    this->entranceY->setDisplayIntegerBase(16);
    this->entranceY->setToolTip(tr("Y Position"));
    this->entranceY->setMaximum(0xffff);
    connect(this->entranceY,QOverload<int>::of(&QSpinBox::valueChanged),this,&LevelWindow::entrancePositionChangedY);
    entranceCoords->addWidget(this->entranceY);
    entranceOptions->addLayout(entranceCoords);

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

    auto exitCoordsLabel = new QLabel(tr("Exit Coords"),this);
    exitOptions->addWidget(exitCoordsLabel);
    auto exitCoords = new QHBoxLayout(this);
    this->exitX = new QSpinBox(this);
    this->exitX->setDisplayIntegerBase(16);
    this->exitX->setToolTip(tr("Exit X Position"));
    this->exitX->setMaximum(0xffff);
    exitCoords->addWidget(this->exitX);
    this->exitY = new QSpinBox(this);
    this->exitY->setDisplayIntegerBase(16);
    this->exitY->setToolTip(tr("Exit Y Position"));
    this->exitY->setMaximum(0xffff);
    exitCoords->addWidget(this->exitY);
    exitOptions->addLayout(exitCoords);

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

    this->detectChanges = true;
}

void LevelWindow::refreshLists() {
    this->detectChanges = false;
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
        exitItem->setData(LevelWindowDataKey::EXIT_INDEX_WINDOW,exitIndex);
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
    this->detectChanges = true;
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
    if (this->detectChanges == false) {
        YUtils::printDebug("Change detection deactivated (Entrance Anim)");
        return;
    }
    // std::cout << "entranceAnimChanged" << std::endl;
    // std::cout << this->entranceAnim->currentIndex() << std::endl;
    auto entranceAnimIndex = this->entranceAnim->currentIndex();
    if (entranceAnimIndex < 0) {
        // Will temporarily be -1 during changes
        return;
    }

    // Do updates for everything else //
    auto curLevel = this->yidsRom->currentLevelSelectData->getLevelByMpdz(this->yidsRom->mapData->filename);
    auto curEntranceSelected = this->entranceListWidget->currentRow();
    auto curEntrance = curLevel->entrances.at(curEntranceSelected);
    if (curEntrance->enterMapAnimation != entranceAnimIndex) {
        YUtils::printDebug("Modifying CRSB: Entrance Animation",DebugType::VERBOSE);
        curEntrance->enterMapAnimation = static_cast<LevelSelectEnums::MapEntranceAnimation>(entranceAnimIndex);
    }
}

void LevelWindow::entranceScreenChanged(const QString text) {
    Q_UNUSED(text);
    if (this->detectChanges == false) {
        YUtils::printDebug("Change detection deactivated (Entrance Screen)");
        return;
    }
    // std::cout << "entranceScreenChanged" << std::endl;
    // std::cout << this->entranceScreen->currentIndex() << std::endl;
    auto entranceScreenComboIndex = this->entranceScreen->currentIndex();
    if (entranceScreenComboIndex < 0) {
        // Will temporarily be -1 during changes
        return;
    }
    auto curLevel = this->yidsRom->currentLevelSelectData->getLevelByMpdz(this->yidsRom->mapData->filename);
    auto curEntranceSelected = this->entranceListWidget->currentRow();
    auto curEntrance = curLevel->entrances.at(curEntranceSelected);
    if (curEntrance->whichDsScreen != entranceScreenComboIndex) {
        YUtils::printDebug("Modifying CRSB: Entrance Screen",DebugType::VERBOSE);
        curEntrance->whichDsScreen = static_cast<LevelSelectEnums::StartingDsScreen>(entranceScreenComboIndex);
    }
}

void LevelWindow::exitTypeChanged(const QString text) {
    Q_UNUSED(text);
    if (this->detectChanges == false) {
        YUtils::printDebug("Change detection deactivated (Exit Type)");
        return;
    }
    // std::cout << "exitTypeChanged" << std::endl;
    // std::cout << this->exitTypeCombo->currentIndex() << std::endl;
    auto exitTypeComboIndex = this->exitTypeCombo->currentIndex();
    if (exitTypeComboIndex < 0) {
        // Will temporarily be -1 during changes
        return;
    }
    auto curLevel = this->yidsRom->currentLevelSelectData->getLevelByMpdz(this->yidsRom->mapData->filename);
    auto curExitSelected = this->exitListWidget->currentRow();
    auto curExit = curLevel->exits.at(curExitSelected);
    if(curExit->exitStartType != exitTypeComboIndex) {
        YUtils::printDebug("Modifying CRSB: Exit Type",DebugType::VERBOSE);
        curExit->exitStartType = static_cast<LevelSelectEnums::MapExitStartType>(exitTypeComboIndex);
    }
}

void LevelWindow::targetMapChanged(const QString text) {
    Q_UNUSED(text);
    if (this->detectChanges == false) {
        YUtils::printDebug("Change detection deactivated (Target map)");
        return;
    }
    //std::cout << "targetMapChanged" << std::endl;
    //std::cout << this->exitMapTarget->currentIndex() << std::endl;
    // ONLY update the available entrances
    auto currentSelectedExitMap = this->exitMapTarget->currentIndex();
    if (currentSelectedExitMap < 0) {
        // Will temporarily be -1 when updating
        //YUtils::printDebug("Invalid currentSelectedExitMap in targetMapChanged",DebugType::ERROR);
        return;
    }
    this->detectChanges = false;
    this->refreshTargetEntrances(currentSelectedExitMap);
    this->detectChanges = true;

    auto curLevel = this->yidsRom->currentLevelSelectData->getLevelByMpdz(this->yidsRom->mapData->filename);
    auto curExitSelected = this->exitListWidget->currentRow();
    if (curExitSelected < 0) {
        // Will temporarily be -1
        return;
    }
    auto curExit = curLevel->exits.at(curExitSelected);
    if (curExit->whichMapTo != currentSelectedExitMap) {
        YUtils::printDebug("Modifying CRSB: Exit Destination",DebugType::VERBOSE);
        curExit->whichMapTo = (uint8_t)currentSelectedExitMap;
    }
}

void LevelWindow::targetEntranceChanged(const QString text) {
    Q_UNUSED(text);
    if (this->detectChanges == false) {
        YUtils::printDebug("Change detection deactivated (Target entrance)");
        return;
    }
    // std::cout << "targetEntranceChanged" << std::endl;
    // std::cout << this->exitEntranceTarget->currentIndex() << std::endl;
    if (this->exitEntranceTarget == nullptr) {
        YUtils::printDebug("exitEntranceTarget was null in targetEntranceChanged",DebugType::ERROR);
        YUtils::popupAlert("exitEntranceTarget was null in targetEntranceChanged");
        return;
    }
    auto currentSelectedExitMapEntrance = this->exitEntranceTarget->currentIndex();
    if (currentSelectedExitMapEntrance < 0) {
        // Will temporarily be -1
        return;
    }

    auto curLevel = this->yidsRom->currentLevelSelectData->getLevelByMpdz(this->yidsRom->mapData->filename);
    auto curExitSelected = this->exitListWidget->currentRow();
    if (curExitSelected < 0) {
        // Will temporarily be -1
        return;
    }
    auto curExit = curLevel->exits.at(curExitSelected);
    if (curExit->whichEntranceTo != currentSelectedExitMapEntrance) {
        YUtils::printDebug("Modifying CRSB: Exit Destination Entrance",DebugType::VERBOSE);
        curExit->whichEntranceTo = currentSelectedExitMapEntrance;
    }
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
        entranceItem->setData(LevelWindowDataKey::ENTRANCE_INDEX_WINDOW,entranceIndex);
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

    auto entranceIndexQVariant = current->data(LevelWindowDataKey::ENTRANCE_INDEX_WINDOW);
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
    this->entranceScreen->setCurrentIndex(static_cast<int>(entranceData->whichDsScreen));
    this->entranceX->setValue((int)entranceData->entranceX);
    this->entranceY->setValue((int)entranceData->entranceY);
    
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
    auto exitIndex = current->data(LevelWindowDataKey::EXIT_INDEX_WINDOW).toUInt();
    
    auto exitData = curLevelData->exits.at(exitIndex);
    //YUtils::printDebug(exitData->toString());
    this->detectChanges = false;
    this->exitTypeCombo->setCurrentIndex(exitData->exitStartType);
    this->exitMapTarget->setCurrentIndex(exitData->whichMapTo);
    this->exitX->setValue((int)exitData->exitLocationX);
    this->exitY->setValue((int)exitData->exitLocationY);
    this->refreshTargetEntrances(exitData->whichMapTo);
    this->detectChanges = true;
}

void LevelWindow::entrancePositionChangedX() {
    if (this->detectChanges == false) {
        YUtils::printDebug("Change detection deactivated (X)");
        return;
    }
    auto curLevel = this->yidsRom->currentLevelSelectData->getLevelByMpdz(this->yidsRom->mapData->filename);
    auto curEntranceSelected = this->entranceListWidget->currentRow();
    if (curEntranceSelected < 0) {
        YUtils::printDebug("curEntranceSelected negative",DebugType::VERBOSE);
        return;
    }
    auto curEntrance = curLevel->entrances.at(curEntranceSelected);
    auto xValue = this->entranceX->value();
    if (xValue < 0) {
        YUtils::printDebug("Entrance X change value negative",DebugType::ERROR);
        return;
    }
    if ((uint32_t)xValue > this->yidsRom->mapData->getGreatestCanvasWidth()) {
        YUtils::printDebug("Entrance X change value too high",DebugType::ERROR);
        return;
    }
    if (curEntrance->entranceX != (uint16_t)xValue) {
        YUtils::printDebug("Modifying CRSB: Entrance X",DebugType::VERBOSE);
        std::cout << std::hex << (uint16_t)xValue << std::endl;
        curEntrance->entranceX = (uint16_t)xValue;
    }
}

void LevelWindow::entrancePositionChangedY() {
    if (this->detectChanges == false) {
        YUtils::printDebug("Change detection deactivated (Y)");
        return;
    }
    auto curLevel = this->yidsRom->currentLevelSelectData->getLevelByMpdz(this->yidsRom->mapData->filename);
    auto curEntranceSelected = this->entranceListWidget->currentRow();
    if (curEntranceSelected < 0) {
        YUtils::printDebug("curEntranceSelected negative",DebugType::VERBOSE);
        return;
    }
    auto curEntrance = curLevel->entrances.at(curEntranceSelected);
    auto yValue = this->entranceY->value();
    if (yValue < 0) {
        YUtils::printDebug("Entrance Y change value negative",DebugType::ERROR);
        return;
    }
    if ((uint32_t)yValue > this->yidsRom->mapData->getGreatestCanvasHeight()) {
        YUtils::printDebug("Entrance Y change value too high",DebugType::ERROR);
        return;
    }
    if (curEntrance->entranceY != (uint16_t)yValue) {
        YUtils::printDebug("Modifying CRSB: Entrance Y",DebugType::VERBOSE);
        std::cout << std::hex << (uint16_t)yValue << std::endl;
        curEntrance->entranceY = (uint16_t)yValue;
    }
}
