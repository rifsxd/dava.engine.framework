#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

class QPlainTextEdit;

class ModernPropertyMultilineEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyMultilineEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyMultilineEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnTextChanged();
    bool eventFilter(QObject* obj, QEvent* event) override;

    QPlainTextEdit* text = nullptr;
};
