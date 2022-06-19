#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

class QLineEdit;

class ModernPropertyFloatEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyFloatEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyFloatEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnEditingFinished();

    QLineEdit* line = nullptr;
};
