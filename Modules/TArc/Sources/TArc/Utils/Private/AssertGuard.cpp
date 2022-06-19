#include "TArc/Utils/AssertGuard.h"
#include "TArc/Utils/ScopedValueGuard.h"

#if defined(__DAVAENGINE_MACOS__)
#include "TArc/Utils/AssertGuardMacOSHack.h"
#endif

#include <Debug/DVAssert.h>
#include <Debug/DVAssertDefaultHandlers.h>
#include <Concurrency/LockGuard.h>
#include <Concurrency/Thread.h>

#include <Base/StaticSingleton.h>

#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QWidget>

namespace DAVA
{
class EventFilter final : public QObject
{
public:
    EventFilter()
    {
        QWidgetList lst = qApp->allWidgets();
        for (int i = 0; i < lst.size(); ++i)
        {
            QWidget* widget = lst[i];
            widget->installEventFilter(this);
        }

        QAbstractEventDispatcher* dispatcher = qApp->eventDispatcher();
        qApp->installEventFilter(this);
        if (dispatcher != nullptr)
        {
            dispatcher->installEventFilter(this);
        }
    }

    bool eventFilter(QObject* obj, QEvent* e) override
    {
        if (e->spontaneous())
        {
            return true;
        }

        QEvent::Type type = e->type();
        switch (type)
        {
        case QEvent::Timer:
        case QEvent::Expose:
        case QEvent::Paint:
            return true;
        default:
            break;
        }

        return false;
    }
};

class AssertGuard : public StaticSingleton<AssertGuard>
{
public:
    void SetMode(eApplicationMode mode_)
    {
        mode = mode_;
    }

    Assert::FailBehaviour HandleAssert(const Assert::AssertInfo& assertInfo)
    {
        LockGuard<Mutex> mutexGuard(mutex);
        ScopedValueGuard<bool> valueGuard(isInAssert, true);

#if defined(__DAVAENGINE_MACOS__)
        MacOSRunLoopGuard macOSGuard;
#endif

        Assert::FailBehaviour behaviour = Assert::FailBehaviour::Default;
        switch (mode)
        {
        case eApplicationMode::GUI_MODE:
            Assert::DefaultLoggerHandler(assertInfo);
            behaviour = Assert::DefaultDialogBoxHandler(assertInfo);
            break;

        case eApplicationMode::CONSOLE_MODE:
        case eApplicationMode::TEST_MODE:
            behaviour = Assert::DefaultLoggerHandler(assertInfo);
            if (Assert::FailBehaviour::Halt != behaviour)
            {
                behaviour = Assert::DefaultDebuggerBreakHandler(assertInfo);
            }
            break;

        default:
            break;
        }

        return behaviour;
    }

    bool IsInsideAssert() const
    {
        return isInAssert;
    }

private:
    Mutex mutex;
    bool isInAssert = false;
    eApplicationMode mode;
};

Assert::FailBehaviour AssertHandler(const Assert::AssertInfo& assertInfo)
{
    return AssertGuard::Instance()->HandleAssert(assertInfo);
}

void SetupToolsAssertHandlers(eApplicationMode mode)
{
    AssertGuard::Instance()->SetMode(mode);
    Assert::AddHandler(&AssertHandler);
}

bool IsInsideAssertHandler()
{
    return AssertGuard::Instance()->IsInsideAssert();
}
} // namespace DAVA
