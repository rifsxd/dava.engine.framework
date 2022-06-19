#include "Classes/Modules/HUDModule/HUDModuleData.h"
#include "Classes/Modules/HUDModule/Private/HUDSystem.h"

DAVA_VIRTUAL_REFLECTION_IMPL(HUDModuleData)
{
    DAVA::ReflectionRegistrator<HUDModuleData>::Begin()
    .ConstructorByPointer()
    .Field(highlightedNodePropertyName.c_str(), &HUDModuleData::highlightedNode)
    .End();
}

HUDModuleData::~HUDModuleData() = default;

ControlNode* HUDModuleData::GetHighlight() const
{
    return highlightedNode;
}

DAVA::FastName HUDModuleData::highlightedNodePropertyName{ "highlighted node" };
