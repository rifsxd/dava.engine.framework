#pragma once

#include "Entity/SceneSystem.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/UniqueStateSet.h"

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

namespace DAVA
{
class LandscapeProxy;
class HeightmapProxy;
class NotPassableTerrainProxy;
class CustomColorsProxy;
class RulerToolProxy;
class RECommandNotificationObject;

class LandscapeEditorDrawSystem : public SceneSystem, public EditorSceneSystem
{
public:
    enum eErrorType
    {
        LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS = 0,
        LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_FULLTILED_TEXTURE_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_COLOR_TEXTURE_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT,
        LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT,
    };

    LandscapeEditorDrawSystem(Scene* scene);
    virtual ~LandscapeEditorDrawSystem();

    LandscapeProxy* GetLandscapeProxy();
    HeightmapProxy* GetHeightmapProxy();
    CustomColorsProxy* GetCustomColorsProxy();
    RulerToolProxy* GetRulerToolProxy();

    eErrorType EnableCustomDraw();
    void DisableCustomDraw();

    eErrorType EnableTilemaskEditing();
    void DisableTilemaskEditing();

    bool IsNotPassableTerrainEnabled();
    eErrorType EnableNotPassableTerrain();
    bool DisableNotPassableTerrain();

    void EnableCursor();
    void DisableCursor();
    void SetCursorTexture(Texture* cursorTexture);
    void SetCursorSize(float32 cursorSize);
    void SetCursorPosition(const Vector2& cursorPos);

    float32 GetTextureSize(const FastName& level);
    Vector3 GetLandscapeSize();
    float32 GetLandscapeMaxHeight();
    float32 GetHeightAtHeightmapPoint(const Vector2& point);
    float32 GetHeightAtTexturePoint(const FastName& level, const Vector2& point);
    KeyedArchive* GetLandscapeCustomProperties();

    Vector2 HeightmapPointToTexturePoint(const FastName& level, const Vector2& point);
    Vector2 TexturePointToHeightmapPoint(const FastName& level, const Vector2& point);
    Vector2 TexturePointToLandscapePoint(const FastName& level, const Vector2& point);
    Vector2 LandscapePointToTexturePoint(const FastName& level, const Vector2& point);
    Vector2 TranslatePoint(const Vector2& point, const Rect& fromRect, const Rect& toRect);

    void ClampToTexture(const FastName& level, Rect& rect);
    void ClampToHeightmap(Rect& rect);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    Rect GetTextureRect(const FastName& level);
    Rect GetHeightmapRect();
    Rect GetLandscapeRect();

    bool SaveTileMaskTexture();
    void ResetTileMaskTexture();
    Texture* GetTileMaskTexture();
    void SetTileMaskTexture(Texture* texture);

    eErrorType VerifyLandscape() const;

    Landscape* GetBaseLandscape() const;

    static String GetDescriptionByError(eErrorType error);

    bool UpdateTilemaskPathname();
    bool InitTilemaskImageCopy();

protected:
    void Process(float32 timeElapsed) override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    void UpdateBaseLandscapeHeightmap();
    eErrorType Init();

    eErrorType InitLandscape(Entity* landscapeEntity, Landscape* landscape);
    void DeinitLandscape();

    eErrorType IsNotPassableTerrainCanBeEnabled();

    Entity* landscapeNode = nullptr;
    Landscape* baseLandscape = nullptr;
    LandscapeProxy* landscapeProxy = nullptr;
    HeightmapProxy* heightmapProxy = nullptr;
    NotPassableTerrainProxy* notPassableTerrainProxy = nullptr;
    CustomColorsProxy* customColorsProxy = nullptr;
    RulerToolProxy* rulerToolProxy = nullptr;
    uint32 customDrawRequestCount = 0;
    FilePath sourceTilemaskPath;
};
} // namespace DAVA
