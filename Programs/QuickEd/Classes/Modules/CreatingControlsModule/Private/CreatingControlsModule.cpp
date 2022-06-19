#include "Classes/Modules/CreatingControlsModule/CreatingControlsModule.h"
#include "Classes/Modules/CreatingControlsModule/CreatingControlsData.h"
#include "Classes/Modules/CreatingControlsModule/CreatingControlsSystem.h"
#include "Classes/Modules/DocumentsModule/EditorSystemsData.h"

#include "Classes/Application/QEGlobal.h"

#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Engine/PlatformApiQt.h>

DAVA_VIRTUAL_REFLECTION_IMPL(CreatingControlsModule)
{
    DAVA::ReflectionRegistrator<CreatingControlsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void CreatingControlsModule::PostInit()
{
    CreateData();
}

void CreatingControlsModule::CreateData()
{
    std::unique_ptr<CreatingControlsData> data = std::make_unique<CreatingControlsData>();
    data->creatingControlsSystem = std::make_unique<CreatingControlsSystem>(GetAccessor(), GetUI());
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void CreatingControlsModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        CreatingControlsData* data = GetAccessor()->GetGlobalContext()->GetData<CreatingControlsData>();
        Interfaces::EditorSystemsManagerInterface* systemsManager = QueryInterface<Interfaces::EditorSystemsManagerInterface>();
        systemsManager->RegisterEditorSystem(data->creatingControlsSystem.get());
        RegisterOperation(QEGlobal::CreateByClick.ID, this, &CreatingControlsModule::OnCreateByClick);
    }
}

void CreatingControlsModule::OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        CreatingControlsData* data = GetAccessor()->GetGlobalContext()->GetData<CreatingControlsData>();
        Interfaces::EditorSystemsManagerInterface* systemsManager = QueryInterface<Interfaces::EditorSystemsManagerInterface>();
        systemsManager->UnregisterEditorSystem(data->creatingControlsSystem.get());
    }
}

void CreatingControlsModule::OnCreateByClick(DAVA::String controlYaml)
{
    CreatingControlsData* data = GetAccessor()->GetGlobalContext()->GetData<CreatingControlsData>();
    data->creatingControlsSystem->SetCreateByClick(controlYaml);
}

DECL_TARC_MODULE(CreatingControlsModule);