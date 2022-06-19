#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/FastName.h>
#include <Math/Color.h>
#include <Math/Rect.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class LandscapeProxy;
class SceneEditor2;
class Entity;
class Image;
class Texture;

class ModifyTilemaskCommand : public RECommand
{
public:
    ModifyTilemaskCommand(LandscapeProxy* landscapeProxy, const Rect& updatedRect);
    ~ModifyTilemaskCommand() override;

    void Undo() override;
    void Redo() override;

protected:
    Image* undoImageMask = nullptr;
    Image* redoImageMask = nullptr;
    LandscapeProxy* landscapeProxy = nullptr;
    Rect updatedRect;

    void ApplyImageToTexture(Image* image, Texture* dstTex);

    DAVA_VIRTUAL_REFLECTION(ModifyTilemaskCommand, RECommand);
};

class SetTileColorCommand : public RECommand
{
public:
    SetTileColorCommand(LandscapeProxy* landscapeProxy, const FastName& level, const Color& color);
    ~SetTileColorCommand() override;

    void Undo() override;
    void Redo() override;

protected:
    const FastName& level;
    Color redoColor;
    Color undoColor;
    LandscapeProxy* landscapeProxy = nullptr;

    DAVA_VIRTUAL_REFLECTION(SetTileColorCommand, RECommand);
};
}
