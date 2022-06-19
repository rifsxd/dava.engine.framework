#include "Modules/ModernPropertiesModule/Editors/QuickEdComboBox.h"

QuickEdComboBox::QuickEdComboBox(QWidget* parent)
    : QComboBox(parent)
{
}

void QuickEdComboBox::showPopup()
{
    emit onShowPopup();
    QComboBox::showPopup();
}
