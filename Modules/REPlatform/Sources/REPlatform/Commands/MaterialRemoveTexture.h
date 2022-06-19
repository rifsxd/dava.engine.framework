#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/FastName.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class Texture;
class NMaterial;
class MaterialRemoveTexture : public RECommand
{
public:
    MaterialRemoveTexture(const FastName& textureSlot_, NMaterial* material_);
    ~MaterialRemoveTexture() override;

    void Undo() override;
    void Redo() override;

private:
    NMaterial* material = nullptr;
    Texture* texture = nullptr;
    FastName textureSlot;

    DAVA_VIRTUAL_REFLECTION(MaterialRemoveTexture, RECommand);
};
} // namespace DAVA
