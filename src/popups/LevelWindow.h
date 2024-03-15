#pragma once

#include "../yidsrom.h"

#include <QtCore>
#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

class LevelWindow : public QWidget {
    Q_OBJECT
public:
    LevelWindow(QWidget *parent, YidsRom* rom);
    QListWidget* entranceListWidget;
    QListWidget* exitListWidget;
    void refreshLists();
private:
    YidsRom* yidsRom;

    QComboBox* musicIdDropdown;

    QComboBox* entranceAnim;
    QComboBox* entranceScreen;

    QComboBox* exitTypeCombo;
    QComboBox* exitMapTarget;
    QComboBox* exitEntranceTarget;

    QSpinBox* exitX;
    QSpinBox* exitY;
    QSpinBox* entranceX;
    QSpinBox* entranceY;

    QPushButton* entrancePlus;
    QPushButton* entranceMinus;
    QPushButton* exitPlus;
    QPushButton* exitMinus;

    bool detectChanges = false;

    void musicIdChanged(const QString text);

    void entranceAnimChanged(const QString text);
    void entranceScreenChanged(const QString text);

    void exitTypeChanged(const QString text);
    void targetMapChanged(const QString text);
    void targetEntranceChanged(const QString text);

    void refreshTargetEntrances(int whichMapToGetEntrancesFrom);
    void refreshEntranceList();

    void selectedEntranceItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void selectedExitItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void entrancePositionChangedX();
    void entrancePositionChangedY();

    void exitPositionChangedX();
    void exitPositionChangedY();

    void entrancePlusClicked();
    void entranceMinusClick();
    void exitPlusClicked();
    void exitMinusClicked();
signals:
    void portalsUpdated();
};