#include "MapSelect.h"

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
    column1->addWidget(this->leftList);

    // Column 2 //
    auto column2 = new QVBoxLayout(this);
    mainLayout->addLayout(column2);
    this->rightList = new QListWidget(this);
    column2->addWidget(this->rightList);
    // Buttons
    auto buttonContainer = new QHBoxLayout(this);
    column2->addLayout(buttonContainer);
    auto cancelButton = new QPushButton("&Cancel",this);
    buttonContainer->addWidget(cancelButton);
    auto confirmButton = new QPushButton("&Load",this);
    buttonContainer->addWidget(confirmButton);

    // Update
    this->updateLeftList();
}

void MapSelect::updateLeftList() {
    std::filesystem::path filesPath = "_nds_unpack/data/file";
    if (!std::filesystem::exists(filesPath)) {
        YUtils::printDebug("Could not retrieve CRSBs, directory invalid",DebugType::ERROR);
        YUtils::popupAlert("Could not retrieve CRSBs, directory invalid");
        return;
    }
    QStringList crsbFilenames;
    for (const auto &entry : std::filesystem::directory_iterator(filesPath)) {
        auto pathName = entry.path().string();
        // QT has a crapton of neat helpers, use them
        QString pathString(pathName.c_str());
        if (pathString.toLower().endsWith(".crsb")) {
            auto fileName = pathString.split("/").last();
            crsbFilenames.push_back(fileName);
        }
    }
    crsbFilenames.sort(Qt::CaseInsensitive);
    this->leftList->addItems(crsbFilenames);
}
