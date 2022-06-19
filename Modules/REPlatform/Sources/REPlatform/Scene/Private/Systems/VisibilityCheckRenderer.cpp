#include "REPlatform/Scene/Private/Systems/VisibilityCheckRenderer.h"

#include <Render/ShaderCache.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <UI/UIControlSystem.h>

namespace DAVA
{
const FastName MaterialParamCubemap("cubemap");
const FastName MaterialParamTransformedNormal("transformedNormal");
const FastName MaterialParamPointProperties("pointProperties");
const FastName MaterialParamOrigin("origin");
const FastName MaterialParamFixedFrameMatrix("fixedFrameMatrix");
const FastName MaterialParamCurrentFrameMatrix("currentFrameMatrix");
const FastName MaterialParamCurrentFrameMatrixInverse("currentFrameMatrixInverse");
const FastName MaterialParamFixedFrameTexture("fixedFrame");
const FastName MaterialParamFixedFrameDistancesTexture("fixedFrameDistances");
const FastName MaterialParamCurrentFrameTexture("currentFrame");
const FastName MaterialParamViewportSize("viewportSize");
const FastName MaterialParamCurrentFrameCompleteness("currentFrameCompleteness");
const FastName MaterialParamPixelOffset("pixelOffset");

struct RenderPassScope
{
    rhi::HRenderPass renderPass;
    rhi::HPacketList packetList;

    RenderPassScope(const rhi::RenderPassConfig& config)
    {
        renderPass = rhi::AllocateRenderPass(config, 1, &packetList);
        rhi::BeginRenderPass(renderPass);
        rhi::BeginPacketList(packetList);
    }

