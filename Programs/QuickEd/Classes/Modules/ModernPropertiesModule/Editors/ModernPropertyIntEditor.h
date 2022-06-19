#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

class QSpinBox;

class ModernPropertyIntEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyIntEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyIntEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnValueChanged(int value);

    QSpinBox* spinBox = nullptr;
};
