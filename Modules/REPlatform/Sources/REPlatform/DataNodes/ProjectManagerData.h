#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>
#include <TArc/DataProcessing/PropertiesHolder.h>

#include <FileSystem/FilePath.h>

class ProjectManagerModule;
namespace DAVA
{
class SpritesPackerModule;
class EditorConfig;

struct MaterialTemplateInfo
{
    String name;
    String path;
    Vector<String> qualities;
};

class ProjectManagerData : public TArcDataNode
{
public:
    ProjectManagerData();
    ProjectManagerData(const ProjectManagerData& other) = delete;
    ~ProjectManagerData();

    bool IsOpened() const;

    const FilePath& GetProjectPath() const;
    FilePath GetDataPath() const;
    FilePath GetDataSourcePath() const;
    FilePath GetDataSource3DPath() const;
    FilePath GetParticlesConfigPath() const;
    FilePath GetParticlesGfxPath() const;

    static FilePath CreateProjectPathFromPath(const FilePath& pathname);
    static FilePath GetDataSourcePath(const FilePath& pathname);
    static FilePath GetDataPath(const FilePath& pathname);
    const EditorConfig* GetEditorConfig() const;
    const Vector<MaterialTemplateInfo>* GetMaterialTemplatesInfo() const;
    DAVA_DEPRECATED(const SpritesPackerModule* GetSpritesModules() const);

    static FilePath GetDataSource3DPath(const FilePath& projectPath);

public:
    static const String ProjectPathProperty;

private:
    friend class ::ProjectManagerModule;
    friend class ProjectResources;

    std::unique_ptr<SpritesPackerModule> spritesPacker;
    std::unique_ptr<EditorConfig> editorConfig;
    Vector<MaterialTemplateInfo> materialTemplatesInfo;

    FilePath projectPath;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ProjectManagerData, TArcDataNode)
    {
        ReflectionRegistrator<ProjectManagerData>::Begin()
        .Field(ProjectPathProperty.c_str(), &ProjectManagerData::GetProjectPath, nullptr)
        .End();
    }
};
} // namespace DAVA