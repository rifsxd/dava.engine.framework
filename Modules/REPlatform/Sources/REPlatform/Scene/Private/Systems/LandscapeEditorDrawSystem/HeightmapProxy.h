#pragma once

#include "REPlatform/Deprecated/EditorHeightmap.h"

namespace DAVA
{
class HeightmapProxy : public EditorHeightmap
{
public:
    HeightmapProxy(Heightmap* heightmap);

    void UpdateRect(const Rect& rect);
    void ResetHeightmapChanged();

    bool IsHeightmapChanged() const;
    const Rect& GetChangedRect() const;

private:
    Rect changedRect;
    bool heightmapChanged = false;
};
} // namespace DAVA