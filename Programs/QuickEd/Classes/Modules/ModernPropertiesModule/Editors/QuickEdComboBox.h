#pragma once

#include <QComboBox>

class QuickEdComboBox : public QComboBox
{
    Q_OBJECT
public:
    QuickEdComboBox(QWidget* parent);

    void showPopup() override;

    Q_SIGNALS :
    void
    onShowPopup();
};
