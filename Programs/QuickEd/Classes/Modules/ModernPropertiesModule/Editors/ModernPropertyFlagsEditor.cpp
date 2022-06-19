#include "Modules/ModernPropertiesModule/Editors/ModernPropertyFlagsEditor.h"

#include "Model/ControlProperties/ValueProperty.h"
#include "Utils/QtDavaConvertion.h"

#include <Base/Any.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QAction>

ModernPropertyFlagsEditor::ModernPropertyFlagsEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyDefaultEditor(context, property)
{
    using namespace DAVA;

    vBoxLayout = new QVBoxLayout();

    const EnumMap* enumMap = property->GetEnumMap();
    DVASSERT(enumMap);

    propertyName->setAlignment(Qt::AlignTop);

    for (size_t i = 0; i < enumMap->GetCount(); ++i)
    {
        int value = 0;
        if (enumMap->GetValue(i, value))
        {
            QCheckBox* checkBox = new QCheckBox(QObject::tr(enumMap->ToString(value)), GetParentWidget());
            checkBox->setProperty("property", true);
            checkBox->setProperty("value", QVariant(value));
            checkBoxes.push_back(checkBox);
            QObject::connect(checkBox, &QCheckBox::toggled, this, &ModernPropertyFlagsEditor::OnCheckBoxToggled);
            vBoxLayout->addWidget(checkBox);
        }
    }

    OnPropertyChanged();
}

ModernPropertyFlagsEditor::~ModernPropertyFlagsEditor()
{
    for (QCheckBox* checkBox : checkBoxes)
    {
        delete checkBox;
    }
    checkBoxes.clear();
    delete vBoxLayout;
}

void ModernPropertyFlagsEditor::AddToGrid(QGridLayout* layout, int row, int col, int colSpan)
{
    layout->addWidget(propertyName, row, col);
    layout->addLayout(vBoxLayout, row, col + 1, 1, colSpan);
}

void ModernPropertyFlagsEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    DAVA::int32 value = property->GetValue().Cast<DAVA::int32>();
    for (QCheckBox* checkBox : checkBoxes)
    {
        QSignalBlocker blocker(checkBox);

        checkBox->setDisabled(property->IsReadOnly());

        DAVA::int32 checkBoxValue = checkBox->property("value").toInt();
        checkBox->setChecked((value & checkBoxValue) != 0);
        ApplyStyleToWidget(checkBox);
    }
}

void ModernPropertyFlagsEditor::OnCheckBoxToggled(bool checked)
{
    DAVA::int32 checkBoxValue = sender()->property("value").toInt();
    DAVA::int32 value = property->GetValue().Cast<DAVA::int32>();
    if (checked)
    {
        value |= checkBoxValue;
    }
    else
    {
        value &= ~checkBoxValue;
    }
    ChangeProperty(value);
}
