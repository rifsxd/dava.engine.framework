#pragma once

#include <TArc/DataProcessing/SettingsNode.h>
#include <TArc/Qt/QtByteArray.h>

#include <AssetCache/AssetCacheConstants.h>
#include <TextureCompression/TextureConverter.h>

#include <Base/BaseTypes.h>
#include <FileSystem/FilePath.h>
#include <Math/Color.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Components/ActionComponent.h>

namespace DAVA
{
class PropertiesItem;
enum class RenderingBackend
{
    DX11 = 0,
    DX9,
    OpenGL
};

class GeneralSettings : public SettingsNode
{
public:
    bool reloadParticlesOnProjectOpening = true;
    bool previewEnabled = false;
    TextureConverter::eConvertQuality compressionQuality = TextureConverter::ECQ_DEFAULT;
    bool showErrorDialog = true;
    uint32 recentScenesCount = 15;

    // Material Editor settings
    Color materialEditorSwitchColor0 = Color(0.0f, 1.0f, 0.0f, 1.0f);
    Color materialEditorSwitchColor1 = Color(1.0f, 0.0f, 0.0f, 1.0f);
    Color materialEditorLodColor0 = Color(0.9f, 0.9f, 0.9f, 1.0f);
    Color materialEditorLodColor1 = Color(0.7f, 0.7f, 0.7f, 1.0f);
    Color materialEditorLodColor2 = Color(0.5f, 0.5f, 0.5f, 1.0f);
    Color materialEditorLodColor3 = Color(0.3f, 0.3f, 0.3f, 1.0f);

    // Particle Editor settings
    float32 particleDebugAlphaTheshold = 0.05f;

    // Lod Editor settings
    Color lodEditorLodColor0 = Color(0.2f, 0.35f, 0.62f, 1.0f);
    Color lodEditorLodColor1 = Color(0.25f, 0.45f, 0.78f, 1.0f);
    Color lodEditorLodColor2 = Color(0.33f, 0.56f, 0.97f, 1.0f);
    Color lodEditorLodColor3 = Color(0.62f, 0.75f, 0.98f, 1.0f);
    Color inactiveColor = Color(0.59f, 0.59f, 0.59f, 1.0f);
    bool fitSliders = false;

    // Height mask tools settings
    Color heightMaskColor0 = Color(0.5f, 0.5f, 0.5f, 1.0f);
    Color heightMaskColor1 = Color(0.0f, 0.0f, 0.0f, 1.0f);

    // Asset cache settings
    bool useAssetCache = false;
    String assetCacheIP = "";
    uint16 assetCachePort = AssetCache::ASSET_SERVER_PORT;
    uint32 assetCacheTimeout = 10;

    // Texture browser settings
    bool autoConversion = true;

    // Renderer settings
    RenderingBackend renderBackend = RenderingBackend::OpenGL;

    // Mouse settings
    bool wheelMoveCamera = true;
    float32 wheelMoveIntensity = 180.0f;
    bool invertWheel = false;

    void Load(const PropertiesItem& settingsNode) override;

    DAVA_VIRTUAL_REFLECTION(GeneralSettings, SettingsNode);
};

class CommonInternalSettings : public SettingsNode
{
public:
    enum MaterialLightViewMode
    {
        LIGHTVIEW_NOTHING = 0x0,

        LIGHTVIEW_ALBEDO = 0x1,
        LIGHTVIEW_AMBIENT = 0x2,
        LIGHTVIEW_DIFFUSE = 0x4,
        LIGHTVIEW_SPECULAR = 0x8,

        LIGHTVIEW_ALL = (LIGHTVIEW_ALBEDO | LIGHTVIEW_AMBIENT | LIGHTVIEW_DIFFUSE | LIGHTVIEW_SPECULAR)
    };

    eGPUFamily textureViewGPU = GPU_ORIGIN;
    eGPUFamily spritesViewGPU = GPU_ORIGIN;
    FilePath cubemapLastFaceDir;
    FilePath cubemapLastProjDir;
    FilePath emitterSaveDir;
    FilePath emitterLoadDir;
    MaterialLightViewMode materialLightViewMode = LIGHTVIEW_ALL;
    bool materialShowLightmapCanvas = false;
    bool lodEditorSceneMode = false;
    bool lodEditorRecursive = false;
    ActionComponent::Action::eEvent runActionEventType = ActionComponent::Action::EVENT_SWITCH_CHANGED;
    String beastLightmapsDefaultDir = String("lightmaps");
    String imageSplitterPath = String("");
    String imageSplitterPathSpecular = String("");
    bool enableSound = true;
    bool gizmoEnabled = true;
    bool validateMatrices = true;
    bool validateSameNames = true;
    bool validateCollisionProperties = true;
    bool validateTextureRelevance = true;
    bool validateMaterialGroups = true;
    bool validateShowConsole = true;
    QByteArray logWidgetState;

    DAVA_VIRTUAL_REFLECTION(CommonInternalSettings, SettingsNode);
};
} // namespace DAVA
