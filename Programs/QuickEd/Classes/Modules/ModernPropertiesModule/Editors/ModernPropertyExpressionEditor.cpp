#include "Modules/ModernPropertiesModule/Editors/ModernPropertyExpressionEditor.h"

#include "Model/ControlProperties/ValueProperty.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Modules/DataBindingInspectorModule/DataBindingInspectorModel.h"
#include "Utils/QtDavaConvertion.h"

#include <Base/Any.h>
#include <Engine/Engine.h>
#include <UI/DataBinding/UIDataBindingSystem.h>
#include <UI/DataBinding/Private/UIDataModel.h>
#include <UI/Formula/FormulaContext.h>
#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>

#include <TArc/Utils/Utils.h>

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QTreeView>

ModernPropertyExpressionEditor::ModernPropertyExpressionEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyDefaultEditor(context, property)
{
    using namespace DAVA;

    line = new QLineEdit();
    line->setProperty("property", true);
    QObject::connect(line, &QLineEdit::editingFinished, this, &ModernPropertyExpressionEditor::OnEditingFinished);

    bindButton = new QToolButton();
    bindButton->setText("...");
    bindButton->setProperty("property", true);
    QObject::connect(bindButton, &QToolButton::clicked, this, &ModernPropertyExpressionEditor::OnButtonClicked);

    layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(4);

    layout->addWidget(line);
    layout->addWidget(bindButton);

    OnPropertyChanged();
}

ModernPropertyExpressionEditor::~ModernPropertyExpressionEditor()
{
    delete line;
    delete bindButton;
    delete layout;
}

void ModernPropertyExpressionEditor::AddToGrid(QGridLayout* grid, int row, int col, int colSpan)
{
    grid->addWidget(propertyName, row, col);
    grid->addLayout(layout, row, col + 1, 1, colSpan);
}

void ModernPropertyExpressionEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    QSignalBlocker blockSignalsLine(line);

    QString stringValue = StringToQString(property->GetValue().Cast<DAVA::String>());
    line->setText(DAVA::UnescapeString(stringValue));
    line->setDisabled(property->IsReadOnly());

    ApplyStyleToWidget(line);
}

void ModernPropertyExpressionEditor::OnEditingFinished()
{
    if (line->isModified())
    {
        ApplyChanges();
    }
}

void ModernPropertyExpressionEditor::OnCurrentIndexChanged(int index)
{
    ApplyChanges();
}

void ModernPropertyExpressionEditor::OnButtonClicked()
{
    using namespace DAVA;

    DataBindingInspectorModel* model = new DataBindingInspectorModel(true, GetParentWidget());
    RootProperty* root = dynamic_cast<RootProperty*>(property->GetRootProperty());
    UIControl* control = nullptr;
    if (root)
    {
        ControlNode* controlNode = root->GetControlNode();
        control = controlNode->GetControl();
    }

    if (control)
    {
        std::shared_ptr<FormulaContext> context = DAVA::GetEngineContext()->uiControlSystem->GetSystem<UIDataBindingSystem>()->GetFormulaContext(control);
        model->UpdateModel(context.get());
    }

    QString expr = bindAction->property("bindingExpr").toString();
    int32 bindingMode = bindAction->property("bindingMode").toInt();
    QVariant modelVariant = bindAction->property("model");

    QDialog* dialog = new QDialog(GetParentWidget());
    dialog->resize(600, 600);
    dialog->setWindowTitle("Bind Data");
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    dialog->setLayout(layout);
    dialog->setModal(true);

    QTreeView* treeView = new QTreeView();
    treeView->setModel(model);
    treeView->expandAll();
    treeView->resizeColumnToContents(0);

    dialog->layout()->addWidget(treeView);

    QDialogButtonBox* buttons = new QDialogButtonBox(Qt::Horizontal);
    buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dialog->layout()->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));

    if (dialog->exec() == QDialog::Accepted)
    {
        QModelIndexList indices = treeView->selectionModel()->selectedIndexes();
        QString exprStr = "";
        if (!indices.empty())
        {
            exprStr = model->data(indices.first(), DataBindingInspectorModel::PATH_DATA).toString();
        }

        ChangeProperty(QStringToString(exprStr));
    }
}

void ModernPropertyExpressionEditor::ApplyChanges()
{
    QString expr = DAVA::EscapeString(line->text());
    ChangeProperty(QStringToString(expr));
}
