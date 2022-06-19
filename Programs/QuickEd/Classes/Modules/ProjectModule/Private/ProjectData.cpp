#include "Modules/ProjectModule/ProjectData.h"

#include "Utils/MacOSSymLinkRestorer.h"

#include <Engine/Engine.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/YamlNode.h>
#include <Utils/Utils.h>
#include <Utils/StringFormat.h>
#include <Utils/MD5.h>
#include <FileSystem/YamlNode.h>
#include <FileSystem/YamlEmitter.h>
#include <FileSystem/YamlParser.h>
#include <FileSystem/FileSystem.h>

#include <QObject>
#include <QFileInfo>
#include <QDir>

DAVA_VIRTUAL_REFLECTION_IMPL(ProjectData)
{
    DAVA::ReflectionRegistrator<ProjectData>::Begin()
    .Field(projectPathPropertyName.c_str(), &ProjectData::GetProjectFile, nullptr)
    .End();
}

using namespace DAVA;

DAVA::FastName ProjectData::projectPathPropertyName{ "ProjectPath" };

namespace ProjectDataDetails
{
void LoadDevices(const YamlNode* devicesNode, Vector<ProjectData::Device>& devicesForPreview, ResultList& resultList)
{
    for (uint32 i = 0; i < devicesNode->GetCount(); i++)
    {
        const YamlNode* node = devicesNode->Get(i);
        DVASSERT(node != nullptr);

        ProjectData::Device device;

        const YamlNode* nameNode = node->Get("name");
        if (nameNode != nullptr)
        {
            device.params[FastName("name")] = nameNode->AsString();
        }
        else
        {
            resultList.AddResult(Result::RESULT_ERROR, Format("Cannot read name for device #%d.", i));
            continue;
        }

        const YamlNode* screenSizeNode = node->Get("screenSize");
        if (screenSizeNode != nullptr)
        {
            Size2i screenSize;
            screenSize.dx = screenSizeNode->Get(0)->AsInt32();
            screenSize.dy = screenSizeNode->Get(1)->AsInt32();

            device.params[FastName("screenSize")] = screenSize;
        }

        const YamlNode* virtualSizeNode = node->Get("virtualScreenSize");
        if (virtualSizeNode != nullptr)
        {
            Size2i virtualScreenSize;
            virtualScreenSize.dx = virtualSizeNode->Get(0)->AsInt32();
            virtualScreenSize.dy = virtualSizeNode->Get(1)->AsInt32();

            device.params[FastName("virtualScreenSize")] = virtualScreenSize;
        }

        const YamlNode* virtualKeyboardHeightNode = node->Get("virtualKeyboardHeight");
        if (virtualKeyboardHeightNode != nullptr)
        {
            device.params[FastName("virtualKeyboardHeight")] = virtualKeyboardHeightNode->AsInt32();
        }

        const YamlNode* langNode = node->Get("lang");
        if (langNode != nullptr)
        {
            device.params[FastName("lang")] = langNode->AsString();
        }

        const YamlNode* rtlNode = node->Get("rtl");
        if (rtlNode != nullptr)
        {
            device.params[FastName("isRtl")] = rtlNode->AsBool();
        }

        devicesForPreview.push_back(device);
    }
}

void LoadBlanks(const DAVA::YamlNode* blanksNode, DAVA::Vector<ProjectData::Blank>& blanksForPreview, ResultList& resultList)
{
    for (uint32 i = 0; i < blanksNode->GetCount(); i++)
    {
        const YamlNode* node = blanksNode->Get(i);
        DVASSERT(node != nullptr);

        ProjectData::Blank blank;

        const YamlNode* nameNode = node->Get("name");
        if (nameNode != nullptr)
        {
            blank.name = nameNode->AsString();
        }
        else
        {
            resultList.AddResult(Result::RESULT_ERROR, Format("Cannot read name for blank #%d.", i));
            continue;
        }

        const YamlNode* pathNode = node->Get("pathToYaml");
        if (pathNode != nullptr)
        {
            blank.path = pathNode->AsString();
        }
        else
        {
            resultList.AddResult(Result::RESULT_ERROR, Format("Cannot read pathToYaml for blank #%d.", i));
            continue;
        }

        const YamlNode* controlNameNode = node->Get("control");
        if (controlNameNode != nullptr)
        {
            blank.controlName = controlNameNode->AsFastName();
        }
        else
        {
            resultList.AddResult(Result::RESULT_ERROR, Format("Cannot read control for blank #%d.", i));
            continue;
        }

        const YamlNode* controlPathNode = node->Get("path");
        if (pathNode != nullptr)
        {
            blank.controlPath = controlPathNode->AsFastName();
        }
        else
        {
            resultList.AddResult(Result::RESULT_ERROR, Format("Cannot read path for blank #%d.", i));
            continue;
        }

        blanksForPreview.push_back(blank);
    }
}
}

