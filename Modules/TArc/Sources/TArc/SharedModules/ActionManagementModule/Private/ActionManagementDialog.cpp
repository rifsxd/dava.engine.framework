#include "TArc/SharedModules/ActionManagementModule/Private/ActionManagementDialog.h"
#include "TArc/SharedModules/ActionManagementModule/Private/ShortcutsModel.h"
#include "TArc/WindowSubSystem/Private/UIManager.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/ComboBox.h"
#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/ReflectedButton.h"
#include "TArc/Qt/QtSize.h"
#include "TArc/Qt/QtIcon.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Base/GlobalEnum.h>

#include <QAction>
#include <QInputDialog>
#include <QTreeView>
#include <QGridLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QKeyEvent>
#include <QKeySequenceEdit>

ENUM_DECLARE(Qt::ShortcutContext)
{
    ENUM_ADD_DESCR(Qt::WidgetShortcut, "Widget");
    ENUM_ADD_DESCR(Qt::WidgetWithChildrenShortcut, "WidgetWithChildren");
    ENUM_ADD_DESCR(Qt::WindowShortcut, "Window");
    ENUM_ADD_DESCR(Qt::ApplicationShortcut, "Application");
}

namespace DAVA
{
namespace ActionManagementDialogDetail
{
bool HasModif(int key)
{
    return (key & Qt::CTRL) == Qt::CTRL ||
    (key & Qt::ALT) == Qt::ALT ||
    (key & Qt::SHIFT) == Qt::SHIFT ||
    (key & Qt::META) == Qt::META;
}

int ExtractKey(int key)
{
    return (key & (~(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::META)));
}

int ExtractModif(int key)
{
    int extractedKey = ExtractKey(key);
    return key ^ extractedKey;
}

bool IsFunctionalKey(int key)
{
    return key >= Qt::Key_F1 && key <= Qt::Key_F12;
}

bool IsDigit(int key)
{
    return key >= Qt::Key_0 && key <= Qt::Key_9;
}

bool IsLetter(int key)
{
    return key >= Qt::Key_A && key <= Qt::Key_Z;
}

bool IsSpecialAllowedSymbol(int key)
{
    if (key >= Qt::Key_Colon && key <= Qt::Key_At)
    {
        return true;
    }

    if (key >= Qt::Key_BracketLeft && key <= Qt::Key_BraceRight)
    {
        return true;
    }

    if (key == Qt::Key_BraceLeft || key == Qt::Key_BraceRight)
    {
        return true;
    }

    return false;
}
}

ActionManagementDialog::ActionManagementDialog(ContextAccessor* accessor_, UIManager* ui_)
    : accessor(accessor_)
    , ui(ui_)
{
    setWindowTitle("Key Bindings");
    setWindowFlags(Qt::Tool);

    Reflection model = Reflection::Create(ReflectedObject(this));
    QtVBoxLayout* layout = new QtVBoxLayout(this);

    QtHBoxLayout* schemesLayout = new QtHBoxLayout();
    {
        ComboBox::Params params(accessor, ui, mainWindowKey);
        params.fields[ComboBox::Fields::Value] = "currentScheme";
        params.fields[ComboBox::Fields::Enumerator] = "schemes";
        schemesLayout->AddControl(new ComboBox(params, accessor, model, this));
    }
    {
        ReflectedButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedButton::Fields::Icon] = "addIcon";
        params.fields[ReflectedButton::Fields::IconSize] = "iconSize";
        params.fields[ReflectedButton::Fields::Tooltip] = "addToolTip";
        params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
        params.fields[ReflectedButton::Fields::Clicked] = "AddKeyBindingsScheme";
        schemesLayout->AddControl(new ReflectedButton(params, accessor, model, this));
    }
    {
        ReflectedButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedButton::Fields::Icon] = "removeIcon";
        params.fields[ReflectedButton::Fields::IconSize] = "iconSize";
        params.fields[ReflectedButton::Fields::Tooltip] = "removeToolTip";
        params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
        params.fields[ReflectedButton::Fields::Clicked] = "RemoveKeyBindingsScheme";
        params.fields[ReflectedButton::Fields::Enabled] = "removeButtonEnabled";
        schemesLayout->AddControl(new ReflectedButton(params, accessor, model, this));
    }
    {
        ReflectedButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedButton::Fields::Icon] = "importIcon";
        params.fields[ReflectedButton::Fields::IconSize] = "iconSize";
        params.fields[ReflectedButton::Fields::Tooltip] = "importToolTip";
        params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
        params.fields[ReflectedButton::Fields::Clicked] = "ImportKeyBindingsScheme";
        schemesLayout->AddControl(new ReflectedButton(params, accessor, model, this));
    }
    {
        ReflectedButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedButton::Fields::Icon] = "exportIcon";
        params.fields[ReflectedButton::Fields::IconSize] = "iconSize";
        params.fields[ReflectedButton::Fields::Tooltip] = "exportToolTip";
        params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
        params.fields[ReflectedButton::Fields::Clicked] = "ExportKeyBindingsScheme";
        schemesLayout->AddControl(new ReflectedButton(params, accessor, model, this));
    }
    layout->addLayout(schemesLayout);

    treeView = new QTreeView(this);
    shortcutsModel = new ShortcutsModel();
    treeView->setModel(shortcutsModel);
    layout->addWidget(treeView);

    QGridLayout* currentSequencesLayout = new QGridLayout();
    {
        ComboBox::Params params(accessor, ui, mainWindowKey);
        params.fields[ComboBox::Fields::Value] = "currentSequence";
        params.fields[ComboBox::Fields::Enumerator] = "sequences";
        params.fields[ComboBox::Fields::MultipleValueText] = "emptySequenceText";
        params.fields[ComboBox::Fields::IsReadOnly] = "sequencesReadOnly";
        ComboBox* comboBox = new ComboBox(params, accessor, model, this);
        currentSequencesLayout->addWidget(comboBox->ToWidgetCast(), 0, 0, 1, 2);
    }
    {
        ReflectedPushButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedPushButton::Fields::Clicked] = "removeSequence";
        params.fields[ReflectedPushButton::Fields::Text] = "removeSequenceText";
        params.fields[ReflectedPushButton::Fields::Enabled] = "removeSequenceEnabled";
        ReflectedPushButton* button = new ReflectedPushButton(params, accessor, model, this);
        currentSequencesLayout->addWidget(button->ToWidgetCast(), 0, 2, 1, 1);
    }
    {
        ComboBox::Params params(accessor, ui, mainWindowKey);
        params.fields[ComboBox::Fields::Value] = "currentContext";
        params.fields[ComboBox::Fields::IsReadOnly] = "contextReadOnly";
        ComboBox* comboBox = new ComboBox(params, accessor, model, this);
        currentSequencesLayout->addWidget(comboBox->ToWidgetCast(), 1, 0, 1, 1);
    }
    {
        sequenceEdit = new QKeySequenceEdit(this);
        connections.AddConnection(sequenceEdit, &QKeySequenceEdit::keySequenceChanged, MakeFunction(this, &ActionManagementDialog::OnShortcutTextChanged));
        currentSequencesLayout->addWidget(sequenceEdit);
    }
    {
        ReflectedPushButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedPushButton::Fields::Clicked] = "assignShortcut";
        params.fields[ReflectedPushButton::Fields::Text] = "assignButtonText";
        params.fields[ReflectedPushButton::Fields::Enabled] = "assignButtonEnabled";
        ReflectedPushButton* button = new ReflectedPushButton(params, accessor, model, this);
        currentSequencesLayout->addWidget(button->ToWidgetCast(), 1, 2, 1, 1);
    }
    layout->addLayout(currentSequencesLayout);

    QItemSelectionModel* selectionModel = new QItemSelectionModel(shortcutsModel);
    treeView->setSelectionModel(selectionModel);
    connections.AddConnection(selectionModel, &QItemSelectionModel::selectionChanged, MakeFunction(this, &ActionManagementDialog::OnActionSelected));

    UpdateSchemes();
    treeView->expandAll();
    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->resizeColumnToContents(0);
}

