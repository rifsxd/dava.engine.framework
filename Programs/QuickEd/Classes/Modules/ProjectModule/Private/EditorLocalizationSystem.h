#ifndef __EDITOR_LOCALIZATION_SYSTEM_H__
#define __EDITOR_LOCALIZATION_SYSTEM_H__

#include <QObject>
#include <QMap>
#include <QStringList>

class QDir;
namespace DAVA
{
class ContextAccessor;
}

class EditorLocalizationSystem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentLocale READ GetCurrentLocale WRITE SetCurrentLocale NOTIFY CurrentLocaleChanged)

public:
    explicit EditorLocalizationSystem(DAVA::ContextAccessor* accessor, QObject* parent = nullptr);

    QStringList GetAvailableLocales() const;
    QString GetCurrentLocale() const;
    void SetCurrentLocale(const QString& localeStr);

    void SetDirectory(const QDir& dir);
    void Cleanup();

signals:
    void CurrentLocaleChanged(const QString& locale);

private:
    QStringList availableLocales;
    QString currentLocale;

    DAVA::ContextAccessor* accessor;
};


#endif //__EDITOR_LOCALIZATION_SYSTEM_H__
