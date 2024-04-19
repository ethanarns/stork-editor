#include "PathWindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>

PathWindow::PathWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
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

void PathWindow::refreshLists() {

}
