#include "TArc/Controls/QtFlowLayout.h"

#include <QWidget>

namespace DAVA
{
QtFlowLayout::QtFlowLayout(QWidget* parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent)
    , hSpace(hSpacing)
    , vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

QtFlowLayout::QtFlowLayout(int margin, int hSpacing, int vSpacing)
    : hSpace(hSpacing)
    , vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

QtFlowLayout::~QtFlowLayout()
{
    for (QLayoutItem* item : items)
    {
        delete item;
    }
    items.clear();
}

void QtFlowLayout::addItem(QLayoutItem* item)
{
    items.push_back(item);
}

int QtFlowLayout::GetHorizontalSpacing() const
{
    if (hSpace >= 0)
    {
        return hSpace;
    }
    else
    {
        return SmartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int QtFlowLayout::GetVerticalSpacing() const
{
    if (vSpace >= 0)
    {
        return vSpace;
    }
    else
    {
        return SmartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int QtFlowLayout::count() const
{
    return static_cast<int>(items.size());
}

QLayoutItem* QtFlowLayout::itemAt(int index) const
{
    if (index >= 0 && index < items.size())
    {
        return items[index];
    }

    return nullptr;
}

QLayoutItem* QtFlowLayout::takeAt(int index)
{
    if (index >= 0 && index < items.size())
    {
        QLayoutItem* item = items[index];
        items.erase(items.begin() + index);
        return item;
    }

    return 0;
}

Qt::Orientations QtFlowLayout::expandingDirections() const
{
    return 0;
}

bool QtFlowLayout::hasHeightForWidth() const
{
    return true;
}

int QtFlowLayout::heightForWidth(int width) const
{
    int height = DoLayout(QRect(0, 0, width, 0), true);
    return height;
}

void QtFlowLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    DoLayout(rect, false);
}

QSize QtFlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize QtFlowLayout::minimumSize() const
{
    QSize size;
    QLayoutItem* item;
    foreach (item, items)
    {
        size = size.expandedTo(item->minimumSize());
    }

    size += QSize(2 * margin(), 2 * margin());
    return size;
}

int QtFlowLayout::DoLayout(const QRect& rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    QLayoutItem* item;
    foreach (item, items)
    {
        QWidget* wid = item->widget();
        int spaceX = GetHorizontalSpacing();
        if (spaceX == -1)
        {
            spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        }
        int spaceY = GetVerticalSpacing();
        if (spaceY == -1)
        {
            spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        }

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
        {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
        {
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        }

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int QtFlowLayout::SmartSpacing(QStyle::PixelMetric pm) const
{
    QObject* parent = this->parent();
    if (!parent)
    {
        return -1;
    }
    else if (parent->isWidgetType())
    {
        QWidget* pw = static_cast<QWidget*>(parent);
        return pw->style()->pixelMetric(pm, 0, pw);
    }
    else
    {
        return static_cast<QLayout*>(parent)->spacing();
    }
}

} // namespace DAVA
