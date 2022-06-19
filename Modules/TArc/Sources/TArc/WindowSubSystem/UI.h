#pragma once

#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/DataProcessing/Common.h"
#include "TArc/Qt/QtString.h"

#include <Functional/Function.h>
#include <Base/Result.h>
#include <Base/FastName.h>
#include <Base/Any.h>

#include <Qt>
#include <QUrl>
#include <QFlags>
#include <QFileDialog>
#include <QPointer>

class QWidget;
class QAction;
class QMainWindow;

namespace DAVA
{
class ClientModule;
class QtReflectionBridge;
class WindowKey
{
public:
    WindowKey(const String& appID);
    const String& GetAppID() const;

    bool operator==(const WindowKey& other) const;
    bool operator!=(const WindowKey& other) const;

private:
    String appID;
};

/** Token of TArc application's main window */
extern const WindowKey mainWindowKey;

/**
    Most common menu items tokens.
    It's recommended to use them instead of literals hardcoding.
    e.g.
    /code
        CreateMenuPoint(MenuItems::menuEdit); // recommended
        CreateMenuPoint("Edit");              // not recommended
    /endcode
    
*/
namespace MenuItems
{
extern const QString menuFile;
extern const QString menuEdit;
extern const QString menuView;
extern const QString menuHelp;
extern const QString menuFind;
}

class ActionPlacementInfo
{
public:
    ActionPlacementInfo() = default;
    ActionPlacementInfo(const QUrl& url);

    void AddPlacementPoint(const QUrl& url);
    const Vector<QUrl>& GetUrls() const;

private:
    Vector<QUrl> urls;
};

struct DockPanelInfo
{
    DockPanelInfo();
    QString title;
    //path where action for change dock visibility will be placed
    ActionPlacementInfo actionPlacementInfo;
    bool tabbed = true;
    bool ensureVisible = false; // force make panel visible in its dock area on create
    Qt::DockWidgetArea area = Qt::RightDockWidgetArea;

    enum class Fields
    {
        Title, // QString
        IsActive // bool
    };
    Map<Fields, FieldDescriptor> descriptors;
};

struct CentralPanelInfo
{
};

class IGeometryProcessor
{
public:
    // Result is a rectangle, where "rectangle.topLeft" point is a pivot for widget and
    // "rectangle.size" is a new size for widget
    virtual QRect GetWidgetGeometry(QWidget* parent, QWidget* content) const = 0;
};

struct OverCentralPanelInfo
{
    std::shared_ptr<IGeometryProcessor> geometryProcessor;
};

void ShowOverCentralPanel(QWidget* view);
void HideOverCentralPanel(QWidget* view);

class PanelKey
{
public:
    enum Type
    {
        DockPanel,
        CentralPanel,
        OverCentralPanel,
        TypesCount
    };

    PanelKey(const QString& viewName, const DockPanelInfo& info);
    PanelKey(const QString& viewName, const CentralPanelInfo& info);
    PanelKey(const QString& viewName, const OverCentralPanelInfo& info);

    const QString& GetViewName() const;
    Type GetType() const;
    const Any& GetInfo() const;

private:
    PanelKey(Type t, const QString& viewName, const Any& info);

    QString viewName;
    Type type;
    Any info;
};

struct WaitDialogParams
{
    QString message = QStringLiteral("Please wait, until current operation will be finished");
    uint32 min = 0; // if min and max value equal 0, than progress bar will be infinite
    uint32 max = 0;
    bool needProgressBar = true;
    bool cancelable = false;
};

class WaitHandle
{
public:
    virtual ~WaitHandle()
    {
    }

    virtual void SetMessage(const QString& msg) = 0;
    virtual void SetRange(uint32 min, uint32 max) = 0;
    virtual void SetProgressValue(uint32 progress) = 0;
    virtual bool WasCanceled() const = 0;

    virtual void Update() = 0;
};

struct FileDialogParams
{
    QString title;
    QString dir;
    QString filters;
};

struct DirectoryDialogParams
{
    QString title;
    QString dir;
    QFileDialog::Option options = QFileDialog::ShowDirsOnly;
};

struct ModalMessageParams
{
    enum Button
    {
        NoButton = 0,
        Ok = 0x1,
        Open = 0x2,
        Cancel = 0x4,
        Close = 0x8,
        Yes = 0x10,
        YesToAll = 0x20,
        No = 0x40,
        NoToAll = 0x80,
        Discard = 0x100,
        Apply = 0x200,
        Save = 0x400,
        SaveAll = 0x800,
        Abort = 0x1000,
        Retry = 0x2000,
        Ignore = 0x4000,
        Reset = 0x8000
    };

    enum Icon
    {
        NoIcon,
        Information,
        Warning,
        Critical,
        Question
    };

    Q_DECLARE_FLAGS(Buttons, Button);

    QString title;
    QString message;
    Buttons buttons = Buttons(Ok | Cancel);
    Button defaultButton = NoButton;
    Icon icon = NoIcon;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ModalMessageParams::Buttons);

struct NotificationParams
{
    Result message;
    String title;
    Function<void()> callback;
};

class ControlProxy;
class UI
{
public:
    UI() = default;
    UI(const UI& other) = delete;
    UI& operator=(const UI& other) = delete;
    virtual ~UI()
    {
    }

    virtual void DeclareToolbar(const WindowKey& windowKey, const ActionPlacementInfo& toogleToolbarVisibility, const QString& toolbarName) = 0;

    void AddControlView(const WindowKey& windowKey, const PanelKey& panelKey, ControlProxy* widget);
    virtual void AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget) = 0;
    virtual void AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action) = 0;
    virtual void RemoveAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, const QString& actionName) = 0;

    virtual void ShowMessage(const WindowKey& windowKey, const QString& message, uint32 duration = 0) = 0;
    virtual void ClearMessage(const WindowKey& windowKey) = 0;
    virtual int ShowModalDialog(const WindowKey& parentWindow, QDialog* dialog) = 0;
    virtual void ShowDialog(const WindowKey& parentWindow, QDialog* dialog) = 0;
    virtual ModalMessageParams::Button ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params) = 0;
    virtual void ShowNotification(const WindowKey& windowKey, const NotificationParams& params) const = 0;

    virtual QString GetOpenFileName(const WindowKey& windowKey, const FileDialogParams& params = FileDialogParams()) = 0;
    virtual QString GetSaveFileName(const WindowKey& windowKey, const FileDialogParams& params = FileDialogParams()) = 0;
    virtual QString GetExistingDirectory(const WindowKey& windowKey, const DirectoryDialogParams& params = DirectoryDialogParams()) = 0;

    virtual std::unique_ptr<WaitHandle> ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params = WaitDialogParams()) = 0;
    virtual bool HasActiveWaitDalogues() const = 0;
    Signal<> lastWaitDialogWasClosed;

    DAVA_DEPRECATED(virtual QWidget* GetWindow(const WindowKey& windowKey) = 0);
    DAVA_DEPRECATED(virtual void InjectWindow(const WindowKey& windowKey, QMainWindow* window) = 0);

protected:
    friend class UIProxy;
    virtual void SetCurrentModule(ClientModule* module) = 0;
};
} // namespace DAVA

namespace std
{
template <>
struct hash<DAVA::WindowKey>
{
    std::size_t operator()(const DAVA::WindowKey& k) const
    {
        std::hash<DAVA::String> hasher;
        return hasher(k.GetAppID());
    }
};
}
