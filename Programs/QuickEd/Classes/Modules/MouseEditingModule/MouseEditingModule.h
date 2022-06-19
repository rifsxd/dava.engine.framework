#pragma once

#include "Classes/Modules/BaseEditorModule.h"

class MouseEditingModule : public BaseEditorModule
{
    void PostInit() override;
    void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;

    void OnDuplicateRequested();

    DAVA_VIRTUAL_REFLECTION(MouseEditingModule, DAVA::ClientModule);
};
