#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/NotPassableTerrainProxy.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/RulerToolProxy.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Utils/RETextureDescriptorUtils.h"

#include "REPlatform/Commands/InspDynamicModifyCommand.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/SetFieldValueCommand.h"
#include "REPlatform/Global/Constants.h"
#include "REPlatform/Global/StringConstants.h"

#include <Base/Any.h>
#include <Debug/DVAssert.h>
#include <Scene3D/Systems/RenderUpdateSystem.h>

namespace DAVA
{
LandscapeEditorDrawSystem::LandscapeEditorDrawSystem(Scene* scene)
    : SceneSystem(scene)
{
}

LandscapeEditorDrawSystem::~LandscapeEditorDrawSystem()
{
    SafeRelease(baseLandscape);
    SafeRelease(landscapeProxy);
    SafeRelease(heightmapProxy);
    SafeRelease(customColorsProxy);
    SafeRelease(rulerToolProxy);
    SafeDelete(notPassableTerrainProxy);
}

LandscapeProxy* LandscapeEditorDrawSystem::GetLandscapeProxy()
{
    return landscapeProxy;
}

HeightmapProxy* LandscapeEditorDrawSystem::GetHeightmapProxy()
{
    return heightmapProxy;
}

CustomColorsProxy* LandscapeEditorDrawSystem::GetCustomColorsProxy()
{
    return customColorsProxy;
}

RulerToolProxy* LandscapeEditorDrawSystem::GetRulerToolProxy()
{
    return rulerToolProxy;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::EnableCustomDraw()
{
    if (customDrawRequestCount != 0)
    {
        ++customDrawRequestCount;
        return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
    }

    eErrorType initError = Init();
    if (initError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return initError;
    }

    landscapeProxy->SetMode(LandscapeProxy::MODE_CUSTOM_LANDSCAPE);
    landscapeProxy->SetHeightmap(heightmapProxy);

    ++customDrawRequestCount;

    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

void LandscapeEditorDrawSystem::DisableCustomDraw()
{
    if (customDrawRequestCount == 0)
    {
        return;
    }

    --customDrawRequestCount;

    if (customDrawRequestCount == 0)
    {
        landscapeProxy->SetMode(LandscapeProxy::MODE_ORIGINAL_LANDSCAPE);
        UpdateBaseLandscapeHeightmap();
    }
}

bool LandscapeEditorDrawSystem::IsNotPassableTerrainEnabled()
{
    if (!notPassableTerrainProxy)
    {
        return false;
    }

    return notPassableTerrainProxy->IsEnabled();
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::IsNotPassableTerrainCanBeEnabled()
{
    return VerifyLandscape();
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::EnableNotPassableTerrain()
{
    eErrorType canBeEnabledError = IsNotPassableTerrainCanBeEnabled();
    if (canBeEnabledError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return canBeEnabledError;
    }

    if (!notPassableTerrainProxy)
    {
        notPassableTerrainProxy = new NotPassableTerrainProxy(baseLandscape->GetHeightmap()->Size());
    }

    if (notPassableTerrainProxy->IsEnabled())
    {
        return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
    }

    eErrorType enableCustomDrawError = EnableCustomDraw();
    if (enableCustomDrawError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return enableCustomDrawError;
    }

    Rect2i updateRect = Rect2i(0, 0, GetHeightmapProxy()->Size(), GetHeightmapProxy()->Size());
    notPassableTerrainProxy->SetEnabled(true);
    notPassableTerrainProxy->UpdateTexture(heightmapProxy, landscapeProxy->GetLandscapeBoundingBox(), updateRect);

    landscapeProxy->SetToolTexture(notPassableTerrainProxy->GetTexture(), false);

    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool LandscapeEditorDrawSystem::DisableNotPassableTerrain()
{
    if (!notPassableTerrainProxy || !notPassableTerrainProxy->IsEnabled())
    {
        return false;
    }

    notPassableTerrainProxy->SetEnabled(false);
    landscapeProxy->SetToolTexture(nullptr, false);

    DisableCustomDraw();
    return true;
}

void LandscapeEditorDrawSystem::EnableCursor()
{
    landscapeProxy->CursorEnable();
}

void LandscapeEditorDrawSystem::DisableCursor()
{
    landscapeProxy->CursorDisable();
}

void LandscapeEditorDrawSystem::SetCursorTexture(Texture* cursorTexture)
{
    landscapeProxy->SetCursorTexture(cursorTexture);
}

void LandscapeEditorDrawSystem::SetCursorSize(float32 cursorSize)
{
    if (landscapeProxy)
    {
        landscapeProxy->SetCursorSize(cursorSize);
    }
}

void LandscapeEditorDrawSystem::SetCursorPosition(const Vector2& cursorPos)
{
    if (landscapeProxy)
    {
        landscapeProxy->SetCursorPosition(cursorPos);
    }
}

void LandscapeEditorDrawSystem::Process(float32 timeElapsed)
{
    if (heightmapProxy && heightmapProxy->IsHeightmapChanged())
    {
        const Rect& changedRect = heightmapProxy->GetChangedRect();
        Rect2i updateRect(static_cast<int32>(changedRect.x), static_cast<int32>(changedRect.y),
                          static_cast<int32>(changedRect.dx), static_cast<int32>(changedRect.dy));

        if (customDrawRequestCount == 0)
        {
            UpdateBaseLandscapeHeightmap();
        }
        else
        {
            if (baseLandscape->IsUpdatable())
            {
                baseLandscape->UpdatePart(updateRect);
            }
            else
            {
                UpdateBaseLandscapeHeightmap();
            }
        }

        if (notPassableTerrainProxy && notPassableTerrainProxy->IsEnabled())
        {
            notPassableTerrainProxy->UpdateTexture(heightmapProxy, landscapeProxy->GetLandscapeBoundingBox(), updateRect);
        }

        heightmapProxy->ResetHeightmapChanged();
    }

    if (customColorsProxy && customColorsProxy->IsTargetChanged())
    {
        customColorsProxy->ResetTargetChanged();
    }
}

void LandscapeEditorDrawSystem::UpdateBaseLandscapeHeightmap()
{
    ScopedPtr<Heightmap> h(new Heightmap());
    heightmapProxy->Clone(h);

    baseLandscape->SetHeightmap(h);

    GetScene()->foliageSystem->SyncFoliageWithLandscape();
}

float32 LandscapeEditorDrawSystem::GetTextureSize(const FastName& level)
{
    float32 size = 0.f;
    Texture* texture = baseLandscape->GetMaterial()->GetEffectiveTexture(level);
    if (texture)
    {
        size = static_cast<float32>(texture->GetWidth());
    }
    return size;
}

Vector3 LandscapeEditorDrawSystem::GetLandscapeSize()
{
    AABBox3 transformedBox;
    baseLandscape->GetBoundingBox().GetTransformedBox(*baseLandscape->GetWorldMatrixPtr(), transformedBox);

    Vector3 landSize = transformedBox.max - transformedBox.min;
    return landSize;
}

float32 LandscapeEditorDrawSystem::GetLandscapeMaxHeight()
{
    Vector3 landSize = GetLandscapeSize();
    return landSize.z;
}

Rect LandscapeEditorDrawSystem::GetTextureRect(const FastName& level)
{
    float32 textureSize = GetTextureSize(level);
    return Rect(Vector2(0.f, 0.f), Vector2(textureSize, textureSize));
}

Rect LandscapeEditorDrawSystem::GetHeightmapRect()
{
    float32 heightmapSize = static_cast<float32>(GetHeightmapProxy()->Size());
    return Rect(Vector2(0.f, 0.f), Vector2(heightmapSize, heightmapSize));
}

Rect LandscapeEditorDrawSystem::GetLandscapeRect()
{
    AABBox3 boundingBox = GetLandscapeProxy()->GetLandscapeBoundingBox();
    Vector2 landPos(boundingBox.min.x, boundingBox.min.y);
    Vector2 landSize((boundingBox.max - boundingBox.min).x,
                     (boundingBox.max - boundingBox.min).y);

    return Rect(landPos, landSize);
}

float32 LandscapeEditorDrawSystem::GetHeightAtHeightmapPoint(const Vector2& point)
{
    Heightmap* heightmap = GetHeightmapProxy();

    int32 hmSize = heightmap->Size();
    int32 x = static_cast<int32>(point.x);
    int32 y = static_cast<int32>(point.y);

    DVASSERT((x >= 0) && (x < hmSize) && (y >= 0) && (y < hmSize),
             "Point must be in heightmap coordinates");

    int nextX = Min(x + 1, hmSize - 1);
    int nextY = Min(y + 1, hmSize - 1);
    int i00 = x + y * hmSize;
    int i01 = nextX + y * hmSize;
    int i10 = x + nextY * hmSize;
    int i11 = nextX + nextY * hmSize;

    const auto hmData = heightmap->Data();
    float h00 = static_cast<float>(hmData[i00]);
    float h01 = static_cast<float>(hmData[i01]);
    float h10 = static_cast<float>(hmData[i10]);
    float h11 = static_cast<float>(hmData[i11]);

    float dx = point.x - static_cast<float>(x);
    float dy = point.y - static_cast<float>(y);
    float h0 = h00 * (1.0f - dx) + h01 * dx;
    float h1 = h10 * (1.0f - dx) + h11 * dx;
    float h = h0 * (1.0f - dy) + h1 * dy;

    return h * GetLandscapeMaxHeight() / static_cast<float32>(Heightmap::MAX_VALUE);
}

float32 LandscapeEditorDrawSystem::GetHeightAtTexturePoint(const FastName& level, const Vector2& point)
{
    auto textureRect = GetTextureRect(level);
    if (textureRect.PointInside(point))
    {
        return GetHeightAtHeightmapPoint(TexturePointToHeightmapPoint(level, point));
    }

    return 0.0f;
}

Vector2 LandscapeEditorDrawSystem::HeightmapPointToTexturePoint(const FastName& level, const Vector2& point)
{
    return TranslatePoint(point, GetHeightmapRect(), GetTextureRect(level));
}

Vector2 LandscapeEditorDrawSystem::TexturePointToHeightmapPoint(const FastName& level, const Vector2& point)
{
    return TranslatePoint(point, GetTextureRect(level), GetHeightmapRect());
}

Vector2 LandscapeEditorDrawSystem::TexturePointToLandscapePoint(const FastName& level, const Vector2& point)
{
    return TranslatePoint(point, GetTextureRect(level), GetLandscapeRect());
}

Vector2 LandscapeEditorDrawSystem::LandscapePointToTexturePoint(const FastName& level, const Vector2& point)
{
    return TranslatePoint(point, GetLandscapeRect(), GetTextureRect(level));
}

Vector2 LandscapeEditorDrawSystem::TranslatePoint(const Vector2& point, const Rect& fromRect, const Rect& toRect)
{
    DVASSERT(fromRect.dx != 0.f && fromRect.dy != 0.f);

    Vector2 origRectSize = fromRect.GetSize();
    Vector2 destRectSize = toRect.GetSize();

    Vector2 scale(destRectSize.x / origRectSize.x,
                  destRectSize.y / origRectSize.y);

    Vector2 relPos = point - fromRect.GetPosition();
    Vector2 newRelPos(relPos.x * scale.x,
                      toRect.dy - 1.0f - relPos.y * scale.y);

    Vector2 newPos = newRelPos + toRect.GetPosition();

    return newPos;
}

KeyedArchive* LandscapeEditorDrawSystem::GetLandscapeCustomProperties()
{
    return GetOrCreateCustomProperties(landscapeNode)->GetArchive();
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::EnableTilemaskEditing()
{
    eErrorType initError = Init();
    if (initError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return initError;
    }

    landscapeProxy->SetMode(LandscapeProxy::MODE_ORIGINAL_LANDSCAPE);
    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

void LandscapeEditorDrawSystem::DisableTilemaskEditing()
{
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::Init()
{
    if (heightmapProxy == nullptr)
    {
        Heightmap* heightmap = baseLandscape->GetHeightmap();
        if ((heightmap == nullptr) || (heightmap->Size() == 0))
        {
            return LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT;
        }
        ScopedPtr<Heightmap> clonedHeightmap(heightmap->Clone(nullptr));
        heightmapProxy = new HeightmapProxy(clonedHeightmap);
    }

    if (customColorsProxy == nullptr)
    {
        customColorsProxy = new CustomColorsProxy(Landscape::CUSTOM_COLOR_TEXTURE_SIZE);
    }

    if (rulerToolProxy == nullptr)
    {
        rulerToolProxy = new RulerToolProxy(static_cast<int32>(GetTextureSize(Landscape::TEXTURE_COLOR)));
    }

    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::InitLandscape(Entity* landscapeEntity, Landscape* landscape)
{
    DeinitLandscape();

    if (!landscapeEntity || !landscape)
    {
        return LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT;
    }

    landscapeNode = landscapeEntity;
    baseLandscape = SafeRetain(landscape);

    UpdateTilemaskPathname();

    DVASSERT(landscapeProxy == nullptr);
    landscapeProxy = new LandscapeProxy(baseLandscape, landscapeNode);

    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

void LandscapeEditorDrawSystem::DeinitLandscape()
{
    landscapeNode = NULL;
    SafeRelease(landscapeProxy);
    SafeRelease(baseLandscape);
}

void LandscapeEditorDrawSystem::ClampToTexture(const FastName& level, Rect& rect)
{
    GetTextureRect(level).ClampToRect(rect);
}

void LandscapeEditorDrawSystem::ClampToHeightmap(Rect& rect)
{
    GetHeightmapRect().ClampToRect(rect);
}

void LandscapeEditorDrawSystem::AddEntity(Entity* entity)
{
    if (IsSystemEnabled() == false)
    {
        return;
    }

    Landscape* landscape = GetLandscape(entity);
    if (landscape != NULL)
    {
        entity->SetLocked(true);

        InitLandscape(entity, landscape);
    }
}

void LandscapeEditorDrawSystem::RemoveEntity(Entity* entity)
{
    if (entity == landscapeNode && IsSystemEnabled())
    {
        SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());

        bool needRemoveBaseLandscape = sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL
                                                                   & ~SceneEditor2::LANDSCAPE_TOOL_TILEMAP_EDITOR);

        sceneEditor->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);

        if (needRemoveBaseLandscape)
        {
            sceneEditor->renderUpdateSystem->RemoveEntity(entity);
        }

        DeinitLandscape();

        Entity* landEntity = FindLandscapeEntity(sceneEditor);
        if (landEntity != nullptr && landEntity != entity)
        {
            InitLandscape(landEntity, GetLandscape(landEntity));
        }
    }
}

void LandscapeEditorDrawSystem::PrepareForRemove()
{
    DeinitLandscape();
}

bool LandscapeEditorDrawSystem::SaveTileMaskTexture()
{
    if (baseLandscape == nullptr || !GetLandscapeProxy()->IsTilemaskChanged())
    {
        return false;
    }

    Texture* texture = GetTileMaskTexture();
    if (texture != nullptr)
    {
        Image* image = texture->CreateImageFromMemory();

        if (image)
        {
            ImageSystem::Save(sourceTilemaskPath, image);
            SafeRelease(image);
        }

        GetLandscapeProxy()->ResetTilemaskChanged();

        return true;
    }

    return false;
}

void LandscapeEditorDrawSystem::ResetTileMaskTexture()
{
    if (baseLandscape == nullptr)
    {
        return;
    }

    ScopedPtr<Texture> texture(Texture::CreateFromFile(sourceTilemaskPath));
    texture->Reload();
    SetTileMaskTexture(texture);
}

void LandscapeEditorDrawSystem::SetTileMaskTexture(Texture* texture)
{
    if (baseLandscape == nullptr)
    {
        return;
    }

    NMaterial* landscapeMaterial = baseLandscape->GetMaterial();
    while (landscapeMaterial != nullptr)
    {
        if (landscapeMaterial->HasLocalTexture(Landscape::TEXTURE_TILEMASK))
            break;

        landscapeMaterial = landscapeMaterial->GetParent();
    }

    if (landscapeMaterial != nullptr)
    {
        landscapeMaterial->SetTexture(Landscape::TEXTURE_TILEMASK, texture);
    }
}

Texture* LandscapeEditorDrawSystem::GetTileMaskTexture()
{
    if (baseLandscape != nullptr)
    {
        NMaterial* landscapeMaterial = baseLandscape->GetMaterial();
        if (landscapeMaterial != nullptr)
        {
            return landscapeMaterial->GetEffectiveTexture(Landscape::TEXTURE_TILEMASK);
        }
    }

    return nullptr;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::VerifyLandscape() const
{
    using namespace DAVA;

    //landscape initialization should be handled by AddEntity/RemoveEntity methods
    if (!landscapeNode || !baseLandscape || !landscapeProxy)
    {
        return LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT;
    }

    Texture* tileMask = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILEMASK);
    if (tileMask == nullptr || tileMask->IsPinkPlaceholder())
    {
        return LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSENT;
    }

    Texture* texTile = baseLandscape->GetMaterial()->GetEffectiveTexture(Landscape::TEXTURE_TILE);
    if ((texTile == nullptr || texTile->IsPinkPlaceholder()))
    {
        return LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE_ABSENT;
    }

    Texture* texColor = baseLandscape->GetMaterial()->GetEffectiveTexture(Landscape::TEXTURE_COLOR);
    if ((texColor == nullptr || texColor->IsPinkPlaceholder()))
    {
        return LANDSCAPE_EDITOR_SYSTEM_COLOR_TEXTURE_ABSENT;
    }

    Heightmap* heightmap = baseLandscape->GetHeightmap();
    if ((heightmap == nullptr) || (heightmap->Size() == 0))
    {
        return LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT;
    }

    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

Landscape* LandscapeEditorDrawSystem::GetBaseLandscape() const
{
    return baseLandscape;
}

String LandscapeEditorDrawSystem::GetDescriptionByError(eErrorType error)
{
    String ret;
    switch (error)
    {
    case LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSETN;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_FULLTILED_TEXTURE_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_FULLTILED_TEXTURE_ABSETN;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE_ABSENT;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_COLOR_TEXTURE_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_COLOR_TEXTURE_ABSENT;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT;
        break;

    default:
        DVASSERT(false && "Unknown error");
        break;
    }
    return ret;
}

void LandscapeEditorDrawSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    static const FastName heightmapPath("heightmapPath");

    commandNotification.ForEach<InspDynamicModifyCommand>([this](const InspDynamicModifyCommand* cmd) {
        if (Landscape::TEXTURE_TILEMASK == cmd->key)
        {
            UpdateTilemaskPathname();
        }
    });

    commandNotification.ForEach<SetFieldValueCommand>([this](const SetFieldValueCommand* cmd) {
        if (heightmapPath == cmd->GetField().key.Cast<FastName>(FastName("")) && baseLandscape != nullptr)
        {
            Heightmap* heightmap = baseLandscape->GetHeightmap();
            if ((heightmap != nullptr) && (heightmap->Size() > 0))
            {
                ScopedPtr<Heightmap> clonedHeightmap(heightmap->Clone(nullptr));
                SafeRelease(heightmapProxy);
                heightmapProxy = new HeightmapProxy(clonedHeightmap);

                float32 size = static_cast<float32>(heightmapProxy->Size());
                heightmapProxy->UpdateRect(Rect(0.f, 0.f, size, size));
            }
        }
    });
}

bool LandscapeEditorDrawSystem::UpdateTilemaskPathname()
{
    if (nullptr != baseLandscape)
    {
        auto texture = baseLandscape->GetMaterial()->GetEffectiveTexture(Landscape::TEXTURE_TILEMASK);
        if (nullptr != texture)
        {
            FilePath path = texture->GetDescriptor()->GetSourceTexturePathname();
            if (path.GetType() == FilePath::PATH_IN_FILESYSTEM)
            {
                sourceTilemaskPath = path;
            }
            return true;
        }
    }

    return false;
}

bool LandscapeEditorDrawSystem::InitTilemaskImageCopy()
{
    DVASSERT(landscapeProxy != nullptr);
    return landscapeProxy->InitTilemaskImageCopy(sourceTilemaskPath);
}
} // namespace DAVA
