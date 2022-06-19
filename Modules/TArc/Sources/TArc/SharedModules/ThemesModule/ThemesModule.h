#pragma once

#include "TArc/DataProcessing/SettingsNode.h"
#include "TArc/Core/ClientModule.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Utils/QtDelayedExecutor.h"

class QApplication;

namespace DAVA
{
class ThemesSettings : public SettingsNode
{
public:
    enum eTheme
    {
        Light,
        Dark
    };

    QColor GetViewLineAlternateColor();
    QColor GetChangedPropertyColor();
    QColor GetPrototypeColor();
    QColor GetStyleSheetNodeColor();
    QColor GetRulerWidgetBackgroungColor();
    QColor GetRulerTextColor();
    QColor GetHighligtedItemTextColor();
    QColor GetErrorColor();

    eTheme GetTheme() const;
    void SetTheme(eTheme theme, QApplication* app);

private:
    friend class ThemesModule;
    eTheme theme = eTheme::Dark;

    void ApplyLightTheme(QApplication* app);
    void ApplyDarkTheme(QApplication* app);

    QColor lightTextColor = QColor(Qt::black);
    QColor lightDisabledTextColor = QColor(0x0, 0x0, 0x0, 0x50);
    QColor lightWindowColor = QColor(0xF0, 0xF0, 0xF0);
    QColor darkTextColor = QColor(0xF2, 0xF2, 0xF2);
    QColor darkDisabledTextColor = QColor(0x75, 0x75, 0x75);
    QColor darkWindowColor = QColor(0x32, 0x32, 0x32);

    DAVA_VIRTUAL_REFLECTION(ThemesSettings, SettingsNode);
};

class ThemesModule : public ClientModule
{
public:
    ThemesModule();
    void PostInit() override;

private:
    ThemesSettings::eTheme GetTheme() const;
    void SetTheme(ThemesSettings::eTheme theme);

    QtConnections connections;
    QtDelayedExecutor executor;

    DAVA_VIRTUAL_REFLECTION(ThemesModule, ClientModule);
};
} // namespace DAVA
