#include "PathWindow.h"
#include "../utils.h"
#include "../GridOverlay.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>

#include <sstream>

PathWindow::PathWindow(QWidget *parent, YidsRom *rom, GridOverlay* gOverlay) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
    this->gridOverlay = gOverlay;
    this->detectChanges = false;
    this->setWindowTitle(tr("Path Window"));
    this->setObjectName(tr("pathWindow"));

    auto mainLayout = new QHBoxLayout(this);
    this->setLayout(mainLayout);
    auto leftColumn = new QVBoxLayout(this);
    auto leftColumnTitle = new QLabel("Paths",this);
    leftColumn->addWidget(leftColumnTitle);
    mainLayout->addLayout(leftColumn);
    auto rightColumn = new QVBoxLayout(this);
    auto rightColumnTitle = new QLabel("Path Points",this);
    rightColumn->addWidget(rightColumnTitle);
    mainLayout->addLayout(rightColumn);

    this->pathListWidget = new QListWidget(this);
    leftColumn->addWidget(this->pathListWidget);
    this->subPathListWidget = new QListWidget(this);
    rightColumn->addWidget(this->subPathListWidget);
}

void PathWindow::refreshPathList() {
    this->detectChanges = false;

    // Wipe everything before checking for data
    while (this->pathListWidget->count() > 0) {
        // Holds them as pointers
        delete this->pathListWidget->takeItem(0);
    }
    while (this->subPathListWidget->count() > 0) {
        delete this->subPathListWidget->takeItem(0);
    }
    this->pathListWidget->clear();
    this->subPathListWidget->clear();

    if (this->yidsRom->mapData == nullptr) {
        YUtils::printDebug("MapData is null",DebugType::ERROR);
        this->detectChanges = true; // Just in case
        return;
    }
    auto pathDataMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::PATH_MAGIC_NUM,true);
    if (pathDataMaybe == nullptr) {
        YUtils::printDebug("No PATH data found",DebugType::WARNING);
        this->detectChanges = true;
        return;
    }
    auto pathData = static_cast<PathData*>(pathDataMaybe);
    YUtils::printDebug("refreshPathList for PathWindow");
    int pathIndex = 0;

    if (pathData->paths.size() == 0) {
        YUtils::printDebug("No paths in PATH, cancel");
        this->detectChanges = true;
        return;
    }
    for (auto pit = pathData->paths.begin(); pit != pathData->paths.end(); pit++) {
        std::stringstream ssPathName;
        ssPathName << "Path 0x" << std::hex << std::setw(2) << std::setfill('0') << pathIndex;
        this->pathListWidget->addItem(QString::fromStdString(ssPathName.str()));
        pathIndex++;
    }
    this->detectChanges = true;
}
