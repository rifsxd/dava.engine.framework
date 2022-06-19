#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/FastName.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class NMaterial;
class MaterialSwitchParentCommand : public RECommand
{
public:
    MaterialSwitchParentCommand(NMaterial* instance, NMaterial* newParent);
    ~MaterialSwitchParentCommand();

    void Undo() override;
    void Redo() override;

protected:
    NMaterial* oldParent;
    NMaterial* newParent;
    NMaterial* currentInstance;

    DAVA_VIRTUAL_REFLECTION(MaterialSwitchParentCommand, RECommand);
};
} // namespace DAVA
