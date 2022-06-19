#include "Modules/ModernPropertiesModule/Editors/ModernPropertyColorEditor.h"

#include "Model/ControlProperties/ValueProperty.h"
#include "Utils/QtDavaConvertion.h"

#include <Base/Any.h>

#include <TArc/Utils/Utils.h>

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QColorDialog>
#include <QFormLayout>

ModernPropertyColorEditor::ModernPropertyColorEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyDefaultEditor(context, property)
{
    using namespace DAVA;

    button = new QPushButton(GetParentWidget());
    button->setProperty("property", true);
    QObject::connect(button, &QPushButton::clicked, this, &ModernPropertyColorEditor::OnButtonClicked);

    line = new QLineEdit(GetParentWidget());
    line->setProperty("property", true);
    QObject::connect(line, &QLineEdit::editingFinished, this, &ModernPropertyColorEditor::OnEditingFinished);

    layout = new QHBoxLayout();
    layout->addWidget(button);
    layout->addWidget(line);

    OnPropertyChanged();
}

ModernPropertyColorEditor::~ModernPropertyColorEditor()
{
    delete button;
    delete line;
    delete layout;
}

void ModernPropertyColorEditor::AddToGrid(QGridLayout* grid, int row, int col, int colSpan)
{
    grid->addWidget(propertyName, row, col);
    grid->addLayout(layout, row, col + 1, 1, colSpan);
}

void ModernPropertyColorEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    button->setDisabled(property->IsReadOnly());
    line->setDisabled(property->IsReadOnly());

    QSignalBlocker blockButtonSignals(button);
    QSignalBlocker blockLineSignals(line);

    QPalette p(GetParentWidget()->palette());
    QColor color = DAVA::ColorToQColor(property->GetValue().Get<DAVA::Color>());
    p.setColor(QPalette::Button, color);
    button->setAutoFillBackground(true);
    button->setPalette(p);

    line->setText(QColorToHex(color));
    ApplyStyleToWidget(line);
}

void ModernPropertyColorEditor::OnButtonClicked()
{
    QColorDialog dlg(GetParentWidget());

    dlg.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    QColor color = DAVA::ColorToQColor(property->GetValue().Get<DAVA::Color>());
    dlg.setCurrentColor(color);

    if (dlg.exec() == QDialog::Accepted)
    {
        ChangeProperty(DAVA::QColorToColor(dlg.selectedColor()));
    }
}

void ModernPropertyColorEditor::OnEditingFinished()
{
    if (line->isModified())
    {
        QColor color = HexToQColor(line->text());
        ChangeProperty(DAVA::QColorToColor(color));
    }
}
