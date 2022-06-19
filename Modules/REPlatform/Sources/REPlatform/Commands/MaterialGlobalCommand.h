#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Render/Material/NMaterial.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class MaterialGlobalSetCommand : public RECommand
{
public:
    MaterialGlobalSetCommand(Scene* _scene, NMaterial* global);
    ~MaterialGlobalSetCommand();

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;

protected:
    Scene* scene;
    NMaterial* oldGlobal;
    NMaterial* newGlobal;

    DAVA_VIRTUAL_REFLECTION(MaterialGlobalSetCommand, RECommand);
};
} // namespace DAVA
