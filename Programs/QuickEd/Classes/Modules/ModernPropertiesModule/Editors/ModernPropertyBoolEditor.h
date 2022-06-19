#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyEditor.h"

class QCheckBox;

class ModernPropertyBoolEditor : public ModernPropertyEditor
{
    Q_OBJECT

public:
    ModernPropertyBoolEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyBoolEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnCheckBoxToggled(bool checked);
    void OnCustomContextMenuRequested(const QPoint& pos);

    QCheckBox* checkBox = nullptr;
};
