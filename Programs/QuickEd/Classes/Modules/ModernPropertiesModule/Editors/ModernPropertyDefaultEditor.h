#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyEditor.h"

class QHBoxLayout;

class ModernPropertyDefaultEditor : public ModernPropertyEditor
{
    Q_OBJECT

public:
    ModernPropertyDefaultEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyDefaultEditor() override;

protected:
    void OnPropertyChanged() override;

private:
    void OnCustomContextMenuRequested(const QPoint& pos);
};
