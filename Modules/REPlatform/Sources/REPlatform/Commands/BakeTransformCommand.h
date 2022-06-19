#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Math/Matrix4.h>
#include <Render/Highlevel/RenderObject.h>

namespace DAVA
{
class RenderObject;
class BakeGeometryCommand : public RECommand
{
public:
    BakeGeometryCommand(Entity* entity, RenderObject* _object, Matrix4 _transform);
    ~BakeGeometryCommand();

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const
    {
        return entity;
    }

protected:
    Entity* entity;
    RenderObject* object;
    Matrix4 transform;

    DAVA_VIRTUAL_REFLECTION(BakeGeometryCommand, RECommand);
};
} // namespace DAVA
