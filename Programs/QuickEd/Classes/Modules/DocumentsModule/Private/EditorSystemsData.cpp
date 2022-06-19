#include "Classes/Modules/DocumentsModule/EditorSystemsData.h"
#include "Classes/Painter/Painter.h"
#include "Classes/EditorSystems/EditorSystemsManager.h"

DAVA_VIRTUAL_REFLECTION_IMPL(EditorSystemsData)
{
    DAVA::ReflectionRegistrator<EditorSystemsData>::Begin()
    .ConstructorByPointer()
    .Field(emulationModePropertyName.c_str(), &EditorSystemsData::emulationMode)
    .End();
}

EditorSystemsData::EditorSystemsData() = default;
EditorSystemsData::~EditorSystemsData() = default;

bool EditorSystemsData::IsHighlightDisabled() const
{
    return highlightDisabled;
}

const EditorSystemsManager* EditorSystemsData::GetSystemsManager() const
{
    return systemsManager.get();
}

DAVA::FastName EditorSystemsData::emulationModePropertyName{ "emulation mode" };
