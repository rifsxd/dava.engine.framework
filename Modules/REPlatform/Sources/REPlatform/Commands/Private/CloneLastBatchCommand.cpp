#include "REPlatform/Commands/CloneLastBatchCommand.h"

#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
CloneLastBatchCommand::CloneLastBatchCommand(RenderObject* ro)
    : RECommand("Clone Last Batch")
{
    DVASSERT(ro);
    renderObject = SafeRetain(ro);

    // find proper LOD and switch indexes and last batches
    maxLodIndexes[0] = maxLodIndexes[1] = -1;
    RenderBatch* lastBatches[2] = { NULL, NULL };

    const uint32 count = renderObject->GetRenderBatchCount();
    for (uint32 i = 0; i < count; ++i)
    {
        int32 lod, sw;
        RenderBatch* batch = renderObject->GetRenderBatch(i, lod, sw);

        DVASSERT(sw < 2);
        if ((lod > maxLodIndexes[sw]) && (sw >= 0 && sw < 2))
        {
            maxLodIndexes[sw] = lod;
            lastBatches[sw] = batch;
        }
    }
    DVASSERT(maxLodIndexes[0] != maxLodIndexes[1]);

    //detect switch index to clone batches
    requestedSwitchIndex = 0;
    int32 maxIndex = maxLodIndexes[1];
    if (maxLodIndexes[0] > maxLodIndexes[1])
    {
        requestedSwitchIndex = 1;
        maxIndex = maxLodIndexes[0];
    }

    //clone batches
    for (int32 i = maxLodIndexes[requestedSwitchIndex]; i < maxIndex; ++i)
    {
        newBatches.push_back(lastBatches[requestedSwitchIndex]->Clone());
    }
}

CloneLastBatchCommand::~CloneLastBatchCommand()
{
    SafeRelease(renderObject);

    for_each(newBatches.begin(), newBatches.end(), SafeRelease<RenderBatch>);
    newBatches.clear();
}

void CloneLastBatchCommand::Redo()
{
    const uint32 count = (uint32)newBatches.size();
    int32 lodIndex = maxLodIndexes[requestedSwitchIndex] + 1;
    for (uint32 i = 0; i < count; ++i)
    {
        renderObject->AddRenderBatch(newBatches[i], lodIndex, requestedSwitchIndex);
        ++lodIndex;
    }
}

void CloneLastBatchCommand::Undo()
{
    const int32 count = (int32)renderObject->GetRenderBatchCount();
    for (int32 i = count - 1; i >= 0; --i)
    {
        int32 lod, sw;
        renderObject->GetRenderBatch(i, lod, sw);

        if ((sw == requestedSwitchIndex) && (lod > maxLodIndexes[requestedSwitchIndex]))
        {
            renderObject->RemoveRenderBatch(i);
        }
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(CloneLastBatchCommand)
{
    ReflectionRegistrator<CloneLastBatchCommand>::Begin()
    .End();
}
} // namespace DAVA
