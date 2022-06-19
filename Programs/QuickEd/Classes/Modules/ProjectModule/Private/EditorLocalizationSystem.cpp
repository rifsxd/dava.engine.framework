#include "Modules/ProjectModule/Private/EditorLocalizationSystem.h"
#include "Modules/ProjectModule/ProjectData.h"

#include <TArc/Core/ContextAccessor.h>

#include <FileSystem/LocalizationSystem.h>
#include <Sound/SoundSystem.h>
#include <Engine/Engine.h>

#include <QLocale>
#include <QDirIterator>
#include <QDir>

using namespace DAVA;

EditorLocalizationSystem::EditorLocalizationSystem(DAVA::ContextAccessor* accessor_, QObject* parent)
    : QObject(parent)
    , accessor(accessor_)
{
}

QStringList EditorLocalizationSystem::GetAvailableLocales() const
{
    return availableLocales;
}

void EditorLocalizationSystem::SetDirectory(const QDir& directoryPath)
{
    Cleanup();
    const EngineContext* engineContext = GetEngineContext();
    LocalizationSystem* localizationSystem = engineContext->localizationSystem;
    DVASSERT(nullptr != localizationSystem);

    FilePath directoryFilePath(directoryPath.absolutePath().toStdString() + "/"); //absolutePath doesn't contains with '/' symbol at end
    localizationSystem->SetDirectory(directoryFilePath);
    if (!directoryFilePath.IsEmpty())
    {
        QDirIterator dirIterator(directoryPath, QDirIterator::NoIteratorFlags);
        while (dirIterator.hasNext())
        {
            dirIterator.next();
            QFileInfo fileInfo = dirIterator.fileInfo();
            if (!fileInfo.isDir())
            {
                QString localeStr = fileInfo.baseName();
                availableLocales << localeStr;
            }
        }
    }
}

void EditorLocalizationSystem::Cleanup()
{
    availableLocales.clear();
    const EngineContext* engineContext = GetEngineContext();
    engineContext->localizationSystem->Cleanup();
}

QString EditorLocalizationSystem::GetCurrentLocale() const
{
    return currentLocale;
}

void EditorLocalizationSystem::SetCurrentLocale(const QString& locale)
{
    DVASSERT(!locale.isEmpty());
    DVASSERT(availableLocales.contains(locale));

    currentLocale = locale;
    String localeStr = locale.toStdString();

    const EngineContext* engineContext = GetEngineContext();
    LocalizationSystem* localizationSystem = engineContext->localizationSystem;
    localizationSystem->SetCurrentLocale(localeStr);
    localizationSystem->Init();

    DAVA::DataContext* globalContext = accessor->GetGlobalContext();
    ProjectData* projectData = globalContext->GetData<ProjectData>();
    DVASSERT(projectData != nullptr);
    const DAVA::Map<DAVA::String, DAVA::String>& soundLocales = projectData->GetSoundLocales();
    auto iter = soundLocales.find(locale.toStdString());
    if (iter != soundLocales.end())
    {
        localeStr = iter->second;
    }

    SoundSystem* soundSystem = engineContext->soundSystem;
    soundSystem->SetCurrentLocale(localeStr);
    soundSystem->InitFromQualitySettings();

    emit CurrentLocaleChanged(currentLocale);
}
