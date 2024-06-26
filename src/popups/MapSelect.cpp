#include "MapSelect.h"
#include "../data/LevelSelectData.h"
#include "../GlobalSettings.h"

#include <filesystem>
#include <string>
#include <iostream>

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QFileDialog>

MapSelect::MapSelect(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
    this->setWindowTitle(tr("Select a map"));
    this->setObjectName(tr("mapSelectWindow"));
    auto mainLayout = new QHBoxLayout(this);
    this->setLayout(mainLayout);

    // Column 1 //
    auto column1 = new QVBoxLayout(this);
    mainLayout->addLayout(column1);
    this->leftList = new QListWidget(this);
    this->leftList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    column1->addWidget(this->leftList);
    connect(this->leftList,&QListWidget::currentItemChanged,this,&MapSelect::currentItemChanged);

    // Column 2 //
    auto column2 = new QVBoxLayout(this);
    mainLayout->addLayout(column2);
    this->rightList = new QListWidget(this);
    this->rightList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    column2->addWidget(this->rightList);
    // Buttons
    auto buttonContainer = new QHBoxLayout(this);
    column2->addLayout(buttonContainer);

    auto confirmButton = new QPushButton("&Load",this);
    buttonContainer->addWidget(confirmButton);
    connect(confirmButton,&QPushButton::pressed,this,&MapSelect::confirmClicked);

    auto addButton = new QPushButton("&Add",this);
    buttonContainer->addWidget(addButton);
    connect(addButton,&QPushButton::pressed,this,&MapSelect::addMapClicked);

    auto deleteButton = new QPushButton("&Delete",this);
    deleteButton->setDisabled(true); // TODO: Add delete
    buttonContainer->addWidget(deleteButton);
    connect(deleteButton,&QPushButton::pressed,this,&MapSelect::deleteMapClicked);
}

void MapSelect::updateLeftList() {
    std::stringstream ssUnpackedFileLoc;
    ssUnpackedFileLoc << globalSettings.extractFolderName << "/data/file";
    std::filesystem::path filesPath = ssUnpackedFileLoc.str();
    if (!std::filesystem::exists(filesPath)) {
        YUtils::printDebug("Could not retrieve CRSBs, directory invalid",DebugType::ERROR);
        YUtils::popupAlert("Could not retrieve CRSBs, directory invalid");
        return;
    }
    YUtils::printDebug("Refreshing CRSB list",DebugType::VERBOSE);
    this->leftList->clearSelection();
    for (int i = 0; i < this->leftList->count(); i++) {
        delete this->leftList->item(i);
    }
    // Item memory is freed, but not references
    this->leftList->clear();
    // Retrieve filenames
    QStringList crsbFilenames;
    for (const auto &entry : std::filesystem::directory_iterator(filesPath)) {
        auto pathName = entry.path().string();
        // QT has a crapton of neat helpers, use them
        QString pathString(pathName.c_str());
        if (pathString.toLower().endsWith(".crsb")) {
            // Handle dumb windows crap
            auto fileName = pathString.split("/").last().split("\\").last();
            crsbFilenames.push_back(fileName);
        }
    }
    if (crsbFilenames.empty()) {
        YUtils::printDebug("No CRSBs found",DebugType::ERROR);
        YUtils::popupAlert("No level files found");
        return;
    }
    crsbFilenames.sort(Qt::CaseInsensitive);
    this->leftList->addItems(crsbFilenames);
}

void MapSelect::currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    Q_UNUSED(previous);
    if (current == nullptr) {
        // This is always triggered on a second popup since memory was freed
        // And it automatically selects something
        YUtils::printDebug("MapSelect current item changed is null",DebugType::VERBOSE);
        return;
    }
    auto crsbFilename = current->text();
    auto crsbVector = this->yidsRom->getByteVectorFromFile(crsbFilename.toStdString());
    if (crsbVector.empty()) {
        YUtils::printDebug("Failed to get CRSB vector in MapSelect",DebugType::ERROR);
        YUtils::popupAlert("Failed to get level data vector");
        return;
    }
    auto crsbData = new LevelSelectData(crsbVector);
    crsbData->filename = crsbFilename.toStdString();
    //YUtils::printDebug("Updating right list",DebugType::VERBOSE);
    this->rightList->clearSelection();
    for (int i = 0; i < this->rightList->count(); i++) {
        delete this->rightList->item(i);
    }
    // Item memory is freed, but not references
    this->rightList->clear();
    QStringList mpdzList;
    for (auto it = crsbData->levels.cbegin(); it != crsbData->levels.cend(); it++) {
        auto mpdzName = (*it)->mpdzFileNoExtension;
        mpdzList.push_back(QString(mpdzName.c_str()));
    }
    if (mpdzList.empty()) {
        YUtils::printDebug("No MPDZs found",DebugType::ERROR);
        YUtils::popupAlert("No map files found");
        return;
    }
    this->rightList->addItems(mpdzList);
    this->crsbTemp = crsbData;
}

