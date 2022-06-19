#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

class QLineEdit;
class QComboBox;
class QToolButton;

class ModernPropertyBindingEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyBindingEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyBindingEditor() override;

    bool IsBindingEditor() const override;
    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnEditingFinished();
    void OnCurrentIndexChanged(int index);
    void OnButtonClicked();

    void ApplyChanges();

    QComboBox* updateModeComboBox = nullptr;
    QLineEdit* line = nullptr;
    QToolButton* bindButton = nullptr;

    QHBoxLayout* layout = nullptr;
};
