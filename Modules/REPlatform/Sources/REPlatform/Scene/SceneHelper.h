#pragma once

#include <FileSystem/FilePath.h>
#include <Render/Texture.h>

namespace DAVA
{
class SceneEditor2;
class Scene;
class Texture;
class Entity;
class NMaterial;

namespace SceneHelper
{
class TextureCollector
{
public:
    enum Options
    {
        Default = 0,
        IncludeNullTextures = 0x1,
        OnlyActiveTextures = 0x2
    };

    TextureCollector(uint32 options = Default);

    void Apply(NMaterial* material);
    TexturesMap& GetTextures();

private:
    bool includeNullTextures = true;
    bool onlyActiveTextures = false;
    TexturesMap textureMap;
};

void EnumerateSceneTextures(Scene* forScene, TextureCollector& collector);
void EnumerateEntityTextures(Scene* forScene, Entity* forNode, TextureCollector& collector);

// enumerates materials from render batches and their parents
void EnumerateMaterials(Entity* forNode, Set<NMaterial*>& materials);

// enumerates only materials from render batches
void EnumerateMaterialInstances(Entity* forNode, Set<NMaterial*>& materials);
int32 EnumerateModifiedTextures(Scene* forScene, Map<Texture*, Vector<eGPUFamily>>& textures);
Entity* CloneEntityWithMaterials(Entity* fromNode);

void BuildMaterialList(Entity* forNode, Set<NMaterial*>& materialList, bool includeRuntime = true);
bool IsEntityChildRecursive(Entity* root, Entity* child);
}

void LookAtSelection(SceneEditor2* scene);
void RemoveSelection(SceneEditor2* scene);
void LockTransform(SceneEditor2* scene);
void UnlockTransform(SceneEditor2* scene);
} // namespace DAVA