    ~RenderPassScope()
    {
        rhi::EndPacketList(packetList);
        rhi::EndRenderPass(renderPass);
    }
};

const float32 VisibilityCheckRenderer::cameraNearClipPlane = 0.1f;

VisibilityCheckRenderer::VisibilityCheckRenderer()
    : cubemapCamera(new Camera())
    , distanceMaterial(new NMaterial())
    , visibilityMaterial(new NMaterial())
    , prerenderMaterial(new NMaterial())
    , reprojectionMaterial(new NMaterial())
{
    cubemapCamera->SetupPerspective(90.0f, 1.0f, cameraNearClipPlane, 5000.0f);

    prerenderConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    prerenderConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    prerenderConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    prerenderConfig.priority = PRIORITY_SERVICE_3D + 1;

    renderTargetConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.priority = PRIORITY_SERVICE_3D;

    rhi::DepthStencilState::Descriptor dsDesc;
    dsDesc.depthFunc = rhi::CMP_EQUAL;
    dsDesc.depthTestEnabled = 1;
    dsDesc.depthWriteEnabled = 0;
    visibilityDepthStencilState = rhi::AcquireDepthStencilState(dsDesc);

    dsDesc.depthFunc = rhi::CMP_LESS;
    dsDesc.depthTestEnabled = 1;
    dsDesc.depthWriteEnabled = 1;
    reprojectionDepthStencilState = rhi::AcquireDepthStencilState(dsDesc);

    visibilityConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    visibilityConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_LOAD;
    visibilityConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    visibilityConfig.priority = PRIORITY_SERVICE_3D - 1;

    reprojectionConfig.priority = PRIORITY_SERVICE_3D - 2;
    reprojectionConfig.colorBuffer[0].clearColor[3] = 1.0f;

    distanceMapConfig.priority = PRIORITY_SERVICE_3D - 1;
    distanceMapConfig.colorBuffer[0].clearColor[0] = 1.0f;
    distanceMapConfig.colorBuffer[0].clearColor[1] = 1.0f;
    distanceMapConfig.colorBuffer[0].clearColor[2] = 1.0f;
    distanceMapConfig.colorBuffer[0].clearColor[3] = 1.0f;
    distanceMapConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    distanceMapConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;

    reprojectionMaterial->SetFXName(FastName("~res:/ResourceEditor/LandscapeEditor/Materials/Distance.Reprojection.material"));
    reprojectionMaterial->AddProperty(MaterialParamCurrentFrameMatrix, Matrix4::IDENTITY.data, rhi::ShaderProp::TYPE_FLOAT4X4);
    reprojectionMaterial->AddProperty(MaterialParamCurrentFrameMatrixInverse, Matrix4::IDENTITY.data, rhi::ShaderProp::TYPE_FLOAT4X4);
    reprojectionMaterial->AddProperty(MaterialParamFixedFrameMatrix, Matrix4::IDENTITY.data, rhi::ShaderProp::TYPE_FLOAT4X4);
    reprojectionMaterial->AddProperty(MaterialParamOrigin, Vector3().data, rhi::ShaderProp::Type::TYPE_FLOAT3);
    reprojectionMaterial->AddProperty(MaterialParamViewportSize, Vector2().data, rhi::ShaderProp::Type::TYPE_FLOAT2);
    reprojectionMaterial->AddProperty(MaterialParamCurrentFrameCompleteness, &frameCompleteness, rhi::ShaderProp::Type::TYPE_FLOAT1);

    Vector2 pixelOffset;
    if (rhi::DeviceCaps().isCenterPixelMapping)
        pixelOffset = Vector2(0.5f, 0.5f);
    reprojectionMaterial->AddProperty(MaterialParamPixelOffset, pixelOffset.data, rhi::ShaderProp::Type::TYPE_FLOAT2);

    prerenderMaterial->SetFXName(FastName("~res:/ResourceEditor/LandscapeEditor/Materials/Distance.Prerender.material"));

    distanceMaterial->SetFXName(FastName("~res:/ResourceEditor/LandscapeEditor/Materials/Distance.Encode.material"));

    visibilityMaterial->SetFXName(FastName("~res:/ResourceEditor/LandscapeEditor/Materials/Distance.Decode.material"));
    visibilityMaterial->AddFlag(NMaterialFlagName::FLAG_BLENDING, BLENDING_ADDITIVE);
    visibilityMaterial->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, Vector4().data, rhi::ShaderProp::TYPE_FLOAT4);
    visibilityMaterial->AddProperty(MaterialParamTransformedNormal, Vector3().data, rhi::ShaderProp::TYPE_FLOAT3);
    visibilityMaterial->AddProperty(MaterialParamPointProperties, Vector3().data, rhi::ShaderProp::TYPE_FLOAT3);
    visibilityMaterial->AddProperty(MaterialParamOrigin, Vector3().data, rhi::ShaderProp::Type::TYPE_FLOAT3);
}

VisibilityCheckRenderer::~VisibilityCheckRenderer()
{
}

void VisibilityCheckRenderer::SetDelegate(VisibilityCheckRendererDelegate* de)
{
    renderDelegate = de;
}

void VisibilityCheckRenderer::RenderToCubemapFromPoint(RenderSystem* renderSystem, const Vector3& point, Texture* cubemapTarget)
{
    renderTargetConfig.colorBuffer[0].texture = cubemapTarget->handle;
    renderTargetConfig.depthStencilBuffer.texture = cubemapTarget->handleDepthStencil;
    renderTargetConfig.viewport.width = cubemapTarget->GetWidth();
    renderTargetConfig.viewport.height = cubemapTarget->GetHeight();

    cubemapCamera->SetPosition(point);
    for (uint32 i = 0; i < 6; ++i)
    {
        SetupCameraToRenderFromPointToFaceIndex(point, i, static_cast<float>(cubemapTarget->width));
        RenderWithCurrentSettings(renderSystem);
    }
}