void MapSelect::addMapClicked() {
    //YUtils::printDebug("Add map clicked");
    auto fileName = QFileDialog::getOpenFileName(this,tr("Select map file"),".",tr("MPDZ files (*.mpdz)"));
    if (fileName.isEmpty()) {
        YUtils::printDebug("Canceled file dialog",DebugType::VERBOSE);
        return;
    } else {
        if (!fileName.endsWith(".mpdz")) {
            YUtils::printDebug("Filename does not end with .mpdz",DebugType::WARNING);
            YUtils::popupAlert("Filename does not end with .mpdz");
            return;
        }
        auto mpdzFileVectorCompressed = YUtils::getUint8VectorFromFile(fileName.toStdString());
        auto mpdz = YCompression::lz10decomp(mpdzFileVectorCompressed);
        auto mpdzHeader = YUtils::getUint32FromVec(mpdz,0);
        if (mpdzHeader != Constants::MPDZ_MAGIC_NUM) {
            YUtils::printDebug("Not an MPDZ file",DebugType::WARNING);
            YUtils::popupAlert("Not an MPDZ file");
            return;
        }
        YUtils::printDebug("MPDZ file confirmed");
        auto newMpdz = new LevelMetadata();
        #ifdef _WIN32
        newMpdz->mpdzFileNoExtension = fileName.split("\\").last().replace(".mpdz","").toStdString();
        #else
        newMpdz->mpdzFileNoExtension = fileName.split("/").last().replace(".mpdz","").toStdString();
        #endif
        newMpdz->musicId = 0;
        auto firstMapEntrance = new MapEntrance();
        firstMapEntrance->enterMapAnimation = LevelSelectEnums::MapEntranceAnimation::SPAWN_STATIC_RIGHT;
        firstMapEntrance->entranceX = 0;
        firstMapEntrance->entranceY = 0;
        firstMapEntrance->whichDsScreen = LevelSelectEnums::StartingDsScreen::START_BOTTOM;
        firstMapEntrance->_uuid = this->crsbTemp->_portalUuid++;
        newMpdz->entrances.push_back(firstMapEntrance);
        if (this->crsbTemp == nullptr) {
            YUtils::printDebug("CRSBTEMP null",DebugType::ERROR);
            YUtils::popupAlert("CrsbTemp Error");
            delete newMpdz;
            return;
        }
        this->crsbTemp->levels.push_back(newMpdz);
        auto compiledCrsb = this->crsbTemp->compile();
        std::stringstream crsbOutFile;
        crsbOutFile << globalSettings.extractFolderName << "/data/file/" << this->crsbTemp->filename;
        YUtils::writeByteVectorToFile(compiledCrsb,crsbOutFile.str());
        YUtils::printDebug("Added new MPDZ to CRSB file");
        this->rightList->addItem(QString::fromStdString(newMpdz->mpdzFileNoExtension));
        YUtils::popupAlert("Success! Ensure your entrances and exits are connected");
        this->rightList->setCurrentRow(this->rightList->count()-1);
        this->confirmClicked();
    }
}

void MapSelect::deleteMapClicked() {
    YUtils::printDebug("Delete map");
}

void MapSelect::confirmClicked() {
    auto rightSelectedItems = this->rightList->selectedItems();
    if (rightSelectedItems.size() != 1) {
        YUtils::printDebug("Empty map file selection",DebugType::VERBOSE);
        YUtils::popupAlert("No map file selected");
        return;
    }
    this->hide();
    // Copy data to NEW pointer, don't transfer pointer
    this->yidsRom->currentLevelSelectData = new LevelSelectData(*this->crsbTemp);
    auto selectedItem = rightSelectedItems.at(0);
    if (selectedItem == nullptr) {
        YUtils::printDebug("Selected item was null",DebugType::WARNING);
        return;
    }
    auto mapName = selectedItem->text().toStdString();
    emit this->mpdzSelected(mapName);
}
