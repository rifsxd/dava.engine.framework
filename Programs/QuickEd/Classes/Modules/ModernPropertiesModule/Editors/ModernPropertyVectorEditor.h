#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

class QLineEdit;

class ModernPropertyVectorEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    enum eComponents
    {
        COMPONENTS_XY = 2,
        COMPONENTS_XYZ = 3,
        COMPONENTS_XYZW = 4,

        COMPONENTS_MAX = 4
    };

    ModernPropertyVectorEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property, eComponents components);
    ~ModernPropertyVectorEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnEditingFinished();
    void ApplyChangedProperties();

    struct NamedEdit
    {
        QLabel* caption = nullptr;
        QLineEdit* line = nullptr;
    };
    std::array<NamedEdit, COMPONENTS_MAX> editors;
    eComponents components = COMPONENTS_XY;

    bool waitTimer = false;

    QHBoxLayout* layout = nullptr;

    static const char* COMPONENT_NAMES[COMPONENTS_MAX];
};
