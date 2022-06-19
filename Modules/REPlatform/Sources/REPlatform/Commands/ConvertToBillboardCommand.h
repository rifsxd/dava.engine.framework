#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class RenderObject;
class RenderSystem;
class BillboardRenderObject;
class RenderComponent;
class Component;
class Entity;

class ConvertToBillboardCommand : public RECommand
{
public:
    ConvertToBillboardCommand(RenderObject*, Entity*);
    ~ConvertToBillboardCommand();

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;

private:
    Entity* entity = nullptr;
    RenderComponent* oldRenderComponent = nullptr;
    RenderComponent* newRenderComponent = nullptr;
    Component* detachedComponent = nullptr;

    DAVA_VIRTUAL_REFLECTION(ConvertToBillboardCommand, RECommand);
};

inline Entity* ConvertToBillboardCommand::GetEntity() const
{
    return entity;
}
} // namespace DAVA
