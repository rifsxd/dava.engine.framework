#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class PolygonGroup;
class RenderBatch;
class RebuildTangentSpaceCommand : public RECommand
{
public:
    RebuildTangentSpaceCommand(RenderBatch* renderBatch, bool computeBinormal);
    virtual ~RebuildTangentSpaceCommand();

    void Undo() override;
    void Redo() override;

protected:
    RenderBatch* renderBatch;
    PolygonGroup* originalGroup;
    int32 materialBinormalFlagState;
    bool computeBinormal;

    DAVA_VIRTUAL_REFLECTION(RebuildTangentSpaceCommand, RECommand);
};
} // namespace DAVA