void ActionManagementDialog::OnShortcutTextChanged(const QKeySequence& seq)
{
    using namespace ActionManagementDialogDetail;
    QKeySequence inputSequence = seq;
    Array<int, 4> correctedKeys;
    correctedKeys.fill(0);
    Vector<QChar> incorrectKeys;
    bool withoutModifFound = false;
    for (int i = 0; i < inputSequence.count(); ++i)
    {
        int key = inputSequence[i];
        bool modifExists = HasModif(key);
        int modif = ExtractModif(key);
        key = ExtractKey(key);

        bool digit = IsDigit(key);
        bool functional = IsFunctionalKey(key);
        bool letter = IsLetter(key);
        bool special = IsSpecialAllowedSymbol(key);

        if (modifExists == false && functional == false)
        {
            withoutModifFound = true;
        }

        bool isKeyExists = digit || functional || letter || special;
        if (isKeyExists == false)
        {
            incorrectKeys.push_back(key);
            key = 0;
        }

        correctedKeys[i] = modif + key;
    }
    inputSequence = QKeySequence(correctedKeys[0], correctedKeys[1], correctedKeys[2], correctedKeys[3]);
    shortcutText = seq;
    if (withoutModifFound == true)
    {
        NotificationParams params;
        params.title = "Incorrect shortcut";
        params.message.type = Result::RESULT_ERROR;
        params.message.message = Format("You should use modification key (Ctrl, Alt, Shift or combination)\nfor all keys in shortcut except F1-F12");
        ui->ShowNotification(DAVA::mainWindowKey, params);
    }
    else if (inputSequence != seq)
    {
        QString incorrectKeysStr;
        for (const QChar& ch : incorrectKeys)
        {
            incorrectKeysStr = QString("%1 %2").arg(incorrectKeysStr).arg(ch);
        }
        NotificationParams params;
        params.title = "Incorrect shortcut";
        params.message.type = Result::RESULT_ERROR;
        params.message.message = Format("Incorrect keys was pressed: %s", incorrectKeysStr.toStdString().c_str());
        ui->ShowNotification(DAVA::mainWindowKey, params);
    }
}

