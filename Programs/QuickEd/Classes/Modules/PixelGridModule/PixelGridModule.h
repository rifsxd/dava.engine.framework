#pragma once

#include "Classes/Modules/BaseEditorModule.h"

class PixelGridModule : public BaseEditorModule
{
public:
    PixelGridModule();

private:
    void PostInit() override;
    void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;

    DAVA_VIRTUAL_REFLECTION(PixelGridModule, DAVA::ClientModule);
};
