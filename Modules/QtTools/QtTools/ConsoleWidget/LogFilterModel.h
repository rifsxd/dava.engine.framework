#pragma once

#include "Logger/Logger.h"

#include <QObject>
#include <QSortFilterProxyModel>
#include <QFlags>

class LogFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit LogFilterModel(QObject* parent = nullptr);
    ~LogFilterModel();
public slots:
    void SetFilters(const QVariantList& filters);

private:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    int filters = ~0;
};
