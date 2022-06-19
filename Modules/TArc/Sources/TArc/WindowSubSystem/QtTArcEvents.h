#pragma once

#include <Base/BaseTypes.h>

#include <QEvent>

#define QT_EVENT_TYPE(x) (static_cast<QEvent::Type>(x))
namespace DAVA
{
enum class EventsTable : int32
{
    DelayedExecute = QEvent::User + 100, // 100 events is reserved for PlatformQt implementation
    OverlayWidgetVisibilityChange,
    End
};
} // namespace DAVA
