#include "TArc/SharedModules/ThemesModule/ThemesModule.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/QtAction.h"

#include <Engine/Engine.h>
#include <Engine/PlatformApiQt.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Debug/DVAssert.h>
#include <Base/GlobalEnum.h>

#include <QAction>
#include <QtGlobal>
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QMenu>

ENUM_DECLARE(DAVA::ThemesSettings::eTheme)
{
    ENUM_ADD_DESCR(DAVA::ThemesSettings::Light, "Classic");
    ENUM_ADD_DESCR(DAVA::ThemesSettings::Dark, "Dark");
};

namespace ThemesDetail
{
#if defined(__DAVAENGINE_WINDOWS__)
int fontSize = 10;
#elif defined(__DAVAENGINE_MACOS__)
int fontSize = 13;
#endif
}

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ThemesSettings)
{
    ReflectionRegistrator<ThemesSettings>::Begin()[M::HiddenField()]
    .ConstructorByPointer()
    .Field("currentTheme", &ThemesSettings::theme)
    .End();
}

ThemesSettings::eTheme ThemesSettings::GetTheme() const
{
    return theme;
}

void ThemesSettings::SetTheme(ThemesSettings::eTheme theme_, QApplication* app)
{
    theme = theme_;
    switch (theme)
    {
    case ThemesSettings::Light:
        ApplyLightTheme(app);
        break;
    case ThemesSettings::Dark:
        ApplyDarkTheme(app);
        break;
    default:
        DVASSERT(false);
        break;
    }
}

void ThemesSettings::ApplyLightTheme(QApplication* app)
{
    app->setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, lightWindowColor);
    lightPalette.setColor(QPalette::WindowText, QColor(0x25, 0x25, 0x25));
    lightPalette.setColor(QPalette::Disabled, QPalette::WindowText, lightDisabledTextColor);

    lightPalette.setColor(QPalette::Dark, lightWindowColor);
    lightPalette.setColor(QPalette::Midlight, lightWindowColor.darker(130));
    lightPalette.setColor(QPalette::Mid, QColor(0xD3, 0xD3, 0xD3));

    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::Disabled, QPalette::Base, lightWindowColor);

    lightPalette.setColor(QPalette::AlternateBase, lightWindowColor);
    lightPalette.setColor(QPalette::ToolTipBase, lightWindowColor);
    lightPalette.setColor(QPalette::ToolTipText, lightTextColor);

    lightPalette.setColor(QPalette::Text, lightTextColor);
    lightPalette.setColor(QPalette::Disabled, QPalette::Text, lightDisabledTextColor);

    lightPalette.setColor(QPalette::Button, lightWindowColor);
    lightPalette.setColor(QPalette::ButtonText, lightTextColor.lighter(130));
    lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText, lightDisabledTextColor);

    lightPalette.setColor(QPalette::Light, lightWindowColor);

    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, Qt::blue);
    lightPalette.setColor(QPalette::Disabled, QPalette::Light, lightWindowColor);

    lightPalette.setColor(QPalette::Highlight, QColor(0x88, 0xBB, 0xFF));
    lightPalette.setColor(QPalette::Inactive, QPalette::Highlight, lightWindowColor.darker(120));
    lightPalette.setColor(QPalette::Disabled, QPalette::Highlight, lightWindowColor.darker(120));

    lightPalette.setColor(QPalette::HighlightedText, lightTextColor);
    lightPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, lightDisabledTextColor);

    QFile styleSheet(":/TArc/Resources/LightTheme.qss");
    const bool opened = styleSheet.open(QIODevice::ReadOnly);
    DVASSERT(opened);
    QString styleSheetContent = styleSheet.readAll();

    styleSheetContent.insert(0, QString("* {font-size:%1pt}\n").arg(ThemesDetail::fontSize));

    app->setPalette(lightPalette);
    app->setStyleSheet(styleSheetContent);
}

