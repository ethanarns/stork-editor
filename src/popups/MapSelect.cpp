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

    auto cancelButton = new QPushButton("&Cancel",this);
    buttonContainer->addWidget(cancelButton);
    connect(cancelButton,&QPushButton::pressed,this,&MapSelect::cancelClicked);

    auto confirmButton = new QPushButton("&Load",this);
    buttonContainer->addWidget(confirmButton);
    connect(confirmButton,&QPushButton::pressed,this,&MapSelect::confirmClicked);
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
    YUtils::printDebug("Refreshing leftList",DebugType::VERBOSE);
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

void MapSelect::cancelClicked() {
    this->close();
}

void MapSelect::confirmClicked() {
    this->hide();
    // Copy data to NEW pointer, don't transfer pointer
    this->yidsRom->currentLevelSelectData = new LevelSelectData(*this->crsbTemp);
    auto rightSelectedItems = this->rightList->selectedItems();
    if (rightSelectedItems.size() != 1) {
        YUtils::printDebug("Unusual right selection size",DebugType::ERROR);
        YUtils::popupAlert("Unusual right selection size");
        return;
    }
    auto selectedItem = rightSelectedItems.at(0);
    if (selectedItem == nullptr) {
        YUtils::printDebug("Selected item was null",DebugType::WARNING);
        return;
    }
    auto mapName = selectedItem->text().toStdString();
    emit this->mpdzSelected(mapName);
}
