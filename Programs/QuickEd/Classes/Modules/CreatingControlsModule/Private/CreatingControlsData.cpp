#include "Classes/Modules/CreatingControlsModule/CreatingControlsData.h"
#include "Classes/Modules/CreatingControlsModule/CreatingControlsSystem.h"

DAVA_VIRTUAL_REFLECTION_IMPL(CreatingControlsData)
{
    DAVA::ReflectionRegistrator<CreatingControlsData>::Begin()
    .ConstructorByPointer()
    .End();
}

CreatingControlsData::CreatingControlsData() = default;
CreatingControlsData::~CreatingControlsData() = default;