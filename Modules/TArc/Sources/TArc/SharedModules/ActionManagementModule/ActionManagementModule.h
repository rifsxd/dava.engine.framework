#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Utils/QtDelayedExecutor.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class ActionManagementModule : public ClientModule
{
protected:
    void PostInit() override;

private:
    QtConnections connections;
    QtDelayedExecutor executor;

    DAVA_VIRTUAL_REFLECTION(ActionManagementModule, ClientModule);
};
} // namespace DAVA
