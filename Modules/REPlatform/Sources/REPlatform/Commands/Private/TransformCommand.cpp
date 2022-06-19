#include "REPlatform/Commands/TransformCommand.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
TransformCommand::TransformCommand(Selectable object_, const Transform& origTransform_, const Transform& newTransform_)
    : RECommand("Transform")
    , object(object_)
    , undoTransform(origTransform_)
    , redoTransform(newTransform_)
{
}

void TransformCommand::Undo()
{
    object.SetLocalTransform(undoTransform);
}

void TransformCommand::Redo()
{
    object.SetLocalTransform(redoTransform);
}

const Selectable& TransformCommand::GetTransformedObject() const
{
    return object;
}

Entity* TransformCommand::GetEntity() const
{
    return object.AsEntity();
}

DAVA_VIRTUAL_REFLECTION_IMPL(TransformCommand)
{
    ReflectionRegistrator<TransformCommand>::Begin()
    .End();
}
} // namespace DAVA
