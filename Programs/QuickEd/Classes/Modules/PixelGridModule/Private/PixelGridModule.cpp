#include "Modules/PixelGridModule/PixelGridModule.h"
#include "Modules/PixelGridModule/PixelGrid.h"
#include "Modules/PixelGridModule/PixelGridData.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(PixelGridModule)
{
    DAVA::ReflectionRegistrator<PixelGridModule>::Begin()
    .ConstructorByPointer()
    .End();
}

PixelGridModule::PixelGridModule()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PixelGridPreferences);
}

void PixelGridModule::PostInit()
{
    std::unique_ptr<PixelGridData> data = std::make_unique<PixelGridData>();
    data->pixelGrid = std::make_unique<PixelGrid>(GetAccessor());
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void PixelGridModule::CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    PixelGridData* data = GetAccessor()->GetGlobalContext()->GetData<PixelGridData>();
    DVASSERT(data != nullptr);
    PixelGrid* pixelGrid = data->pixelGrid.get();

    systemsManager->RegisterEditorSystem(pixelGrid);
}

void PixelGridModule::DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    PixelGridData* data = GetAccessor()->GetGlobalContext()->GetData<PixelGridData>();
    DVASSERT(data != nullptr);
    PixelGrid* pixelGrid = data->pixelGrid.get();

    systemsManager->UnregisterEditorSystem(pixelGrid);
}

DECL_TARC_MODULE(PixelGridModule);