void VisibilityCheckRenderer::SetupCameraToRenderFromPointToFaceIndex(const Vector3& point, uint32 faceIndex, float32 cubemapSize)
{
    static const Vector3 directions[6] =
    {
      Vector3(1.0f, 0.0f, 0.0f),
      Vector3(-1.0f, 0.0f, 0.0f),
      Vector3(0.0f, 1.0f, 0.0f),
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, 0.0f, 1.0f),
      Vector3(0.0f, 0.0f, -1.0f),
    };
    static const Vector3 upVectors[6] =
    {
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, 0.0f, 1.0f),
      Vector3(0.0f, 0.0f, -1.0f),
      Vector3(0.0f, -1.0f, 0.0f),
      Vector3(0.0f, -1.0f, 0.0f),
    };
    static const rhi::TextureFace targetFaces[6] =
    {
      rhi::TEXTURE_FACE_POSITIVE_X,
      rhi::TEXTURE_FACE_NEGATIVE_X,
      rhi::TEXTURE_FACE_POSITIVE_Y,
      rhi::TEXTURE_FACE_NEGATIVE_Y,
      rhi::TEXTURE_FACE_POSITIVE_Z,
      rhi::TEXTURE_FACE_NEGATIVE_Z,
    };

    renderTargetConfig.colorBuffer[0].textureFace = targetFaces[faceIndex];
    cubemapCamera->SetTarget(point + directions[faceIndex]);
    cubemapCamera->SetUp(upVectors[faceIndex]);

    if (rhi::DeviceCaps().isCenterPixelMapping)
    {
        // See explanation why 1.0 / cubemapSize but not 0.5 / cubemapSize here
        // https://gamedev.stackexchange.com/questions/83191/offset-a-camera-render-without-changing-perspective
        const float offset = 1.0f / cubemapSize;
        const Vector2 projectionOffets[6] =
        {
          Vector2(-offset, -offset),
          Vector2(+offset, -offset),
          Vector2(+offset, -offset),
          Vector2(+offset, +offset),
          Vector2(+offset, -offset),
          Vector2(+offset, -offset),
        };
        cubemapCamera->SetProjectionMatrixOffset(projectionOffets[faceIndex]);
    }
}

bool VisibilityCheckRenderer::ShouldRenderObject(RenderObject* object)
{
    DVASSERT(renderDelegate != nullptr);
    return renderDelegate->ShouldDrawRenderObject(object);
}

bool VisibilityCheckRenderer::ShouldRenderBatch(RenderBatch* batch)
{
    return batch->GetMaterial()->GetEffectiveFXName() != NMaterialName::SKYOBJECT;
}

void VisibilityCheckRenderer::CollectRenderBatches(RenderSystem* renderSystem, Camera* fromCamera, Vector<RenderBatch*>& batches)
{
    ShaderDescriptorCache::ClearDynamicBindigs();
    fromCamera->SetupDynamicParameters(false, nullptr);

    Vector<RenderObject*> renderObjects;
    renderSystem->GetRenderHierarchy()->Clip(fromCamera, renderObjects, RenderObject::VISIBLE);

    for (auto renderObject : renderObjects)
    {
        if (ShouldRenderObject(renderObject))
        {
            if (renderObject->GetFlags() & RenderObject::CUSTOM_PREPARE_TO_RENDER)
            {
                renderObject->PrepareToRender(fromCamera);
            }

            uint32 batchCount = renderObject->GetActiveRenderBatchCount();
            for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
            {
                RenderBatch* batch = renderObject->GetActiveRenderBatch(batchIndex);
                if (ShouldRenderBatch(batch))
                {
                    NMaterial* material = batch->GetMaterial();
                    if ((material != nullptr) && material->PreBuildMaterial(PASS_FORWARD))
                    {
                        if ((material->GetRenderLayerID() == RenderLayer::RENDER_LAYER_OPAQUE_ID) ||
                            (material->GetRenderLayerID() == RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID) ||
                            (material->GetRenderLayerID() == RenderLayer::RENDER_LAYER_ALPHA_TEST_LAYER_ID))
                        {
                            batches.push_back(batch);
                        }
                    }
                }
            }
        }
    }
}

