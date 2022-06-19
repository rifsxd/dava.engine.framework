#pragma once

#include <Base/BaseTypes.h>

#include <QLayout>
#include <QStyle>

namespace DAVA
{
class QtFlowLayout : public QLayout
{
public:
    explicit QtFlowLayout(QWidget* parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit QtFlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~QtFlowLayout();

    void addItem(QLayoutItem* item) override;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect& rect) override;
    QSize sizeHint() const override;
    QLayoutItem* takeAt(int index) override;

    int GetHorizontalSpacing() const;
    int GetVerticalSpacing() const;

private:
    int DoLayout(const QRect& rect, bool testOnly) const;
    int SmartSpacing(QStyle::PixelMetric pm) const;

    Vector<QLayoutItem*> items;
    int hSpace;
    int vSpace;
};

} // namespace DAVA
