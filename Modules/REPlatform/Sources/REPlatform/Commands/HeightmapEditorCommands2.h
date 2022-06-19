#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Math/Rect.h>
#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Heightmap;
class Entity;
class LandscapeEditorDrawSystem;
class ModifyHeightmapCommand : public RECommand
{
public:
    ModifyHeightmapCommand(LandscapeEditorDrawSystem* drawSystem, Heightmap* originalHeightmap, const Rect& updatedRect);
    ~ModifyHeightmapCommand() override;

private:
    void Redo() override;
    void Undo() override;

    uint16* GetHeightmapRegion(Heightmap* heightmap);
    void ApplyHeightmapRegion(uint16* region);

private:
    LandscapeEditorDrawSystem* drawSystem = nullptr;

    uint16* undoRegion = nullptr;
    uint16* redoRegion = nullptr;
    Rect updatedRect;

    DAVA_VIRTUAL_REFLECTION(ModifyHeightmapCommand, RECommand);
};
} // namespace DAVA
