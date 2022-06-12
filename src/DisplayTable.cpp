#include "DisplayTable.h"

#include <QtCore>
#include <QTableWidget>

DisplayTable::DisplayTable(QWidget* parent) {
    Q_UNUSED(parent);
    
    // Layout //
    setRowCount(0xff);
    setColumnCount(0xff);

    // Drag and Drop //
    setMouseTracking(true);
}