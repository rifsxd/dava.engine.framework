#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class RenderObject;
class RenderBatch;
class CloneLastBatchCommand : public RECommand
{
public:
    CloneLastBatchCommand(RenderObject* renderObject);
    virtual ~CloneLastBatchCommand();

    void Undo() override;
    void Redo() override;

    inline const Vector<RenderBatch*>& GetNewBatches() const;

protected:
    RenderObject* renderObject;
    int32 maxLodIndexes[2];

    int32 requestedSwitchIndex;
    Vector<RenderBatch*> newBatches;

    DAVA_VIRTUAL_REFLECTION(CloneLastBatchCommand, RECommand);
};

inline const Vector<RenderBatch*>& CloneLastBatchCommand::GetNewBatches() const
{
    return newBatches;
}
} // namespace DAVA
