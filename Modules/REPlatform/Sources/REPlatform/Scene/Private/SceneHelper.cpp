#include "REPlatform/Scene/SceneHelper.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Systems/CameraSystem.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Systems/StructureSystem.h"

#include "REPlatform/DataNodes/ProjectManagerData.h"
#include "REPlatform/DataNodes/SelectableGroup.h"
#include "REPlatform/Deprecated/SceneValidator.h"

#include <TArc/Core/Deprecated.h>

#include <FileSystem/FileSystem.h>
#include <Render/GPUFamilyDescriptor.h>
#include <Render/Texture.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
namespace SceneHelper
{
TextureCollector::TextureCollector(uint32 options)
{
    includeNullTextures = (options & IncludeNullTextures) != 0;
    onlyActiveTextures = (options & OnlyActiveTextures) != 0;
}

void TextureCollector::Apply(NMaterial* material)
{
    Set<MaterialTextureInfo*> materialTextures;
    if (onlyActiveTextures)
        material->CollectActiveLocalTextures(materialTextures);
    else
        material->CollectLocalTextures(materialTextures);

    SceneValidator validator;
    ProjectManagerData* data = Deprecated::GetDataNode<ProjectManagerData>();
    if (data)
    {
        validator.SetPathForChecking(data->GetProjectPath());
    }

    for (auto const& matTex : materialTextures)
    {
        const FilePath& texturePath = matTex->path;
        Texture* texture = matTex->texture;

        if (texturePath.IsEmpty() || !validator.IsPathCorrectForProject(texturePath))
        {
            continue;
        }

        if ((includeNullTextures == false) && (nullptr == texture || texture->isRenderTarget))
        {
            continue;
        }

        textureMap[FILEPATH_MAP_KEY(texturePath)] = texture;
    }
}

TexturesMap& TextureCollector::GetTextures()
{
    return textureMap;
}

void EnumerateSceneTextures(Scene* forScene, TextureCollector& collector)
{
    EnumerateEntityTextures(forScene, forScene, collector);
}

void BuildMaterialList(Entity* forNode, Set<NMaterial*>& materialList, bool includeRuntime)
{
    if (nullptr == forNode)
        return;

    List<NMaterial*> materials;
    forNode->GetDataNodes(materials);

    for (auto& mat : materials)
    {
        if (!includeRuntime && mat->IsRuntime())
        {
            continue;
        }

        materialList.insert(mat);
    }
}

void EnumerateEntityTextures(Scene* forScene, Entity* forNode, TextureCollector& collector)
{
    if (nullptr == forNode || nullptr == forScene)
    {
        return;
    }

    Set<NMaterial*> materials;
    BuildMaterialList(forNode, materials);

    Set<MaterialTextureInfo*> materialTextures;
    for (auto& mat : materials)
    {
        String materialName = mat->GetMaterialName().IsValid() ? mat->GetMaterialName().c_str() : String();
        String parentName = mat->GetParent() && mat->GetParent()->GetMaterialName().IsValid() ? mat->GetParent()->GetMaterialName().c_str() : String();

        if ((parentName.find("Particle") != String::npos) || (materialName.find("Particle") != String::npos))
        { //because particle materials has textures only after first start, so we have different result during scene life.
            continue;
        }

        collector.Apply(mat);
    }
}

int32 EnumerateModifiedTextures(Scene* forScene, Map<Texture*, Vector<eGPUFamily>>& textures)
{
    int32 retValue = 0;
    textures.clear();
    TextureCollector collector;
    EnumerateSceneTextures(forScene, collector);

    for (auto& it : collector.GetTextures())
    {
        Texture* texture = it.second;
        if (nullptr == texture)
        {
            continue;
        }

        TextureDescriptor* descriptor = texture->GetDescriptor();
        DVASSERT(descriptor);
        DVASSERT(descriptor->compression);

        Vector<eGPUFamily> markedGPUs;
        for (int i = 0; i < GPU_DEVICE_COUNT; ++i)
        {
            eGPUFamily gpu = static_cast<eGPUFamily>(i);
            if (GPUFamilyDescriptor::IsFormatSupported(gpu, static_cast<PixelFormat>(descriptor->compression[gpu].format)))
            {
                FilePath texPath = descriptor->GetSourceTexturePathname();
                if (FileSystem::Instance()->Exists(texPath) && !descriptor->IsCompressedTextureActual(gpu))
                {
                    markedGPUs.push_back(gpu);
                    retValue++;
                }
            }
        }
        if (markedGPUs.size() > 0)
        {
            textures[texture] = markedGPUs;
        }
    }
    return retValue;
}

void EnumerateMaterials(Entity* forNode, Set<NMaterial*>& materials)
{
    EnumerateMaterialInstances(forNode, materials);

    // collect parent materials
    for (auto material : materials)
    {
        while (material->GetParent() != nullptr)
        {
            material = material->GetParent();
            materials.insert(material);
        }
    }
}

void EnumerateMaterialInstances(Entity* forNode, Set<NMaterial*>& materials)
{
    uint32 childrenCount = forNode->GetChildrenCount();
    for (uint32 i = 0; i < childrenCount; ++i)
    {
        EnumerateMaterialInstances(forNode->GetChild(i), materials);
    }

    RenderObject* ro = GetRenderObject(forNode);
    if (ro != nullptr)
    {
        uint32 batchCount = ro->GetRenderBatchCount();
        for (uint32 i = 0; i < batchCount; ++i)
        {
            auto material = ro->GetRenderBatch(i)->GetMaterial();
            if (material != nullptr)
            {
                materials.insert(material);
            }
        }
    }
}

Entity* CloneEntityWithMaterials(Entity* fromNode)
{
    Scene* scene = fromNode->GetScene();
    NMaterial* globalMaterial = (scene) ? scene->GetGlobalMaterial() : nullptr;

    Entity* newEntity = fromNode->Clone();

    Set<NMaterial*> materialInstances;
    EnumerateMaterialInstances(newEntity, materialInstances);

    Set<NMaterial*> materialParentsSet;
    for (auto material : materialInstances)
    {
        materialParentsSet.insert(material->GetParent());
    }
    materialParentsSet.erase(globalMaterial);

    Map<NMaterial*, NMaterial*> clonedParents;
    for (auto& mp : materialParentsSet)
    {
        NMaterial* mat = mp ? mp->Clone() : nullptr;
        if (mat && mat->GetParent() == globalMaterial)
        {
            mat->SetParent(nullptr); // exclude material from scene
        }
        clonedParents[mp] = mat;
    }

    for (auto material : materialInstances)
    {
        NMaterial* parent = material->GetParent();
        material->SetParent(clonedParents[parent]);
    }

    for (auto& cp : clonedParents)
    {
        SafeRelease(cp.second);
    }

    return newEntity;
}

bool IsEntityChildRecursive(Entity* root, Entity* child)
{
    if (std::find(root->children.begin(), root->children.end(), child) != root->children.end())
    {
        return true;
    }
    else
    {
        return std::any_of(root->children.begin(), root->children.end(), [&](Entity* ch) { return IsEntityChildRecursive(ch, child); });
    }
}
} // namespace SceneHelper

