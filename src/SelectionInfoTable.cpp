#include "SelectionInfoTable.h"

#include "yidsrom.h"
#include "LevelObject.h"
#include "StateCommands.h"

#include <sstream>
#include <string>

#include <QtCore>
#include <QWidget>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

SelectionInfoTable::SelectionInfoTable(QWidget* parent, YidsRom* rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setColumnCount(2);
    this->setRowCount(8);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();

    int i = 0;
    this->setText(0,i++,"ID",false);
    this->setText(0,i++,"UUID",false);
    this->setText(0,i++,"Name",false);
    this->setText(0,i++,"Description",false);
    this->setText(0,i++,"X Position",false);
    this->setText(0,i++,"Y Position",false);
    this->setText(0,i++,"Settings Length",false);
    this->setText(0,i++,"Settings",false);

    connect(this,&QTableWidget::cellDoubleClicked,this,&SelectionInfoTable::cellDoubleClicked);
    connect(this,&QTableWidget::cellChanged,this,&SelectionInfoTable::cellChanged);
}

void SelectionInfoTable::setText(int x, int y, std::string text, bool editable) {
    auto potentialExisting = this->item(y,x);
    if (potentialExisting == nullptr) {
        // Nothing is here, so lets make a new one and set it!
        QTableWidgetItem *newItem = new QTableWidgetItem();
        newItem->setText(tr(text.c_str()));
        if (editable) {
            newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        } else {
            newItem->setFlags(Qt::ItemIsEnabled);
        }
        this->setItem(y,x,newItem);
    } else {
        potentialExisting->setText(tr(text.c_str()));
        if (editable) {
            potentialExisting->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        } else {
            potentialExisting->setFlags(Qt::ItemIsEnabled);
        }
    }
}

void SelectionInfoTable::updateWithLevelObject(LevelObject *lo) {
    this->spritePointer = lo;
    auto textMetadata = this->yidsRom->getSpriteMetadata(lo->objectId);
    std::stringstream s0;
    s0 << "0x" << std::hex << std::setw(2) << std::setfill('0') << lo->objectId;
    this->setText(1,0,s0.str(),false);
    std::stringstream s1;
    s1 << "0x" << std::hex << std::setw(2) << std::setfill('0') << lo->uuid;
    this->setText(1,1,s1.str(),false);
    this->setText(1,2,textMetadata.name,false);
    this->setText(1,3,textMetadata.info,false);
    std::stringstream sX;
    sX << "0x" << std::hex << std::setw(2) << std::setfill('0') << lo->xPosition;
    std::stringstream sY;
    sY << "0x" << std::hex << std::setw(2) << std::setfill('0') << lo->yPosition;
    this->setText(1,SelectionInfoTable::XPOSROW,sX.str(),true);
    this->setText(1,SelectionInfoTable::YPOSROW,sY.str(),true);
    // Settings
    std::stringstream sSettingsLength;
    sSettingsLength << "0x" << std::hex << std::setw(2) << std::setfill('0') << lo->settingsLength;
    this->setText(1,6,sSettingsLength.str(),false);
    std::stringstream ssSettings;
    if (lo->settingsLength != lo->settings.size()) {
        YUtils::printDebug("Settings length value and settings size not matching",DebugType::ERROR);
    }
    for (int j = 0; j < lo->settingsLength; j++) {
        ssSettings << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)lo->settings.at(j) << " ";
    }
    this->setText(1,SelectionInfoTable::SETTINGSDATAROW,ssSettings.str(),true);
}

void SelectionInfoTable::cellDoubleClicked(int row, int column) {
    auto cell = this->item(row,column);
    if (cell == nullptr) {
        YUtils::printDebug("Cell double clicked is null",DebugType::WARNING);
        return;
    }
    if (cell->flags() & Qt::ItemIsEditable) {
        this->cellBeingEdited = cell;
    }
}

void SelectionInfoTable::cellChanged(int row, int column) {
    auto item = this->item(row,column);
    if (item == nullptr) {
        YUtils::printDebug("cellChanged given null item",DebugType::WARNING);
        return;
    }
    if (this->cellBeingEdited == nullptr) {
        // Fail silently, it will output a LOT
        return;
    }
    if (item == this->cellBeingEdited) {
        this->cellBeingEdited = nullptr;
        if (row == SelectionInfoTable::YPOSROW) {
            bool ok;
            uint32_t value = item->text().toUInt(&ok,16);
            if (ok == false) {
                YUtils::printDebug("Base 16 value parse failed");
                return;
            }
            std::cout << "Changing Y POS to " << std::hex << value << std::endl;
            this->spritePointer->yPosition = value;
        } else if (row == SelectionInfoTable::XPOSROW) {
            bool ok;
            uint32_t value = item->text().toUInt(&ok,16);
            if (ok == false) {
                YUtils::printDebug("Base 16 value parse failed");
                return;
            }
            std::cout << "Changing X POS to " << std::hex << value << std::endl;
            this->spritePointer->xPosition = value;
        } else if (row == SelectionInfoTable::SETTINGSDATAROW) {
            auto str = item->text().toStdString();
            auto hexVector = this->hexStringToByteVector(str);
            if (hexVector.size() == 0) {
                YUtils::popupAlert("Error parsing settings data");
                return;
            }
            if (hexVector.size() != this->spritePointer->settingsLength) {
                YUtils::popupAlert("Invalid settings length");
                return;
            }
            SpriteSettingsChangeCommand *modCmd = new SpriteSettingsChangeCommand(this->spritePointer->uuid,hexVector,this->yidsRom);
            emit this->pushCommandToUndoStack(modCmd);
            //this->spritePointer->settings = hexVector;
        }
        std::cout << "Emitting updateMainWindow" << std::endl;
        emit this->updateMainWindow(this->spritePointer);
    }
}

std::vector<uint8_t> SelectionInfoTable::hexStringToByteVector(std::string hexString) {
    // String is auto trimmed to have zero spaces in front and 1 space in back
    std::vector<uint8_t> result;
    if (hexString.empty()) {
        return result;
    }
    if (!std::isspace(hexString.at(hexString.size()-1))) {
        hexString = hexString.append(" ");
    }
    uint stringIndex = 0;
    while (stringIndex < hexString.size()) {
        auto spaceIndex = hexString.find_first_of(' ',stringIndex);
        auto length = spaceIndex - stringIndex;
        auto charString = hexString.substr(stringIndex,length);
        stringIndex += length+1;
        //std::cout << "'" << charString << "'" << std::endl;
        uint64_t toUnsigned = std::strtoul(charString.c_str(), nullptr, 16);
        if (toUnsigned > 0xff) {
            std::stringstream ssTooBig;
            ssTooBig << "Hex value bigger than 0xff: " << std::hex << toUnsigned;
            YUtils::popupAlert(ssTooBig.str());
            return std::vector<uint8_t>();
        }
        result.push_back((uint8_t)toUnsigned);
    }
    return result;
}
