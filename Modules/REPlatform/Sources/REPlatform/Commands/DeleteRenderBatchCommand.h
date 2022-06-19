#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class RenderObject;
class RenderBatch;
class DeleteRenderBatchCommand : public RECommand
{
public:
    DeleteRenderBatchCommand(Entity* entity, RenderObject* renderObject, uint32 renderBatchIndex);
    virtual ~DeleteRenderBatchCommand();

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;

    RenderBatch* GetRenderBatch() const;

protected:
    Entity* entity;
    RenderObject* renderObject;
    RenderBatch* renderBatch;

    int32 lodIndex;
    int32 switchIndex;

    DAVA_VIRTUAL_REFLECTION(DeleteRenderBatchCommand, RECommand);
};
} // namespace DAVA
