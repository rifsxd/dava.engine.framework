#include "Modules/BaseEditorModule.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

DAVA_VIRTUAL_REFLECTION_IMPL(BaseEditorModule)
{
    DAVA::ReflectionRegistrator<BaseEditorModule>::Begin()
    .End();
}

void BaseEditorModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        CreateSystems(QueryInterface<Interfaces::EditorSystemsManagerInterface>());
    }
}

void BaseEditorModule::OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        DestroySystems(QueryInterface<Interfaces::EditorSystemsManagerInterface>());
    }
}