void VisibilityCheckRenderer::PreRenderScene(RenderSystem* renderSystem, Camera* fromCamera)
{
    Vector<RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, fromCamera, renderBatches);

    prerenderMaterial->PreBuildMaterial(PASS_FORWARD);

    prerenderConfig.colorBuffer[0].texture = renderTarget->handle;
    prerenderConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;
    RenderPassScope pass(prerenderConfig);

    fromCamera->SetupDynamicParameters(rhi::NeedInvertProjection(prerenderConfig));
    for (auto batch : renderBatches)
    {
        batch->GetRenderObject()->BindDynamicParameters(fromCamera, batch);
        rhi::Packet packet;
        batch->BindGeometryData(packet);
        prerenderMaterial->BindParams(packet);
        rhi::AddPacket(pass.packetList, packet);
    }
}

void VisibilityCheckRenderer::RenderWithCurrentSettings(RenderSystem* renderSystem)
{
    Vector<RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, cubemapCamera, renderBatches);

    distanceMaterial->PreBuildMaterial(PASS_FORWARD);

    RenderPassScope pass(renderTargetConfig);
    cubemapCamera->SetupDynamicParameters(rhi::NeedInvertProjection(renderTargetConfig));
    for (auto batch : renderBatches)
    {
        batch->GetRenderObject()->BindDynamicParameters(cubemapCamera, batch);
        rhi::Packet packet;
        batch->BindGeometryData(packet);
        distanceMaterial->BindParams(packet);
        packet.cullMode = rhi::CULL_NONE;
        rhi::AddPacket(pass.packetList, packet);
    }
}

void VisibilityCheckRenderer::UpdateVisibilityMaterialProperties(Texture* cubemapTexture, const VisbilityPoint& vp)
{
    if (visibilityMaterial->HasLocalTexture(MaterialParamCubemap))
    {
        visibilityMaterial->SetTexture(MaterialParamCubemap, cubemapTexture);
    }
    else
    {
        visibilityMaterial->AddTexture(MaterialParamCubemap, cubemapTexture);
    }

    Vector3 propValue(vp.downAngleCosine, vp.upAngleCosine, vp.maxDistance);
    visibilityMaterial->SetPropertyValue(MaterialParamPointProperties, propValue.data);
    visibilityMaterial->SetPropertyValue(MaterialParamOrigin, vp.point.data);
    visibilityMaterial->SetPropertyValue(MaterialParamTransformedNormal, vp.normal.data);
    visibilityMaterial->SetPropertyValue(NMaterialParamName::PARAM_FLAT_COLOR, vp.color.color);
    visibilityMaterial->PreBuildMaterial(PASS_FORWARD);
}

void VisibilityCheckRenderer::RenderVisibilityToTexture(RenderSystem* renderSystem, Camera* batchesCamera,
                                                        Camera* fromCamera, Texture* cubemap, const VisbilityPoint& vp)
{
    Vector<RenderBatch*> renderBatches;
    UpdateVisibilityMaterialProperties(cubemap, vp);
    CollectRenderBatches(renderSystem, batchesCamera, renderBatches);

    visibilityConfig.colorBuffer[0].texture = renderTarget->handle;
    visibilityConfig.depthStencilBuffer.texture = renderTarget->handleDepthStencil;

    ShaderDescriptorCache::ClearDynamicBindigs();
    fromCamera->SetupDynamicParameters(rhi::NeedInvertProjection(visibilityConfig));

    RenderPassScope pass(visibilityConfig);
    for (auto batch : renderBatches)
    {
        batch->GetRenderObject()->BindDynamicParameters(fromCamera, batch);
        rhi::Packet packet;
        batch->BindGeometryData(packet);
        visibilityMaterial->BindParams(packet);
        packet.depthStencilState = visibilityDepthStencilState;
        rhi::AddPacket(pass.packetList, packet);
    }
}

