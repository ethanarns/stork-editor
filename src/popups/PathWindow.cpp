#include "PathWindow.h"
#include "../utils.h"
#include "../GridOverlay.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QSpinBox>

#include <sstream>
#include <climits>

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
    this->setWindowTitle(tr("Path Window (WIP)"));
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
    this->xSpinBox->setDisplayIntegerBase(16);
    this->xSpinBox->setMinimum(0);
    this->xSpinBox->setMaximum(INT_MAX);
    this->xSpinBox->setSingleStep(0x1000);
    this->xSpinBox->setDisabled(false);
    this->xSpinBox->setToolTip("Fine X Location");
    locationsLayout->addWidget(xSpinBox);
    this->ySpinBox = new QSpinBox(this);
    this->ySpinBox->setDisplayIntegerBase(16);
    this->ySpinBox->setMinimum(0);
    this->ySpinBox->setMaximum(INT_MAX);
    this->ySpinBox->setSingleStep(0x1000);
    this->ySpinBox->setDisabled(false);
    this->ySpinBox->setToolTip("Fine Y Location");
    locationsLayout->addWidget(ySpinBox);

    auto pointInfoLayout = new QHBoxLayout(this);
    this->distanceBox = new QSpinBox(this);
    this->distanceBox->setDisabled(false);
    this->distanceBox->setMinimum(0);
    this->distanceBox->setMaximum(INT_MAX);
    this->distanceBox->setDisplayIntegerBase(16);
    this->distanceBox->setToolTip("Distance (Fine)");
    pointInfoLayout->addWidget(this->distanceBox);
    this->angleBox = new QDoubleSpinBox(this);
    this->angleBox->setDisabled(false);
    this->angleBox->setMinimum(-720);
    this->angleBox->setMaximum(720);
    this->angleBox->setToolTip("Angle (Degrees");
    this->angleBox->setSingleStep(0.1);
    pointInfoLayout->addWidget(this->angleBox);

    mainLayout->addLayout(rightColumn);

    this->pathListWidget = new QListWidget(this);
    leftColumn->addWidget(this->pathListWidget);
    this->pointListWidget = new QListWidget(this);
    rightColumn->addWidget(this->pointListWidget);
    rightColumn->addLayout(locationsLayout);
    rightColumn->addLayout(pointInfoLayout);

    connect(this->pathListWidget,&QListWidget::currentRowChanged,this,&PathWindow::pathListRowSelectionChanged);
    connect(this->pointListWidget,&QListWidget::currentRowChanged,this,&PathWindow::pointSelectionChanged);
    connect(this->xSpinBox,&QSpinBox::valueChanged,this,&PathWindow::xSpinChange);
    connect(this->ySpinBox,&QSpinBox::valueChanged,this,&PathWindow::ySpinChange);
    connect(this->distanceBox,&QSpinBox::valueChanged,this,&PathWindow::distanceSpinChange);
    connect(this->angleBox,&QDoubleSpinBox::valueChanged,this,&PathWindow::angleSpinChange);
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
    this->pointListWidget->setCurrentRow(-1);
    this->pointListWidget->clearSelection();
    // Since there should be nothing selected now
    this->xSpinBox->setDisabled(true);
    this->xSpinBox->setValue(0);
    this->ySpinBox->setDisabled(true);
    this->ySpinBox->setValue(0);
    this->distanceBox->setDisabled(true);
    this->distanceBox->setValue(0);
    this->angleBox->setDisabled(true);
    this->angleBox->setValue(0);
    this->detectChanges = true;
}

void PathWindow::pointSelectionChanged(int rowIndex) {
    if (this->detectChanges == false) {
        return;
    }
    // Debug stuff
    //std::cout << "pointSelectionChanged: 0x" << std::hex << rowIndex << std::endl;
    if (rowIndex < -1) {
        YUtils::printDebug("rowIndex too low, below -1 somehow", DebugType::ERROR);
        return;
    }
    if (rowIndex >= this->pointListWidget->count()) {
        YUtils::printDebug("rowIndex too high in pointSelectionChanged",DebugType::ERROR);
        return;
    }

    // Update GridOverlay
    if (this->gridOverlay == nullptr) {
        YUtils::printDebug("gridOverlay was null",DebugType::ERROR);
        return;
    }
    this->gridOverlay->selectedPathSubIndex = rowIndex;
    this->gridOverlay->repaint();

    // Now update the spinboxes
    auto point = this->getPathPoint(rowIndex);
    auto x = point->xFine;
    auto y = point->yFine;
    this->detectChanges = false;
    this->xSpinBox->setValue(x);
    this->ySpinBox->setValue(y);
    this->distanceBox->setValue(point->distance);
    float degrees = YUtils::yanglesToDegrees(point->angle);
    std::cout << std::hex << "Yangles: 0x" << point->angle << ", Degrees: " << std::dec << degrees << std::endl;
    this->angleBox->setValue(degrees);
    this->detectChanges = true;

    if (rowIndex != -1) {
        this->xSpinBox->setDisabled(false);
        this->ySpinBox->setDisabled(false);
        this->angleBox->setDisabled(false);
        this->distanceBox->setDisabled(false);
    }
}

