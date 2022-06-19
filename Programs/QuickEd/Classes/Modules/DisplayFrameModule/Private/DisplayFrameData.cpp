#include "Modules/DisplayFrameModule/DisplayFrameData.h"
#include "Modules/DisplayFrameModule/DisplaySafeArea.h"

DAVA_VIRTUAL_REFLECTION_IMPL(DisplayFrameData)
{
    DAVA::ReflectionRegistrator<DisplayFrameData>::Begin()
    .ConstructorByPointer()
    .End();
}

DisplayFrameData::~DisplayFrameData() = default;
