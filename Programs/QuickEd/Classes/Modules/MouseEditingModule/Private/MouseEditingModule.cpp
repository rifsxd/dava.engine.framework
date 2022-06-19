#include "Classes/Modules/MouseEditingModule/MouseEditingModule.h"
#include "Classes/Modules/MouseEditingModule/Private/MouseEditingModuleData.h"
#include "Classes/Modules/MouseEditingModule/Private/MouseEditingSystem.h"

#include "Classes/Application/QEGlobal.h"

#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(MouseEditingModule)
{
    DAVA::ReflectionRegistrator<MouseEditingModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void MouseEditingModule::PostInit()
{
    std::unique_ptr<MouseEditingModuleData> data = std::make_unique<MouseEditingModuleData>();
    data->system = std::make_unique<MouseEditingSystem>(GetAccessor());
    data->system->duplicateRequest.Connect(this, &MouseEditingModule::OnDuplicateRequested);
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void MouseEditingModule::CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    MouseEditingModuleData* data = GetAccessor()->GetGlobalContext()->GetData<MouseEditingModuleData>();
    DVASSERT(data != nullptr);
    MouseEditingSystem* system = data->system.get();

    systemsManager->RegisterEditorSystem(system);
}

void MouseEditingModule::DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    MouseEditingModuleData* data = GetAccessor()->GetGlobalContext()->GetData<MouseEditingModuleData>();
    DVASSERT(data != nullptr);
    MouseEditingSystem* system = data->system.get();

    systemsManager->UnregisterEditorSystem(system);
}

void MouseEditingModule::OnDuplicateRequested()
{
    InvokeOperation(QEGlobal::Duplicate.ID);
}

DECL_TARC_MODULE(MouseEditingModule);