DAVA::ResultList ProjectData::ParseLegacyProperties(const DAVA::FilePath& projectFile, const YamlNode* root)
{
    ResultList resultList;

    additionalResourceDirectory.relative = String("./Data/");

    if (root == nullptr) // for support old project
    {
        SetDefaultLanguage("");
    }
    else
    {
        const YamlNode* fontNode = root->Get("font");
        // Get font node
        if (nullptr != fontNode)
        {
            // Get default font node
            const YamlNode* defaultFontPath = fontNode->Get("DefaultFontsPath");
            if (nullptr != defaultFontPath)
            {
                String fontsConfigsPath = FilePath(defaultFontPath->AsString()).GetDirectory().GetRelativePathname("~res:/");
                fontsConfigsDirectory.relative = fontsConfigsPath;
            }
        }

        const YamlNode* localizationPathNode = root->Get("LocalizationPath");
        const YamlNode* localeNode = root->Get("Locale");
        if (localizationPathNode != nullptr && localeNode != nullptr)
        {
            String localePath = FilePath(localizationPathNode->AsString()).GetRelativePathname("~res:/");
            textsDirectory.relative = localePath;
            defaultLanguage = localeNode->AsString();
        }

        const YamlNode* libraryNode = root->Get("Library");
        if (libraryNode != nullptr)
        {
            for (uint32 i = 0; i < libraryNode->GetCount(); i++)
            {
                LibrarySection section;
                section.packagePath.relative = FilePath(libraryNode->Get(i)->AsString()).GetRelativePathname("~res:/");
                librarySections.push_back(std::move(section));
            }
        }
    }

    SetProjectFile(projectFile);

    return resultList;
}

void ProjectData::RefreshAbsolutePaths()
{
    DVASSERT(!projectFile.IsEmpty());
    projectDirectory = projectFile.GetDirectory();

    resourceDirectory.absolute = projectDirectory + resourceDirectory.relative;

    convertedResourceDirectory.absolute = projectDirectory + convertedResourceDirectory.relative;
    pluginsDirectory.absolute = projectDirectory + pluginsDirectory.relative;

    if (additionalResourceDirectory.relative.empty())
    {
        additionalResourceDirectory.absolute = FilePath();
    }
    else
    {
        additionalResourceDirectory.absolute = projectDirectory + additionalResourceDirectory.relative;
    }

    uiDirectory.absolute = MakeAbsolutePath(uiDirectory.relative);
    fontsDirectory.absolute = MakeAbsolutePath(fontsDirectory.relative);
    fontsConfigsDirectory.absolute = MakeAbsolutePath(fontsConfigsDirectory.relative);
    textsDirectory.absolute = MakeAbsolutePath(textsDirectory.relative);

    for (auto& gfxDir : gfxDirectories)
    {
        gfxDir.directory.absolute = MakeAbsolutePath(gfxDir.directory.relative);
    }

    for (PinnedControl& pinnedControl : pinnedControls)
    {
        pinnedControl.packagePath.absolute = MakeAbsolutePath(pinnedControl.packagePath.relative);
        pinnedControl.iconPath.absolute = MakeAbsolutePath(pinnedControl.iconPath.relative);
    }

    for (LibrarySection& section : librarySections)
    {
        section.packagePath.absolute = MakeAbsolutePath(section.packagePath.relative);
        section.iconPath.absolute = MakeAbsolutePath(section.iconPath.relative);
    }
    
#if defined(__DAVAENGINE_MACOS__)
    QString directoryPath = QString::fromStdString(resourceDirectory.absolute.GetStringValue());
    symLinkRestorer = std::make_unique<MacOSSymLinkRestorer>(directoryPath);
#endif
}

