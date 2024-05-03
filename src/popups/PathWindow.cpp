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
    if (gOverlay == nullptr) {
        YUtils::printDebug("gOverlay was null",DebugType::FATAL);
        YUtils::popupAlert("gOverlay was null");
        exit(EXIT_FAILURE);
    }
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
    this->pointListWidget = new QListWidget(this);
    rightColumn->addWidget(this->pointListWidget);

    connect(this->pathListWidget,&QListWidget::currentRowChanged,this,&PathWindow::pathListRowSelectionChanged);
}

void PathWindow::refreshPathList() {
    //std::cout << "refreshPathList" << std::endl;
    this->detectChanges = false;

    // Wipe everything before checking for data
    while (this->pathListWidget->count() > 0) {
        // Holds them as pointers
        delete this->pathListWidget->takeItem(0);
    }

    this->pathListWidget->clear();

    if (this->yidsRom->mapData == nullptr) {
        YUtils::printDebug("MapData is null",DebugType::ERROR);
        this->detectChanges = true; // Just in case
        return;
    }
    auto pathDataMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::PATH_MAGIC_NUM,true);
    if (pathDataMaybe == nullptr) {
        //YUtils::printDebug("No PATH data found in refreshPathList",DebugType::WARNING);
        this->detectChanges = true;
        return;
    }
    auto pathData = static_cast<PathData*>(pathDataMaybe);
    //YUtils::printDebug("refreshPathList for PathWindow");
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

void PathWindow::refreshPointList() {
    this->detectChanges = false;
    while (this->pointListWidget->count() > 0) {
        delete this->pointListWidget->takeItem(0);
    }
    this->pointListWidget->clear();

    auto pathIndex = this->pathListWidget->currentRow();
    if (pathIndex == -1) {
        //YUtils::printDebug("No Path selected",DebugType::WARNING);
        this->detectChanges = true;
        return;
    }

    // Get data
    if (this->yidsRom->mapData == nullptr) {
        YUtils::printDebug("MapData is null",DebugType::ERROR);
        this->detectChanges = true; // Just in case
        return;
    }
    auto pathDataMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::PATH_MAGIC_NUM,true);
    if (pathDataMaybe == nullptr) {
        YUtils::printDebug("No PATH data found in refreshPointList",DebugType::WARNING);
        this->detectChanges = true;
        return;
    }
    auto pathData = static_cast<PathData*>(pathDataMaybe);

    if (pathIndex >= (int)pathData->paths.size()) {
        YUtils::printDebug("Seeking Path data out of bounds",DebugType::ERROR);
        this->detectChanges = true;
        return;
    }

    auto pathDataSelected = pathData->paths.at(pathIndex);

    uint pointIndex = 0;
    for (auto pit = pathDataSelected.cbegin(); pit != pathDataSelected.cend(); pit++) {
        auto point = (*pit);
        Q_UNUSED(point);
        std::stringstream ss;
        ss << "Point 0x" << std::hex << pointIndex;
        this->pointListWidget->addItem(QString::fromStdString(ss.str()));
        pointIndex++;
    }

    this->detectChanges = true;
}

void PathWindow::pathListRowSelectionChanged(int rowIndex) {
    //std::cout << "pathListRowSelectionChanged " << rowIndex << std::endl;
    this->refreshPointList();
    if (rowIndex >= this->pathListWidget->count()) {
        YUtils::printDebug("rowIndex too high in pathListRowSelectionChanged",DebugType::ERROR);
        return;
    }
    if (rowIndex < -1) {
        YUtils::printDebug("Unusually negative rowIndex",DebugType::ERROR);
        return;
    }
    if (this->gridOverlay == nullptr) {
        YUtils::printDebug("gridOverlay was null",DebugType::ERROR);
        return;
    }
    this->gridOverlay->selectedPathIndex = rowIndex;
    this->gridOverlay->repaint();
}
