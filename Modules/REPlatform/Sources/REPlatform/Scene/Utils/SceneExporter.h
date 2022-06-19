#pragma once

#include <TextureCompression/TextureConverter.h>
#include <AssetCache/AssetCache.h>

#include <Utils/StringFormat.h>

namespace DAVA
{
class TextureDescriptor;
class Scene;
class AssetCacheClient;

class SceneExporter final
{
public:
    enum eExportedObjectType : int32
    {
        OBJECT_NONE = -1,

        OBJECT_SCENE = 0,
        OBJECT_TEXTURE,
        OBJECT_HEIGHTMAP,
        OBJECT_EMITTER_CONFIG,
        OBJECT_SLOT_CONFIG,
        OBJECT_ANIMATION_CLIP,

        OBJECT_COUNT
    };

    struct ExportedObjectDesc
    {
        ExportedObjectDesc(eExportedObjectType objType, const Vector<String>& ext)
            : type(objType)
            , extensions(ext)
        {
        }
        eExportedObjectType type;
        Vector<String> extensions;
    };

    static const Array<ExportedObjectDesc, OBJECT_COUNT>& GetExportedObjectsDescriptions();

    struct ExportedObject
    {
        ExportedObject(eExportedObjectType type_, String path)
            : type(type_)
            , relativePathname(std::move(path))
        {
        }

        eExportedObjectType type = OBJECT_NONE;
        String relativePathname;
    };

    using ExportedObjectCollection = Vector<ExportedObject>;

    struct Params
    {
        struct Output
        {
            Output(const FilePath& path, const Vector<eGPUFamily>& gpus, TextureConverter::eConvertQuality quality_, bool useHD)
                : exportForGPUs(gpus)
                , dataFolder(path)
                , quality(quality_)
                , useHDTextures(useHD)
            {
            }
            Vector<eGPUFamily> exportForGPUs;
            FilePath dataFolder;
            TextureConverter::eConvertQuality quality = TextureConverter::eConvertQuality::ECQ_DEFAULT;
            bool useHDTextures = false;
        };

        Vector<Output> outputs;
        FilePath dataSourceFolder;
        String filenamesTag;

        bool optimizeOnExport = false;
    };

    SceneExporter() = default;
    ~SceneExporter();

    void SetExportingParams(const SceneExporter::Params& exportingParams);
    void SetCacheClient(AssetCacheClient* cacheClient, String machineName, String runDate, String comment);

    bool ExportScene(Scene* scene, const FilePath& scenePathname, const FilePath& outScenePathname, Vector<ExportedObjectCollection>& exportedObjects);
    bool ExportObjects(const ExportedObjectCollection& exportedObjects);

private:
    bool PrepareData(const ExportedObjectCollection& exportedObjects);
    bool ExportSceneObject(const ExportedObject& object);
    bool ExportTextureObjectTagged(const ExportedObject& object);
    bool ExportTextureObject(const ExportedObject& object);
    bool ExportSlotObject(const ExportedObject& object);
    bool CopyObject(const ExportedObject& object);

    bool ExportSceneFileInternal(const FilePath& scenePathname, const FilePath& outScenePathname, Vector<ExportedObjectCollection>& exportedObjects); //without cache
    bool ExportDescriptor(TextureDescriptor& descriptor, const Params::Output& output);
    bool SplitCompressedFile(const TextureDescriptor& descriptor, eGPUFamily gpu, const Params::Output& output) const;
    void CollectObjects(Scene* scene, Vector<ExportedObjectCollection>& exportedObjects);

    void CreateFoldersStructure(const ExportedObject& object);
    bool CopyFile(const FilePath& fromPath, const FilePath& toPath) const;
    bool CopyFileToOutput(const FilePath& fromPath, const Params::Output& output) const;

    AssetCacheClient* cacheClient = nullptr;
    AssetCache::CachedItemValue::Description cacheItemDescription;

    SceneExporter::Params exportingParams;
    Vector<ExportedObjectCollection> objectsToExport;

    Set<FilePath> alreadyExportedScenes;

    UnorderedSet<String> cachedFoldersForCreation;
};

bool operator==(const SceneExporter::ExportedObject& left, const SceneExporter::ExportedObject& right);
bool operator<(const SceneExporter::ExportedObject& left, const SceneExporter::ExportedObject& right);

} // namespace DAVA
