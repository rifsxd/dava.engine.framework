#include "TArc/Controls/TableView.h"
#include "TArc/Utils/ScopedValueGuard.h"

#include <QAbstractTableModel>
#include <QItemSelectionModel>
#include <QVariant>

namespace DAVA
{
namespace TableViewDetails
{
class TableModel : public QAbstractTableModel
{
public:
    int rowCount(const QModelIndex& parent) const override
    {
        return static_cast<int>(values.size());
    }

    int columnCount(const QModelIndex& parent) const override
    {
        return static_cast<int>(header.size());
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        {
            return header[section];
        }

        return QVariant();
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (role == Qt::DisplayRole)
        {
            DVASSERT(index.row() < static_cast<int>(values.size()));
            const Vector<Any>& row = values[index.row()].second;
            DVASSERT(index.column() < static_cast<int>(row.size()));
            return row[index.column()].Cast<QString>();
        }
        else if (role == Qt::SizeHintRole)
        {
            return QSize(100, 14);
        }

        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEnabled);
    }

    void BeginReset()
    {
        beginResetModel();
    }

    void EndReset()
    {
        endResetModel();
    }

    Vector<std::pair<Any, Vector<Any>>> values;
    Vector<QString> header;
};
}

TableView::TableView(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

TableView::TableView(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void TableView::SetupControl()
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    tableModel = new TableViewDetails::TableModel();
    setModel(tableModel);

    QItemSelectionModel* selectModel = new QItemSelectionModel(tableModel, this);
    connections.AddConnection(selectModel, &QItemSelectionModel::selectionChanged, MakeFunction(this, &TableView::OnSelectionChanged));
    setSelectionModel(selectModel);

    connections.AddConnection(this, &QAbstractItemView::doubleClicked, MakeFunction(this, &TableView::OnDoubleClick));
}

void TableView::UpdateControl(const ControlDescriptor& fields)
{
    SCOPED_VALUE_GUARD(bool, updateGuard, true, void());

    TableViewDetails::TableModel* m = static_cast<TableViewDetails::TableModel*>(tableModel);

    if (fields.IsChanged(Fields::Values) || fields.IsChanged(Fields::Header))
    {
        m->BeginReset();

        if (fields.IsChanged(Fields::Header))
        {
            header.clear();
            m->header.clear();

            Reflection r = model.GetField(fields.GetName(Fields::Header));
            DVASSERT(r.IsValid());
            Vector<Reflection::Field> fields = r.GetFields();
            header.reserve(fields.size());
            m->header.reserve(fields.size());
            for (const Reflection::Field& f : fields)
            {
                header.push_back(f.key.Cast<String>());
                const M::DisplayName* displayName = f.ref.GetMeta<M::DisplayName>();
                if (displayName != nullptr)
                {
                    m->header.push_back(QString::fromStdString(displayName->displayName));
                }
                else if (header.back().CanCast<QString>())
                {
                    m->header.push_back(header.back().Cast<QString>());
                }
                else
                {
                    m->header.push_back(QString(""));
                }
            }
        }

        m->values.clear();

        Reflection r = model.GetField(fields.GetName(Fields::Values));
        DVASSERT(r.IsValid());
        Vector<Reflection::Field> rows = r.GetFields();
        m->values.reserve(rows.size());
        for (const Reflection::Field& row : rows)
        {
            Vector<Any> v;
            for (const Any& key : header)
            {
                Reflection cell = row.ref.GetField(key);
                if (cell.IsValid())
                {
                    v.push_back(cell.GetValue());
                }
                else
                {
                    v.push_back("");
                }
            }

            m->values.emplace_back(row.key, std::move(v));
        }

        m->EndReset();
    }

    if (fields.IsChanged(Fields::CurrentValue) || fields.IsChanged(Fields::Values))
    {
        Any currentValue = GetFieldValue(Fields::CurrentValue, Any());
        bool currentValueSet = false;
        for (size_t i = 0; i < m->values.size(); ++i)
        {
            if (m->values[i].first == currentValue)
            {
                selectRow(static_cast<int>(i));
                currentValueSet = true;
                break;
            }
        }

        if (currentValueSet == false)
        {
            wrapper.SetFieldValue(GetFieldName(Fields::CurrentValue), Any());
        }
    }

    if (fields.IsChanged(Fields::IsReadOnly))
    {
        setEnabled(!IsValueReadOnly(fields, Fields::CurrentValue, Fields::IsReadOnly));
    }
}

void TableView::OnSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection)
{
    SCOPED_VALUE_GUARD(bool, updateGuard, true, void());

    QModelIndexList indexList = selectionModel()->selectedRows();
    DVASSERT(indexList.size() < 2);
    if (indexList.size() > 0)
    {
        const QModelIndex& index = indexList.front();
        TableViewDetails::TableModel* m = static_cast<TableViewDetails::TableModel*>(tableModel);
        DVASSERT(index.row() < m->values.size());
        wrapper.SetFieldValue(GetFieldName(Fields::CurrentValue), m->values[index.row()].first);
    }
    else
    {
        wrapper.SetFieldValue(GetFieldName(Fields::CurrentValue), Any());
    }
}

void TableView::OnDoubleClick(const QModelIndex& index)
{
    FastName activateMethodName = GetFieldName(Fields::ItemActivated);
    if (activateMethodName.IsValid())
    {
        AnyFn method = model.GetMethod(activateMethodName.c_str());
        if (method.IsValid())
        {
            TableViewDetails::TableModel* m = static_cast<TableViewDetails::TableModel*>(tableModel);
            const AnyFn::Params& params = method.GetInvokeParams();

            const Type* retType = params.retType;
            Vector<const Type*> argsType = params.argsType;

            DVASSERT(index.row() < static_cast<int>(m->values.size()));
            Any rowKey = m->values[index.row()].first;

            if (argsType.size() == 1)
            {
                method.InvokeWithCast(rowKey);
            }
            else
            {
                DVASSERT(false, "We could invoke only methods with row key as argument");
            }
        }
    }
}

} // namespace DAVA
