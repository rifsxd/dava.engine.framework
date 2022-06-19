#pragma once

#include "TArc/WindowSubSystem/QtTArcEvents.h"
#include <QEvent>

namespace DAVA
{
class QtOverlayWidgetVisibilityChange : public QEvent
{
public:
    QtOverlayWidgetVisibilityChange(bool isVisible_);

    bool IsVisible() const;

private:
    bool isVisible = false;
};

inline QtOverlayWidgetVisibilityChange::QtOverlayWidgetVisibilityChange(bool isVisible_)
    : QEvent(static_cast<QEvent::Type>(EventsTable::OverlayWidgetVisibilityChange))
    , isVisible(isVisible_)
{
}

inline bool QtOverlayWidgetVisibilityChange::IsVisible() const
{
    return isVisible;
}
} // namespace DAVA
