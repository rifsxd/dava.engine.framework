#include "REPlatform/DataNodes/Settings/RESettings.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Base/GlobalEnum.h>

ENUM_DECLARE(DAVA::RenderingBackend)
{
#if defined(__DAVAENGINE_WIN32__)
    // Uncomment this line to allow DX11 backend
    //ENUM_ADD_DESCR(static_cast<int>(DAVA::RenderingBackend::DX11), "DirectX 11");
    ENUM_ADD_DESCR(static_cast<int>(DAVA::RenderingBackend::DX9), "DirectX 9");
#endif
    ENUM_ADD_DESCR(static_cast<int>(DAVA::RenderingBackend::OpenGL), "OpenGL");
}

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(GeneralSettings)
{
    ReflectionRegistrator<GeneralSettings>::Begin()[M::DisplayName("General"), M::SettingsSortKey(100)]
    .ConstructorByPointer()
    .Field("ReloadParticlesOnProjectOpening", &GeneralSettings::reloadParticlesOnProjectOpening)[M::DisplayName("Convert particles on project opening")]
    .Field("PreviewEnabled", &GeneralSettings::previewEnabled)[M::DisplayName("Show scene preview")]
    .Field("CompressionQuality", &GeneralSettings::compressionQuality)[M::DisplayName("Compression quality"), M::EnumT<TextureConverter::eConvertQuality>()]
    .Field("ShowErrorDialog", &GeneralSettings::showErrorDialog)[M::DisplayName("Show error dialog")]
    .Field("recentScenesCount", &GeneralSettings::recentScenesCount)[M::DisplayName("Number of recent scenes"), M::Range(0, 50, 1)]
    .Field("materialEditorSwitchColor0", &GeneralSettings::materialEditorSwitchColor0)[M::DisplayName("Switch 0 color"), M::Group("Material Editor")]
    .Field("materialEditorSwitchColor1", &GeneralSettings::materialEditorSwitchColor1)[M::DisplayName("Switch 1 color"), M::Group("Material Editor")]
    .Field("materialEditorLod0Color", &GeneralSettings::materialEditorLodColor0)[M::DisplayName("Lod 0 color"), M::Group("Material Editor")]
    .Field("materialEditorLod1Color", &GeneralSettings::materialEditorLodColor1)[M::DisplayName("Lod 1 color"), M::Group("Material Editor")]
    .Field("materialEditorLod2Color", &GeneralSettings::materialEditorLodColor2)[M::DisplayName("Lod 2 color"), M::Group("Material Editor")]
    .Field("materialEditorLod3Color", &GeneralSettings::materialEditorLodColor3)[M::DisplayName("Lod 3 color"), M::Group("Material Editor")]
    .Field("ParticleDebugAlphaThreshold", &GeneralSettings::particleDebugAlphaTheshold)[M::DisplayName("Particle alpha threshold"), M::Group("Particle editor")]
    .Field("lodEditorLodColor0", &GeneralSettings::lodEditorLodColor0)[M::DisplayName("Lod 0 color"), M::Group("LOD Editor")]
    .Field("lodEditorLodColor1", &GeneralSettings::lodEditorLodColor1)[M::DisplayName("Lod 1 color"), M::Group("LOD Editor")]
    .Field("lodEditorLodColor2", &GeneralSettings::lodEditorLodColor2)[M::DisplayName("Lod 2 color"), M::Group("LOD Editor")]
    .Field("lodEditorLodColor3", &GeneralSettings::lodEditorLodColor3)[M::DisplayName("Lod 3 color"), M::Group("LOD Editor")]
    .Field("inactiveColor", &GeneralSettings::inactiveColor)[M::DisplayName("Inactive color"), M::Group("LOD Editor")]
    .Field("fitSliders", &GeneralSettings::fitSliders)[M::DisplayName("Fit sliders to maximum distance"), M::Group("LOD Editor")]
    .Field("heightMaskColor0", &GeneralSettings::heightMaskColor0)[M::DisplayName("Color 0"), M::Group("Height Mask Tool")]
    .Field("heightMaskColor1", &GeneralSettings::heightMaskColor1)[M::DisplayName("Color 1"), M::Group("Height Mask Tool")]
    .Field("useAssetCache", &GeneralSettings::useAssetCache)[M::DisplayName("Use cache"), M::Group("Asset Cache")]
    .Field("assetCacheIP", &GeneralSettings::assetCacheIP)[M::DisplayName("IP"), M::Group("Asset Cache")]
    .Field("assetCachePort", &GeneralSettings::assetCachePort)[M::DisplayName("Port"), M::Group("Asset Cache")]
    .Field("assetCacheTimeout", &GeneralSettings::assetCacheTimeout)[M::DisplayName("Timeout"), M::Group("Asset Cache")]
    .Field("autoConversion", &GeneralSettings::autoConversion)[M::DisplayName("Convert automatically"), M::Group("Texture Browser")]
    .Field("renderBackend", &GeneralSettings::renderBackend)[
#if defined(DEPLOY_BUILD)
    M::HiddenField(),
#endif
    M::DisplayName("Backend"), M::Group("Renderer"), M::EnumT<RenderingBackend>()]
    .Field("wheelMoveCamera", &GeneralSettings::wheelMoveCamera)[M::DisplayName("Move camera on Wheel"), M::Group("Mouse")]
    .Field("wheelMoveIntensity", &GeneralSettings::wheelMoveIntensity)[M::DisplayName("Move intensity on Wheel"), M::Group("Mouse")]
    .Field("invertWheel", &GeneralSettings::invertWheel)[M::DisplayName("Invert Wheel"), M::Group("Mouse")]
    .End();
}

