#pragma once


#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Core/ContextAccessor.h"

#include <Base/BaseTypes.h>

#include <QPointer>
#include <QAction>

class QMainWindow;
namespace DAVA
{
class PropertiesItem;

struct KeyBindableAction
{
    QString blockName;
    QString actionName;
    Qt::ShortcutContext context = Qt::WidgetShortcut;
    Qt::ShortcutContext defaultContext = Qt::WidgetShortcut;
    QList<QKeySequence> sequences;
    QList<QKeySequence> defaultSequences;
    QPointer<QAction> action;
    bool isReadOnly = false;
};

class UIManager final : public UI
{
public:
    class Delegate
    {
    public:
        virtual bool WindowCloseRequested(const WindowKey& key) = 0;
        virtual void OnWindowClosed(const WindowKey& key) = 0;
    };
    UIManager(ContextAccessor* accessor, Delegate* delegate, PropertiesItem&& holder);
    ~UIManager();

    void InitializationFinished();

    void DeclareToolbar(const WindowKey& windowKey, const ActionPlacementInfo& toogleToolbarVisibility, const QString& toolbarName) override;

    void AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget) override;
    void AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action) override;
    void RemoveAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, const QString& actionName) override;

    void ShowMessage(const WindowKey& windowKey, const QString& message, uint32 duration = 0) override;
    void ClearMessage(const WindowKey& windowKey) override;
    int ShowModalDialog(const WindowKey& windowKey, QDialog* dialog) override;
    void ShowDialog(const WindowKey& windowKey, QDialog* dialog) override;
    ModalMessageParams::Button ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params) override;
    void ShowNotification(const WindowKey& windowKey, const NotificationParams& params) const override;

    QString GetSaveFileName(const WindowKey& windowKey, const FileDialogParams& params) override;
    QString GetOpenFileName(const WindowKey& windowKey, const FileDialogParams& params) override;
    QString GetExistingDirectory(const WindowKey& windowKey, const DirectoryDialogParams& params) override;

    std::unique_ptr<WaitHandle> ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params = WaitDialogParams()) override;
    bool HasActiveWaitDalogues() const override;
    DAVA_DEPRECATED(QWidget* GetWindow(const WindowKey& windowKey) override);

    DAVA_DEPRECATED(void InjectWindow(const WindowKey& windowKey, QMainWindow* window) override);
    void ModuleDestroyed(ClientModule* module);

    String GetCurrentKeyBindingsScheme() const;
    void SetCurrentKeyBindingsScheme(const String& schemeName);
    Vector<String> GetKeyBindingsSchemeNames() const;
    void AddKeyBindingsScheme(const String& schemeName);
    void RemoveKeyBindingsScheme(const String& schemeName);
    String ImportKeyBindingsScheme(const FilePath& path);
    void ExportKeyBindingsScheme(const FilePath& path, const String& schemeName) const;

    const Vector<KeyBindableAction>& GetKeyBindableActions() const;
    void AddShortcut(const QKeySequence& shortcut, QPointer<QAction> action);
    void RemoveShortcut(const QKeySequence& shortcut, QPointer<QAction> action);
    void SetActionContext(QPointer<QAction> action, Qt::ShortcutContext context);

protected:
    void SetCurrentModule(ClientModule* module) override;

private:
    void RegisterAction(QAction* action);
    void LoadScheme();
    void SaveScheme() const;
    FilePath BuildSchemePath(const String& scheme) const;
    void SaveSchemeNames(const Vector<String>& schemes) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
} // namespace DAVA
