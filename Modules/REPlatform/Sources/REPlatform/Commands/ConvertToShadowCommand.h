#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class RenderBatch;
class RenderObject;
class ConvertToShadowCommand : public RECommand
{
public:
    ConvertToShadowCommand(Entity* entity, RenderBatch* batch);
    ~ConvertToShadowCommand();

    void Undo() override;
    void Redo() override;
    Entity* GetEntity() const;

    static bool CanConvertBatchToShadow(RenderBatch* renderBatch);

    Entity* entity;
    RenderObject* renderObject;
    RenderBatch* oldBatch;
    RenderBatch* newBatch;

private:
    DAVA_VIRTUAL_REFLECTION(ConvertToShadowCommand, RECommand);
};
} // namespace DAVA
