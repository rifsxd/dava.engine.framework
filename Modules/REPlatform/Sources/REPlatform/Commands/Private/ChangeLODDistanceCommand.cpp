#include "REPlatform/Commands/ChangeLODDistanceCommand.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Lod/LodComponent.h>

namespace DAVA
{
ChangeLODDistanceCommand::ChangeLODDistanceCommand(LodComponent* lod, int32 lodLayer, float32 distance)
    : RECommand("Change LOD Distance")
    , lodComponent(lod)
    , layer(lodLayer)
    , newDistance(distance)
    , oldDistance(0)
{
}

void ChangeLODDistanceCommand::Redo()
{
    if (!lodComponent)
        return;

    oldDistance = lodComponent->GetLodLayerDistance(layer);
    lodComponent->SetLodLayerDistance(layer, newDistance);
}

void ChangeLODDistanceCommand::Undo()
{
    if (!lodComponent)
        return;

    lodComponent->SetLodLayerDistance(layer, oldDistance);
}

Entity* ChangeLODDistanceCommand::GetEntity() const
{
    if (lodComponent)
        return lodComponent->GetEntity();

    return nullptr;
}

DAVA_VIRTUAL_REFLECTION_IMPL(ChangeLODDistanceCommand)
{
    ReflectionRegistrator<ChangeLODDistanceCommand>::Begin()
    .End();
}
} // namespace DAVA
