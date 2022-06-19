#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/Controls/ControlProxy.h"

#include "Debug/DVAssert.h"
#include "Base/BaseTypes.h"

#include <QAction>
#include <QList>
#include <QPair>
#include <QUrlQuery>

namespace DAVA
{
namespace ActionUtilsDetail
{
Vector<std::pair<QString, InsertionParams::eInsertionMethod>> convertionMap = {
    { QString("after"), InsertionParams::eInsertionMethod::AfterItem },
    { QString("before"), InsertionParams::eInsertionMethod::BeforeItem }
};

QString ConvertToUrlPath(QList<QString> menusPath)
{
    QString path;
    if (!menusPath.isEmpty())
    {
        path = menusPath.front();
        menusPath.pop_front();
        while (!menusPath.isEmpty())
        {
            path += "$/" + menusPath.front();
            menusPath.pop_front();
        }
    }

    return path;
}

QUrl CreateUrl(const QString scemeName, const QString& path, const InsertionParams& params)
{
    QUrl url;
    url.setPath(path);
    url.setScheme(scemeName);

    QList<QPair<QString, QString>> items;
    items.push_back(qMakePair(QString("eInsertionMethod"), InsertionParams::Convert(params.method)));
    items.push_back(qMakePair(QString("itemName"), params.item));

    QUrlQuery query;
    query.setQueryItems(items);
    url.setQuery(query);

    return url;
}
}

InsertionParams::InsertionParams(eInsertionMethod method_, const QString& item_ /*= QString()*/)
    : method(method_)
    , item(item_)
{
}

QUrl CreateMenuPoint(const QString& menuName, const InsertionParams& params)
{
    return ActionUtilsDetail::CreateUrl(menuScheme, menuName, params);
}

QUrl CreateMenuPoint(QList<QString> menusPath, const InsertionParams& params /*= InsertionParams()*/)
{
    QString path = ActionUtilsDetail::ConvertToUrlPath(menusPath);
    return CreateMenuPoint(path, params);
}

QUrl CreateToolbarPoint(const QString& toolbarName, const InsertionParams& params)
{
    return ActionUtilsDetail::CreateUrl(toolbarScheme, toolbarName, params);
}

QUrl CreateToolbarMenuPoint(const QString& toolbarName, QList<QString> menusPath, const InsertionParams& params /*= InsertionParams()*/)
{
    menusPath.prepend(toolbarName);
    QString path = ActionUtilsDetail::ConvertToUrlPath(menusPath);
    return ActionUtilsDetail::CreateUrl(toolbarScheme, path, params);
}

QUrl CreateStatusbarPoint(bool isPermanent, uint32 stretchFactor, const InsertionParams& params)
{
    QUrl url = ActionUtilsDetail::CreateUrl(statusbarScheme, "", params);
    if (isPermanent)
    {
        url.setPath(permanentStatusbarAction);
    }

    url.setFragment(QString::number(stretchFactor));

    return url;
}

QUrl CreateInvisiblePoint()
{
    QUrl url = ActionUtilsDetail::CreateUrl(invisibleScheme, "", InsertionParams());
    return url;
}

void AttachWidgetToAction(QAction* action, QWidget* widget)
{
    action->setData(QVariant::fromValue(widget));
}

void AttachWidgetToAction(QAction* action, ControlProxy* control)
{
    AttachWidgetToAction(action, control->ToWidgetCast());
}

QWidget* GetAttachedWidget(QAction* action)
{
    QVariant data = action->data();
    if (data.canConvert<QWidget*>())
    {
        return data.value<QWidget*>();
    }

    return nullptr;
}

InsertionParams::eInsertionMethod InsertionParams::Convert(const QString& v)
{
    auto iter = std::find_if(ActionUtilsDetail::convertionMap.begin(), ActionUtilsDetail::convertionMap.end(), [&v](const std::pair<QString, InsertionParams::eInsertionMethod>& node)
                             {
                                 return node.first == v;
                             });

    if (iter == ActionUtilsDetail::convertionMap.end())
    {
        DVASSERT(false);
        return InsertionParams::eInsertionMethod::AfterItem;
    }

    return iter->second;
}

QString InsertionParams::Convert(eInsertionMethod v)
{
    auto iter = std::find_if(ActionUtilsDetail::convertionMap.begin(), ActionUtilsDetail::convertionMap.end(), [&v](const std::pair<QString, InsertionParams::eInsertionMethod>& node)
                             {
                                 return node.second == v;
                             });

    if (iter == ActionUtilsDetail::convertionMap.end())
    {
        DVASSERT(false);
        return ActionUtilsDetail::convertionMap[0].first;
    }

    return iter->first;
}

InsertionParams InsertionParams::Create(const QUrl& url)
{
    QUrlQuery query(url.query());
    InsertionParams params;
    params.item = query.queryItemValue("itemName");
    params.method = InsertionParams::Convert(query.queryItemValue("eInsertionMethod"));
    return params;
}

void MakeActionKeyBindable(QAction* action, const KeyBindableActionInfo& info)
{
    QUrl url;
    url.setScheme("actionbindablebinfo");
    url.setPath(info.blockName);

    QList<QPair<QString, QString>> items;
    items.push_back(qMakePair(QString("context"), QString::number(static_cast<int>(info.context))));
    items.push_back(qMakePair(QString("readonly"), QString::number(info.readOnly == false ? 0 : 1)));
    items.push_back(qMakePair(QString("defSequencesCount"), QString::number(info.defaultShortcuts.size())));
    for (int i = 0; i < info.defaultShortcuts.size(); ++i)
    {
        QString sequence = info.defaultShortcuts[i].toString();
        items.push_back(qMakePair(QString("shortcut_%1").arg(i), sequence));
    }

    QUrlQuery query;
    query.setQueryItems(items);
    url.setQuery(query);

    action->setProperty("keyBindableInfo", QVariant::fromValue(url));
}

bool GetActionKeyBindableInfo(QAction* action, KeyBindableActionInfo& info)
{
    QVariant value = action->property("keyBindableInfo");
    if (value.canConvert<QUrl>() == false)
    {
        return false;
    }

    QUrl url = value.value<QUrl>();
    if (url.scheme() != "actionbindablebinfo")
    {
        DVASSERT(false);
        return false;
    }

    info.blockName = url.path();

    QUrlQuery query(url.query());
    {
        bool parsed = false;
        int context = query.queryItemValue("context").toInt(&parsed);
        if (parsed == true)
        {
            info.context = static_cast<Qt::ShortcutContext>(context);
        }
    }
    info.readOnly = query.queryItemValue("readonly").toInt() == 0 ? false : true;
    int defSequencesCount = query.queryItemValue("defSequencesCount").toInt();
    info.defaultShortcuts.reserve(defSequencesCount);
    for (int i = 0; i < defSequencesCount; ++i)
    {
        QString sequence = query.queryItemValue(QString("shortcut_%1").arg(i));
        QKeySequence keySequence = QKeySequence::fromString(sequence);
        info.defaultShortcuts.push_back(keySequence);
    }

    return true;
}
} // namespace DAVA
