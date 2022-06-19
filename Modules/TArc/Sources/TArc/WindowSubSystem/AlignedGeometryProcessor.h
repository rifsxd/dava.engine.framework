#pragma once

#include "TArc/WindowSubSystem/UI.h"

#include <QRect>

namespace DAVA
{
class AlignedGeometryProcessor : public IGeometryProcessor
{
public:
    AlignedGeometryProcessor(eAlign alignment, const QPoint& offset);

    QRect GetWidgetGeometry(QWidget* parent, QWidget* content) const override;

private:
    eAlign alignment;
    QPoint offset;
};
} // namespace DAVA
