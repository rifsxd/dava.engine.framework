#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtDelayedExecutor.h>

class EditorSystemsManager;

class CreatingControlsModule : public DAVA::ClientModule
{
public:
    CreatingControlsModule() = default;

private:
    // ClientModule
    void PostInit() override;
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    void CreateData();

    void OnCreateByClick(DAVA::String controlYaml);

private:
    DAVA::QtDelayedExecutor delayedExecutor;

    DAVA_VIRTUAL_REFLECTION(CreatingControlsModule, DAVA::ClientModule);
};