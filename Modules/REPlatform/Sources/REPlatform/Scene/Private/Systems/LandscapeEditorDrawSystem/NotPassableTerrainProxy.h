#pragma once

#include <Base/BaseTypes.h>
#include <Math/AABBox3.h>
#include <Math/Color.h>
#include <Math/Math2D.h>
#include <Math/Vector.h>
#include <Render/RHI/rhi_Public.h>

namespace DAVA
{
class Texture;
class Heightmap;
class NotPassableTerrainProxy
{
public:
    NotPassableTerrainProxy(int32 heightmapSize);
    virtual ~NotPassableTerrainProxy();

    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    Texture* GetTexture();
    void UpdateTexture(Heightmap* heightmap,
                       const AABBox3& landscapeBoundingBox,
                       const Rect2i& forRect);

private:
    static const int32 NOT_PASSABLE_ANGLE = 23;

    struct TerrainColor
    {
        Color color;
        Vector2 angleRange;

        TerrainColor(const Vector2& angle, const Color& color)
        {
            this->color = color;
            this->angleRange = angle;
        }
    };

    bool enabled;
    Texture* notPassableTexture;
    float32 notPassableAngleTan;
    Vector<TerrainColor> angleColor;

    Vector<rhi::HVertexBuffer> gridBuffers;

    void LoadColorsArray();
    bool PickColor(float32 tan, Color& color) const;
};
} // namespace DAVA
