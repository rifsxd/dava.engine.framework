#include "Modules/PixelGridModule/PixelGridData.h"
#include "Modules/PixelGridModule/PixelGrid.h"

DAVA_VIRTUAL_REFLECTION_IMPL(PixelGridData)
{
    DAVA::ReflectionRegistrator<PixelGridData>::Begin()
    .ConstructorByPointer()
    .End();
}

PixelGridData::~PixelGridData() = default;
