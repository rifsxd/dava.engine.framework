#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

#include "Model/ControlProperties/ValueProperty.h"

#include <QLabel>
#include <QAction>

ModernPropertyDefaultEditor::ModernPropertyDefaultEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyEditor(context, property)
{
    propertyName = new QLabel(GetParentWidget());
    propertyName->setProperty("property", true);
    propertyName->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    propertyName->setText(QObject::tr(property->GetDisplayName().c_str()));
    connect(propertyName, &QLabel::customContextMenuRequested, this, &ModernPropertyDefaultEditor::OnCustomContextMenuRequested);
}

ModernPropertyDefaultEditor::~ModernPropertyDefaultEditor()
{
    delete propertyName;
    propertyName = nullptr;
}

void ModernPropertyDefaultEditor::OnPropertyChanged()
{
    ModernPropertyEditor::OnPropertyChanged();

    ApplyStyleToWidget(propertyName);

    propertyName->setDisabled(property->IsReadOnly());
}

void ModernPropertyDefaultEditor::OnCustomContextMenuRequested(const QPoint& pos)
{
    ShowActionsMenu(propertyName->mapToGlobal(pos));
}
