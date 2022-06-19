#pragma once

#include <Base/BaseObject.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <FileSystem/FilePath.h>
#include <Math/AABBox3.h>
#include <Math/Color.h>
#include <Math/Vector.h>

namespace DAVA
{
class Image;
class Heightmap;
class Texture;
class Landscape;
class Entity;
class NMaterial;

class LandscapeProxy : public BaseObject
{
public:
    enum eTilemaskTextures
    {
        TILEMASK_TEXTURE_SOURCE = 0,
        TILEMASK_TEXTURE_DESTINATION,

        TILEMASK_TEXTURE_COUNT
    };

    enum eLandscapeMode
    {
        MODE_CUSTOM_LANDSCAPE = 0,
        MODE_ORIGINAL_LANDSCAPE,

        MODES_COUNT
    };

    static const FastName LANDSCAPE_TEXTURE_TOOL;
    static const FastName LANDSCAPE_TEXTURE_CURSOR; //should use clamp wrap mode
    static const FastName LANSDCAPE_FLAG_CURSOR;
    static const FastName LANSDCAPE_FLAG_TOOL;
    static const FastName LANSDCAPE_FLAG_TOOL_MIX;
    static const FastName LANDSCAPE_PARAM_CURSOR_COORD_SIZE; //x,y - cursor position [0...1] (in landscape space); z,w - cursor size [0...1] (fraction of landscape)

protected:
    virtual ~LandscapeProxy();

public:
    LandscapeProxy(Landscape* landscape, Entity* node);

    void SetMode(LandscapeProxy::eLandscapeMode mode);

    const AABBox3& GetLandscapeBoundingBox();
    Texture* GetLandscapeTexture(const FastName& level);
    Color GetLandscapeTileColor(const FastName& level);
    void SetLandscapeTileColor(const FastName& level, const Color& color);

    void SetToolTexture(Texture* texture, bool mixColors);

    void SetHeightmap(Heightmap* heightmap);

    void CursorEnable();
    void CursorDisable();
    void SetCursorTexture(Texture* texture);
    void SetCursorSize(float32 size);
    void SetCursorPosition(const Vector2& position);

    Vector3 PlacePoint(const Vector3& point);

    bool IsTilemaskChanged();
    void ResetTilemaskChanged();
    void IncreaseTilemaskChanges();
    void DecreaseTilemaskChanges();

    bool InitTilemaskImageCopy(const FilePath& sourceTilemaskPath);
    Image* GetTilemaskImageCopy();

    void InitTilemaskDrawTextures();
    Texture* GetTilemaskDrawTexture(int32 number);
    void SwapTilemaskDrawTextures();

private:
    void RestoreResources();

protected:
    enum eToolTextureType
    {
        TEXTURE_TYPE_NOT_PASSABLE = 0,
        TEXTURE_TYPE_CUSTOM_COLORS,
        TEXTURE_TYPE_VISIBILITY_CHECK_TOOL,
        TEXTURE_TYPE_RULER_TOOL,

        TEXTURE_TYPES_COUNT
    };

    Image* tilemaskImageCopy = nullptr;
    Array<Texture*, TILEMASK_TEXTURE_COUNT> tilemaskDrawTextures;

    int32 tilemaskWasChanged = 0;

    Landscape* baseLandscape = nullptr;
    NMaterial* landscapeEditorMaterial = nullptr;
    Vector4 cursorCoordSize;

    eLandscapeMode mode = MODE_ORIGINAL_LANDSCAPE;

    Texture* cursorTexture = nullptr;
};
} // namespace DAVA