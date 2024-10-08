#include "PathWindow.h"
#include "../utils.h"
#include "../GridOverlay.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QSpinBox>

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
    auto locationsLayout = new QHBoxLayout(this);
    this->xSpinBox = new QSpinBox(this);
    this->xSpinBox->setMinimum(0);
    this->xSpinBox->setSingleStep(1);
    locationsLayout->addWidget(xSpinBox);
    this->ySpinBox = new QSpinBox(this);
    this->ySpinBox->setMinimum(0);
    this->ySpinBox->setSingleStep(1);
    locationsLayout->addWidget(ySpinBox);

    mainLayout->addLayout(rightColumn);

    this->pathListWidget = new QListWidget(this);
    leftColumn->addWidget(this->pathListWidget);
    this->pointListWidget = new QListWidget(this);
    rightColumn->addWidget(this->pointListWidget);
    rightColumn->addLayout(locationsLayout);

    connect(this->pathListWidget,&QListWidget::currentRowChanged,this,&PathWindow::pathListRowSelectionChanged);
    connect(this->pointListWidget,&QListWidget::currentRowChanged,this,&PathWindow::pointSelectionChanged);
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
    // Wipe the existing list
    while (this->pointListWidget->count() > 0) {
        delete this->pointListWidget->takeItem(0);
    }
    this->pointListWidget->clear();

    // Get the path data from PATH
    auto pathDataSelected = this->getSelectedPathData();

    // Refill the list
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
    this->detectChanges = false;
    //std::cout << "pathListRowSelectionChanged " << rowIndex << std::endl;
    this->refreshPointList();
    if (rowIndex >= this->pathListWidget->count()) {
        YUtils::printDebug("rowIndex too high in pathListRowSelectionChanged",DebugType::ERROR);
        this->detectChanges = true;
        return;
    }
    if (rowIndex < -1) {
        YUtils::printDebug("Unusually negative rowIndex",DebugType::ERROR);
        this->detectChanges = true;
        return;
    }
    if (this->gridOverlay == nullptr) {
        YUtils::printDebug("gridOverlay was null",DebugType::ERROR);
        this->detectChanges = true;
        return;
    }
    this->gridOverlay->selectedPathIndex = rowIndex;
    this->gridOverlay->repaint();
    this->detectChanges = true;
}

void PathWindow::pointSelectionChanged(int rowIndex) {
    if (this->detectChanges == false) {
        return;
    }
    // Debug stuff
    std::cout << "pointSelectionChanged: " << rowIndex << std::endl;
    if (rowIndex < -1) {
        YUtils::printDebug("rowIndex too low, below -1 somehow", DebugType::ERROR);
        return;
    }
    if (rowIndex >= this->pointListWidget->count()) {
        YUtils::printDebug("rowIndex too high in pointSelectionChanged",DebugType::ERROR);
        return;
    }

    // Retrieve the major/parent path data within PATH for this map
    auto selectedPath = this->getSelectedPathData();

    // Update GridOverlay
    if (this->gridOverlay == nullptr) {
        YUtils::printDebug("gridOverlay was null",DebugType::ERROR);
        return;
    }
    this->gridOverlay->selectedPathSubIndex = rowIndex;
    this->gridOverlay->repaint();
}

std::vector<PathSection *> PathWindow::getSelectedPathData() {
    if (this->yidsRom->mapData == nullptr) {
        YUtils::printDebug("MapData is null",DebugType::ERROR);
        return std::vector<PathSection *>(); // Return empty
    }
    int rowIndex = this->pathListWidget->currentRow();
    if (rowIndex >= this->pathListWidget->count()) {
        YUtils::printDebug("rowIndex too high in getSelectedPathData",DebugType::ERROR);
        return std::vector<PathSection *>(); // Return empty
    }
    if (rowIndex < 0) {
        // Fail silently, there just isn't anything selected
        return std::vector<PathSection *>();
    }
    // Retrieve the PATH data for this map
    auto pathDataMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::PATH_MAGIC_NUM,true);
    if (pathDataMaybe == nullptr) {
        YUtils::printDebug("No PATH data found in getSelectedPathData",DebugType::WARNING);
        this->detectChanges = true;
        return std::vector<PathSection *>();
    }
    auto pathData = static_cast<PathData*>(pathDataMaybe);

    if (rowIndex >= (int)pathData->paths.size()) {
        YUtils::printDebug("Seeking Path data out of bounds in getSelectedPathData",DebugType::ERROR);
        return std::vector<PathSection *>();
    }
    auto pathDataSelected = pathData->paths.at(rowIndex);
    return pathDataSelected;
}

PathSection* PathWindow::getSelectedPathPoint() {
    // Get the row properly
    int rowIndex = this->pointListWidget->currentRow();
    if (rowIndex >= this->pathListWidget->count()) {
        YUtils::printDebug("rowIndex too high in getSelectedPathPoint",DebugType::ERROR);
        return nullptr; // Return empty
    }
    // This should never be called if there is nothing selected, check prior
    if (rowIndex < 0) {
        YUtils::printDebug("negative rowIndex in getSelectedPathPoint",DebugType::ERROR);
        return nullptr;
    }

    auto pathData = this->getSelectedPathData();
    if (pathData.size() == 0) {
        YUtils::printDebug("No selected path data found in getSelectedPathPoint",DebugType::WARNING);
        return nullptr;
    }
    // Safe to cast to unsigned as negative check was earlier
    if ((unsigned int)rowIndex >= pathData.size()) {
        YUtils::printDebug("point index selected is out of bounds in actual point list",DebugType::ERROR);
        return nullptr;
    }
    auto pointSelected = pathData.at(rowIndex);
    if (pointSelected == nullptr) {
        YUtils::printDebug("pointSelected is null",DebugType::ERROR);
        return nullptr;
    }
    return pointSelected;
}
