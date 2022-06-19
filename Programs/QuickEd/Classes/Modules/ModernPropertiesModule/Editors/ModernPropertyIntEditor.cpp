#include "Modules/ModernPropertiesModule/Editors/ModernPropertyIntEditor.h"

#include "Model/ControlProperties/ValueProperty.h"

#include <Base/Any.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QSpinBox>

ModernPropertyIntEditor::ModernPropertyIntEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyDefaultEditor(context, property)
{
    using namespace DAVA;

    spinBox = new QSpinBox(GetParentWidget());
    spinBox->setProperty("property", true);
    spinBox->setMinimum(-99999);
    spinBox->setMaximum(99999);
    spinBox->setFocusPolicy(Qt::StrongFocus);
    spinBox->installEventFilter(this);

    DAVA::Any value = property->GetValue();
    if (value.CanGet<DAVA::int8>() || value.CanGet<DAVA::int16>() || value.CanGet<DAVA::int32>() || value.CanGet<DAVA::int64>())
    {
    }
    else if (value.CanGet<DAVA::uint8>() || value.CanGet<DAVA::uint16>() || value.CanGet<DAVA::uint32>() || value.CanGet<DAVA::uint64>())
    {
        spinBox->setMinimum(0);
    }
    else
    {
        DVASSERT(false);
    }

    QObject::connect(spinBox, static_cast<void (QSpinBox::*)(int value)>(&QSpinBox::valueChanged), this, &ModernPropertyIntEditor::OnValueChanged);

    OnPropertyChanged();
}

ModernPropertyIntEditor::~ModernPropertyIntEditor()
{
    delete spinBox;
}

void ModernPropertyIntEditor::AddToGrid(QGridLayout* layout, int row, int col, int colSpan)
{
    layout->addWidget(propertyName, row, col);
    layout->addWidget(spinBox, row, col + 1, 1, colSpan);
}

void ModernPropertyIntEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    spinBox->setDisabled(property->IsReadOnly());

    QSignalBlocker blockSignals(spinBox);
    spinBox->setValue(property->GetValue().Cast<DAVA::int32>());
    ApplyStyleToWidget(spinBox);
}

void ModernPropertyIntEditor::OnValueChanged(int value)
{
    ChangeProperty(DAVA::Any(value));
}
