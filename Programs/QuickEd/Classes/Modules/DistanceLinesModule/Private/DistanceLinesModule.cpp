#include "Classes/Modules/DistanceLinesModule/DistanceLinesModule.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceSystem.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceLinesPreferences.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceLinesModuleData.h"

#include "Classes/Modules/HUDModule/HUDModuleData.h"

#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(DistanceLinesModule)
{
    DAVA::ReflectionRegistrator<DistanceLinesModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DistanceLinesModule::DistanceLinesModule()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DistanceSystemPreferences);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DistanceSystem);
}

void DistanceLinesModule::PostInit()
{
    std::unique_ptr<DistanceLinesModuleData> data = std::make_unique<DistanceLinesModuleData>();
    data->system = std::make_unique<DistanceSystem>(GetAccessor());
    data->system->getHighlight = DAVA::MakeFunction(this, &DistanceLinesModule::GetHighlightNode);
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void DistanceLinesModule::CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    DistanceLinesModuleData* data = GetAccessor()->GetGlobalContext()->GetData<DistanceLinesModuleData>();
    DVASSERT(data != nullptr);
    DistanceSystem* system = data->system.get();

    systemsManager->RegisterEditorSystem(system);
}

void DistanceLinesModule::DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    DistanceLinesModuleData* data = GetAccessor()->GetGlobalContext()->GetData<DistanceLinesModuleData>();
    DVASSERT(data != nullptr);
    DistanceSystem* system = data->system.get();

    systemsManager->UnregisterEditorSystem(system);
}

ControlNode* DistanceLinesModule::GetHighlightNode() const
{
    HUDModuleData* hudData = GetAccessor()->GetGlobalContext()->GetData<HUDModuleData>();
    if (hudData != nullptr)
    {
        return hudData->GetHighlight();
    }
    return nullptr;
}

DECL_TARC_MODULE(DistanceLinesModule);
