#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>
#include <TArc/DataProcessing/PropertiesHolder.h>
#include <TArc/Qt/QtString.h>

#include <Base/Result.h>
#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <Math/Math2D.h>
#include <FileSystem/FilePath.h>

namespace DAVA
{
class YamlNode;
class ResultList;
}

class ProjectData : public DAVA::TArcDataNode
{
public:
    struct ResDir
    {
        DAVA::FilePath absolute;
        DAVA::String relative;
        bool operator==(const ResDir& other) const;
    };

    struct PinnedControl
    {
        ResDir packagePath;
        ResDir iconPath;
        DAVA::String controlName;
    };

    struct LibrarySection
    {
        bool pinned = false;
        ResDir packagePath;
        ResDir iconPath;
    };

    struct GfxDir
    {
        ResDir directory;
        DAVA::float32 scale = 1.f;
    };

    struct Device
    {
        DAVA::UnorderedMap<DAVA::FastName, DAVA::Any> params;
    };

    struct Blank
    {
        DAVA::FilePath path;
        DAVA::String name;
        DAVA::FastName controlName;
        DAVA::FastName controlPath;
    };

    ProjectData(const DAVA::String& projectDir);
    ~ProjectData() override;

    static const DAVA::int32 CURRENT_PROJECT_FILE_VERSION = 2;

    static const DAVA::String& GetProjectFileName();
    static const DAVA::String& GetFontsConfigFileName();
    static DAVA::Result CreateNewProjectInfrastructure(const QString& projectPath);

    const DAVA::FilePath& GetProjectFile() const;
    const DAVA::FilePath& GetProjectDirectory() const;

    const ResDir& GetPluginsDirectory() const;
    const ResDir& GetResourceDirectory() const;
    const ResDir& GetAdditionalResourceDirectory() const;
    const ResDir& GetConvertedResourceDirectory() const;

    const ResDir& GetUiDirectory() const;
    const ResDir& GetFontsDirectory() const;
    const ResDir& GetFontsConfigsDirectory() const;
    const ResDir& GetTextsDirectory() const;
    const DAVA::Vector<GfxDir>& GetGfxDirectories() const;
    const DAVA::Vector<PinnedControl>& GetPinnedControls() const;
    const DAVA::Vector<LibrarySection>& GetLibrarySections() const;
    const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& GetPrototypes() const;

    const DAVA::String& GetDefaultLanguage() const;

    const DAVA::Vector<Device>& GetDevices() const;
    const DAVA::Vector<Blank>& GetBlanks() const;

    const DAVA::Map<DAVA::String, DAVA::String>& GetSoundLocales() const;

    bool Save() const;

    DAVA::PropertiesItem CreatePropertiesNode(const DAVA::String& nodeName);

    QString RestoreResourcesSymLinkInFilePath(const QString& filePath) const;

    static DAVA::FastName projectPathPropertyName;

private:
    friend class ProjectModule;

    DAVA::ResultList ParseProjectFile(const DAVA::FilePath& projectFile, const DAVA::YamlNode* node);

    DAVA::int32 ParseVersion(const DAVA::YamlNode* root);

    DAVA::ResultList ParseLegacyProperties(const DAVA::FilePath& projectFile, const DAVA::YamlNode* root);
    DAVA::ResultList ParseProjectFileV1(const DAVA::FilePath& projectFile, const DAVA::YamlNode* node);
    DAVA::ResultList ParseProjectFileV2(const DAVA::FilePath& projectFile, const DAVA::YamlNode* node);

    void ParsePluginsDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseResourceDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseAdditionalResourceDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseUiDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseFontsDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseFontsConfigsDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseLibraryV1(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParsePrototypes(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseTextsDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseDevices(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseGfxDirectories(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseConvertedResourceDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);

    void ParseLibraryV2(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseLibraryControlsV2(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);
    void ParseLibrarySectionsV2(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList);

    void RefreshAbsolutePaths();
    DAVA::FilePath MakeAbsolutePath(const DAVA::String& relPath) const;

    DAVA::ResultList LoadProject(const QString& path);
    DAVA::RefPtr<DAVA::YamlNode> SerializeToYamlNode() const;

    void SetProjectFile(const DAVA::FilePath& newProjectFile);
    void SetDefaultLanguage(const DAVA::String& lang);

    DAVA::FilePath projectFile;
    DAVA::FilePath projectDirectory;

    DAVA::String defaultLanguage;

    ResDir pluginsDirectory;
    ResDir resourceDirectory;
    ResDir additionalResourceDirectory;
    ResDir convertedResourceDirectory;

    ResDir uiDirectory;
    ResDir fontsDirectory;
    ResDir fontsConfigsDirectory;
    ResDir textsDirectory;
    DAVA::Vector<GfxDir> gfxDirectories;

    DAVA::Vector<PinnedControl> pinnedControls;
    DAVA::Vector<LibrarySection> librarySections;
    DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>> prototypes;

    DAVA::Vector<Device> devicesForPreview;
    DAVA::Vector<Blank> blanksForPreview;
    DAVA::Map<DAVA::String, DAVA::String> soundLocales;

    std::unique_ptr<DAVA::PropertiesHolder> propertiesHolder;

    DAVA_VIRTUAL_REFLECTION(ProjectData, DAVA::TArcDataNode);
    
#if defined(__DAVAENGINE_MACOS__)
    std::unique_ptr<class MacOSSymLinkRestorer> symLinkRestorer;
#endif
};
