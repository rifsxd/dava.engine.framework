#include "REPlatform/Commands/DeleteLODCommand.h"
#include "REPlatform/Commands/DeleteRenderBatchCommand.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderObject.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Lod/LodComponent.h>

namespace DAVA
{
DeleteLODCommand::DeleteLODCommand(LodComponent* lod, int32 lodIndex, int32 switchIndex)
    : RECommand("Delete LOD")
    , lodComponent(lod)
    , deletedLodIndex(lodIndex)
    , requestedSwitchIndex(switchIndex)
{
    DVASSERT(lodComponent);
    Entity* entity = GetEntity();
    RenderObject* ro = GetRenderObject(entity);

    DVASSERT(ro);
    DVASSERT(ro->GetType() != RenderObject::TYPE_PARTICLE_EMITTER);

    //save renderBatches
    int32 count = (int32)ro->GetRenderBatchCount();
    for (int32 i = count - 1; i >= 0; --i)
    {
        int32 batchLodIndex = 0, batchSwitchIndex = 0;
        ro->GetRenderBatch(i, batchLodIndex, batchSwitchIndex);
        if (batchLodIndex == deletedLodIndex && (requestedSwitchIndex == batchSwitchIndex || requestedSwitchIndex == -1))
        {
            DeleteRenderBatchCommand* command = new DeleteRenderBatchCommand(entity, ro, i);
            deletedBatches.push_back(command);
        }
    }
}

DeleteLODCommand::~DeleteLODCommand()
{
    uint32 count = (uint32)deletedBatches.size();
    for (uint32 i = 0; i < count; ++i)
    {
        SafeDelete(deletedBatches[i]);
    }
    deletedBatches.clear();
}

void DeleteLODCommand::Redo()
{
    for (DeleteRenderBatchCommand* command : deletedBatches)
    {
        command->Redo();
    }

    //update indexes
    RenderObject* ro = GetRenderObject(GetEntity());
    int32 count = ro->GetRenderBatchCount();
    for (int32 i = (int32)count - 1; i >= 0; --i)
    {
        int32 lodIndex = 0, switchIndex = 0;
        RenderBatch* batch = ro->GetRenderBatch(i, lodIndex, switchIndex);
        if (lodIndex > deletedLodIndex && (requestedSwitchIndex == switchIndex || requestedSwitchIndex == -1))
        {
            batch->Retain();

            ro->RemoveRenderBatch(i);
            ro->AddRenderBatch(batch, lodIndex - 1, switchIndex);

            batch->Release();
        }
    }
}

void DeleteLODCommand::Undo()
{
    RenderObject* ro = GetRenderObject(GetEntity());

    //restore lodindexes
    uint32 count = ro->GetRenderBatchCount();
    for (int32 i = (int32)count - 1; i >= 0; --i)
    {
        int32 lodIndex = 0, switchIndex = 0;
        RenderBatch* batch = ro->GetRenderBatch(i, lodIndex, switchIndex);
        if (lodIndex >= deletedLodIndex && (requestedSwitchIndex == switchIndex || requestedSwitchIndex == -1))
        {
            batch->Retain();

            ro->RemoveRenderBatch(i);
            ro->AddRenderBatch(batch, lodIndex + 1, switchIndex);

            batch->Release();
        }
    }

    //restore batches
    count = (uint32)deletedBatches.size();
    for (uint32 i = 0; i < count; ++i)
    {
        deletedBatches[i]->Undo();
    }
}

Entity* DeleteLODCommand::GetEntity() const
{
    return lodComponent->GetEntity();
}

const Vector<DeleteRenderBatchCommand*>& DeleteLODCommand::GetRenderBatchCommands() const
{
    return deletedBatches;
}

DAVA_VIRTUAL_REFLECTION_IMPL(DeleteLODCommand)
{
    ReflectionRegistrator<DeleteLODCommand>::Begin()
    .End();
}
} // namespace DAVA
