#include "Modules/ModernPropertiesModule/Editors/ModernPropertyFloatEditor.h"

#include "Modules/ModernPropertiesModule/Editors/QuickEdDoubleValidator.h"
#include "Model/ControlProperties/ValueProperty.h"

#include <Base/Any.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QLineEdit>

ModernPropertyFloatEditor::ModernPropertyFloatEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyDefaultEditor(context, property)
{
    using namespace DAVA;

    line = new QLineEdit(GetParentWidget());
    line->setProperty("property", true);
    line->setValidator(new QuickEdDoubleValidator());
    QObject::connect(line, &QLineEdit::editingFinished, this, &ModernPropertyFloatEditor::OnEditingFinished);

    OnPropertyChanged();
}

ModernPropertyFloatEditor::~ModernPropertyFloatEditor()
{
    delete line;
}

void ModernPropertyFloatEditor::AddToGrid(QGridLayout* layout, int row, int col, int colSpan)
{
    layout->addWidget(propertyName, row, col);
    layout->addWidget(line, row, col + 1, 1, colSpan);
}

void ModernPropertyFloatEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    line->setDisabled(property->IsReadOnly());

    QSignalBlocker blockSignals(line);

    DAVA::float32 value = property->GetValue().Cast<DAVA::float32>();
    line->setText(QString::asprintf("%g", value));

    ApplyStyleToWidget(line);
}

void ModernPropertyFloatEditor::OnEditingFinished()
{
    if (line->isModified())
    {
        DAVA::float32 value = line->text().replace(",", ".").toFloat();
        ChangeProperty(value);
    }
}
