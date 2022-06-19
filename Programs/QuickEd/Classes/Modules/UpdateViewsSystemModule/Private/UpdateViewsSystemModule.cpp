#include "Modules/UpdateViewsSystemModule/UpdateViewsSystemModule.h"
#include "Modules/UpdateViewsSystemModule/UpdateViewsSystem.h"

#include <UI/Render/UIRenderSystem.h>
#include <UI/UIControlSystem.h>

DAVA_VIRTUAL_REFLECTION_IMPL(UpdateViewsSystemModule)
{
    DAVA::ReflectionRegistrator<UpdateViewsSystemModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void UpdateViewsSystemModule::PostInit()
{
    DAVA::UIControlSystem* uiControlSystem = GetAccessor()->GetEngineContext()->uiControlSystem;
    uiControlSystem->AddSystem(std::make_unique<UpdateViewsSystem>(), uiControlSystem->GetRenderSystem());
}

void UpdateViewsSystemModule::OnWindowClosed(const DAVA::WindowKey& key)
{
    DVASSERT(key == DAVA::mainWindowKey);
    DAVA::UIControlSystem* uiControlSystem = GetAccessor()->GetEngineContext()->uiControlSystem;
    uiControlSystem->RemoveSystem(uiControlSystem->GetSystem<UpdateViewsSystem>());
}
