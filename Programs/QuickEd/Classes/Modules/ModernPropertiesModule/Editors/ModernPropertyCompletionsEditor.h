#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

#include "UI/Properties/CompletionsProvider.h"

class QuickEdComboBox;

class ModernPropertyCompletionsEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyCompletionsEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property, std::unique_ptr<CompletionsProvider> completionsProvider, bool isEditable);
    ~ModernPropertyCompletionsEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnActivated(const QString& value);
    void OnShowPopup();

    QuickEdComboBox* comboBox = nullptr;
    std::unique_ptr<CompletionsProvider> completionsProvider;
};