void VisibilityCheckRenderer::InvalidateMaterials()
{
    distanceMaterial->InvalidateRenderVariants();
    distanceMaterial->InvalidateBufferBindings();

    visibilityMaterial->InvalidateRenderVariants();
    visibilityMaterial->InvalidateBufferBindings();

    prerenderMaterial->InvalidateRenderVariants();
    prerenderMaterial->InvalidateBufferBindings();

    reprojectionMaterial->InvalidateRenderVariants();
    reprojectionMaterial->InvalidateBufferBindings();
}

void VisibilityCheckRenderer::FixFrame()
{
    shouldFixFrame = true;
}

void VisibilityCheckRenderer::ReleaseFrame()
{
    frameFixed = false;
    shouldFixFrame = false;
}

void VisibilityCheckRenderer::CreateOrUpdateRenderTarget(const Size2i& sz)
{
    if ((renderTarget == nullptr) || (renderTarget->GetWidth() != sz.dx) || (renderTarget->GetHeight() != sz.dy))
    {
        SafeRelease(renderTarget);
        renderTarget = Texture::CreateFBO(sz.dx, sz.dy, PixelFormat::FORMAT_RGBA8888, true, rhi::TEXTURE_TYPE_2D, false);
        renderTarget->SetMinMagFilter(rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureMipFilter::TEXMIPFILTER_NONE);
    }
}

namespace VCRLocal
{
inline void PutTexture(NMaterial* mat, const FastName& slot, Texture* tex)
{
    if (mat->HasLocalTexture(slot))
    {
        mat->SetTexture(slot, tex);
    }
    else
    {
        mat->AddTexture(slot, tex);
    }
}
}

void VisibilityCheckRenderer::FixFrame(RenderSystem* renderSystem, Camera* fromCamera)
{
    SafeRelease(fixedFrame);
    SafeRelease(reprojectionTexture);
    SafeRelease(distanceRenderTarget);

    auto rs2d = RenderSystem2D::Instance();
    uint32 w = renderTarget->GetWidth();
    uint32 h = renderTarget->GetHeight();

    fixedFrame = Texture::CreateFBO(w, h, PixelFormat::FORMAT_RGBA8888, false, rhi::TextureType::TEXTURE_TYPE_2D, false);
    fixedFrame->SetMinMagFilter(rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureMipFilter::TEXMIPFILTER_NONE);
    fromCamera->PrepareDynamicParameters(rhi::NeedInvertProjection(visibilityConfig));
    fixedFrameMatrix = fromCamera->GetViewProjMatrix();
    fixedFrameCameraPosition = fromCamera->GetPosition();
    frameCompleteness = 1.0f;

    RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.clearColor = Color::Clear;
    desc.colorAttachment = fixedFrame->handle;
    desc.depthAttachment = fixedFrame->handleDepthStencil;
    desc.transformVirtualToPhysical = false;
    rs2d->BeginRenderTargetPass(desc);
    rs2d->DrawTexture(renderTarget, RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL, Color::White);
    rs2d->EndRenderTargetPass();

    reprojectionTexture = Texture::CreateFBO(w, h, PixelFormat::FORMAT_RGBA8888, true, rhi::TextureType::TEXTURE_TYPE_2D, false);
    reprojectionConfig.colorBuffer[0].texture = reprojectionTexture->handle;
    reprojectionConfig.depthStencilBuffer.texture = reprojectionTexture->handleDepthStencil;

    distanceRenderTarget = Texture::CreateFBO(w, h, TEXTURE_FORMAT, true, rhi::TEXTURE_TYPE_2D, false);
    distanceRenderTarget->SetMinMagFilter(rhi::TextureFilter::TEXFILTER_NEAREST, rhi::TextureFilter::TEXFILTER_NEAREST, rhi::TextureMipFilter::TEXMIPFILTER_NONE);
    distanceMapConfig.colorBuffer[0].texture = distanceRenderTarget->handle;
    distanceMapConfig.depthStencilBuffer.texture = distanceRenderTarget->handleDepthStencil;

    RenderToDistanceMapFromCamera(renderSystem, fromCamera);

    shouldFixFrame = false;
    frameFixed = true;
}