const ProjectData::ResDir& ProjectData::GetResourceDirectory() const
{
    return resourceDirectory;
}

const ProjectData::ResDir& ProjectData::GetAdditionalResourceDirectory() const
{
    return additionalResourceDirectory;
}

const ProjectData::ResDir& ProjectData::GetConvertedResourceDirectory() const
{
    return convertedResourceDirectory;
}

const ProjectData::ResDir& ProjectData::GetUiDirectory() const
{
    return uiDirectory;
}

const ProjectData::ResDir& ProjectData::GetFontsDirectory() const
{
    return fontsDirectory;
}

const ProjectData::ResDir& ProjectData::GetFontsConfigsDirectory() const
{
    return fontsConfigsDirectory;
}

const ProjectData::ResDir& ProjectData::GetTextsDirectory() const
{
    return textsDirectory;
}

const Vector<ProjectData::GfxDir>& ProjectData::GetGfxDirectories() const
{
    return gfxDirectories;
}

const DAVA::Vector<ProjectData::PinnedControl>& ProjectData::GetPinnedControls() const
{
    return pinnedControls;
}

const Vector<ProjectData::LibrarySection>& ProjectData::GetLibrarySections() const
{
    return librarySections;
}

const Map<String, DAVA::Set<FastName>>& ProjectData::GetPrototypes() const
{
    return prototypes;
}

const DAVA::String& ProjectData::GetDefaultLanguage() const
{
    return defaultLanguage;
}

bool ProjectData::Save() const
{
    using namespace DAVA;

    RefPtr<YamlNode> node = SerializeToYamlNode();
    return YamlEmitter::SaveToYamlFile(GetProjectFile(), node.Get());
}

DAVA::PropertiesItem ProjectData::CreatePropertiesNode(const DAVA::String& nodeName)
{
    DVASSERT(propertiesHolder != nullptr);
    return propertiesHolder->CreateSubHolder(nodeName);
}

QString ProjectData::RestoreResourcesSymLinkInFilePath(const QString& filePath) const
{
#if defined(__DAVAENGINE_MACOS__)
    return symLinkRestorer->RestoreSymLinkInFilePath(filePath);
#else
    return filePath;
#endif
}

void ProjectData::SetDefaultLanguage(const DAVA::String& lang)
{
    defaultLanguage = lang;
}

