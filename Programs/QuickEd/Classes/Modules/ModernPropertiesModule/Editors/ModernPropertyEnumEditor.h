#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

#include <QComboBox>

class ModernPropertyEnumEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyEnumEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyEnumEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnCurrentIndexChanged(int index);

    QComboBox* comboBox = nullptr;
};