void VisibilityCheckRenderer::RenderToDistanceMapFromCamera(RenderSystem* renderSystem, Camera* fromCamera)
{
    Vector<RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, fromCamera, renderBatches);

    distanceMaterial->PreBuildMaterial(PASS_FORWARD);

    fromCamera->SetupDynamicParameters(rhi::NeedInvertProjection(distanceMapConfig));
    RenderPassScope pass(distanceMapConfig);
    for (auto batch : renderBatches)
    {
        rhi::Packet packet;
        batch->GetRenderObject()->BindDynamicParameters(fromCamera, batch);
        batch->BindGeometryData(packet);
        distanceMaterial->BindParams(packet);
        rhi::AddPacket(pass.packetList, packet);
    }
}

void VisibilityCheckRenderer::RenderCurrentOverlayTexture(RenderSystem* renderSystem, Camera* camera)
{
    auto rs2d = RenderSystem2D::Instance();
    if (frameFixed)
    {
        RenderWithReprojection(renderSystem, camera);
        rs2d->DrawTexture(reprojectionTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL, Color::White);
    }
    else
    {
        rs2d->DrawTexture(renderTarget, RenderSystem2D::DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL, Color::White);
    }

    if (shouldFixFrame)
    {
        FixFrame(renderSystem, camera);
    }
}

void VisibilityCheckRenderer::RenderProgress(float ratio, const Color& clr)
{
    frameCompleteness = ratio;
    auto rs2d = RenderSystem2D::Instance();
    float32 width = static_cast<float32>(Renderer::GetFramebufferWidth());
    rs2d->FillRect(Rect(0.0f, 0.0f, frameCompleteness * width, 5.0f), clr);
}

void VisibilityCheckRenderer::RenderWithReprojection(RenderSystem* renderSystem, Camera* fromCamera)
{
    Vector<RenderBatch*> renderBatches;
    CollectRenderBatches(renderSystem, fromCamera, renderBatches);

    Vector2 vpSize(static_cast<float>(reprojectionTexture->GetWidth()), static_cast<float>(reprojectionTexture->GetHeight()));
    reprojectionMaterial->SetPropertyValue(MaterialParamOrigin, fixedFrameCameraPosition.data);
    reprojectionMaterial->SetPropertyValue(MaterialParamFixedFrameMatrix, fixedFrameMatrix.data);
    reprojectionMaterial->SetPropertyValue(MaterialParamViewportSize, vpSize.data);
    reprojectionMaterial->SetPropertyValue(MaterialParamCurrentFrameCompleteness, &frameCompleteness);
    VCRLocal::PutTexture(reprojectionMaterial, MaterialParamFixedFrameTexture, fixedFrame);
    VCRLocal::PutTexture(reprojectionMaterial, MaterialParamFixedFrameDistancesTexture, distanceRenderTarget);
    VCRLocal::PutTexture(reprojectionMaterial, MaterialParamCurrentFrameTexture, renderTarget);
    reprojectionMaterial->PreBuildMaterial(PASS_FORWARD);

    fromCamera->SetupDynamicParameters(rhi::NeedInvertProjection(reprojectionConfig));
    RenderPassScope pass(reprojectionConfig);
    for (auto batch : renderBatches)
    {
        rhi::Packet packet;
        batch->GetRenderObject()->BindDynamicParameters(fromCamera, batch);
        batch->BindGeometryData(packet);
        reprojectionMaterial->BindParams(packet);
        packet.depthStencilState = reprojectionDepthStencilState;
        rhi::AddPacket(pass.packetList, packet);
    }
}
} // namespace DAVA