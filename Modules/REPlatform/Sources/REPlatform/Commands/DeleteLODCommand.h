#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class DeleteRenderBatchCommand;
class LodComponent;
class Entity;
class DeleteLODCommand : public RECommand
{
public:
    DeleteLODCommand(LodComponent* lod, int32 lodIndex, int32 switchIndex);
    virtual ~DeleteLODCommand();

    void Undo() override;
    void Redo() override;
    Entity* GetEntity() const;

    const Vector<DeleteRenderBatchCommand*>& GetRenderBatchCommands() const;

protected:
    LodComponent* lodComponent;
    int32 deletedLodIndex;
    int32 requestedSwitchIndex;

    Vector<DeleteRenderBatchCommand*> deletedBatches;

    DAVA_VIRTUAL_REFLECTION(DeleteLODCommand, RECommand);
};
} // namespace DAVA
