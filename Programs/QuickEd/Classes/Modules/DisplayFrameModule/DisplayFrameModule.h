#pragma once

#include "Classes/Modules/BaseEditorModule.h"

namespace DAVA
{
class Any;
class FieldBinder;
}

class DisplayFrameModule : public BaseEditorModule
{
public:
    DisplayFrameModule();

private:
    void PostInit() override;
    void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;

    void OnSafeAreaChanged(const DAVA::Any& values);

    std::unique_ptr<DAVA::FieldBinder> fieldBinder;

    DAVA_VIRTUAL_REFLECTION(DisplayFrameModule, DAVA::ClientModule);

    DAVA::Token virtualSizeChangedToken;
};
