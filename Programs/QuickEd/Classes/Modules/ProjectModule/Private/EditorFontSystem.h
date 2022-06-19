#ifndef __EDITOR_FONT_SYSTEM__
#define __EDITOR_FONT_SYSTEM__

#include "Base/BaseTypes.h"
#include "Render/2D/Font.h"
#include "Render/2D/FontPreset.h"
#include "FileSystem/FilePath.h"
#include <QObject>
#include <QStringList>

class EditorFontSystem : public QObject
{
    Q_OBJECT
public:
    EditorFontSystem(QObject* parent = nullptr);
    ~EditorFontSystem();
    const char* GetDefaultFontLocale() const;
    const DAVA::FontPreset& GetFont(const DAVA::String& presetName, const DAVA::String& locale) const;
    void SetFont(const DAVA::String& presetName, const DAVA::String& locale, const DAVA::FontPreset& fontPreset);
    const QStringList& GetAvailableFontLocales() const;
    const QStringList& GetDefaultPresetNames() const;
    void LoadLocalizedFonts();
    void SaveLocalizedFonts();

    void ClearAllFonts();
    void CreateNewPreset(const DAVA::String& originalPresetName, const DAVA::String& newPresetName);

    void SetDefaultFontsPath(const DAVA::FilePath& path);
    DAVA::FilePath GetLocalizedFontsPath(const DAVA::String& locale) const;
    DAVA::FilePath GetDefaultFontsPath() const;
signals:
    void FontPresetChanged(const DAVA::String& presetName);
public slots:
    void RegisterCurrentLocaleFonts();

private:
    void ClearFonts(DAVA::UnorderedMap<DAVA::String, DAVA::FontPreset>& fonts);
    void RemoveFont(DAVA::UnorderedMap<DAVA::String, DAVA::FontPreset>* fonts, const DAVA::String& fontName);
    void RefreshAvailableFontLocales();
    DAVA::FilePath defaultFontsPath;

    DAVA::UnorderedMap<DAVA::String, DAVA::UnorderedMap<DAVA::String, DAVA::FontPreset>> localizedFonts;

    QStringList availableFontLocales;
    QStringList defaultPresetNames;
    const char* defaultFontLocale;
    DAVA::String currentFontLocale; //wraps LocalizationSystem::currentLocale
};

inline const char* EditorFontSystem::GetDefaultFontLocale() const
{
    return defaultFontLocale;
}

inline const QStringList& EditorFontSystem::GetAvailableFontLocales() const
{
    return availableFontLocales;
}

inline const QStringList& EditorFontSystem::GetDefaultPresetNames() const
{
    return defaultPresetNames;
}

#endif //__EDITOR_FONT_SYSTEM__
