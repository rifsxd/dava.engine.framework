#pragma once

#include "Modules/ProjectModule/ProjectData.h"
#include "UI/mainwindow.h"

#include <TArc/DataProcessing/TArcDataNode.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/Core/OperationInvoker.h>

#include <Base/Result.h>

#include <QObject>
#include <QVector>
#include <QPair>

class EditorFontSystem;
class EditorLocalizationSystem;
class MainWindow;
class MacOSSymLinkRestorer;
class FileSystemCache;

namespace DAVA
{
class AssetCacheClient;
class YamlNode;
class ContextAccessor;
class OperationInvoker;
}

DAVA_DEPRECATED(class Project)
{
public:
    static const QStringList& GetFontsFileExtensionFilter();
    static const QString& GetGraphicsFileExtension();
    static const QString& Get3dFileExtension();
    static const QString& GetUiFileExtension();

    Project(MainWindow::ProjectView * view, DAVA::ContextAccessor * accessor);
    ~Project();

    QString GetProjectPath() const;
    const QString& GetProjectDirectory() const;
    const QString& GetProjectName() const;
    QString GetResourceDirectory() const;

    QString RestoreSymLinkInFilePath(const QString& filePath) const;

    QStringList GetAvailableLanguages() const;
    QString GetCurrentLanguage() const;
    void SetCurrentLanguage(const QString& newLanguageCode);

    const QStringList& GetDefaultPresetNames() const;

    EditorFontSystem* GetEditorFontSystem() const;

    void SetRtl(bool isRtl);
    void SetBiDiSupport(bool support);
    void SetGlobalStyleClasses(const QString& classesStr);

    const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& GetPrototypes() const;

private:
    void OnFontPresetChanged();

    QString projectDirectory;
    QString projectName;

    MainWindow::ProjectView* view = nullptr;
    std::unique_ptr<EditorFontSystem> editorFontSystem;
    std::unique_ptr<EditorLocalizationSystem> editorLocalizationSystem;

    DAVA::QtConnections connections;
    DAVA::ContextAccessor* accessor = nullptr;
};
