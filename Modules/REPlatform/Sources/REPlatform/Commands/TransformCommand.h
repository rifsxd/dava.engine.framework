#pragma once

#include "REPlatform/Commands/RECommand.h"
#include "REPlatform/DataNodes/Selectable.h"

#include <Reflection/Reflection.h>
#include <Math/Transform.h>

namespace DAVA
{
class TransformCommand : public RECommand
{
public:
    TransformCommand(Selectable object, const Transform& origTransform, const Transform& newTransform);

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;
    const Selectable& GetTransformedObject() const;

protected:
    Selectable object;
    Transform undoTransform;
    Transform redoTransform;

    DAVA_VIRTUAL_REFLECTION(TransformCommand, RECommand);
};
} // namespace DAVA
