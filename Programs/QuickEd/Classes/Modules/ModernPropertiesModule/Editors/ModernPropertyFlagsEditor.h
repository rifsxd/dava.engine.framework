#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

#include <QVector>

class QCheckBox;
class QVBoxLayout;

class ModernPropertyFlagsEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyFlagsEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyFlagsEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnCheckBoxToggled(bool checked);

    QVector<QCheckBox*> checkBoxes;
    QVBoxLayout* vBoxLayout = nullptr;
};
