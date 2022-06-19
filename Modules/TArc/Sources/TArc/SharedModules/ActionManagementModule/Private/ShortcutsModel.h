#pragma once

#include "TArc/WindowSubSystem/Private/UIManager.h"
#include <QAbstractItemModel>
#include <QString>

namespace DAVA
{
class ShortcutsModel : public QAbstractItemModel
{
public:
    void SetData(const Vector<KeyBindableAction>& actionsData);
    const KeyBindableAction* GetKeyBindableAction(const QModelIndex& index);
    QModelIndex GetIndex(const QString& blockName) const;
    QModelIndex GetIndex(const QString& blockName, QPointer<QAction> action) const;

    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    Vector<QString> blocks;
    struct Node
    {
        QString conflictsWith;
        KeyBindableAction action;
    };
    Map<QString, Vector<Node>> actions;
};
} // namespace DAVA
