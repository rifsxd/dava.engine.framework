#pragma once

#include <TArc/Core/ClientModule.h>

namespace Interfaces
{
class EditorSystemsManagerInterface;
}

class BaseEditorModule : public DAVA::ClientModule
{
private:
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    virtual void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) = 0;
    virtual void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) = 0;

    DAVA_VIRTUAL_REFLECTION(BaseEditorModule, DAVA::ClientModule);
};