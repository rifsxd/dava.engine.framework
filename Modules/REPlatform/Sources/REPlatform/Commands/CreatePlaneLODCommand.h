#pragma once

#include "REPlatform/Commands/RECommand.h"
#include "REPlatform/Commands/CreatePlaneLODCommandHelper.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class CreatePlaneLODCommand : public RECommand
{
public:
    CreatePlaneLODCommand(const CreatePlaneLODCommandHelper::RequestPointer& request);

    void Undo() override;
    void Redo() override;
    Entity* GetEntity() const;

    RenderBatch* GetRenderBatch() const;

protected:
    void CreateTextureFiles();

private:
    CreatePlaneLODCommandHelper::RequestPointer request;

    DAVA_VIRTUAL_REFLECTION(CreatePlaneLODCommand, RECommand);
};
} // namespace DAVA