void GeneralSettings::Load(const PropertiesItem& settingsNode)
{
    SettingsNode::Load(settingsNode);
#if defined(DEPLOY_BUILD)
    renderBackend = RenderingBackend::OpenGL;
#endif
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommonInternalSettings)
{
    ReflectionRegistrator<CommonInternalSettings>::Begin()[M::HiddenField()]
    .ConstructorByPointer()
    .Field("textureViewGPU", &CommonInternalSettings::textureViewGPU)
    .Field("spritesViewGPU", &CommonInternalSettings::spritesViewGPU)
    .Field("cubemapLastFaceDir", &CommonInternalSettings::cubemapLastFaceDir)
    .Field("cubemapLastProjDir", &CommonInternalSettings::cubemapLastProjDir)
    .Field("emitterSaveDir", &CommonInternalSettings::emitterSaveDir)
    .Field("emitterLoadDir", &CommonInternalSettings::emitterLoadDir)
    .Field("materialLightViewMode", &CommonInternalSettings::materialLightViewMode)
    .Field("materialShowLightmapCanvas", &CommonInternalSettings::materialShowLightmapCanvas)
    .Field("lodEditorSceneMode", &CommonInternalSettings::lodEditorSceneMode)
    .Field("lodEditorRecursive", &CommonInternalSettings::lodEditorRecursive)
    .Field("runActionEventType", &CommonInternalSettings::runActionEventType)
    .Field("beastLightmapsDefaultDir", &CommonInternalSettings::beastLightmapsDefaultDir)
    .Field("imageSplitterPath", &CommonInternalSettings::imageSplitterPath)
    .Field("imageSplitterPathSpecular", &CommonInternalSettings::imageSplitterPathSpecular)
    .Field("enableSound", &CommonInternalSettings::enableSound)
    .Field("gizmoEnabled", &CommonInternalSettings::gizmoEnabled)
    .Field("validateMatrices", &CommonInternalSettings::validateMatrices)
    .Field("validateSameNames", &CommonInternalSettings::validateSameNames)
    .Field("validateCollisionProperties", &CommonInternalSettings::validateCollisionProperties)
    .Field("validateTextureRelevance", &CommonInternalSettings::validateTextureRelevance)
    .Field("validateMaterialGroups", &CommonInternalSettings::validateMaterialGroups)
    .Field("validateShowConsole", &CommonInternalSettings::validateShowConsole)
    .Field("logWidgetState", &CommonInternalSettings::logWidgetState)
    .End();
}
} // namespace DAVA