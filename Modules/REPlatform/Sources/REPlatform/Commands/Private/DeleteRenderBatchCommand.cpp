#include "REPlatform/Commands/DeleteRenderBatchCommand.h"

#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DeleteRenderBatchCommand::DeleteRenderBatchCommand(Entity* en, RenderObject* ro, uint32 batchIndex)
    : RECommand("Delete Render Batch")
    , entity(en)
    , renderObject(ro)
{
    DVASSERT(entity);
    DVASSERT(renderObject);
    DVASSERT(batchIndex < renderObject->GetRenderBatchCount());

    renderBatch = renderObject->GetRenderBatch(batchIndex, lodIndex, switchIndex);
    SafeRetain(renderBatch);
}

DeleteRenderBatchCommand::~DeleteRenderBatchCommand()
{
    SafeRelease(renderBatch);
}

void DeleteRenderBatchCommand::Redo()
{
    renderObject->RemoveRenderBatch(renderBatch);
}

void DeleteRenderBatchCommand::Undo()
{
    renderObject->AddRenderBatch(renderBatch, lodIndex, switchIndex);
}

Entity* DeleteRenderBatchCommand::GetEntity() const
{
    return entity;
}

RenderBatch* DeleteRenderBatchCommand::GetRenderBatch() const
{
    return renderBatch;
}

DAVA_VIRTUAL_REFLECTION_IMPL(DeleteRenderBatchCommand)
{
    ReflectionRegistrator<DeleteRenderBatchCommand>::Begin()
    .End();
}
} // namespace DAVA
