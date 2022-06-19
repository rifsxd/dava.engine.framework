#include "REPlatform/Deprecated/SceneValidator.h"
#include "REPlatform/DataNodes/ProjectManagerData.h"
#include "REPlatform/Global/StringConstants.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/SceneHelper.h"

#include <QtTools/ConsoleWidget/PointerSerializer.h>
#include <TArc/Core/Deprecated.h>

#include <Math/Transform.h>
#include <Render/Highlevel/Heightmap.h>
#include <Render/Highlevel/RenderPassNames.h>
#include <Render/Image/LibPVRHelper.h>
#include <Render/Material/NMaterialNames.h>
#include <Render/TextureDescriptor.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Systems/QualitySettingsSystem.h>

namespace DAVA
{
template <typename... A>
void PushLogMessage(Entity* object, const char* format, A... args)
{
    String infoText = Format(format, args...);
    if (nullptr != object)
        infoText += PointerSerializer::FromPointer(object);
    Logger::Error(infoText.c_str());
}

void SceneValidator::ValidateScene(Scene* scene, const FilePath& scenePath)
{
    if (scene != nullptr)
    {
        String tmp = scenePath.GetAbsolutePathname();
        size_t pos = tmp.find("/Data");
        if (pos != String::npos)
        {
            SetPathForChecking(tmp.substr(0, pos + 1));
            sceneName = scenePath.GetFilename();
        }

        ValidateSceneNode(scene);
        ValidateMaterials(scene);

        for (Set<Entity*>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
        {
            Entity* node = *it;
            if (node->GetParent() != nullptr)
            {
                node->GetParent()->RemoveNode(node);
            }
        }

        for (Set<Entity*>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
        {
            Entity* node = *it;
            SafeRelease(node);
        }

        emptyNodesForDeletion.clear();
    }
    else
    {
        PushLogMessage(nullptr, "Scene is not initialized!");
    }
}

void SceneValidator::ValidateScales(Scene* scene)
{
    if (nullptr == scene)
        PushLogMessage(nullptr, "Scene is not initialized!");
    else
        ValidateScalesInternal(scene);
}

void SceneValidator::ValidateScalesInternal(Entity* sceneNode)
{
    if (nullptr == sceneNode)
    {
        return;
    }

    TransformComponent* tc = sceneNode->GetComponent<TransformComponent>();
    const Matrix4& t = tc->GetLocalMatrix();
    float32 sx = sqrt(t._00 * t._00 + t._10 * t._10 + t._20 * t._20);
    float32 sy = sqrt(t._01 * t._01 + t._11 * t._11 + t._21 * t._21);
    float32 sz = sqrt(t._02 * t._02 + t._12 * t._12 + t._22 * t._22);

    if ((!FLOAT_EQUAL(sx, 1.0f)) || (!FLOAT_EQUAL(sy, 1.0f)) || (!FLOAT_EQUAL(sz, 1.0f)))
    {
        PushLogMessage(sceneNode, "Node %s: has scale (%.3f, %.3f, %.3f) ! Re-design level. Scene: %s",
                       sceneNode->GetName().c_str(), sx, sy, sz, sceneName.c_str());
    }

    int32 count = sceneNode->GetChildrenCount();
    for (int32 i = 0; i < count; ++i)
    {
        ValidateScalesInternal(sceneNode->GetChild(i));
    }
}

void SceneValidator::ValidateSceneNode(Entity* sceneNode)
{
    if (nullptr == sceneNode)
    {
        return;
    }

    int32 count = sceneNode->GetChildrenCount();
    for (int32 i = 0; i < count; ++i)
    {
        Entity* node = sceneNode->GetChild(i);

        ValidateRenderComponent(node);
        ValidateParticleEffectComponent(node);
        ValidateSceneNode(node);
        ValidateNodeCustomProperties(node);
    }
}

void SceneValidator::ValidateNodeCustomProperties(Entity* sceneNode)
{
    if (!GetLight(sceneNode))
    {
        KeyedArchive* props = GetCustomPropertiesArchieve(sceneNode);
        if (props != nullptr)
        {
            props->DeleteKey("editor.staticlight.used");
            props->DeleteKey("editor.staticlight.enable");
            props->DeleteKey("editor.staticlight.castshadows");
            props->DeleteKey("editor.staticlight.receiveshadows");
            props->DeleteKey("lightmap.size");
        }
    }
}

void SceneValidator::ValidateRenderComponent(Entity* ownerNode)
{
    RenderComponent* rc = ownerNode->GetComponent<RenderComponent>();
    if (nullptr == rc)
    {
        return;
    }

    RenderObject* ro = rc->GetRenderObject();
    if (nullptr == ro)
    {
        return;
    }

    uint32 count = ro->GetRenderBatchCount();
    for (uint32 b = 0; b < count; ++b)
    {
        RenderBatch* renderBatch = ro->GetRenderBatch(b);
        ValidateRenderBatch(ownerNode, renderBatch);
    }

    if (ro->GetType() == RenderObject::TYPE_LANDSCAPE)
    {
        ownerNode->SetLocked(true);
        FixIdentityTransform(ownerNode, Format("Landscape had wrong transform. Please re-save scene: %s", sceneName.c_str()));

        Landscape* landscape = static_cast<Landscape*>(ro);
        ValidateLandscape(landscape);

        ValidateCustomColorsTexture(ownerNode);
    }

    if (ro->GetType() == RenderObject::TYPE_VEGETATION)
    {
        ownerNode->SetLocked(true);
        FixIdentityTransform(ownerNode, Format("Vegetation had wrong transform. Please re-save scene: %s", sceneName.c_str()));
    }
}

void SceneValidator::FixIdentityTransform(Entity* ownerNode, const String& errorMessage)
{
    TransformComponent* ownerTC = ownerNode->GetComponent<TransformComponent>();
    if (ownerTC->GetLocalTransform() != Transform())
    {
        ownerTC->SetLocalTransform(Transform());
        SceneEditor2* sc = dynamic_cast<SceneEditor2*>(ownerNode->GetScene());
        if (sc != nullptr)
        {
            sc->MarkAsChanged();
        }
        PushLogMessage(ownerNode, errorMessage.c_str());
    }
}

void SceneValidator::ValidateParticleEffectComponent(Entity* ownerNode) const
{
    ParticleEffectComponent* effect = GetEffectComponent(ownerNode);
    if (effect != nullptr)
    {
        uint32 count = effect->GetEmittersCount();
        for (uint32 i = 0; i < count; ++i)
        {
            ValidateParticleEmitter(effect->GetEmitterInstance(i), effect->GetEntity());
        }
    }
}

void SceneValidator::ValidateParticleEmitter(ParticleEmitterInstance* instance, Entity* owner) const
{
    DVASSERT(instance);

    if (nullptr == instance)
        return;

    auto emitter = instance->GetEmitter();

    if (emitter->configPath.IsEmpty())
    {
        PushLogMessage(owner, "Empty config path for emitter %s. Scene: %s", emitter->name.c_str(), sceneName.c_str());
    }

    for (auto layer : emitter->layers)
    {
        if (layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
        {
            ValidateParticleEmitter(layer->innerEmitter, owner);
        }
    }
}

void SceneValidator::ValidateRenderBatch(Entity* ownerNode, RenderBatch* renderBatch)
{
}

void SceneValidator::ValidateMaterials(Scene* scene)
{
    Set<NMaterial*> materials;
    SceneHelper::BuildMaterialList(scene, materials, false);
    auto globalMaterial = scene->GetGlobalMaterial();
    if (nullptr != globalMaterial)
    {
        materials.erase(globalMaterial);
    }

    const Vector<MaterialTemplateInfo>* materialTemplates = nullptr;
    ProjectManagerData* data = Deprecated::GetDataNode<ProjectManagerData>();
    if (data != nullptr)
    {
        materialTemplates = data->GetMaterialTemplatesInfo();
    }

    FastName textureNames[] = {
        NMaterialTextureName::TEXTURE_ALBEDO,
        NMaterialTextureName::TEXTURE_NORMAL,
        NMaterialTextureName::TEXTURE_DETAIL,
        NMaterialTextureName::TEXTURE_LIGHTMAP,
        NMaterialTextureName::TEXTURE_DECAL,
        NMaterialTextureName::TEXTURE_CUBEMAP,
        NMaterialTextureName::TEXTURE_DECALMASK,
        NMaterialTextureName::TEXTURE_DECALTEXTURE,
    };

    Map<Texture*, String> texturesMap;
    auto endItMaterials = materials.end();
    for (auto it = materials.begin(); it != endItMaterials; ++it)
    {
        /// pre load all textures from all configs
        NMaterial* material = *it;
        uint32 currentConfig = material->GetCurrentConfigIndex();
        for (uint32 i = 0; i < material->GetConfigCount(); ++i)
        {
            material->SetCurrentConfigIndex(i);
            material->PreBuildMaterial(PASS_FORWARD);
        }

        material->SetCurrentConfigIndex(currentConfig);

        for (const FastName& textureName : textureNames)
        {
            if ((*it)->HasLocalTexture(textureName))
            {
                Texture* tex = (*it)->GetLocalTexture(textureName);
                if ((*it)->GetParent())
                {
                    texturesMap[tex] = Format("Material: %s (parent - %s). Texture %s.", (*it)->GetMaterialName().c_str(), (*it)->GetParent()->GetMaterialName().c_str(), textureName.c_str());
                }
                else
                {
                    texturesMap[tex] = Format("Material: %s. Texture %s.", (*it)->GetMaterialName().c_str(), textureName.c_str());
                }
            }
        }

        bool qualityGroupIsOk = false;
        FastName materialGroup = (*it)->GetQualityGroup();

        // if some group is set in material we should check it exists in quality system
        if (materialGroup.IsValid())
        {
            size_t qcount = QualitySettingsSystem::Instance()->GetMaterialQualityGroupCount();
            for (size_t q = 0; q < qcount; ++q)
            {
                if (materialGroup == QualitySettingsSystem::Instance()->GetMaterialQualityGroupName(q))
                {
                    qualityGroupIsOk = true;
                    break;
                }
            }

            if (!qualityGroupIsOk)
            {
                Logger::Error("Material \"%s\" has unknown quality group \"%s\"", (*it)->GetMaterialName().c_str(), materialGroup.c_str());
            }
        }

        const FastName& fxName = (*it)->GetEffectiveFXName();
        if (fxName.IsValid() && materialTemplates && !materialTemplates->empty() && fxName != NMaterialName::SHADOW_VOLUME) //ShadowVolume material is non-assignable and it's okey
        {
            // ShadowVolume material is non-assignable and it's okey
            bool templateFound = false;
            for (const MaterialTemplateInfo& materialTemplate : *materialTemplates)
            {
                if (0 == materialTemplate.path.compare(fxName.c_str()))
                {
                    templateFound = true;
                    break;
                }
            }
            if (!templateFound)
            {
                Logger::Error("Material \"%s\" has non-assignable template", (*it)->GetMaterialName().c_str());
            }
        }
    }

    auto endItTextures = texturesMap.end();
    for (auto it = texturesMap.begin(); it != endItTextures; ++it)
    {
        ValidateTexture(it->first, it->second);
    }
}

void SceneValidator::ValidateLandscape(Landscape* landscape)
{
    if (nullptr == landscape)
        return;
    ValidateLandscapeTexture(landscape, Landscape::TEXTURE_COLOR);
    ValidateLandscapeTexture(landscape, Landscape::TEXTURE_TILE);
    ValidateLandscapeTexture(landscape, Landscape::TEXTURE_TILEMASK);

    //validate heightmap
    bool pathIsCorrect = ValidatePathname(landscape->GetHeightmapPathname(), String("Landscape. Heightmap."));
    if (!pathIsCorrect)
    {
        ProjectManagerData* data = Deprecated::GetDataNode<ProjectManagerData>();
        String path = landscape->GetHeightmapPathname().GetAbsolutePathname();
        if (data != nullptr)
        {
            path = landscape->GetHeightmapPathname().GetRelativePathname(data->GetDataSource3DPath());
        }
        PushLogMessage(nullptr, "Wrong path of Heightmap: %s. Scene: %s", path.c_str(), sceneName.c_str());
    }
}

void SceneValidator::ValidateLandscapeTexture(Landscape* landscape, const FastName& texLevel)
{
    Texture* texture = landscape->GetMaterial()->GetEffectiveTexture(texLevel);
    if (texture)
    {
        FilePath landTexName = landscape->GetMaterial()->GetEffectiveTexture(texLevel)->GetPathname();
        if (!IsTextureDescriptorPath(landTexName) && landTexName.GetAbsolutePathname().size() > 0)
        {
            texture->SetPathname(TextureDescriptor::GetDescriptorPathname(landTexName));
        }

        ValidateTexture(texture, Format("Landscape. %s", texLevel.c_str()));
    }
}

VariantType* SceneValidator::GetCustomPropertyFromParentsTree(Entity* ownerNode, const String& key)
{
    KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (nullptr == props)
    {
        return 0;
    }

    if (props->IsKeyExists(key))
    {
        return props->GetVariant(key);
    }
    else
    {
        return GetCustomPropertyFromParentsTree(ownerNode->GetParent(), key);
    }
}

bool SceneValidator::NodeRemovingDisabled(Entity* node)
{
    KeyedArchive* customProperties = GetCustomPropertiesArchieve(node);
    return (customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_DO_NOT_REMOVE));
}

void SceneValidator::ValidateTexture(Texture* texture, const String& validatedObjectName)
{
    if (nullptr == texture)
    {
        return;
    }

    const FilePath& texturePathname = texture->GetPathname();

    String path = texturePathname.GetRelativePathname(pathForChecking);
    String textureInfo = path + " for object: " + validatedObjectName;

    if (texture->IsPinkPlaceholder())
    {
        if (texturePathname.IsEmpty())
        {
            PushLogMessage(nullptr, "Texture not set for object: %s. Scene: %s", validatedObjectName.c_str(), sceneName.c_str());
        }
        else
        {
            PushLogMessage(nullptr, "Can't load texture: %s. Scene: %s", textureInfo.c_str(), sceneName.c_str());
        }
        return;
    }

    bool pathIsCorrect = ValidatePathname(texturePathname, validatedObjectName);
    if (!pathIsCorrect)
    {
        PushLogMessage(nullptr, "Wrong path of: %s. Scene: %s", textureInfo.c_str(), sceneName.c_str());
        return;
    }

    if (!IsPowerOf2(texture->GetWidth()) || !IsPowerOf2(texture->GetHeight()))
    {
        PushLogMessage(nullptr, "Texture %s has now power of two dimensions. Scene: %s", textureInfo.c_str(), sceneName.c_str());
    }

    if ((texture->GetWidth() > 2048) || (texture->GetHeight() > 2048))
    {
        PushLogMessage(nullptr, "Texture %s is too big. Scene: %s", textureInfo.c_str(), sceneName.c_str());
    }
}

bool SceneValidator::WasTextureChanged(Texture* texture, eGPUFamily forGPU)
{
    if (IsFBOTexture(texture))
    {
        return false;
    }

    FilePath texturePathname = texture->GetPathname();
    return (IsPathCorrectForProject(texturePathname) && IsTextureChanged(texturePathname, forGPU));
}

bool SceneValidator::IsFBOTexture(Texture* texture)
{
    if (texture->isRenderTarget)
    {
        return true;
    }

    String::size_type textTexturePos = texture->GetPathname().GetAbsolutePathname().find("Text texture");
    if (String::npos != textTexturePos)
    {
        return true; //is text texture
    }

    return false;
}

FilePath SceneValidator::SetPathForChecking(const FilePath& pathname)
{
    FilePath oldPath = pathForChecking;
    pathForChecking = pathname;
    return oldPath;
}

bool SceneValidator::ValidateTexturePathname(const FilePath& pathForValidation)
{
    DVASSERT(!pathForChecking.IsEmpty(), "Need to set pathname for DataSource folder");

    bool pathIsCorrect = IsPathCorrectForProject(pathForValidation);
    if (pathIsCorrect)
    {
        String textureExtension = pathForValidation.GetExtension();
        if (!TextureDescriptor::IsSupportedTextureExtension(textureExtension))
        {
            PushLogMessage(nullptr, "Path %s has incorrect extension. Scene: %s",
                           pathForValidation.GetAbsolutePathname().c_str(), sceneName.c_str());
            return false;
        }
    }
    else
    {
        PushLogMessage(nullptr, "Path %s is incorrect for project %s. Scene: %s",
                       pathForValidation.GetAbsolutePathname().c_str(), pathForChecking.GetAbsolutePathname().c_str(), sceneName.c_str());
    }

    return pathIsCorrect;
}

bool SceneValidator::ValidateHeightmapPathname(const FilePath& pathForValidation)
{
    DVASSERT(!pathForChecking.IsEmpty(), "Need to set pathname for DataSource folder");

    bool pathIsCorrect = IsPathCorrectForProject(pathForValidation);
    if (pathIsCorrect)
    {
        auto extension = pathForValidation.GetExtension();

        bool isSourceTexture = false;
        bool isHeightmap = false;
        if (!extension.empty())
        {
            if (TextureDescriptor::IsSourceTextureExtension(extension))
                isSourceTexture = true;
            else if (CompareCaseInsensitive(extension, Heightmap::FileExtension()) == 0)
                isHeightmap = true;
        }

        pathIsCorrect = isSourceTexture || isHeightmap;
        if (!pathIsCorrect)
        {
            PushLogMessage(nullptr, "Heightmap path %s is wrong. Scene: %s",
                           pathForValidation.GetAbsolutePathname().c_str(), sceneName.c_str());
            return false;
        }

        ScopedPtr<Heightmap> heightmap(new Heightmap());
        if (isSourceTexture)
        {
            ScopedPtr<Image> image(ImageSystem::LoadSingleMip(pathForValidation));
            pathIsCorrect = heightmap->BuildFromImage(image);
        }
        else
        {
            pathIsCorrect = heightmap->Load(pathForValidation);
        }

        if (!pathIsCorrect)
        {
            PushLogMessage(nullptr, "Can't load Heightmap from path %s. Scene: %s",
                           pathForValidation.GetAbsolutePathname().c_str(), sceneName.c_str());
            return false;
        }

        pathIsCorrect = IsPowerOf2(heightmap->Size());
        if (!pathIsCorrect)
        {
            PushLogMessage(nullptr, "Heightmap %s has wrong size. Scene: %s",
                           pathForValidation.GetAbsolutePathname().c_str(), sceneName.c_str());
        }

        return pathIsCorrect;
    }
    else
    {
        PushLogMessage(nullptr, "Path %s is incorrect for project %s.",
                       pathForValidation.GetAbsolutePathname().c_str(), pathForChecking.GetAbsolutePathname().c_str());
    }

    return pathIsCorrect;
}

bool SceneValidator::ValidatePathname(const FilePath& pathForValidation, const String& validatedObjectName)
{
    DVASSERT(!pathForChecking.IsEmpty());
    //Need to set path to DataSource/3d for path correction
    //Use SetPathForChecking();

    String pathname = pathForValidation.GetAbsolutePathname();

    String::size_type fboFound = pathname.find(String("FBO"));
    String::size_type resFound = pathname.find(String("~res:"));
    if ((String::npos != fboFound) || (String::npos != resFound))
    {
        return true;
    }

    return IsPathCorrectForProject(pathForValidation);
}

bool SceneValidator::IsPathCorrectForProject(const FilePath& pathname)
{
    String normalizedPath = pathname.GetAbsolutePathname();
    String::size_type foundPos = normalizedPath.find(pathForChecking.GetAbsolutePathname());
    return (String::npos != foundPos);
}

void SceneValidator::EnumerateNodes(Scene* scene)
{
    int32 nodesCount = 0;
    if (scene != nullptr)
    {
        for (int32 i = 0; i < scene->GetChildrenCount(); ++i)
        {
            nodesCount += EnumerateSceneNodes(scene->GetChild(i));
        }
    }
}

int32 SceneValidator::EnumerateSceneNodes(Entity* node)
{
    //TODO: lode node can have several nodes at layer

    int32 nodesCount = 1;
    for (int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        nodesCount += EnumerateSceneNodes(node->GetChild(i));
    }

    return nodesCount;
}

bool SceneValidator::IsTextureChanged(const FilePath& texturePathname, eGPUFamily forGPU)
{
    bool isChanged = false;

    TextureDescriptor* descriptor = TextureDescriptor::CreateFromFile(texturePathname);
    if (descriptor != nullptr)
    {
        isChanged = IsTextureChanged(descriptor, forGPU);
        delete descriptor;
    }

    return isChanged;
}

bool SceneValidator::IsTextureChanged(const TextureDescriptor* descriptor, eGPUFamily forGPU)
{
    DVASSERT(descriptor);

    return !descriptor->IsCompressedTextureActual(forGPU);
}

bool SceneValidator::IsTextureDescriptorPath(const FilePath& path)
{
    return path.IsEqualToExtension(TextureDescriptor::GetDescriptorExtension());
}

void SceneValidator::ValidateCustomColorsTexture(Entity* landscapeEntity)
{
    KeyedArchive* customProps = GetCustomPropertiesArchieve(landscapeEntity);
    if (customProps != nullptr && customProps->IsKeyExists(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP))
    {
        String currentSaveName = customProps->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
        FilePath path = "/" + currentSaveName;

        if (!TextureDescriptor::IsSourceTextureExtension(path.GetExtension()))
        {
            PushLogMessage(landscapeEntity, "Custom colors texture has to have .png, .jpeg or .tga extension. Scene: %s", sceneName.c_str());
        }

        String::size_type foundPos = currentSaveName.find("DataSource/3d/");
        if (String::npos == foundPos)
        {
            PushLogMessage(landscapeEntity, "Custom colors texture has to begin from DataSource/3d/. Scene: %s", sceneName.c_str());
        }
    }
}

bool SceneValidator::ValidateColor(Color& color)
{
    bool ok = true;
    for (int32 i = 0; i < 4; ++i)
    {
        if (color.color[i] < 0.f || color.color[i] > 1.f)
        {
            color.color[i] = Clamp(color.color[i], 0.f, 1.f);
            ok = false;
        }
    }

    return ok;
}

void SceneValidator::FindSwitchesWithDifferentLODs(Entity* entity, Set<FastName>& names)
{
    if (IsEntityHasDifferentLODsCount(entity))
    {
        names.insert(entity->GetName());
    }
    else
    {
        const uint32 count = entity->GetChildrenCount();
        for (uint32 i = 0; i < count; ++i)
        {
            FindSwitchesWithDifferentLODs(entity->GetChild(i), names);
        }
    }
}

bool SceneValidator::IsEntityHasDifferentLODsCount(Entity* entity)
{
    if ((GetSwitchComponent(entity) == NULL) || (GetLodComponent(entity) == NULL))
    {
        return false;
    }

    RenderObject* ro = GetRenderObject(entity);
    if (ro != nullptr)
    {
        return IsObjectHasDifferentLODsCount(ro);
    }

    return false;
}

bool SceneValidator::IsObjectHasDifferentLODsCount(RenderObject* renderObject)
{
    DVASSERT(renderObject);

    int32 maxLod[2] = { -1, -1 };

    const uint32 count = renderObject->GetRenderBatchCount();
    for (uint32 i = 0; i < count; ++i)
    {
        int32 lod, sw;
        renderObject->GetRenderBatch(i, lod, sw);

        DVASSERT(sw < 2);
        if ((lod > maxLod[sw]) && (sw >= 0 && sw < 2))
        {
            maxLod[sw] = lod;
        }
    }

    return ((maxLod[0] != maxLod[1]) && (maxLod[0] != -1 && maxLod[1] != -1));
}

void SceneValidator::ExtractEmptyRenderObjects(Entity* entity)
{
    auto renderObject = GetRenderObject(entity);
    if ((nullptr != renderObject) && (0 == renderObject->GetRenderBatchCount()) && RenderObject::TYPE_MESH == renderObject->GetType())
    {
        entity->RemoveComponent<RenderComponent>();
        PushLogMessage(entity, "Entity %s has empty render object", entity->GetName().c_str());
    }

    const uint32 count = entity->GetChildrenCount();
    for (uint32 i = 0; i < count; ++i)
    {
        ExtractEmptyRenderObjects(entity->GetChild(i));
    }
}
} // namespace DAVA