String ActionManagementDialog::GetCurrentKeyBindingsScheme() const
{
    return ui->GetCurrentKeyBindingsScheme();
}

void ActionManagementDialog::SetCurrentKeyBindingsScheme(const String& scheme)
{
    ui->SetCurrentKeyBindingsScheme(scheme);
    UpdateSchemes();
}

void ActionManagementDialog::AddKeyBindingsScheme()
{
    QInputDialog dlg;
    dlg.setWindowFlags(Qt::Tool);
    dlg.setWindowTitle("New key binding scheme name");
    dlg.setObjectName("newSchemeNameDlg");
    dlg.setInputMode(QInputDialog::TextInput);
    QObject::connect(&dlg, &QInputDialog::textValueChanged, [&dlg](const QString& text) {
        const Array<QChar, 15> forbiddenSymbols = {
            '/', '\\', '.', ':', ';', '*', '?', '\"', '<', '>', '|', '{', '}', '[', ']'
        };

        QString replacedText = text;
        for (const QChar& symbol : forbiddenSymbols)
        {
            replacedText.replace(QString(symbol), "");
        }
        if (replacedText != text)
        {
            dlg.setTextValue(replacedText);
        }
    });

    QString newSchemeName;
    while (true)
    {
        int result = ui->ShowModalDialog(mainWindowKey, &dlg);
        if (result == QDialog::Rejected)
            break;

        QString inputValue = dlg.textValue();
        String value = inputValue.toStdString();
        auto iter = std::find(schemes.begin(), schemes.end(), value);
        if (iter == schemes.end())
        {
            newSchemeName = inputValue;
            break;
        }

        NotificationParams notifParams;
        notifParams.title = "Add key binding scheme";
        notifParams.message.type = Result::RESULT_ERROR;
        notifParams.message.message = Format("Key binding scheme with name \"%s\" already exists", value.c_str());
        ui->ShowNotification(mainWindowKey, notifParams);
    }

    if (newSchemeName.isEmpty() == false)
    {
        String schemeToAdd = newSchemeName.toStdString();
        ui->AddKeyBindingsScheme(schemeToAdd);
        ui->SetCurrentKeyBindingsScheme(schemeToAdd);
        UpdateSchemes();
    }
}

void ActionManagementDialog::RemoveKeyBindingsScheme()
{
    ui->RemoveKeyBindingsScheme(ui->GetCurrentKeyBindingsScheme());
    UpdateSchemes();
}

