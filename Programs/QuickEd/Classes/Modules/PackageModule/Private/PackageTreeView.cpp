#include "Classes/Modules/PackageModule/Private/PackageTreeView.h"
#include "Classes/Modules/PackageModule/Private/PackageModel.h"

#include <QPainter>
#include <QHeaderView>
#include <QApplication>
#include <QStyleOptionButton>
#include <QMouseEvent>

PackageTreeView::PackageTreeView(QWidget* parent /*= NULL*/)
    : QTreeView(parent)
{
    viewport()->installEventFilter(this);
}

void PackageTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QTreeView::drawRow(painter, option, index);
    if (IsIndexCheckable(index))
    {
        QStyleOptionViewItem checkBoxOption(option);
        checkBoxOption.rect.setX(columnViewportPosition(0));

        checkBoxOption.displayAlignment = Qt::AlignLeft | Qt::AlignTop;

        switch (index.data(PackageModel::PackageCheckStateRole).toInt())
        {
        case Qt::Unchecked:
            checkBoxOption.state |= QStyle::State_Off;
            break;
        case Qt::PartiallyChecked:
            checkBoxOption.state |= QStyle::State_NoChange;
            break;
        case Qt::Checked:
            checkBoxOption.state |= QStyle::State_On;
            break;
        }
        checkBoxOption.features |= QStyleOptionViewItem::HasCheckIndicator;

        QStyle* currentStyle = style();

        checkBoxRect = currentStyle->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &checkBoxOption, this);
        checkBoxOption.rect = checkBoxRect;

        currentStyle->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &checkBoxOption, painter, this);
    }
}

bool PackageTreeView::IsIndexCheckable(const QModelIndex& index) const
{
    return index.data(PackageModel::PackageCheckStateRole).isValid();
}

bool PackageTreeView::IsMouseUnderCheckBox(const QPoint& pos) const
{
    QModelIndex index = indexAt(pos);
    if (index.isValid())
    {
        if (IsIndexCheckable(index))
        {
            QRect rect = visualRect(index);
            rect.setX(checkBoxRect.x());
            rect.setSize(checkBoxRect.size());
            return rect.contains(pos);
        }
    }
    return false;
}

bool PackageTreeView::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == viewport() &&
        (event->type() == QEvent::MouseButtonPress ||
         event->type() == QEvent::MouseButtonRelease ||
         event->type() == QEvent::MouseButtonDblClick))
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton)
        {
            QPoint pos = mouseEvent->pos();
            if (IsMouseUnderCheckBox(pos))
            {
                if (event->type() == QEvent::MouseButtonRelease)
                {
                    QModelIndex index = indexAt(pos);
                    QVariant checked = index.data(PackageModel::PackageCheckStateRole);
                    Qt::CheckState newState = checked.toInt() == Qt::Checked ? Qt::Unchecked : Qt::Checked;
                    model()->setData(index, newState, PackageModel::PackageCheckStateRole);
                    viewport()->update();
                }
                return true;
            }
        }
    }
    return QTreeView::eventFilter(obj, event);
}

void PackageTreeView::setModel(QAbstractItemModel* model)
{
    QTreeView::setModel(model);
    connect(this->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)), this, SLOT(update()));
}
