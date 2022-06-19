#pragma once

#include "TArc/WindowSubSystem/UI.h"

namespace DAVA
{
class ClientModule;
class UIProxy : public UI
{
public:
    UIProxy(ClientModule* module, UI* globalUI);

    void DeclareToolbar(const WindowKey& windowKey, const ActionPlacementInfo& toogleToolbarVisibility, const QString& toolbarName) override;

    void AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget) override;
    void AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action) override;
    void RemoveAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, const QString& actionName) override;
    void ShowMessage(const WindowKey& windowKey, const QString& message, uint32 duration = 0) override;
    void ShowNotification(const WindowKey& windowKey, const NotificationParams& params) const override;

    void ClearMessage(const WindowKey& windowKey) override;
    int ShowModalDialog(const WindowKey& parentWindow, QDialog* dialog) override;
    void ShowDialog(const WindowKey& parentWindow, QDialog* dialog) override;
    ModalMessageParams::Button ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params) override;
    QString GetOpenFileName(const WindowKey& windowKey, const FileDialogParams& params) override;
    QString GetSaveFileName(const WindowKey& windowKey, const FileDialogParams& params) override;
    QString GetExistingDirectory(const WindowKey& windowKey, const DirectoryDialogParams& params) override;
    std::unique_ptr<WaitHandle> ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params) override;
    bool HasActiveWaitDalogues() const override;

    QWidget* GetWindow(const WindowKey& windowKey) override;
    void InjectWindow(const WindowKey& windowKey, QMainWindow* window) override;
    UI* GetGlobalUI();

protected:
    void SetCurrentModule(ClientModule* module) override;

    void LockModule();
    void UnlockModule();

    class Guard;

private:
    ClientModule* module = nullptr;
    UI* globalUI = nullptr;
};
} // namespace DAVA
