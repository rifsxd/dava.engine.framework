#include "REPlatform/Commands/BakeTransformCommand.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderObject.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
BakeGeometryCommand::BakeGeometryCommand(Entity* entity_, RenderObject* _object, Matrix4 _transform)
    : RECommand("Bake geometry")
    , entity(entity_)
    , object(_object)
    , transform(_transform)
{
}

BakeGeometryCommand::~BakeGeometryCommand()
{
}

void BakeGeometryCommand::Undo()
{
    if (NULL != object)
    {
        DAVA::Matrix4 undoTransform = transform;
        undoTransform.Inverse();

        object->BakeGeometry(undoTransform);
    }
}

void BakeGeometryCommand::Redo()
{
    if (NULL != object)
    {
        object->BakeGeometry(transform);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(BakeGeometryCommand)
{
    ReflectionRegistrator<BakeGeometryCommand>::Begin()
    .End();
}
} // namespace DAVA