std::vector<PathSection *> PathWindow::getSelectedPathData() {
    if (this->yidsRom->mapData == nullptr) {
        YUtils::printDebug("MapData is null",DebugType::ERROR);
        return std::vector<PathSection *>(); // Return empty
    }
    int rowIndex = this->pathListWidget->currentRow();
    if (rowIndex >= this->pathListWidget->count()) {
        YUtils::printDebug("rowIndex too high in getSelectedPathData",DebugType::ERROR);
        std::stringstream ss;
        ss << "rowIndex: 0x" << std::hex << rowIndex << ", pathListCount: 0x" << std::hex << this->pathListWidget->count();
        YUtils::printDebug(ss.str(),DebugType::ERROR);
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
        std::stringstream ss;
        ss << "rowIndex: 0x" << std::hex << rowIndex << ", pathListCount: 0x" << std::hex << this->pathListWidget->count();
        YUtils::printDebug(ss.str(),DebugType::ERROR);
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

PathSection* PathWindow::getPathPoint(int index) {
    if (index < 0) {
        YUtils::printDebug("negative rowIndex in getPathPoint",DebugType::ERROR);
        return nullptr;
    }

    auto pathData = this->getSelectedPathData();
    if (pathData.size() == 0) {
        YUtils::printDebug("No selected path data found in getPathPoint",DebugType::WARNING);
        return nullptr;
    }
    // Safe to cast to unsigned as negative check was earlier
    if ((unsigned int)index >= pathData.size()) {
        YUtils::printDebug("point index selected is out of bounds in actual point list in getPathPoint",DebugType::ERROR);
        return nullptr;
    }
    auto pointSelected = pathData.at(index);
    if (pointSelected == nullptr) {
        YUtils::printDebug("pointSelected is null in getPathPoint",DebugType::ERROR);
        return nullptr;
    }
    return pointSelected;
}

void PathWindow::xSpinChange(int newValueX) {
    if (this->pointListWidget->currentRow() == -1) {
        //YUtils::printDebug("No Point selected, cannot change X",DebugType::WARNING);
        return;
    }
    if (this->detectChanges == false) {
        //YUtils::printDebug("Don't allow changes to X",DebugType::WARNING);
        return;
    }
    //std::cout << "xSpinChange: 0x" << std::hex << newValueX << std::endl;
    if (newValueX < 0) {
        YUtils::printDebug("X cannot be below 0",DebugType::ERROR);
        return;
    }
    if (newValueX >= INT_MAX) {
        YUtils::printDebug("X cannot be above INT_MAX",DebugType::ERROR);
        return;
    }
    auto curPoint = this->getSelectedPathPoint();
    if (curPoint == nullptr) {
        YUtils::printDebug("No point active in xSpinChange",DebugType::WARNING);
        return;
    }
    curPoint->xFine = newValueX;
    emit this->markSavableChange();
    this->gridOverlay->repaint();
}

void PathWindow::ySpinChange(int newValueY) {
    if (this->pointListWidget->currentRow() == -1) {
        //YUtils::printDebug("No Point selected, cannot change Y",DebugType::WARNING);
        return;
    }
    if (this->detectChanges == false) {
        //YUtils::printDebug("Don't allow changes to Y",DebugType::WARNING);
        return;
    }
    //std::cout << "ySpinChange: 0x" << std::hex << newValueY << std::endl;
    if (newValueY < 0) {
        YUtils::printDebug("Y cannot be below 0",DebugType::ERROR);
        return;
    }
    if (newValueY >= INT_MAX) {
        YUtils::printDebug("Y cannot be above INT_MAX",DebugType::ERROR);
        return;
    }
    auto curPoint = this->getSelectedPathPoint();
    if (curPoint == nullptr) {
        YUtils::printDebug("No point active in ySpinChange",DebugType::WARNING);
        return;
    }
    curPoint->yFine = newValueY;
    emit this->markSavableChange();
    this->gridOverlay->repaint();
}

void PathWindow::angleSpinChange(double newValueAngleDegrees) {
    if (this->pointListWidget->currentRow() == -1) {
        //YUtils::printDebug("No Point selected, cannot change",DebugType::WARNING);
        return;
    }
    if (this->detectChanges == false) {
        //YUtils::printDebug("Don't allow changes",DebugType::WARNING);
        return;
    }
    if (newValueAngleDegrees < 0) {
        YUtils::printDebug("angle cannot be below 0",DebugType::ERROR);
        return;
    }
    if (newValueAngleDegrees >= 720) {
        YUtils::printDebug("angle cannot be above 720 degrees",DebugType::ERROR);
        return;
    }
    auto curPoint = this->getSelectedPathPoint();
    if (curPoint == nullptr) {
        YUtils::printDebug("No point active in angleSpinChange",DebugType::WARNING);
        return;
    }
    curPoint->angle = YUtils::degreesToYangles(newValueAngleDegrees);
    emit this->markSavableChange();
    this->gridOverlay->repaint();
}

void PathWindow::distanceSpinChange(int newValueDistance) {
    if (this->pointListWidget->currentRow() == -1) {
        //YUtils::printDebug("No Point selected, cannot change",DebugType::WARNING);
        return;
    }
    if (this->detectChanges == false) {
        //YUtils::printDebug("Don't allow changes",DebugType::WARNING);
        return;
    }
    if (newValueDistance < 0) {
        YUtils::printDebug("distance cannot be below 0",DebugType::ERROR);
        return;
    }
    if (newValueDistance >= INT_MAX) {
        YUtils::printDebug("distance cannot be above INT_MAX",DebugType::ERROR);
        return;
    }
    auto curPoint = this->getSelectedPathPoint();
    if (curPoint == nullptr) {
        YUtils::printDebug("No point active in distanceSpinChange",DebugType::WARNING);
        return;
    }
    curPoint->distance = newValueDistance;
    emit this->markSavableChange();
    this->gridOverlay->repaint();
}
