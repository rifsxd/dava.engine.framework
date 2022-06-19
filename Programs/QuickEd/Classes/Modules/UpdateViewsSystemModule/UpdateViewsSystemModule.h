#pragma once

#include <TArc/Core/ClientModule.h>

class UpdateViewsSystemModule : public DAVA::ClientModule
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::WindowKey& key) override;

    DAVA_VIRTUAL_REFLECTION(UpdateViewsSystemModule, DAVA::ClientModule);
};
