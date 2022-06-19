#include "REPlatform/Commands/MaterialGlobalCommand.h"

#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
MaterialGlobalSetCommand::MaterialGlobalSetCommand(DAVA::Scene* _scene, DAVA::NMaterial* global)
    : RECommand("Set global material")
    , scene(_scene)
{
    DVASSERT(nullptr != scene);

    newGlobal = SafeRetain(global);
    oldGlobal = SafeRetain(scene->GetGlobalMaterial());
}

MaterialGlobalSetCommand::~MaterialGlobalSetCommand()
{
    SafeRelease(oldGlobal);
    SafeRelease(newGlobal);
}

void MaterialGlobalSetCommand::Redo()
{
    scene->SetGlobalMaterial(newGlobal);
}

void MaterialGlobalSetCommand::Undo()
{
    scene->SetGlobalMaterial(oldGlobal);
}

DAVA::Entity* MaterialGlobalSetCommand::GetEntity() const
{
    return scene;
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialGlobalSetCommand)
{
    ReflectionRegistrator<MaterialGlobalSetCommand>::Begin()
    .End();
}
} // namespace DAVA
