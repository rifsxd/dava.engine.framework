#include "Modules/DataBindingInspectorModule/DataBindingInspectorModel.h"

#include <UI/Formula/Private/FormulaFormatter.h>
#include <Logger/Logger.h>

#include <QMimeData>

DataBindingInspectorModel::DataBindingInspectorModel(bool plain, QObject* parent)
    : QStandardItemModel(parent)
{
    QStringList header;
    header += "Data";
    header += "Value";
    setHorizontalHeaderLabels(header);

    invisibleRootItem()->setData(QString(""), PATH_DATA);
}

QStringList DataBindingInspectorModel::mimeTypes() const
{
    QStringList types;
    types << "text/plain";
    return types;
}

Qt::ItemFlags DataBindingInspectorModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    flags |= Qt::ItemIsDragEnabled;

    return flags;
}

QMimeData* DataBindingInspectorModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData();
    if (!indexes.empty())
    {
        QStandardItem* item = this->itemFromIndex(indexes.first());
        if (item)
        {
            mimeData->setText(item->data(PATH_DATA).value<QString>());
        }
    }
    return mimeData;
}

void DataBindingInspectorModel::UpdateModel(DAVA::FormulaContext* context)
{
    using namespace DAVA;

    ClearModel();

    current.first = invisibleRootItem();
    current.second = nullptr;

    highlight = true;
    while (context != nullptr)
    {
        FormulaReflectionContext* reflectionContext = dynamic_cast<FormulaReflectionContext*>(context);
        if (reflectionContext)
        {
            Reflection ref = reflectionContext->GetReflection();
            if (ref.IsValid())
            {
                InsertDataFromReflection(ref);
            }
            context = reflectionContext->GetParent().get();
        }
        else
        {
            context = nullptr;
        }
        highlight = false;
    }
    current.first = nullptr;
    current.second = nullptr;
}

void DataBindingInspectorModel::ClearModel()
{
    invisibleRootItem()->removeRows(0, invisibleRootItem()->rowCount());
}

void DataBindingInspectorModel::InsertDataFromReflection(const DAVA::Reflection& ref)
{
    using namespace DAVA;
    Any val = ref.GetValue();

    Vector<Reflection::Field> fields = ref.GetFields();

    String res;
    if (val.CanCast<std::shared_ptr<FormulaExpression>>())
    {
        std::shared_ptr<FormulaExpression> expr = val.Cast<std::shared_ptr<FormulaExpression>>();
        res = FormulaFormatter().Format(expr.get());
    }
    else
    {
        if (ref.GetFieldsCaps().hasFlatStruct || !fields.empty())
        {
            res = "";
        }
        else
        {
            res = FormulaFormatter::AnyToString(val);
        }
    }

    if (!res.empty() && current.second)
    {
        current.second->setText(QString::fromStdString(res));
    }

    std::pair<QStandardItem*, QStandardItem*> oldCurrent = current;
    QString parentPath = current.first->data(DataBindingInspectorModel::PATH_DATA).value<QString>();

    for (auto& it : fields)
    {
        const Any& key = it.key;

        QString name = "";
        QString dot = "";

        if (key.CanGet<int32>())
        {
            name = QString("[%1]").arg(key.Get<int32>());
        }
        else if (key.CanGet<uint32>())
        {
            name = QString("[%1U]").arg(key.Get<uint32>());
        }
        else if (key.CanGet<int64>())
        {
            name = QString("[%1L]").arg(key.Get<int64>());
        }
        else if (key.CanGet<uint64>())
        {
            name = QString("[%1UL]").arg(key.Get<uint64>());
        }
        else if (key.CanGet<bool>())
        {
            name = QString("[%1]").arg(key.Get<bool>());
        }
        else if (key.CanGet<size_t>())
        {
            name = QString("[%1]").arg(key.Get<size_t>());
        }
        else if (key.CanCast<String>())
        {
            if (!parentPath.isEmpty())
            {
                dot = ".";
            }
            name = QString::fromStdString(key.Cast<String>());
        }
        else
        {
            DVASSERT(false);
        }

        current = AddChild(oldCurrent.first, name, parentPath + dot + name);
        InsertDataFromReflection(it.ref);
    }

    current = oldCurrent;
}

std::pair<QStandardItem*, QStandardItem*> DataBindingInspectorModel::AddChild(QStandardItem* parent, const QString& name, const QString& path)
{
    std::pair<QStandardItem*, QStandardItem*> pair;
    pair.first = new QStandardItem(name); // name
    QFont myFont;
    myFont.setFamily(pair.first->font().family());
    myFont.setBold(highlight);
    myFont.setItalic(!highlight);
    pair.first->setFont(myFont);

    pair.first->setData(path, DataBindingInspectorModel::PATH_DATA);
    pair.first->setEditable(false);

    pair.second = new QStandardItem(""); // value
    pair.second->setFont(myFont);
    pair.second->setData(path, DataBindingInspectorModel::PATH_DATA);
    pair.second->setEditable(false);

    QList<QStandardItem*> list;
    list.append(pair.first);
    list.append(pair.second);
    parent->appendRow(list);
    return pair;
}
