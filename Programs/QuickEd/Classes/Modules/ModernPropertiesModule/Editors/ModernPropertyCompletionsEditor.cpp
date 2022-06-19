#include "Modules/ModernPropertiesModule/Editors/ModernPropertyCompletionsEditor.h"

#include "Modules/ModernPropertiesModule/Editors/QuickEdComboBox.h"

#include "Model/ControlProperties/ValueProperty.h"
#include "Utils/QtDavaConvertion.h"

#include <Base/Any.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QAction>

ModernPropertyCompletionsEditor::ModernPropertyCompletionsEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property, std::unique_ptr<CompletionsProvider> completionsProvider_, bool isEditable)
    : ModernPropertyDefaultEditor(context, property)
    , completionsProvider(std::move(completionsProvider_))
{
    using namespace DAVA;

    comboBox = new QuickEdComboBox(GetParentWidget());
    comboBox->setProperty("property", true);
    comboBox->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
    comboBox->setEditable(isEditable);

    comboBox->clear();
    comboBox->addItem("");
    comboBox->addItems(completionsProvider->GetCompletions(property));

    connect(comboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated), this, &ModernPropertyCompletionsEditor::OnActivated);
    connect(comboBox, &QuickEdComboBox::onShowPopup, this, &ModernPropertyCompletionsEditor::OnShowPopup);

    OnPropertyChanged();
}

ModernPropertyCompletionsEditor::~ModernPropertyCompletionsEditor()
{
    delete comboBox;
}

void ModernPropertyCompletionsEditor::AddToGrid(QGridLayout* layout, int row, int col, int colSpan)
{
    layout->addWidget(propertyName, row, col);
    layout->addWidget(comboBox, row, col + 1, 1, colSpan);
}

void ModernPropertyCompletionsEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    comboBox->setDisabled(property->IsReadOnly());

    QSignalBlocker blockSignals(comboBox);

    int index = comboBox->findText(property->GetValue().Cast<QString>());
    comboBox->setCurrentIndex(index);
    comboBox->setEditText(property->GetValue().Cast<QString>());

    ApplyStyleToWidget(comboBox);
}

void ModernPropertyCompletionsEditor::OnActivated(const QString& value)
{
    ChangeProperty(value.toStdString());
}

void ModernPropertyCompletionsEditor::OnShowPopup()
{
    comboBox->clear();
    comboBox->addItem("");
    comboBox->addItems(completionsProvider->GetCompletions(property.Get()));
}
