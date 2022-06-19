#include "REPlatform/Commands/RebuildTangentSpaceCommand.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/3D/MeshUtils.h>
#include <Render/Highlevel/RenderBatch.h>

namespace DAVA
{
RebuildTangentSpaceCommand::RebuildTangentSpaceCommand(RenderBatch* _renderBatch, bool _computeBinormal)
    : RECommand("Rebuild Tangent Space")
    , renderBatch(_renderBatch)
    , computeBinormal(_computeBinormal)
{
    DVASSERT(renderBatch);
    PolygonGroup* srcGroup = renderBatch->GetPolygonGroup();
    DVASSERT(srcGroup);
    originalGroup = new PolygonGroup();
    MeshUtils::CopyGroupData(srcGroup, originalGroup);
}

RebuildTangentSpaceCommand::~RebuildTangentSpaceCommand()
{
    SafeRelease(originalGroup);
}

void RebuildTangentSpaceCommand::Redo()
{
    MeshUtils::RebuildMeshTangentSpace(renderBatch->GetPolygonGroup(), computeBinormal);
}

void RebuildTangentSpaceCommand::Undo()
{
    MeshUtils::CopyGroupData(originalGroup, renderBatch->GetPolygonGroup());
}

DAVA_VIRTUAL_REFLECTION_IMPL(RebuildTangentSpaceCommand)
{
    ReflectionRegistrator<RebuildTangentSpaceCommand>::Begin()
    .End();
}
} // namespace DAVA
