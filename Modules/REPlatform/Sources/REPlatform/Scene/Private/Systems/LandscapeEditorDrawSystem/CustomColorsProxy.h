#pragma once

#include <Base/BaseObject.h>
#include <Base/BaseTypes.h>
#include <Math/Rect.h>

namespace DAVA
{
class Texture;
class NMaterial;
class CustomColorsProxy : public BaseObject
{
protected:
    ~CustomColorsProxy();

public:
    CustomColorsProxy(int32 size);

    Texture* GetTexture();
    void UpdateRect(const Rect& rect);

    void ResetTargetChanged();
    bool IsTargetChanged();

    void ResetLoadedState(bool isLoaded = true);
    bool IsTextureLoaded() const;

    Rect GetChangedRect();

    int32 GetChangesCount() const;
    void ResetChanges();
    void IncrementChanges();
    void DecrementChanges();

    void UpdateSpriteFromConfig();

    NMaterial* GetBrushMaterial() const;

protected:
    Texture* customColorsRenderTarget;
    Rect changedRect;
    bool spriteChanged;
    bool textureLoaded;
    int32 size;

    int32 changes;

    ScopedPtr<NMaterial> brushMaterial;
};
} // namespace DAVA
