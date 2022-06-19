#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class LodComponent;
class ChangeLODDistanceCommand : public RECommand
{
public:
    ChangeLODDistanceCommand(LodComponent* lod, int32 lodLayer, float32 distance);

    void Undo() override;
    void Redo() override;
    Entity* GetEntity() const;

protected:
    LodComponent* lodComponent;
    int32 layer;
    float32 newDistance;
    float32 oldDistance;

    DAVA_VIRTUAL_REFLECTION(ChangeLODDistanceCommand, RECommand);
};
} // namespace DAVA
