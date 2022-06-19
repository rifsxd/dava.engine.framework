#pragma once

#include <TArc/Core/ClientModule.h>

class PreferencesModule : public DAVA::ClientModule
{
public:
    PreferencesModule();

private:
    void PostInit() override;

    DAVA_VIRTUAL_REFLECTION(PreferencesModule, DAVA::ClientModule);
};
