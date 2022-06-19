#include "Classes/Modules/IssueNavigatorModule/IssueNavigatorData.h"

DAVA_VIRTUAL_REFLECTION_IMPL(IssueNavigatorData)
{
    DAVA::ReflectionRegistrator<IssueNavigatorData>::Begin()
    .ConstructorByPointer()
    .End();
}

IssueNavigatorData::~IssueNavigatorData() = default;
