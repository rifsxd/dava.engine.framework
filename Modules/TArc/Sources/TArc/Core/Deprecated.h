#pragma once

#include "TArc/Core/ContextAccessor.h"
#include "TArc/Core/OperationInvoker.h"
#include "TArc/Core/OperationRegistrator.h"
#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/WindowSubSystem/UI.h"

#include <Base/FastName.h>

namespace DAVA
{
class Core;
class FieldBinder;

// never use "using namespace Deprecated" in your code
namespace Deprecated
{
void InitTArcCore(Core* core);

DataContext* GetGlobalContext();
DataContext* GetActiveContext();

OperationInvoker* GetInvoker();
ContextAccessor* GetAccessor();
UI* GetUI();

FieldBinder* CreateFieldBinder();

DataWrapper CreateDataWrapper(const ReflectedType* type);
ModalMessageParams::Button ShowModalMessage(const ModalMessageParams& params);
void ShowNotification(const NotificationParams& params);

template <typename T>
T* GetDataNode()
{
    DataContext* ctx = GetGlobalContext();
    if (ctx == nullptr)
        return nullptr;
    return ctx->GetData<T>();
}

template <typename T>
T* GetActiveDataNode()
{
    DataContext* ctx = GetActiveContext();
    if (ctx == nullptr)
        return nullptr;
    return ctx->GetData<T>();
}
} // namespace Deprecated
} // namespace DAVA