#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Utils/QtDelayedExecutor.h"

namespace DAVA
{
class SettingsManager;
class SettingsModule : public ClientModule
{
private:
    void PostInit() override;
    void ShowSettings();

    std::unique_ptr<SettingsManager> manager;
    QtConnections connections;
    QtDelayedExecutor executor;

    DAVA_VIRTUAL_REFLECTION(SettingsModule, ClientModule);
};

void InitColorPickerOptions(bool initForHiddenUsage);
} // namespace DAVA
