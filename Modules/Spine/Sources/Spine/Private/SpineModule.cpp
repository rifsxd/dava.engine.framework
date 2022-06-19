#include "Spine/SpineModule.h"
#include "UI/Spine/UISpineAttachControlsToBonesComponent.h"
#include "UI/Spine/UISpineComponent.h"
#include "UI/Spine/UISpineSingleComponent.h"
#include "UI/Spine/UISpineSystem.h"

#include <Base/GlobalEnum.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Entity/ComponentManager.h>
#include <Reflection/ReflectionRegistrator.h>
#include <UI/Styles/UIStyleSheetSystem.h>
#include <UI/UIControlSystem.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SpineModule)
{
    ReflectionRegistrator<SpineModule>::Begin()
    .End();
}

SpineModule::SpineModule(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UISpineComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UISpineAttachControlsToBonesComponent);
}

void SpineModule::Init()
{
    const Engine* engine = Engine::Instance();
    const EngineContext* context = engine->GetContext();

    ComponentManager* cm = context->componentManager;
    cm->RegisterComponent<UISpineComponent>();
    cm->RegisterComponent<UISpineAttachControlsToBonesComponent>();

    UIControlSystem* cs = context->uiControlSystem;
    cs->AddSystem(std::make_unique<UISpineSystem>(), cs->GetStyleSheetSystem());
    cs->AddSingleComponent(std::make_unique<UISpineSingleComponent>());
}

void SpineModule::Shutdown()
{
    const Engine* engine = Engine::Instance();
    const EngineContext* context = engine->GetContext();

    UIControlSystem* cs = context->uiControlSystem;
    cs->RemoveSystem(cs->GetSystem<UISpineSystem>());
    cs->RemoveSingleComponent(cs->GetSingleComponent<UISpineSingleComponent>());
}
}
