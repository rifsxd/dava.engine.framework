#pragma once

#include <Base/Noncopyable.h>
#include <Render/Highlevel/RenderPass.h>
#include <Render/RenderBase.h>
#include <Render/Texture.h>

namespace DAVA
{
struct VisibilityCheckRendererDelegate
{
    virtual ~VisibilityCheckRendererDelegate() = default;
    virtual bool ShouldDrawRenderObject(RenderObject*) = 0;
};

class VisibilityCheckRenderer : public Noncopyable
{
public:
    struct VisbilityPoint
    {
        Vector3 point;
        Vector3 normal;
        Color color;
        float32 upAngleCosine;
        float32 downAngleCosine;
        float32 maxDistance;
        VisbilityPoint(const Vector3& p, const Vector3& n, const Color& clr,
                       float32 upAngle, float32 downAngle, float32 md)
            : point(p)
            , normal(n)
            , color(clr)
            , upAngleCosine(upAngle)
            , downAngleCosine(downAngle)
            , maxDistance(md)
        {
        }
    };

    static const float32 cameraNearClipPlane;
    static const PixelFormat TEXTURE_FORMAT = PixelFormat::FORMAT_RG32F;

public:
    VisibilityCheckRenderer();
    ~VisibilityCheckRenderer();

    void SetDelegate(VisibilityCheckRendererDelegate*);

    void PreRenderScene(RenderSystem* renderSystem, Camera* camera);

    void RenderToCubemapFromPoint(RenderSystem* renderSystem, const Vector3& point, Texture* cubemapTarget);

    void RenderVisibilityToTexture(RenderSystem* renderSystem, Camera* batchesCamera,
                                   Camera* drawCamera, Texture* cubemap, const VisbilityPoint& vp);

    void RenderCurrentOverlayTexture(RenderSystem* renderSystem, Camera* camera);
    void RenderProgress(float, const Color&);

    void InvalidateMaterials();

    void FixFrame();
    void ReleaseFrame();

    void CreateOrUpdateRenderTarget(const Size2i&);

    bool FrameFixed() const;

private:
    void SetupCameraToRenderFromPointToFaceIndex(const Vector3& point, uint32 faceIndex, float32 cubemapSize);
    void RenderWithCurrentSettings(RenderSystem* renderSystem);
    bool ShouldRenderObject(RenderObject*);
    bool ShouldRenderBatch(RenderBatch*);

    void CollectRenderBatches(RenderSystem* renderSystem, Camera* fromCamera, Vector<RenderBatch*>& batches);

    void UpdateVisibilityMaterialProperties(Texture* cubemapTexture, const VisbilityPoint& vp);

    void RenderToDistanceMapFromCamera(RenderSystem* renderSystem, Camera* fromCamera);
    void RenderWithReprojection(RenderSystem* renderSystem, Camera* fromCamera);

    void FixFrame(RenderSystem* renderSystem, Camera* fromCamera);

private:
    VisibilityCheckRendererDelegate* renderDelegate = nullptr;
    ScopedPtr<Camera> cubemapCamera;
    ScopedPtr<NMaterial> distanceMaterial;
    ScopedPtr<NMaterial> visibilityMaterial;
    ScopedPtr<NMaterial> prerenderMaterial;
    ScopedPtr<NMaterial> reprojectionMaterial;
    rhi::HDepthStencilState visibilityDepthStencilState;
    rhi::HDepthStencilState reprojectionDepthStencilState;
    rhi::RenderPassConfig renderTargetConfig;
    rhi::RenderPassConfig visibilityConfig;
    rhi::RenderPassConfig prerenderConfig;
    rhi::RenderPassConfig reprojectionConfig;
    rhi::RenderPassConfig distanceMapConfig;
    Matrix4 fixedFrameMatrix;
    Texture* renderTarget = nullptr;
    Texture* distanceRenderTarget = nullptr;
    Texture* fixedFrame = nullptr;
    Texture* reprojectionTexture = nullptr;
    Vector3 fixedFrameCameraPosition;
    float frameCompleteness = 0.0f;
    bool frameFixed = false;
    bool shouldFixFrame = false;

    friend class VisibilityCheckSystem;
};

inline bool VisibilityCheckRenderer::FrameFixed() const
{
    return frameFixed;
}
} // namespace DAVA
