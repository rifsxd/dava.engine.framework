#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class LodComponent;
class RenderBatch;
class CopyLastLODToLod0Command : public RECommand
{
public:
    //TODO: remove after lod editing implementation
    DAVA_DEPRECATED(CopyLastLODToLod0Command(LodComponent* lod));
    ~CopyLastLODToLod0Command();

    void Undo() override;
    void Redo() override;
    Entity* GetEntity() const;

    LodComponent* lodComponent;
    Vector<RenderBatch*> newBatches;
    Vector<int32> switchIndices;

private:
    DAVA_VIRTUAL_REFLECTION(CopyLastLODToLod0Command, RECommand);
};
} // namespace DAVA
