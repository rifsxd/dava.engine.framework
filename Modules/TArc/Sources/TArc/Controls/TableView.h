#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Utils/QtConnections.h"

#include <QTableView>
#include <QAbstractItemModel>

namespace DAVA
{
class TableView : public ControlProxyImpl<QTableView>
{
    using TBase = ControlProxyImpl<QTableView>;

public:
    enum Fields : uint32
    {
        CurrentValue,
        Header,
        Values,
        IsReadOnly,
        ItemActivated, // void method(<RowKeyType> activatedRowIndexKey) - method argument and values row key must have same types, e.g. size_t
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    TableView(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    TableView(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void SetupControl();
    void UpdateControl(const ControlDescriptor& fields) override;

    void OnSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection);
    void OnDoubleClick(const QModelIndex& index);

    QAbstractItemModel* tableModel = nullptr;
    QtConnections connections;
    bool updateGuard = false;
    Vector<Any> header;
};
} // namespace DAVA
