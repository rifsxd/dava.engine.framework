#pragma once

#include <Base/BaseTypes.h>
#include <Base/TypeHolders.h>
#include <Concurrency/Atomic.h>
#include <Render/RenderBase.h>
#include <Render/RHI/rhi_Public.h>
#include <Scene3D/Lod/LodComponent.h>

namespace DAVA
{
class LodComponent;
class RenderBatch;
class Image;
class Texture;

namespace CreatePlaneLODCommandHelper
{
struct Request : public RefCounter
{
    LodComponent* lodComponent = nullptr;
    RenderBatch* planeBatch = nullptr;
    Image* planeImage = nullptr;
    Texture* targetTexture = nullptr;
    int32 fromLodLayer = 0;
    int32 newLodIndex = 0;
    uint32 textureSize = 0;
    FilePath texturePath;
    Atomic<bool> completed;
    rhi::HTexture depthTexture;

    Request();
    ~Request();
    void RegisterRenderCallback();
    void OnRenderCallback(rhi::HSyncObject object);
    void ReloadTexturesToGPU(eGPUFamily);
};
using RequestPointer = RefPtr<Request>;

RequestPointer RequestRenderToTexture(LodComponent* lodComponent, int32 fromLodLayer,
                                      uint32 textureSize, const FilePath& texturePath);
};
} // namespace DAVA
