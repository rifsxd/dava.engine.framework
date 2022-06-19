#pragma once

#include <QStandardItemModel>

#include "Model/PackageHierarchy/PackageBaseNode.h"

#include <UI/Formula/FormulaContext.h>

class PackageBaseNode;

class DataBindingInspectorModel : public QStandardItemModel
{
    Q_OBJECT

public:
    enum
    {
        PATH_DATA = Qt::UserRole + 1,
    };

    DataBindingInspectorModel(bool plain, QObject* parent = Q_NULLPTR);
    QStringList mimeTypes() const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;

    void UpdateModel(DAVA::FormulaContext* context);
    void ClearModel();

private:
    void InsertDataFromReflection(const DAVA::Reflection& ref);

    std::pair<QStandardItem*, QStandardItem*> AddChild(QStandardItem* parent, const QString& name, const QString& path);

    std::pair<QStandardItem*, QStandardItem*> current;

    bool plain = false;
    bool highlight = false;
};
