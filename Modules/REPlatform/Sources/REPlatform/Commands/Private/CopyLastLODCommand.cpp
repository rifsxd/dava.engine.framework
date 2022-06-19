#include "REPlatform/Commands/CopyLastLODCommand.h"

#include <Base/BaseObject.h>
#include <Entity/Component.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderObject.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Lod/LodComponent.h>

namespace DAVA
{
CopyLastLODToLod0Command::CopyLastLODToLod0Command(LodComponent* component)
    : RECommand("Copy last LOD to lod0")
    , lodComponent(component)
{
    RenderObject* ro = GetRenderObject(GetEntity());
    DVASSERT(ro);

    uint32 maxLodIndex = ro->GetMaxLodIndex();
    uint32 batchCount = ro->GetRenderBatchCount();
    int32 lodIndex, switchIndex;
    for (uint32 ri = 0; ri < batchCount; ++ri)
    {
        RenderBatch* batch = ro->GetRenderBatch(ri, lodIndex, switchIndex);
        if (lodIndex == maxLodIndex)
        {
            RenderBatch* newBatch = batch->Clone();
            newBatches.push_back(newBatch);
            switchIndices.push_back(switchIndex);
        }
    }
}

CopyLastLODToLod0Command::~CopyLastLODToLod0Command()
{
    for (RenderBatch* batch : newBatches)
    {
        SafeRelease(batch);
    }

    newBatches.clear();
    switchIndices.clear();
}

void CopyLastLODToLod0Command::Redo()
{
    if (!lodComponent)
        return;

    RenderObject* ro = GetRenderObject(GetEntity());
    DVASSERT(ro);

    uint32 batchCount = ro->GetRenderBatchCount();
    int32 lodIndex, switchIndex;
    for (uint32 ri = 0; ri < batchCount; ++ri)
    {
        ro->GetRenderBatch(ri, lodIndex, switchIndex);
        ro->SetRenderBatchLODIndex(ri, lodIndex + 1);
    }

    size_t newBatchCount = newBatches.size();
    for (size_t ri = 0; ri < newBatchCount; ++ri)
    {
        ro->AddRenderBatch(newBatches[ri], 0, switchIndices[ri]);
    }
}

void CopyLastLODToLod0Command::Undo()
{
    if (!lodComponent)
        return;

    RenderObject* ro = GetRenderObject(GetEntity());
    DVASSERT(ro);

    size_t newBatchCount = newBatches.size();
    for (size_t ri = 0; ri < newBatchCount; ++ri)
    {
        ro->RemoveRenderBatch(newBatches[ri]);
    }

    uint32 batchCount = ro->GetRenderBatchCount();
    int32 lodIndex, switchIndex;
    for (uint32 ri = 0; ri < batchCount; ++ri)
    {
        ro->GetRenderBatch(ri, lodIndex, switchIndex);
        ro->SetRenderBatchLODIndex(ri, lodIndex - 1);
    }
}

Entity* CopyLastLODToLod0Command::GetEntity() const
{
    if (lodComponent)
        return lodComponent->GetEntity();

    return nullptr;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CopyLastLODToLod0Command)
{
    ReflectionRegistrator<CopyLastLODToLod0Command>::Begin()
    .End();
}
} // namespace DAVA
