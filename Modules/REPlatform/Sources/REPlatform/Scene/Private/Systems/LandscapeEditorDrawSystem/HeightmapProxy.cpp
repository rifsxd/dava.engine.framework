#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/HeightmapProxy.h"

namespace DAVA
{
HeightmapProxy::HeightmapProxy(Heightmap* heightmap)
    : EditorHeightmap(heightmap)
{
}

void HeightmapProxy::UpdateRect(const Rect& rect)
{
    int32 size = Size();

    Rect bounds(0.f, 0.f, static_cast<float32>(size), static_cast<float32>(size));

    changedRect = rect;
    bounds.ClampToRect(changedRect);

    heightmapChanged = true;
}

void HeightmapProxy::ResetHeightmapChanged()
{
    heightmapChanged = false;
}

bool HeightmapProxy::IsHeightmapChanged() const
{
    return heightmapChanged;
}

const Rect& HeightmapProxy::GetChangedRect() const
{
    return changedRect;
}
} // namespace DAVA
