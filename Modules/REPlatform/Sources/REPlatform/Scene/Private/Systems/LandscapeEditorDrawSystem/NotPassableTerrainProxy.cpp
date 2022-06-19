#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/NotPassableTerrainProxy.h"

#include <FileSystem/YamlNode.h>
#include <FileSystem/YamlParser.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/DynamicBindings.h>
#include <Render/Highlevel/Heightmap.h>
#include <Render/Material/NMaterial.h>
#include <Render/RenderBase.h>
#include <Render/Renderer.h>
#include <Render/RenderHelper.h>
#include <Render/Texture.h>

namespace DAVA
{
static const int32 GRID_QUAD_SIZE = 65;

NotPassableTerrainProxy::NotPassableTerrainProxy(int32 heightmapSize)
    : enabled(false)
{
    LoadColorsArray();

    notPassableAngleTan = static_cast<float32>(tan(DegToRad(static_cast<float32>(NOT_PASSABLE_ANGLE))));
    notPassableTexture = Texture::CreateFBO(2048, 2048, FORMAT_RGBA8888);
    notPassableTexture->SetMinMagFilter(rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureMipFilter::TEXMIPFILTER_NONE);

    rhi::Viewport viewport;
    viewport.width = viewport.height = 2048;
    RenderHelper::CreateClearPass(notPassableTexture->handle, rhi::HTexture(), PRIORITY_SERVICE_2D + 1, Color::Clear, viewport);

    int32 quadLineSize = static_cast<int32>(ceilf(static_cast<float32>(heightmapSize) / GRID_QUAD_SIZE));
    int32 buffersCount = quadLineSize * quadLineSize;
    gridBuffers.resize(buffersCount);

    rhi::VertexBuffer::Descriptor desc;
    desc.size = GRID_QUAD_SIZE * GRID_QUAD_SIZE * 4 * sizeof(float32) * 4;
    desc.needRestore = false;

    for (int32 i = 0; i < buffersCount; ++i)
        gridBuffers[i] = rhi::CreateVertexBuffer(desc);
}

NotPassableTerrainProxy::~NotPassableTerrainProxy()
{
    SafeRelease(notPassableTexture);
    for (const rhi::HVertexBuffer& quadBuffer : gridBuffers)
        rhi::DeleteVertexBuffer(quadBuffer);
}

void NotPassableTerrainProxy::LoadColorsArray()
{
    RefPtr<YamlParser> parser = YamlParser::Create("~res:/ResourceEditor/Configs/LandscapeAngle.yaml");

    if (parser.Valid())
    {
        YamlNode* rootNode = parser->GetRootNode();
        int32 anglesCount = rootNode->GetCount();

        angleColor.reserve(anglesCount);
        for (int32 i = 0; i < anglesCount; ++i)
        {
            const YamlNode* node = rootNode->Get(i);
            if (!node || node->GetCount() != 3)
            {
                continue;
            }

            float32 angle1 = node->Get(0)->AsFloat();
            float32 angle2 = node->Get(1)->AsFloat();

            angle1 = Min(angle1, 89.f);
            angle2 = Min(angle2, 89.f);

            float32 tangentMin = tan(DegToRad(angle1));
            float32 tangentMax = tan(DegToRad(angle2));

            const YamlNode* colorNode = node->Get(2);
            if (!colorNode || colorNode->GetCount() != 4)
            {
                continue;
            }

            Color color(colorNode->Get(0)->AsFloat() / 255.f,
                        colorNode->Get(1)->AsFloat() / 255.f,
                        colorNode->Get(2)->AsFloat() / 255.f,
                        colorNode->Get(3)->AsFloat() / 255.f);

            angleColor.push_back(TerrainColor(Vector2(tangentMin, tangentMax), color));
        }
    }
}

bool NotPassableTerrainProxy::PickColor(float32 tan, Color& color) const
{
    for (uint32 i = 0; i < angleColor.size(); ++i)
    {
        if (tan >= angleColor[i].angleRange.x && tan < angleColor[i].angleRange.y)
        {
            color = angleColor[i].color;
            return true;
        }
    }
    return false;
}

void NotPassableTerrainProxy::SetEnabled(bool enabled_)
{
    enabled = enabled_;
}

bool NotPassableTerrainProxy::IsEnabled() const
{
    return enabled;
}

Texture* NotPassableTerrainProxy::GetTexture()
{
    return notPassableTexture;
}

void NotPassableTerrainProxy::UpdateTexture(Heightmap* heightmap, const AABBox3& landscapeBoundingBox, const Rect2i& forRect)
{
    const Vector3 landSize = landscapeBoundingBox.max - landscapeBoundingBox.min;

    const float32 angleCellDistance = landSize.x / static_cast<float32>(heightmap->Size());
    const float32 angleHeightDelta = landSize.z / static_cast<float32>(Heightmap::MAX_VALUE - 1);
    const float32 tanCoef = angleHeightDelta / angleCellDistance;

    const int32 heightmapSize = heightmap->Size();

    const float32 targetWidth = static_cast<float32>(notPassableTexture->GetWidth());
    const float32 dx = targetWidth / static_cast<float32>(heightmapSize);

    ///////////////////////////////

    Size2f textureSize(float32(notPassableTexture->GetWidth()), float32(notPassableTexture->GetHeight()));

    Matrix4 projMatrix;
    if (!rhi::DeviceCaps().isUpperLeftRTOrigin)
    {
        projMatrix.BuildOrtho(0.0f, textureSize.dx, 0.0f, textureSize.dy, -1.0f, 1.0f, rhi::DeviceCaps().isZeroBaseClipRange);
    }
    else
    {
        projMatrix.BuildOrtho(0.0f, textureSize.dx, textureSize.dy, 0.0f, -1.0f, 1.0f, rhi::DeviceCaps().isZeroBaseClipRange);
    }

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, reinterpret_cast<pointer_size>(&Matrix4::IDENTITY));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEW, &Matrix4::IDENTITY, reinterpret_cast<pointer_size>(&Matrix4::IDENTITY));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PROJ, &projMatrix, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

    rhi::Packet gridQuadPacket;
    RenderSystem2D::DEFAULT_2D_COLOR_MATERIAL->BindParams(gridQuadPacket);

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_COLOR, 0, rhi::VDT_UINT8N, 4);

    gridQuadPacket.vertexStreamCount = 1;
    gridQuadPacket.primitiveType = rhi::PRIMITIVE_LINELIST;
    gridQuadPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(layout);

    ///////////////////////////////

    rhi::RenderPassConfig passConfig;
    passConfig.colorBuffer[0].texture = notPassableTexture->handle;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_NONE;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.priority = PRIORITY_SERVICE_2D;
    passConfig.viewport.width = notPassableTexture->GetWidth();
    passConfig.viewport.height = notPassableTexture->GetHeight();

    rhi::HPacketList packetListHandle;
    rhi::HRenderPass passTargetHandle = rhi::AllocateRenderPass(passConfig, 1, &packetListHandle);

    rhi::BeginRenderPass(passTargetHandle);
    rhi::BeginPacketList(packetListHandle);

    ///////////////////////////////

    int32 bufferIndex = 0;
    float32* tempBuffer = new float32[GRID_QUAD_SIZE * GRID_QUAD_SIZE * 4 * 4];
    for (int32 yRect = 0; yRect < heightmapSize; yRect += GRID_QUAD_SIZE)
    {
        for (int32 xRect = 0; xRect < heightmapSize; xRect += GRID_QUAD_SIZE)
        {
            if (Rect2i(xRect, yRect, GRID_QUAD_SIZE, GRID_QUAD_SIZE).RectIntersects(forRect))
            {
                float32* bufferPtr = tempBuffer;
                int32 primitiveCount = 0;

                for (int32 y = yRect; (y < yRect + GRID_QUAD_SIZE) && y < heightmapSize; ++y)
                {
                    const float32 ydx = (heightmapSize - y) * dx;

                    for (int32 x = xRect; (x < xRect + GRID_QUAD_SIZE) && x < heightmapSize; ++x)
                    {
                        const uint16 currentPoint = heightmap->GetHeightClamp(x, y);
                        const uint16 rightPoint = heightmap->GetHeightClamp(x + 1, y);
                        const uint16 bottomPoint = heightmap->GetHeightClamp(x, y + 1);

                        const uint16 deltaRight = static_cast<uint16>(abs(static_cast<int32>(currentPoint) - static_cast<int32>(rightPoint)));
                        const uint16 deltaBottom = static_cast<uint16>(abs(static_cast<int32>(currentPoint) - static_cast<int32>(bottomPoint)));

                        const float32 tanRight = static_cast<float32>(deltaRight) * tanCoef;
                        const float32 tanBottom = static_cast<float32>(deltaBottom) * tanCoef;

                        const float32 xdx = x * dx;

                        Color color(0.f, 0.f, 0.f, 0.f);

                        PickColor(tanRight, color);

                        {
                            *(reinterpret_cast<Vector3*>(bufferPtr)) = Vector3(xdx, ydx, 0.f);
                            bufferPtr += 3;
                            *(reinterpret_cast<uint32*>(bufferPtr)) = rhi::NativeColorRGBA(color.r, color.g, color.b, color.a);
                            ++bufferPtr;
                            *(reinterpret_cast<Vector3*>(bufferPtr)) = Vector3((xdx + dx), ydx, 0.f);
                            bufferPtr += 3;
                            *(reinterpret_cast<uint32*>(bufferPtr)) = rhi::NativeColorRGBA(color.r, color.g, color.b, color.a);
                            ++bufferPtr;
                        }

                        PickColor(tanBottom, color);

                        {
                            *(reinterpret_cast<Vector3*>(bufferPtr)) = Vector3(xdx, ydx, 0.f);
                            bufferPtr += 3;
                            *(reinterpret_cast<uint32*>(bufferPtr)) = rhi::NativeColorRGBA(color.r, color.g, color.b, color.a);
                            ++bufferPtr;
                            *(reinterpret_cast<Vector3*>(bufferPtr)) = Vector3(xdx, (ydx - dx), 0.f);
                            bufferPtr += 3;
                            *(reinterpret_cast<uint32*>(bufferPtr)) = rhi::NativeColorRGBA(color.r, color.g, color.b, color.a);
                            ++bufferPtr;
                        }

                        primitiveCount += 2;
                    }
                }

                rhi::UpdateVertexBuffer(gridBuffers[bufferIndex], tempBuffer, 0, primitiveCount * 4 * 2 * sizeof(float32));

                gridQuadPacket.vertexStream[0] = gridBuffers[bufferIndex];
                gridQuadPacket.primitiveCount = primitiveCount;
                gridQuadPacket.vertexCount = gridQuadPacket.primitiveCount * 2;

                rhi::AddPacket(packetListHandle, gridQuadPacket);
            }

            ++bufferIndex;
        }
    }

    SafeDeleteArray(tempBuffer);

    ///////////////////////////////

    rhi::EndPacketList(packetListHandle);
    rhi::EndRenderPass(passTargetHandle);
}
} // namespace DAVA
