#include "REPlatform/Commands/MaterialSwitchParentCommand.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Material/NMaterial.h>

namespace DAVA
{
MaterialSwitchParentCommand::MaterialSwitchParentCommand(NMaterial* instance, NMaterial* _newParent)
    : RECommand("Switch Material Parent")
{
    DVASSERT(instance);
    DVASSERT(_newParent);
    DVASSERT(instance->GetParent());

    currentInstance = SafeRetain(instance);
    newParent = SafeRetain(_newParent);
    oldParent = SafeRetain(instance->GetParent());
}

MaterialSwitchParentCommand::~MaterialSwitchParentCommand()
{
    SafeRelease(oldParent);
    SafeRelease(newParent);
    SafeRelease(currentInstance);
}

void MaterialSwitchParentCommand::Redo()
{
    currentInstance->SetParent(newParent);
}

void MaterialSwitchParentCommand::Undo()
{
    currentInstance->SetParent(oldParent);
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialSwitchParentCommand)
{
    ReflectionRegistrator<MaterialSwitchParentCommand>::Begin()
    .End();
}

} // namespace DAVA
