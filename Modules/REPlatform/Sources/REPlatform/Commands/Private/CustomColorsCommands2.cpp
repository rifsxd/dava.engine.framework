#include "REPlatform/Commands/CustomColorsCommands2.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/CustomColorsProxy.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/Image/Image.h>
#include <Render/Texture.h>

namespace DAVA
{
ModifyCustomColorsCommand::ModifyCustomColorsCommand(Image* originalImage, Image* currentImage, CustomColorsProxy* customColorsProxy_,
                                                     const Rect& updatedRect_, bool shouldClear)
    : RECommand("Custom Colors Modification")
    , shouldClearTexture(shouldClear)
{
    const Vector2 topLeft(floorf(updatedRect_.x), floorf(updatedRect_.y));
    const Vector2 bottomRight(ceilf(updatedRect_.x + updatedRect_.dx), ceilf(updatedRect_.y + updatedRect_.dy));

    updatedRect = Rect(topLeft, bottomRight - topLeft);

    customColorsProxy = SafeRetain(customColorsProxy_);

    undoImage = Image::CopyImageRegion(originalImage, updatedRect);
    redoImage = Image::CopyImageRegion(currentImage, updatedRect);
}

ModifyCustomColorsCommand::~ModifyCustomColorsCommand()
{
    SafeRelease(undoImage);
    SafeRelease(redoImage);
    SafeRelease(customColorsProxy);
}

void ModifyCustomColorsCommand::Undo()
{
    ApplyImage(undoImage, true);
    customColorsProxy->DecrementChanges();
}

void ModifyCustomColorsCommand::Redo()
{
    ApplyImage(redoImage, false);
    customColorsProxy->IncrementChanges();
}

void ModifyCustomColorsCommand::ApplyImage(Image* image, bool disableBlend)
{
    ScopedPtr<Texture> fboTexture(Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false));

    RenderSystem2D::RenderTargetPassDescriptor desc;

    auto material = disableBlend ? RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL : customColorsProxy->GetBrushMaterial();

    Texture* proxy = customColorsProxy->GetTexture();
    desc.colorAttachment = proxy->handle;
    desc.depthAttachment = proxy->handleDepthStencil;
    desc.width = proxy->GetWidth();
    desc.height = proxy->GetHeight();
    desc.clearTarget = shouldClearTexture;
    desc.transformVirtualToPhysical = false;

    RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
    RenderSystem2D::Instance()->DrawTexture(fboTexture, material, Color::White, updatedRect);
    RenderSystem2D::Instance()->EndRenderTargetPass();

    customColorsProxy->UpdateRect(updatedRect);
}

DAVA_VIRTUAL_REFLECTION_IMPL(ModifyCustomColorsCommand)
{
    ReflectionRegistrator<ModifyCustomColorsCommand>::Begin()
    .End();
}
} // namespace DAVA
