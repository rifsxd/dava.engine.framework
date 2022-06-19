#pragma once

#include <Base/BaseTypes.h>
#include <FileSystem/FilePath.h>
#include <Render/RenderBase.h>

namespace DAVA
{
class Scene;
class Entity;
class RenderObject;
class KeyedArchive;
class ParticleEffectComponent;
class ParticleEmitter;
class ParticleEmitterInstance;
class NMaterial;
class Sprite;
class SlotComponent;
class MotionComponent;

class SceneDumper
{
public:
    enum class eMode
    {
        REQUIRED = 0,
        EXTENDED,
        COMPRESSED
    };

    static Set<FilePath> DumpLinks(const FilePath& scenePath, eMode mode, const Vector<eGPUFamily>& compressedGPUs, const Vector<String>& tags);

private:
    static Set<FilePath> DumpLinks(const FilePath& scenePath, eMode mode, const Vector<eGPUFamily>& compressedGPUs, const Vector<String>& tags, Set<FilePath>& dumpedLinks);

    SceneDumper(const FilePath& scenePath, eMode mode, const Vector<eGPUFamily>& compressedGPUs, const Vector<String>& tags);
    ~SceneDumper();

    void DumpLinksRecursive(Entity* entity, Set<FilePath>& links, Set<FilePath>& redumpScenes) const;

    void DumpCustomProperties(KeyedArchive* properties, Set<FilePath>& links) const;
    void DumpRenderObject(RenderObject* renderObject, Set<FilePath>& links) const;
    void DumpMaterial(NMaterial* material, Set<FilePath>& links, Set<FilePath>& descriptorPathnames) const;
    void DumpTextureDescriptor(const FilePath& descriptorPathname, Set<FilePath>& links) const;

    void DumpEffect(ParticleEffectComponent* effect, Set<FilePath>& links) const;
    void DumpEmitter(ParticleEmitterInstance* emitter, Set<FilePath>& links, Set<FilePath>& gfxFolders) const;
    void DumpSlot(SlotComponent* slot, Set<FilePath>& links, Set<FilePath>& redumpScenes) const;
    void DumpAnimations(MotionComponent* motion, Set<FilePath>& links) const;

    void ProcessSprite(Sprite* sprite, Set<FilePath>& links, Set<FilePath>& gfxFolders) const;

    Scene* scene = nullptr;
    FilePath scenePathname;

    Vector<eGPUFamily> compressedGPUs;
    Vector<String> tags;

    SceneDumper::eMode mode = SceneDumper::eMode::REQUIRED;
};

} // namespace DAVA