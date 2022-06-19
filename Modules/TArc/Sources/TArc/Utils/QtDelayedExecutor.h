#pragma once

#include <Functional/Function.h>
#include <QObject>

namespace DAVA
{
class QtDelayedExecutor : public QObject
{
public:
    QtDelayedExecutor(QObject* parent = nullptr);

    void DelayedExecute(const Function<void()>& functor);

protected:
    bool event(QEvent* e) override;
};
} // namespace DAVA