void ThemesSettings::ApplyDarkTheme(QApplication* app)
{
    app->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;

    darkPalette.setColor(QPalette::Window, darkWindowColor);
    darkPalette.setColor(QPalette::WindowText, darkTextColor);

    darkPalette.setColor(QPalette::Dark, darkWindowColor);
    darkPalette.setColor(QPalette::Midlight, darkWindowColor.lighter(130));
    darkPalette.setColor(QPalette::Mid, QColor(0x4C, 0x4C, 0x4C));

    darkPalette.setColor(QPalette::Base, darkWindowColor.darker(130));
    darkPalette.setColor(QPalette::AlternateBase, darkWindowColor);
    darkPalette.setColor(QPalette::ToolTipBase, darkWindowColor);
    darkPalette.setColor(QPalette::ToolTipText, darkTextColor);

    darkPalette.setColor(QPalette::Text, darkTextColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, darkDisabledTextColor);

    darkPalette.setColor(QPalette::Button, darkWindowColor);
    darkPalette.setColor(QPalette::ButtonText, darkTextColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, darkDisabledTextColor);

    darkPalette.setColor(QPalette::Light, darkWindowColor);

    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(0x2A, 0x82, 0xDA));
    darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkWindowColor);

    darkPalette.setColor(QPalette::Highlight, QColor(0x37, 0x63, 0xAD));
    darkPalette.setColor(QPalette::Inactive, QPalette::Highlight, darkDisabledTextColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, darkDisabledTextColor);

    darkPalette.setColor(QPalette::HighlightedText, QColor(Qt::white));
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(0xC0, 0xC0, 0xC0));

    QFile styleSheet(":/TArc/Resources/DarkTheme.qss");
    const bool opened = styleSheet.open(QIODevice::ReadOnly);
    DVASSERT(opened);
    QString styleSheetContent = styleSheet.readAll();

    auto colorToString = [](const QColor& color)
    {
        return QString("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green())
        .arg(color.blue())
        .arg(color.alpha());
    };

    QString tabBarStyle = QString("QTabBar::tab:selected { color: %1 }"
                                  "QTabBar::tab:!selected { color: %2 }")
                          .arg(colorToString(darkTextColor))
                          .arg(colorToString(darkDisabledTextColor));

    styleSheetContent.insert(0, QString("* {font-size:%1pt}\n").arg(ThemesDetail::fontSize));
    styleSheetContent.append(tabBarStyle);

    app->setPalette(darkPalette);
    app->setStyleSheet(styleSheetContent);
}

QColor ThemesSettings::GetViewLineAlternateColor()
{
    return theme == Light ? Qt::lightGray : QColor(0x3F, 0x3F, 0x46);
}

QColor ThemesSettings::GetChangedPropertyColor()
{
    return theme == Light ? Qt::black : QColor(0xE3, 0xE1, 0x8A);
}

QColor ThemesSettings::GetPrototypeColor()
{
    return theme == Light ? QColor(Qt::blue) : QColor("CadetBlue");
}

QColor ThemesSettings::GetStyleSheetNodeColor()
{
    return theme == Light ? QColor(Qt::darkGreen) : QColor("light green");
}

QColor ThemesSettings::GetRulerWidgetBackgroungColor()
{
    return theme == Light ? lightWindowColor : darkWindowColor;
}

QColor ThemesSettings::GetRulerTextColor()
{
    return theme == Light ? lightDisabledTextColor : QColor(0x62, 0x62, 0x62);
}

QColor ThemesSettings::GetHighligtedItemTextColor()
{
    return theme == Light ? QColor(0x37, 0x63, 0xAD) : QColor(0x88, 0xBB, 0xFF);
}

QColor ThemesSettings::GetErrorColor()
{
    return theme == Light ? QColor(0xDF, 0x1A, 0x21) : QColor(0xCE, 0x3D, 0x42);
}

DAVA_VIRTUAL_REFLECTION_IMPL(ThemesModule)
{
    ReflectionRegistrator<ThemesModule>::Begin()
    .ConstructorByPointer()
    .Field("currentTheme", &ThemesModule::GetTheme, &ThemesModule::SetTheme)
    .End();
}

ThemesModule::ThemesModule()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ThemesSettings);
}

void ThemesModule::PostInit()
{
    executor.DelayedExecute([this]() {
        UI* ui = GetUI();
        ContextAccessor* accessor = GetAccessor();
        QAction* menu = new QAction("App style", nullptr);

        QActionGroup* styleGroup = new QActionGroup(menu);

        ActionPlacementInfo placementInfo(CreateMenuPoint(QList<QString>() << "Tools"));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, menu);

        {
            ActionPlacementInfo placementInfo(CreateMenuPoint(QList<QString>() << "Tools"
                                                                               << "App style"));

            const EnumMap* map = GlobalEnumMap<ThemesSettings::eTheme>::Instance();
            for (size_t i = 0; i < map->GetCount(); ++i)
            {
                int value = 0;
                map->GetValue(i, value);
                QtAction* action = new QtAction(accessor, map->ToString(value), nullptr);
                FieldDescriptor descr;
                descr.type = ReflectedTypeDB::Get<ThemesSettings>();
                descr.fieldName = FastName("currentTheme");
                styleGroup->addAction(action);

                action->SetStateUpdationFunction(QtAction::Checked, descr, [value](const Any& v) -> Any {
                    if (v.CanCast<ThemesSettings::eTheme>() == false)
                    {
                        return false;
                    }

                    return v.Cast<ThemesSettings::eTheme>() == static_cast<ThemesSettings::eTheme>(value);
                });

                connections.AddConnection(action, &QAction::triggered, Bind(&ThemesModule::SetTheme, this, static_cast<ThemesSettings::eTheme>(value)));
                ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
            }
        }
    });

    ThemesSettings* settings = GetAccessor()->GetGlobalContext()->GetData<ThemesSettings>();
    SetTheme(settings->theme);
}

ThemesSettings::eTheme ThemesModule::GetTheme() const
{
    return GetAccessor()->GetGlobalContext()->GetData<ThemesSettings>()->theme;
}

void ThemesModule::SetTheme(ThemesSettings::eTheme theme)
{
    GetAccessor()->GetGlobalContext()->GetData<ThemesSettings>()->SetTheme(theme, PlatformApi::Qt::GetApplication());
}
} // namespace DAVA
