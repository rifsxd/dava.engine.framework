#include "REPlatform/DataNodes/ProjectManagerData.h"

#include "REPlatform/Deprecated/EditorConfig.h"
#include "REPlatform/DataNodes/SpritesPackerModule.h"

namespace DAVA
{
namespace ProjectManagerDataDetails
{
const char* DATA_PATH = "Data/";
const char* DATASOURCE_PATH = "DataSource/";
const char* DATASOURCE_3D_PATH = "DataSource/3d/";
const char* PARTICLE_CONFIG_PATH = "DataSource/Configs/Particles/";
const char* PARTICLE_GFX_PATH = "DataSource/Gfx/Particles/";
}

const String ProjectManagerData::ProjectPathProperty = String("ProjectPath");

ProjectManagerData::ProjectManagerData()
{
}

ProjectManagerData::~ProjectManagerData() = default;

bool ProjectManagerData::IsOpened() const
{
    return (!projectPath.IsEmpty());
}

const FilePath& ProjectManagerData::GetProjectPath() const
{
    return projectPath;
}

FilePath ProjectManagerData::GetDataPath() const
{
    return projectPath + ProjectManagerDataDetails::DATA_PATH;
}

FilePath ProjectManagerData::GetDataSourcePath() const
{
    return projectPath + ProjectManagerDataDetails::DATASOURCE_PATH;
}

FilePath ProjectManagerData::GetDataSource3DPath() const
{
    return GetDataSource3DPath(projectPath);
}

FilePath ProjectManagerData::GetDataSource3DPath(const FilePath& projectPath)
{
    return projectPath + ProjectManagerDataDetails::DATASOURCE_3D_PATH;
}

FilePath ProjectManagerData::GetParticlesConfigPath() const
{
    return projectPath + ProjectManagerDataDetails::PARTICLE_CONFIG_PATH;
}

FilePath ProjectManagerData::GetParticlesGfxPath() const
{
    return projectPath + ProjectManagerDataDetails::PARTICLE_GFX_PATH;
}

FilePath ProjectManagerData::CreateProjectPathFromPath(const FilePath& pathname)
{
    String fullPath = pathname.GetAbsolutePathname();
    String::size_type pos = fullPath.find("/Data");
    if (pos != String::npos)
    {
        return fullPath.substr(0, pos + 1);
    }

    return FilePath();
}

FilePath ProjectManagerData::GetDataSourcePath(const FilePath& pathname)
{
    String etalon = String("/DataSource");
    String fullPath = pathname.GetAbsolutePathname();
    String::size_type pos = fullPath.find(etalon);
    if (pos != String::npos)
    {
        return fullPath.substr(0, pos + etalon.size() + 1);
    }

    return FilePath();
}

FilePath ProjectManagerData::GetDataPath(const FilePath& pathname)
{
    String etalon = String("/Data");
    String fullPath = pathname.GetAbsolutePathname();
    String::size_type pos = fullPath.find(etalon);
    if (pos != String::npos)
    {
        return fullPath.substr(0, pos + etalon.size() + 1);
    }

    return FilePath();
}

const EditorConfig* ProjectManagerData::GetEditorConfig() const
{
    return editorConfig.get();
}

const SpritesPackerModule* ProjectManagerData::GetSpritesModules() const
{
    return spritesPacker.get();
}

const Vector<MaterialTemplateInfo>* ProjectManagerData::GetMaterialTemplatesInfo() const
{
    return &materialTemplatesInfo;
}
} // namespace DAVA