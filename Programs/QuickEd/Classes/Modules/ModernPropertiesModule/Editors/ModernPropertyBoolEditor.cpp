#include "Modules/ModernPropertiesModule/Editors/ModernPropertyBoolEditor.h"

#include "Model/ControlProperties/ValueProperty.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QCheckBox>
#include <QStyle>
#include <QFormLayout>
#include <QMenu>

ModernPropertyBoolEditor::ModernPropertyBoolEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyEditor(context, property)
{
    checkBox = new QCheckBox(QObject::tr(property->GetDisplayName().c_str()), context->GetParent());
    checkBox->setProperty("property", true);
    checkBox->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    checkBox->setContentsMargins(0, 0, 0, 0);

    QObject::connect(checkBox, &QCheckBox::toggled, this, &ModernPropertyBoolEditor::OnCheckBoxToggled);
    QObject::connect(checkBox, &QCheckBox::customContextMenuRequested, this, &ModernPropertyBoolEditor::OnCustomContextMenuRequested);

    OnPropertyChanged();
}

ModernPropertyBoolEditor::~ModernPropertyBoolEditor()
{
    delete checkBox;
}

void ModernPropertyBoolEditor::AddToGrid(QGridLayout* layout, int row, int col, int colSpan)
{
    layout->addWidget(checkBox, row, col, 1, colSpan);
}

void ModernPropertyBoolEditor::OnPropertyChanged()
{
    ModernPropertyEditor::OnPropertyChanged();

    QSignalBlocker blocker(checkBox);
    checkBox->setChecked(property->GetValue().Cast<bool>());

    checkBox->setDisabled(property->IsReadOnly());

    ApplyStyleToWidget(checkBox);
}

void ModernPropertyBoolEditor::OnCheckBoxToggled(bool checked)
{
    ChangeProperty(DAVA::Any(checked));
}

void ModernPropertyBoolEditor::OnCustomContextMenuRequested(const QPoint& pos)
{
    ShowActionsMenu(checkBox->mapToGlobal(pos));
}
