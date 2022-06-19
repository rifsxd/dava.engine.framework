#pragma once

#include "REPlatform/Scene/Utils/RESceneUtils.h"

#include <FileSystem/FilePath.h>
#include <Render/Texture.h>
#include <Scene3D/Components/ParticleEffectComponent.h>

namespace DAVA
{
class Entity;
class Scene;
class ParticleEmitter;
class SceneSaver final
{
public:
    SceneSaver();
    ~SceneSaver();

    void SetInFolder(const FilePath& folderPathname);
    void SetOutFolder(const FilePath& folderPathname);

    void SaveFile(const String& fileName);
    void ResaveFile(const String& fileName);
    void SaveScene(Scene* scene, const FilePath& fileName);

    void EnableCopyConverted(bool enabled);
    void SetTags(const Vector<String>& tags);

    void ResaveYamlFilesRecursive(const FilePath& folder) const;

private:
    void ReleaseTextures();

    void CopyTextures(Scene* scene);
    void CopyTexture(const FilePath& texturePathname);

    void CopyReferencedObject(Entity* node);
    void CopyAnimationClips(Entity* node);
    void CopySlots(Entity* node, Set<FilePath>& externalScenes);
    void CopyEffects(Entity* node);
    void CopyAllParticlesEmitters(ParticleEmitterInstance* instance);
    void CopyEmitterByPath(const FilePath& emitterConfigPath);
    void CopyEmitter(ParticleEmitter* emitter);
    void ProcessSprite(Sprite* sprite);

    Set<FilePath> EnumAlternativeEmittersFilepaths(const FilePath& originalFilepath) const;

    void CopyCustomColorTexture(Scene* scene, const FilePath& sceneFolder);

    RESceneUtils sceneUtils;
    TexturesMap texturesForSave;
    Set<FilePath> effectFolders;
    Set<FilePath> savedExternalScenes;

    Vector<String> tags;
    bool copyConverted = false;
};
} // namespace DAVA
