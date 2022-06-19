#include "REPlatform/Commands/TilemaskEditorCommands.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/SceneEditor2.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Image/Image.h>

namespace DAVA
{
ModifyTilemaskCommand::ModifyTilemaskCommand(LandscapeProxy* landscapeProxy_, const Rect& updatedRect_)
    : RECommand("Tile Mask Modification")
    , landscapeProxy(SafeRetain(landscapeProxy_))
{
    updatedRect = Rect(std::floor(updatedRect_.x), std::floor(updatedRect_.y), std::ceil(updatedRect_.dx), std::ceil(updatedRect_.dy));

    undoImageMask = Image::CopyImageRegion(landscapeProxy->GetTilemaskImageCopy(), updatedRect);

    ScopedPtr<Image> currentImageMask(landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILEMASK)->CreateImageFromMemory());
    redoImageMask = Image::CopyImageRegion(currentImageMask, updatedRect);
}

ModifyTilemaskCommand::~ModifyTilemaskCommand()
{
    SafeRelease(undoImageMask);
    SafeRelease(redoImageMask);
    SafeRelease(landscapeProxy);
}

void ModifyTilemaskCommand::Undo()
{
    ApplyImageToTexture(undoImageMask, landscapeProxy->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE));
    ApplyImageToTexture(undoImageMask, landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILEMASK));

    landscapeProxy->DecreaseTilemaskChanges();

    Rect r = Rect(Vector2(0, 0), Vector2(undoImageMask->GetWidth(), undoImageMask->GetHeight()));
    Image* mask = landscapeProxy->GetTilemaskImageCopy();
    mask->InsertImage(undoImageMask, updatedRect.GetPosition(), r);
}

void ModifyTilemaskCommand::Redo()
{
    ApplyImageToTexture(redoImageMask, landscapeProxy->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE));
    ApplyImageToTexture(redoImageMask, landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILEMASK));

    landscapeProxy->IncreaseTilemaskChanges();

    Rect r = Rect(Vector2(0, 0), Vector2(redoImageMask->GetWidth(), redoImageMask->GetHeight()));
    Image* mask = landscapeProxy->GetTilemaskImageCopy();
    mask->InsertImage(redoImageMask, updatedRect.GetPosition(), r);
}

void ModifyTilemaskCommand::ApplyImageToTexture(Image* image, Texture* dstTex)
{
    ScopedPtr<Texture> fboTexture(Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false));

    auto material = RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL;

    RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.colorAttachment = dstTex->handle;
    desc.depthAttachment = dstTex->handleDepthStencil;
    desc.width = dstTex->GetWidth();
    desc.height = dstTex->GetHeight();
    desc.clearTarget = false;
    desc.transformVirtualToPhysical = false;
    RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
    RenderSystem2D::Instance()->DrawTexture(fboTexture, material, Color::White, updatedRect);
    RenderSystem2D::Instance()->EndRenderTargetPass();
}

DAVA_VIRTUAL_REFLECTION_IMPL(ModifyTilemaskCommand)
{
    ReflectionRegistrator<ModifyTilemaskCommand>::Begin()
    .End();
}

SetTileColorCommand::SetTileColorCommand(LandscapeProxy* landscapeProxy_, const FastName& level_, const Color& color_)
    : RECommand("Set tile color")
    , level(level_)
    , redoColor(color_)
    , landscapeProxy(SafeRetain(landscapeProxy_))
{
    undoColor = landscapeProxy->GetLandscapeTileColor(level);
}

SetTileColorCommand::~SetTileColorCommand()
{
    SafeRelease(landscapeProxy);
}

void SetTileColorCommand::Undo()
{
    landscapeProxy->SetLandscapeTileColor(level, undoColor);
}

void SetTileColorCommand::Redo()
{
    landscapeProxy->SetLandscapeTileColor(level, redoColor);
}

DAVA_VIRTUAL_REFLECTION_IMPL(SetTileColorCommand)
{
    ReflectionRegistrator<SetTileColorCommand>::Begin()
    .End();
}
} // namespace DAVA
