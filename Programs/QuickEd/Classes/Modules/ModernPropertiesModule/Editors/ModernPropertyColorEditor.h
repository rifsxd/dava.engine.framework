#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

class QPushButton;
class QLineEdit;

class ModernPropertyColorEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyColorEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyColorEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnButtonClicked();
    void OnEditingFinished();

    QPushButton* button = nullptr;
    QLineEdit* line = nullptr;
    QHBoxLayout* layout = nullptr;
};