void ActionManagementDialog::ImportKeyBindingsScheme()
{
    FileDialogParams params;
    params.title = "Import key bindings scheme";
    params.filters = "Key binding scheme (*.kbs)";
    QString fileName = ui->GetOpenFileName(mainWindowKey, params);
    if (fileName.isEmpty() == true)
    {
        return;
    }

    String schemeName = ui->ImportKeyBindingsScheme(FilePath(fileName.toStdString()));
    ui->SetCurrentKeyBindingsScheme(schemeName);
    UpdateSchemes();
}

void ActionManagementDialog::ExportKeyBindingsScheme()
{
    FileDialogParams params;
    params.title = "Export key bindings scheme";
    params.filters = "Key binding scheme (*.kbs)";
    QString fileName = ui->GetSaveFileName(mainWindowKey, params);
    if (fileName.isEmpty() == true)
    {
        return;
    }

    ui->ExportKeyBindingsScheme(FilePath(fileName.toStdString()), ui->GetCurrentKeyBindingsScheme());
}

void ActionManagementDialog::UpdateSchemes()
{
    Vector<QString> expandedBlocks;
    int rows = shortcutsModel->rowCount(QModelIndex());
    for (int i = 0; i < rows; ++i)
    {
        QModelIndex index = shortcutsModel->index(i, 0, QModelIndex());
        if (treeView->isExpanded(index))
        {
            expandedBlocks.push_back(index.data(Qt::DisplayRole).value<QString>());
        }
    }

    Vector<String> s = ui->GetKeyBindingsSchemeNames();
    schemes = Set<String>(s.begin(), s.end());
    shortcutsModel->SetData(ui->GetKeyBindableActions());

    for (QString expandedBlock : expandedBlocks)
    {
        QModelIndex blockIndex = shortcutsModel->GetIndex(expandedBlock);
        treeView->expand(blockIndex);
    }

    QModelIndex selectedIndex = shortcutsModel->GetIndex(selectedBlockName, selectedAction);
    treeView->selectionModel()->select(selectedIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void ActionManagementDialog::RemoveSequence()
{
    QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(currentSequence), QKeySequence::NativeText);
    ui->RemoveShortcut(sequence, selectedAction);
    UpdateSchemes();
}

bool ActionManagementDialog::CanBeAssigned() const
{
    if (isSelectedActionReadOnly == true)
    {
        return false;
    }

    if (selectedAction == nullptr)
    {
        return false;
    }

    using namespace ActionManagementDialogDetail;
    for (int i = 0; i < shortcutText.count(); ++i)
    {
        int key = shortcutText[i];
        bool modifExists = HasModif(key);
        key = ExtractKey(key);
        bool digit = IsDigit(key);
        bool functional = IsFunctionalKey(key);
        bool letter = IsLetter(key);
        bool special = IsSpecialAllowedSymbol(key);

        bool isKeyExists = digit || functional || letter || special;
        if (isKeyExists == false)
        {
            return false;
        }

        if (modifExists == false && functional == false)
        {
            return false;
        }
    }

    if (shortcutText.count() == 0)
    {
        return false;
    }

    return selectedAction->shortcuts().indexOf(shortcutText) == -1;
}

void ActionManagementDialog::AssignShortcut()
{
    ui->AddShortcut(shortcutText, selectedAction);
    shortcutText = QKeySequence();
    sequenceEdit->clear();
    sequenceEdit->clearFocus();
    UpdateSchemes();
}

Qt::ShortcutContext ActionManagementDialog::GetContext() const
{
    return context;
}

void ActionManagementDialog::SetContext(Qt::ShortcutContext v)
{
    context = v;
    if (selectedAction != nullptr)
    {
        ui->SetActionContext(selectedAction, context);
        UpdateSchemes();
    }
}

void ActionManagementDialog::OnActionSelected(const QItemSelection& selected, const QItemSelection&)
{
    QModelIndexList indexes = selected.indexes();
    const KeyBindableAction* action = nullptr;
    if (indexes.isEmpty() == false)
    {
        action = shortcutsModel->GetKeyBindableAction(indexes.front());
    }

    selectedAction = nullptr;
    currentSequence = String();
    selectedBlockName = QString();
    isSelectedActionReadOnly = false;
    sequences.clear();

    if (action != nullptr)
    {
        foreach (const QKeySequence& seq, action->sequences)
        {
            sequences.insert(seq.toString(QKeySequence::NativeText).toStdString());
        }
        if (sequences.empty() == false)
        {
            currentSequence = *sequences.begin();
        }
        selectedBlockName = action->blockName;
        context = action->context;
        selectedAction = action->action;
        isSelectedActionReadOnly = action->isReadOnly;
    }
}

DAVA_REFLECTION_IMPL(ActionManagementDialog)
{
    ReflectionRegistrator<ActionManagementDialog>::Begin()
    // schemes combobox
    .Field("schemes", &ActionManagementDialog::schemes)
    .Field("currentScheme", &ActionManagementDialog::GetCurrentKeyBindingsScheme, &ActionManagementDialog::SetCurrentKeyBindingsScheme)
    // add, remove, import, export buttons
    .Field("iconSize", [](ActionManagementDialog*) { return QSize(16, 16); }, nullptr)
    .Field("autoRaise", [](ActionManagementDialog*) { return false; }, nullptr)
    .Field("addIcon", [](ActionManagementDialog*) { return QIcon(":/TArc/Resources/cplus.png"); }, nullptr)
    .Field("addToolTip", [](ActionManagementDialog*) { return "Create new key binding scheme"; }, nullptr)
    .Field("removeIcon", [](ActionManagementDialog*) { return QIcon(":/TArc/Resources/cminus.png"); }, nullptr)
    .Field("removeToolTip", [](ActionManagementDialog*) { return "Delete current key binding scheme"; }, nullptr)
    .Field("removeButtonEnabled", [](ActionManagementDialog* obj) { return obj->schemes.size() > 1; }, nullptr)
    .Field("importIcon", [](ActionManagementDialog*) { return QIcon(":/TArc/Resources/import.png"); }, nullptr)
    .Field("importToolTip", [](ActionManagementDialog*) { return "Import key binding scheme"; }, nullptr)
    .Field("exportIcon", [](ActionManagementDialog*) { return QIcon(":/TArc/Resources/export.png"); }, nullptr)
    .Field("exportToolTip", [](ActionManagementDialog*) { return "Export current key binding scheme"; }, nullptr)
    .Method("AddKeyBindingsScheme", &ActionManagementDialog::AddKeyBindingsScheme)
    .Method("RemoveKeyBindingsScheme", &ActionManagementDialog::RemoveKeyBindingsScheme)
    .Method("ImportKeyBindingsScheme", &ActionManagementDialog::ImportKeyBindingsScheme)
    .Method("ExportKeyBindingsScheme", &ActionManagementDialog::ExportKeyBindingsScheme)
    // shortcuts combobox
    .Field("currentSequence", &ActionManagementDialog::currentSequence)
    .Field("sequences", &ActionManagementDialog::sequences)
    .Field("emptySequenceText", [](ActionManagementDialog* obj) { return ""; }, nullptr)
    .Field("sequencesReadOnly", [](ActionManagementDialog* obj) { return obj->sequences.empty() == true; }, nullptr)
    // remove shortcut
    .Field("removeSequenceEnabled", [](ActionManagementDialog* obj) { return obj->currentSequence.empty() == false && obj->isSelectedActionReadOnly == false; }, nullptr)
    .Field("removeSequenceText", [](ActionManagementDialog*) { return "Remove"; }, nullptr)
    .Method("removeSequence", &ActionManagementDialog::RemoveSequence)
    // context combobox
    .Field("currentContext", &ActionManagementDialog::GetContext, &ActionManagementDialog::SetContext)[M::EnumT<Qt::ShortcutContext>()]
    .Field("contextReadOnly", [](ActionManagementDialog* obj) { return obj->selectedAction == nullptr || obj->isSelectedActionReadOnly == true; }, nullptr)
    //  Shortcut line edit
    .Field("shortcutEnabled", [](ActionManagementDialog* obj) {
        bool result = obj->selectedAction != nullptr && obj->isSelectedActionReadOnly == false;
        if (obj->sequenceEdit->isEnabled() != result)
        {
            obj->sequenceEdit->setEnabled(result);
        }

        return result;
    },
           nullptr)
    // assign button
    .Field("assignButtonText", [](ActionManagementDialog*) { return "Assign"; }, nullptr)
    .Field("assignButtonEnabled", &ActionManagementDialog::CanBeAssigned, nullptr)
    .Method("assignShortcut", &ActionManagementDialog::AssignShortcut)
    .End();
}
} // namespace DAVA
