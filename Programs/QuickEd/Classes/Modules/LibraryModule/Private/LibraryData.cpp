#include "Classes/Modules/LibraryModule/LibraryData.h"

#include "Classes/Model/PackageHierarchy/ControlNode.h"

DAVA_VIRTUAL_REFLECTION_IMPL(LibraryData)
{
    DAVA::ReflectionRegistrator<LibraryData>::Begin()
    .ConstructorByPointer()
    .End();
}

LibraryData::LibraryData() = default;
LibraryData::~LibraryData() = default;

const DAVA::Vector<DAVA::RefPtr<ControlNode>>& LibraryData::GetDefaultControls() const
{
    return defaultControls;
}
