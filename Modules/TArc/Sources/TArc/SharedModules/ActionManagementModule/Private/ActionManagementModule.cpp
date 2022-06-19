#include "TArc/SharedModules/ActionManagementModule/ActionManagementModule.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/Private/UIProxy.h"
#include "TArc/WindowSubSystem/Private/UIManager.h"
#include "TArc/WindowSubSystem/QtAction.h"
#include "TArc/WindowSubSystem/ActionUtils.h"

#include "TArc/SharedModules/ActionManagementModule/Private/ActionManagementDialog.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
void ActionManagementModule::PostInit()
{
    executor.DelayedExecute([this]() {
        UI* ui = GetUI();
        ContextAccessor* accessor = GetAccessor();
        ActionPlacementInfo placementInfo(CreateMenuPoint(QList<QString>() << "Tools"));
        QAction* action = new QAction("Key bindings", nullptr);
        ui->AddAction(mainWindowKey, placementInfo, action);

        connections.AddConnection(action, &QAction::triggered, [accessor, ui]() {
            ActionManagementDialog dlg(accessor, static_cast<UIManager*>(static_cast<UIProxy*>(ui)->GetGlobalUI()));
            ui->ShowModalDialog(mainWindowKey, &dlg);
        });
    });
}

DAVA_VIRTUAL_REFLECTION_IMPL(ActionManagementModule)
{
    ReflectionRegistrator<ActionManagementModule>::Begin()
    .ConstructorByPointer()
    .End();
}
} // namespace DAVA
