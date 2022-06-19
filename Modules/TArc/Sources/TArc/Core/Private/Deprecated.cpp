#include "TArc/Core/Deprecated.h"

#include <TArc/Core/Core.h>
#include <TArc/Core/FieldBinder.h>

namespace DAVA
{
namespace DeprecatedDetails
{
Core* coreInstance = nullptr;

CoreInterface* GetCoreInterface()
{
    return coreInstance->GetCoreInterface();
}

UI* GetUI()
{
    return coreInstance->GetUI();
}
} // namespace DeprecatedDetails

namespace Deprecated
{
DataContext* GetGlobalContext()
{
    CoreInterface* coreInterface = DeprecatedDetails::GetCoreInterface();
    if (coreInterface == nullptr)
        return nullptr;
    return coreInterface->GetGlobalContext();
}

DataContext* GetActiveContext()
{
    CoreInterface* coreInterface = DeprecatedDetails::GetCoreInterface();
    if (coreInterface == nullptr)
        return nullptr;
    return coreInterface->GetActiveContext();
}

OperationInvoker* GetInvoker()
{
    return DeprecatedDetails::GetCoreInterface();
}

ContextAccessor* GetAccessor()
{
    return DeprecatedDetails::GetCoreInterface();
}

UI* GetUI()
{
    return DeprecatedDetails::GetUI();
}

FieldBinder* CreateFieldBinder()
{
    return new FieldBinder(GetAccessor());
}

DataWrapper CreateDataWrapper(const ReflectedType* type)
{
    CoreInterface* coreInterface = DeprecatedDetails::GetCoreInterface();
    if (coreInterface == nullptr)
        return DataWrapper();
    return coreInterface->CreateWrapper(type);
}

ModalMessageParams::Button ShowModalMessage(const ModalMessageParams& params)
{
    UI* ui = DeprecatedDetails::GetUI();
    DVASSERT(ui != nullptr);
    if (ui == nullptr)
    {
        return ModalMessageParams::NoButton;
    }
    return ui->ShowModalMessage(DAVA::mainWindowKey, params);
}

void ShowNotification(const NotificationParams& params)
{
    UI* ui = DeprecatedDetails::GetUI();
    DVASSERT(ui != nullptr);
    if (ui != nullptr)
    {
        ui->ShowNotification(DAVA::mainWindowKey, params);
    }
}

void InitTArcCore(Core* core)
{
    DeprecatedDetails::coreInstance = core;
}
}
} // namespace DAVA