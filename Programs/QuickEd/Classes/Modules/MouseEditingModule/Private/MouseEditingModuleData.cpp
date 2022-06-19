#include "Classes/Modules/MouseEditingModule/Private/MouseEditingModuleData.h"
#include "Classes/Modules/MouseEditingModule/Private/MouseEditingSystem.h"

DAVA_VIRTUAL_REFLECTION_IMPL(MouseEditingModuleData)
{
    DAVA::ReflectionRegistrator<MouseEditingModuleData>::Begin()
    .ConstructorByPointer()
    .End();
}

MouseEditingModuleData::~MouseEditingModuleData() = default;
