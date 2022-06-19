#include "Classes/Modules/DistanceLinesModule/Private/DistanceLinesModuleData.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceSystem.h"

DAVA_VIRTUAL_REFLECTION_IMPL(DistanceLinesModuleData)
{
    DAVA::ReflectionRegistrator<DistanceLinesModuleData>::Begin()
    .ConstructorByPointer()
    .End();
}

DistanceLinesModuleData::~DistanceLinesModuleData() = default;
