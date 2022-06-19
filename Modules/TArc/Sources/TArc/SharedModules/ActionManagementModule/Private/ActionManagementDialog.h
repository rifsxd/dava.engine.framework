#pragma once

#include "TArc/Utils/QtConnections.h"
#include <Reflection/Reflection.h>

#include <QDialog>
#include <QPointer>

class QItemSelectionModel;
class QItemSelection;
class QTreeView;
class QKeySequenceEdit;

namespace DAVA
{
class UIManager;
class ContextAccessor;
class ShortcutsModel;
class ActionManagementDialog : public QDialog
{
public:
    ActionManagementDialog(ContextAccessor* accessor, UIManager* ui);

private:
    String GetCurrentKeyBindingsScheme() const;
    void SetCurrentKeyBindingsScheme(const String& scheme);

    void AddKeyBindingsScheme();
    void RemoveKeyBindingsScheme();
    void ImportKeyBindingsScheme();
    void ExportKeyBindingsScheme();

    void UpdateSchemes();

    void RemoveSequence();

    bool CanBeAssigned() const;
    void AssignShortcut();

    Qt::ShortcutContext GetContext() const;
    void SetContext(Qt::ShortcutContext v);

    void OnActionSelected(const QItemSelection& selected, const QItemSelection& deselected);
    void OnShortcutTextChanged(const QKeySequence& seq);

private:
    ContextAccessor* accessor = nullptr;
    UIManager* ui = nullptr;
    QTreeView* treeView = nullptr;
    ShortcutsModel* shortcutsModel = nullptr;
    QtConnections connections;

    Set<String> schemes;

    QString selectedBlockName;
    QPointer<QAction> selectedAction;
    String currentSequence;
    Set<String> sequences;
    Qt::ShortcutContext context = Qt::WidgetWithChildrenShortcut;
    bool isSelectedActionReadOnly = false;

    QKeySequence shortcutText;
    mutable QKeySequenceEdit* sequenceEdit;

    DAVA_REFLECTION(ActionManagementDialog);
};
} // namespace DAVA