DAVA::Result ProjectData::CreateNewProjectInfrastructure(const QString& projectFilePath)
{
    using namespace DAVA;
    QDir projectDir(QFileInfo(projectFilePath).absolutePath());

    std::unique_ptr<ProjectData> defaultSettings(new ProjectData(projectFilePath.toStdString()));

    QString pluginsDirectory = QString::fromStdString(defaultSettings->GetPluginsDirectory().relative);
    if (pluginsDirectory.isEmpty() == false && projectDir.mkpath(pluginsDirectory) == false)
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create plugins directory %1").arg(pluginsDirectory).toStdString());
    }

    QString resourceDirectory = QString::fromStdString(defaultSettings->GetResourceDirectory().relative);
    if (!projectDir.mkpath(resourceDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create resource directory %1.").arg(resourceDirectory).toStdString());
    }

    QString intermediateResourceDirectory = QString::fromStdString(defaultSettings->GetConvertedResourceDirectory().relative);
    if (intermediateResourceDirectory != resourceDirectory && !projectDir.mkpath(intermediateResourceDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create intermediate resource directory %1.").arg(intermediateResourceDirectory).toStdString());
    }

    QDir resourceDir(projectDir.absolutePath() + "/" + resourceDirectory);

    QString gfxDirectory = QString::fromStdString(defaultSettings->GetGfxDirectories().front().directory.relative);
    if (!resourceDir.mkpath(gfxDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create gfx directory %1.").arg(gfxDirectory).toStdString());
    }

    QString uiDirectory = QString::fromStdString(defaultSettings->GetUiDirectory().relative);
    if (!resourceDir.mkpath(uiDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create UI directory %1.").arg(uiDirectory).toStdString());
    }

    QString textsDirectory = QString::fromStdString(defaultSettings->GetTextsDirectory().relative);
    if (!resourceDir.mkpath(textsDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create UI directory %1.").arg(textsDirectory).toStdString());
    }

    QString fontsDirectory = QString::fromStdString(defaultSettings->GetFontsDirectory().relative);
    if (!resourceDir.mkpath(fontsDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create fonts directory %1.").arg(fontsDirectory).toStdString());
    }

    QString fontsConfigDirectory = QString::fromStdString(defaultSettings->GetFontsConfigsDirectory().relative);
    if (!resourceDir.mkpath(fontsConfigDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create fonts config directory %1.").arg(fontsConfigDirectory).toStdString());
    }

    QDir textsDir(resourceDir.absolutePath() + "/" + textsDirectory);
    QFile defaultLanguageTextFile(textsDir.absoluteFilePath(QString::fromStdString(defaultSettings->GetDefaultLanguage()) + ".yaml"));
    if (!defaultLanguageTextFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create localization file %1.").arg(defaultLanguageTextFile.fileName()).toStdString());
    }
    defaultLanguageTextFile.close();

    QDir fontsConfigDir(resourceDir.absolutePath() + "/" + fontsConfigDirectory);
    QFile defaultFontsConfigFile(fontsConfigDir.absoluteFilePath(QString::fromStdString(ProjectData::GetFontsConfigFileName())));
    if (!defaultFontsConfigFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create fonts config file %1.").arg(defaultFontsConfigFile.fileName()).toStdString());
    }
    defaultFontsConfigFile.close();

    defaultSettings->SetProjectFile(projectFilePath.toStdString());
    if (!defaultSettings->Save())
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create project file %1.").arg(projectFilePath).toStdString());
    }

    return Result();
}

DAVA::FilePath ProjectData::MakeAbsolutePath(const DAVA::String& relPath) const
{
    if (relPath.empty())
        return FilePath();

    const DAVA::EngineContext* engineContext = GetEngineContext();
    DAVA::FileSystem* fileSystem = engineContext->fileSystem;

    FilePath pathInResDir = resourceDirectory.absolute + relPath;
    if (fileSystem->Exists(pathInResDir))
    {
        return pathInResDir;
    }

    FilePath pathInAddResDir = additionalResourceDirectory.absolute + relPath;
    if (fileSystem->Exists(pathInAddResDir))
    {
        return pathInAddResDir;
    }

    return FilePath();
}

DAVA::ResultList ProjectData::ParseProjectFile(const DAVA::FilePath& projectFile, const YamlNode* root)
{
    int32 version = ParseVersion(root);

    switch (version)
    {
    case 0:
        return ParseLegacyProperties(projectFile, root);
    case 1:
        return ParseProjectFileV1(projectFile, root);
    case 2:
        return ParseProjectFileV2(projectFile, root);
    default:
    {
        String msg = Format("Project file version %u is not supported. Maximal supported version is %u", version, ProjectData::CURRENT_PROJECT_FILE_VERSION);
        return ResultList(Result(Result::RESULT_ERROR, msg));
    }
    }
}

DAVA::int32 ProjectData::ParseVersion(const DAVA::YamlNode* root)
{
    DAVA::int32 version = 0;
    if (root != nullptr)
    {
        const YamlNode* headerNode = root->Get("Header");
        if (headerNode != nullptr)
        {
            const YamlNode* versionNode = headerNode->Get("version");
            if (versionNode != nullptr && versionNode->AsInt32())
            {
                version = versionNode->AsInt32();
            }
        }
    }

    return version;
}

DAVA::ResultList ProjectData::ParseProjectFileV1(const DAVA::FilePath& projectFile, const DAVA::YamlNode* root)
{
    ResultList resultList;

    const YamlNode* projectDataNode = root->Get("ProjectProperties");
    if (projectDataNode == nullptr)
    {
        String message = Format("Wrong project properties in file %s.", projectFile.GetAbsolutePathname().c_str());
        resultList.AddResult(Result::RESULT_ERROR, message);

        return resultList;
    }

    ParsePluginsDirectory(projectDataNode, resultList);
    ParseResourceDirectory(projectDataNode, resultList);
    ParseAdditionalResourceDirectory(projectDataNode, resultList);
    ParseConvertedResourceDirectory(projectDataNode, resultList);
    ParseGfxDirectories(projectDataNode, resultList);
    ParseUiDirectory(projectDataNode, resultList);
    ParseFontsDirectory(projectDataNode, resultList);
    ParseFontsConfigsDirectory(projectDataNode, resultList);
    ParseTextsDirectory(projectDataNode, resultList);
    ParseLibraryV1(projectDataNode, resultList);
    ParsePrototypes(projectDataNode, resultList);
    ParseDevices(projectDataNode, resultList);

    SetProjectFile(projectFile);

    return resultList;
}

DAVA::ResultList ProjectData::ParseProjectFileV2(const DAVA::FilePath& projectFile, const DAVA::YamlNode* root)
{
    ResultList resultList;

    const YamlNode* projectDataNode = root->Get("ProjectProperties");
    if (projectDataNode == nullptr)
    {
        String message = Format("Wrong project properties in file %s.", projectFile.GetAbsolutePathname().c_str());
        resultList.AddResult(Result::RESULT_ERROR, message);

        return resultList;
    }

    ParsePluginsDirectory(projectDataNode, resultList);
    ParseResourceDirectory(projectDataNode, resultList);
    ParseAdditionalResourceDirectory(projectDataNode, resultList);
    ParseConvertedResourceDirectory(projectDataNode, resultList);
    ParseGfxDirectories(projectDataNode, resultList);
    ParseUiDirectory(projectDataNode, resultList);
    ParseFontsDirectory(projectDataNode, resultList);
    ParseFontsConfigsDirectory(projectDataNode, resultList);
    ParseTextsDirectory(projectDataNode, resultList);
    ParseLibraryV2(projectDataNode, resultList);
    ParsePrototypes(projectDataNode, resultList);
    ParseDevices(projectDataNode, resultList);

    SetProjectFile(projectFile);

    return resultList;
}

void ProjectData::ParsePluginsDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* pluginsDirNode = projectDataNode->Get("PluginsDirectory");
    if (pluginsDirNode != nullptr)
    {
        pluginsDirectory.relative = pluginsDirNode->AsString();
    }
}

void ProjectData::ParseResourceDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* resourceDirNode = projectDataNode->Get("ResourceDirectory");
    if (resourceDirNode != nullptr)
    {
        resourceDirectory.relative = resourceDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directory not set. Used default directory: %s.", resourceDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }
}

void ProjectData::ParseAdditionalResourceDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* additionalResourceDirNode = projectDataNode->Get("AdditionalResourceDirectory");
    if (additionalResourceDirNode != nullptr)
    {
        additionalResourceDirectory.relative = additionalResourceDirNode->AsString();
    }
}

void ProjectData::ParseGfxDirectories(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* gfxDirsNode = projectDataNode->Get("GfxDirectories");
    if (gfxDirsNode != nullptr)
    {
        if (gfxDirsNode->GetCount() > 0)
        {
            gfxDirectories.clear();
        }

        for (uint32 index = 0; index < gfxDirsNode->GetCount(); ++index)
        {
            const YamlNode* gfxDirNode = gfxDirsNode->Get(index);
            DVASSERT(gfxDirNode);

            GfxDir gfxDir;
            gfxDir.directory.relative = gfxDirNode->Get("directory")->AsString();
            gfxDir.scale = (gfxDirNode->Get("scale") != nullptr) ? gfxDirNode->Get("scale")->AsFloat() : 1.0f;
            gfxDirectories.push_back(gfxDir);
        }
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s, with scale %f.",
                                gfxDirectories.front().directory.relative.c_str(),
                                gfxDirectories.front().scale);
        resultList.AddResult(Result::RESULT_WARNING, message);
    }
}

void ProjectData::ParseConvertedResourceDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* convertedResourceDirNode = projectDataNode->Get("ConvertedResourceDirectory");
    if (convertedResourceDirNode != nullptr)
    {
        convertedResourceDirectory.relative = convertedResourceDirNode->AsString();
    }
    else
    {
        String message = Format("Directory for converted sources not set. Used default directory: %s.", convertedResourceDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }
}

void ProjectData::ParseUiDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* uiDirNode = projectDataNode->Get("UiDirectory");
    if (uiDirNode != nullptr)
    {
        uiDirectory.relative = uiDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", uiDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }
}

void ProjectData::ParseFontsDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* fontsDirNode = projectDataNode->Get("FontsDirectory");
    if (fontsDirNode != nullptr)
    {
        fontsDirectory.relative = fontsDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", fontsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }
}

void ProjectData::ParseFontsConfigsDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* fontsConfigsDirNode = projectDataNode->Get("FontsConfigsDirectory");
    if (fontsConfigsDirNode != nullptr)
    {
        fontsConfigsDirectory.relative = fontsConfigsDirNode->AsString();
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", fontsConfigsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }
}

void ProjectData::ParseLibraryV1(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* libraryNode = projectDataNode->Get("Library");
    if (libraryNode != nullptr)
    {
        for (uint32 i = 0; i < libraryNode->GetCount(); i++)
        {
            LibrarySection section;
            section.packagePath.relative = libraryNode->Get(i)->AsString();
            librarySections.emplace_back(std::move(section));
        }
    }
}

void ProjectData::ParseLibraryV2(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* libraryNode = projectDataNode->Get("Library");
    if (libraryNode != nullptr)
    {
        ParseLibraryControlsV2(libraryNode, resultList);
        ParseLibrarySectionsV2(libraryNode, resultList);
    }
}

void ProjectData::ParseLibraryControlsV2(const DAVA::YamlNode* libraryNode, DAVA::ResultList& resultList)
{
    const YamlNode* controlsNode = libraryNode->Get("Controls");
    if (controlsNode != nullptr)
    {
        for (uint32 i = 0; i < controlsNode->GetCount(); i++)
        {
            const YamlNode* controlNode = controlsNode->Get(i);
            PinnedControl pinnedControl;

            {
                const YamlNode* node = controlNode->Get("package");
                if (node != nullptr)
                {
                    pinnedControl.packagePath.relative = node->AsString();
                }
                else
                {
                    resultList.AddResult(Result::RESULT_WARNING, "'package' node was not found in Controls list");
                    continue;
                }
            }

            {
                const YamlNode* node = controlNode->Get("control");
                if (node != nullptr)
                {
                    pinnedControl.controlName = node->AsString();
                }
                else
                {
                    resultList.AddResult(Result::RESULT_WARNING, "'control' node was not found in Controls entry");
                    continue;
                }
            }

            {
                const YamlNode* node = controlNode->Get("icon");
                if (node != nullptr)
                {
                    pinnedControl.iconPath.relative = node->AsString();
                }
            }

            pinnedControls.emplace_back(std::move(pinnedControl));
        }
    }
}

void ProjectData::ParseLibrarySectionsV2(const DAVA::YamlNode* libraryNode, DAVA::ResultList& resultList)
{
    const YamlNode* sectionsNode = libraryNode->Get("Sections");
    if (sectionsNode != nullptr)
    {
        for (uint32 i = 0; i < sectionsNode->GetCount(); i++)
        {
            const YamlNode* sectionNode = sectionsNode->Get(i);
            LibrarySection section;

            {
                const YamlNode* node = sectionNode->Get("package");
                if (node != nullptr)
                {
                    section.packagePath.relative = node->AsString();
                }
                else
                {
                    resultList.AddResult(Result::RESULT_WARNING, "'package' node was not found in Section entry");
                    continue;
                }
            }

            {
                const YamlNode* node = sectionNode->Get("icon");
                if (node != nullptr)
                {
                    section.iconPath.relative = node->AsString();
                }
            }

            {
                const YamlNode* node = sectionNode->Get("pinned");
                if (node != nullptr)
                {
                    section.pinned = node->AsBool();
                }
            }

            librarySections.emplace_back(std::move(section));
        }
    }
}

void ProjectData::ParsePrototypes(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* prototypesNode = projectDataNode->Get("Prototypes");
    if (prototypesNode != nullptr)
    {
        for (uint32 i = 0; i < prototypesNode->GetCount(); i++)
        {
            Set<FastName> packagePrototypes;
            const YamlNode* packNode = prototypesNode->Get(i);
            const YamlNode* packagePrototypesNode = packNode->Get("prototypes");

            for (uint32 j = 0; j < packagePrototypesNode->GetCount(); j++)
            {
                packagePrototypes.insert(packagePrototypesNode->Get(j)->AsFastName());
            }

            const String& packagePath = packNode->Get("file")->AsString();
            prototypes[packagePath] = packagePrototypes;
        }
    }
}

void ProjectData::ParseTextsDirectory(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* textsDirNode = projectDataNode->Get("TextsDirectory");
    if (textsDirNode != nullptr)
    {
        textsDirectory.relative = textsDirNode->AsString();
        const YamlNode* defaultLanguageNode = projectDataNode->Get("DefaultLanguage");
        if (defaultLanguageNode != nullptr)
        {
            defaultLanguage = defaultLanguageNode->AsString();
        }
    }
    else
    {
        String message = Format("Data source directories not set. Used default directory: %s.", textsDirectory.relative.c_str());
        resultList.AddResult(Result::RESULT_WARNING, message);
    }
}

void ProjectData::ParseDevices(const DAVA::YamlNode* projectDataNode, DAVA::ResultList& resultList)
{
    const YamlNode* devicesNode = projectDataNode->Get("Devices");
    if (devicesNode != nullptr)
    {
        ProjectDataDetails::LoadDevices(devicesNode, devicesForPreview, resultList);
    }

    const YamlNode* blanksNode = projectDataNode->Get("Blanks");
    if (blanksNode != nullptr)
    {
        ProjectDataDetails::LoadBlanks(blanksNode, blanksForPreview, resultList);
    }

    const YamlNode* localesNode = projectDataNode->Get("Locales");
    if (localesNode != nullptr)
    {
        for (uint32 i = 0; i < localesNode->GetCount(); i++)
        {
            String locale = localesNode->GetItemKeyName(i);
            const YamlNode* localeNode = localesNode->Get(i);
            for (uint32 j = 0; j < localeNode->GetCount(); j++)
            {
                String type = localeNode->GetItemKeyName(j);
                if (type == "sound")
                {
                    String value = localeNode->Get(j)->AsString();
                    soundLocales[locale] = value;
                }
            }
        }
    }
}

DAVA::ResultList ProjectData::LoadProject(const QString& path)
{
    using namespace DAVA;
    ResultList resultList;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        QString message = QObject::tr("%1 does not exist.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return resultList;
    }

    if (!fileInfo.isFile())
    {
        QString message = QObject::tr("%1 is not a file.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return resultList;
    }

    RefPtr<YamlParser> parser(YamlParser::Create(path.toStdString()));
    if (parser.Get() == nullptr)
    {
        QString message = QObject::tr("Can not parse project file %1.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());

        return resultList;
    }
    return ParseProjectFile(path.toStdString(), parser->GetRootNode());
}

RefPtr<YamlNode> ProjectData::SerializeToYamlNode() const
{
    RefPtr<YamlNode> root(YamlNode::CreateMapNode(false));

    RefPtr<YamlNode> headerNode(YamlNode::CreateMapNode(false));
    headerNode->Add("version", CURRENT_PROJECT_FILE_VERSION);
    root->Add("Header", headerNode);

    RefPtr<YamlNode> propertiesNode(YamlNode::CreateMapNode(false));
    propertiesNode->Add("ResourceDirectory", resourceDirectory.relative);
    propertiesNode->Add("PluginsDirectory", pluginsDirectory.relative);

    if (!additionalResourceDirectory.relative.empty())
    {
        propertiesNode->Add("AdditionalResourceDirectory", additionalResourceDirectory.relative);
    }

    propertiesNode->Add("IntermediateResourceDirectory", convertedResourceDirectory.relative);

    propertiesNode->Add("UiDirectory", uiDirectory.relative);
    propertiesNode->Add("FontsDirectory", fontsDirectory.relative);
    propertiesNode->Add("FontsConfigsDirectory", fontsConfigsDirectory.relative);
    propertiesNode->Add("TextsDirectory", textsDirectory.relative);
    propertiesNode->Add("DefaultLanguage", defaultLanguage);

    RefPtr<YamlNode> gfxDirsNode(YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION));
    for (const auto& gfxDir : gfxDirectories)
    {
        RefPtr<YamlNode> gfxDirNode(YamlNode::CreateMapNode(false));
        gfxDirNode->Add("directory", gfxDir.directory.relative);
        gfxDirNode->Add("scale", gfxDir.scale);
        gfxDirsNode->Add(gfxDirNode);
    }
    propertiesNode->Add("GfxDirectories", gfxDirsNode);

    RefPtr<YamlNode> libraryNode(YamlNode::CreateMapNode(false));

    RefPtr<YamlNode> controlsNode(YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION));
    for (const PinnedControl& pinnedControl : pinnedControls)
    {
        RefPtr<YamlNode> controlNode(YamlNode::CreateMapNode(false));
        controlNode->Add("package", pinnedControl.packagePath.relative);
        controlNode->Add("control", pinnedControl.controlName);
        if (!pinnedControl.iconPath.relative.empty())
        {
            controlNode->Add("icon", pinnedControl.iconPath.relative);
        }
        controlsNode->Add(controlNode);
    }
    libraryNode->Add("Controls", controlsNode);

    RefPtr<YamlNode> sectionsNode(YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION));
    for (const LibrarySection& pinnedControl : librarySections)
    {
        RefPtr<YamlNode> sectionNode(YamlNode::CreateMapNode(false));
        sectionNode->Add("package", pinnedControl.packagePath.relative);
        if (!pinnedControl.iconPath.relative.empty())
        {
            sectionNode->Add("icon", pinnedControl.iconPath.relative);
        }
        if (pinnedControl.pinned)
        {
            sectionNode->Add("pinned", "true");
        }
        sectionsNode->Add(sectionNode);
    }
    libraryNode->Add("Sections", sectionsNode);

    propertiesNode->Add("Library", libraryNode);
    root->Add("ProjectProperties", propertiesNode);

    return root;
}

ProjectData::ProjectData(const DAVA::String& projectDir)
{
    using namespace DAVA;

    pluginsDirectory.relative = "./QuickEdPlugins/";
    resourceDirectory.relative = "./DataSource/";
    convertedResourceDirectory.relative = "./Data/";

    GfxDir gfxDir;
    gfxDir.directory.relative = "./Gfx/";
    gfxDir.scale = 1.0f;
    gfxDirectories.push_back(gfxDir);

    uiDirectory.relative = "./UI/";
    fontsDirectory.relative = "./Fonts/";
    fontsConfigsDirectory.relative = "./Fonts/Configs/";
    textsDirectory.relative = "./Strings/";
    defaultLanguage = "en";

    MD5::MD5Digest digest;
    MD5::ForData(reinterpret_cast<const uint8*>(projectDir.c_str()), static_cast<uint32>(projectDir.size()), digest);
    String fileName = "project_" + MD5::HashToString(digest);

    FileSystem* fileSystem = GetEngineContext()->fileSystem;
    propertiesHolder.reset(new PropertiesHolder(fileName, fileSystem->GetCurrentDocumentsDirectory()));
}

ProjectData::~ProjectData()
{
    FilePath::RemoveResourcesFolder(resourceDirectory.absolute);
    FilePath::RemoveResourcesFolder(additionalResourceDirectory.absolute);
    FilePath::RemoveResourcesFolder(convertedResourceDirectory.absolute);
}

const DAVA::String& ProjectData::GetProjectFileName()
{
    static const String projectFile("ui.quicked");
    return projectFile;
}

const DAVA::String& ProjectData::GetFontsConfigFileName()
{
    static const String configFile("fonts.yaml");
    return configFile;
}

const DAVA::FilePath& ProjectData::GetProjectFile() const
{
    return projectFile;
}

void ProjectData::SetProjectFile(const DAVA::FilePath& newProjectFile)
{
    projectFile = newProjectFile;
    RefreshAbsolutePaths();
}

const DAVA::FilePath& ProjectData::GetProjectDirectory() const
{
    return projectDirectory;
}

const ProjectData::ResDir& ProjectData::GetPluginsDirectory() const
{
    return pluginsDirectory;
}

bool ProjectData::ResDir::operator==(const ProjectData::ResDir& other) const
{
    if (this == &other)
    {
        return true;
    }
    return other.absolute == absolute &&
    other.relative == relative;
}

const DAVA::Vector<ProjectData::Device>& ProjectData::GetDevices() const
{
    return devicesForPreview;
}

const DAVA::Vector<ProjectData::Blank>& ProjectData::GetBlanks() const
{
    return blanksForPreview;
}

const DAVA::Map<DAVA::String, DAVA::String>& ProjectData::GetSoundLocales() const
{
    return soundLocales;
}
