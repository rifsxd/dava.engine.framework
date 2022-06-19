#include "Modules/ModernPropertiesModule/Editors/ModernPropertyStringEditor.h"

#include "Model/ControlProperties/ValueProperty.h"
#include "Utils/QtDavaConvertion.h"

#include <Base/Any.h>

#include <TArc/Utils/Utils.h>

#include <QLineEdit>

ModernPropertyStringEditor::ModernPropertyStringEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyDefaultEditor(context, property)
{
    using namespace DAVA;

    line = new QLineEdit();
    line->setProperty("property", true);
    QObject::connect(line, &QLineEdit::editingFinished, this, &ModernPropertyStringEditor::OnEditingFinished);

    OnPropertyChanged();
}

ModernPropertyStringEditor::~ModernPropertyStringEditor()
{
    delete line;
}

void ModernPropertyStringEditor::AddToGrid(QGridLayout* layout, int row, int col, int colSpan)
{
    layout->addWidget(propertyName, row, col);
    layout->addWidget(line, row, col + 1, 1, colSpan);
}

void ModernPropertyStringEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    QSignalBlocker blockSignals(line);

    QString stringValue = StringToQString(property->GetValue().Cast<DAVA::String>());
    line->setText(DAVA::UnescapeString(stringValue));
    line->setDisabled(property->IsReadOnly());

    ApplyStyleToWidget(line);
}

void ModernPropertyStringEditor::OnEditingFinished()
{
    if (line->isModified())
    {
        QString stringValue = DAVA::EscapeString(line->text());
        ChangeProperty(DAVA::Any(QStringToString(stringValue)));
    }
}
