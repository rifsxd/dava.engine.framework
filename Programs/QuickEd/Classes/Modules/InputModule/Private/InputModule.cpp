#include "Modules/InputModule/InputModule.h"
#include "Modules/InputModule/Private/EditorInput.h"
#include "Modules/InputModule/Private/InputModuleData.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(InputModule)
{
    DAVA::ReflectionRegistrator<InputModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void InputModule::PostInit()
{
    std::unique_ptr<InputModuleData> data = std::make_unique<InputModuleData>();
    data->system = std::make_unique<EditorInput>(GetAccessor());
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void InputModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        InputModuleData* data = GetAccessor()->GetGlobalContext()->GetData<InputModuleData>();
        DVASSERT(data != nullptr);
        EditorInput* system = data->system.get();

        Interfaces::EditorSystemsManagerInterface* systemsManager = QueryInterface<Interfaces::EditorSystemsManagerInterface>();
        systemsManager->RegisterEditorSystem(system);
    }
}

void InputModule::OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        InputModuleData* data = GetAccessor()->GetGlobalContext()->GetData<InputModuleData>();
        DVASSERT(data != nullptr);
        EditorInput* system = data->system.get();

        Interfaces::EditorSystemsManagerInterface* systemsManager = QueryInterface<Interfaces::EditorSystemsManagerInterface>();
        systemsManager->UnregisterEditorSystem(system);
    }
}

DECL_TARC_MODULE(InputModule);
