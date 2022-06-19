#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

class QLineEdit;
class QToolButton;

class ModernPropertyExpressionEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyExpressionEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyExpressionEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnEditingFinished();
    void OnCurrentIndexChanged(int index);
    void OnButtonClicked();

    void ApplyChanges();

    QLineEdit* line = nullptr;
    QToolButton* bindButton = nullptr;

    QHBoxLayout* layout = nullptr;
};
