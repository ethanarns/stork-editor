#pragma once

#include <QFrame>
#include <QWidget>

class GridOverlay : public QFrame {
    Q_OBJECT
public:
    GridOverlay(QWidget* viewport);
    void updateSizeToGrid(int rows, int columns);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};