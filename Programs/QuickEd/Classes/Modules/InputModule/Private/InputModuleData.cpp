#include "Classes/Modules/InputModule/Private/InputModuleData.h"
#include "Classes/Modules/InputModule/Private/EditorInput.h"

DAVA_VIRTUAL_REFLECTION_IMPL(InputModuleData)
{
    DAVA::ReflectionRegistrator<InputModuleData>::Begin()
    .ConstructorByPointer()
    .End();
}

InputModuleData::InputModuleData() = default;
InputModuleData::~InputModuleData() = default;
