#include "Classes/Modules/PackageModule/PackageData.h"
#include "Classes/Modules/PackageModule/Private/PackageWidget.h"

DAVA_VIRTUAL_REFLECTION_IMPL(PackageData)
{
    DAVA::ReflectionRegistrator<PackageData>::Begin()
    .ConstructorByPointer()
    .End();
}
