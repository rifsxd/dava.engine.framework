#pragma once

#include <QTreeView>

class PackageTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit PackageTreeView(QWidget* parent = NULL);

    //override setModel method to repaint checkboxes on dataChanged signal
    void setModel(QAbstractItemModel* model) override;

private:
    void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    bool IsIndexCheckable(const QModelIndex& index) const;
    bool IsMouseUnderCheckBox(const QPoint& pos) const;

    bool eventFilter(QObject* obj, QEvent* event) override;

    //hash-like variable to remember checkBox rect
    mutable QRect checkBoxRect;
};