void LookAtSelection(SceneEditor2* scene)
{
    if (scene != nullptr)
    {
        scene->GetSystem<SceneCameraSystem>()->MoveToSelection(scene->GetSystem<SelectionSystem>()->GetSelection());
    }
}

void RemoveSelection(SceneEditor2* scene)
{
    if (scene == nullptr)
        return;

    const SelectableGroup& selection = scene->GetSystem<SelectionSystem>()->GetSelection();

    SelectableGroup objectsToRemove;
    for (const auto& item : selection.GetContent())
    {
        if (item.CanBeCastedTo<Entity>())
        {
            Entity* entity = item.AsEntity();
            if (entity->GetLocked() || entity->GetNotRemovable())
            {
                //Don't remove entity
                continue;
            }

            Camera* camera = GetCamera(entity);
            if (camera != nullptr && camera == scene->GetCurrentCamera())
            {
                //Don't remove current camera
                continue;
            }
        }

        objectsToRemove.Add(item.GetContainedObject(), item.GetBoundingBox());
    }

    if (objectsToRemove.IsEmpty() == false)
    {
        scene->GetSystem<StructureSystem>()->Remove(objectsToRemove);
    }
}

void LockTransform(SceneEditor2* scene)
{
    if (scene != nullptr)
    {
        const SelectableGroup& selection = scene->GetSystem<SelectionSystem>()->GetSelection();
        scene->GetSystem<EntityModificationSystem>()->LockTransform(selection, true);
    }
}

void UnlockTransform(SceneEditor2* scene)
{
    if (scene != nullptr)
    {
        const SelectableGroup& selection = scene->GetSystem<SelectionSystem>()->GetSelection();
        scene->GetSystem<EntityModificationSystem>()->LockTransform(selection, false);
    }
}
} // namespace DAVA