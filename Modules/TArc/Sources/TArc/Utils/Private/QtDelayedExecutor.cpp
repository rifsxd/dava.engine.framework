#include "TArc/Utils/QtDelayedExecutor.h"
#include "TArc/WindowSubSystem/QtTArcEvents.h"

#include <QEvent>
#include <QApplication>

namespace DAVA
{
namespace QtDelayedExecutorDetail
{
class QtDelayedExecuteEvent : public QEvent
{
public:
    QtDelayedExecuteEvent(const Function<void()>& functor_)
        : QEvent(QT_EVENT_TYPE(EventsTable::DelayedExecute))
        , functor(functor_)
    {
    }

    void Execute()
    {
        functor();
    }

private:
    Function<void()> functor;
};
}

QtDelayedExecutor::QtDelayedExecutor(QObject* parent /*= nullptr*/)
    : QObject(parent)
{
}

void QtDelayedExecutor::DelayedExecute(const Function<void()>& functor)
{
    qApp->postEvent(this, new QtDelayedExecutorDetail::QtDelayedExecuteEvent(functor));
}

bool QtDelayedExecutor::event(QEvent* e)
{
    if (e->type() == QT_EVENT_TYPE(EventsTable::DelayedExecute))
    {
        QtDelayedExecutorDetail::QtDelayedExecuteEvent* event = static_cast<QtDelayedExecutorDetail::QtDelayedExecuteEvent*>(e);
        event->Execute();
        return true;
    }

    return QObject::event(e);
}
} // namespace DAVA
