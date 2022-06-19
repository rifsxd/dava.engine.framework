#include "EditorFontSystem.h"
#include <TArc/Utils/Utils.h>

#include <UI/UIYamlLoader.h>
#include <Engine/Engine.h>

#include <DAVAEngine.h>

using namespace DAVA;

EditorFontSystem::EditorFontSystem(QObject* parent)
    : QObject(parent)
    , defaultFontLocale("default")
    , currentFontLocale(defaultFontLocale)
{
}

EditorFontSystem::~EditorFontSystem()
{
    ClearAllFonts();
}

const FontPreset& EditorFontSystem::GetFont(const String& presetName, const String& locale) const
{
    auto fontsIt = localizedFonts.find(locale);
    if (fontsIt == localizedFonts.end())
    {
        return FontPreset::EMPTY;
    }
    const auto& fonts = fontsIt->second;
    auto it = fonts.find(presetName);
    return it != fonts.end() ? it->second : FontPreset::EMPTY;
}

void EditorFontSystem::SetFont(const String& presetName, const String& locale, const FontPreset& fontPreset)
{
    if (!fontPreset.Valid())
    {
        DVASSERT(false, "wrong argument: fontPreset isn't valid");
        return;
    }
    auto fontsIt = localizedFonts.find(locale);
    if (fontsIt == localizedFonts.end())
    {
        DVASSERT(false, Format("wrong argument: locale = %s passed to this function not found", locale.c_str()).c_str());
        return;
    }
    auto& fonts = fontsIt->second;
    auto it = fonts.find(presetName);

    if (it == fonts.end())
    {
        DVASSERT(false, Format("wrong argument: presetName = %s passed to this function not found for locale %s", presetName.c_str(), locale.c_str()).c_str());
        return;
    }

    Font* oldFont = it->second.GetFontPtr();
    DVASSERT(oldFont);
    it->second = fontPreset;
    if (locale == currentFontLocale)
    {
        const EngineContext* engineContext = GetEngineContext();
        FontManager* fontManager = engineContext->fontManager;
        fontManager->UnregisterFont(oldFont);
        fontManager->RegisterFont(fontPreset.GetFontPtr());
        fontManager->SetFontPreset(fontPreset, presetName);
        emit FontPresetChanged(presetName);
    }
}

void EditorFontSystem::LoadLocalizedFonts()
{
    ClearAllFonts();
    RefreshAvailableFontLocales();
    const EngineContext* engineContext = GetEngineContext();
    FontManager* fontManager = engineContext->fontManager;
    fontManager->UnregisterFontsPresets();
    for (auto& locale : availableFontLocales)
    {
        UIYamlLoader::LoadFonts(GetLocalizedFontsPath(locale.toStdString()));
        for (auto& pair : fontManager->GetFontPresetMap())
        {
            localizedFonts[locale.toStdString()][pair.first] = pair.second;
        }
        fontManager->UnregisterFontsPresets();
    }
    UIYamlLoader::LoadFonts(GetDefaultFontsPath());
    for (auto& pair : fontManager->GetFontPresetMap())
    {
        defaultPresetNames.append(QString::fromStdString(pair.first));
        localizedFonts[defaultFontLocale][pair.first] = pair.second;
    }
    //now check that all font are correct
    for (auto& pair : localizedFonts[defaultFontLocale])
    {
        for (auto& locale : availableFontLocales)
        {
            const auto& localizedMap = localizedFonts.at(locale.toStdString());
            DVASSERT(localizedMap.find(pair.first) != localizedMap.end());
        }
    }
    defaultPresetNames.sort();
    RegisterCurrentLocaleFonts();
}

void EditorFontSystem::SaveLocalizedFonts()
{
    const EngineContext* engineContext = GetEngineContext();
    FontManager* fontManager = engineContext->fontManager;
    FileSystem* fileSystem = engineContext->fileSystem;
    for (auto& localizedFontsIt : localizedFonts)
    {
        fontManager->RegisterFontsPresets(localizedFontsIt.second);
        //load localized fonts into FontManager
        const FilePath& localizedFontsPath = GetLocalizedFontsPath(localizedFontsIt.first);
        if (!fileSystem->IsDirectory(localizedFontsPath.GetDirectory()))
        {
            fileSystem->CreateDirectory(localizedFontsPath.GetDirectory());
        }
        UIYamlLoader::SaveFonts(localizedFontsPath);
    }
    RegisterCurrentLocaleFonts();
}

void EditorFontSystem::ClearAllFonts()
{
    for (auto& map : localizedFonts)
    {
        ClearFonts(map.second);
    }
    localizedFonts.clear();
    defaultPresetNames.clear();
}

void EditorFontSystem::RegisterCurrentLocaleFonts()
{
    const EngineContext* engineContext = GetEngineContext();
    const auto& locale = engineContext->localizationSystem->GetCurrentLocale();
    currentFontLocale = availableFontLocales.contains(QString::fromStdString(locale)) ? locale : defaultFontLocale;
    auto it = localizedFonts.find(currentFontLocale);
    const auto& fonts = it != localizedFonts.end() ? it->second : localizedFonts.at(defaultFontLocale);
    engineContext->fontManager->RegisterFontsPresets(fonts);
    emit FontPresetChanged(String());
}

void EditorFontSystem::ClearFonts(UnorderedMap<String, FontPreset>& fonts)
{
    fonts.clear();
}

void EditorFontSystem::RemoveFont(UnorderedMap<String, FontPreset>* fonts, const String& fontName)
{
    if (fonts->find(fontName) != fonts->end())
    {
        auto findOriginalIt = fonts->find(fontName);
        fonts->erase(findOriginalIt);
    }
}

void EditorFontSystem::RefreshAvailableFontLocales()
{
    availableFontLocales.clear();
    if (!defaultFontsPath.IsDirectoryPathname())
    {
        return;
    }

    FileList* fileList = new FileList(defaultFontsPath);
    for (uint32 count = fileList->GetCount(), i = 0u; i < count; ++i)
    {
        if (fileList->IsDirectory(i) && !fileList->IsNavigationDirectory(i))
        {
            availableFontLocales.push_back(QString::fromStdString(fileList->GetFilename(i)));
        }
    }
    SafeRelease(fileList);
}

void EditorFontSystem::SetDefaultFontsPath(const FilePath& path)
{
    defaultFontsPath = path.GetType() == FilePath::PATH_IN_RESOURCES ? path.GetAbsolutePathname() : path;
    RefreshAvailableFontLocales();
}

FilePath EditorFontSystem::GetDefaultFontsPath() const
{
    return defaultFontsPath + "fonts.yaml";
}

FilePath EditorFontSystem::GetLocalizedFontsPath(const String& locale) const
{
    return locale == defaultFontLocale ? GetDefaultFontsPath() : (defaultFontsPath + locale + "/fonts.yaml");
}

void EditorFontSystem::CreateNewPreset(const String& originalPresetName, const String& newPresetName)
{
    DVASSERT(localizedFonts[defaultFontLocale].size() > 0);
    const auto& presetName = defaultPresetNames.contains(QString::fromStdString(originalPresetName)) ? originalPresetName : localizedFonts[defaultFontLocale].begin()->first;
    for (auto& localizedFontsPairs : localizedFonts)
    {
        auto& fonts = localizedFontsPairs.second;
        fonts[newPresetName] = FontPreset(fonts.at(presetName));
    }
    defaultPresetNames.append(QString::fromStdString(newPresetName));
    defaultPresetNames.sort();
}
