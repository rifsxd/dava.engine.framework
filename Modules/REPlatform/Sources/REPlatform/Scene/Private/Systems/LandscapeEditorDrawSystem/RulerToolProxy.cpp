#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/RulerToolProxy.h"

#include <Render/RenderBase.h>
#include <Render/RenderHelper.h>
#include <Render/RHI/rhi_Type.h>
#include <Render/Texture.h>

namespace DAVA
{
RulerToolProxy::RulerToolProxy(int32 size)
    : size(size)
    , spriteChanged(false)
{
    uint32 unsignedSize = static_cast<uint32>(size);
    rulerToolTexture = Texture::CreateFBO(unsignedSize, unsignedSize, FORMAT_RGBA8888);

    rhi::Viewport viewport;
    viewport.x = viewport.y = 0U;
    viewport.width = unsignedSize;
    viewport.height = unsignedSize;
    RenderHelper::CreateClearPass(rulerToolTexture->handle, rhi::HTexture(), PRIORITY_CLEAR, Color(0.f, 0.f, 0.f, 0.f), viewport);
}

RulerToolProxy::~RulerToolProxy()
{
    SafeRelease(rulerToolTexture);
}

int32 RulerToolProxy::GetSize()
{
    return size;
}

Texture* RulerToolProxy::GetTexture()
{
    return rulerToolTexture;
}
} // namespace DAVA
